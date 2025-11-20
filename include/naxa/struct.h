/**
 * @file struct.h
 * @author ItsHighNoon
 * @brief Naxa structure definitions for the application.
 * @date 10-09-2025
 * 
 * @copyright Copyright (c) 2025
 */

#ifndef __naxa_struct_h__
#define __naxa_struct_h__
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include <cglm/cglm.h>

/**
 * @brief A texture in VRAM managed by the Naxa loader.
 */
typedef struct NaxaTexture {
    uint32_t texture;
    int32_t refs;
    char* path;
    struct NaxaTexture* next;
} NaxaTexture_t;

/**
 * @brief An individually renderable portion of a NaxaModel_t.
 */
typedef struct {
    int32_t vertex_count;
    int32_t offset;
    NaxaTexture_t* diffuse;
} NaxaSubmodel_t;

/**
 * @brief A bone that is part of the skeleton of a NaxaModel_t.
 */
typedef struct {
    char* name;
    int32_t index;
    mat4 matrix;
} NaxaBone_t;

/**
 * @brief A vertex buffer in VRAM managed by the Naxa loader.
 */
typedef struct NaxaModel {
    uint32_t vao;
    uint32_t vbo;
    uint32_t ebo;
    int32_t submodel_count;
    NaxaSubmodel_t* submodels;
    int32_t bone_count;
    NaxaBone_t* bones;
    int32_t refs;
    char* path;
    struct NaxaModel* next;    
    // skeleton_t
} NaxaModel_t;

/**
 * @brief An object that exists in the game world.
 */
typedef struct {
    vec3 position;
    vec4 rotation_quat;
    NaxaModel_t* model;
} NaxaEntity_t;

#ifdef __cplusplus
}
#endif
#endif