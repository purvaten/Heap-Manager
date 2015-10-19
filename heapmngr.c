#include <stdlib.h>
#include "heapmngr.h"
#include "chunk.h"
#define MAX_SIZE	1000000	/* Units */
#define NUM_BINS	1024


/* INITIALLY ....................................................................*/

static Chunk_T HeapStart = NULL;
/* Starting address pointer of avalilable memory set to NULL */

static Chunk_T HeapEnd = NULL;
/* Ending address pointer of avaliable memory set to NULL. */

static Chunk_T freebinArray[NUM_BINS];
/* Array of bins to doubly linked NUL terminated free lists of different bin sizes */

/* ................................................................................ */

int findBin(size_t Units)
/* Returns correct bin size */
{
	if (Units > (NUM_BINS - 1))
		return NUM_BINS - 1;
	return (int)Units;
}

void removefromList(Chunk_T chunk_ptr)
/* Removes the chunk from the linked structure by adjusting next and prev in list */
{
	Chunk_T nextinList, previnList;
	size_t chunk_ptr_val = Chunk_getUnits(chunk_ptr);

	previnList = Chunk_getPrevInList(chunk_ptr);
	nextinList = Chunk_getNextInList(chunk_ptr);
	if (previnList != NULL)
		Chunk_setNextInList(previnList, nextinList);
	else
		freebinArray[chunk_ptr_val] = nextinList;
	if (nextinList != NULL)
		Chunk_setPrevInList(nextinList, previnList);
}

void InsertinBin(Chunk_T chunk_ptr, size_t chunk_size)
/* Inserts chunk at beginning of link structure in respective bin */
{	
	Chunk_T binptr;

	if (chunk_size > NUM_BINS - 1)
		binptr = freebinArray[NUM_BINS - 1];
	else
		binptr = freebinArray[chunk_size];

	Chunk_setNextInList(chunk_ptr, binptr);
	freebinArray[chunk_size] = chunk_ptr;
}


Chunk_T useChunk(Chunk_T chunk_ptr, size_t Units)
/* Uses the chunk. If chunk size is close to Units then remove the chunk from free list and return it.
	If chunk size is too big split it and make re arrange link structure
	and return pointer to correct memory location slice */
{	
	size_t chunk_ptr_val = Chunk_getUnits(chunk_ptr);
	size_t splitchunk_size = chunk_ptr_val - Units;
	Chunk_T temp_ptr, splitchunk;

	if (chunk_ptr_val < Units + MIN_UNITS_PER_CHUNK) {
	/* Allocate all the memory */
		/* Update status */
		Chunk_setStatus(chunk_ptr, CHUNK_INUSE);

		/* Removing chunk_ptr from list */
		removefromList(chunk_ptr);

		return chunk_ptr;
	}

	/* Split the chunk pointed by chunk_ptr */
	temp_ptr = chunk_ptr;
	removefromList(chunk_ptr);
	
	Chunk_setUnits(temp_ptr, Units);
	Chunk_setStatus(temp_ptr, CHUNK_INUSE);

	splitchunk = (Chunk_T)((char *)temp_ptr + Units);
	Chunk_setUnits(splitchunk, splitchunk_size);
	Chunk_setStatus(splitchunk, CHUNK_FREE);

	/* Depending on size of the split chunk, insert it in correct bin */
	InsertinBin(splitchunk, splitchunk_size);

	return temp_ptr;

}

Chunk_T getmoreMemory(Chunk_T chunk_ptr, size_t Units) {
	return NULL;
}

/* .................................................................................. */

void *my_malloc(size_t size)
/* Allocate numbytes of memory from the free memory pool and return a pointer to the base of the newly allocated region. 
	Returns NULL if size is zero or if memory allocation failed*/
{	
	size_t UnitSize = Chunk_getUnitSize();
	size_t Units;
	int ibin;
	Chunk_T chunk_ptr;

	if (size == 0)
		return NULL;

	/* Initialize if this is the first call */
	if (HeapStart == NULL) {
		HeapStart = (Chunk_T) malloc(MAX_SIZE * UnitSize);
		/* Place this chunk in last bin */
		InsertinBin(HeapStart, MAX_SIZE);
		HeapEnd = HeapStart;
	}

	/* Determine the number of units the new chunk should contain */
	Units = ((size - 1) / UnitSize) + 1;
	Units = Units + 2;	/* For Header and Footer */

	/* Traverse through the array and look if bin of required size is available.
		If it is, allocate it. If not, Check larger bins. */
	for (ibin = findBin(Units); ibin < NUM_BINS; ibin++) {
		chunk_ptr = freebinArray[ibin];

		/* Traverse the link structure starting from index ibin */
		while (chunk_ptr != NULL) {
			if (Chunk_getUnits(chunk_ptr) >= Units) {
				chunk_ptr = useChunk(chunk_ptr, Units);

				return (void *)((char *)chunk_ptr + UnitSize);
			}
			chunk_ptr = Chunk_getNextInList(chunk_ptr);
		}
	}

	/* Required memory is not found. Obtain new memory by doing malloc() */
	chunk_ptr = getmoreMemory(chunk_ptr, Units);
	if (chunk_ptr == NULL)
	/* malloc failed */
		return NULL;

	/* Chunk is available for use */
	chunk_ptr = useChunk(chunk_ptr, Units);
	return (void *)((char *)chunk_ptr + UnitSize);
}

/* ................................................................................ */

int my_free(void *region)
/* Free a previously allocated region. */
{
	return 0;
}