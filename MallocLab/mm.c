#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <memory.h>
#include "mm.h"
#include "memlib.h"
#include <assert.h>
#include <string.h>

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
	/* Team name */
	"Super Senior Club",
	/* First member's full name */
	"Devin Burke",
	/* First member's email address */
	"thpa9933@colorado.edu",
	"Devin Burke"

};


#define malloc mm_malloc
#define free mm_free
#define realloc mm_realloc


#define WSIZE       4       /* word size (bytes) */  
#define DSIZE       8       /* doubleword size (bytes) */
#define CHUNKSIZE  (1<<12)  /* initial heap size (bytes) */
#define OVERHEAD    8       /* overhead of header and footer (bytes) */
#define MINIMUM 24          /* minimum block size*/


#define MAX(x,y) ((x) > (y) ? (x) : (y))

#define ALIGNMENT 8 //single -4- double -8- alignment




//
// Rounds to nearest multiple of alignment 
//
#define ALIGN(p) (((size_t)(p)+(ALIGNMENT-1)) & ~0x7)   

//
// Pack a size and allocated bit into a word
// We mask of the "alloc" field to insure only
// the lower bit is used
//
#define PACK(size,alloc) ((size) | (alloc))

// Read a word to an address
#define GET(p) (*(int*)(p))
// writes a word to an address
#define PUT(p,val) (*(int *)(p) = (val))

// Read the size and allocated fields from address p
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

// Given block ptr bp, compute address of its header and footer
#define HDRP(bp) ((void*)(bp) - WSIZE)
#define FTRP(bp) ((void *)(bp)+ GET_SIZE(HDRP(bp)) - DSIZE)

// Given block ptr bp, head jumps to next block
// prev jumps to previous block
#define NEXT_BLKP(bp) ((void*)(bp) + GET_SIZE(HDRP(bp)))
#define PREV_BLKP(bp) ((void*)(bp) - GET_SIZE(HDRP(bp) - WSIZE))

// Given block pointer bp, compute address of next and previous free blocks
// void** is a pointer to void*
#define NEXT_FREEP(bp)(*(void **)(bp + DSIZE))
#define PREV_FREEP(bp)(*(void **)(bp))


/////////////////////////////////////////////////////////////////////////////
//
// Global Variables
//
static char *free_listp = 0; // pointer to first free block
static char *heap_listp = 0;  /* pointer to first block */  

//
// function prototypes for internal helper routines
//
static void *extend_heap(size_t words);
static void place(void *bp, size_t asize);
static void *find_fit(size_t asize);
static void *coalesce(void *bp);
static void printblock(void *bp); 
static void checkblock(void *bp);
static void headInsert(void *bp);
static void removeBlock(void *bp);


// sources: A Programmer's Perspective, github.com/AHAAAAAAA, github.com/ltganesa, https://danluu.com/malloc-tutorial/, github.com/Somsubhra
//https://www.cs.cmu.edu/~fp/courses/15213-s05/code/18-malloc/malloc.c

// Initializes heap to look like:
//|PAD - 4 bytes -| Header - 8 bytes - | Footer - 8 bytes - | Tail - 4 bytes -|
// Puts in dummy blocks to ensure alignment 
//4 byte boundary alignment is retained 
// tail block implies end of heap and free list 
int mm_init(void) 
{
  
	if ((heap_listp = mem_sbrk(2*MINIMUM)) == NULL) 
	{
	  return -1; //returns -1 if heap space is too small
	}
	PUT(heap_listp, 0); //Alginment padding
	
	// dummy header initialized
	PUT(heap_listp + WSIZE, PACK(MINIMUM, 1));   //WSIZE padding
	PUT(heap_listp + DSIZE, 0);  //previous pointer
	PUT(heap_listp + DSIZE + WSIZE, 0); //NEXT pointer 
	

	PUT(heap_listp + MINIMUM, PACK(MINIMUM, 1));//block footer
	
	PUT(heap_listp + WSIZE + MINIMUM, PACK(0,1)); //tail block
	
	
	free_listp = heap_listp + DSIZE;  //free list points to tail 
	
	if(extend_heap(CHUNKSIZE/WSIZE) == NULL)
	{ 
	  return -1; //returns -1 if unable to extend heap at this point 
	}
	return 0;
}

// sources: A Programmer's Perspective, github.com/AHAAAAAAA, github.com/ltganesa, https://danluu.com/malloc-tutorial/, github.com/Somsubhra
//https://www.cs.cmu.edu/~fp/courses/15213-s05/code/18-malloc/malloc.c

