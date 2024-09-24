#include "string/utility.h"
#include "dwarf4.h"
#include "elf.h"
#include "error.h"
#include "memory/heap.h"

struct DWARF_CONTEXT
{
	void *debug_info;
	size_t debug_info_size;
	void *debug_abbrev;
	size_t debug_abbrev_size;
	void *debug_aranges;
	size_t debug_aranges_size;
	void *debug_ranges;
	size_t debug_ranges_size;
	void *debug_line;
	size_t debug_line_size;
	void *debug_str;
	size_t debug_str_size;
};

#define DECODE_ULEB128(stream, intType) ((intType)decode_uleb128((uint8_t **)stream))
#define DECODE_SLEB128(stream, intType) ((intType)decode_sleb128((uint8_t **)stream, sizeof(intType) * 8))

#define DEBUG_STR(ctx, offset) ((char *)(ctx->debug_str + offset))

void print_hex_table(void *address, size_t length)
{
	uint8_t *byte = (uint8_t *)address;

	printf("                 |  0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f | Decoded text\n");

	for (size_t i = 0; i < length; i++)
	{
		if (i % 16 == 0)
		{
			if (i != 0)
			{
				printf("| ");

				for (size_t ii = (i - 16); ii < i; ii++)
				{
					char c = *(byte + ii);
					if (c > 31 && c < 127)
					{
						printf("%c", c);
					}
					else
					{
						printf(".");
					}
				}

				printf("\n");
			}

			printf("%016lx | ", (uintptr_t)byte + i);
		}

		printf("%02x ", *(byte + i));
	}

	printf("\n");
}

static const char *tag_code_to_string(uint16_t tag)
{
	switch (tag)
	{
	// Pg: 151
	case DW_TAG_array_type:
		return STRINGIFY(DW_TAG_array_type);
	case DW_TAG_class_type:
		return STRINGIFY(DW_TAG_class_type);
	case DW_TAG_entry_point:
		return STRINGIFY(DW_TAG_entry_point);
	case DW_TAG_enumeration_type:
		return STRINGIFY(DW_TAG_enumeration_type);
	case DW_TAG_formal_parameter:
		return STRINGIFY(DW_TAG_formal_parameter);
	case DW_TAG_imported_declaration:
		return STRINGIFY(DW_TAG_imported_declaration);
	case DW_TAG_label:
		return STRINGIFY(DW_TAG_label);
	case DW_TAG_lexical_block:
		return STRINGIFY(DW_TAG_lexical_block);
	case DW_TAG_member:
		return STRINGIFY(DW_TAG_member);
	case DW_TAG_pointer_type:
		return STRINGIFY(DW_TAG_pointer_type);
	case DW_TAG_reference_type:
		return STRINGIFY(DW_TAG_reference_type);
	case DW_TAG_compile_unit:
		return STRINGIFY(DW_TAG_compile_unit);
	case DW_TAG_string_type:
		return STRINGIFY(DW_TAG_string_type);
	case DW_TAG_structure_type:
		return STRINGIFY(DW_TAG_structure_type);
	case DW_TAG_subroutine_type:
		return STRINGIFY(DW_TAG_subroutine_type);
	case DW_TAG_typedef:
		return STRINGIFY(DW_TAG_typedef);
	// Pg: 152
	case DW_TAG_union_type:
		return STRINGIFY(DW_TAG_union_type);
	case DW_TAG_unspecified_parameters:
		return STRINGIFY(DW_TAG_unspecified_parameters);
	case DW_TAG_variant:
		return STRINGIFY(DW_TAG_variant);
	case DW_TAG_common_block:
		return STRINGIFY(DW_TAG_common_block);
	case DW_TAG_common_inclusion:
		return STRINGIFY(DW_TAG_common_inclusion);
	case DW_TAG_inheritance:
		return STRINGIFY(DW_TAG_inheritance);
	case DW_TAG_inlined_subroutine:
		return STRINGIFY(DW_TAG_inlined_subroutine);
	case DW_TAG_module:
		return STRINGIFY(DW_TAG_module);
	case DW_TAG_ptr_to_member_type:
		return STRINGIFY(DW_TAG_ptr_to_member_type);
	case DW_TAG_set_type:
		return STRINGIFY(DW_TAG_set_type);
	case DW_TAG_subrange_type:
		return STRINGIFY(DW_TAG_subrange_type);
	case DW_TAG_with_stmt:
		return STRINGIFY(DW_TAG_with_stmt);
	case DW_TAG_access_declaration:
		return STRINGIFY(DW_TAG_access_declaration);
	case DW_TAG_base_type:
		return STRINGIFY(DW_TAG_base_type);
	case DW_TAG_catch_block:
		return STRINGIFY(DW_TAG_catch_block);
	case DW_TAG_const_type:
		return STRINGIFY(DW_TAG_const_type);
	case DW_TAG_constant:
		return STRINGIFY(DW_TAG_constant);
	case DW_TAG_enumerator:
		return STRINGIFY(DW_TAG_enumerator);
	case DW_TAG_file_type:
		return STRINGIFY(DW_TAG_file_type);
	case DW_TAG_friend:
		return STRINGIFY(DW_TAG_friend);
	// Pg: 153
	case DW_TAG_namelist:
		return STRINGIFY(DW_TAG_namelist);
	case DW_TAG_namelist_item:
		return STRINGIFY(DW_TAG_namelist_item);
	case DW_TAG_packed_type:
		return STRINGIFY(DW_TAG_packed_type);
	case DW_TAG_subprogram:
		return STRINGIFY(DW_TAG_subprogram);
	case DW_TAG_template_type_parameter:
		return STRINGIFY(DW_TAG_template_type_parameter);
	case DW_TAG_template_value_parameter:
		return STRINGIFY(DW_TAG_template_value_parameter);
	case DW_TAG_thrown_type:
		return STRINGIFY(DW_TAG_thrown_type);
	case DW_TAG_try_block:
		return STRINGIFY(DW_TAG_try_block);
	case DW_TAG_variant_part:
		return STRINGIFY(DW_TAG_variant_part);
	case DW_TAG_variable:
		return STRINGIFY(DW_TAG_variable);
	case DW_TAG_volatile_type:
		return STRINGIFY(DW_TAG_volatile_type);
	case DW_TAG_dwarf_procedure:
		return STRINGIFY(DW_TAG_dwarf_procedure);
	case DW_TAG_restrict_type:
		return STRINGIFY(DW_TAG_restrict_type);
	case DW_TAG_interface_type:
		return STRINGIFY(DW_TAG_interface_type);
	case DW_TAG_namespace:
		return STRINGIFY(DW_TAG_namespace);
	case DW_TAG_imported_module:
		return STRINGIFY(DW_TAG_imported_module);
	case DW_TAG_unspecified_type:
		return STRINGIFY(DW_TAG_unspecified_type);
	case DW_TAG_partial_unit:
		return STRINGIFY(DW_TAG_partial_unit);
	case DW_TAG_imported_unit:
		return STRINGIFY(DW_TAG_imported_unit);
	case DW_TAG_condition:
		return STRINGIFY(DW_TAG_condition);
	// Pg: 154
	case DW_TAG_shared_type:
		return STRINGIFY(DW_TAG_shared_type);
	case DW_TAG_type_unit:
		return STRINGIFY(DW_TAG_type_unit);
	case DW_TAG_rvalue_reference_type:
		return STRINGIFY(DW_TAG_rvalue_reference_type);
	case DW_TAG_template_alias:
		return STRINGIFY(DW_TAG_template_alias);
	case DW_TAG_lo_user:
		return STRINGIFY(DW_TAG_lo_user);
	case DW_TAG_hi_user:
		return STRINGIFY(DW_TAG_hi_user);
	}

	printf(KERROR "Encountered unknown code: %x\n", tag);
	abort("Unknown tag code!");
}

