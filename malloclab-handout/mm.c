/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "HOLIC0105",
    /* First member's full name */
    "holic",
    /* First member's email address */
    "3485995896@qq.com",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

#define WSIZE       4           //Word and header/footer size (bytes)
#define DSIZE       8           //Double word size (bytes) 
#define CHUNKSIZE   (1 << 12)   //Extend heap by this amout (bytes)

// Pack a size and allocated bit into a word
#define PACK(size, alloc)           ((size) | (alloc))

//Read and write a word at address p
#define GET(p)                      (*(unsigned int *)(p))
#define PUT(p, val)                 (*(unsigned int *)(p) = (val))

//Write an address at address p
#define PUTPTR(p, val)              (*(void **)(p) = val)

//UPDATE the flag of whether the prev block is empty
#define UPDATE_prev_EMPTY(p, val)   (*(unsigned int *)(p) |= (val << 1))

//Read the size and allocated fields from address p
#define GET_SIZE(p)                 (GET(p) & ~0x7)
#define GET_ALLOC(p)                (GET(p) & 0x1)

//GET whether the prev block is empty
#define GET_PREV_EMPTY(p)           (GET(p) & (0x2))

//Given block ptr bp, compute address of its header and footer
#define HDRP(bp)                    ((char *)(bp) - WSIZE)
#define FTRP(bp)                    ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

//Given block ptr bp, compute address of next and previous blocks
#define NEXT_BLKP(bp)               ((char *)(bp) + GET_SIZE((void *)(bp) - WSIZE))
#define PREV_BLKP(bp)               ((char *)(bp) - GET_SIZE((void *)(bp) - DSIZE))

//Get the address of the prev and next block
#define GET_PREV_ADDRESS(bp)        (*((void **)(bp) + 1))
#define GET_NEXT_ADDRESS(bp)        (*(void **)(bp))

#define CLASS 20

#define HEAP_HEAD_SIZE (SIZE_T_SIZE * CLASS) 

char *heap_head_list;

static void eraseFromFreeList(void * bp) {
    return;
    void * prev = GET_PREV_ADDRESS(bp);
    void * next = GET_NEXT_ADDRESS(bp);

    PUTPTR(GET_NEXT_ADDRESS(prev), next);
    PUTPTR(GET_PREV_ADDRESS(next), prev);
}

static void * coalesce(void * bp) {
    void * mid_bp = bp;
    void * mid_bp_header = HDRP(bp);

    void * next_bp = NEXT_BLKP(bp);
    void * next_bp_header = HDRP(next_bp);

    int preflag = GET_PREV_EMPTY(mid_bp_header); //Whether prev block empty
    int nextflag = GET_ALLOC(next_bp_header);//Whether next block empty
    if(!preflag && !nextflag) {
        UPDATE_prev_EMPTY(next_bp_header, 1);
        return mid_bp;
    } else if(preflag && !nextflag) {
        void * prev_bp = PREV_BLKP(bp);
        void * prev_bp_header = HDRP(prev_bp);
        int siz = GET_SIZE(mid_bp_header) + GET_SIZE(prev_bp_header);
        PUT(prev_bp_header, PACK(siz, 0));
        PUT(FTRP(prev_bp), PACK(siz, 0));
        UPDATE_prev_EMPTY(next_bp_header, 1);
        eraseFromFreeList(mid_bp);  
        return prev_bp;
    } else if(!preflag && nextflag) {
        int siz = GET_SIZE(mid_bp_header) + GET_SIZE(next_bp_header);
        PUT(mid_bp_header, PACK(siz, 0));
        PUT(FTRP(mid_bp), PACK(siz, 0));
        eraseFromFreeList(next_bp);
        return mid_bp;
    } else {
        void * prev_bp = PREV_BLKP(bp);
        void * prev_bp_header = HDRP(prev_bp);
        int siz = GET_SIZE(mid_bp_header) + GET_SIZE(prev_bp_header) + GET_SIZE(next_bp_header);
        PUT(prev_bp_header, PACK(siz, 0));
        PUT(FTRP(prev_bp), PACK(siz, 0));
        eraseFromFreeList(mid_bp);
        eraseFromFreeList(next_bp);
        return prev_bp;
    }
}

static void * extendHeap(int num) {
    int siz = (num & 1 ? num + 1 : num) * WSIZE; //align dsize

    void * bp = mem_sbrk(siz);

    if(bp == (void *) -1) return NULL;

    PUT(HDRP(bp), PACK(siz, 0)); //update header
    PUT(FTRP(bp), PACK(siz, 0)); //update footer
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); //update the last block header
    return coalesce(bp);
}

/* 
 * mm_init - initialize the malloc package.
 */

int mm_init(void)
{
    if((heap_head_list = mem_sbrk(HEAP_HEAD_SIZE + (WSIZE << 2))) == (void *) -1) 
        return -1;
    for(int i = 0; i < CLASS; i ++) {
       // PUTPTR((heap_head_list + SIZE_T_SIZE * i), NULL);
    }
    
    PUT(heap_head_list + HEAP_HEAD_SIZE, 0);
    PUT(heap_head_list + HEAP_HEAD_SIZE, PACK(DSIZE, 1));
    PUT(heap_head_list + HEAP_HEAD_SIZE, PACK(DSIZE, 1));
    PUT(heap_head_list + HEAP_HEAD_SIZE, PACK(0, 1));
    if(extendHeap(CHUNKSIZE / WSIZE) == NULL)
        return -1;
    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    int newsize = ALIGN(size + SIZE_T_SIZE);
    void *p = mem_sbrk(newsize);
    if (p == (void *)-1)
	return NULL;
    else {
        *(size_t *)p = size;
        return (void *)((char *)p + SIZE_T_SIZE);
    }
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;
    
    newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;
    copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
    if (size < copySize)
      copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}














