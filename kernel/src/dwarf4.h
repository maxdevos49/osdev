#ifndef __DWARF4_H
#define __DWARF4_H

#include <stdint.h>
#include "type.h"
#include "macro.h"
#include "elf.h"

///
/// DWARF 4: Specification https://dwarfstd.org/doc/DWARF4.pdf
///
/// Introduction to Dwarf: https://dwarfstd.org/doc/Debugging-using-DWARF-2012.pdf
///
/// This file defines structures and types required to implement DWARF 4's 64 bit
/// format.
///

// Header representing data in ".debug_info" section. Specification Pg: 143
struct DWARF4_COMPILATION_HEADER
{
	uint32_t dwarf_64_format;	  /* Always has the value 0xffffffff to alert debuggers this unit is 64-bits */
	uint64_t unit_length;		  /* The actual unit length*/
	uint16_t version;			  /* DWARF version number */
	uint64_t debug_abbrev_offset; /* A offset into the .debug_abbrev section */
	uint8_t address_size;		  /* Size in bytes of an address on the target architecture */
} ATTR_PACK;

//
// Header representing data in ".debug_aranges" section. Specification Pg: 177
//
struct DWARF4_ADDRESS_RANGE_HEADER
{
	uint32_t dwarf_64_format;	/* Always has the value 0xffffffff to alert debuggers this unit is 64-bits */
	uint64_t unit_length;		/* The actual unit length*/
	uint16_t version;			/* DWARF version number */
	uint64_t debug_info_offset; /* Offset into .debug_info table for relevant compilation unit */
	uint8_t address_size;		/* Size in bytes of a address on the target system */
	uint8_t segment_size;		/* Size in bytes of the segment selector on the target system */
} ATTR_PACK;

// Header representing data in .debug_line section // TODO pg num?
struct DWARF4_LINE_HEADER
{
	uint32_t dwarf_64_format;					/* Always has the value 0xffffffff to alert debuggers this unit is 64-bits */
	uint64_t unit_length;						/* The actual unit length*/
	uint16_t version;							/* DWARF version number */
	uint64_t header_length;						/* Length of this header to the first byte of the line program */
	uint8_t minimum_instruction_length;			/* Size in bytes of the smallest target machine instruction. */
	uint8_t maximum_operations_per_instruction; /* Maximum number of individual operations that may be encoded in an instruction. */
	uint8_t default_is_stmt;					// TODO add descriptions
	int8_t line_base;
	uint8_t line_range;
	uint8_t opcode_base;
	uint8_t standard_opcode_lengths[12];
} ATTR_PACK;

enum DW_TAG
{
	// Pg: 151
	DW_TAG_array_type = 0x01,
	DW_TAG_class_type = 0x02,
	DW_TAG_entry_point = 0x03,
	DW_TAG_enumeration_type = 0x04,
	DW_TAG_formal_parameter = 0x05,
	DW_TAG_imported_declaration = 0x08,
	DW_TAG_label = 0x0a,
	DW_TAG_lexical_block = 0x0b,
	DW_TAG_member = 0x0d,
	DW_TAG_pointer_type = 0x0f,
	DW_TAG_reference_type = 0x10,
	DW_TAG_compile_unit = 0x11,
	DW_TAG_string_type = 0x12,
	DW_TAG_structure_type = 0x13,
	DW_TAG_subroutine_type = 0x15,
	DW_TAG_typedef = 0x16,
	// Pg: 152
	DW_TAG_union_type = 0x17,
	DW_TAG_unspecified_parameters = 0x18,
	DW_TAG_variant = 0x19,
	DW_TAG_common_block = 0x1a,
	DW_TAG_common_inclusion = 0x1b,
	DW_TAG_inheritance = 0x1c,
	DW_TAG_inlined_subroutine = 0x1d,
	DW_TAG_module = 0x1e,
	DW_TAG_ptr_to_member_type = 0x1f,
	DW_TAG_set_type = 0x20,
	DW_TAG_subrange_type = 0x21,
	DW_TAG_with_stmt = 0x22,
	DW_TAG_access_declaration = 0x23,
	DW_TAG_base_type = 0x24,
	DW_TAG_catch_block = 0x25,
	DW_TAG_const_type = 0x26,
	DW_TAG_constant = 0x27,
	DW_TAG_enumerator = 0x28,
	DW_TAG_file_type = 0x29,
	DW_TAG_friend = 0x2a,
	// Pg: 153
	DW_TAG_namelist = 0x2b,
	DW_TAG_namelist_item = 0x2c,
	DW_TAG_packed_type = 0x2d,
	DW_TAG_subprogram = 0x2e,
	DW_TAG_template_type_parameter = 0x2f,
	DW_TAG_template_value_parameter = 0x30,
	DW_TAG_thrown_type = 0x31,
	DW_TAG_try_block = 0x32,
	DW_TAG_variant_part = 0x33,
	DW_TAG_variable = 0x34,
	DW_TAG_volatile_type = 0x35,
	DW_TAG_dwarf_procedure = 0x36,
	DW_TAG_restrict_type = 0x37,
	DW_TAG_interface_type = 0x38,
	DW_TAG_namespace = 0x39,
	DW_TAG_imported_module = 0x3a,
	DW_TAG_unspecified_type = 0x3b,
	DW_TAG_partial_unit = 0x3c,
	DW_TAG_imported_unit = 0x3d,
	DW_TAG_condition = 0x3f,
	// Pg: 154
	DW_TAG_shared_type = 0x40,
	DW_TAG_type_unit = 0x41,
	DW_TAG_rvalue_reference_type = 0x42,
	DW_TAG_template_alias = 0x43,
	DW_TAG_lo_user = 0x4080,
	DW_TAG_hi_user = 0xffff
};

