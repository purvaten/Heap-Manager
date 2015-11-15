/*****************************************************************************
 * Copyright (C) Purva Tendulkar purva.tendulkar@gmail.com
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include "heapmngr.h"
#include "chunk.h"
#define MAX_SIZE			1024	/* Units */
#define NUM_BINS			1024
#define MIN_UNITS_FROM_OS	1024

/* INITIALLY ....................................................................*/

static Chunk_T HeapStart = NULL;
/* Starting address pointer of avalilable memory set to NULL */

static Chunk_T HeapEnd = NULL;
/* Ending address pointer of avaliable memory set to NULL */

static Chunk_T freebinArray[NUM_BINS];
/* Array of bins to doubly linked NUL terminated free lists of different bin sizes */

/* ................................................................................ */

/* TESTING PURPOSES............................................................ */

void PrintBin()

/* Prints entire link structure of all bins which are non empty */

{
	int i;
	Chunk_T ptr;

	for (i = 0; i < NUM_BINS; i++) {
		ptr = freebinArray[i];
		while (ptr != NULL) {
			printf("i : %d, size : %d\n", i, (int)Chunk_getUnits(ptr));
			ptr = Chunk_getNextInList(ptr);
		}
	}
}

void PrintMemory()

/* Prints all chunks in physical memory from HeapStart to HeapEnd
   i.e. their size and status */

{
   Chunk_T ChunkPtr = HeapStart;

  	while (ChunkPtr != HeapEnd && ChunkPtr != NULL) {
   		if (Chunk_getStatus(ChunkPtr) == CHUNK_FREE)
      		printf("\tSTATUS : FREE  , ");
    	else
    		printf("\tSTATUS : IN USE, ");

    	printf("SIZE : %d Chunks\n", (int)Chunk_getUnits(ChunkPtr));
    	ChunkPtr = Chunk_getNextInMem(ChunkPtr, HeapEnd);
    }
}
/*--------------------------------------------------------------------*/

int HeapMgr_isValid()

/* Return 1 (TRUE) if the heap manager is in a valid state, or
   0 (FALSE) otherwise. */

