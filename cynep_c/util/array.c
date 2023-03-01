#pragma once

typedef struct MemPool {
    uint8_t* bytes;
    size_t size;
    size_t capacity;
} MemPool;

typedef struct MemPool_Record {
    void* pointer;
    size_t index;
} MemPool_Record;

MemPool* MemPool_Make(size_t initial_size) {
    MemPool* pool = malloc(sizeof(MemPool));
    pool->bytes = malloc(initial_size);
    pool->size = 0;
    pool->capacity = initial_size;
    return pool;
}

void MemPool_MaybeGrow(MemPool* pool, size_t requested_size) {
    if (pool->size + requested_size > pool->capacity) {
        pool->capacity *= 2;
        pool->bytes = realloc(pool->bytes, pool->capacity);
    }
}

MemPool_Record MemPool_GetMem(MemPool* pool, size_t size){
    MemPool_MaybeGrow(pool, size);

    MemPool_Record record = {
        .pointer = NULL
    };

    record.pointer = &pool->bytes[pool->size];
    record.index = pool->size;

    pool->size += size;

    return record;
}