enum DW_CHILDREN
{
	// Pg: 154
	DW_CHILDREN_no = 0x00,
	DW_CHILDREN_yes = 0x01
};

enum DW_AT
{
	// Pg: 155
	DW_AT_sibling = 0x01,
	DW_AT_location = 0x02,
	DW_AT_name = 0x03,
	DW_AT_ordering = 0x09,
	DW_AT_byte_size = 0x0b,
	DW_AT_bit_offset = 0x0c,
	DW_AT_bit_size = 0x0d,
	DW_AT_stmt_list = 0x10,
	DW_AT_low_pc = 0x11,
	DW_AT_high_pc = 0x12,
	DW_AT_language = 0x13,
	DW_AT_discr = 0x15,
	DW_AT_discr_value = 0x16,
	DW_AT_visibility = 0x17,
	DW_AT_import = 0x18,
	DW_AT_string_length = 0x19,
	DW_AT_common_reference = 0x1a,
	DW_AT_comp_dir = 0x1b,
	DW_AT_const_value = 0x1c,
	DW_AT_containing_type = 0x1d,
	// Pg: 156
	DW_AT_default_value = 0x1d,
	DW_AT_inline = 0x1e,
	DW_AT_is_optional = 0x20,
	DW_AT_lower_bound = 0x22,
	DW_AT_producer = 0x25,
	DW_AT_prototyped = 0x27,
	DW_AT_return_addr = 0x2a,
	DW_AT_start_scope = 0x2c,
	DW_AT_bit_stride = 0x2e,
	DW_AT_upper_bound = 0x2f,
	DW_AT_abstract_origin = 0x31,
	DW_AT_accessibility = 0x32,
	DW_AT_address_class = 0x33,
	DW_AT_artificial = 0x34,
	DW_AT_base_types = 0x35,
	DW_AT_calling_convention = 0x36,
	DW_AT_count = 0x37,
	DW_AT_data_member_location = 0x38,
	DW_AT_decl_column = 0x39,
	// Pg: 157
	DW_AT_decl_file = 0x3a,
	DW_AT_decl_line = 0x3b,
	DW_AT_declaration = 0x3c,
	DW_AT_discr_list = 0x3d,
	DW_AT_encoding = 0x3e,
	DW_AT_external = 0x3f,
	DW_AT_frame_base = 0x40,
	DW_AT_friend = 0x41,
	DW_AT_identifier_case = 0x42,
	DW_AT_macro_case = 0x43,
	DW_AT_namelist_item = 0x44,
	DW_AT_priority = 0x45,
	DW_AT_segment = 0x46,
	DW_AT_specification = 0x47,
	DW_AT_static_link = 0x48,
	DW_AT_type = 0x49,
	DW_AT_use_parameter = 0x4a,
	DW_AT_variable_parameter = 0x4b,
	DW_AT_virtuality = 0x4c,
	DW_AT_vtable_elem_location = 0x4d,
	// Pg: 158
	DW_AT_allocated = 0x4e,
	DW_AT_associated = 0x4f,
	DW_AT_data_location = 0x50,
	DW_AT_byte_stride = 0x51,
	DW_AT_entry_pc = 0x52,
	DW_AT_use_UTF8 = 0x53,
	DW_AT_extension = 0x54,
	DW_AT_ranges = 0x55,
	DW_AT_trampoline = 0x56,
	DW_AT_call_column = 0x57,
	DW_AT_call_file = 0x58,
	DW_AT_call_line = 0x59,
	DW_AT_description = 0x5a,
	DW_AT_binary_scale = 0x5b,
	DW_AT_decimal_scale = 0x5c,
	DW_AT_small = 0x5d,
	DW_AT_decimal_sign = 0x5e,
	DW_AT_digit_count = 0x5f,
	DW_AT_picture_string = 0x60,
	DW_AT_mutable = 0x61,
	// Pg: 159
	DW_AT_threads_scaled = 0x62,
	DW_AT_explicit = 0x63,
	DW_AT_object_pointer = 0x64,
	DW_AT_endianity = 0x65,
	DW_AT_elemental = 0x66,
	DW_AT_pure = 0x67,
	DW_AT_recursive = 0x68,
	DW_AT_signature = 0x69,
	DW_AT_main_subprogram = 0x6a,
	DW_AT_data_bit_offset = 0x6b,
	DW_AT_const_expr = 0x6c,
	DW_AT_enum_class = 0x6d,
	DW_AT_linkage_name = 0x6e,
	DW_AT_lo_user = 0x2000,
	DW_AT_hi_user = 0x3fff,
};

