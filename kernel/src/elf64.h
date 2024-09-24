#ifndef __ELF64_H
#define __ELF64_H 1

#include <stdint.h>
#include "macro.h"

// 64-Bit ELF Specification: https://uclibc.org/docs/elf-64-gen.pdf

//
// Elf Types
//

typedef uint64_t Elf64_Addr;  // Unsigned program address.
typedef uint64_t Elf64_Off;	  // Unsigned file offset
typedef uint16_t Elf64_Half;  // Unsigned medium integer
typedef uint32_t Elf64_Word;  // Unsigned integer
typedef int32_t Elf64_Sword;  // Signed integer
typedef uint64_t Elf64_Xword; // Unsigned long integer
typedef int64_t Elf64_Sxword; // Signed long integer

//
// Elf Header
//

typedef struct
{
	unsigned char e_ident[16]; /* Elf identifier */
	Elf64_Half e_type;		   /* Object file type*/
	Elf64_Half e_machine;	   /* Machine type */
	Elf64_Word e_version;	   /* Object file  version */
	Elf64_Addr e_entry;		   /* Entry point address */
	Elf64_Off e_phoff;		   /* Program header offset */
	Elf64_Off e_shoff;		   /* Section header offset  */
	Elf64_Word e_flags;		   /* Processor specific flags */
	Elf64_Half e_ehsize;	   /* Elf header size */
	Elf64_Half e_phentsize;	   /* Size of program header entry */
	Elf64_Half e_phnum;		   /* Number of program entries */
	Elf64_Half e_shentsize;	   /* Size of section header entry */
	Elf64_Half e_shnum;		   /* Number of section header entries */
	Elf64_Half e_shstrndx;	   /* Section name string table index */
} Elf64_Ehdr;

#define EL_MAG0 0
#define EL_MAG1 1
#define EL_MAG2 2
#define EL_MAG3 3
#define EL_CLASS 4
#define EL_DATA 5
#define EL_VERSION 6
#define EL_OSABI 7
#define EL_ABIVERSION 8
#define EL_PAD 9
#define EL_NIDENT 16

#define ELF_MAGIC(ident) (ident[EL_MAG0] == '\x7f' && ident[EL_MAG1] == 'E' && ident[EL_MAG2] == 'L' && ident[EL_MAG3] == 'F')

enum ELF_CLASS
{
	ELFCLASS32 = 1, /* 32-bit objects */
	ELFCLASS64 = 2	/* 64-bit objects */
};

enum ELF_DATA
{
	ELFDATA2LSB = 1, /* Object file data structures are little endian */
	ELFDATA2MSB = 2	 /* Object file data structures are big endian */
};

enum ELF_OSABI
{
	ELFOSABI_SYSV = 0,		  /* System V ABI */
	ELFOSABI_HPUX = 1,		  /* HP-UX operating system */
	ELFOSABI_STANDALONE = 255 /* Standalone (embedded) application */
};

enum ELF_TYPE
{
	ET_NONE = 0,	  /* No file type */
	ET_REL = 1,		  /* Relocatable object file */
	ET_EXEC = 2,	  /* Executable file */
	ET_DYN = 3,		  /* Shared object file */
	ET_CORE = 4,	  /* Core file*/
	ET_LOOS = 0xfe00, /* Environment-specific use */
	ET_HIOS = 0xfeff,
	ET_LOPROC = 0xff00, /* Processor-specific use*/
	ET_HIPROC = 0xffff,
};

//
// Elf Sections header table | Specification page: 6
//
// Notes:
// 	Sections contain all information in a ELF file. Section index 0 is reserved
//	as a null entry. Indexes 0xff00 through 0xffff are reserved for specific
//	purposes.
//

typedef struct
{
	Elf64_Word sh_name;		  /* Section name */
	Elf64_Word sh_type;		  /* Section type */
	Elf64_Xword sh_flags;	  /* Section attributes */
	Elf64_Addr sh_addr;		  /* Virtual address in memory */
	Elf64_Off sh_offset;	  /* Offset in file */
	Elf64_Xword sh_size;	  /* Size of section */
	Elf64_Word sh_link;		  /* Link to other section */
	Elf64_Word sh_info;		  /* Misc information*/
	Elf64_Xword sh_addralign; /* Address alignment boundary */
	Elf64_Xword sh_entsize;	  /* Size of entries, if section has table */
} ELF64_Shdr;

enum ELF_SECTION_HEADER_TYPE
{
	SHT_NULL = 0,		   /* Marks an unused section header */
	SHT_PROGBITS = 1,	   /* Contains information defined by the program */
	SHT_SYMTAB = 2,		   /* Contains a linker symbol table */
	SHT_STRTAB = 3,		   /* Contains a string table */
	SHT_RELA = 4,		   /* Contains "Rela" type relocation entries */
	SHT_HASH = 5,		   /* Contains a symbol hash table */
	SHT_DYNAMIC = 6,	   /* Contains a dynamic linking tables */
	SHT_NOTE = 7,		   /* Contains note information */
	SHT_NOBITS = 8,		   /* Contains uninitialized space; does not occupy any space in the file */
	SHT_REL = 9,		   /* Contains "Rel" type relocation entries */
	SHT_SHLIB = 10,		   /* Reserved */
	SHT_DYNSYM = 11,	   /* Contains a dynamic loader symbol table */
	SHT_LOOS = 0x60000000, /* Environment-specific use */
	SHT_HIOS = 0x6fffffff,
	SHT_LOPROC = 0x70000000, /* Processor-specific use*/
	SHT_HIPROC = 0x7fffffff
};

enum ELF_SECTION_HEADER_FLAGS
{
	SHF_WRITE = 0x1,		  /* Section contains writeable data */
	SHF_ALLOC = 0x2,		  /* Section is allocated in memory image of program */
	SHF_EXECINSTR = 0x4,	  /* Section contains executable instructions */
	SHF_MERGE = 0x10,		  /* Data may be merged to eliminate duplication */
	SHF_STRINGS = 0x20,		  /* Section contains strings */
	SHF_INFO_LINK = 0x40,	  /* sh_info holds a section header table index */
	SHF_MASKOS = 0x0f000000,  /* Environment-specific use */
	SHF_MASKPROC = 0xf0000000 /* Processor-specific use */
};

//
// ELF Symbols table | Specification page: 9
//

typedef struct
{
	Elf64_Word st_name;		/* Symbol name */
	unsigned char st_info;	/* Type and Binding attributes */
	unsigned char st_other; /* Reserved */
	Elf64_Half st_shndx;	/* Section table index */
	Elf64_Addr st_value;	/* Symbol value */
	Elf64_Xword st_size;	/* Size of object (e.g., common) */
} ELF64_Sym;

#endif