{
	int iBin;
	Chunk_T Chunk;
	Chunk_T NextList;
	Chunk_T NextMem;

	int i = 0;

	Chunk_T MemChunk;
	Chunk_T ListChunk;

	if (HeapStart == NULL) {
		fprintf(stderr, "Uninitialized heap start\n"); return 0;
	}
	if (HeapEnd == NULL) {
		fprintf(stderr, "Uninitialized heap end\n"); return 0;
	}

	if (HeapStart == HeapEnd) {
		for (iBin = 0; iBin < NUM_BINS; iBin++) {
			if (freebinArray[iBin] != NULL) {
				fprintf(stderr, "Inconsistent empty heap\n");
				return 0;
			}
		}
		return 1;
	}

	/* Check to make sure the first MIN_UNITS_PER_CHUNK bins do not contain anything */
	for (iBin = 0; iBin < MIN_UNITS_PER_CHUNK; iBin++) {
		if (freebinArray[iBin] != NULL) {
			fprintf(stderr, "Chunks placed in too small bins, Bin = %d\n", iBin);
	        return 0;
	    }
	}

	/* Check to make sure the first free Chunk in each bin has a null prev list chunk */
	for (iBin = 0; iBin < NUM_BINS; iBin++) {
		Chunk = freebinArray[iBin];
		if (Chunk != NULL) {
			if (Chunk_getPrevInList(Chunk) != NULL) {
        		fprintf(stderr, "First Free Chunk has Faulty backwards pointer, ""Bin = %d\n", iBin);
        		return 0;
        	}
        }
    }

    /* Check if all chunks are valid */
    /* We already know that oHeapStart != oHeapEnd */
    Chunk = HeapStart;
    assert(Chunk_isValid(Chunk, HeapStart, HeapEnd));

    Chunk = Chunk_getNextInMem(Chunk, HeapEnd);
    while (Chunk != NULL) {
   		assert(Chunk_isValid(Chunk, HeapStart, HeapEnd));
   		Chunk = Chunk_getNextInMem(Chunk, HeapEnd);
   	}

   	/* Check to make sure there are no adjacent free chunks */
   	/* We already know that oHeapStart != oHeapEnd */
   	Chunk = HeapStart;
   	NextMem = Chunk_getNextInMem(Chunk, HeapEnd);

   	while (NextMem != NULL) {
   		if (Chunk_getStatus(Chunk) == CHUNK_FREE && Chunk_getStatus(NextMem) == CHUNK_FREE) {
   			fprintf(stderr, "Uncoalesced free chunks\n");
       		return 0;
    	}
   		Chunk = NextMem;
    	NextMem = Chunk_getNextInMem(NextMem, HeapEnd);
  	}

  	/* Check if each foward link is matched with the correct backwards link */
  	for (iBin = 0; iBin < NUM_BINS; iBin++) {
   		Chunk = freebinArray[iBin];
   		NextList = NULL;
   		if (Chunk != NULL) {
        	NextList = Chunk_getNextInList(Chunk);
      
        	while (NextList != NULL) {
        		if (Chunk_getPrevInList(NextList) != Chunk) {
               		fprintf(stderr, "Mismatched Foward/Backward Link\n");
               		return 0;
            	}

        		Chunk = NextList;
        		NextList = Chunk_getNextInList(NextList);
        	}
      	}
    }

    /* Check if each chunk in the bin is of the right size */
    /* Also checks if each chunk in the bin is set to free */
    for (iBin = 0; iBin < NUM_BINS - 1; iBin++) {
		Chunk = freebinArray[iBin];
		while (Chunk != NULL) {
			if ((int)Chunk_getUnits(Chunk) != iBin) {
            	fprintf(stderr, "Chunk in Wrong Bin, ""Bin = %d\n", iBin);
				return 0;
			}
         
        	if (Chunk_getStatus(Chunk) != CHUNK_FREE) {
            	fprintf(stderr, "Used Chunk in Free List, ""Bin = %d\n", iBin);
            	return 0;
       		}

       		Chunk = Chunk_getNextInList(Chunk);
		}
	}

	/* Make sure all free chunks are in the free list */
	MemChunk = HeapStart;
	ListChunk = freebinArray[0];
   
   	while (MemChunk != NULL) {
      	/* If MemChunk is free, make sure it is in free list */
      	if (Chunk_getStatus(MemChunk) == CHUNK_FREE) {
			for (iBin = 0; iBin < NUM_BINS; iBin++) {
           		ListChunk = freebinArray[iBin];
      
            	/* Iterate free list to find MemChunk */
            	while (ListChunk != NULL) {
            		if (ListChunk == MemChunk)
               			break;
               		ListChunk = Chunk_getNextInList(ListChunk);
            	}
            
           		/* If MemChunk found, then break out */
            	if (ListChunk != NULL)
               		break;
        	}
         
        	/* If MemChunk wasn't in free list, error */
        	if (ListChunk == NULL) {
        		fprintf(stderr, "Free chunks not in free list\n");
            	return 0;
        	}
      	}
      	MemChunk = Chunk_getNextInMem(MemChunk, HeapEnd);
    }

    /* Check if the free list in each bin is a complete loop */
    /* i.e. same number of chunks going fowards and backwards and that we end up in the same spot */
    for (iBin = 0; iBin < NUM_BINS; iBin++) {
    	Chunk = freebinArray[iBin];
      	if (Chunk != NULL) {
        	/*going fowards*/
        	while (Chunk_getNextInList(Chunk) != NULL) {
            	i++;
        		Chunk = Chunk_getNextInList(Chunk);
        	}
      
        	/*retracing backwards*/
        	while (Chunk_getPrevInList(Chunk) != NULL) {
        		i--;
            	Chunk = Chunk_getPrevInList(Chunk);
        	}
      
        	/*should end where we started*/
         	if (Chunk != freebinArray[iBin]) {
            	fprintf(stderr, "Doubly Linked-List Not Complete, ""Bin = %d\n", iBin);
            	return 0;
         	}
      
        	/* Should be the same number of chunks going forwards and backwards */
        	if (i != 0) {
            	fprintf(stderr, "Number of Forwards Chunks Does Not ""Match Number of Backwards Chunks, Bin = %d\n", iBin);
            	return 0;
         	}
      	}
    }
   
	return 1;
}

/* ............................................................................. */

int FindBin(size_t Units)

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
		freebinArray[FindBin(chunk_ptr_val)] = nextinList;
	if (nextinList != NULL)
		Chunk_setPrevInList(nextinList, previnList);
}

void InsertinBin(Chunk_T chunk_ptr)

/* Inserts chunk in link structure in respective bin */

