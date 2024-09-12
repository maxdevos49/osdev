#include <stdbool.h>
#include <stddef.h>
#include <limine.h>

#include "memory.h"
#include "virtual.h"
#include "physical.h"
#include "../macro.h"
#include "../error.h"
#include "../cpuid.h"
#include "../instruction.h"
#include "../string/utility.h"

#define PT_POOL_SIZE (10)

#define PML5E_INDEX(virtual_addr) (((uintptr_t)virtual_address >> 48) & 0x1ffULL)
#define PML4E_INDEX(virtual_addr) (((uintptr_t)virtual_address >> 39) & 0x1ffULL)
#define PDPE_INDEX(virtual_addr) (((uintptr_t)virtual_address >> 30) & 0x1ffULL)
#define PDE_INDEX(virtual_addr) (((uintptr_t)virtual_address >> 21) & 0x1ffULL)
#define PTE_INDEX(virtual_addr) (((uintptr_t)virtual_address >> 12) & 0x1ffULL)

union PAGE_ENTRY
{
	uint64_t raw;

	struct
	{
		uint8_t present : 1,
			read_write : 1,
			user_supervisor : 1,
			write_through : 1,
			cache_disable : 1,
			accessed : 1,
			dirty : 1,
			large_page_or_pat : 1;
		uint8_t addr_bits[6];
		uint8_t : 3, protection_key : 4, execute_disable : 1;
	};
};
_Static_assert(sizeof(union PAGE_ENTRY) == sizeof(uint64_t));

struct PAGE_TABLE
{
	union PAGE_ENTRY entries[512];
};
_Static_assert(sizeof(struct PAGE_TABLE) == (sizeof(uint64_t) * 512));

struct VIRTUAL_MEMORY_CONTEXT
{
	struct PAGE_TABLE *pml4_table;
	uint64_t pt_pool_next_index;
	virt_addr_t pt_pool[PT_POOL_SIZE];
	bool pt_pool_ready;
	uint8_t physical_address_size;
	uint8_t virtual_address_size;
};

static struct VIRTUAL_MEMORY_CONTEXT _vm_context = {0};

// Gets the table pointed by a page entry.
static struct PAGE_TABLE *get_table_from_entry(union PAGE_ENTRY *entry)
{
	uint64_t phys_mask = (1ULL << (_vm_context.physical_address_size - 12)) - 1;
	phys_addr_t physical_address = (phys_addr_t)((entry->raw >> 12) & phys_mask) * PAGE_BYTE_SIZE;
	virt_addr_t virtual_address = phys_to_virt(physical_address);

	return (struct PAGE_TABLE *)virtual_address;
}

// Gets a new page table from the page pool.
static struct PAGE_TABLE *get_new_page_table()
{
	uint64_t index = _vm_context.pt_pool_next_index++ % PT_POOL_SIZE;

	virt_addr_t virtual_address = _vm_context.pt_pool[index];
	_vm_context.pt_pool[index] = NULL;

	return (struct PAGE_TABLE *)virtual_address;
}

// Restocks the page pool by allocating a new page frame and mapping it into kernel space.
static void maybe_restock_page_table_pool(void)
{
	if (_vm_context.pt_pool_ready == false)
	{
		return;
	}

	for (uint64_t i = 0; i < PT_POOL_SIZE; i++)
	{
		if (_vm_context.pt_pool[i] == NULL)
		{
			phys_addr_t physical_address = allocate_memory(PAGE_BYTE_SIZE);

			if (physical_address == INVALID_PHYS)
			{
				abort("Unable to allocate page frame for a page table.\n");
			}

			virt_addr_t virtual_address = phys_to_virt(physical_address);

			// To avoid a recursive loop we are gonna go ahead and assign the virtual address.
			_vm_context.pt_pool[i] = virtual_address;

			map_memory(physical_address, virtual_address, PAGE_BYTE_SIZE, PAGE_MAP_WRITEABLE);

			memset(virtual_address, 0, PAGE_BYTE_SIZE);
		}
	}
}

// Sets a page entry values.
static void set_page_entry(union PAGE_ENTRY *entry, phys_addr_t addr, uint16_t flags)
{
	entry->present = true;
	entry->read_write = flags & PAGE_MAP_WRITEABLE ? true : false;
	entry->user_supervisor = flags & PAGE_MAP_USER ? true : false;

	uint64_t phys_mask = (1ULL << (_vm_context.physical_address_size - 12)) - 1;
	uint64_t phys_index = addr / PAGE_BYTE_SIZE; // This will effectively shift the address right 12 bits

	if (phys_index > phys_mask)
	{
		// Whats this mean?
		abort("Something went wrong.");
		return;
	}

	entry->raw &= ~(phys_mask << 12); // Clear existing address if present.
	entry->raw |= phys_index << 12;	  // Set new address
}