// Dwarf Information format: Pg 160
enum DW_FORM
{
	DW_FORM_addr = 0x01,
	DW_FORM_block2 = 0x03,
	DW_FORM_block4 = 0x04,
	DW_FORM_data2 = 0x05,
	DW_FORM_data4 = 0x06,
	DW_FORM_data8 = 0x007,
	DW_FORM_string = 0x08,
	DW_FORM_block = 0x09,
	DW_FORM_block1 = 0x0a,
	DW_FORM_data1 = 0x0b, /* Unsigned LEB128 encoded byte */
	DW_FORM_flag = 0x0c,
	DW_FORM_sdata = 0x0d,
	DW_FORM_strp = 0x0e,
	DW_FORM_udata = 0x0f,
	DW_FORM_ref_addr = 0x10,
	DW_FORM_ref1 = 0x11,
	DW_FORM_ref2 = 0x12,
	DW_FORM_ref4 = 0x13,
	DW_FORM_ref8 = 0x14,
	DW_FORM_ref_udata = 0x15,
	DW_FORM_indirect = 0x16,
	DW_FORM_sec_offset = 0x17,
	DW_FORM_exprloc = 0x18,
	DW_FORM_flag_present = 0x19,
	DW_FORM_ref_sig8 = 0x20,
	// TODO more
};

enum DW_OP
{
	// Pg: 163
	DW_OP_addr = 0x03, /* No. of Operands: 1. Notes: Constant address size target specific*/
	DW_OP_deref = 0x06,
	// Pg: 163
	DW_OP_const1u = 0x08, /* No. of Operands: 1. Notes: 1-byte unsigned constant */
	DW_OP_const1s = 0x09, /* No. of Operands: 1. Notes: 1-byte signed constant */
	DW_OP_const2u = 0x0a, /* No. of Operands: 1. Notes: 2-byte unsigned constant */
	DW_OP_const2s = 0x0b, /* No. of Operands: 1. Notes: 2-byte signed constant */
	DW_OP_const4u = 0x0c, /* No. of Operands: 1. Notes: 4-byte unsigned constant */
	DW_OP_const4s = 0x0d, /* No. of Operands: 1. Notes: 4-byte signed constant */
	DW_OP_const8u = 0x0e, /* No. of Operands: 1. Notes: 8-byte unsigned constant */
	DW_OP_const8s = 0x0f, /* No. of Operands: 1. Notes: 8-byte signed constant */
	DW_OP_constu = 0x10,  /* No. of Operands: 1. Notes: ULEB128 constant */
	DW_OP_consts = 0x11,  /* No. of Operands: 1. Notes: SLEB128 constant */
	DW_OP_dup = 0x12,
	DW_OP_drop = 0x13,
	DW_OP_over = 0x14,
	DW_OP_pick = 0x15, /* No. of Operands: 1. 1-byte stack index */
	DW_OP_swap = 0x16,
	DW_OP_rot = 0x17,
	DW_OP_xderef = 0x18,
	DW_OP_abs = 0x19,
	DW_OP_and = 0x1a,
	DW_OP_div = 0x1b,
	// Pg: 165
	DW_OP_minus = 0x1c,
	DW_OP_mod = 0x1d,
	DW_OP_mul = 0x1e,
	DW_OP_neg = 0x1f,
	DW_OP_not = 0x20,
	DW_OP_or = 0x21,
	DW_OP_plus = 0x22,
	DW_OP_plus_uconst = 0x23, /* No. of Operands: 1. Notes: ULEB128 addend */
	DW_OP_shl = 0x24,
	DW_OP_shr = 0x25,
	DW_OP_shra = 0x26,
	DW_OP_xor = 0x27,
	DW_OP_skip = 0x2f, /* No. of Operands: 1. Notes: Signed 2-byte constant */
	DW_OP_bra = 0x28,  /* No. of Operands: 1. Notes: Signed 2-byte constant */
	DW_OP_eq = 0x29,
	DW_OP_ge = 0x2a,
	DW_OP_gt = 0x2b,
	DW_OP_le = 0x2c,
	DW_OP_lt = 0x2d,
	DW_OP_ne = 0x2e,
	// Pg: 166
	DW_OP_lit0 = 0x30,
	DW_OP_lit1 = 0x31,
	DW_OP_lit2 = 0x32,
	DW_OP_lit3 = 0x33,
	DW_OP_lit4 = 0x34,
	DW_OP_lit5 = 0x35,
	DW_OP_lit6 = 0x36,
	DW_OP_lit7 = 0x37,
	DW_OP_lit8 = 0x38,
	DW_OP_lit9 = 0x39,
	DW_OP_lit10 = 0x3A,
	DW_OP_lit11 = 0x3B,
	DW_OP_lit12 = 0x3C,
	DW_OP_lit13 = 0x3D,
	DW_OP_lit14 = 0x3E,
	DW_OP_lit15 = 0x3F,
	DW_OP_lit16 = 0x40,
	DW_OP_lit17 = 0x41,
	DW_OP_lit18 = 0x42,
	DW_OP_lit19 = 0x43,
	DW_OP_lit20 = 0x44,
	DW_OP_lit21 = 0x45,
	DW_OP_lit22 = 0x46,
	DW_OP_lit23 = 0x47,
	DW_OP_lit24 = 0x48,
	DW_OP_lit25 = 0x49,
	DW_OP_lit26 = 0x4A,
	DW_OP_lit27 = 0x4B,
	DW_OP_lit28 = 0x4C,
	DW_OP_lit29 = 0x4D,
	DW_OP_lit30 = 0x4E,
	DW_OP_lit31 = 0x4F,