static const char *attribute_code_to_string(uint16_t attribute)
{
	switch (attribute)
	{
		// Pg: 155
	case DW_AT_sibling:
		return STRINGIFY(DW_AT_sibling);
	case DW_AT_location:
		return STRINGIFY(DW_AT_location);
	case DW_AT_name:
		return STRINGIFY(DW_AT_name);
	case DW_AT_ordering:
		return STRINGIFY(DW_AT_ordering);
	case DW_AT_byte_size:
		return STRINGIFY(DW_AT_byte_size);
	case DW_AT_bit_offset:
		return STRINGIFY(DW_AT_bit_offset);
	case DW_AT_bit_size:
		return STRINGIFY(DW_AT_bit_size);
	case DW_AT_stmt_list:
		return STRINGIFY(DW_AT_stmt_list);
	case DW_AT_low_pc:
		return STRINGIFY(DW_AT_low_pc);
	case DW_AT_high_pc:
		return STRINGIFY(DW_AT_high_pc);
	case DW_AT_language:
		return STRINGIFY(DW_AT_language);
	case DW_AT_discr:
		return STRINGIFY(DW_AT_discr);
	case DW_AT_discr_value:
		return STRINGIFY(DW_AT_discr_value);
	case DW_AT_visibility:
		return STRINGIFY(DW_AT_visibility);
	case DW_AT_import:
		return STRINGIFY(DW_AT_import);
	case DW_AT_string_length:
		return STRINGIFY(DW_AT_string_length);
	case DW_AT_common_reference:
		return STRINGIFY(DW_AT_common_reference);
	case DW_AT_comp_dir:
		return STRINGIFY(DW_AT_comp_dir);
	case DW_AT_const_value:
		return STRINGIFY(DW_AT_const_value);
	// Pg: 156
	case DW_AT_default_value:
		return STRINGIFY(DW_AT_default_value);
	case DW_AT_inline:
		return STRINGIFY(DW_AT_inline);
	case DW_AT_is_optional:
		return STRINGIFY(DW_AT_is_optional);
	case DW_AT_lower_bound:
		return STRINGIFY(DW_AT_lower_bound);
	case DW_AT_producer:
		return STRINGIFY(DW_AT_producer);
	case DW_AT_prototyped:
		return STRINGIFY(DW_AT_prototyped);
	case DW_AT_return_addr:
		return STRINGIFY(DW_AT_return_addr);
	case DW_AT_start_scope:
		return STRINGIFY(DW_AT_start_scope);
	case DW_AT_bit_stride:
		return STRINGIFY(DW_AT_bit_stride);
	case DW_AT_upper_bound:
		return STRINGIFY(DW_AT_upper_bound);
	case DW_AT_abstract_origin:
		return STRINGIFY(DW_AT_abstract_origin);
	case DW_AT_accessibility:
		return STRINGIFY(DW_AT_accessibility);
	case DW_AT_address_class:
		return STRINGIFY(DW_AT_address_class);
	case DW_AT_artificial:
		return STRINGIFY(DW_AT_artificial);
	case DW_AT_base_types:
		return STRINGIFY(DW_AT_base_types);
	case DW_AT_calling_convention:
		return STRINGIFY(DW_AT_calling_convention);
	case DW_AT_count:
		return STRINGIFY(DW_AT_count);
	case DW_AT_data_member_location:
		return STRINGIFY(DW_AT_data_member_location);
	case DW_AT_decl_column:
		return STRINGIFY(DW_AT_decl_column);
		// Pg: 157
	case DW_AT_decl_file:
		return STRINGIFY(DW_AT_decl_file);
	case DW_AT_decl_line:
		return STRINGIFY(DW_AT_decl_line);
	case DW_AT_declaration:
		return STRINGIFY(DW_AT_declaration);
	case DW_AT_discr_list:
		return STRINGIFY(DW_AT_discr_list);
	case DW_AT_encoding:
		return STRINGIFY(DW_AT_encoding);
	case DW_AT_external:
		return STRINGIFY(DW_AT_external);
	case DW_AT_frame_base:
		return STRINGIFY(DW_AT_frame_base);
	case DW_AT_friend:
		return STRINGIFY(DW_AT_friend);
	case DW_AT_identifier_case:
		return STRINGIFY(DW_AT_identifier_case);
	case DW_AT_macro_case:
		return STRINGIFY(DW_AT_macro_case);
	case DW_AT_namelist_item:
		return STRINGIFY(DW_AT_namelist_item);
	case DW_AT_priority:
		return STRINGIFY(DW_AT_priority);
	case DW_AT_segment:
		return STRINGIFY(DW_AT_segment);
	case DW_AT_specification:
		return STRINGIFY(DW_AT_specification);
	case DW_AT_static_link:
		return STRINGIFY(DW_AT_static_link);
	case DW_AT_type:
		return STRINGIFY(DW_AT_type);
	case DW_AT_use_parameter:
		return STRINGIFY(DW_AT_use_parameter);
	case DW_AT_variable_parameter:
		return STRINGIFY(DW_AT_variable_parameter);
	case DW_AT_virtuality:
		return STRINGIFY(DW_AT_virtuality);
	case DW_AT_vtable_elem_location:
		return STRINGIFY(DW_AT_vtable_elem_location);
		// Pg: 158
	case DW_AT_allocated:
		return STRINGIFY(DW_AT_allocated);
	case DW_AT_associated:
		return STRINGIFY(DW_AT_associated);
	case DW_AT_data_location:
		return STRINGIFY(DW_AT_data_location);
	case DW_AT_byte_stride:
		return STRINGIFY(DW_AT_byte_stride);
	case DW_AT_entry_pc:
		return STRINGIFY(DW_AT_entry_pc);
	case DW_AT_use_UTF8:
		return STRINGIFY(DW_AT_use_UTF8);
	case DW_AT_extension:
		return STRINGIFY(DW_AT_extension);
	case DW_AT_ranges:
		return STRINGIFY(DW_AT_ranges);
	case DW_AT_trampoline:
		return STRINGIFY(DW_AT_trampoline);
	case DW_AT_call_column:
		return STRINGIFY(DW_AT_call_column);
	case DW_AT_call_file:
		return STRINGIFY(DW_AT_call_file);
	case DW_AT_call_line:
		return STRINGIFY(DW_AT_call_line);
	case DW_AT_description:
		return STRINGIFY(DW_AT_description);
	case DW_AT_binary_scale:
		return STRINGIFY(DW_AT_binary_scale);
	case DW_AT_small:
		return STRINGIFY(DW_AT_small);
	case DW_AT_decimal_sign:
		return STRINGIFY(DW_AT_decimal_sign);
	case DW_AT_digit_count:
		return STRINGIFY(DW_AT_digit_count);
	case DW_AT_picture_string:
		return STRINGIFY(DW_AT_picture_string);
	case DW_AT_mutable:
		return STRINGIFY(DW_AT_mutable);
		// Pg: 159
	case DW_AT_threads_scaled:
		return STRINGIFY(DW_AT_threads_scaled);
	case DW_AT_explicit:
		return STRINGIFY(DW_AT_explicit);
	case DW_AT_object_pointer:
		return STRINGIFY(DW_AT_object_pointer);
	case DW_AT_endianity:
		return STRINGIFY(DW_AT_endianity);
	case DW_AT_elemental:
		return STRINGIFY(DW_AT_elemental);
	case DW_AT_pure:
		return STRINGIFY(DW_AT_pure);
	case DW_AT_recursive:
		return STRINGIFY(DW_AT_recursive);
	case DW_AT_signature:
		return STRINGIFY(DW_AT_signature);
	case DW_AT_main_subprogram:
		return STRINGIFY(DW_AT_main_subprogram);
	case DW_AT_data_bit_offset:
		return STRINGIFY(DW_AT_data_bit_offset);
	case DW_AT_const_expr:
		return STRINGIFY(DW_AT_const_expr);
	case DW_AT_enum_class:
		return STRINGIFY(DW_AT_enum_class);
	case DW_AT_linkage_name:
		return STRINGIFY(DW_AT_linkage_name);
	case DW_AT_lo_user:
		return STRINGIFY(DW_AT_lo_user);
	case DW_AT_hi_user:
		return STRINGIFY(DW_AT_hi_user);
	}

	printf(KERROR "Encountered unknown code: %x\n", attribute);
	abort("Unknown attribute code!");
}

