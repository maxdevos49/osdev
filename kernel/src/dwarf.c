#include "dwarf.h"
#include "elf.h"
#include "error.h"
#include "memory/heap.h"
#include "string/utility.h"

#define DEBUG_STR(ctx, offset) ((char *)(ctx->debug_str + offset))
#define DEBUG_LINE_STR(ctx, offset) ((char *)(ctx->debug_line_str + offset))

struct DWARF_CONTEXT {
	void *debug_info;
	size_t debug_info_size;
	void *debug_abbrev;
	size_t debug_abbrev_size;
	void *debug_aranges;
	size_t debug_aranges_size;
	void *debug_line;
	size_t debug_line_size;
	void *debug_line_str;
	size_t debug_line_str_size;
	void *debug_str;
	size_t debug_str_size;
};

struct DIE_ATTRIBUTE {
	uint16_t attribute_name_code;
	uint16_t attribute_form_code;
	// Value should be interpreted based on the form_code value. This might be a
	// pointer or a simple integer value.
	uint64_t value;
};

struct DIE {
	uint16_t abbreviation_code;
	uint16_t tag_code;

	uint8_t has_children;

	uint8_t attribute_count;
	struct DIE_ATTRIBUTE attributes[15];
};

static inline struct DIE_ATTRIBUTE *_die_attribute(struct DIE *die,
												   enum DW_AT attribute_code)
{
	for (int i = 0; i < die->attribute_count; i++) {
		if (die->attributes[i].attribute_name_code == attribute_code) {
			return &die->attributes[i];
		}
	}

	return NULL;
}

// Finds the associated abbreviation entry based on the abbreviation code.
// Updates the `abbrev_ptr` and `abbrev_ptr_end` parameters accordingly. If no
// entry is found both params are zeroed/nulled. Failing to find a entry is not
// a error.
static err_code _abbrev_table_entry(const uint32_t target_abbrev_code,
									void **abbrev_ptr,
									uintptr_t *abbrev_ptr_end)
{
	do {
		void *abbrev_start = *abbrev_ptr;

		uint32_t abbrev_code = DECODE_ULEB128(abbrev_ptr, uint32_t);
		if ((uintptr_t)*abbrev_ptr >= *abbrev_ptr_end) {
			return DW_ERROR_FAILED_BOUND_CHECK;
		}

		// The goal of the remaining code here is to find the end of the
		// abbreviation.

		// Read past tag_code
		decode_uleb128((uint8_t **)abbrev_ptr);
		if ((uintptr_t)*abbrev_ptr >= *abbrev_ptr_end) {
			return DW_ERROR_FAILED_BOUND_CHECK;
		}

		// Read past has_children byte.
		*abbrev_ptr += sizeof(uint8_t);
		if ((uintptr_t)*abbrev_ptr >= *abbrev_ptr_end) {
			return DW_ERROR_FAILED_BOUND_CHECK;
		}

		// Find the end of the attributes
		do {
			uint16_t attribute_name_code = DECODE_ULEB128(abbrev_ptr, uint16_t);
			if ((uintptr_t)*abbrev_ptr >= *abbrev_ptr_end) {
				return DW_ERROR_FAILED_BOUND_CHECK;
			}

			uint16_t attribute_form_code = DECODE_ULEB128(abbrev_ptr, uint16_t);
			if ((uintptr_t)*abbrev_ptr >= *abbrev_ptr_end) {
				return DW_ERROR_FAILED_BOUND_CHECK;
			}

			// Pg: 207 Ln: 11
			// The form "DW_FORM_implicit_const" has a constant value stored
			// within the .debug_abbrev data instead of the .debug_info section
			if (attribute_form_code == DW_FORM_implicit_const) {
				decode_sleb128((uint8_t **)abbrev_ptr, sizeof(uint32_t));

				if ((uintptr_t)*abbrev_ptr >= *abbrev_ptr_end) {
					return DW_ERROR_FAILED_BOUND_CHECK;
				}
			}

			if (attribute_name_code == 0 && attribute_form_code == 0) {
				break;
			}

		} while (1);

		if (abbrev_code == target_abbrev_code) {
			*abbrev_ptr_end = (uintptr_t)*abbrev_ptr;
			*abbrev_ptr = abbrev_start;

			return 0;
		}

	} while ((uintptr_t)*abbrev_ptr < *abbrev_ptr_end);

	*abbrev_ptr = NULL;
	*abbrev_ptr_end = 0;

	return 0;
}

