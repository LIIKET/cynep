#pragma once

typedef struct SourceFile SourceFile;

struct SourceFile
{
    size_t length;
    char* buffer;
    char* path;
};

SourceFile* File_Read_Text(char* filename)
{
    int64 t1 = timestamp();

    size_t result_bytes = 0;
    size_t chunk_bytes = 1000000; // 50000000 = 50 Megabytes

    char* buffer = (char*)malloc(chunk_bytes);
    char* result = NULL;

    FILE *file = NULL;
    size_t read_bytes = 0;

    file = fopen(filename, "r");

    if (file == NULL){
        // TODO: Handle failed to open file
    }

    while ((read_bytes = fread(buffer, 1, chunk_bytes, file)) > 0){
        result = (char*)realloc(result, result_bytes + read_bytes);
        memcpy(result + result_bytes, buffer, read_bytes);
        result_bytes = result_bytes + read_bytes;

        if (ferror(file)) {
            // TODO: Deal with potential errors?
        }
    }

    // Null terminate
    result = (char*)realloc(result, result_bytes + sizeof(char));
    result[result_bytes / sizeof(char)] = NULL_CHAR;

    fclose(file);
    free(buffer);

    SourceFile* return_file = malloc(sizeof(SourceFile));
    return_file->buffer = result;
    return_file->path = filename;
    return_file->length = result_bytes / sizeof(char);

    // size_t result_length = result_bytes / sizeof(char);

    //size_t test = getSize(result);
    // for (size_t i = 0; i < result_length; i++)
    // {
    //     if(result[i]== EOF) {
    //         break;
    //     }

    //     printf("%c", result[i]);
    // }
    
    int64 t2 = timestamp();

    printf("File read: %d ms\n", t2/1000-t1/1000);

    return return_file;
}


