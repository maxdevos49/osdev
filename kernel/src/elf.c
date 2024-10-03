#include <limine.h>
#include <stddef.h>
#include "string/utility.h"

#include "elf64.h"
#include "elf.h"

static const char *elf_type_name(const Elf64_Ehdr *header)
{
	switch (header->e_type)
	{
	case ET_NONE:
		return "NONE";
	case ET_REL:
		return "REL (Relocatable object file)";
	case ET_EXEC:
		return "EXEC (Executable file)";
	case ET_DYN:
		return "DYN (Shared object file)";
	case ET_CORE:
		return "CORE";
	default:
		return "UNKNOWN";
	}
}

// Gets the section type name null terminated string.
static const char *section_type_name(const ELF64_Shdr *section)
{
	switch (section->sh_type)
	{
	case SHT_NULL:
		return "NULL";
	case SHT_PROGBITS:
		return "PROGBITS";
	case SHT_SYMTAB:
		return "SYMTAB";
	case SHT_STRTAB:
		return "STRTAB";
	case SHT_RELA:
		return "RELA";
	case SHT_HASH:
		return "HASH";
	case SHT_DYNAMIC:
		return "DYNAMIC";
	case SHT_NOTE:
		return "NOTE";
	case SHT_NOBITS:
		return "NOBITS";
	case SHT_REL:
		return "REL";
	case SHT_SHLIB:
		return "SHLIB";
	case SHT_DYNSYM:
		return "DYNSYM";
	default:
		return "UNKNOWN";
	}
}

// Prints a human readable string of the section flags.
static void snprint_section_flags(const ELF64_Shdr *section, char *flags_str_output, const size_t output_size)
{
	size_t i = 0;

	do
	{
		if (section->sh_flags & SHF_WRITE)
		{
			flags_str_output[i++] = 'W';
		}

		if (output_size - 1 == i)
			break;

		if (section->sh_flags & SHF_ALLOC)
		{
			flags_str_output[i++] = 'A';
		}

		if (output_size - 1 == i)
			break;

		if (section->sh_flags & SHF_EXECINSTR)
		{
			flags_str_output[i++] = 'X';
		}

		if (output_size - 1 == i)
			break;

		if (section->sh_flags & SHF_MERGE)
		{
			flags_str_output[i++] = 'M';
		}

		if (output_size - 1 == i)
			break;

		if (section->sh_flags & SHF_STRINGS)
		{
			flags_str_output[i++] = 'S';
		}

		if (output_size - 1 == i)
			break;

		if (section->sh_flags & SHF_INFO_LINK)
		{
			flags_str_output[i++] = 'I';
		}

	} while (0);

	flags_str_output[i] = '\0';
}

/**
 * Sets the `elf_header_ouput` param to a valid pointer. If the elf header is invalid
 * the `elf_header_ouput` param is set to NULL.
 *
 * Error codes:
 * 	1 -> Invalid magic bytes
 * 	2 -> Elf file is not 64 bit
 * 	3 -> Elf file is not little endian
 */
err_code elf64_header(const void *restrict elf_file, Elf64_Ehdr **elf_header_ouput)
{
	*elf_header_ouput = NULL;
	Elf64_Ehdr *header = (Elf64_Ehdr *)elf_file;

	// Check Magic bytes
	if (!ELF_MAGIC(header->e_ident))
	{
		return 1;
	}

	// Check ELF class
	if (header->e_ident[EL_CLASS] != ELFCLASS64)
	{
		return 2;
	}

	// Check endian
	if (header->e_ident[EL_DATA] != ELFDATA2LSB)
	{
		return 3;
	}

	// TODO verify ABI + ABI Version

	// TODO verify Machine

	// TODO verify version

	*elf_header_ouput = header;

	return 0;
}

// Gets a elf section via its name.
ELF64_Shdr *elf64_section_header_by_name(const Elf64_Ehdr *restrict header, const char *section_name)
{
	ELF64_Shdr *string_section = ELF64_SHDR(header, header->e_shstrndx);

	for (int i = 0; i < header->e_shnum; i++)
	{
		ELF64_Shdr *section = ELF64_SHDR(header, i);

		if (0 == strcmp(section_name, ELF64_SHDR_NAME(header, string_section, section)))
		{
			return section;
		}
	}

	return NULL;
}


// Prints details about the elf file header.
void elf64_print_header(const Elf64_Ehdr *restrict header)
{
	printf("ELF Header:\n");
	printf(
		"\tMagic:    %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
		header->e_ident[0],
		header->e_ident[1],
		header->e_ident[2],
		header->e_ident[3],
		header->e_ident[4],
		header->e_ident[5],
		header->e_ident[6],
		header->e_ident[7],
		header->e_ident[8],
		header->e_ident[9],
		header->e_ident[10],
		header->e_ident[11],
		header->e_ident[12],
		header->e_ident[13],
		header->e_ident[14],
		header->e_ident[15]);
	printf("\tClass:                             %s\n", header->e_ident[EL_CLASS] == ELFCLASS64 ? "ELF64" : "ELF32");
	printf("\tData:                              %s\n", header->e_ident[EL_DATA] == ELFDATA2LSB ? "2's complement, little endian" : "big endian");
	printf("\tVersion:                           %d\n", header->e_ident[EL_VERSION]);
	printf("\tOS/ABI:                            %s\n", header->e_ident[EL_OSABI] == ELFOSABI_SYSV ? "UNIX - System V" : "Unknown/Unsupported ABI");
	printf("\tABI Version:                       %d\n", header->e_ident[EL_ABIVERSION]);
	printf("\tType:                              %s\n", elf_type_name(header));
	printf("\tMachine:                           %s\n", header->e_machine == 62 ? "Advanced Micro Devices X86-64" : "Unknown machine");
	printf("\tVersion:                           %#01x\n", header->e_version);
	printf("\tEntry point address:               %#018lx\n", header->e_entry);
	printf("\tStart of program headers:          %ld (bytes into file)\n", header->e_phoff);
	printf("\tStart of section headers:          %ld (bytes into file)\n", header->e_shoff);
	printf("\tFlags:                             %#01x\n", header->e_flags);
	printf("\tSize of this header:               %d (bytes)\n", header->e_ehsize);
	printf("\tSize of program headers:           %d (bytes)\n", header->e_phentsize);
	printf("\tNumber of program headers:         %d\n", header->e_phnum);
	printf("\tSize of section headers:           %d (bytes)\n", header->e_shentsize);
	printf("\tNumber of section headers:         %d\n", header->e_shnum);
	printf("\tSection header string table index: %d\n", header->e_shstrndx);
}

// Prints the sections pointed by the given elf header.
void elf64_print_section_headers(const Elf64_Ehdr *restrict elf_header)
{
	ELF64_Shdr *string_section = ELF64_SHDR(elf_header, elf_header->e_shstrndx);

	printf("[Nr] Name            Type     Addr             Off      Size   ES Flg Lk Inf Al\n");
	for (int i = 0; i < elf_header->e_shnum; i++)
	{
		ELF64_Shdr *section = ELF64_SHDR(elf_header, i);

		char section_flags_str[4] = {0};
		snprint_section_flags(section, section_flags_str, 4);

		printf(
			"[%2d] %-15s %-8s %016lx %08lx %06lx %02lx %3s %2d %3d %2ld\n",
			i,
			ELF64_SHDR_NAME(elf_header, string_section, section),
			section_type_name(section),
			section->sh_addr,
			section->sh_offset,
			section->sh_size,
			section->sh_entsize,
			section_flags_str,
			section->sh_link,
			section->sh_info,
			section->sh_addralign);
	}
}
