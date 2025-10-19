#include <stdio.h>
#include <string.h>

#include <naxa/naxa_internal.h>

uint32_t hash_code(char* string) {
    uint32_t hash = 7;
    int32_t length = strlen(string);
    for (int32_t i = 0; i < length; i++) {
        hash = hash * 31 + string[i];
    }
    return 0;
}

char* read_file_into_buffer(FILE* fp, uint32_t* len) {
    if (fp == NULL) {
        return NULL;
    }
    fseek(fp, 0l, SEEK_END);
    long file_len = ftell(fp);
    if (file_len == -1) {
        return NULL;
    }
    char* data = malloc(file_len + 1);
    fseek(fp, 0l, SEEK_SET);
    file_len = fread(data, 1, file_len, fp);
    data[file_len] = '\0'; // In case we are treating this as a C string
    if (len != NULL) {
        *len = (uint32_t)file_len;
    }
    return data;
}