#pragma once

// Array arr = Array_Create(1, sizeof(uint64_t));
// uint64_t val = 5;
// uint64_t val2 = 10;
// Array_Append(&arr, &val);
// Array_Append(&arr, &val2);
// uint64_t test = arr.array[1 * arr.segment_size];

typedef struct Array {
    uint8_t* array;
    size_t used;
    size_t size;
    size_t segment_size;
} Array;

Array Array_Create(size_t initial_size, size_t segment_size) {
    Array arr;
    arr.array = malloc(initial_size * segment_size);
    arr.used = 0;
    arr.size = initial_size;
    arr.segment_size = segment_size;

    return arr;
}

void Array_Resize(Array *arr) {
    if (arr->used == arr->size) {
        arr->size *= 2;
        arr->array = (char*)realloc(arr->array, arr->size * arr->segment_size);
    }
}

void Array_Append(Array *arr, void* element) {
    Array_Resize(arr);

    memcpy(&arr->array[arr->used * arr->segment_size], element, arr->segment_size);
    // arr->array[arr->used++] = element;
    arr->used = 1;
}

// size_t array_count(array_t *arr) {
//     return arr->used;
// }

// void array_insert_before(array_t *arr, char element, size_t index) {
//     _array_resize_if_tapped(arr);

//     size_t position = index + 1;

//     arr->used++;
 
//     // shift elements forward
//     for (size_t i = arr->used - 1; i >= position; i--){
//         arr->array[i] = arr->array[i - 1];
//     }

//     arr->array[position - 1] = element;
// }

// void array_free(array_t *arr) {
//     free(arr->array);
//     arr->array = NULL;
//     arr->used = arr->size = 0;
// }