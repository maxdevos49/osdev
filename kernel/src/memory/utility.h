#ifndef __MEMORY_UTILITY_H
#define __MEMORY_UTILITY_H 1

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define INVALID_PHYS (1UL << 52)

typedef uintptr_t phys_addr_t;
typedef void *virt_addr_t;

void init_memory(void);

// inline phys_addr_t io_to_physical(virt_addr_t addr);
// inline virt_addr_t physical_to_io(phys_addr_t addr);


// //
// // #### Physical Memory Management #############################################
// //

// size_t frame_allocator_required_size(const size_t total_system_memory_in_bytes);
// void initialize_frame_allocator(const void *start_address, const size_t size);
// void open_frame_region(const uint64_t start_address, const size_t size);
// void close_frame_region(const uint64_t start_address, const size_t size);
// void *allocate_frames(const uint64_t num_frames);
// void free_frames(uint64_t ptr, uint64_t num_frames);

// //
// // #### Paging Management ######################################################
// //

// void initialize_page_allocator(void);
// bool map_page(uint64_t physical_address, uint64_t virtual_address, uint8_t flags);
// void unmap_page(void *virtual_address);

// //
// // #### General Purpose Heap memory management #################################
// //

// void initialize_heap(void *address, size_t size);
// void *kmalloc(size_t size);
// void kfree(void *ptr);

// // typedef uintptr_t phys_addr_t;
// // typedef void *virt_addr_t;

// // TODO whats io mean?
// // virt_addr_t physical_to_io(phys_addr_t addr);
// // phys_addr_t io_to_physical(virt_addr_t addr);

// // phys_addr_t pmm_allocate(size_t size);
// // void pmm_deallocate(phys_addr_t addr, size_t size);

// // void vmm_init(struct cpu_context* context);
// // void vmm_init_for_kernel_space(struct cpu_context* context);
// // void vmm_deinit(struct cpu_context* context);
// // void vmm_load(struct cpu_context* context);
// // bool vmm_map(struct cpu_context* context, phys_addr_t phys, virt_addr_t virt, size_t size, uint32_t flags);
// // bool vmm_unmap(struct cpu_context* context, virt_addr_t virt, size_t size);
// // phys_addr_t vmm_virt_to_phys(struct cpu_context* context, virt_addr_t virt);

// // void vmm_print_mapping(struct cpu_context* context, virt_addr_t virt);

// // void init_memory(void);

#endif