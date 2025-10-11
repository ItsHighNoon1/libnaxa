#include <glad/glad.h>

#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/types.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <naxa/err.h>
#include <naxa/gfx.h>
#include <naxa/log.h>
#include <naxa/struct.h>
#include <naxa/naxa_internal.h>

typedef struct {
    vec3 position;
    vec2 texture;
    vec3 normal;
} VertexData_t;

int32_t naxa_load_model(NaxaModel_t** dest, char* path) {
    if (dest == NULL || path == NULL) {
        report_error(NAXA_E_NULLPTR);
        return NAXA_E_NULLPTR;
    }

    // We are going to need to extract the directory path first for texture
    // loads later on since they are specified as relative.
    int32_t directory_len = strlen(path);
    char* directory = malloc(directory_len + 1);
    memcpy(directory, path, directory_len + 1);
    for (int32_t i = directory_len; i >= 0; i--) {
        if (directory[i] == '/' || directory[i] == '\\') {
            if (directory[i] == '\\') {
                internal_logs(NAXA_SEVERITY_WARN, "Windows style paths are largely untested");
            }
            directory_len = i + 1;
            directory[directory_len] = '\0';
            directory = realloc(directory, directory_len + 1);
            break;
        }
    }

    // Read via Assimp
    // https://the-asset-importer-lib-documentation.readthedocs.io/en/latest/usage/use_the_lib.html
    // https://learnopengl.com/Model-Loading/Assimp
    *dest = NULL;
    const struct aiScene* scene = aiImportFile(path,
        aiProcess_CalcTangentSpace |
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices |
        aiProcess_SortByPType
    );
    if (scene == NULL) {
        report_error(NAXA_E_FILE);
        return NAXA_E_FILE;
    }

    // In case of multiple meshes, find the biggest one
    struct aiMesh* mainMesh = NULL;
    int32_t mainMeshFaces = 0;
    for (int32_t i = 0; i < scene->mNumMeshes; i++) {
        if (scene->mMeshes[i]->mNumVertices > mainMeshFaces) {
            mainMesh = scene->mMeshes[i];
            mainMeshFaces = mainMesh->mNumFaces;
        }
    }

    // Either no meshes or a mesh with 0 triangles
    if (scene->mNumMeshes <= 0) {
        free(directory);
        aiReleaseImport(scene);
        report_error(NAXA_E_FILE);
        return NAXA_E_FILE;
    }

    internal_logf(NAXA_SEVERITY_INFO, "Loading model %s (%d tris, skipped %d meshes)",
        path, mainMeshFaces, scene->mNumMeshes - 1);

    // Allocate OpenGL objects
    uint32_t vao = 0;
    uint32_t vbo = 0;
    uint32_t ebo = 0;
    uint32_t texture;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    glGenTextures(1, &texture);
    if (vao == 0) {
        free(directory);
        aiReleaseImport(scene);
        report_error(NAXA_E_INTERNAL);
        return NAXA_E_INTERNAL;
    }
    if (vbo == 0) {
        free(directory);
        aiReleaseImport(scene);
        report_error(NAXA_E_INTERNAL);
        return NAXA_E_INTERNAL;
    }
    if (ebo == 0) {
        free(directory);
        aiReleaseImport(scene);
        report_error(NAXA_E_INTERNAL);
        return NAXA_E_INTERNAL;
    }
    if (texture == 0) {
        free(directory);
        aiReleaseImport(scene);
        report_error(NAXA_E_INTERNAL);
        return NAXA_E_INTERNAL;
    }

    // Attempt to load the texture before we start doing OpenGL stuff
    struct aiMaterial* material = scene->mMaterials[mainMesh->mMaterialIndex];
    struct aiString texture_path;
    aiReturn ai_rc = aiGetMaterialTexture(material, aiTextureType_DIFFUSE, 0, &texture_path, NULL, NULL, NULL, NULL, NULL, NULL);
    if (ai_rc != aiReturn_SUCCESS) {
        free(directory);
        aiReleaseImport(scene);
        report_error(NAXA_E_INTERNAL);
        return NAXA_E_INTERNAL;
    }
    int32_t texture_width;
    int32_t texture_height;
    int32_t texture_channels;
    int32_t full_path_len = directory_len + texture_path.length;
    char* full_path = malloc(full_path_len + 1);
    memcpy(full_path, directory, directory_len);
    memcpy(full_path + directory_len, texture_path.data, texture_path.length);
    full_path[full_path_len] = 0;
    stbi_uc* texture_data = stbi_load(full_path, &texture_width, &texture_height, &texture_channels, 0);
    
    if (texture_data == NULL) {
        internal_logf(NAXA_SEVERITY_ERROR, "Failed to load texture %s for model %s", full_path, path);
        free(full_path);
        free(directory);
        aiReleaseImport(scene);
        report_error(NAXA_E_FILE);
        return NAXA_E_FILE;
    }
    free(full_path);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    switch (texture_channels) {
        case 3:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture_width, texture_height, 0, GL_RGB, GL_UNSIGNED_BYTE, texture_data);
            break;
        case 4:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture_width, texture_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture_data);
            break;
        default:
            free(directory);
            aiReleaseImport(scene);
            stbi_image_free(texture_data);
            report_error(NAXA_E_INTERNAL);
            return NAXA_E_INTERNAL;
    }
    stbi_image_free(texture_data);
    free(directory);

    // Load vertex data into VAO
    // TODO decide if malloc is fine
    int32_t vertex_buffer_size = mainMesh->mNumVertices * sizeof(VertexData_t);
    VertexData_t* vertices = malloc(vertex_buffer_size);
    for (int32_t i = 0; i < mainMesh->mNumVertices; i++) {
        vertices[i].position[0] = mainMesh->mVertices[i].x;
        vertices[i].position[1] = mainMesh->mVertices[i].y;
        vertices[i].position[2] = mainMesh->mVertices[i].z;
        vertices[i].texture[0] = mainMesh->mTextureCoords[0][i].x;
        vertices[i].texture[1] = mainMesh->mTextureCoords[0][i].y;
        vertices[i].normal[0] = mainMesh->mNormals[i].x;
        vertices[i].normal[1] = mainMesh->mNormals[i].y;
        vertices[i].normal[2] = mainMesh->mNormals[i].z;
    }
    int32_t element_buffer_size = mainMesh->mNumFaces * 3 * sizeof(VertexData_t);
    uint32_t* elements = malloc(element_buffer_size);
    for (int32_t i = 0; i < mainMesh->mNumFaces; i++) {
        elements[i * 3 + 0] = mainMesh->mFaces[i].mIndices[0];
        elements[i * 3 + 1] = mainMesh->mFaces[i].mIndices[1];
        elements[i * 3 + 2] = mainMesh->mFaces[i].mIndices[2];
    }
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertex_buffer_size, vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, element_buffer_size, elements, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData_t), (void*)offsetof(VertexData_t, position));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData_t), (void*)offsetof(VertexData_t, texture));
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData_t), (void*)offsetof(VertexData_t, normal));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);
    free(vertices);
    free(elements);
    aiReleaseImport(scene);

    // We set everything up in OpenGL, wrap the handles up in an object
    // TODO ok this should definitely be allocated somewhere real
    NaxaModel_t* model = malloc(sizeof(NaxaModel_t));
    model->vao = vao;
    model->vbo = vbo;
    model->ebo = ebo;
    model->vertex_count = mainMesh->mNumFaces * 3;
    model->diffuse = texture;
    *dest = model;
    return NAXA_E_SUCCESS;
}

int32_t naxa_free_model(NaxaModel_t* model) {
    if (model == NULL) {
        return NAXA_E_SUCCESS;
    }
    glDeleteVertexArrays(1, &model->vao);
    glDeleteBuffers(1, &model->vbo);
    glDeleteBuffers(1, &model->ebo);
    free(model);
    return NAXA_E_SUCCESS;
}