#ifndef __ELF_H
#define __ELF_H 1

#include <type.h>

#include "elf64.h"

#define ELF64_SHDR(elf_header, index) (ELF64_Shdr *)((void *)elf_header + elf_header->e_shoff + (index * elf_header->e_shentsize))
#define ELF64_SHDR_NAME(elf_hdr, string_section_hdr, section_hdr) (char *)((void *)elf_hdr + string_section_hdr->sh_offset + section_hdr->sh_name)
#define ELF64_SECTION(elf_header, section_header)((void *)elf_header + section_header->sh_offset)

void elf64_print_header(const Elf64_Ehdr *restrict elf_header);
void elf64_print_section_headers(const Elf64_Ehdr *restrict elf_header);
// void elf64_print_program_headers(const Elf64_Ehdr *restrict elf_header);
// void elf64_print_symbols(const Elf64_Ehdr *restrict elf_header);

err_code elf64_header(const void *restrict elf_file, Elf64_Ehdr **elf_header_ouput);
ELF64_Shdr *elf64_section_header_by_name(const Elf64_Ehdr *restrict header, const char *section_name);

// struct ELF_SECTION_LIST elf64_section_list(void *elf_file);

#endif
