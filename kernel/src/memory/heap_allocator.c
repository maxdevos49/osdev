// #include <stdint.h>
// #include <stddef.h>

// #include "../string/utility.h"

// // #define HEAP_BLOCK_FREE_MASK ((size_t)1 << 31)
// // #define HEAP_BLOCK_ALLOCATED_MASK (~HEAP_BLOCK_FREE_MASK)

// // #define HEAP_BLOCK_LENGTH(len) ((size_t)(len & HEAP_BLOCK_ALLOCATED_MASK))
// // #define HEAP_BLOCK_IS_FREE(len) (len & HEAP_BLOCK_FREE_MASK)

// struct HEAP_BLOCK
// {
// 	size_t length;
// 	uint64_t free;
// 	struct HEAP_BLOCK *previous;
// 	struct HEAP_BLOCK *next;
// 	struct HEAP_BLOCK *next_free;
// };

// static struct HEAP_BLOCK *_root_block = NULL;
// static struct HEAP_BLOCK *_first_free_block = NULL;

// void print_heap()
// {
// 	struct HEAP_BLOCK *ptr = _root_block;
// 	printf("=======Heap report=======\n");
// 	do
// 	{
// 		printf("Block Address: %p Size: %d bytes Status: %s\n", ptr, ptr->length, ptr->free ? "Free" : "Allocated");
// 		printf("\tPrevious:      %p\n\tNext:          %p\n\tNext Free      %p\n", ptr->previous, ptr->next, ptr->next_free);
// 		ptr = ptr->next;
// 	} while (ptr != NULL);
// }

// void *kmalloc(size_t size)
// {
// 	// Minimum allocated size is 8 bytes.
// 	uint64_t rem = size % 8;
// 	size -= rem;
// 	if (rem != 0)
// 	{
// 		size += 8;
// 	}

// 	// Find a block which is big enough for the requested size
// 	struct HEAP_BLOCK *block = _first_free_block;
// 	while (block != NULL && block->length < size)
// 	{
// 		block = block->next_free;
// 	}

// 	if (block == NULL)
// 	{
// 		// No sufficient length memory blocks are available
// 		printf("Aborting! Not enough memory left to malloc %d bytes", size);
// 		// TODO Abort or page more mem?
// 		return NULL;
// 	}

// 	// A free block was found which is exactly as big as needed.
// 	if (block->length == size)
// 	{
// 		block->free = 0;

// 		if (block == _first_free_block)
// 		{
// 			_first_free_block = block->next_free;
// 		}

// 		void *ptr = (void *)block + sizeof(struct HEAP_BLOCK);
// 		memset(ptr, 0, size);

// 		return ptr;
// 	}

// 	//
// 	// A free block was found but it needs subdivided. The extra length will be
// 	// "cleaved" off into a new block
// 	//

// 	// TODO check if we have enough space for a valid cleaved block(Don't overflow)

// 	struct HEAP_BLOCK *cleaved_block = ((void *)block) + sizeof(struct HEAP_BLOCK) + size;
// 	cleaved_block->length = block->length - (sizeof(struct HEAP_BLOCK) + size);
// 	cleaved_block->free = 1;
// 	cleaved_block->previous = block;
// 	cleaved_block->next = block->next;
// 	cleaved_block->next_free = block->next_free;

// 	if (cleaved_block->next != NULL)
// 	{
// 		cleaved_block->next->previous = cleaved_block;
// 	}

// 	block->length = size;
// 	block->free = 0;
// 	block->next = cleaved_block;
// 	block->next_free = cleaved_block;

// 	if (block->previous != NULL)
// 	{
// 		block->previous->next_free = cleaved_block;
// 	}

// 	if (cleaved_block < _first_free_block || block == _first_free_block)
// 	{
// 		_first_free_block = cleaved_block;
// 	}

// 	void *ptr = (void *)block + sizeof(struct HEAP_BLOCK);
// 	memset(ptr, 0, size);

// 	return ptr;
// }

// void kfree(void *ptr)
// {
// 	if (ptr == NULL)
// 	{
// 		printf("Aborting! Can't free a null pointer!"); // TODO proper abort
// 		return;
// 	}

// 	struct HEAP_BLOCK *block = ptr - sizeof(struct HEAP_BLOCK);
// 	block->free = 1;

// 	if (block < _first_free_block)
// 	{
// 		_first_free_block = block;
// 	}

// 	if (block->previous != NULL)
// 	{
// 		block->previous->next_free = block;
// 	}

// 	// Maybe absorb the next block if its free
// 	struct HEAP_BLOCK *next_block = block->next;
// 	if (next_block != NULL && next_block->free)
// 	{
// 		if (next_block->next != NULL)
// 		{
// 			next_block->next->previous = block;
// 		}

// 		block->length = block->length + sizeof(struct HEAP_BLOCK) + next_block->length;
// 		block->next = next_block->next;
// 		block->next_free = next_block->next_free;

// 		if (block < _first_free_block)
// 		{
// 			_first_free_block = block;
// 		}
// 	}

// 	// Maybe merge with previous block
// 	struct HEAP_BLOCK *previous_block = block->previous;
// 	if (previous_block != NULL && previous_block->free)
// 	{
// 		previous_block->length = previous_block->length + sizeof(struct HEAP_BLOCK) + block->length;
// 		previous_block->free = 1;
// 		previous_block->next = block->next;
// 		previous_block->next_free = block->next_free;

// 		if (block->next != NULL)
// 		{
// 			block->next->previous = previous_block;
// 		}

// 		if (previous_block < _first_free_block)
// 		{
// 			_first_free_block = previous_block;
// 		}
// 	}
// }

// void initialize_heap(void *address, size_t size)
// {
// 	_root_block = (struct HEAP_BLOCK *)address;

// 	_root_block->length = size - sizeof(struct HEAP_BLOCK);
// 	_root_block->free = 1;
// 	_root_block->next = NULL;
// 	_root_block->previous = NULL;
// 	_root_block->next_free = NULL;

// 	_first_free_block = _root_block;

// 	printf("Heap address: %p. Size: %'d bytes\n", address, size);

// 	print_heap();

// 	void *ptr1 = kmalloc(100);
// 	if (ptr1 == NULL)
// 		printf("Malloc error!\n");

// 	void *ptr2 = kmalloc(490);
// 	if (ptr2 == NULL)
// 		printf("Malloc error!\n");

// 	void *ptr3 = kmalloc(10);
// 	if (ptr3 == NULL)
// 		printf("Malloc error!\n");

// 	kfree(ptr2);

// 	ptr2 = kmalloc(490);
// 	if (ptr2 == NULL)
// 		printf("Malloc error!\n");

// 	// void *ptr4 = kmalloc(89);
// 	// if (ptr4 == NULL)
// 	// 	printf("Malloc error!\n");

// 	print_heap();

// 	kfree(ptr3);
// 	kfree(ptr1);
// 	kfree(ptr2);
// 	// kfree(ptr4);

// 	print_heap();
// }