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
    "Supoer Nova",
    /* First member's full name */
    "TAEJU AHN",
    /* First member's email address */
    "Echaismine@naver.com",
    /* Second member's full name (leave blank if none) */
    "WOOJIN SIN",
    /* Second member's email address (leave blank if none) */
    "idontknow@naver.com",
    /* Third member's full name (leave blank if none) */
    "SEOKPYO HONG",
    /* Third member's email address (leave blank if none) */
    "idontknow@naver.com",
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8
#define WSIZE 4
#define DSIZE 8
#define CHUNKSIZE (1<<12)

#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define PACK(size, alloc) ((size) | (alloc))

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)
#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

#define GET(p) (*(size_t *)(p))
#define PUT(p, val) (*(size_t *)(p) = (val))

#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

#define HDRP(bp) ((char *)(bp) - WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE((char *)(bp) - DSIZE))

static char* heap_listp = NULL;

static void *extend_heap(size_t words);
static void place(void *bp, size_t asize);
static void *find_fit(size_t asize);
static void *coalesce(void* bp);
static void mm_checkheap(int verbose);
static void printblock(void *bp);
static void checkblock(void *bp);

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    if ((heap_listp = mem_sbrk(4 * WSIZE)) == (void *)-1)
        return -1;
    
    PUT(heap_listp, 0); // Alignment padding -> Unused Block
    PUT(heap_listp + (1 * WSIZE), PACK(DSIZE, 1)); // Prologue header (16 byte)
    PUT(heap_listp + (2 * WSIZE), PACK(DSIZE, 1)); // Prologue footer (16 byte)
    PUT(heap_listp + (3 * WSIZE), PACK(0, 1)); // Epilogue header
    heap_listp += DSIZE;

    if(extend_heap(CHUNKSIZE/WSIZE) == NULL)
        return -1;
    return 0;
}

/*
 * extend_heap - Extends the heap with a new free block of at least 'words' words.
 * Returns a pointer to the new block or NULL on failure.
 */

static void *extend_heap(size_t words)
{
    char *bp;
    size_t size;

    // size는 총 할당 free block을 의미한다
    // bp = 이전 brk를 가리키고 있다
    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
    if((bp = mem_sbrk(size)) == (void *)-1)
        return NULL;
    
    // bp를 -WSIZE만큼 이동하면, epilogue block이 나오고, 이를 가용 가능(free)block으로 할당한다
    // FTRP(bp), PACK(size, 0)을 통해서, footer block 또한 생성한다
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));

    return coalesce(bp);
}


static void *coalesce(void *bp)
{
    size_t PREV_ALLOC = GET_ALLOC(HDRP(PREV_BLKP(bp)));
    size_t NEXT_ALLOC = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    char *prev_bp = PREV_BLKP(bp);
    char *next_bp = NEXT_BLKP(bp);

    if(PREV_ALLOC && NEXT_ALLOC)
        return bp;
    else if (!PREV_ALLOC && NEXT_ALLOC)
    {
        size += GET_SIZE(HDRP(prev_bp));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(prev_bp), PACK(size, 0));
        bp = prev_bp;
    }
    else if (PREV_ALLOC && !NEXT_ALLOC)
    {
        size += GET_SIZE(HDRP(next_bp));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(next_bp), PACK(size, 0));
    }
    else
    {
        size += GET_SIZE(HDRP(prev_bp)) + GET_SIZE(HDRP(next_bp));
        PUT(FTRP(next_bp), PACK(size, 0));
        PUT(HDRP(prev_bp), PACK(size, 0));
        bp = prev_bp;
    }
    return bp;
}


/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    size_t asize, extendsize;
    char *bp;

#ifdef DEBUG
    mm_checkheap(1);
#endif

    if (size <= 0)
        return NULL;
    
    if (size <= DSIZE)
        asize = 2 * DSIZE;
    else
        asize = ALIGN(size + DSIZE);
    
    if ((bp = find_fit(asize)) != NULL) {
        place(bp, asize);
        return bp;
    }

    extendsize = MAX(CHUNKSIZE, asize);
    
    if ((bp = extend_heap(extendsize/WSIZE)) == NULL)
        return NULL;
    place(bp, extendsize);
    return bp;

    // Original code but recently not used
    // size_t newsize = ALIGN(size + SIZE_T_SIZE);
    // void *p = mem_sbrk(newsize);

    // if (p == (void *)-1)
	//     return NULL;
    // else {
    //     *(size_t *)p = size;
    //     return (void *)((char *)p + SIZE_T_SIZE);
    // }
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *bp)
{
    size_t size = GET_SIZE(HDRP(bp));
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    coalesce(bp);
}

