#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>

#include "mm.h"
#include "memlib.h"

team_t team = {
    /* Team name */
    "Jungle Team 10",
    /* First member's full name */
    "TJ",
    /* First member's email address */
    "my-email@gmail.com",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

// mm_free, coalesce, insert_node, remove_node
#define ALIGNMENT 16
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))
#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))
#define WSIZE 8
#define DSIZE 16
#define CHUNKSIZE (1 << 12)

#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define PACK(size, alloc) ((size) | (alloc))

#define GET(p) (*(size_t *)(p)) 
#define PUT(p, val) (*(size_t *)(p) = (val))

#define GET_SIZE(p) (GET(p) & ~(ALIGNMENT - 1))
#define GET_ALLOC(p) (GET(p) & 0x1)

#define HDRP(bp) ((char *)(bp) - WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE((char *)(bp) - WSIZE))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE((char *)(bp) - DSIZE))

#define SUCP(bp) ((char *)(bp) + WSIZE)
#define PREP(bp) ((char *)(bp))

#define PRED_FREEP(bp) ((char **)(PREP(bp)))
#define SUCC_FREEP(bp) ((char **)(SUCP(bp)))

#define GET_PRED(bp) (*(char **)(PREP(bp)))
#define GET_SUCC(bp) (*(char **)(SUCP(bp)))

#define SET_PRED(bp, ptr) (GET_PRED(bp) = (ptr))
#define SET_SUCC(bp, ptr) (GET_SUCC(bp) = (ptr))

static char *heap_listp = NULL;
static char *free_listp = NULL;

static void *extend_heap(size_t words);
static void place(void *bp, size_t asize);
static void *find_fit(size_t asize);
static void *coalesce(void* bp);
static void add_free_ptr(void *bp);
static void remove_free_ptr(void *bp);
/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    if ((heap_listp = mem_sbrk(2 * WSIZE)) == (void *)-1)
        return -1;

    PUT(heap_listp, 0); // Alignment padding -> Unused Block
    PUT(heap_listp + (1 * WSIZE), PACK(DSIZE, 1)); // Prologue header 
    PUT(heap_listp + (2 * WSIZE), PACK(DSIZE, 1)); // Prologue footer
    PUT(heap_listp + (3 * WSIZE), PACK(0, 1)); // Epilogue header
    
    heap_listp += (2 * WSIZE);
    free_listp = NULL;

    if(extend_heap(CHUNKSIZE / WSIZE) == NULL)
        return -1;

    if (extend_heap(4) == NULL)                  
        return -1;
    return 0;
}

/*
 * add_free_ptr - If created new free block, then invoke add_free_ptr
 * Purpose : Add to free_ptr and change the relation between PRED and free_listp and SUCC
 */

static void add_free_ptr(void *bp)
{
    SET_SUCC(bp, free_listp);
    SET_PRED(bp, NULL);

    if (free_listp != NULL)
        SET_PRED(free_listp, bp);
    free_listp = bp;
}


/*
 * remove_free_ptr -If allocate to the free blcok, then remove_free_ptr
 * Moderate the pointer relation between successor and predecessor
 */


static void remove_free_ptr(void *bp)
{
    // Case 1: 유일한 노드
    if (GET_PRED(bp) == NULL && GET_SUCC(bp) == NULL) {
        free_listp = NULL;
    }
    // Case 2: 맨 앞 노드
    else if (GET_PRED(bp) == NULL) {
        free_listp = GET_SUCC(bp);
        SET_PRED(free_listp, NULL);
    }
    // Case 3: 맨 끝 노드
    else if (GET_SUCC(bp) == NULL) {
        SET_SUCC(GET_PRED(bp), NULL);
    }
    // Case 4: 중간 노드
    else {
        SET_SUCC(GET_PRED(bp), GET_SUCC(bp));
        SET_PRED(GET_SUCC(bp), GET_PRED(bp));
    }
}



/*
 * extend_heap - Extends the heap with a new free block of at least 'words' words.
 * Returns a pointer to the new block or NULL on failure.
 */

static void *extend_heap(size_t words)
{
    char *bp;
    size_t size;

    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
    if ((long)(bp = mem_sbrk(size)) == -1)
        return NULL;
    
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));

    add_free_ptr(bp);

    return coalesce(bp);
}


/* 가용 블록 연결하기 */
static void *coalesce(void *bp)
{
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp))); 
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp))); 
    size_t size = GET_SIZE(HDRP(bp));                   

    // Case 1. 이전 블록, 다음 블록 모두 할당된 상태
    if (prev_alloc && next_alloc)
    {
        return bp; 
    }

    // Case 2. 이전 블록은 할당된 상태, 다음 블록은 가용한 상태
    else if (prev_alloc && !next_alloc)
    {
        size += GET_SIZE(HDRP(NEXT_BLKP(bp))); 
        PUT(HDRP(bp), PACK(size, 0));          
        PUT(FTRP(bp), PACK(size, 0));          
    }

    // Case 3. 이전 블록은 가용한 상태, 다음 불록은 할당된 상태
    else if (!prev_alloc && next_alloc)
    {
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0)); 
        bp = PREV_BLKP(bp);
    }

    // Case 4. 이전 블록, 다음 블록 모두 가용한 상태
    else
    {
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
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
    
    place(bp, asize);
    return bp;
}


/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *bp)
{
    size_t size = GET_SIZE(HDRP(bp));
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    remove_free_ptr(bp);
    coalesce(bp);
}

/*
 * find_fit - function for the simple allocator
 * My solution should perform a BEST-fit search of the explicit free list
 */

static void *find_fit(size_t asize)
{
    // Best-fit
    char *bp, *min_bp = NULL;
    size_t min_size = SIZE_MAX;

    for(bp = free_listp; bp != NULL; bp = SUCC_FREEP(bp))
    {
        if (GET_SIZE(HDRP(bp)) < min_size && !GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp))))
        {
            min_size = GET_SIZE(HDRP(bp));
            min_bp = bp;
        }
    }
    return min_bp;
}

/*
 * place - My function should place the reqeusted block at the beginning of the free block,
 * splitting only if the size of the remainder would equal or exceed the minimum block size
 * 
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
 * mm_realloc - 기존에 할당된 메모리 블록의 사이즈 변경
 * 새로 할당 받은 메모리 블록에는 기존 메모리 블록의 데이터가 복사되어 들어가야 함
 */
void *mm_realloc(void *bp, size_t size)
{
    void *old_bp = bp;
    void *new_bp = bp;
    size_t copy_size;

    // size가 0인 경우 메모리 반환만 수행
    if (size <= 0)
    {
        mm_free(bp);
        return 0;
    }

    // 새로운 메모리 블록 할당하기
    new_bp = mm_malloc(size);
    if (new_bp == NULL)
        return NULL;

    // 기존 데이터 복사
    copy_size = GET(HDRP(old_bp)) - DSIZE;
    if (size < copy_size)
        copy_size = size;
    memcpy(new_bp, old_bp, copy_size);

    // 이전 메모리 블록 해제
    mm_free(old_bp);

    return new_bp;
}

