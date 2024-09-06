// #include <stdbool.h>

// #include "utility.h"
// #include "../string/utility.h"

// enum CR3_MASKS
// {
// 	CR3_PM_ADDRESS = 0x000ffffffffff000,
// 	// When CR4.PCIDE = 0
// 	CR3_PCID = 0x0000000000000fff,
// 	// When CR4.PCIDE = 1
// 	CR3_PCD = 0x0000000000000010,
// 	CR3_PWT = 0x0000000000000008,
// };

// enum PAGE_ADDRESS_TRANSLATION_MASKS
// {
// 	PML5E_INDEX_SHIFT = 48,
// 	PML4E_INDEX_SHIFT = 39,
// 	PDPE_INDEX_SHIFT  = 30,
// 	PDE_INDEX_SHIFT   = 21,
// 	PTE_INDEX_SHIFT   = 12,
// 	PT_ENTRY          = 0x00000000000001ff,
// 	PAGE_OFFSET       = 0x0000000000000fff
// };

// enum PAGE_TABLE_ENTRY_MASKS
// {
// 	PT_ENTRY_ADDRESS = 0x000ffffffffff000,
// 	PTE_A            = 0x0000000000000020,
// 	PTE_PCD          = 0x0000000000000010,
// 	PTE_PWT          = 0x0000000000000008,
// 	PTE_U            = 0x0000000000000004,
// 	PTE_W            = 0x0000000000000002,
// 	PT_ENTRY_PRESENT = 0x0000000000000001
// };

// // Get the contents of the CR3 Register
// static inline uint64_t read_CR3(void)
// {
// 	uint64_t val;
// 	__asm__ volatile("mov %%cr3, %0" : "=r"(val));
// 	return val;
// }

// // Set the contents of the CR3 Register.
// static inline void write_CR3(uint64_t value) {
// 	__asm__ volatile("movq %[cr3_val], %%cr3;" :: [cr3_val]"r"(value));
// }

// // Invalidates the given page entry.
// static inline void flush_tlb(uint64_t virtual_address)
// {
// 	__asm__ volatile("invlpg (%0)" ::"r"(virtual_address) : "memory");
// }

// bool map_page(uint64_t physical_address, uint64_t virtual_address, uint8_t flags)
// {
// 	// Note this assumes we are using PML4 paging and only 4kb pages will be mapped.

// 	uint64_t* pml4_address = (uint64_t*)(read_CR3() & CR3_PM_ADDRESS);
// 	uint64_t  pml4_entry   = pml4_address[(virtual_address >> PML4E_INDEX_SHIFT) & PT_ENTRY];
// 	if(!(pml4_entry & PT_ENTRY_PRESENT)) {
// 		// TODO Allocate a frame for the new PDPE table.

// 		// TODO SET address of PDPE table and mark present on PML4 entry. 
// 	}else {
// 		// TODO Already present
// 	}

// 	uint64_t* pdpe_address = (uint64_t*)(pml4_entry & PT_ENTRY_ADDRESS);
// 	uint64_t  pdpe_entry   = pdpe_address[(virtual_address >> PDPE_INDEX_SHIFT) & PT_ENTRY];
// 	if(!(pdpe_entry & PT_ENTRY_PRESENT)){
// 		// TODO Allocate a frame for the new PDE table.

// 		// TODO SET address of PDE table and mark present on PDPE entry.
// 	}else {
// 		// TODO Already present
// 	}

// 	uint64_t* pde_address = (uint64_t*)(pdpe_entry & PT_ENTRY_ADDRESS);
// 	uint64_t  pde_entry   = pde_address[(virtual_address >> PDE_INDEX_SHIFT) & PT_ENTRY];
// 	if(!(pde_entry & PT_ENTRY_PRESENT)) {
// 		// TODO Allocate a frame for the new PTE table.

// 		// TODO SET address of PTE table and mark present on PDE entry. 
// 	}else {
// 		// TODO Already present
// 	}

// 	uint64_t* pte_address = (uint64_t*)(pde_entry & PT_ENTRY_ADDRESS);
// 	uint64_t  pte_entry   = pte_address[(virtual_address >> PTE_INDEX_SHIFT) & PT_ENTRY];
// 	if(!(pte_entry & PT_ENTRY_PRESENT)) {
// 		// TODO allocate frame for page.

// 		// TODO SET address of page and mark present on PTE entry.
// 	}else {
// 		//TODO abort page is already allocated
// 		return false;
// 	}

// 	uint64_t physical_page_offset = virtual_address & PAGE_OFFSET;

// 	flush_tlb(virtual_address);

// 	return true;
// }

// void unmap_page(void *virtual_address)
// {
// 	// TODO
// }

// void initialize_page_allocator(void)
// {
// 	// Get the current PML4 table setup by limine. (We will be changing this probably)
// 	void *pml4_address = (void *)((read_CR3() & CR3_PM_ADDRESS));

// 	printf("CR3 = %p\n", pml4_address);
// }