// static const char *format_code_to_string(uint16_t format)
// {
// 	switch (format)
// 	{
// 	case DW_FORM_addr:
// 		return STRINGIFY(DW_FORM_addr);
// 	case DW_FORM_block2:
// 		return STRINGIFY(DW_FORM_block2);
// 	case DW_FORM_block4:
// 		return STRINGIFY(DW_FORM_block4);
// 	case DW_FORM_data2:
// 		return STRINGIFY(DW_FORM_data2);
// 	case DW_FORM_data4:
// 		return STRINGIFY(DW_FORM_data4);
// 	case DW_FORM_data8:
// 		return STRINGIFY(DW_FORM_data8);
// 	case DW_FORM_string:
// 		return STRINGIFY(DW_FORM_string);
// 	case DW_FORM_block:
// 		return STRINGIFY(DW_FORM_block);
// 	case DW_FORM_block1:
// 		return STRINGIFY(DW_FORM_block1);
// 	case DW_FORM_data1:
// 		return STRINGIFY(DW_FORM_data1);
// 	case DW_FORM_flag:
// 		return STRINGIFY(DW_FORM_flag);
// 	case DW_FORM_sdata:
// 		return STRINGIFY(DW_FORM_sdata);
// 	case DW_FORM_strp:
// 		return STRINGIFY(DW_FORM_strp);
// 		// ... more after here
// 	}

