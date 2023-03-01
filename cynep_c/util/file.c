#pragma once

typedef struct TextFile TextFile;

struct TextFile {
    size_t length;
    char* buffer;
    char* path;
};

TextFile* read_entire_file(char* filename) {
    int64 t1 = timestamp();

    FILE *stream;
    char *contents;
    size_t fileSize = 0;

    // Note "b" to avoid DOS/UNIX new line conversion.
    stream = fopen(filename, "rb");

    // Determine the file size
    fseek(stream, 0L, SEEK_END);
    fileSize = ftell(stream);
    fseek(stream, 0L, SEEK_SET);

    contents = malloc(fileSize + 1);

    size_t size=fread(contents, 1, fileSize, stream);
    contents[size] = 0;

    fclose(stream);

    TextFile* return_file = malloc(sizeof(TextFile));
    return_file->buffer = contents;
    return_file->path = filename;
    return_file->length = size;
 
    int64 t2 = timestamp();
    printf("File read: %d ms\n", t2/1000-t1/1000);

    return return_file;
}