// Extend with free block with even number of words 
// Take the epilogue of allocated heap and convert to header of new free block
// Once extended, put epilogue header at the beginning of new free block 
static void *extend_heap(size_t words) 
{
  char *bp;
  size_t size;
  
	
  size = (words % 2) ? (words+1) * WSIZE : words * WSIZE; //allocate an even number of words

	if(size < MINIMUM)
	{
		size = MINIMUM; // if below minimum reallign (atleast 24 bytes)
	}
    // mem_sbrk returns -1 if unsuccessfull and 0 if successful positive size
  if((long)(bp = mem_sbrk(size)) == -1) 
  {
      return NULL; // if not enough space
  }
  
  // initialize freeblock header/footer and the epilogue header
  // shifting epilogue block over, and redefining the epilogue header
  // block header and epilogue header point to the same place
  // just need to keep track of epilogue cuz its the LAST block in the heap
	PUT(HDRP(bp), PACK(size, 0)); //free header
	PUT(FTRP(bp), PACK(size, 0)); //free footer
	PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); //new epilogue header
    
  // if the previous block was free then merge the two free blocks
  return coalesce(bp); 
  
}

// sources: A Programmer's Perspective, github.com/AHAAAAAAA, github.com/ltganesa, https://danluu.com/malloc-tutorial/, github.com/Somsubhra
//https://www.cs.cmu.edu/~fp/courses/15213-s05/code/18-malloc/malloc.c

// Practice problem 9.8
// find_fit - Find a fit for a block with asize bytes 
// interates through free list to find a free block large enough
static void *find_fit(size_t asize)
{
	
	void *bp;
	for(bp = free_listp; GET_ALLOC(HDRP(bp)) == 0; bp = NEXT_FREEP(bp))
	{
        //only need to search free blocks in explicit list
	  if(asize <= (size_t)(GET_SIZE(HDRP(bp)))) 
		{
		  return bp;
		}
	}
	return NULL; 
}

// sources: A Programmer's Perspective, github.com/AHAAAAAAA, github.com/ltganesa, https://danluu.com/malloc-tutorial/, github.com/Somsubhra
//https://www.cs.cmu.edu/~fp/courses/15213-s05/code/18-malloc/malloc.c


// mm_free - Free a block 
// Frees a previously allocated block the coaleses the space
// set header and footer to unallocated
void mm_free(void *bp)
{
	if(!bp)
	{
		return; 
	}
	
    size_t size = GET_SIZE(HDRP(bp));
    
    PUT(HDRP(bp), PACK(size, 0)); //set to unallocated
    PUT(FTRP(bp), PACK(size, 0)); //set to unallocated
    coalesce(bp); //merge if possible and add to free list
}

// sources: A Programmer's Perspective, github.com/AHAAAAAAA, github.com/ltganesa, https://danluu.com/malloc-tutorial/, github.com/Somsubhra
//https://www.cs.cmu.edu/~fp/courses/15213-s05/code/18-malloc/malloc.c


// joins connected free blocks together
// gets newsize and removes smaller blocks from free list
// changes header and footer to bigger block specs
// adds bigger block to free list
static void *coalesce(void *bp) 
{
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp))) || PREV_BLKP(bp) == bp;
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));
    
  // if next is free (NOT ALLOCATED) then we extend to next 
    if(prev_alloc && !next_alloc){
        size += GET_SIZE(HDRP(NEXT_BLKP(bp))); // gets new size
		removeBlock(NEXT_BLKP(bp)); //remove next block from free list
        PUT(HDRP(bp), PACK(size, 0)); //header updated
        PUT(FTRP(bp), PACK(size, 0)); //footer updated 
    }

 // if prev is free then extend to previous 
    else if(!prev_alloc && next_alloc){
        size += GET_SIZE(HDRP(PREV_BLKP(bp))); //gets new size
		bp = PREV_BLKP(bp);
		removeBlock(bp); //remove previous block from free list 
		PUT(HDRP(bp), PACK(size, 0)); //update header
        PUT(FTRP(bp), PACK(size, 0)); //update footer
        
    }
    
 // if prev and next are free then we extend in both directions
    else if(!prev_alloc && !next_alloc) 
	{
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) +
            GET_SIZE(HDRP(NEXT_BLKP(bp))); //get new size of all three blocks
		removeBlock(PREV_BLKP(bp)); // remove previous block from free list 
		removeBlock(NEXT_BLKP(bp)); //remove next block from free list
		bp = PREV_BLKP(bp); 
		PUT(HDRP(bp), PACK(size, 0)); //update header
        PUT(FTRP(bp), PACK(size, 0)); //update footer
       
    }
	headInsert(bp); //we then add the new free block to the front of the free list
  return bp;
}

// sources: A Programmer's Perspective, github.com/AHAAAAAAA, github.com/ltganesa, https://danluu.com/malloc-tutorial/, github.com/Somsubhra
//https://www.cs.cmu.edu/~fp/courses/15213-s05/code/18-malloc/malloc.c


