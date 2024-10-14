#include <stdio.h>
#include <stdlib.h>
#include "sfl_structures.h"

// sort the array of list

void sort_heap_sfl(heap_t *heap)
{
	for (int i = 0; i < heap->nr_list - 1; i++) {
		for (int j = 0; j < heap->nr_list - i - 1; j++) {
			if (heap->sfl[j]->block_size > heap->sfl[j + 1]->block_size) {
				fl_ddl_t *temp = heap->sfl[j];
				heap->sfl[j] = heap->sfl[j + 1];
				heap->sfl[j + 1] = temp;
			}
		}
	}
}

// add a new node to the allocated list

void add_allc_node(allc_list_t *list, size_t addr, int size)
{
	allc_block *new_block = malloc(sizeof(allc_block));
	new_block->data = malloc(size);
	new_block->start_addr = addr;
	new_block->size = size;
	list->total_size += size;
	if (!list->head) {
		new_block->next = NULL;
		new_block->prev = NULL;
	list->head = new_block;
	} else {
		allc_block *current = list->head;
		allc_block *prev = NULL;
	while (current && current->start_addr < new_block->start_addr) {
		prev = current;
		current = current->next;
	}
	new_block->next = current;
	new_block->prev = prev;
	if (prev) {
		prev->next = new_block;
	} else {
		list->head = new_block;
	}
	if (current) {
		current->prev = new_block;
	}
	}
}

// create a new free list

fl_ddl_t *
dll_create_fl()
{
	fl_ddl_t *list = (fl_ddl_t *)malloc(sizeof(fl_ddl_t));
	list->head = NULL;
	list->size = 0;
	list->nr_blocks = 0;
	return list;
}

// add a new node to the free list

void add_fl_node(fl_ddl_t *list, size_t addr, int size)
{
	fl_node_t *new_node = (fl_node_t *)malloc(sizeof(fl_node_t));
	new_node->start_addr = addr;
	new_node->next = NULL;
	new_node->size = size;
	if (!list->head) {
		new_node->next = NULL;
		new_node->prev = NULL;
		list->head = new_node;
	} else {
		fl_node_t *current = list->head;
	fl_node_t *prev = NULL;
	while (current && current->start_addr < new_node->start_addr) {
		prev = current;
		current = current->next;
	}
	new_node->next = current;
	new_node->prev = prev;
	if (prev) {
		prev->next = new_node;
	} else {
		list->head = new_node;
	}
	if (current) {
		current->prev = new_node;
	}
	}
	list->size++;
}

// function to initialize the heap

fl_ddl_t **init_heap(size_t start_addr, unsigned int nr_list, unsigned int nr_bytes_list)
{
	fl_ddl_t **sfl = (fl_ddl_t **)malloc(nr_list * sizeof(fl_ddl_t *));
	int block = 8;
	for (int i = 0; i < (int)nr_list; i++) {
		sfl[i] = dll_create_fl();
		sfl[i]->block_size = block;
		block *= 2;
	}
	for (int i = 0; i < (int)nr_list; i++) {
		for (int j = 0; j < (int)(nr_bytes_list / sfl[i]->block_size); j++) {
			add_fl_node(sfl[i], start_addr, sfl[i]->block_size);
			sfl[i]->nr_blocks++;
			start_addr += sfl[i]->block_size;
		}
	}
	return sfl;
}

// function to free structures used

void destroy(heap_t *heap, allc_list_t *allc_list)
{
	for (int i = 0; i < heap->nr_list; i++) {
		fl_ddl_t *sfl = heap->sfl[i];
		fl_node_t *current = sfl->head;
		while (current != NULL) {
			fl_node_t *next = current->next;
			free(current);
			current = next;
		}
		free(sfl);
	}
	free(heap->sfl);
	free(heap);
	allc_block *current = allc_list->head;
	while (current != NULL) {
		allc_block *next = current->next;
		free(current->data);
		free(current);
		current = next;
	}
	free(allc_list);
}

// function to take block from the free list and add it to the allocated list

