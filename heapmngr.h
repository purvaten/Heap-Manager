#include <stdlib.h>

void *my_malloc(size_t numbytes);
/* Allocate numbytes of memory from the free memory pool and return a pointer to the base of the newly allocated region. 
	Returns NULL if size is zero or if memory allocation failed */

void my_free(void *region);
/* Free a previously allocated region. Region points to a region allocated by my_malloc().
	If region is NULL do nothing */

void *my_realloc(void *ptr, size_t size);
/* Resize the memory block pointed to by ptr that was previously allocated with a call to my_malloc or my_calloc.
	Returns a pointer to the newly allocated memory, or NULL if the request fails. */

void *my_calloc(size_t nitems, size_t size);
/* Allocate the requested memory, initialize it to zero and returns a pointer to the beginning of allocated region.
	Returns NULL if memory allocation failed */