// 	printf(KERROR "Encountered unknown code: %x\n", format);
// 	abort("Unknown attribute format code!");
// }

// static const char *op_code_to_string(uint16_t op)
// {
// 	switch (op)
// 	{
// 	// Pg: 163
// 	case DW_OP_addr:
// 		return STRINGIFY(DW_OP_addr);
// 	case DW_OP_deref:
// 		return STRINGIFY(DW_OP_deref);
// 	// Pg: 163
// 	case DW_OP_const1u:
// 		return STRINGIFY(DW_OP_const1u);
// 	case DW_OP_const1s:
// 		return STRINGIFY(DW_OP_const1s);
// 	case DW_OP_const2u:
// 		return STRINGIFY(DW_OP_const2u);
// 	case DW_OP_const2s:
// 		return STRINGIFY(DW_OP_const2s);
// 	case DW_OP_const4u:
// 		return STRINGIFY(DW_OP_const4u);
// 	case DW_OP_const4s:
// 		return STRINGIFY(DW_OP_const4s);
// 	case DW_OP_const8u:
// 		return STRINGIFY(DW_OP_const8u);
// 	case DW_OP_const8s:
// 		return STRINGIFY(DW_OP_const8s);
// 	case DW_OP_constu:
// 		return STRINGIFY(DW_OP_constu);
// 	case DW_OP_consts:
// 		return STRINGIFY(DW_OP_consts);
// 	case DW_OP_dup:
// 		return STRINGIFY(DW_OP_dup);
// 	case DW_OP_drop:
// 		return STRINGIFY(DW_OP_drop);
// 	case DW_OP_over:
// 		return STRINGIFY(DW_OP_over);
// 	case DW_OP_pick:
// 		return STRINGIFY(DW_OP_pick);
// 	case DW_OP_swap:
// 		return STRINGIFY(DW_OP_swap);
// 	case DW_OP_rot:
// 		return STRINGIFY(DW_OP_rot);
// 	case DW_OP_xderef:
// 		return STRINGIFY(DW_OP_xderef);
// 	case DW_OP_abs:
// 		return STRINGIFY(DW_OP_abs);
// 	case DW_OP_and:
// 		return STRINGIFY(DW_OP_and);
// 	case DW_OP_div:
// 		return STRINGIFY(DW_OP_div);
// 	// Pg: 165
// 	case DW_OP_minus:
// 		return STRINGIFY(DW_OP_minus);
// 	case DW_OP_mod:
// 		return STRINGIFY(DW_OP_mod);
// 	case DW_OP_mul:
// 		return STRINGIFY(DW_OP_mul);
// 	case DW_OP_neg:
// 		return STRINGIFY(DW_OP_neg);
// 	case DW_OP_not:
// 		return STRINGIFY(DW_OP_not);
// 	case DW_OP_or:
// 		return STRINGIFY(DW_OP_or);
// 	case DW_OP_plus:
// 		return STRINGIFY(DW_OP_plus);
// 	case DW_OP_plus_uconst:
// 		return STRINGIFY(DW_OP_plus_uconst);
// 	case DW_OP_shl:
// 		return STRINGIFY(DW_OP_shl);
// 	case DW_OP_shr:
// 		return STRINGIFY(DW_OP_shr);
// 	case DW_OP_shra:
// 		return STRINGIFY(DW_OP_shra);
// 	case DW_OP_xor:
// 		return STRINGIFY(DW_OP_xor);
// 	case DW_OP_skip:
// 		return STRINGIFY(DW_OP_skip);
// 	case DW_OP_bra:
// 		return STRINGIFY(DW_OP_bra);
// 	case DW_OP_eq:
// 		return STRINGIFY(DW_OP_eq);
// 	case DW_OP_ge:
// 		return STRINGIFY(DW_OP_ge);
// 	case DW_OP_gt:
// 		return STRINGIFY(DW_OP_gt);
// 	case DW_OP_le:
// 		return STRINGIFY(DW_OP_le);
// 	case DW_OP_lt:
// 		return STRINGIFY(DW_OP_lt);
// 	case DW_OP_ne:
// 		return STRINGIFY(DW_OP_ne);
// 	// Pg: 166
// 	case DW_OP_lit0:
// 		return STRINGIFY(DW_OP_lit0);
// 	case DW_OP_lit1:
// 		return STRINGIFY(DW_OP_lit1);
// 	case DW_OP_lit2:
// 		return STRINGIFY(DW_OP_lit2);
// 	case DW_OP_lit3:
// 		return STRINGIFY(DW_OP_lit3);
// 	case DW_OP_lit4:
// 		return STRINGIFY(DW_OP_lit4);
// 	case DW_OP_lit5:
// 		return STRINGIFY(DW_OP_lit5);
// 	case DW_OP_lit6:
// 		return STRINGIFY(DW_OP_lit6);
// 	case DW_OP_lit7:
// 		return STRINGIFY(DW_OP_lit7);
// 	case DW_OP_lit8:
// 		return STRINGIFY(DW_OP_lit8);
// 	case DW_OP_lit9:
// 		return STRINGIFY(DW_OP_lit9);
// 	case DW_OP_lit10:
// 		return STRINGIFY(DW_OP_lit10);
// 	case DW_OP_lit11:
// 		return STRINGIFY(DW_OP_lit11);
// 	case DW_OP_lit12:
// 		return STRINGIFY(DW_OP_lit12);
// 	case DW_OP_lit13:
// 		return STRINGIFY(DW_OP_lit13);
// 	case DW_OP_lit14:
// 		return STRINGIFY(DW_OP_lit14);
// 	case DW_OP_lit15:
// 		return STRINGIFY(DW_OP_lit15);
// 	case DW_OP_lit16:
// 		return STRINGIFY(DW_OP_lit16);
// 	case DW_OP_lit17:
// 		return STRINGIFY(DW_OP_lit17);
// 	case DW_OP_lit18:
// 		return STRINGIFY(DW_OP_lit18);
// 	case DW_OP_lit19:
// 		return STRINGIFY(DW_OP_lit19);
// 	case DW_OP_lit20:
// 		return STRINGIFY(DW_OP_lit20);
// 	case DW_OP_lit21:
// 		return STRINGIFY(DW_OP_lit21);
// 	case DW_OP_lit22:
// 		return STRINGIFY(DW_OP_lit22);
// 	case DW_OP_lit23:
// 		return STRINGIFY(DW_OP_lit23);
// 	case DW_OP_lit24:
// 		return STRINGIFY(DW_OP_lit24);
// 	case DW_OP_lit25:
// 		return STRINGIFY(DW_OP_lit25);
// 	case DW_OP_lit26:
// 		return STRINGIFY(DW_OP_lit26);
// 	case DW_OP_lit27:
// 		return STRINGIFY(DW_OP_lit27);
// 	case DW_OP_lit28:
// 		return STRINGIFY(DW_OP_lit28);
// 	case DW_OP_lit29:
// 		return STRINGIFY(DW_OP_lit29);
// 	case DW_OP_lit30:
// 		return STRINGIFY(DW_OP_lit30);
// 	case DW_OP_lit31:
// 		return STRINGIFY(DW_OP_lit31);
// 	case DW_OP_reg0:
// 		return STRINGIFY(DW_OP_reg0);
// 	case DW_OP_reg1:
// 		return STRINGIFY(DW_OP_reg1);
// 	case DW_OP_reg2:
// 		return STRINGIFY(DW_OP_reg2);
// 	case DW_OP_reg3:
// 		return STRINGIFY(DW_OP_reg3);
// 	case DW_OP_reg4:
// 		return STRINGIFY(DW_OP_reg4);
// 	case DW_OP_reg5:
// 		return STRINGIFY(DW_OP_reg5);
// 	case DW_OP_reg6:
// 		return STRINGIFY(DW_OP_reg6);
// 	case DW_OP_reg7:
// 		return STRINGIFY(DW_OP_reg7);
// 	case DW_OP_reg8:
// 		return STRINGIFY(DW_OP_reg8);
// 	case DW_OP_reg9:
// 		return STRINGIFY(DW_OP_reg9);
// 	case DW_OP_reg10:
// 		return STRINGIFY(DW_OP_reg10);
// 	case DW_OP_reg11:
// 		return STRINGIFY(DW_OP_reg11);
// 	case DW_OP_reg12:
// 		return STRINGIFY(DW_OP_reg12);
// 	case DW_OP_reg13:
// 		return STRINGIFY(DW_OP_reg13);
// 	case DW_OP_reg14:
// 		return STRINGIFY(DW_OP_reg14);
// 	case DW_OP_reg15:
// 		return STRINGIFY(DW_OP_reg15);
// 	case DW_OP_reg16:
// 		return STRINGIFY(DW_OP_reg16);
// 	case DW_OP_reg17:
// 		return STRINGIFY(DW_OP_reg17);
// 	case DW_OP_reg18:
// 		return STRINGIFY(DW_OP_reg18);
// 	case DW_OP_reg19:
// 		return STRINGIFY(DW_OP_reg19);
// 	case DW_OP_reg20:
// 		return STRINGIFY(DW_OP_reg20);
// 	case DW_OP_reg21:
// 		return STRINGIFY(DW_OP_reg21);
// 	case DW_OP_reg22:
// 		return STRINGIFY(DW_OP_reg22);
// 	case DW_OP_reg23:
// 		return STRINGIFY(DW_OP_reg23);
// 	case DW_OP_reg24:
// 		return STRINGIFY(DW_OP_reg24);
// 	case DW_OP_reg25:
// 		return STRINGIFY(DW_OP_reg25);
// 	case DW_OP_reg26:
// 		return STRINGIFY(DW_OP_reg26);
// 	case DW_OP_reg27:
// 		return STRINGIFY(DW_OP_reg27);
// 	case DW_OP_reg28:
// 		return STRINGIFY(DW_OP_reg28);
// 	case DW_OP_reg29:
// 		return STRINGIFY(DW_OP_reg29);
// 	case DW_OP_reg30:
// 		return STRINGIFY(DW_OP_reg30);
// 	case DW_OP_reg31:
// 		return STRINGIFY(DW_OP_reg31);
// 	case DW_OP_breg0:
// 		return STRINGIFY(DW_OP_breg0);
// 	case DW_OP_breg1:
// 		return STRINGIFY(DW_OP_breg1);
// 	case DW_OP_breg2:
// 		return STRINGIFY(DW_OP_breg2);
// 	case DW_OP_breg3:
// 		return STRINGIFY(DW_OP_breg3);
// 	case DW_OP_breg4:
// 		return STRINGIFY(DW_OP_breg4);
// 	case DW_OP_breg5:
// 		return STRINGIFY(DW_OP_breg5);
// 	case DW_OP_breg6:
// 		return STRINGIFY(DW_OP_breg6);
// 	case DW_OP_breg7:
// 		return STRINGIFY(DW_OP_breg7);
// 	case DW_OP_breg8:
// 		return STRINGIFY(DW_OP_breg8);
// 	case DW_OP_breg9:
// 		return STRINGIFY(DW_OP_breg9);
// 	case DW_OP_breg10:
// 		return STRINGIFY(DW_OP_breg10);
// 	case DW_OP_breg11:
// 		return STRINGIFY(DW_OP_breg11);
// 	case DW_OP_breg12:
// 		return STRINGIFY(DW_OP_breg12);
// 	case DW_OP_breg13:
// 		return STRINGIFY(DW_OP_breg13);
// 	case DW_OP_breg14:
// 		return STRINGIFY(DW_OP_breg14);
// 	case DW_OP_breg15:
// 		return STRINGIFY(DW_OP_breg15);
// 	case DW_OP_breg16:
// 		return STRINGIFY(DW_OP_breg16);
// 	case DW_OP_breg17:
// 		return STRINGIFY(DW_OP_breg17);
// 	case DW_OP_breg18:
// 		return STRINGIFY(DW_OP_breg18);
// 	case DW_OP_breg19:
// 		return STRINGIFY(DW_OP_breg19);
// 	case DW_OP_breg20:
// 		return STRINGIFY(DW_OP_breg20);
// 	case DW_OP_breg21:
// 		return STRINGIFY(DW_OP_breg21);
// 	case DW_OP_breg22:
// 		return STRINGIFY(DW_OP_breg22);
// 	case DW_OP_breg23:
// 		return STRINGIFY(DW_OP_breg23);
// 	case DW_OP_breg24:
// 		return STRINGIFY(DW_OP_breg24);
// 	case DW_OP_breg25:
// 		return STRINGIFY(DW_OP_breg25);
// 	case DW_OP_breg26:
// 		return STRINGIFY(DW_OP_breg26);
// 	case DW_OP_breg27:
// 		return STRINGIFY(DW_OP_breg27);
// 	case DW_OP_breg28:
// 		return STRINGIFY(DW_OP_breg28);
// 	case DW_OP_breg29:
// 		return STRINGIFY(DW_OP_breg29);
// 	case DW_OP_breg30:
// 		return STRINGIFY(DW_OP_breg30);
// 	case DW_OP_breg31:
// 		return STRINGIFY(DW_OP_breg31);
// 	case DW_OP_regx:
// 		return STRINGIFY(DW_OP_regx);
// 	case DW_OP_fbreg:
// 		return STRINGIFY(DW_OP_fbreg);
// 	case DW_OP_bregx:
// 		return STRINGIFY(DW_OP_bregx);
// 	case DW_OP_piece:
// 		return STRINGIFY(DW_OP_piece);
// 	case DW_OP_deref_size:
// 		return STRINGIFY(DW_OP_deref_size);
// 	case DW_OP_xderef_size:
// 		return STRINGIFY(DW_OP_xderef_size);
// 	// Pg: 167
// 	case DW_OP_nop:
// 		return STRINGIFY(DW_OP_nop);
// 	case DW_OP_push_object_address:
// 		return STRINGIFY(DW_OP_push_object_address);
// 	case DW_OP_call2:
// 		return STRINGIFY(DW_OP_call2);
// 	case DW_OP_call4:
// 		return STRINGIFY(DW_OP_call4);
// 	case DW_OP_call_ref:
// 		return STRINGIFY(DW_OP_call_ref);
// 	case DW_OP_form_tls_address:
// 		return STRINGIFY(DW_OP_form_tls_address);
// 	case DW_OP_call_frame_cfa:
// 		return STRINGIFY(DW_OP_call_frame_cfa);
// 	case DW_OP_bit_piece:
// 		return STRINGIFY(DW_OP_bit_piece);
// 	case DW_OP_implicit_value:
// 		return STRINGIFY(DW_OP_implicit_value);
// 	case DW_OP_stack_value:
// 		return STRINGIFY(DW_OP_stack_value);
// 	case DW_OP_lo_user:
// 		return STRINGIFY(DW_OP_lo_user);
// 	case DW_OP_hi_user:
// 		return STRINGIFY(DW_OP_hi_user);
// 	}

