/**
 * @file err.h
 * @author ItsHighNoon
 * @brief Error codes.
 * @date 10-01-2025
 * 
 * @copyright Copyright (c) 2025
 */

#ifndef __naxa_err_h__
#define __naxa_err_h__
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define NAXA_E_SUCCESS 0
#define NAXA_E_INTERNAL 1
#define NAXA_E_BOUNDS 2
#define NAXA_E_EXHAUSTED 3
#define NAXA_E_TOOLONG 4
#define NAXA_E_FILE 5
#define NAXA_E_NULLPTR 6

/**
 * @brief Naxa version of strerror.
 * 
 * @param error The error code returned from Naxa.
 * @return char* A text representation of the given error code.
 */
extern const char* naxa_strerror(int32_t error);

#ifdef __cplusplus
}
#endif
#endif