// Map a single page.
static bool map_page(phys_addr_t physical_address, virt_addr_t virtual_address, uint32_t flags)
{
	struct PAGE_TABLE *pml4_table = _vm_context.pml4_table;
	if (pml4_table == NULL)
	{
		printf(KERROR "PML4 table is null\n");
		return false;
	}

	union PAGE_ENTRY *pml4_entry = &pml4_table->entries[PML4E_INDEX(virtual_address)];
	struct PAGE_TABLE *pdp_table = NULL;
	if (!pml4_entry->present)
	{
		pdp_table = get_new_page_table();
		set_page_entry(pml4_entry, virt_to_phys((virt_addr_t)pdp_table), PAGE_MAP_WRITEABLE);
	}
	else
	{
		pdp_table = get_table_from_entry(pml4_entry);
	}

	union PAGE_ENTRY *pdp_entry = &pdp_table->entries[PDPE_INDEX(virtual_address)];
	struct PAGE_TABLE *pd_table = NULL;
	if (!pdp_entry->present)
	{
		pd_table = get_new_page_table();
		set_page_entry(pdp_entry, virt_to_phys((virt_addr_t)pd_table), PAGE_MAP_WRITEABLE);
	}
	else
	{
		pd_table = get_table_from_entry(pdp_entry);
	}

	union PAGE_ENTRY *pd_entry = &pd_table->entries[PDE_INDEX(virtual_address)];
	struct PAGE_TABLE *pt_table = NULL;
	if (!pd_entry->present)
	{
		pt_table = get_new_page_table();
		set_page_entry(pd_entry, virt_to_phys((virt_addr_t)pt_table), PAGE_MAP_WRITEABLE);
	}
	else
	{
		pt_table = get_table_from_entry(pd_entry);
	}

	union PAGE_ENTRY *pt_entry = &pt_table->entries[PTE_INDEX(virtual_address)];
	if (pt_entry->present)
	{
		// Page is already mapped!
		printf(KWARN "[WARNING] Page %p is already mapped!\n", physical_address);
		return false;
	}
	else
	{
		set_page_entry(pt_entry, physical_address, flags);
	}

	// if (read_CR3() == _vm_context.pml4_table) {
	// 	flush_tlb(virtual_address);
	// }

	maybe_restock_page_table_pool();

	return true;
}

// Maps a virtual address to a given physical address for the required amount of pages needed by the given size in bytes.
bool map_memory(phys_addr_t physical_addr, virt_addr_t virtual_addr, size_t size_in_bytes, uint32_t flags)
{
	for (uintptr_t offset = 0; offset < size_in_bytes; offset += PAGE_BYTE_SIZE)
	{
		if (!map_page(physical_addr + offset, virtual_addr + offset, flags))
		{
			return false;
		}
	}

	return true;
}

void print_memory_mapping(void)
{
	uint64_t entry_address_mask = (0xffffffffffffffff >> (64 - _vm_context.physical_address_size)) & 0xfffffffffffff000;

	struct PAGE_TABLE *pml4_table = _vm_context.pml4_table;
	printf("PML4 Table: %p\n", virt_to_phys(pml4_table));

	for (uint64_t i = 0; i < 512; i++)
	{

		union PAGE_ENTRY *pml4_entry = &pml4_table->entries[i];
		if (!pml4_entry->present)
		{
			continue;
		}

		struct PAGE_TABLE *pdp_table = get_table_from_entry(pml4_entry);
		printf("PML4E Index: %3d | PML4E: %p | PDP Table: %p\n", i, *pml4_entry, virt_to_phys(pdp_table));

		for (uint64_t ii = 0; ii < 512; ii++)
		{

			union PAGE_ENTRY *pdp_entry = &pdp_table->entries[ii];
			if (!pdp_entry->present)
			{
				continue;
			}

			struct PAGE_TABLE *pd_table = get_table_from_entry(pdp_entry);
			printf("\tPDPE Index: %3d | PDPE: %p | PD Table: %p\n", ii, *pdp_entry, virt_to_phys(pd_table));

			for (uint64_t iii = 0; iii < 512; iii++)
			{

				union PAGE_ENTRY *pd_entry = &pd_table->entries[iii];
				if (!pd_entry->present)
				{
					continue;
				}

				struct PAGE_TABLE *pt_table = get_table_from_entry(pd_entry);
				printf("\t\tPDE Index: %3d | PDE: %p | PT Table: %p\n", iii, *pd_entry, virt_to_phys(pt_table));

				for (uint64_t iiii = 0; iiii < 512; iiii++)
				{

					union PAGE_ENTRY *page_entry = &pt_table->entries[iiii];
					if (!page_entry->present)
					{
						continue;
					}

					uintptr_t virtual_address = ((i << 39) | (ii << 30) | (iii << 21) | (iiii << 12)) | (0xffffffffffffffff << _vm_context.virtual_address_size);
					printf("\t\t\tPTE Index: %3d | PTE: %p | Physical: %p | Virtual: %p\n", iiii, *page_entry, page_entry->raw & entry_address_mask, virtual_address);
				}
			}
		}
	}
}

