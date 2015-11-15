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

#include "chunk.h"
#include <stddef.h>
#include <stdio.h>
#include <assert.h>

/*--------------------------------------------------------------------*/

typedef struct Chunk {
   size_t uiUnits;
   /* The number of Units in the Chunk.  The low-order bit
      stores the Chunk's status. */

   Chunk_T AdjacentChunk;
   /* The address of an adjacent Chunk. */
}Chunk;

/*--------------------------------------------------------------------*/

size_t Chunk_getUnitSize(void)

/* Return the number of bytes in a Unit. */

{
   return sizeof(struct Chunk);
}

/*--------------------------------------------------------------------*/

enum ChunkStatus Chunk_getStatus(Chunk_T Chunk)

/* Return the status of Chunk. */

{
   assert(Chunk != NULL);

   return Chunk->uiUnits & 1U;
}

/*--------------------------------------------------------------------*/

void Chunk_setStatus(Chunk_T Chunk, enum ChunkStatus eStatus)

/* Set the status of Chunk to eStatus. */

{
   assert(Chunk != NULL);
   assert((eStatus == CHUNK_FREE) || (eStatus == CHUNK_INUSE));

   Chunk->uiUnits &= ~1U;
   Chunk->uiUnits |= eStatus;
}

/*--------------------------------------------------------------------*/

size_t Chunk_getUnits(Chunk_T Chunk)

/* Return Chunk's number of units. */

{
   assert(Chunk != NULL);

   return Chunk->uiUnits >> 1;
}

/*--------------------------------------------------------------------*/

void Chunk_setUnits(Chunk_T Chunk, size_t uiUnits)

/* Set Chunk's number of units to uiUnits. */

{
   assert(Chunk != NULL);
   assert(uiUnits >= MIN_UNITS_PER_CHUNK);

   /* Set the Units in Chunk's header. */
   Chunk->uiUnits &= 1U;
   Chunk->uiUnits |= uiUnits << 1U;

   /* Set the Units in Chunk's footer. */
   (Chunk + uiUnits - 1)->uiUnits = uiUnits;
}

/*--------------------------------------------------------------------*/

Chunk_T Chunk_getNextInList(Chunk_T Chunk)

/* Return Chunk's next Chunk in the free list, or NULL if there
   is no next Chunk. */

{
   assert(Chunk != NULL);

   return Chunk->AdjacentChunk;
}

/*--------------------------------------------------------------------*/

void Chunk_setNextInList(Chunk_T Chunk, Chunk_T NextChunk)

/* Set Chunk's next Chunk in the free list to NextChunk. */

{
   assert(Chunk != NULL);

   Chunk->AdjacentChunk = NextChunk;
}

/*--------------------------------------------------------------------*/

Chunk_T Chunk_getPrevInList(Chunk_T Chunk)

/* Return Chunk's previous Chunk in the free list, or NULL if there
   is no previous Chunk. */

{
   assert(Chunk != NULL);

   return (Chunk + Chunk_getUnits(Chunk) - 1)->AdjacentChunk;
}

/*--------------------------------------------------------------------*/

void Chunk_setPrevInList(Chunk_T Chunk, Chunk_T PrevChunk)

/* Set Chunk's previous Chunk in the free list to PrevChunk. */

{
   assert(Chunk != NULL);

   (Chunk + Chunk_getUnits(Chunk) - 1)->AdjacentChunk = PrevChunk;
}

/*--------------------------------------------------------------------*/

Chunk_T Chunk_getNextInMem(Chunk_T Chunk, Chunk_T HeapEnd)

/* Return Chunk's next Chunk in memory, or NULL if there is no
   next Chunk.  Use HeapEnd to determine if there is no next
   Chunk.  Chunk's number of units must be set properly for this
   function to work. */

{
   Chunk_T NextChunk;

   assert(Chunk != NULL);
   assert(HeapEnd != NULL);
   assert(Chunk < HeapEnd);

   NextChunk = Chunk + Chunk_getUnits(Chunk);
   assert(NextChunk <= HeapEnd);

   if (NextChunk == HeapEnd)
      return NULL;
   return NextChunk;
}

/*--------------------------------------------------------------------*/

Chunk_T Chunk_getPrevInMem(Chunk_T Chunk, Chunk_T HeapStart)

/* Return Chunk's previous Chunk in memory, or NULL if there is no
   previous Chunk. Use HeapStart to determine if there is no
   previous Chunk. The previous Chunk's number of units must be set
   properly for this function to work. */

{  
   assert(HeapStart != NULL);
   assert(Chunk != NULL);
   assert(Chunk >= HeapStart);

   size_t UnitSize = Chunk_getUnitSize();
   Chunk_T PrevChunk;

   if (Chunk == HeapStart)
      return NULL;

   Chunk_T PrevFooter = (Chunk_T)((char *)Chunk - UnitSize);
   size_t PrevSize = PrevFooter->uiUnits;

   PrevChunk = Chunk - PrevSize;
   assert(PrevChunk >= HeapStart);

   return PrevChunk;
}

/*--------------------------------------------------------------------*/

size_t Chunk_getFooterUnits(Chunk_T Chunk)

/* Return the number of units as stored in Chunk's footer. */

{
   assert(Chunk != NULL);

   return (Chunk + Chunk_getUnits(Chunk) - 1)->uiUnits;
}

/*--------------------------------------------------------------------*/

int Chunk_isValid(Chunk_T Chunk, Chunk_T HeapStart, Chunk_T HeapEnd)

/* Return 1 (TRUE) if Chunk is valid, notably with respect to
   HeapStart and HeapEnd, or 0 (FALSE) otherwise. */

{
   assert(Chunk != NULL);
   assert(HeapStart != NULL);
   assert(HeapEnd != NULL);

   if (Chunk < HeapStart)
      {fprintf(stderr, "Bad heap start\n"); return 0; }
   if (Chunk >= HeapEnd)
      {fprintf(stderr, "Bad heap end\n"); return 0; }
   if (Chunk_getUnits(Chunk) == 0)
      {fprintf(stderr, "Zero units\n"); return 0; }
   if (Chunk_getUnits(Chunk) < MIN_UNITS_PER_CHUNK)
      {fprintf(stderr, "Bad size\n"); return 0; }
   if (Chunk + Chunk_getUnits(Chunk) > HeapEnd)
      {fprintf(stderr, "Bad chunk end\n"); return 0; }
   if (Chunk_getUnits(Chunk) != Chunk_getFooterUnits(Chunk))
      {fprintf(stderr, "Inconsistent sizes\n"); return 0; }
   return 1;
}