/*
 * find_fit - function for the simple allocator described in Section 9.9.12
 * My solution should perform a first-fit search of the impliticit free list
 */

static void *find_fit(size_t asize)
{
    char* bp;

    for(bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp))
    {
        if(!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp))))
            return bp;
    }
    return NULL;
}


/*
 * place - My function should place the reqeusted block at the beginning of the free block,
 * splitting only if the size of the remainder would equal or exceed the minimum block size
 * 
 * 요청된 블록을 가용 블록의 시작 부분에 배치해야 하며,
 * 나머지 크기가 최소 블록 크기와 같거나 이를 초과할 경우에만 분할해야 한다.
 */

static void place(void *bp, size_t asize)
{
    size_t fsize = GET_SIZE(HDRP(bp));

    if((fsize - asize) >= (2 * DSIZE)) {
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK(fsize - asize, 0));
        PUT(FTRP(bp), PACK(fsize - asize, 0));
    } else {
        PUT(HDRP(bp), PACK(fsize, 1));
        PUT(FTRP(bp), PACK(fsize, 1));
    }
}


/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */

 void *mm_realloc(void *ptr, size_t size) {
    if (ptr == NULL) {
        return mm_malloc(size);
    }
    if (size == 0) {
        mm_free(ptr);
        return NULL;
    }

    size_t asize = ALIGN(size + DSIZE);
    size_t oldsize = GET_SIZE(HDRP(ptr));
    if (asize <= oldsize) { 
        place(ptr, asize);
        return ptr;
    }

    void *newptr = mm_malloc(size);
    if (newptr == NULL)
        return NULL;

    size_t copysize = GET_SIZE(HDRP(ptr)) - DSIZE; // 페이로드 크기
    if (size < copysize)
        copysize = size;
    memcpy(newptr, ptr, copysize);
    mm_free(ptr);
    return newptr;

    // Previous Code
    // void *oldptr = ptr;
    // void *newptr;
    // size_t copySize;
    
    // newptr = mm_malloc(size);
    // if (newptr == NULL)
    //   return NULL;
    // copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
    // if (size < copySize)
    //   copySize = size;
    // memcpy(newptr, oldptr, copySize);
    // mm_free(oldptr);
    // return newptr;
}

/*
 * mm_checkheap - Check the heap for consistency
 */

static void mm_checkheap(int verbose)
{
    char *bp = heap_listp;

    if(verbose)
        printf("Heap: %p\n", heap_listp);
    
    if((GET_SIZE(HDRP(bp))) != DSIZE || !GET_ALLOC(HDRP(bp)))
        printf("Bad prologue header\n");
    checkblock(bp);

    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp=NEXT_BLKP(bp))
    {
        if(verbose)
            printblock(bp);
        checkblock(bp);

        if(!GET_ALLOC(HDRP(bp)) && !GET_ALLOC(HDRP(NEXT_BLKP(bp))))
            printf("ERROR: adjacent free blocks at %p\n", bp);
    }
    
    if(verbose)
        printblock(bp);
    if((GET_SIZE(HDRP(bp)) != 0 || !GET_ALLOC(HDRP(bp))))
        printf("Bad epilogue header\n");
}

static void printblock(void *bp)
{
    size_t hsize, halloc, fsize, falloc;

    hsize = GET_SIZE(HDRP(bp));
    halloc = GET_ALLOC(HDRP(bp));
    fsize = GET_SIZE(FTRP(bp));
    falloc = GET_ALLOC(FTRP(bp));

    if (hsize == 0) {
        printf("%p : EOL\n", bp);
        return;
    }

    printf("%p : header : [%zu:%c] footer : [%zu:%c]\n", bp, hsize, (halloc ? 'a' : 'f'), fsize, (falloc ? 'a' : 'f'));
}

static void checkblock(void *bp) {
    if ((size_t)bp % 8) {
        printf("Error: %p is not doubleword aligned\n", bp);
    } else if (GET(HDRP(bp)) != GET(FTRP(bp))) {
        printf("Error: header does not match footer at %p\n", bp);
    } else {
        printf("Block is consistent at %p\n", bp);
    }
}