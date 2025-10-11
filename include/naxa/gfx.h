/**
 * @file gfx.h
 * @author ItsHighNoon
 * @brief Naxa graphics APIs.
 * @date 10-09-2025
 * 
 * @copyright Copyright (c) 2025
 */

#ifndef __naxa_gfx_h__
#define __naxa_gfx_h__
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include <naxa/struct.h>

/**
 * @brief Load a 3D model at a specified path.
 * 
 * @param dest A pointer to a NaxaModel_t* which will hold the allocated model.
 * @param path The path on the file system relative to the working directory.
 * @return int32_t NAXA_E_SUCCESS or an error code. On error, dest is set to NULL.
 *
 */
int32_t naxa_load_model(NaxaModel_t** dest, char* path);

/**
 * @brief Free resources associated with a 3D model.
 * 
 * @param model A pointer to the model to be freed.
 * @return int32_t NAXA_E_SUCCESS.
 */
int32_t naxa_free_model(NaxaModel_t* model);

#ifdef __cplusplus
}
#endif
#endif