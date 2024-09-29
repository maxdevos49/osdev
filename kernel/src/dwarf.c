#include "dwarf.h"
#include "debug.h"
#include "elf.h"
#include "error.h"
#include "memory/heap.h"
#include "stdbool.h"
#include "stream.h"
#include "string/utility.h"

#define DEBUG_STR(offset) ((char *)(_ctx.debug_str + offset))
#define DEBUG_LINE_STR(offset) ((char *)(_ctx.debug_line_str + offset))

struct DWARF_CONTEXT {
	bool loaded;

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
	uint16_t name_code;
	uint16_t form_code;
	// Value should be interpreted based on the form_code value. This might be a
	// pointer or a simple integer value. Not all values can be represented by
	// this... dwarf expressions. We dont need that yet though.
	uint64_t value;
};

struct DIE {
	uint16_t abbreviation_code;
	uint16_t tag_code;
	uint8_t has_children;
	uint8_t attribute_count;
	struct DIE_ATTRIBUTE attributes[15];
};

struct DWARF_CONTEXT _ctx = {0};

static inline struct DIE_ATTRIBUTE *_die_attribute(struct DIE *die,
												   enum DW_AT attribute_code)
{
	for (int i = 0; i < die->attribute_count; i++) {
		if (die->attributes[i].name_code == attribute_code) {
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
	err_code err = 0;

	do {
		void *abbrev_start = *abbrev_ptr;

		uint32_t abbrev_code;
		if ((err = leb128_to_u32(abbrev_ptr, *abbrev_ptr_end, &abbrev_code))) {
			debug_code(err);
			return err;
		}

		// The goal of the remaining code here is to find the end of the
		// abbreviation.

		// Read past the tag code
		uint64_t tag_code = 0;
		if ((err = leb128_to_u64(abbrev_ptr, *abbrev_ptr_end, &tag_code))) {
			debug_code(err);
			return err;
		}

		// Read past has_children byte.
		uint8_t has_children = 0;
		if ((err = read_u8(abbrev_ptr, *abbrev_ptr_end, &has_children))) {
			debug_code(err);
			return err;
		}

		// Find the end of the attributes
		do {
			uint16_t attribute = 0;
			if ((err =
					 leb128_to_u16(abbrev_ptr, *abbrev_ptr_end, &attribute))) {
				debug_code(err);
				return err;
			}

			uint16_t form = 0;
			if ((err = leb128_to_u16(abbrev_ptr, *abbrev_ptr_end, &form))) {
				debug_code(err);
				return err;
			}

			// Pg: 207 Ln: 11
			// The form "DW_FORM_implicit_const" has a constant value stored
			// within the .debug_abbrev data instead of the .debug_info section
			if (form == DW_FORM_implicit_const) {

				int64_t temp = 0;
				if ((err = leb128_to_s64(abbrev_ptr, *abbrev_ptr_end, &temp))) {
					debug_code(err);
					return err;
				}
			}

			if (attribute == 0 && form == 0) {
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

err_code dwarf_load_sections(const Elf64_Ehdr *restrict elf_header)
{
	if (_ctx.loaded) {
		return 0;
	}

	if (elf_header == NULL) {
		debug_code(ERROR_UNEXPECTED_NULL_POINTER);
		return ERROR_UNEXPECTED_NULL_POINTER;
	}

	ELF64_Shdr *debug_info_hdr =
		elf64_section_header_by_name(elf_header, ".debug_info");
	if (debug_info_hdr == NULL) {
		debug_code(ERROR_UNEXPECTED_NULL_POINTER);
		return ERROR_UNEXPECTED_NULL_POINTER;
	}

	ELF64_Shdr *debug_abbrev_hdr =
		elf64_section_header_by_name(elf_header, ".debug_abbrev");
	if (debug_abbrev_hdr == NULL) {
		debug_code(ERROR_UNEXPECTED_NULL_POINTER);
		return ERROR_UNEXPECTED_NULL_POINTER;
	}

	ELF64_Shdr *debug_aranges_hdr =
		elf64_section_header_by_name(elf_header, ".debug_aranges");
	if (debug_aranges_hdr == NULL) {
		debug_code(ERROR_UNEXPECTED_NULL_POINTER);
		return ERROR_UNEXPECTED_NULL_POINTER;
	}

	ELF64_Shdr *debug_line_hdr =
		elf64_section_header_by_name(elf_header, ".debug_line");
	if (debug_line_hdr == NULL) {
		debug_code(ERROR_UNEXPECTED_NULL_POINTER);
		return ERROR_UNEXPECTED_NULL_POINTER;
	}

	ELF64_Shdr *debug_line_str_hdr =
		elf64_section_header_by_name(elf_header, ".debug_line_str");
	if (debug_line_str_hdr == NULL) {
		debug_code(ERROR_UNEXPECTED_NULL_POINTER);
		return ERROR_UNEXPECTED_NULL_POINTER;
	}

	ELF64_Shdr *debug_str_hdr =
		elf64_section_header_by_name(elf_header, ".debug_str");
	if (debug_str_hdr == NULL) {
		debug_code(ERROR_UNEXPECTED_NULL_POINTER);
		return ERROR_UNEXPECTED_NULL_POINTER;
	}

	_ctx.debug_info = ELF64_SECTION(elf_header, debug_info_hdr);
	_ctx.debug_info_size = debug_info_hdr->sh_size;

	_ctx.debug_abbrev = ELF64_SECTION(elf_header, debug_abbrev_hdr);
	_ctx.debug_abbrev_size = debug_abbrev_hdr->sh_size;

	_ctx.debug_aranges = ELF64_SECTION(elf_header, debug_aranges_hdr);
	_ctx.debug_aranges_size = debug_aranges_hdr->sh_size;

	_ctx.debug_line = ELF64_SECTION(elf_header, debug_line_hdr);
	_ctx.debug_line_size = debug_line_hdr->sh_size;

	_ctx.debug_line_str = ELF64_SECTION(elf_header, debug_line_str_hdr);
	_ctx.debug_line_str_size = debug_line_str_hdr->sh_size;

	_ctx.debug_str = ELF64_SECTION(elf_header, debug_str_hdr);
	_ctx.debug_str_size = debug_str_hdr->sh_size;

	_ctx.loaded = true;

	return 0;
}

// Gets the compilation unit header for a given instruction address. Returns an
// error code if an error or unsupported case is encountered. If no compilation
// unit is found the 'compilation_unit_header' ouput remains null.
err_code dwarf_cu_for_address(const uintptr_t instruction_address,
							  DW_Chdr **compilation_unit_header)
{
	*compilation_unit_header = NULL;

	if (_ctx.loaded == false) {
		debug_code(ERROR_DEPENDENCY_NOT_LOADED);
		return ERROR_DEPENDENCY_NOT_LOADED;
	}

	uintptr_t aranges_section_end =
		(uintptr_t)_ctx.debug_aranges + _ctx.debug_aranges_size;
	DW_ARhdr *arange = _ctx.debug_aranges;

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
					(DW_Chdr *)(_ctx.debug_info + arange->debug_info_offset);

				return 0;
			}

		} while ((uintptr_t)arange_ptr < arange_ptr_end);

		arange = (DW_ARhdr *)arange_ptr_end;

	} while ((uintptr_t)arange < aranges_section_end);

	return 0;
}

// Gets the next Debug information entry(DIE)
static err_code _next_die(const DW_Chdr *cu, void **info_ptr,
						  const uintptr_t info_ptr_end, struct DIE *die_output)
{
	err_code err = 0;

	// ".debug_info" section abbreviation code.
	uint32_t target_abbrev_code = 0;
	if ((err = leb128_to_u32(info_ptr, info_ptr_end, &target_abbrev_code))) {
		debug_code(err);
		return err;
	}

	die_output->abbreviation_code = target_abbrev_code;

	if (target_abbrev_code == 0)
		return 0;

	void *abbrev_ptr = _ctx.debug_abbrev + cu->debug_abbrev_offset;
	uintptr_t abbrev_ptr_end =
		(uintptr_t)_ctx.debug_abbrev + _ctx.debug_abbrev_size;

	if ((err = _abbrev_table_entry(target_abbrev_code, &abbrev_ptr,
								   &abbrev_ptr_end))) {
		debug_code(err);
		return err;
	}

	// `_abbrev_table_entry` does not return an error code for simply not
	// finding an entry so we need to check the pointer and size.
	if (abbrev_ptr == NULL || abbrev_ptr_end == 0)
		return DW_ERROR_INVALID_UNIT;

	// ".debug_abbrev" section abbreviation code.
	uint32_t abbrev_code = 0;
	if ((err = leb128_to_u32(&abbrev_ptr, abbrev_ptr_end, &abbrev_code))) {
		debug_code(err);
		return err;
	}

	if (target_abbrev_code != abbrev_code)
		return DW_ERROR_INVALID_UNIT;

	uint16_t tag_code = 0;
	if ((err = leb128_to_u16(&abbrev_ptr, abbrev_ptr_end, &tag_code))) {
		debug_code(err);
		return err;
	}

	die_output->tag_code = tag_code;

	uint8_t has_children = 0;
	if ((err = read_u8(&abbrev_ptr, abbrev_ptr_end, &has_children))) {
		debug_code(err);
		return err;
	}

	die_output->has_children = has_children;

	int i = 0;
	do {
		uint16_t attribute = 0;
		if ((err = leb128_to_u16(&abbrev_ptr, abbrev_ptr_end, &attribute))) {
			debug_code(err);
			return err;
		}

		uint16_t form = 0;
		if ((err = leb128_to_u16(&abbrev_ptr, abbrev_ptr_end, &form))) {
			debug_code(err);
			return err;
		}

		if (attribute == 0 && form == 0) {
			break;
		}

		die_output->attributes[i].name_code = attribute;
		die_output->attributes[i].form_code = form;

		if (form == DW_FORM_implicit_const) {
			// Pg: 207 Ln: 11
			// The form "DW_FORM_implicit_const" has a constant value stored
			// within the .debug_abbrev data instead of the .debug_info section
			int64_t constant = 0;
			if ((err = leb128_to_s64(&abbrev_ptr, abbrev_ptr_end, &constant))) {
				debug_code(err);
				return err;
			}

			die_output->attributes[i].value = (uint64_t)constant;
		} else {
			switch (form) {

			//
			// String class form. Pg: 218 Ln : 15
			//
			case DW_FORM_line_strp:
			case DW_FORM_strp: {
				uint32_t str_offset = 0;
				if ((err = read_u32(info_ptr, info_ptr_end, &str_offset))) {
					debug_code(err);
					return err;
				}

				die_output->attributes[i].value = (uint64_t)str_offset;
				break;
			}
			case DW_FORM_string: {
				char *str = *(char **)info_ptr;

				size_t str_len = strlen(str) + 1;
				if (*(uintptr_t *)info_ptr + str_len > info_ptr_end) {
					debug_code(ERROR_OUT_OF_BOUNDS);
					return ERROR_OUT_OF_BOUNDS;
				}

				die_output->attributes[i].value = (uintptr_t)str;

				*info_ptr += str_len;

				break;
			}

			//
			// Constant class form. Specification Pg: 214 Ln: 8
			//
			case DW_FORM_data1: {
				uint8_t data1 = 0;
				if ((err = read_u8(info_ptr, info_ptr_end, &data1))) {
					debug_code(err);
					return err;
				}

				die_output->attributes[i].value = (uint64_t)data1;
				break;
			}
			case DW_FORM_data2: {
				uint16_t data2 = 0;
				if ((err = read_u16(info_ptr, info_ptr_end, &data2))) {
					debug_code(err);
					return err;
				}

				die_output->attributes[i].value = (uint64_t)data2;
				break;
			}
			case DW_FORM_data4: {
				uint32_t data4 = 0;
				if ((err = read_u32(info_ptr, info_ptr_end, &data4))) {
					debug_code(err);
					return err;
				}

				die_output->attributes[i].value = (uint64_t)data4;
				break;
			}
			case DW_FORM_data8: {
				uint64_t data8 = 0;
				if ((err = read_u64(info_ptr, info_ptr_end, &data8))) {
					debug_code(err);
					return err;
				}

				die_output->attributes[i].value = data8;
				break;
			}

			//
			// DW_FORM_sec_offset is a multi class form. Specification Pg: 212
			// Ln: 6
			//
			case DW_FORM_sec_offset: {
				uint32_t offset = 0;
				if ((err = read_u32(info_ptr, info_ptr_end, &offset))) {
					debug_code(err);
					return err;
				}

				die_output->attributes[i].value = (uint64_t)offset;
				break;
			}

			//
			// Address class form. Specification Pg: 213 Ln: 3
			//
			case DW_FORM_addr: {
				// Note: uint64_t is an assumption!
				uint64_t address = 0;
				if ((err = read_u64(info_ptr, info_ptr_end, &address))) {
					debug_code(err);
					return err;
				}

				die_output->attributes[i].value = address;
				break;
			}

			//
			// Reference class form. Specification Pg: 217 Ln: 1
			//
			case DW_FORM_ref4: {
				uint32_t ref4 = 0;
				if ((err = read_u32(info_ptr, info_ptr_end, &ref4))) {
					debug_code(err);
					return err;
				}

				die_output->attributes[i].value = (uint64_t)ref4;
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
				uint64_t data_size = 0;
				if ((err = leb128_to_u64(info_ptr, info_ptr_end, &data_size))) {
					debug_code(err);
					return err;
				}

				// "Consume the data"
				*info_ptr += data_size;

				die_output->attributes[i].value = data_size;
				break;
			};

			default:
				printf(KERROR "Tag: %x\n", tag_code);
				printf(KERROR "Attribute: %x\n", attribute);
				printf(KERROR "Form: %x\n", form);
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
err_code dwarf_query_func(const uintptr_t instruction_address,
						  char **symbol_string)
{
	err_code error = 0;

	*symbol_string = NULL;

	if (_ctx.loaded == false) {
		debug_code(ERROR_DEPENDENCY_NOT_LOADED);
		return ERROR_DEPENDENCY_NOT_LOADED;
	}

	DW_Chdr *cu = NULL;
	if ((error = dwarf_cu_for_address((uintptr_t)instruction_address, &cu))) {
		debug_code(error);
		return error;
	}

	if (cu == NULL) {
		// Address was not inside a function. This is not considered an error.
		return 0;
	}

	if (cu->version != 5) {
		debug_code(DW_ERROR_UNSUPPORTED_VERSION);
		return DW_ERROR_UNSUPPORTED_VERSION;
	}

	if (cu->unit_type != DW_UT_compile) {
		debug_code(DW_ERROR_UNSUPPORTED_HEADER);
		return DW_ERROR_UNSUPPORTED_HEADER;
	}

	void *info_ptr = (void *)cu + sizeof(DW_Chdr);
	uintptr_t info_ptr_end = (uintptr_t)cu + cu->length + 4;
	if (info_ptr_end > (uintptr_t)_ctx.debug_info + _ctx.debug_info_size) {
		debug_code(ERROR_OUT_OF_BOUNDS);
		return ERROR_OUT_OF_BOUNDS;
	}

	do {
		struct DIE die = {0};

		error = _next_die(cu, &info_ptr, info_ptr_end, &die);
		if (error) {
			return error;
		}

		if (die.abbreviation_code == 0 || die.tag_code != DW_TAG_subprogram) {
			continue;
		}

		struct DIE_ATTRIBUTE *low_pc = _die_attribute(&die, DW_AT_low_pc);
		if (low_pc == NULL) {
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
			if (name->form_code == DW_FORM_strp) {
				*symbol_string = DEBUG_STR(name->value);
			} else {
				*symbol_string = DEBUG_LINE_STR(name->value);
			}
			break;
		}

	} while ((uintptr_t)info_ptr < info_ptr_end);

	return 0;
}