void init_virtual_memory(struct MEMORY_BITMAP bitmap)
{
	printf(KINFO "Initiating virtual memory management...\n");

	struct LONG_MODE_SIZE_IDENTIFIERS lmsi = cpuid_long_mode_size_identifiers();

	_vm_context.physical_address_size = lmsi.physical_address_size;
	_vm_context.virtual_address_size = lmsi.virtual_address_size;

	printf("\tSupported physical address bits: %d\n", _vm_context.physical_address_size);
	printf("\tSupported virtual address bits: %d\n", _vm_context.virtual_address_size);
	printf("\tHHDM offset: %p\n", hhdm_request.response->offset);

	// Start a new PML4 table
	phys_addr_t pml4_table_physical_address = allocate_memory(1);
	if (pml4_table_physical_address == INVALID_PHYS)
	{
		abort("Invalid physical address returned for PML4 table\n");
	}

	virt_addr_t pml4_table_virtual_address = phys_to_virt(pml4_table_physical_address);
	memset((void *)pml4_table_virtual_address, 0, PAGE_BYTE_SIZE);

	_vm_context.pml4_table = pml4_table_virtual_address;

	printf(KINFO "Populating page table pool...\n");

	// Prefill the page table pool.
	for (uint64_t i = 0; i < PT_POOL_SIZE; i++)
	{
		phys_addr_t physical_address = allocate_memory(1);
		virt_addr_t virtual_address = phys_to_virt(physical_address);

		memset((void *)virtual_address, 0, PAGE_BYTE_SIZE);

		_vm_context.pt_pool[i] = virtual_address;
	}

	printf("\t%'d pages added to the page pool\n", PT_POOL_SIZE);

	printf(KINFO "Setting up new PML4 table...\n\tPhysical address: %p\n\tVirtual address: %p\n", pml4_table_physical_address, pml4_table_virtual_address);

	printf(KINFO "Populating PML4 table...\n");

	// Map the page table pool
	map_memory(virt_to_phys(_vm_context.pt_pool[0]), _vm_context.pt_pool[0], PT_POOL_SIZE * PAGE_BYTE_SIZE, PAGE_MAP_WRITEABLE);

	// Mark the page table pool as ready for use and restocking.
	_vm_context.pt_pool_ready = true;

	// Map new PML4 table
	map_memory(pml4_table_physical_address, pml4_table_virtual_address, PAGE_BYTE_SIZE, PAGE_MAP_WRITEABLE);

	// Map the Page frame bitmap
	map_memory(virt_to_phys(bitmap.address), bitmap.address, bitmap.size, PAGE_MAP_WRITEABLE);

	uint64_t pages_mapped = 11 + bitmap.size / PAGE_BYTE_SIZE;
	// Map the kernel, framebuffer, and bootloader regions(until own GDT is setup)
	for (uintptr_t i = 0; i < memmap_request.response->entry_count; i++)
	{
		struct limine_memmap_entry *entry = memmap_request.response->entries[i];

		switch (entry->type)
		{
		case LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE:
		case LIMINE_MEMMAP_KERNEL_AND_MODULES:
		case LIMINE_MEMMAP_FRAMEBUFFER:
			phys_addr_t physical_address = (phys_addr_t)entry->base;
			virt_addr_t virtual_address = phys_to_virt(entry->base);

			if (entry->type == LIMINE_MEMMAP_KERNEL_AND_MODULES)
			{
				// The Limine protocol demands the kernel to be placed at 0xffffffff80000000 at
				// minimum. This is different then the HHDM Offset so dont use the HHDM Offset here.
				virtual_address = (virt_addr_t)kernel_address_request.response->virtual_base;
			}

			map_memory(physical_address, virtual_address, entry->length, PAGE_MAP_WRITEABLE);
			pages_mapped += entry->length / PAGE_BYTE_SIZE;
			break;
		}
	}

	printf("\t%'d pages mapped into PML4 table\n", pages_mapped);

	printf(KINFO "Transferring to new PML4 table...\n");
	// Start using new page table
	uint64_t current_cr3 = read_CR3();
	uint64_t new_cr3_address = (current_cr3 & 0xFFFULL) | (pml4_table_physical_address & ~0xFFFULL);
	printf("\tOld CR3 value: %p\n", current_cr3);
	write_CR3(new_cr3_address);
	printf("\tNew CR3 value: %p\n", read_CR3());

	printf(KOK "Virtual memory management ready\n");
}