{	
	Chunk_T binptr, prev_chunk, prevptr = NULL;
	size_t chunk_size = Chunk_getUnits(chunk_ptr);
	size_t temp_size = chunk_size;

	if (chunk_size > NUM_BINS - 1) {
		binptr = freebinArray[NUM_BINS - 1];
		temp_size = NUM_BINS - 1;
	}
	else
		binptr = freebinArray[chunk_size];

	/* Traversing the link list */
	while (binptr != NULL && Chunk_getUnits(binptr) < chunk_size) {
		prevptr = binptr;
		binptr = Chunk_getNextInList(binptr);
	}

	/* Insert before binptr in the link structure */
	Chunk_setNextInList(chunk_ptr, binptr);
	if (binptr != NULL) {
		//printf("In InsertBin() - NON NULL case\n");
		prev_chunk = Chunk_getPrevInList(binptr);
		Chunk_setPrevInList(chunk_ptr, prev_chunk);
		if (prev_chunk == NULL)
			freebinArray[temp_size] = chunk_ptr;

		Chunk_setPrevInList(binptr, chunk_ptr);
		if (prevptr != NULL)
			Chunk_setNextInList(prevptr, chunk_ptr);
	}
	else {
		//printf("In InsertBin() - NULL case\n");

		if (freebinArray[temp_size] == NULL) {
			freebinArray[temp_size] = chunk_ptr;
			Chunk_setPrevInList(chunk_ptr, NULL);
		}
		else {
			//printf("HERE..........................................\n");
			Chunk_setNextInList(prevptr, chunk_ptr);
			Chunk_setPrevInList(chunk_ptr, prevptr);
		}
	}
}

Chunk_T useChunk(Chunk_T chunk_ptr, size_t Units, int ibin)

/* Uses the chunk. If chunk size is close to Units then remove the chunk from free list and return it.
	If chunk size is too big split it and re arrange link structure
	and return pointer to correct memory location slice */
/* Before returning also be sure to maintain the repective bin link structure from which chunk is being used */

{
	assert(Chunk_isValid(chunk_ptr, HeapStart, HeapEnd));
	assert(Chunk_getStatus(chunk_ptr) == CHUNK_FREE);
	assert(Chunk_getUnits(chunk_ptr) >= Units);

	size_t chunk_ptr_val = Chunk_getUnits(chunk_ptr);
	size_t splitchunk_size = chunk_ptr_val - Units;
	Chunk_T temp_ptr, splitchunk;

	/* Allocate all the memory */
	if (chunk_ptr_val < Units + MIN_UNITS_PER_CHUNK) {
		/* Update status */
		Chunk_setStatus(chunk_ptr, CHUNK_INUSE);

		/* Removing chunk_ptr from list */
		removefromList(chunk_ptr);

		assert(Chunk_isValid(chunk_ptr, HeapStart, HeapEnd));

		return chunk_ptr;
	}

	/* Split the chunk pointed by chunk_ptr */
	temp_ptr = chunk_ptr;
	removefromList(chunk_ptr);
	
	Chunk_setUnits(temp_ptr, Units);
	Chunk_setStatus(temp_ptr, CHUNK_INUSE);

	splitchunk = Chunk_getNextInMem(temp_ptr, HeapEnd);
	Chunk_setUnits(splitchunk, splitchunk_size);
	Chunk_setStatus(splitchunk, CHUNK_FREE);

	assert(Chunk_isValid(splitchunk, HeapStart, HeapEnd));

	/* Depending on size of the split chunk, insert it in correct bin */
	InsertinBin(splitchunk);

	return temp_ptr;
}

/* .................................................................................. */

Chunk_T Chunk_coalesce(Chunk_T a_chunk_ptr, Chunk_T b_chunk_ptr)

/* Coalesces chunks pointed by a_chunk_ptr and b_chunk_ptr and returns pointer
	to the newly created chunk */

