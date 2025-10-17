/**
 * @file struct.h
 * @author ItsHighNoon
 * @brief Naxa structures.
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

typedef struct NaxaTexture {
    uint32_t texture;
    int32_t refs;
    char* path;
    struct NaxaTexture* next;
} NaxaTexture_t;

typedef struct {
    int32_t vertex_count;
    int32_t offset;
    NaxaTexture_t* diffuse;
} NaxaSubmodel_t;

typedef struct NaxaModel {
    uint32_t vao;
    uint32_t vbo;
    uint32_t ebo;
    int32_t submodel_count;
    NaxaSubmodel_t* submodels;
    int32_t refs;
    char* path;
    struct NaxaModel* next;    
    // skeleton_t
} NaxaModel_t;

typedef struct {
    vec3 position;
    vec4 rotation_quat;
    NaxaModel_t* model;
} NaxaEntity_t;

#ifdef __cplusplus
}
#endif
#endif