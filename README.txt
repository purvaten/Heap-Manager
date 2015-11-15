TITLE : Heap Manager
AUTHOR : Purva Tendulkar
MIS : 111403049
DESCRIPTION :

The project is an implementation of Heap Memory Manager which includes my_malloc, my_free, my_calloc and my_realloc functions.
It is an attempt at improving K&R's implementation of Dynamic Memory Management as given in section 8.7 (a Storage Allocator)

Provides a Faster free by simply inserting in an array of free memory chunks. No search for placement is required.

Provides a Faster malloc by directly jumping to required index of the array and using the the chunk if it is available, else go to 
the further indices where chunk of greater size is available.

Calloc and realloc are implemented using malloc and free.

Debugging has been done extensively using functions like HeapisValid and ChunkisValid. Also gdb (Debugger) was used. 
Testing has been done using simple test cases to test each function separately. Extensive testing has been done by calling malloc 
and free in random order with random allocation sizes.