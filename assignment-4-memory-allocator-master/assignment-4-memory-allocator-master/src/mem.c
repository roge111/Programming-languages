#define _DEFAULT_SOURCE

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "mem_internals.h"
#include "mem.h"
#include "util.h"

void debug_block(struct block_header* b, const char* fmt, ... );
void debug(const char* fmt, ... );

extern inline block_size size_from_capacity( block_capacity cap );
extern inline block_capacity capacity_from_size( block_size sz );

static bool            block_is_big_enough( size_t query, struct block_header* block ) { return block->capacity.bytes >= query; }
static size_t          pages_count   ( size_t mem )                      { return mem / getpagesize() + ((mem % getpagesize()) > 0); }
static size_t          round_pages   ( size_t mem )                      { return getpagesize() * pages_count( mem ) ; }

static void block_init( void* restrict addr, block_size block_sz, void* restrict next ) {
  *((struct block_header*)addr) = (struct block_header) {
    .next = next,
    .capacity = capacity_from_size(block_sz),
    .is_free = true
  };
}

static size_t region_actual_size( size_t query ) { return size_max( round_pages( query ), REGION_MIN_SIZE ); }

extern inline bool region_is_invalid( const struct region* r );



static void* map_pages(void const* addr, size_t length, int additional_flags) {
  return mmap( (void*) addr, length, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | additional_flags , -1, 0 );
}

/*  аллоцировать регион памяти и инициализировать его блоком */
static struct region alloc_region  ( void const * addr, size_t query ) {
  /*  ??? */
  struct region new_region;
  new_region.size = new_region.size > REGION_MIN_SIZE ? new_region.size : REGION_MIN_SIZE;
  new_region.extends = true;
  new_region.addr = map_pages(addr, new_region.size, MAP_FIXED_NOREPLACE);
  if (new_region.addr == MAP_FAILED) {
    printf("Region allocation failed!\n");
    return REGION_INVALID;
  }

  struct block_header* header = (struct block_header*) new_region.addr;
  header->capacity.bytes = new_region.size - offsetof(struct block_header, contents);
  header->next = NULL;
  header->is_free = true;

  return new_region;
}

void dump_headers(struct block_header* block) {
  printf("Dumping blocks\n");
  printf("Address   Capacity Next \n");
  while (block) {
    printf("%p ", block);
    printf("%*lu ", 10, block->capacity.bytes);
    printf("%p ", block->next);
    if (block->is_free) {
      printf("free\n");
    } else {
      printf("allocated\n");
    }

    block = block->next;
  }

  printf("End of blocks list\n");
}

static void* block_after( struct block_header const* block )         ;

void* heap_init( size_t initial ) {
  const struct region region = alloc_region( HEAP_START, initial );
  if ( region_is_invalid(&region) ) return NULL;

  dump_headers(region.addr);

  return region.addr;
}

#define BLOCK_MIN_CAPACITY 24

/*  --- Разделение блоков (если найденный свободный блок слишком большой )--- */

static bool block_splittable( struct block_header* restrict block, size_t query) {
  return block-> is_free && query + offsetof( struct block_header, contents ) + BLOCK_MIN_CAPACITY <= block->capacity.bytes;
}

static bool split_if_too_big( struct block_header* block, size_t query ) {
  /*  ??? */
  struct block_header* next = NULL;
  printf("splitting block %p to size %lu\n", block, query);
  if (query < BLOCK_MIN_CAPACITY) {
      query = BLOCK_MIN_CAPACITY;
  }

  if (!block_splittable(block, query)) {
    printf("not splittable\n");
    block->is_free = false;
    return false;
  }

  printf("splittable\n");

  next = (struct block_header*) (block->contents + query);
  printf("privet\n");
  next->capacity.bytes = block->capacity.bytes - query - offsetof(struct block_header, contents);
  next->is_free = true;
  next->next = block->next;
  block->capacity.bytes = query;
  block->is_free = false;
  block->next = next;

  printf("=======================\n");
  dump_headers(block);
  printf("=======================\n");

  return true;
}


/*  --- Слияние соседних свободных блоков --- */

static void* block_after( struct block_header const* block )              {
  return  (void*) (block->contents + block->capacity.bytes);
}
static bool blocks_continuous (
                               struct block_header const* fst,
                               struct block_header const* snd ) {
  return (void*)snd == block_after(fst);
  printf("fst %p, snd %p, fst_cont %p, fst_cpc %p\n", fst, snd, fst->contents, fst->contents + fst->capacity.bytes);
}