// 	printf(KERROR "Encountered unknown code: %x\n", op);
// 	abort("Unknown opcode!");
// }

static void print_attribute(struct DWARF_CONTEXT *ctx, uint16_t attribute_code, uint16_t attribute_form_code, void **info_stream)
{
	uint64_t info_offset = (uintptr_t)(*info_stream) - (uintptr_t)ctx->debug_info;

	const char *attribute_string = attribute_code_to_string(attribute_code);

	printf("     <%04lx>   %-17s : ", info_offset, attribute_string);

	switch (attribute_form_code)
	{
	case DW_FORM_data1:
	{
		uint8_t constant = **((uint8_t **)info_stream);
		*info_stream += sizeof(uint8_t);
		printf("%d\n", constant);
		break;
	}

	case DW_FORM_data2:
	{
		uint16_t constant = **((uint16_t **)info_stream);
		*info_stream += sizeof(uint16_t);
		printf("%d\n", constant);
		break;
	}

	case DW_FORM_data4:
	{
		uint32_t constant = **((uint32_t **)info_stream);
		*info_stream += sizeof(uint32_t);
		printf("%d\n", constant);
		break;
	}

	case DW_FORM_data8:
	case DW_FORM_sec_offset:
	case DW_FORM_ref8:
	// WARNING: This is a shortcut! The DW_FORM_addr size is normally indicated
	// by the compilation unit header `address_size`. In our use cases it should
	// always be 8.
	case DW_FORM_addr:
	{
		uint64_t constant = **((uint64_t **)info_stream);
		*info_stream += sizeof(uint64_t);
		printf("%#lx\n", constant);
		break;
	}

	case DW_FORM_strp:
	{
		uint64_t string_offset = **((uint64_t **)info_stream);
		*info_stream += sizeof(uint64_t);
		printf("(indirect string, offset: %#lx): %s\n", string_offset, DEBUG_STR(ctx, string_offset));
		break;
	}

	case DW_FORM_string:
	{
		char *str = *((char **)info_stream);
		*info_stream += strlen(str) + 1;
		printf("%s\n", str);
		break;
	}

	// Note: flag present means no actual data exists in the .debug_info. The flag
	// should be treated as implicitly on.
	case DW_FORM_flag_present:
	{
		printf("1\n");
		break;
	}

	case DW_FORM_exprloc:
	{
		uint64_t length = DECODE_ULEB128(info_stream, uint64_t);
		printf("%ld byte block: ", length);

		for (uint64_t i = 0; i < length; i++)
		{
			uint8_t opcode = *(*(uint8_t **)info_stream + i);
			printf("%02x ", opcode);
		}
		printf("( TODO Print opcodes)");
		// for (uint64_t i = 0; i < length; i++)
		// {
		// 	uint8_t opcode = *(*(uint8_t **)info_stream + i);
		// 	if (i + 1 < length)
		// 	{
		// 		printf("%s ", op_code_to_string(opcode)); // TODO this is not accurate. It should print the interpreted instructions + operands based on the data.
		// 	}
		// 	else
		// 	{
		// 		printf("%s", op_code_to_string(opcode));
		// 	}
		// }
		printf(")\n");
		*info_stream += length;
		break;
	}

	default:
		printf("\nUnknown attribute format code: %x\n", attribute_form_code);
		print_hex_table(*(uint8_t **)info_stream, 64);
		abort("Unhandled attribute format\n");
	}
}