	DW_OP_reg0 = 0x50,
	DW_OP_reg1 = 0x51,
	DW_OP_reg2 = 0x52,
	DW_OP_reg3 = 0x53,
	DW_OP_reg4 = 0x54,
	DW_OP_reg5 = 0x55,
	DW_OP_reg6 = 0x56,
	DW_OP_reg7 = 0x57,
	DW_OP_reg8 = 0x58,
	DW_OP_reg9 = 0x59,
	DW_OP_reg10 = 0x5A,
	DW_OP_reg11 = 0x5B,
	DW_OP_reg12 = 0x5C,
	DW_OP_reg13 = 0x5D,
	DW_OP_reg14 = 0x5E,
	DW_OP_reg15 = 0x5F,
	DW_OP_reg16 = 0x60,
	DW_OP_reg17 = 0x61,
	DW_OP_reg18 = 0x62,
	DW_OP_reg19 = 0x63,
	DW_OP_reg20 = 0x64,
	DW_OP_reg21 = 0x65,
	DW_OP_reg22 = 0x66,
	DW_OP_reg23 = 0x67,
	DW_OP_reg24 = 0x68,
	DW_OP_reg25 = 0x69,
	DW_OP_reg26 = 0x6A,
	DW_OP_reg27 = 0x6B,
	DW_OP_reg28 = 0x6C,
	DW_OP_reg29 = 0x6D,
	DW_OP_reg30 = 0x6E,
	DW_OP_reg31 = 0x6F,

