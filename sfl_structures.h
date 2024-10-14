
#ifndef STRUCTURI_H
#define STRUCTURI_H

// structures for the heap

typedef struct fl_node_t fl_node_t;
struct fl_node_t {
	size_t start_addr;
	int size;
	fl_node_t *prev, *next;
};

typedef struct fl_ddl_t fl_ddl_t;
struct fl_ddl_t {
	fl_node_t *head;
	unsigned int block_size;
	unsigned int size;
	unsigned int nr_blocks;
};

typedef struct heap_t heap_t;
struct heap_t {
	size_t start_addr;
	int nr_list;
	int nr_bytes_list;
	int reconstruct_type;
	fl_ddl_t **sfl;
};

// structures for the allocated list

typedef struct allc_block {
	void *data;
	size_t start_addr;
	int size;
	struct allc_block *next, *prev;
} allc_block;

typedef struct allc_list {
	allc_block *head;
	int nr_blocks;
	int total_size;
} allc_list_t;

// structures for the calls

typedef struct calls {
	int nr_mallocs;
	int nr_frees;
	int nr_frag;
} calls_t;

#endif