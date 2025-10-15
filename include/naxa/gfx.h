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
 * @brief Load a texture at a specified path.
 * 
 * @param dest A pointer to a NaxaTexture_t* which will hold the texture.
 * @param path The path on the file system relative to the working directory.
 * @return int32_t NAXA_E_SUCCESS or an error code. On error, dest is set to NULL.
 *
 * If the path specified matches the path of a texture that has already been loaded,
 * the texture will be fetched from memory instead of being loaded from disk
 * and its reference count will be increased.
 */
int32_t naxa_load_texture(NaxaTexture_t** dest, char* path);

/**
 * @brief Mark a texture as no longer used.
 * 
 * @param texture A pointer to the texture to be freed.
 * @return int32_t NAXA_E_SUCCESS.
 *
 * Decrease the number of references to the specified texture. If the number
 * of references reaches 0, the texture will be deleted via glDeleteTextures
 * and the NaxaTexture_t will be returned to the free list.
 */
int32_t naxa_free_texture(NaxaTexture_t* texture);

/**
 * @brief Load a 3D model at a specified path.
 * 
 * @param dest A pointer to a NaxaModel_t* which will hold the allocated model.
 * @param path The path on the file system relative to the working directory.
 * @return int32_t NAXA_E_SUCCESS or an error code. On error, dest is set to NULL.
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