void my_malloc(heap_t *heap, allc_list_t *allc_list, int nr_bytes, calls_t *calls)
{
	int ok = 0;
	for (int i = 0; i < heap->nr_list; i++) {
// iterate through the free list to find a block that fits the size
		if (heap->sfl[i]->block_size >= (unsigned int)nr_bytes && heap->sfl[i]->size > 0) {
			fl_node_t *current = heap->sfl[i]->head;
		while (current) {
			if (current->size >= nr_bytes) {
				add_allc_node(allc_list, current->start_addr, nr_bytes);
				calls->nr_mallocs++;
				ok = 1;
			if (current->size > nr_bytes) {
				calls->nr_frag++;
				fl_node_t *remaining = malloc(sizeof(fl_node_t));
				remaining->start_addr = current->start_addr + nr_bytes;
				remaining->size = current->size - nr_bytes;
				remaining->next = NULL;
				remaining->prev = NULL;
				int j;
				for (j = 0; j < heap->nr_list; j++) {
					if (heap->sfl[j]->block_size == (unsigned int)remaining->size) {
						add_fl_node(heap->sfl[j], remaining->start_addr, remaining->size);
						heap->sfl[j]->nr_blocks++;
						break;
					}
				}
				if (j == heap->nr_list) {
// create a new free list
					heap->nr_list++;
					heap->sfl = realloc(heap->sfl, heap->nr_list * sizeof(fl_ddl_t *));
					heap->sfl[j] = dll_create_fl();
					heap->sfl[j]->block_size = remaining->size;
					add_fl_node(heap->sfl[j], remaining->start_addr, remaining->size);
					heap->sfl[j]->nr_blocks = 1;
				}
// update the free list
				if (current->prev) {
					current->prev->next = current->next;
				} else {
					heap->sfl[i]->head = current->next;
				}
				if (current->next) {
					current->next->prev = current->prev;
				}
				free(current);
				heap->sfl[i]->nr_blocks--;
				if (heap->sfl[i]->nr_blocks == 0)
					heap->sfl[i]->head = NULL;
				sort_heap_sfl(heap);
				allc_list->nr_blocks++;
				free(remaining);
				} else {
				if (!current->prev) {
					heap->sfl[i]->head = current->next;
				} else {
					current->prev->next = current->next;
				}
				if (current->next) {
					current->next->prev = current->prev;
				}
				free(current);
				heap->sfl[i]->nr_blocks--;
				if (heap->sfl[i]->nr_blocks == 0)
					heap->sfl[i]->head = NULL;
				allc_list->nr_blocks++;
				}
				break;
			}
		current = current->next;
		}
		if (ok)
			break;
		}
	}
	if (!ok)
		printf("Out of memory\n");
}

// check if the address is a initial address of a block

int check_initial_addr(heap_t *heap, size_t addr, int size)
{
	int block = 8;
	size_t start_addr = heap->start_addr;
	for (int i = 0; i < heap->nr_list; i++) {
		for(int j = 0; j < heap->nr_bytes_list / block; j++) {
			if (start_addr == addr) {
				if (block == size)
					return 1;
				else
					return 0;
			}
			start_addr += block;
		}
		block *= 2;
	}
	return 0;
}

// function to move a block from the allocated list to the free list