{
	/* Chunks should be valid */
	assert(Chunk_isValid(a_chunk_ptr, HeapStart, HeapEnd));
	assert(Chunk_isValid(b_chunk_ptr, HeapStart, HeapEnd));

	/* Chunks should be free */
	assert(Chunk_getStatus(a_chunk_ptr) == CHUNK_FREE);
	assert(Chunk_getStatus(b_chunk_ptr) == CHUNK_FREE);

	/* Chunks should be adjacent */
	assert(Chunk_getNextInMem(a_chunk_ptr, HeapEnd) == b_chunk_ptr); 

	size_t a_chunk_size = Chunk_getUnits(a_chunk_ptr);
	size_t b_chunk_size = Chunk_getUnits(b_chunk_ptr);
	size_t coalesce_chunk_size = a_chunk_size + b_chunk_size;

	Chunk_setUnits(a_chunk_ptr, coalesce_chunk_size);

	assert(Chunk_getStatus(a_chunk_ptr) == CHUNK_FREE);
	assert(Chunk_isValid(a_chunk_ptr, HeapStart, HeapEnd));

	return a_chunk_ptr;
}

/* .................................................................................. */

Chunk_T getmoreMemory(size_t uiUnits)

/* Request more memory from the operating system -- enough to store
   uiUnits units.  Create a new chunk, coalesce it with adjacent free
   chunks, and insert into the start of its bin's free list. 
   Returns the start of the new chunk. */

{
	Chunk_T Chunk;
	Chunk_T PrevMem;
	Chunk_T NewHeapEnd;

	if (uiUnits < MAX_SIZE)
		uiUnits = MAX_SIZE;

	/* Move the program break. */
	NewHeapEnd = (Chunk_T)((char *)HeapEnd + (uiUnits * Chunk_getUnitSize()));
	if (NewHeapEnd < HeapEnd)  /* Check for overflow */
		return NULL;
	if (brk(NewHeapEnd) == -1)
		return NULL;

	Chunk = HeapEnd;
	HeapEnd = NewHeapEnd;
	PrevMem = Chunk_getPrevInMem(Chunk, HeapStart);

	/* Set the fields of the new chunk */
	Chunk_setUnits(Chunk, uiUnits);
	Chunk_setStatus(Chunk, CHUNK_FREE);

	/* Coalesce the last chunk in memory if it is free */
	if ((PrevMem != NULL) && (Chunk_getStatus(PrevMem) == CHUNK_FREE)) {
		removefromList(PrevMem);

		Chunk = Chunk_coalesce(PrevMem, Chunk);
	}


	/* Add the new chunk to the front of its correct bin's free list. */
	InsertinBin(Chunk);

	assert(Chunk_isValid(Chunk, HeapStart, HeapEnd));
	assert(Chunk_getStatus(Chunk) == CHUNK_FREE);
	assert(Chunk == freebinArray[FindBin(Chunk_getUnits(Chunk))]);

	return Chunk;
}

/* .................................................................................. */

void *my_malloc(size_t size)

/* Allocate numbytes of memory from the free memory pool and return a pointer to the base of the newly allocated region. 
	Returns NULL if size is zero or if memory allocation failed */

{
	size_t UnitSize = Chunk_getUnitSize();
	size_t Units;
	int ibin;
	Chunk_T chunk_ptr;

	if (size == 0)
		return NULL;

	/* Determine the number of units the new chunk should contain */
	Units = ((size - 1) / UnitSize) + 1;
	Units = Units + 2;	/* For Header and Footer */

	/* Initialize if this is the first call */
	if (HeapStart == NULL) {
		HeapStart = (Chunk_T)sbrk(0);
		HeapEnd = HeapStart;
	}

	assert(HeapMgr_isValid());

	/* Traverse through the array and look if bin of required size is available.
		If it is, allocate it. If not, Check larger bins. */
	for (ibin = FindBin(Units); ibin < NUM_BINS; ibin++) {
		chunk_ptr = freebinArray[ibin];

		/* Traverse the link structure starting from index ibin */
		while (chunk_ptr != NULL) {
			if (Chunk_getUnits(chunk_ptr) >= Units) {
				chunk_ptr = useChunk(chunk_ptr, Units, ibin);

				assert(HeapMgr_isValid());
				assert(Chunk_isValid(chunk_ptr, HeapStart, HeapEnd));

				return (void *)((char *)chunk_ptr + UnitSize);
			}
			chunk_ptr = Chunk_getNextInList(chunk_ptr);
		}
	}

	/* Required memory is not found. Obtain new memory by doing malloc() */
	chunk_ptr = getmoreMemory(Units);

	/* malloc failed */
	if (chunk_ptr == NULL) {
		assert(HeapMgr_isValid());

		return NULL;
	}

	/* Chunk is available for use */
	chunk_ptr = useChunk(chunk_ptr, Units, MAX_SIZE - 1);

	assert(HeapMgr_isValid());

	return (void *)((char *)chunk_ptr + UnitSize);
}