// mm_malloc - Allocate a block with at least size bytes of payload 
// adjustes block size so alignment and heap space is not violated
// object must be a minimum of 24 bytes to be considered a block
// if not enough space is found, extend the heap
void *mm_malloc(size_t size) 
{
    size_t asize; // asize is adjusted block size with alignment considered 
    size_t extendsize; // amount to extend to if no fit is found
    char *bp;
    
   /* ignore spurious requests */
    if(size <= 0)
	{
        return NULL;
    }
    
	asize = MAX(ALIGN(size) + DSIZE, MINIMUM); 

        // search the free list for a fit
    // places requested block and splits if necessary
    if((bp = find_fit(asize))) 
	{
        place(bp, asize);
        return bp;
    }
    

    extendsize = MAX(asize, CHUNKSIZE); // if we can't find fit then we extend the heap
    if((bp = extend_heap(extendsize/WSIZE)) == NULL)
	{
        return NULL;
    }
    place(bp, asize); //place block into heap
    return bp;
} 

// sources: A Programmer's Perspective, github.com/AHAAAAAAA, github.com/ltganesa, https://danluu.com/malloc-tutorial/, github.com/Somsubhra
//https://www.cs.cmu.edu/~fp/courses/15213-s05/code/18-malloc/malloc.c


//compare asize with the total block size (csize) 
//if the remainder is atleast 24 bytes (minimum)
//split the block into an allocated block and a free block
//otherwise, declare the whole block as allocated to avoid fragmentations
//We mark a minimum to avoide excessive fragmentation
static void place(void *bp, size_t asize)
{
    size_t csize = GET_SIZE(HDRP(bp)); //gets size of entire free block of interest
    
        //if enough room for 24 bit free block
    if((csize - asize) >= MINIMUM) //if difference is greater than minimum then split 
	{
        PUT(HDRP(bp), PACK(asize, 1)); //set header of allocated block
        PUT(FTRP(bp), PACK(asize, 1)); //set footer of allocated block
		removeBlock(bp); // remove block from free list
        bp = NEXT_BLKP(bp); 
 // check remainder to see if there is enough space to split
        PUT(HDRP(bp), PACK(csize - asize, 0)); //split block and changer header
        PUT(FTRP(bp), PACK(csize - asize, 0)); // and footer
		coalesce(bp); //join new free block with other connected free blocks
    }
    else // if cant split block into minimum then make entire block as allocated
	{ 
        PUT(HDRP(bp), PACK(csize, 1)); // change header
        PUT(FTRP(bp), PACK(csize, 1)); // change footer
		removeBlock(bp); // remove block from free list
    }
}

// sources: A Programmer's Perspective, github.com/AHAAAAAAA, github.com/ltganesa, https://danluu.com/malloc-tutorial/, github.com/Somsubhra
//https://www.cs.cmu.edu/~fp/courses/15213-s05/code/18-malloc/malloc.c


// if new size is <= 0, free the block
// if new size is same as old size, return the old bp
// if the new size is less than the old size, shrink the block
    // then if the remaining is atleast 24(minimum) create a free block
// if new size is > old size, call malloc using new size
// copy all old data into new block, then call free to origional block 
void *mm_realloc(void *ptr, size_t size)
{
	void *newptr;
	size_t copySize;

	size_t asize = MAX(ALIGN(size) + DSIZE, MINIMUM);

	if (size <= 0) //then we free the block
	{
			free(ptr);
			return 0;
	}
	if(ptr == NULL) //need to make a block because we have no heap
	{
		return malloc(size);
	}
	copySize = GET_SIZE(HDRP(ptr)); //gets size of original block
    // return origional block ptr
	if(asize == copySize)
	{
		return ptr; 
	}
    // shrink the block
	if(asize <= copySize)
	{
		size = asize;
			if(copySize - size <= MINIMUM) // then the new blck is below the minimum and we can't make a new block
			{
				return ptr; 
			}
		PUT(HDRP(ptr), PACK(size, 1)); // new header
		PUT(FTRP(ptr), PACK(size,1)); // new footer 
		PUT(HDRP(NEXT_BLKP(ptr)), PACK(copySize - size, 1)); // header of the new free block
		free(NEXT_BLKP(ptr)); // free the free next block
		return ptr;
	}
	newptr = malloc(size);  //else make new block
	
	if(!newptr) //would only reach here if malloc fails to produce new block fails 
	{
		return 0; 
	}
	
	if(size < copySize) //copy data off original block
	{
		copySize = size;
	}
		memcpy(newptr, ptr, copySize); 
		free(ptr); //free the origianl block 
		return newptr;
	
}