static bool mergeable(struct block_header const* restrict fst, struct block_header const* restrict snd) {
  return fst->is_free && snd->is_free && blocks_continuous( fst, snd ) ;
}

static bool try_merge_with_next( struct block_header* block ) {
  /*  ??? */
  printf("TRYING TO MERGE %p %p\n", block, block->next);
  if (block->next && mergeable(block, block->next)) {
      block->capacity.bytes += block->next->capacity.bytes + offsetof(struct block_header, contents);
      block->next = block->next->next;
      return true;
  }

  return false;
}


/*  --- ... ecли размера кучи хватает --- */

struct block_search_result {
  enum {BSR_FOUND_GOOD_BLOCK, BSR_REACHED_END_NOT_FOUND, BSR_CORRUPTED} type;
  struct block_header* block;
};


static struct block_search_result find_good_or_last  ( struct block_header*  block, size_t sz )    {
  /*??? */
  struct block_search_result bsr;
  printf("BLOCK_INIT %p\n", block->next);

  while (block->next) {
    printf("BLOCK %p\n", block);
    if (block->capacity.bytes >= sz && block->is_free) {
      bsr.block = block;
      bsr.type = BSR_FOUND_GOOD_BLOCK;
      return bsr;
    }

    block = block->next;
  }

printf("block\n");
  if (block->capacity.bytes >= sz) {
      bsr.block = block;
      bsr.type = BSR_FOUND_GOOD_BLOCK;
      printf("bsr_found_good_block\n");
      return bsr;
  }

  bsr.block = block;
  bsr.type = BSR_REACHED_END_NOT_FOUND;
  return bsr;
}

/*  Попробовать выделить память в куче начиная с блока `block` не пытаясь расширить кучу
 Можно переиспользовать как только кучу расширили. */
static struct block_search_result try_memalloc_existing ( size_t query, struct block_header* block ) {
  struct block_search_result bsr;
  bsr = find_good_or_last(block, query);
  if (bsr.type != BSR_FOUND_GOOD_BLOCK) {
      return bsr;
  }

  if (split_if_too_big(bsr.block, query)) {
      //printf("Added new block, split\n");
  } else {
      //printf("Added new block, not split\n");
  }

  return bsr;
}



static struct block_header* grow_heap( struct block_header* restrict last, size_t query ) {
  /*  ??? */
  struct block_header* header = NULL;
  header = map_pages(last->contents + last->capacity.bytes, REGION_MIN_SIZE, MAP_FIXED);
  if (header == MAP_FAILED) {
    return NULL;
  }

  if ((last->contents + last->capacity.bytes == header) && last->is_free) {
    last->capacity.bytes += REGION_MIN_SIZE;
    return last;
  }
}

/*  Реализует основную логику malloc и возвращает заголовок выделенного блока */
static struct block_header* memalloc( size_t query, struct block_header* heap_start) {
  /*  ??? */
  struct block* new_block = NULL;
  struct block_search_result bsr;

  bsr = try_memalloc_existing(query, heap_start);
  if (bsr.type == BSR_FOUND_GOOD_BLOCK) {
    //printf("good block!\n");
  } else if (bsr.type == BSR_REACHED_END_NOT_FOUND) {
    new_block = grow_heap(bsr.block, query);
    if (!new_block) {
      return NULL;
    }
    try_memalloc_existing(query, new_block);
    bsr.block = new_block;
    //printf("bad block!\n");
  } else {
    return NULL;
  }
  
  dump_headers(heap_start);
  return bsr.block;
}

void* _malloc( size_t query ) {
  struct block_header* const addr = memalloc( query, (struct block_header*) HEAP_START );
  if (addr) return addr->contents;
  else return NULL;
}

static struct block_header* block_get_header(void* contents) {
  return (struct block_header*) (((uint8_t*)contents)-offsetof(struct block_header, contents));
}

void _free( void* mem ) {
  if (!mem) return ;
  struct block_header* header = block_get_header( mem );

  header->is_free = true;
  while (try_merge_with_next(header)) {
      //printf("merged with next\n");
  }

  dump_headers(HEAP_START);
  /*  ??? */
}
