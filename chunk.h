/***************************************************************************** 
*    This file is part of project.
*
*    project is free software: you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation, either version 3 of the License, or
*    (at your option) any later version.
*
*    project is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with project.  If not, see <http://www.gnu.org/licenses/>.
*****************************************************************************/

#ifndef CHUNK_INCLUDED
#define CHUNK_INCLUDED

#include <stddef.h>

enum ChunkStatus {CHUNK_FREE, CHUNK_INUSE};
/* A Chunk can be either free or in use. */

typedef struct Chunk *Chunk_T;

/* A Chunk is a sequence of Units.
   The first Unit is a header that indicates the number of Units in the
   Chunk, whether the Chunk is free, and, if the Chunk is free, a
   pointer to the next Chunk in the free list.  The last Unit is a
   footer that indicates the number of Units in the Chunk and, if the
   Chunk is free, a pointer to the previous Chunk in the free list.
   The Units between the header and footer store client data. */

#define MIN_UNITS_PER_CHUNK   3
/* The minimum number of units that a Chunk can contain. */

size_t Chunk_getUnitSize(void);
/* Return the number of bytes in a Unit. */

enum ChunkStatus Chunk_getStatus(Chunk_T Chunk);
/* Return the status of Chunk. */

void Chunk_setStatus(Chunk_T Chunk, enum ChunkStatus eStatus);
/* Set the status of Chunk to eStatus. */

size_t Chunk_getUnits(Chunk_T Chunk);
/* Return Chunk's number of units. */

void Chunk_setUnits(Chunk_T Chunk, size_t uiUnits);
/* Set Chunk's number of units to uiUnits. */

Chunk_T Chunk_getNextInList(Chunk_T Chunk);
/* Return Chunk's next Chunk in the free list, or NULL if there
   is no next Chunk. */

void Chunk_setNextInList(Chunk_T Chunk, Chunk_T oNextChunk);
/* Set Chunk's next Chunk in the free list to oNextChunk. */

Chunk_T Chunk_getPrevInList(Chunk_T Chunk);
/* Return Chunk's previous Chunk in the free list, or NULL if there
   is no previous Chunk. */

void Chunk_setPrevInList(Chunk_T Chunk, Chunk_T oPrevChunk);
/* Set Chunk's previous Chunk in the free list to oPrevChunk. */

Chunk_T Chunk_getNextInMem(Chunk_T Chunk, Chunk_T oHeapEnd);
/* Return Chunk's next Chunk in memory, or NULL if there is no
   next Chunk.  Use oHeapEnd to determine if there is no next
   Chunk.  Chunk's number of units must be set properly for this
   function to work. */

Chunk_T Chunk_getPrevInMem(Chunk_T Chunk, Chunk_T oHeapStart);
/* Return Chunk's previous Chunk in memory, or NULL if there is no
   previous Chunk.  Use oHeapStart to determine if there is no
   previous Chunk.  The previous Chunk's number of units must be set
   properly for this function to work. */

int Chunk_isValid(Chunk_T Chunk, Chunk_T oHeapStart, Chunk_T oHeapEnd);
/* Return 1 (TRUE) if Chunk is valid, notably with respect to
   psHeapStart and psHeapEnd, or 0 (FALSE) otherwise. */

#endif