// Creates a DWARF context struct for use with dwarf utility function.
DW_Ctx *dwarf_init_context(const Elf64_Ehdr *restrict elf_header)
{
	ELF64_Shdr *debug_info_hdr =
		elf64_section_header_by_name(elf_header, ".debug_info");
	if (debug_info_hdr == NULL) {
		return NULL; /* Failed to find ".debug_info" section */
	}

	ELF64_Shdr *debug_abbrev_hdr =
		elf64_section_header_by_name(elf_header, ".debug_abbrev");
	if (debug_abbrev_hdr == NULL) {
		return NULL; /* Failed to find ".debug_abbrev" section */
	}

	ELF64_Shdr *debug_aranges_hdr =
		elf64_section_header_by_name(elf_header, ".debug_aranges");
	if (debug_aranges_hdr == NULL) {
		return NULL; /* Failed to find ".debug_aranges" section */
	}

	ELF64_Shdr *debug_line_hdr =
		elf64_section_header_by_name(elf_header, ".debug_line");
	if (debug_line_hdr == NULL) {
		return NULL; /* Failed to find ".debug_line" section */
	}

	ELF64_Shdr *debug_line_str_hdr =
		elf64_section_header_by_name(elf_header, ".debug_line_str");
	if (debug_line_str_hdr == NULL) {
		return NULL; /* Failed to find ".debug_line_str" section */
	}

	ELF64_Shdr *debug_str_hdr =
		elf64_section_header_by_name(elf_header, ".debug_str");
	if (debug_str_hdr == NULL) {
		return NULL; /* Failed to find ".debug_str" section */
	}

	struct DWARF_CONTEXT *ctx =
		(struct DWARF_CONTEXT *)kmalloc(sizeof(struct DWARF_CONTEXT));
	if (ctx == NULL) {
		abort("Failed to allocate memory for DWARF context.");
	}

	ctx->debug_info = ELF64_SECTION(elf_header, debug_info_hdr);
	ctx->debug_info_size = debug_info_hdr->sh_size;

	ctx->debug_abbrev = ELF64_SECTION(elf_header, debug_abbrev_hdr);
	ctx->debug_abbrev_size = debug_abbrev_hdr->sh_size;

	ctx->debug_aranges = ELF64_SECTION(elf_header, debug_aranges_hdr);
	ctx->debug_aranges_size = debug_aranges_hdr->sh_size;

	ctx->debug_line = ELF64_SECTION(elf_header, debug_line_hdr);
	ctx->debug_line_size = debug_line_hdr->sh_size;

	ctx->debug_line_str = ELF64_SECTION(elf_header, debug_line_str_hdr);
	ctx->debug_line_str_size = debug_line_str_hdr->sh_size;

	ctx->debug_str = ELF64_SECTION(elf_header, debug_str_hdr);
	ctx->debug_str_size = debug_str_hdr->sh_size;

	return ctx;
}

