#pragma once

#include <stdio.h>
#include <stdlib.h>

typedef struct Arena Arena;

Arena* arena_create();
void* arena_malloc(Arena *arena, size_t size);
void arena_destroy(Arena *arena);

#define PAGE_SIZE 400000000

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

  while(temp != NULL)
  { 
      size_t remaining = temp->capacity - temp->used;

      if(remaining >= size){
        temp->used += size;
        return &temp->region[temp->used - size];
      }

      last = temp;
      temp = temp->next;
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