	DW_OP_breg0 = 0x70,		  /* No. of Operands: 1. SLEB128 offset */
	DW_OP_breg1 = 0x71,		  /* No. of Operands: 1. SLEB128 offset */
	DW_OP_breg2 = 0x72,		  /* No. of Operands: 1. SLEB128 offset */
	DW_OP_breg3 = 0x73,		  /* No. of Operands: 1. SLEB128 offset */
	DW_OP_breg4 = 0x74,		  /* No. of Operands: 1. SLEB128 offset */
	DW_OP_breg5 = 0x75,		  /* No. of Operands: 1. SLEB128 offset */
	DW_OP_breg6 = 0x76,		  /* No. of Operands: 1. SLEB128 offset */
	DW_OP_breg7 = 0x77,		  /* No. of Operands: 1. SLEB128 offset */
	DW_OP_breg8 = 0x78,		  /* No. of Operands: 1. SLEB128 offset */
	DW_OP_breg9 = 0x79,		  /* No. of Operands: 1. SLEB128 offset */
	DW_OP_breg10 = 0x7A,	  /* No. of Operands: 1. SLEB128 offset */
	DW_OP_breg11 = 0x7B,	  /* No. of Operands: 1. SLEB128 offset */
	DW_OP_breg12 = 0x7C,	  /* No. of Operands: 1. SLEB128 offset */
	DW_OP_breg13 = 0x7D,	  /* No. of Operands: 1. SLEB128 offset */
	DW_OP_breg14 = 0x7E,	  /* No. of Operands: 1. SLEB128 offset */
	DW_OP_breg15 = 0x7F,	  /* No. of Operands: 1. SLEB128 offset */
	DW_OP_breg16 = 0x80,	  /* No. of Operands: 1. SLEB128 offset */
	DW_OP_breg17 = 0x81,	  /* No. of Operands: 1. SLEB128 offset */
	DW_OP_breg18 = 0x82,	  /* No. of Operands: 1. SLEB128 offset */
	DW_OP_breg19 = 0x83,	  /* No. of Operands: 1. SLEB128 offset */
	DW_OP_breg20 = 0x84,	  /* No. of Operands: 1. SLEB128 offset */
	DW_OP_breg21 = 0x85,	  /* No. of Operands: 1. SLEB128 offset */
	DW_OP_breg22 = 0x86,	  /* No. of Operands: 1. SLEB128 offset */
	DW_OP_breg23 = 0x87,	  /* No. of Operands: 1. SLEB128 offset */
	DW_OP_breg24 = 0x88,	  /* No. of Operands: 1. SLEB128 offset */
	DW_OP_breg25 = 0x89,	  /* No. of Operands: 1. SLEB128 offset */
	DW_OP_breg26 = 0x8A,	  /* No. of Operands: 1. SLEB128 offset */
	DW_OP_breg27 = 0x8B,	  /* No. of Operands: 1. SLEB128 offset */
	DW_OP_breg28 = 0x8C,	  /* No. of Operands: 1. SLEB128 offset */
	DW_OP_breg29 = 0x8D,	  /* No. of Operands: 1. SLEB128 offset */
	DW_OP_breg30 = 0x8E,	  /* No. of Operands: 1. SLEB128 offset */
	DW_OP_breg31 = 0x8F,	  /* No. of Operands: 1. SLEB128 offset */
	DW_OP_regx = 0x90,		  /* No. of Operands: 1. ULEB128 register */
	DW_OP_fbreg = 0x91,		  /* No. of Operands: 1. SLEB128 offset */
	DW_OP_bregx = 0x92,		  /* No. of Operands: 2. ULEB128 register followed by SLEB128 offset */
	DW_OP_piece = 0x93,		  /* No. of Operands: 1. ULEB128 size of piece addressed */
	DW_OP_deref_size = 0x94,  /* No. of Operands: 1. 1-byte size of data retrieved */
	DW_OP_xderef_size = 0x95, /* No. of Operands: 1. 1 byte size of data retrieved */
	DW_OP_nop = 0x96,
	// Pg: 167
	DW_OP_push_object_address = 0x97,
	DW_OP_call2 = 0x98,	   /* No. of Operands: 1. 2-byte offset of DIE */
	DW_OP_call4 = 0x99,	   /* No. of Operands: 1. 4-byte offset of DIE */
	DW_OP_call_ref = 0x9a, /* No. of Operands: 1. 4- or 8-byte offset of DIE */
	DW_OP_form_tls_address = 0x9b,
	DW_OP_call_frame_cfa = 0x9c,
	DW_OP_bit_piece = 0x9d,		 /* No. of Operands: 2. ULEB128 size followed by ULEB128 offset */
	DW_OP_implicit_value = 0x9e, /* No. of Operands: 2. ULEB128 size followed by block of that size */
	DW_OP_stack_value = 0x9f,
	DW_OP_lo_user = 0xe0,
	DW_OP_hi_user = 0xff,
};

struct DWARF_CONTEXT;

struct DWARF_CONTEXT *dwarf4_init_context(const Elf64_Ehdr *restrict elf_header);

void dwarf4_print_compilation(struct DWARF_CONTEXT *ctx, uint64_t offset);
// void dwarf4_print_address_range(struct DWARF4_ADDRESS_RANGE_HEADER *header);
// void dwarf4_print_line(struct DWARF4_LINE_HEADER *header);
void dwarf4_compilation_unit_for_address(struct DWARF_CONTEXT *ctx, uintptr_t address);

uint64_t decode_uleb128(uint8_t **stream);
int64_t decode_sleb128(uint8_t **stream, size_t bit_size);

#endif
