#include "chunk.h"
#include <stddef.h>
#include <stdio.h>
#include <assert.h>

/*--------------------------------------------------------------------*/

typedef struct Chunk
{
   size_t uiUnits;
   /* The number of Units in the Chunk.  The low-order bit
      stores the Chunk's status. */

   Chunk_T oAdjacentChunk;
   /* The address of an adjacent Chunk. */
}Chunk;

/*--------------------------------------------------------------------*/

size_t Chunk_getUnitSize(void)

/* Return the number of bytes in a Unit. */

{
   return sizeof(struct Chunk);
}

/*--------------------------------------------------------------------*/

enum ChunkStatus Chunk_getStatus(Chunk_T oChunk)

/* Return the status of oChunk. */

{
   assert(oChunk != NULL);

   return oChunk->uiUnits & 1U;
}

/*--------------------------------------------------------------------*/

void Chunk_setStatus(Chunk_T oChunk, enum ChunkStatus eStatus)

/* Set the status of oChunk to eStatus. */

{
   assert(oChunk != NULL);
   assert((eStatus == CHUNK_FREE) || (eStatus == CHUNK_INUSE));

   oChunk->uiUnits &= ~1U;
   oChunk->uiUnits |= eStatus;
}

/*--------------------------------------------------------------------*/

size_t Chunk_getUnits(Chunk_T oChunk)

/* Return oChunk's number of units. */

{
   assert(oChunk != NULL);

   return oChunk->uiUnits >> 1;
}

/*--------------------------------------------------------------------*/

void Chunk_setUnits(Chunk_T oChunk, size_t uiUnits)

/* Set oChunk's number of units to uiUnits. */

{
   assert(oChunk != NULL);
   assert(uiUnits >= MIN_UNITS_PER_CHUNK);

   /* Set the Units in oChunk's header. */
   oChunk->uiUnits &= 1U;
   oChunk->uiUnits |= uiUnits << 1U;

   /* Set the Units in oChunk's footer. */
   (oChunk + uiUnits - 1)->uiUnits = uiUnits;
}

/*--------------------------------------------------------------------*/

Chunk_T Chunk_getNextInList(Chunk_T oChunk)

/* Return oChunk's next Chunk in the free list, or NULL if there
   is no next Chunk. */

{
   assert(oChunk != NULL);

   return oChunk->oAdjacentChunk;
}

/*--------------------------------------------------------------------*/

void Chunk_setNextInList(Chunk_T oChunk, Chunk_T oNextChunk)

/* Set oChunk's next Chunk in the free list to oNextChunk. */

{
   assert(oChunk != NULL);

   oChunk->oAdjacentChunk = oNextChunk;
}

/*--------------------------------------------------------------------*/

Chunk_T Chunk_getPrevInList(Chunk_T oChunk)

/* Return oChunk's previous Chunk in the free list, or NULL if there
   is no previous Chunk. */

{
   assert(oChunk != NULL);

   return (oChunk + Chunk_getUnits(oChunk) - 1)->oAdjacentChunk;
}

/*--------------------------------------------------------------------*/

void Chunk_setPrevInList(Chunk_T oChunk, Chunk_T oPrevChunk)

/* Set oChunk's previous Chunk in the free list to oPrevChunk. */

{
   assert(oChunk != NULL);

   (oChunk + Chunk_getUnits(oChunk) - 1)->oAdjacentChunk = oPrevChunk;
}

/*--------------------------------------------------------------------*/

Chunk_T Chunk_getNextInMem(Chunk_T oChunk, Chunk_T oHeapEnd)

/* Return oChunk's next Chunk in memory, or NULL if there is no
   next Chunk.  Use oHeapEnd to determine if there is no next
   Chunk.  oChunk's number of units must be set properly for this
   function to work. */

{
   Chunk_T oNextChunk;

   assert(oChunk != NULL);
   assert(oHeapEnd != NULL);
   assert(oChunk < oHeapEnd);

   oNextChunk = oChunk + Chunk_getUnits(oChunk);
   assert(oNextChunk <= oHeapEnd);

   if (oNextChunk == oHeapEnd)
      return NULL;
   return oNextChunk;
}

/*--------------------------------------------------------------------*/

Chunk_T Chunk_getPrevInMem(Chunk_T oChunk, Chunk_T oHeapStart)

/* Return oChunk's previous Chunk in memory, or NULL if there is no
   previous Chunk.  Use oHeapStart to determine if there is no
   previous Chunk.  The previous Chunk's number of units must be set
   properly for this function to work. */

{
   Chunk_T oPrevChunk;

   assert(oChunk != NULL);
   assert(oHeapStart != NULL);
   assert(oChunk >= oHeapStart);

   if (oChunk == oHeapStart)
      return NULL;

   oPrevChunk = oChunk - ((oChunk - 1)->uiUnits);
   assert(oPrevChunk >= oHeapStart);

   return oPrevChunk;
}

/*--------------------------------------------------------------------*/

#ifndef NDEBUG

static size_t Chunk_getFooterUnits(Chunk_T oChunk)

/* Return the number of units as stored in oChunk's footer. */

{
   assert(oChunk != NULL);

   return (oChunk + Chunk_getUnits(oChunk) - 1)->uiUnits;
}

#endif

/*--------------------------------------------------------------------*/

#ifndef NDEBUG

int Chunk_isValid(Chunk_T oChunk, Chunk_T oHeapStart, Chunk_T oHeapEnd)

/* Return 1 (TRUE) if oChunk is valid, notably with respect to
   psHeapStart and psHeapEnd, or 0 (FALSE) otherwise. */

{
   assert(oChunk != NULL);
   assert(oHeapStart != NULL);
   assert(oHeapEnd != NULL);

   if (oChunk < oHeapStart)
      {fprintf(stderr, "Bad heap start\n"); return 0; }
   if (oChunk >= oHeapEnd)
      {fprintf(stderr, "Bad heap end\n"); return 0; }
   if (Chunk_getUnits(oChunk) == 0)
      {fprintf(stderr, "Zero units\n"); return 0; }
   if (Chunk_getUnits(oChunk) < MIN_UNITS_PER_CHUNK)
      {fprintf(stderr, "Bad size\n"); return 0; }
   if (oChunk + Chunk_getUnits(oChunk) > oHeapEnd)
      {fprintf(stderr, "Bad chunk end\n"); return 0; }
   if (Chunk_getUnits(oChunk) != Chunk_getFooterUnits(oChunk))
      {fprintf(stderr, "Inconsistent sizes\n"); return 0; }
   return 1;
}

#endif