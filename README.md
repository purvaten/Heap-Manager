# HEAP MEMORY MANAGER  

.................................................................................................................................

## What does the program do?  

>> Manage Heap memory and improve on K&R's implementation of Dynamic Memory Management  
	as described in Section 8.7 (A Storage Allocator)  
>> Implement my_malloc() and my_free() functions as improved versions malloc() and free()  

.................................................................................................................................

## Purpose  

>> Getting a better understanding of how Dynamic Memory Management actually occurs  

.................................................................................................................................

## K&R implementation  

>> ### Free list  
	– Free block with header (pointer and size) and user data  
	– Align the header with the largest data type  
	– Circular singly linked list of free blocks arranged in order of increasing memory address  
>> ### Malloc  
	– Allocate memory in multiples of header size  
	– Find the first element in the free list that is large enough (first fit)  
	– If block is larger than required, split it  
	– Allocate more memory, if block of required size is not found  
>> ### Free  
	– Put a block back in the free list  
	– Coalesce with adjacent blocks on either side, if any  

.................................................................................................................................

## Limitations of K&R implementation  

>> Malloc requires scanning the free list (linear time)  
	– To find the first free block that is big enough  
	– Block found may still not be best fit  

>> Free requires scanning the free list (linear time)  
	– To find the location to insert the to-be-freed block  

>> Large amount of Fragmentation  
	– Decrease in efficeincy  
	– Reduction in probability of getting next blocks of exact size  

.................................................................................................................................

## Improvements brought about by Program  

>> Split only if the split chunk is large enough to meet smallest my_malloc() request (1 byte)  
>> Faster free (constant time)  
	– Size information in both header and footer  
	– Next and previous free-list pointers in header and footer  
	– Free block is directly placed at beginning of linked structre in appropriate bin  
>> Faster malloc (constant time)  
	– Separate free list for free chunks of different sizes  
	– One bin per chunk size (except last bin)  
	– One bin for a range of sizes (only for last bin)  

.................................................................................................................................

## Data Structures used  

>> Data is being stored in 2 main data structures :  
	– Chunk structure  
	– Free Bin Array with Link List  

>> Chunk structure :  

>> Memory is stored in the form of a chunk which has 3 main parts –  
	– Header  
	– Actual memory as requested by user   
	– Footer  

>> Header has following data –  
	- Size of the chunk (header size, footer size and actual memory size)  
	- Status of the chunk (whether free or in use)  
	- Pointer to address of adjacent chunk (next chunk in the link structure)  
>> Actual memory –  
	- Stored in multiples of UnitSize (size of header/footer)  
>> Footer has following data –  
	- Size of the chunk (header size, footer size and actual memory size)  

>> Free Bin Array with Link List :  

	– We maintain a fixed size array where each index represents a bin with a pointer to a NULL terminitaed linked list  
	– This is a linked list of chunks of the same size  
	– If size of a certain chunk exceeds max size of array it is placed in the last bin  


>> Use of Footer  

>> Size of previous chunk in memory is easy accessible due to Footer due to following –  
	- Size of header/footer is UnitSize and constant  
	- Footer stores information regarding size of chunk  
	- Chunks in free list can be maintained in any order (independent of memory address)  

.................................................................................................................................

## Implementation Details  

>> Initially  
	– Large chunk of memory is allocated using malloc()  
		(K&R uses sbrk() and brk(). I am not using OS calls because I am not familiar with them)  
	– Pointers are kept at beginning and end of this memory chunk. (HeapStart and HeapEnd)  
		They are used only to validate that memory allocated lies between those 2 ends  
	– Free list array bin pointers set to NULL (using static allocation)  

>> When my_malloc() is called  
	– Jumps to index in free list to check for availibility of chunk of that size  
	– If not found, goes to next index (and this continues) until block of exact or greater size is found  
	– If last bin is reached scans entire bin and if required size is still not available requests more memory using malloc()  
	– Splits the chunk only if size obtained after splitting is greater than or equal to minimum size allowed for a block  
	– Remove split chunk from old location and place in new array index depending on size of the split chunk  
		- Check if chunks at previous and next memory locations are free  
		- If so, remove them from their location, coalesce with current chunks and place in new location in free array  
	– Always insert at beginning of link structure for particular bin in array (constant time)  
	– Return pointer to the other part of split chunk to user and set its status as "being used"  

>> When my_free() is called  
	– Status of the chunk is changed to "free"  
	– Chunk is placed in appropriate bin in free list array  
		- Check if chunks at previous and next memory locations are free  
		- If so, remove them from their location and coalesce with current chunks and place in new location in free array  

.................................................................................................................................

## Debugging  

>> Following methods have been used for debugging the program :  
	– HeapMgr_isValid()		code written  
	– Chunk_isValid()		code written  
	– PrintBin()			code written  
	– PrintMemory()			code written  
	– assert()				assert.h included  
	– gdb (debugger)		gdb installed  

>> HeapMgr_isValid()   

>> Checks the following –  
	– Check to make sure the first MIN_UNITS_PER_CHUNK bins do not contain anything   
	– Check if all chunks are valid  
	– Check to make sure there are no adjacent free chunks  
	– Check if each foward link is matched with the correct backwards link  
	– Check if each chunk in the bin is of the right size  
	– Checks if each chunk in the bin is set to free  
	– Make sure all free chunks are in the free list  
	– Check if the free list in each bin is a complete loop (same number of bins while traversing back and forth)  

>> Chunk_isValid() –  

>> Checks the following –  
	– Address of Chunk should not be less than HeapStart    
	– Address of Chunk should not be greater than or equal to HeapEnd   
	– Size of Chunk should not be equal to zero  
	– SIze of chunk should not be less than MIN_UNITS_PER_CHUNK   
	– Address of chunk + size should not exceed HeapEnd  
	– Size of the chunk as stored in footer should be same as size stored in header  

>> PrintBin() –  

	– Prints link structure of all bins which are not empty i.e. size of all the chunks for a given bin   

>> PrintMemory() –  

	– Prints the size and status of all the chunks in physical memory from HeapStart to HeapEnd  

.................................................................................................................................

## Testing  

>>Two test conditions are provided :  
	– Fixed Simple test case  
	– Random test case  

>> Fixed Simple test case : 
	– Used primarily during initial testing   
	– my_malloc() and my_free() are implemented by hardcoding values  
	– Gives a simple idea about how functions work  
	– Easy for debugging  

>> Random test case :  
	– This is used after major bugs found during simple testing were fixed  
	– Asks user for number of chunks to be generated and maximum size a chunk can have  
	– Randomly allocates memory using my_malloc()  
	– If random size repeats itself in next iteration free that chunk using my_free()  
		after checking that its contents haven't been corrupted  
	– After all allocations are done, free them all using my_free() if they haven't already been freed  

.................................................................................................................................

## How to run the program  

>> Run following on terminal :  
make  
./try  

.................................................................................................................................