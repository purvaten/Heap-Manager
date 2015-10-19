void *my_malloc(size_t numbytes);
/* Allocate numbytes of memory from the free memory pool and return a pointer to the base of the newly allocated region. 
	Returns NULL if size is zero or if memory allocation failed*/

int my_free(void *region);
/* Free a previously allocated region. */