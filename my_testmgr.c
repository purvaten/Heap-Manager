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

#include "heapmngr.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* The maximum allowable number of calls of HeapMgr_malloc(). */
#define MAX_CALLS      10000

enum {FALSE, TRUE};

/*--------------------------------------------------------------------*/
#define ASSURE(i) assure(i, __LINE__)

static void assure(int iSuccessful, int iLineNum)

/* If !iSuccessful, print an error message indicating that the test
   at line iLineNum failed. */

{
   if (! iSuccessful)
      fprintf(stderr, "Test at line %d failed.\n", iLineNum);
}
/*--------------------------------------------------------------------*/

/* Memory chunks allocated by my_malloc(). */
static char *Chunks[MAX_CALLS];

/* Randomly generated chunk sizes. */
static int aiSizes[MAX_CALLS];

/*--------------------------------------------------------------------*/

void testRandomRandom(int iCount, int iSize)

/* Allocate and free iCount memory chunks, each of some random size
   less than iSize, in a random order. */

{
   int i;
   int iRand;
   int iLogicalArraySize;

   iLogicalArraySize = (iCount / 3) + 1;

   /* Fill aiSizes, an array of random integers in the range 1 to iSize. */
   for (i = 0; i < iLogicalArraySize; i++)
      aiSizes[i] = (rand() % iSize) + 1;

   /* Call my_malloc() and my_free() in a randomly interleaved manner. */
   iRand = 0;
   for (i = 0; i < iCount; i++) {
      Chunks[iRand] = (char *)my_malloc((size_t)aiSizes[iRand]);
      assert(Chunks[iRand] != NULL);

      /* Fill the newly allocated chunk with some character.
         The character is derived from the last digit of iRand.
         So later, given iRand, we can check to make sure that
         the contents haven't been corrupted. */
      int iCol;
      char c = (char)((iRand % 10) + '0');
      for (iCol = 0; iCol < aiSizes[iRand]; iCol++)
         Chunks[iRand][iCol] = c;

      /* Assign some random integer to iRand. */
      iRand = rand() % iLogicalArraySize;

      /* If Chunks[iRand] contains a chunk, free it and set Chunks[iRand] to NULL. */
      if (Chunks[iRand] != NULL) {

         /* Check the chunk that is about to be freed to make sure
            that its contents haven't been corrupted. */
         int iCol;
         char c = (char)((iRand % 10) + '0');
         for (iCol = 0; iCol < aiSizes[iRand]; iCol++)
            ASSURE(Chunks[iRand][iCol] == c);

         my_free(Chunks[iRand]);
         Chunks[iRand] = NULL;
      }
   }

   /* Free the rest of the chunks. */
   for (i = 0; i < iLogicalArraySize; i++) {
      if (Chunks[i] != NULL) {

         /* Check the chunk that is about to be freed to make sure
            that its contents haven't been corrupted. */
         int iCol;
         char c = (char)((i % 10) + '0');
         for (iCol = 0; iCol < aiSizes[i]; iCol++)
            ASSURE(Chunks[i][iCol] == c);

         my_free(Chunks[i]);
         Chunks[i] = NULL;
      }
   }
}

/*--------------------------------------------------------------------*/

void malloc_free_test()

/* Testing my_malloc() & my_free() giving fixed values */

{
   char *p, *q, *r, *s;

   p = (char *)my_malloc(28);
 
   q = (char *)my_malloc(160);
   r = (char *)my_malloc(0);

   my_free(q);

   s = (char *)my_malloc(100);

   my_free(p);
   my_free(r);
   my_free(s);
}

void calloc_test()

/* Testing my_calloc() giving fixed valus */

{
   char *p;
   int i;
   p = my_calloc(5, 20);
   printf("Printing all 100 characters : \n");

   for (i = 0; i < 100; i++) {
     printf("%d ", p[i]);
   }
   printf("\n");
}

void realloc_test()

/* Testing my_realloc() giving fixed values */

{
   char *p;

   p = (char *)my_malloc(28);
   strcpy(p, "abcdefghijklmnopqrstuvwxyz");
   p = (char *)my_realloc(p, 150);
   printf("Initial Result : %s\n", p);

   p[12] = '\0';
   p = (char *)my_realloc(p, 12);
   printf("Intermediate Result : %s\n", p);

   p = (char *)my_realloc(p, 20);
   printf("Final Result : %s\n", p);
   my_free(p);
}

/*--------------------------------------------------------------------*/

int main(int argc, char *argv[])

/* Test the HeapMgr_malloc() and HeapMgr_free() functions.
      RandomRandom: random order with random size chunks */

{  
   int option, n, size;

   printf("Enter option to test the code : \n");
   printf("1) Test for my_malloc() and my_free()\n");
   printf("2) Test for my_calloc()\n");
   printf("3) Test for my_realloc()\n");
   printf("4) Random test case\n");
   scanf("%d", &option);

   switch (option) {
      case 1 :
         malloc_free_test();
         break;

      case 2 :
         calloc_test();
         break;

      case 3 :
         realloc_test();
         break;

      case 4 :
         printf("Enter the number of chunks : \n");
         scanf("%d", &n);
         printf("Enter the max size boundary : \n");
         scanf("%d", &size);
         testRandomRandom(n, size);
         break;

      default : 
         break;

   }
   return 0;
}

/*--------------------------------------------------------------------*/