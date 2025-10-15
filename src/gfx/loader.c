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

#define MODEL_CACHE_SIZE 512
#define TEXTURE_CACHE_SIZE 512
#define MODEL_CACHE_HASH_SIZE 16
#define TEXTURE_CACHE_HASH_SIZE 16

typedef struct {
    vec3 position;
    vec2 texture;
    vec3 normal;
} VertexData_t;

NaxaModel_t* model_cache_next;
NaxaModel_t model_cache[MODEL_CACHE_SIZE];
NaxaModel_t* model_cache_hash_map[MODEL_CACHE_HASH_SIZE];
NaxaTexture_t* texture_cache_next;
NaxaTexture_t texture_cache[TEXTURE_CACHE_SIZE];
NaxaTexture_t* texture_cache_hash_map[TEXTURE_CACHE_HASH_SIZE];

int32_t init_loader_caches() {
    memset(model_cache, 0, sizeof(model_cache));
    memset(model_cache_hash_map, 0, sizeof(model_cache_hash_map));
    for (int32_t i = 0; i < MODEL_CACHE_SIZE - 1; i++) {
        model_cache[i].next = &model_cache[i + 1];
    }
    model_cache[MODEL_CACHE_SIZE - 1].next = NULL;
    model_cache_next = &model_cache[0];
    memset(texture_cache, 0, sizeof(texture_cache));
    memset(texture_cache_hash_map, 0, sizeof(texture_cache_hash_map));
    for (int32_t i = 0; i < TEXTURE_CACHE_SIZE - 1; i++) {
        texture_cache[i].next = &texture_cache[i + 1];
    }
    texture_cache[TEXTURE_CACHE_SIZE - 1].next = NULL;
    texture_cache_next = &texture_cache[0];
    return NAXA_E_SUCCESS;
}

int32_t naxa_load_texture(NaxaTexture_t** dest, char* path) {
    if (dest == NULL || path == NULL) {
        report_error(NAXA_E_NULLPTR);
        return NAXA_E_NULLPTR;
    }
    *dest = NULL;

    // Check if this is already in the hash table
    int32_t hash_bucket = hash_code(path) % TEXTURE_CACHE_HASH_SIZE;
    NaxaTexture_t* current = texture_cache_hash_map[hash_bucket];
    while (current) {
        if (strcmp(path, current->path) == 0) {
            // We found our texture
            internal_logf(NAXA_SEVERITY_TRACE, "Found texture %s in texture cache", path);
            current->refs++;
            *dest = current;
            return NAXA_E_SUCCESS;
        }
        current = current->next;
    }

    // Do the load
    int32_t width;
    int32_t height;
    int32_t channels;
    stbi_uc* texture_data = stbi_load(path, &width, &height, &channels, 0);
    if (texture_data == NULL) {
             report_error(NAXA_E_FILE);
        return NAXA_E_FILE;
    }
    uint32_t texture_id = 0;
    glGenTextures(1, &texture_id);
    if (texture_id == 0) {
        report_error(NAXA_E_INTERNAL);
        return NAXA_E_INTERNAL;
    }
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    switch (channels) {
        case 3:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, texture_data);
            break;
        case 4:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture_data);
            break;
        default:
            stbi_image_free(texture_data);
            report_error(NAXA_E_INTERNAL);
            return NAXA_E_INTERNAL;
    }
    stbi_image_free(texture_data);

    // Set up a texture cache slot
    if (texture_cache_next == NULL) {
        report_error(NAXA_E_EXHAUSTED);
        return NAXA_E_EXHAUSTED;
    }
    int32_t path_len = strlen(path);
    char* path_copy = malloc(path_len + 1);
    memcpy(path_copy, path, path_len + 1);
    NaxaTexture_t* texture = texture_cache_next;
    texture_cache_next = texture->next;
    texture->next = texture_cache_hash_map[hash_bucket];
    texture_cache_hash_map[hash_bucket] = texture;
    texture->path = path_copy;
    texture->refs = 1;
    texture->texture = texture_id;

    internal_logf(NAXA_SEVERITY_INFO, "Newly loaded texture %s", path);
    *dest = texture;
    return NAXA_E_SUCCESS;
}

int32_t naxa_free_texture(NaxaTexture_t* texture) {
    if (texture == NULL) {
        report_error(NAXA_E_NULLPTR);
        return NAXA_E_NULLPTR;
    }
    if (texture < texture_cache || texture >= texture_cache + TEXTURE_CACHE_SIZE) {
        report_error(NAXA_E_BOUNDS);
        return NAXA_E_BOUNDS;
    }
    texture->refs--;
    if (texture->refs == 0) {
        // Remove from path hash table
        int32_t hash_bucket = hash_code(texture->path) % TEXTURE_CACHE_HASH_SIZE;
        if (texture_cache_hash_map[hash_bucket] == texture) {
            texture_cache_hash_map[hash_bucket] = texture->next;
        } else {
            NaxaTexture_t* current = texture_cache_hash_map[hash_bucket];
            while (current) {
                if (current->next == texture) {
                    current->next = texture->next;
                    break;
                }
                current = current->next;
            }
            if (current == NULL) {
                // Texture wasn't in the hash map where it should have been
                report_error(NAXA_E_INTERNAL);
                return NAXA_E_INTERNAL;
            }
        }

        // Add to the available list
        glDeleteTextures(1, &texture->texture);
        internal_logf(NAXA_SEVERITY_INFO, "Unloaded texture %s", texture->path);
        memset(texture, 0, sizeof(NaxaTexture_t));
        texture->next = texture_cache_next;
        texture_cache_next = texture;
    } else if (texture->refs < 0) {
        report_error(NAXA_E_INTERNAL);
        return NAXA_E_INTERNAL;
    }
    return NAXA_E_SUCCESS;
}

int32_t naxa_load_model(NaxaModel_t** dest, char* path) {
    if (dest == NULL || path == NULL) {
        report_error(NAXA_E_NULLPTR);
        return NAXA_E_NULLPTR;
    }

    // We are going to need to extract the directory path first for texture
    // loads later on since they are specified as relative
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
        
        int32_t full_path_len = directory_len + texture_path.length;
        char* full_path = malloc(full_path_len + 1);
        memcpy(full_path, directory, directory_len);
        memcpy(full_path + directory_len, texture_path.data, texture_path.length);
        full_path[full_path_len] = 0;
        naxa_load_texture(&submodels[mesh_idx].diffuse, full_path);
        free(full_path);
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
        naxa_free_texture(model->submodels[i].diffuse);
    }
    free(model->submodels);
    free(model);
    return NAXA_E_SUCCESS;
}