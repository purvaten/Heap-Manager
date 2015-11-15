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