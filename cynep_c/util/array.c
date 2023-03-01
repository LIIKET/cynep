#pragma once

typedef struct MemPool {
    uint8_t* bytes;
    size_t used_bytes;
    size_t capacity_bytes;
    size_t initial_size_bytes;
} MemPool;

typedef struct MemPool_Record {
    void* pointer;
    size_t index;
} MemPool_Record;

MemPool* MemPool_Make(size_t initial_size) {
    MemPool* pool = malloc(sizeof(MemPool));
    pool->bytes = malloc(initial_size);
    pool->used_bytes = 0;
    pool->capacity_bytes = initial_size;
    pool->initial_size_bytes = initial_size;

    return pool;
}

void MemPool_MaybeGrow(MemPool* pool, size_t requested_size) {
    if (pool->used_bytes + requested_size > pool->capacity_bytes) {
        pool->capacity_bytes *= 2;
        pool->bytes = realloc(pool->bytes, pool->capacity_bytes);
    }
}

MemPool_Record MemPool_GetMem(MemPool* pool, size_t size){
    MemPool_MaybeGrow(pool, size);

    MemPool_Record record = {
        .pointer = NULL
    };

    record.pointer = &pool->bytes[pool->used_bytes];
    record.index = pool->used_bytes;

    pool->used_bytes += size;

    return record;
}