// Gets the compilation unit header for a given instruction address. Returns an
// error code if an error or unsupported case is encountered. If no compilation
// unit is found the 'compilation_unit_header' ouput remains null.
err_code dwarf_cu_for_address(const DW_Ctx *restrict ctx,
							  const uintptr_t instruction_address,
							  DW_Chdr **compilation_unit_header)
{
	*compilation_unit_header = NULL;

	uintptr_t aranges_section_end =
		(uintptr_t)ctx->debug_aranges + ctx->debug_aranges_size;
	DW_ARhdr *arange = ctx->debug_aranges;

	do {
		if (arange->version != 2)
			return DW_ERROR_UNSUPPORTED_VERSION;

		if (arange->address_size != 8)
			return DW_ERROR_UNSUPPORTED_HEADER;

		if (arange->segment_size != 0)
			return DW_ERROR_UNSUPPORTED_HEADER;

		void *arange_ptr = (void *)arange + sizeof(DW_ARhdr);
		// Why "-8"? The "-8" is because the unit length starts after the length
		// member in the header. Meaning the length starts part way through the
		// header.
		uintptr_t arange_ptr_end = (uintptr_t)arange_ptr + arange->length - 8;

		// The tuple data has a start padding after the header depending on the
		// size of the tuple data members. The data stream might start right
		// after the header but not always depending on alignment needs.

		uint64_t tuple_size = arange->segment_size + arange->address_size * 2;
		uintptr_t remainder =
			((uintptr_t)arange_ptr - (uintptr_t)arange) % tuple_size;

		if (remainder != 0)
			arange_ptr += (2 * arange->address_size) - remainder;

		do {
			// We are assuming the segment selector is 0. Because of this we do
			// not attempt to read a segment selector because it wont exist in
			// the data we are expecting.

			uint64_t address = *(uint64_t *)arange_ptr;
			arange_ptr += sizeof(uint64_t);
			if ((uintptr_t)arange_ptr >= arange_ptr_end)
				return DW_ERROR_INVALID_UNIT;

			uint64_t length = *(uint64_t *)arange_ptr;
			arange_ptr += sizeof(uint64_t);

			if (address == 0 && length == 0)
				break;

			if (instruction_address >= address &&
				instruction_address < address + length) {
				*compilation_unit_header =
					(DW_Chdr *)(ctx->debug_info + arange->debug_info_offset);

				return 0;
			}

		} while ((uintptr_t)arange_ptr < arange_ptr_end);

		arange = (DW_ARhdr *)arange_ptr_end;

	} while ((uintptr_t)arange < aranges_section_end);

	return 0;
}

// Gets the next Debug information entry(DIE)
static err_code _next_die(const DW_Ctx *restrict ctx, const DW_Chdr *cu,
						  void **info_ptr, const uintptr_t info_ptr_end,
						  struct DIE *die_output)
{
	// ".debug_info" section abbreviation code.
	uint32_t target_abbrev_code = DECODE_ULEB128(info_ptr, uint32_t);

	die_output->abbreviation_code = target_abbrev_code;

	if (target_abbrev_code == 0) {
		return 0;
	}

	if (*(uintptr_t *)info_ptr >= info_ptr_end) {
		return DW_ERROR_FAILED_BOUND_CHECK;
	}

	void *abbrev_ptr = ctx->debug_abbrev + cu->debug_abbrev_offset;
	uintptr_t abbrev_ptr_end =
		(uintptr_t)ctx->debug_abbrev + ctx->debug_abbrev_size;

	err_code error =
		_abbrev_table_entry(target_abbrev_code, &abbrev_ptr, &abbrev_ptr_end);
	if (error) {
		return error;
	}

	// `_abbrev_table_entry` does not return an error code for simply not
	// finding an entry so we check.
	if (abbrev_ptr == NULL || abbrev_ptr_end == 0) {
		return DW_ERROR_INVALID_UNIT;
	}

	// ".debug_abbrev" section abbreviation code.
	uint32_t abbrev_code = DECODE_ULEB128(&abbrev_ptr, uint32_t);
	if (*(uintptr_t *)abbrev_ptr >= abbrev_ptr_end) {
		return DW_ERROR_FAILED_BOUND_CHECK;
	}

	if (target_abbrev_code != abbrev_code) {
		return DW_ERROR_INVALID_UNIT;
	}

	uint16_t tag_code = DECODE_ULEB128(&abbrev_ptr, uint16_t);
	if (*(uintptr_t *)abbrev_ptr >= abbrev_ptr_end) {
		return DW_ERROR_FAILED_BOUND_CHECK;
	}

	die_output->tag_code = tag_code;

	uint8_t has_children = *(uint8_t *)abbrev_ptr;
	abbrev_ptr += sizeof(uint8_t);
	if (*(uintptr_t *)abbrev_ptr >= abbrev_ptr_end) {
		return DW_ERROR_FAILED_BOUND_CHECK;
	}

	die_output->has_children = has_children;

