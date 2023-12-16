#include "tests.h"

static struct block_header *block_get_header(void *contents) {
  return (struct block_header *)(((uint8_t *)contents) -
                                 offsetof(struct block_header, contents));
}

static void test1() {
  printf("\n--------- TEST 1 ---------\n\n");
  void *heap = heap_init(42069);
  if (!heap) {
    error("heap wanted to initialize but got tired");
  }
  log_message("heap initialized");
  debug_heap(stdout, heap);
  void *a = _malloc(1000);
  if (!a) {
    error("memory wanted to allocate but got tired");
  }
  log_message("memory (a) allocated");
  debug_heap(stdout, heap);
  _free(a);
  log_message("memory (a) freed");
  debug_heap(stdout, heap);
  munmap(heap, size_from_capacity((block_capacity){.bytes = 42069}).bytes);
  printf("\n--------- TEST 1 PASSED ---------\n");
}

static void test2() {
  printf("\n--------- TEST 2 ---------\n\n");
  void *heap = heap_init(42069);
  if (!heap) {
    error("heap wanted to initialize but got tired");
  }
  log_message("heap initialized");
  debug_heap(stdout, heap);
  void *a = _malloc(222);
  void *b = _malloc(333);
  void *c = _malloc(444);
  if (!a || !b || !c) {
    error("memory wanted to allocate but got tired");
  }
  log_message("memory allocated");
  debug_heap(stdout, heap);
  _free(b);
  log_message("memory (b) freed");
  debug_heap(stdout, heap);
  _free(a);
  _free(c);
  munmap(heap, size_from_capacity((block_capacity){.bytes = 42069}).bytes);
  printf("\n--------- TEST 2 PASSED ---------\n");
}

static void test3() {
  printf("\n--------- TEST 3 ---------\n\n");
  void *heap = heap_init(42069);
  if (!heap) {
    error("heap wanted to initialize but got tired");
  }
  log_message("heap initialized");
  debug_heap(stdout, heap);
  void *a = _malloc(222);
  void *b = _malloc(333);
  void *c = _malloc(444);
  void *d = _malloc(555);
  if (!a || !b || !c || !d) {
    error("memory wanted to allocate but got tired");
  }
  log_message("memory allocated");
  debug_heap(stdout, heap);
  _free(c);
  log_message("memory (c) freed");
  debug_heap(stdout, heap);
  _free(b);
  log_message("memory (b) freed");
  debug_heap(stdout, heap);
  void *e = _malloc(69);
  if (!e) {
    error("memory wanted to allocate but got tired");
  }
  log_message("memory allocated");
  debug_heap(stdout, heap);
  _free(a);
  _free(e);
  _free(d);
  debug_heap(stdout, heap);
  munmap(heap, size_from_capacity((block_capacity){.bytes = 42069}).bytes);
  printf("\n--------- TEST 3 PASSED ---------\n");
}

static void test4() {
  printf("\n--------- TEST 4 ---------\n\n");
  void *heap = heap_init(10000);
  if (!heap) {
    error("heap wanted to initialize but got tired");
  }
  debug_heap(stdout, heap);
  void *hello_there = _malloc(69696);
  if (!hello_there) {
    error("memory wanted to allocate but got tired");
  }
  log_message("memory allocated");
  debug_heap(stdout, heap);
  _free(hello_there);
  log_message("memory freed");
  debug_heap(stdout, heap);
  munmap(heap, size_from_capacity((block_capacity){.bytes = 85999}).bytes);
  printf("\n--------- TEST 4 PASSED ---------\n");
}

static void test5() {
  printf("\n--------- TEST 5 ---------\n\n");
  void *heap = heap_init(1);
  if (!heap) {
    error("heap wanted to initialize but got tired");
  }
  debug_heap(stdout, heap);
  void *a = _malloc(1024);
  if (!a) {
    error("memory wanted to allocate but got tired");
  }
  log_message("memory allocated");
  debug_heap(stdout, heap);
  struct block_header *header = block_get_header(a);
  (void)mmap(header->contents + header->capacity.bytes, REGION_MIN_SIZE,
             PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_FIXED, -1, 0);
  void *b = _malloc(4096);
  if (!b) {
    error("memory wanted to allocate but got tired");
  }
  log_message("memory allocated");
  debug_heap(stdout, heap);
  _free(a);
  _free(b);
  munmap(header->next, REGION_MIN_SIZE);
  munmap(heap, REGION_MIN_SIZE);
  printf("\n--------- TEST 5 PASSED ---------\n");
}

void invokeTests() {
  test1();
  test2();
  test3();
  test4();
  test5();
}