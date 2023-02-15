#pragma once

typedef struct smart_array_t {
    void* array;
    size_t used;
    size_t size;
    size_t _segment_bytes;
} smart_array_t;

smart_array_t array_create(size_t initial_size, size_t segment_bytes) {
    smart_array_t arr;
    arr.array = (void*)malloc(initial_size * segment_bytes);
    arr.used = 0;
    arr.size = initial_size;

    return arr;
}

void _array_resize_if_tapped(smart_array_t *arr) {
    if (arr->used == arr->size) {
        arr->size *= 2;
        arr->array = (char*)realloc(arr->array, arr->size * arr->_segment_bytes);
    }
}

// void array_append(smart_array_t *arr, void* element) {
//     _array_resize_if_tapped(arr);

//     arr->array[arr->used++] = element;
// }

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