	int i = 0;
	do {
		uint16_t attribute_name_code = DECODE_ULEB128(&abbrev_ptr, uint16_t);
		if (*(uintptr_t *)abbrev_ptr >= abbrev_ptr_end) {
			return DW_ERROR_FAILED_BOUND_CHECK;
		}

		uint16_t attribute_form_code = DECODE_ULEB128(&abbrev_ptr, uint16_t);
		if (*(uintptr_t *)abbrev_ptr >= abbrev_ptr_end) {
			return DW_ERROR_FAILED_BOUND_CHECK;
		}

		if (attribute_name_code == 0 && attribute_form_code == 0) {
			break;
		}

		die_output->attributes[i].attribute_name_code = attribute_name_code;
		die_output->attributes[i].attribute_form_code = attribute_form_code;

		if (attribute_form_code == DW_FORM_implicit_const) {
			// Pg: 207 Ln: 11
			// The form "DW_FORM_implicit_const" has a constant value stored
			// within the .debug_abbrev data instead of the .debug_info section
			int32_t value = DECODE_SLEB128(&abbrev_ptr, uint32_t);

			if ((uintptr_t)abbrev_ptr >= abbrev_ptr_end) {
				return DW_ERROR_FAILED_BOUND_CHECK;
			}

			die_output->attributes[i].value = (uint64_t)value;
		} else {
			switch (attribute_form_code) {

			//
			// String class form. Pg: 218 Ln : 15
			//
			case DW_FORM_line_strp:
			case DW_FORM_strp: {
				if (*(uintptr_t *)info_ptr + sizeof(uint32_t) >= info_ptr_end) {
					return DW_ERROR_FAILED_BOUND_CHECK;
				}

				uint32_t str_offset = **(uint32_t **)info_ptr;
				*info_ptr += sizeof(uint32_t);

				die_output->attributes[i].value = (uint64_t)str_offset;
				break;
			}
			case DW_FORM_string: {
				char *str = *(char **)info_ptr;
				size_t str_len = strlen(str) + 1;
				if (*(uintptr_t *)info_ptr + str_len >= info_ptr_end) {
					return DW_ERROR_FAILED_BOUND_CHECK;
				}

				*info_ptr += str_len;

				die_output->attributes[i].value = (uintptr_t)str;
				break;
			}

			//
			// Constant class form. Specification Pg: 214 Ln: 8
			//
			case DW_FORM_data1: {
				if (*(uintptr_t *)info_ptr + sizeof(uint8_t) >= info_ptr_end) {
					return DW_ERROR_FAILED_BOUND_CHECK;
				}
				uint8_t value = **((uint8_t **)info_ptr);
				*info_ptr += sizeof(uint8_t);

				die_output->attributes[i].value = (uint64_t)value;
				break;
			}
			case DW_FORM_data2: {
				if (*(uintptr_t *)info_ptr + sizeof(uint16_t) >= info_ptr_end) {
					return DW_ERROR_FAILED_BOUND_CHECK;
				}
				uint16_t value = **((uint16_t **)info_ptr);
				*info_ptr += sizeof(uint16_t);

				die_output->attributes[i].value = (uint64_t)value;
				break;
			}
			case DW_FORM_data4: {
				if (*(uintptr_t *)info_ptr + sizeof(uint32_t) >= info_ptr_end) {
					return DW_ERROR_FAILED_BOUND_CHECK;
				}
				uint32_t value = **((uint32_t **)info_ptr);
				*info_ptr += sizeof(uint32_t);

				die_output->attributes[i].value = (uint64_t)value;
				break;
			}
			case DW_FORM_data8: {
				if (*(uintptr_t *)info_ptr + sizeof(uint64_t) >= info_ptr_end) {
					return DW_ERROR_FAILED_BOUND_CHECK;
				}
				uint64_t value = **((uint64_t **)info_ptr);
				*info_ptr += sizeof(uint64_t);

				die_output->attributes[i].value = value;
				break;
			}

			//
			// DW_FORM_sec_offset is a multi class form. Specification Pg: 212
			// Ln: 6
			//
			case DW_FORM_sec_offset: {
				if (*(uintptr_t *)info_ptr + sizeof(uint32_t) >= info_ptr_end) {
					return DW_ERROR_FAILED_BOUND_CHECK;
				}
				uint32_t offset = *(uint32_t *)info_ptr;
				*info_ptr += sizeof(uint32_t);

				die_output->attributes[i].value = (uint64_t)offset;
				break;
			}

			//
			// Address class form. Specification Pg: 213 Ln: 3
			//
			case DW_FORM_addr: {
				// Note: uint64_t is an assumption!
				if (*(uintptr_t *)info_ptr + sizeof(uint64_t) >= info_ptr_end) {
					return DW_ERROR_FAILED_BOUND_CHECK;
				}
				uintptr_t address = **(uint64_t **)info_ptr;
				*info_ptr += sizeof(uint64_t);

				die_output->attributes[i].value = address;
				break;
			}

			//
			// Reference class form. Specification Pg: 217 Ln: 1
			//
			case DW_FORM_ref4: {
				if (*(uintptr_t *)info_ptr + sizeof(uint32_t) >= info_ptr_end) {
					return DW_ERROR_FAILED_BOUND_CHECK;
				}
				uint32_t reference = *(uint32_t *)info_ptr;
				*info_ptr += sizeof(uint32_t);

				die_output->attributes[i].value = (uint64_t)reference;
				break;
			}

			//
			// Flag class form. Specification Pg: 215 Ln: 1
			//
			case DW_FORM_flag_present: {
				die_output->attributes[i].value = 1;
				break;
			};

			//
			// exprloc class form. Specification Pg: 214 Ln: 28
			//
			case DW_FORM_exprloc: {
				uint64_t data_size = DECODE_ULEB128(info_ptr, uint64_t);
				if (*(uintptr_t *)info_ptr + data_size >= info_ptr_end) {
					return DW_ERROR_FAILED_BOUND_CHECK;
				}

				*info_ptr += data_size;

				die_output->attributes[i].value = data_size;
				break;
			};

			default:
				printf(KERROR "Tag: %x\n", tag_code);
				printf(KERROR "Attribute: %x\n", attribute_name_code);
				printf(KERROR "Form: %x\n", attribute_form_code);
				abort("Unknown DWARF attribute form!\n");
			}
		}

		die_output->attribute_count = i + 1;
		i++;
	} while (1);

