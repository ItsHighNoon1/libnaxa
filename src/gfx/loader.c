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

    // Either no meshes or a mesh with 0 triangles
    if (scene->mNumMeshes <= 0) {
        free(directory);
        aiReleaseImport(scene);
        report_error(NAXA_E_FILE);
        return NAXA_E_FILE;
    }

    // Allocate OpenGL objects
    uint32_t vao = 0;
    uint32_t vbo = 0;
    uint32_t ebo = 0;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
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

    // Figure out the total number of vertices and faces
    int32_t total_vertices = 0;
    int32_t total_faces = 0;
    for (int32_t i = 0; i < scene->mNumMeshes; i++) {
        total_vertices += scene->mMeshes[i]->mNumVertices;
        total_faces += scene->mMeshes[i]->mNumFaces;
    }
    internal_logf(NAXA_SEVERITY_INFO, "Loading model %s (%d meshes, %d tris)",
        path, scene->mNumMeshes, total_faces);

    // Load all submodels
    NaxaSubmodel_t* submodels = malloc(sizeof(NaxaSubmodel_t) * scene->mNumMeshes);
    int32_t vertex_buffer_size = total_vertices * sizeof(VertexData_t);
    int32_t element_buffer_size = total_faces * 3 * sizeof(int32_t);
    int32_t vertex_offset = 0;
    int32_t element_offset = 0;
    VertexData_t* vertices = malloc(vertex_buffer_size);
    uint32_t* elements = malloc(element_buffer_size);
    for (int32_t mesh_idx = 0; mesh_idx < scene->mNumMeshes; mesh_idx++) {
        // Copy vertex and index data into the big buffers
        for (int32_t v_idx = 0; v_idx < scene->mMeshes[mesh_idx]->mNumVertices; v_idx++) {
            vertices[v_idx + vertex_offset].position[0] = scene->mMeshes[mesh_idx]->mVertices[v_idx].x;
            vertices[v_idx + vertex_offset].position[1] = scene->mMeshes[mesh_idx]->mVertices[v_idx].y;
            vertices[v_idx + vertex_offset].position[2] = scene->mMeshes[mesh_idx]->mVertices[v_idx].z;
            vertices[v_idx + vertex_offset].texture[0] = scene->mMeshes[mesh_idx]->mTextureCoords[0][v_idx].x;
            vertices[v_idx + vertex_offset].texture[1] = scene->mMeshes[mesh_idx]->mTextureCoords[0][v_idx].y;
            vertices[v_idx + vertex_offset].normal[0] = scene->mMeshes[mesh_idx]->mNormals[v_idx].x;
            vertices[v_idx + vertex_offset].normal[1] = scene->mMeshes[mesh_idx]->mNormals[v_idx].y;
            vertices[v_idx + vertex_offset].normal[2] = scene->mMeshes[mesh_idx]->mNormals[v_idx].z;
        }
        for (int32_t e_idx = 0; e_idx < scene->mMeshes[mesh_idx]->mNumFaces; e_idx++) {
            elements[e_idx * 3 + 0 + element_offset] = scene->mMeshes[mesh_idx]->mFaces[e_idx].mIndices[0] + vertex_offset;
            elements[e_idx * 3 + 1 + element_offset] = scene->mMeshes[mesh_idx]->mFaces[e_idx].mIndices[1] + vertex_offset;
            elements[e_idx * 3 + 2 + element_offset] = scene->mMeshes[mesh_idx]->mFaces[e_idx].mIndices[2] + vertex_offset;
        }
        submodels[mesh_idx].vertex_count = scene->mMeshes[mesh_idx]->mNumFaces * 3;
        submodels[mesh_idx].offset = element_offset * sizeof(uint32_t);
        vertex_offset += scene->mMeshes[mesh_idx]->mNumVertices;
        element_offset += scene->mMeshes[mesh_idx]->mNumFaces * 3;

        // Load the texture for this model
        struct aiMaterial* material = scene->mMaterials[scene->mMeshes[mesh_idx]->mMaterialIndex];
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
        internal_logf(NAXA_SEVERITY_INFO, "Loading texture %s for model %s", full_path, path);
        stbi_uc* texture_data = stbi_load(full_path, &texture_width, &texture_height, &texture_channels, 0);
        
        if (texture_data == NULL) {
            internal_logf(NAXA_SEVERITY_ERROR, "Failed to load texture %s for model %s", full_path, path);
            free(submodels);
            free(vertices);
            free(elements);
            free(full_path);
            free(directory);
            aiReleaseImport(scene);
            report_error(NAXA_E_FILE);
            return NAXA_E_FILE;
        }
        free(full_path);
        uint32_t texture = 0;
        glGenTextures(1, &texture);
        if (texture == 0) {
            free(submodels);
            free(vertices);
            free(elements);
            free(directory);
            aiReleaseImport(scene);
            stbi_image_free(texture_data);
            report_error(NAXA_E_INTERNAL);
            return NAXA_E_INTERNAL;
        }
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
                free(submodels);
                free(vertices);
                free(elements);
                free(directory);
                aiReleaseImport(scene);
                stbi_image_free(texture_data);
                report_error(NAXA_E_INTERNAL);
                return NAXA_E_INTERNAL;
        }
        stbi_image_free(texture_data);
        submodels[mesh_idx].diffuse = texture;
    }
    free(directory);

    // Load vertex data into VAO
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

    // We set everything up in OpenGL, wrap the handles up in an object
    // TODO ok this should definitely be allocated somewhere real
    NaxaModel_t* model = malloc(sizeof(NaxaModel_t));
    model->vao = vao;
    model->vbo = vbo;
    model->ebo = ebo;
    model->submodel_count = scene->mNumMeshes;
    model->submodels = submodels;
    *dest = model;
    aiReleaseImport(scene);
    return NAXA_E_SUCCESS;
}

int32_t naxa_free_model(NaxaModel_t* model) {
    if (model == NULL) {
        return NAXA_E_SUCCESS;
    }
    glDeleteVertexArrays(1, &model->vao);
    glDeleteBuffers(1, &model->vbo);
    glDeleteBuffers(1, &model->ebo);
    for (int32_t i = 0; i < model->submodel_count; i++) {
        glDeleteTextures(1, &model->submodels[i].diffuse);
    }
    free(model->submodels);
    free(model);
    return NAXA_E_SUCCESS;
}