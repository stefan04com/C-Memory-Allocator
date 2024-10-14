#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "sfl_structures.h"
#include "sfl_functions.c"

int main(void)
{
	// declare and initialize needed variables
	calls_t calls;
	calls.nr_mallocs = 0;
	calls.nr_frees = 0;
	calls.nr_frag = 0;

	while (1) {
		char command[15], date[585];
		size_t start_addr = 0x0, addr = 0x0;
		int nr_list = 0, nr_bytes_list = 0, reconstruct_type = 0, nr_bytes = 0;

		heap_t *heap;
		allc_list_t *allc_list;

// 		read the command

		scanf("%s", command);

		if (strcmp(command, "INIT_HEAP") == 0) {
// 			read the parameters for the heap
			scanf("%zx %d", &start_addr, &nr_list);
			scanf("%d %d", &nr_bytes_list, &reconstruct_type);
			heap = malloc(sizeof(heap_t));
			allc_list = malloc(sizeof(allc_list_t));
			allc_list->head = NULL;
			allc_list->nr_blocks = 0;
			allc_list->total_size = 0;
			heap->start_addr = start_addr;
			heap->nr_list = nr_list;
			heap->nr_bytes_list = nr_bytes_list;
			heap->reconstruct_type = reconstruct_type;
			heap->sfl = init_heap(start_addr, nr_list, nr_bytes_list);
		}

		if (strcmp(command, "MALLOC") == 0) {
// allocate a block
			scanf("%d", &nr_bytes);
			my_malloc(heap, allc_list, nr_bytes, &calls);
		}

		if (strcmp(command, "FREE") == 0) {
// free a block
			scanf("%zx", &addr);
			if (addr != 0x0)
				my_free(heap, allc_list, addr, &calls);
		}

		if (strcmp(command, "READ") == 0) {
// read from a block
			scanf("%zx %d", &addr, &nr_bytes);
			read(allc_list, addr, nr_bytes, &calls, heap);
		}

		if (strcmp(command, "WRITE") == 0) {
// write to a block
			scanf("%zx", &addr);
			fgets(date, 585, stdin);
// obtain the text from "" and the number of bytes
			char *first = strchr(date, '"');
			char *last = strrchr(date, '"');
			int len = last - first - 1;
			sscanf(last + 1, "%d", &nr_bytes);
			char *text = (char *)malloc((len + 1) * sizeof(char));
			strncpy(text, first + 1, len);
			text[len] = '\0';
			write(allc_list, addr, text, nr_bytes, &calls, heap);
			free(text);
		}
// 		print the memory
		if (strcmp(command, "DUMP_MEMORY") == 0)
			dump_memory(*heap, calls, *allc_list);
// 		free structures used
		if (strcmp(command, "DESTROY_HEAP") == 0) {
			destroy(heap, allc_list);
			break;
		}
	}
	return 0;
}