static uint64_t find_abbrev_entry_offset(const void *restrict abbrev_table, const uint64_t target_abbreviation_code)
{
	uint8_t *stream = (uint8_t *)abbrev_table;

	while (true) // TODO ensure we dont leave the abounds of the .debug_abbrev section
	{
		// Get offset before consuming from stream.
		uint64_t offset = (uintptr_t)stream - (uintptr_t)abbrev_table;

		uint64_t abbreviation_code = DECODE_ULEB128(&stream, uint64_t);
		if (abbreviation_code == target_abbreviation_code)
		{
			return offset;
		}

		while (true)
		{
			if (*stream == 0 && *(stream + 1) == 0)
			{
				stream += 2;
				break;
			}

			stream += 1;
		}
	}
}

// Creates a DWARF context struct for use with dwarf utility function.
struct DWARF_CONTEXT *dwarf4_init_context(const Elf64_Ehdr *restrict elf_header)
{
	ELF64_Shdr *debug_info_hdr = elf64_section_header_by_name(elf_header, ".debug_info");
	if (debug_info_hdr == NULL)
	{
		return NULL; /* Failed to find ".debug_info" section */
	}

	ELF64_Shdr *debug_abbrev_hdr = elf64_section_header_by_name(elf_header, ".debug_abbrev");
	if (debug_abbrev_hdr == NULL)
	{
		return NULL; /* Failed to find ".debug_abbrev" section */
	}

