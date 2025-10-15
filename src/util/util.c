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