	return 0;
}

// Gets the function name string for the given instruction address. Returns
// an error code if an error or unsupported case is encountered. If no
// function or function name is found then the `symbol_string` ouput remains
// null.
err_code dwarf_cu_query_func(const DW_Ctx *restrict ctx, const DW_Chdr *cu,
							 const uintptr_t instruction_address,
							 char **symbol_string)
{
	*symbol_string = NULL;

	if (cu->version != 5) {
		return DW_ERROR_UNSUPPORTED_VERSION;
	}

	if (cu->unit_type != DW_UT_compile) {
		return DW_ERROR_UNSUPPORTED_HEADER;
	}

	printf("Compilation Unit @ offset %#lx\n",
		   (uintptr_t)cu - (uintptr_t)ctx->debug_info);
	printf(" Length: %#x\n", cu->length);
	printf(" Version: %d\n", cu->version);
	printf(" Unit Type: %d\n", cu->unit_type);
	printf(" Abbrev Offset: %#x\n", cu->debug_abbrev_offset);
	printf(" Pointer Size: %d\n", cu->address_size);

	void *info_ptr = (void *)cu + sizeof(DW_Chdr);
	uintptr_t info_ptr_end = (uintptr_t)cu + cu->length + 4;
	if (info_ptr_end > (uintptr_t)ctx->debug_info + ctx->debug_info_size) {
		return DW_ERROR_FAILED_BOUND_CHECK;
	}

	do {
		struct DIE die = {0};

		err_code error = _next_die(ctx, cu, &info_ptr, info_ptr_end, &die);
		if (error) {
			return error;
		}

		if (die.abbreviation_code == 0 || die.tag_code != DW_TAG_subprogram) {
			continue;
		}

		struct DIE_ATTRIBUTE *low_pc = _die_attribute(&die, DW_AT_low_pc);
		if (low_pc == NULL) { // TODO check expected form?
			continue;
		}

		struct DIE_ATTRIBUTE *high_pc = _die_attribute(&die, DW_AT_high_pc);
		if (high_pc == NULL) {
			continue;
		}

		if (instruction_address >= low_pc->value &&
			instruction_address < low_pc->value + high_pc->value) {
			struct DIE_ATTRIBUTE *name = _die_attribute(&die, DW_AT_name);
			if (name == NULL) {
				continue;
			}
			if (name->attribute_form_code == DW_FORM_strp) {
				*symbol_string = DEBUG_STR(ctx, name->value);

			} else {
				*symbol_string = DEBUG_LINE_STR(ctx, name->value);
			}
			break;
		}

	} while ((uintptr_t)info_ptr < info_ptr_end);

	return 0;
}