void my_free(heap_t *heap, allc_list_t *allc_list, size_t addr, calls_t *calls)
{
	if (!allc_list->head) {
		printf("Invalid free\n");
		return;
	}
// iterate through the allocated list to find the block
	allc_block *current = allc_list->head;
	current->prev = NULL;
	int ok = 0;
	while (current) {
		if (current->start_addr == addr) {
			ok = 1;
			break;
		}
		current = current->next;
	}
	if (!ok)
		printf("Invalid free\n");
	else {
		calls->nr_frees++;
		allc_list->total_size -= current->size;
		if (current == allc_list->head)
			allc_list->head = current->next;
		if (current->prev)
			current->prev->next = current->next;
		if (current->next)
			current->next->prev = current->prev;
		current->next = NULL;
		if (heap->reconstruct_type == 0) {
			int i;
			for (i = 0; i < heap->nr_list; i++) {
				if (heap->sfl[i]->block_size == (unsigned int)current->size) {
					add_fl_node(heap->sfl[i], current->start_addr, current->size);
					heap->sfl[i]->nr_blocks++;
					break;
				}
			}
			if (i == heap->nr_list) {
// create a new free list
				heap->nr_list++;
				heap->sfl = realloc(heap->sfl, heap->nr_list * sizeof(fl_ddl_t *));
				heap->sfl[i] = dll_create_fl();
				heap->sfl[i]->block_size = current->size;
				add_fl_node(heap->sfl[i], current->start_addr, current->size);
				heap->sfl[i]->nr_blocks = 1;
			}
			allc_list->nr_blocks--;
			if (allc_list->nr_blocks == 0)
				allc_list->head = NULL;
			sort_heap_sfl(heap);
			free(current->data);
			free(current);
		} else {
// reconstruct_type == 1
			int frag = 1;
			while(frag) {
// iterate through the free list to check for blocks that can be merged
				frag = 0;
				for( int i = 0; i < heap->nr_list; i++) {
					fl_node_t *aux = heap->sfl[i]->head;
					int ok = 0;
					while(aux) {
						if ((current->start_addr - heap->start_addr) / heap->nr_bytes_list == (aux->start_addr - heap->start_addr) / heap->nr_bytes_list)
						{
							if(!check_initial_addr(heap, aux->start_addr, aux->size) && !check_initial_addr(heap, current->start_addr, current->size))
							if((aux->start_addr + aux->size == current->start_addr) || (current->start_addr + current->size == aux->start_addr)) {
								// update current block with merged block
								size_t new_start = aux->start_addr < current->start_addr ? aux->start_addr : current->start_addr;
								int new_size = aux->size + current->size;
								current->start_addr = new_start;
								current->size = new_size;
								if (aux->prev) {
									aux->prev->next = aux->next;
								} else {
									heap->sfl[i]->head = aux->next;
								}
								if (aux->next) {
									aux->next->prev = aux->prev;
								}
								free(aux);
								heap->sfl[i]->nr_blocks--;
								if (heap->sfl[i]->nr_blocks == 0)
									heap->sfl[i]->head = NULL;
								frag = 1;
								ok = 1;
							} 	
						}
						if (ok)
							break;
						aux = aux->next;
					}
				}
				}
// add the block to the free list
				int i;
			for (i = 0; i < heap->nr_list; i++) {
				if (heap->sfl[i]->block_size == (unsigned int)current->size) {
					add_fl_node(heap->sfl[i], current->start_addr, current->size);
					heap->sfl[i]->nr_blocks++;
					break;
				}
			}
			if (i == heap->nr_list) {
// create a new free list
				heap->nr_list++;
				heap->sfl = realloc(heap->sfl, heap->nr_list * sizeof(fl_ddl_t *));
				heap->sfl[i] = dll_create_fl();
				heap->sfl[i]->block_size = current->size;
				add_fl_node(heap->sfl[i], current->start_addr, current->size);
				heap->sfl[i]->nr_blocks = 1;
			}
			sort_heap_sfl(heap);
			free(current->data);
			free(current);
			
			allc_list->nr_blocks--;
			if (allc_list->nr_blocks == 0)
				allc_list->head = NULL;
		}
	}
}

// function to print the memory

void dump_memory(heap_t heap, calls_t calls, allc_list_t allc_list)
{
	int free_blocks = 0, free_memory = 0;
	for (int i = 0; i < heap.nr_list; i++) {
		free_blocks += heap.sfl[i]->nr_blocks;
		free_memory += heap.sfl[i]->nr_blocks * heap.sfl[i]->block_size;
	}
	printf("+++++DUMP+++++\n");
	printf("Total memory: %d bytes\n", free_memory + allc_list.total_size);
	printf("Total allocated memory: %d bytes\n", allc_list.total_size);
	printf("Total free memory: %d bytes\n", free_memory);
	printf("Free blocks: %d\n", free_blocks);
	printf("Number of allocated blocks: %d\n", allc_list.nr_blocks);
	printf("Number of malloc calls: %d\n", calls.nr_mallocs);
	printf("Number of fragmentations: %d\n", calls.nr_frag);
	printf("Number of free calls: %d\n", calls.nr_frees);
	for (int i = 0; i < heap.nr_list; i++) {
		if (heap.sfl[i]->nr_blocks != 0)
			printf("Blocks with %d bytes - %d free block(s) : ", heap.sfl[i]->block_size, heap.sfl[i]->nr_blocks);
		fl_node_t *current = heap.sfl[i]->head;
		while (current) {
			if (current->next)
				printf("0x%zx ", current->start_addr);
			else
				printf("0x%zx", current->start_addr);
			current = current->next;
		}
		if (heap.sfl[i]->nr_blocks != 0)
		printf("\n");
	}
	printf("Allocated blocks :");
	allc_block *current = allc_list.head;
	if (allc_list.head)
		printf(" ");
	while (current) {
		if (current->next)
			printf("(0x%zx - %d) ", current->start_addr, current->size);
		else
			printf("(0x%zx - %d)", current->start_addr, current->size);
		current = current->next;
	}
	printf("\n");
	printf("-----DUMP-----\n");
	free(current);
}

