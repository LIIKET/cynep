#pragma once

typedef struct DynArray {
    uint8_t* array;
    size_t used;
    size_t size;
    size_t segment_size;
} DynArray;

DynArray DynArray_Create(size_t initial_size, size_t segment_size) {
    DynArray arr;
    arr.array = malloc(initial_size * segment_size);
    arr.used = 0;
    arr.size = initial_size;
    arr.segment_size = segment_size;

    return arr;
}

void DynArray_Resize(DynArray *arr) {
    if (arr->used == arr->size) {
        arr->size *= 2;
        arr->array = (char*)realloc(arr->array, arr->size * arr->segment_size);
    }
}

void DynArray_Append(DynArray *arr, void* element) {
    DynArray_Resize(arr);

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