/* ................................................................................ */

void my_free(void *region)

/* Free a previously allocated region. Region points to a region allocated by my_malloc().
	If region is NULL do nothing */

{
	size_t UnitSize = Chunk_getUnitSize();
	Chunk_T chunk_ptr, PrevMem, NextMem;

	if (region == NULL)
		return;

	assert(HeapMgr_isValid());

	chunk_ptr = (Chunk_T)((char*)region - UnitSize);
	assert(Chunk_isValid(chunk_ptr, HeapStart, HeapEnd));

	Chunk_setStatus(chunk_ptr, CHUNK_FREE);
	NextMem = Chunk_getNextInMem(chunk_ptr, HeapEnd);
	PrevMem = Chunk_getPrevInMem(chunk_ptr, HeapStart);

	/* If PrevMem is also free coalesce the two chunks */
	if (PrevMem != NULL && Chunk_getStatus(PrevMem) == CHUNK_FREE) {
		removefromList(PrevMem);

		chunk_ptr = Chunk_coalesce(PrevMem, chunk_ptr);

		assert(Chunk_isValid(chunk_ptr, HeapStart, HeapEnd));
		assert(Chunk_getStatus(chunk_ptr) == CHUNK_FREE);
	}
	
	/* If NextMem is also free coalesce the two chunks */
	if (NextMem != NULL && Chunk_getStatus(NextMem) == CHUNK_FREE) {
		removefromList(NextMem);

		chunk_ptr = Chunk_coalesce(chunk_ptr, NextMem);

		assert(Chunk_isValid(chunk_ptr, HeapStart, HeapEnd));
		assert(Chunk_getStatus(chunk_ptr) == CHUNK_FREE);
	}

	/* Place final bigger chunk in starting of linked structure for correct bin */
	InsertinBin(chunk_ptr);

	assert(HeapMgr_isValid());
}

/* .................................................................................. */

void *my_calloc(size_t nitems, size_t size)

/* Allocate the requested memory, initialize it to zero and returns a pointer to the beginning of allocated region.
	Returns NULL if memory allocation failed */

{	
	char *chunk_ptr;
	size_t total_size = nitems * size;
	chunk_ptr = (char *)my_malloc(total_size);

	/* Setting all bytes to zero */
	if (chunk_ptr != NULL)
		chunk_ptr = memset(chunk_ptr, 0, (int)total_size);

	return chunk_ptr;
}

/* .................................................................................. */

void *my_realloc(void *ptr, size_t size)

/* Resize the memory block pointed to by ptr that was previously allocated with a call to my_malloc or my_calloc.
	Returns a pointer to the newly allocated memory, or NULL if the request fails. */

{	
	Chunk_T ptr_header, splitchunk;
	size_t initialsize;
	size_t UnitSize = Chunk_getUnitSize();
	size_t size_units, splitchunk_size_units;	

	Chunk_T new_ptr;

	ptr_header = (Chunk_T)((char *)ptr - UnitSize);
	initialsize = Chunk_getUnits(ptr_header);
	size_units = (size - 1) / UnitSize + 3;

	/* Change the chunk units in the header and return same pointer */
	if (size_units < initialsize - 3) {
		Chunk_setUnits(ptr_header, size_units);

		/* Free next pointers */
		/* Split the chunk, set size and status of the splitchunk and insert it in freebinArray */
		splitchunk = Chunk_getNextInMem(ptr_header, HeapEnd);
		splitchunk_size_units = initialsize - size_units;

		Chunk_setUnits(splitchunk, splitchunk_size_units);
		Chunk_setStatus(splitchunk, CHUNK_INUSE);

		my_free((Chunk_T)((char *)splitchunk + UnitSize));

		return ptr;
	}

	if (size_units <= initialsize)
		return ptr;

	new_ptr = (Chunk_T)my_malloc(size);

	/* Copying byte by byte to new location */
	if (new_ptr != NULL) {
		new_ptr = (Chunk_T) strncpy((char *)new_ptr, (char *)ptr, (initialsize - 2) * UnitSize);
		Chunk_setUnits((Chunk_T)((char *)new_ptr - UnitSize), size_units);
		Chunk_setStatus((Chunk_T)((char *)new_ptr - UnitSize), CHUNK_INUSE);

		my_free(ptr);
	}

	return new_ptr;
}