	ELF64_Shdr *debug_aranges_hdr = elf64_section_header_by_name(elf_header, ".debug_aranges");
	if (debug_aranges_hdr == NULL)
	{
		return NULL; /* Failed to find ".debug_aranges" section */
	}

	ELF64_Shdr *debug_ranges_hdr = elf64_section_header_by_name(elf_header, ".debug_ranges");
	if (debug_ranges_hdr == NULL)
	{
		return NULL; /* Failed to find ".debug_ranges" section */
	}

	ELF64_Shdr *debug_line_hdr = elf64_section_header_by_name(elf_header, ".debug_line");
	if (debug_line_hdr == NULL)
	{
		return NULL; /* Failed to find ".debug_line" section */
	}

	ELF64_Shdr *debug_str_hdr = elf64_section_header_by_name(elf_header, ".debug_str");
	if (debug_str_hdr == NULL)
	{
		return NULL; /* Failed to find ".debug_str" section */
	}

	struct DWARF_CONTEXT *ctx = (struct DWARF_CONTEXT *)kmalloc(sizeof(struct DWARF_CONTEXT));
	if (ctx == NULL)
	{
		abort("Failed to allocate memory for DWARF context.");
	}

	ctx->debug_info = ELF64_SECTION(elf_header, debug_info_hdr);
	ctx->debug_info_size = debug_info_hdr->sh_size;

