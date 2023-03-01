#pragma once

#include <stdio.h>
#include <stdlib.h>

typedef struct Arena Arena;

Arena* arena_create();
void* arena_malloc(Arena *arena, size_t size);
void arena_destroy(Arena *arena);

struct Arena {
  uint8_t* region;
  size_t capacity;
  size_t used;
  Arena* next;
};

static Arena* _arena_create(size_t size) {
  Arena* arena = malloc(sizeof(Arena));
  arena->region = malloc(size * sizeof(uint8_t));
  arena->capacity = size;
  arena->next = NULL;
  arena->used = 0;

  return arena;
}

Arena* arena_create(size_t initial_size) {
  return _arena_create(initial_size);
}

void* arena_alloc(Arena* arena, size_t size) {
  Arena* last = arena;
  Arena* temp = arena;

  while(temp != NULL) { 
      last = temp;
      temp = temp->next;
  }

  // We could do this check for every node, but we mostly allocate same size elements anyway.
  // Hence it would probably never find a slot in a block that it didn't previously.
  size_t remaining = last->capacity - last->used;
  
  if(remaining >= size) {
    last->used += size;
    return &last->region[last->used - size];
  }

  size_t allocation_size = size > last->capacity ? size : last->capacity * 2;

  Arena* next = _arena_create(allocation_size);

  last->next = next;
  next->used += size;

  return next->region;
}

void arena_destroy(Arena *arena) {
  Arena *next, *last = arena;
  do {
    next = last->next;
    free(last->region);
    free(last);
    last = next;
  } 
  while(next != NULL);
}