// function to write to a memory block

void write(allc_list_t *allc_list, size_t addr, char *text, int nr_bytes, calls_t *calls, heap_t *heap)
{
	if ((int)strlen(text) < nr_bytes)
	nr_bytes = strlen(text);
	size_t final_addr = addr + nr_bytes;
	allc_block *current = allc_list->head;
	allc_block *start = NULL;
	int ok = 0;
//check for enough memory
	while (current) {
		if (current->start_addr <= addr && current->start_addr + current->size > addr) {
			if (!ok) {
				start = current;
				ok = 1;
			}
			allc_block *next = current->next;
			while (next && next->start_addr == current->start_addr + current->size) {	
				current = next;
				next = next->next;
			}
			if (current && current->start_addr + current->size < final_addr) {
				printf("Segmentation fault (core dumped)\n");
			dump_memory(*heap, *calls, *allc_list);
			destroy(heap, allc_list);
			exit(0);
			return;
			}
		}
		current = current->next;
	}
//check valid address
	if (!ok) {
		printf("Segmentation fault (core dumped)\n");
		dump_memory(*heap, *calls, *allc_list);
		destroy(heap, allc_list);
		exit(0);
		return;
	}
//write data
	current = start;
	int remaining;
	int len = strlen(text);
	remaining = len < nr_bytes ? len : nr_bytes;
	while (current) {
	if (current->start_addr <= addr && current->start_addr + current->size > addr) {
	while (current && remaining > 0) {
		int available = current->size - (addr - current->start_addr);
		int to_write = remaining < available ? remaining : available;
		memcpy(current->data + (addr - current->start_addr), text, to_write);
		remaining -= to_write;
		addr += to_write;
		text += to_write;
		current = current->next;
	}
	break;
	}
	current = current->next;
	}
}

// function to read from a memory block

void read(allc_list_t *allc_list, size_t addr, int nr_bytes, calls_t *calls, heap_t *heap)
{
	size_t final_addr = addr + nr_bytes;
	allc_block *current = allc_list->head;
	allc_block *start = current;
	int ok = 0;
//check for enough memory
	while (current) {
		if (current->start_addr <= addr && current->start_addr + current->size > addr) {
			if (!ok) {
				start = current;
				ok = 1;
			}
			allc_block *next = current->next;
			while (next && next->start_addr == current->start_addr + current->size) {		
				current = next;
				next = next->next;
			}
			if (current && current->start_addr + current->size < final_addr) {
				printf("Segmentation fault (core dumped)\n");
			dump_memory(*heap, *calls, *allc_list);
			destroy(heap, allc_list);
			exit(0);
			return;
			}
		}
		current = current->next;
	}
//check valid address
	if (!ok) {
		printf("Segmentation fault (core dumped)\n");
		dump_memory(*heap, *calls, *allc_list);
		destroy(heap, allc_list);
		exit(0);
		return;
	}
//print data
	current = start;
	while (current) {
	if (current->start_addr <= addr && (current->start_addr + current->size) > addr) {
	while (current && nr_bytes > 0) {
		int available = current->size - (addr - current->start_addr);
		int to_read = nr_bytes < available ? nr_bytes : available;
		printf("%.*s", to_read, (char *)current->data + (addr - current->start_addr));
		nr_bytes -= to_read;
		addr += to_read;
		current = current->next;
	}
	break;
	}
	current = current->next;
	}
	printf("\n");
}