// sources: A Programmer's Perspective, github.com/AHAAAAAAA, github.com/ltganesa, https://danluu.com/malloc-tutorial/, github.com/Somsubhra
//https://www.cs.cmu.edu/~fp/courses/15213-s05/code/18-malloc/malloc.c


//inserts block of interest at the front of free list
static void headInsert (void* bp)
{
	NEXT_FREEP(bp) = free_listp; //sets next free block pointer to the start of free list
	PREV_FREEP(free_listp) = bp; // sets previous pointer to new block 
	PREV_FREEP(bp) = NULL; // puts previous to null
	free_listp = bp; //sets new block as the front of free list 
}

// sources: A Programmer's Perspective, github.com/AHAAAAAAA, github.com/ltganesa, https://danluu.com/malloc-tutorial/, github.com/Somsubhra
//https://www.cs.cmu.edu/~fp/courses/15213-s05/code/18-malloc/malloc.c


//removes block from free list
static void removeBlock(void* bp)
{
	//set previous block next pointer to next block if there is a previous
	if(PREV_FREEP(bp))  //if theres previous block set next pointer
	{
		NEXT_FREEP(PREV_FREEP(bp)) = NEXT_FREEP(bp);
	}
    // if at the head then set prev pointer to the previous
	else  
	{
		free_listp = NEXT_FREEP(bp); 
	}
	PREV_FREEP(NEXT_FREEP(bp)) = PREV_FREEP(bp);
}

// sources: A Programmer's Perspective, github.com/AHAAAAAAA, github.com/ltganesa, https://danluu.com/malloc-tutorial/, github.com/Somsubhra
//https://www.cs.cmu.edu/~fp/courses/15213-s05/code/18-malloc/malloc.c


void mm_checkheap(int verbose) 
{

  void *bp = heap_listp;
  
  if (verbose) {
    printf("Heap (%p):\n", heap_listp);
  }

  if ((GET_SIZE(HDRP(heap_listp)) != MINIMUM) || !GET_ALLOC(HDRP(heap_listp))) {
	printf("Bad prologue header\n");
  }
  checkblock(heap_listp);

  for (bp = heap_listp; GET_ALLOC(HDRP(bp))==0; bp = NEXT_FREEP(bp)) {
    if (verbose)  {
      printblock(bp);
    }
    checkblock(bp);
  }
     
  if (verbose) {
    printblock(bp);
  }

  if ((GET_SIZE(HDRP(bp)) != 0) || !(GET_ALLOC(HDRP(bp)))) {
    printf("Bad epilogue header\n");
  }
}
// sources: A Programmer's Perspective, github.com/AHAAAAAAA, github.com/ltganesa, https://danluu.com/malloc-tutorial/, github.com/Somsubhra
//https://www.cs.cmu.edu/~fp/courses/15213-s05/code/18-malloc/malloc.c


static void printblock(void *bp) 
{
  int hsize, halloc, fsize, falloc;

  hsize = GET_SIZE(HDRP(bp));
  halloc = GET_ALLOC(HDRP(bp));  
  fsize = GET_SIZE(FTRP(bp));
  falloc = GET_ALLOC(FTRP(bp));  
    
  if (hsize == 0) {
    printf("%p: EOL\n", bp);
    return;
  }
	
	  if (halloc) //header and footer of allocated block printed 
	  {
    printf("%p: header:[%d:%c] footer:[%d:%c]\n", bp,
      hsize, (halloc ? 'a' : 'f'),
      fsize, (falloc ? 'a' : 'f'));
	  }
  else //same if its a freed block 
  {
    printf("%p:header:[%d:%c] prev:%p next:%p footer:[%d:%c]\n",
      bp, hsize, (halloc ? 'a' : 'f'), PREV_FREEP(bp),
      NEXT_FREEP(bp), fsize, (falloc ? 'a' : 'f'));
  }

}

// sources: A Programmer's Perspective, github.com/AHAAAAAAA, github.com/ltganesa, https://danluu.com/malloc-tutorial/, github.com/Somsubhra
//https://www.cs.cmu.edu/~fp/courses/15213-s05/code/18-malloc/malloc.c

static void checkblock(void *bp) 
{

  if (NEXT_FREEP(bp)< mem_heap_lo() || NEXT_FREEP(bp) > mem_heap_hi())
    printf("Error: next pointer %p is not within heap bounds \n"
        , NEXT_FREEP(bp));
  if (PREV_FREEP(bp)< mem_heap_lo() || PREV_FREEP(bp) > mem_heap_hi())
    printf("Error: prev pointer %p is not within heap bounds \n"
        , PREV_FREEP(bp));

  if ((size_t)bp % 8)
    printf("Error: %p is not doubleword aligned\n", bp);

  
  if (GET(HDRP(bp)) != GET(FTRP(bp)))
    printf("Error: header does not match footer\n");
}