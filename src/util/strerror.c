#include <naxa/err.h>

char* STRERROR_TABLE[] = {
    [NAXA_E_SUCCESS] =      "Success",
    [NAXA_E_INTERNAL] =     "Internal error",
    [NAXA_E_BOUNDS] =       "Out of bounds",
    [NAXA_E_EXHAUSTED] =    "Exhausted",
    [NAXA_E_TOOLONG] =      "String too long",
    [NAXA_E_FILE] =         "File error",
};

char* naxa_strerror(int32_t error) {
    if (error < 0 || error >= sizeof(STRERROR_TABLE) / sizeof(char*)) {
        return STRERROR_TABLE[NAXA_E_BOUNDS];
    }
    return STRERROR_TABLE[error];
}