	ctx->debug_abbrev = ELF64_SECTION(elf_header, debug_abbrev_hdr);
	ctx->debug_abbrev_size = debug_abbrev_hdr->sh_size;

	ctx->debug_aranges = ELF64_SECTION(elf_header, debug_aranges_hdr);
	ctx->debug_aranges_size = debug_aranges_hdr->sh_size;

	ctx->debug_ranges = ELF64_SECTION(elf_header, debug_ranges_hdr);
	ctx->debug_ranges_size = debug_ranges_hdr->sh_size;

	ctx->debug_line = ELF64_SECTION(elf_header, debug_line_hdr);
	ctx->debug_line_size = debug_line_hdr->sh_size;

	ctx->debug_str = ELF64_SECTION(elf_header, debug_str_hdr);
	ctx->debug_str_size = debug_str_hdr->sh_size;

	return ctx;
}

void dwarf4_print_compilation(struct DWARF_CONTEXT *ctx, uint64_t compilation_offset)
{
	struct DWARF4_COMPILATION_HEADER *header = (struct DWARF4_COMPILATION_HEADER *)(ctx->debug_info + compilation_offset);

	if (header->dwarf_64_format != 0xffffffff)
	{
		abort("32-bit compilation unit headers are not supported.");
	}

	printf("Compilation Unit @ offset %#08lx:\n", compilation_offset);
	printf("\tLength: %#lx (64-bit)\n", header->unit_length);
	printf("\tVersion: %d\n", header->version);
	printf("\tAbbrev Offset: %lx\n", header->debug_abbrev_offset);
	printf("\tPointer Size: %d\n", header->address_size);

	uint64_t depth = 0;

	void *abbreviation_table = ctx->debug_abbrev + header->debug_abbrev_offset;
	void *debug_info_stream = ctx->debug_info + compilation_offset + sizeof(struct DWARF4_COMPILATION_HEADER);
	uintptr_t debug_info_stream_end = (uintptr_t)debug_info_stream + header->unit_length;

	while (debug_info_stream_end > (uintptr_t)debug_info_stream)
	{
		// Get the .debug_info offset before we consume the abbreviation code from the info_stream.
		uint64_t debug_info_offset = (uintptr_t)debug_info_stream - (uintptr_t)ctx->debug_info;

		uint64_t target_abbreviation_code = DECODE_ULEB128(&debug_info_stream, uint64_t);

		// Check if we have overflowed into the next compilation unit. If we have have we should stop.
		if (target_abbreviation_code > 0xffff)
		{
			break;
		}

		// Check for null abbreviations
		if (target_abbreviation_code == 0)
		{
			printf("  <%ld><%04lx>: Abbrev Number: %ld\n", depth, debug_info_offset, target_abbreviation_code);
			continue;
		}

		void *abbrev_stream = abbreviation_table + find_abbrev_entry_offset(abbreviation_table, target_abbreviation_code);

		uint64_t abbreviation_code = DECODE_ULEB128(&abbrev_stream, uint64_t);
		uint16_t tag_code = DECODE_ULEB128(&abbrev_stream, uint16_t);

		printf("  <%ld><%04lx>: Abbrev Number: %ld (%s)\n", depth, debug_info_offset, abbreviation_code, tag_code_to_string(tag_code));

		uint8_t has_children = *(uint8_t *)abbrev_stream;
		abbrev_stream += sizeof(uint8_t);

		if (has_children)
		{
			depth++;
		}

		while (true)
		{
			uint16_t attribute_name_code = DECODE_ULEB128(&abbrev_stream, uint16_t);
			uint16_t attribute_form_code = DECODE_ULEB128(&abbrev_stream, uint16_t);

			if (attribute_name_code == 0 && attribute_form_code == 0)
			{
				break;
			}

			print_attribute(ctx, attribute_name_code, attribute_form_code, &debug_info_stream);
		}
	}
}

uint64_t decode_uleb128(uint8_t **stream)
{
	uint64_t output = 0;

	int shift = 0;
	while (true)
	{
		uint8_t byte = **stream;
		*stream += sizeof(uint8_t);

		output |= ((byte & 0x7f) << shift);

		if ((byte & 0x80) == 0)
			break;

		shift += 7;
	}

	return output;
}

int64_t decode_sleb128(uint8_t **stream, size_t bit_size)
{
	int64_t output = 0;

	size_t shift = 0;
	while (true)
	{
		uint8_t byte = **stream;
		*stream += sizeof(uint8_t);

		output |= ((byte & 0x7f) << shift);

		shift += 7;
		/* sign bit of byte is second high order bit (0x40) */
		if ((byte & 0x80) == 0)
		{
			if ((shift < bit_size) && (byte & 0x40))
				/* sign extend. */
				output |= (~0UL << shift); // TODO test

			break;
		}
	}

	return output;
}
