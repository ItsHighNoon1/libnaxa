#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <naxa/err.h>
#include <naxa/gfx.h>
#include <naxa/log.h>
#include <naxa/naxa_internal.h>

typedef struct {
    NaxaModel_t* model;
    vec3 position;
    vec4 rotation_quat;
} Renderable_t;

int32_t render_queue_len;
int32_t render_queue_size;
Renderable_t* render_queue;

static int32_t compare_renderables(const void* a, const void* b) {
    Renderable_t* left = (Renderable_t*)a;
    Renderable_t* right = (Renderable_t*)b;
    if (left->model == right->model) {
        return 0;
    }
    return right->model->vao - left->model->vao;
}

int32_t init_renderer() {
    // TODO malloc
    render_queue_len = 0;
    render_queue_size = 10;
    render_queue = malloc(render_queue_size * sizeof(Renderable_t));

    glClearColor(0.5f, 0.0f, 0.5f, 1.0f);
    return NAXA_E_SUCCESS;
}

int32_t render_all() {
    glClear(GL_COLOR_BUFFER_BIT);

    qsort(render_queue, render_queue_len, sizeof(Renderable_t), compare_renderables);

    uint32_t last_vao = 0;
    uint32_t last_texture = 0;
    for (int32_t i = 0; i < render_queue_len; i++) {
        if (render_queue[i].model->vao != last_vao) {
            last_vao = render_queue[i].model->vao;
            glBindVertexArray(last_vao);
        }
        for (int32_t j = 0; j < render_queue[i].model->submodel_count; j++) {
            NaxaSubmodel_t* submodel = &render_queue[i].model->submodels[j];
            if (submodel->diffuse->texture != last_texture) {
                last_texture = submodel->diffuse->texture;
                glBindTexture(GL_TEXTURE0, last_texture);
            }
            glDrawElements(GL_TRIANGLES, submodel->vertex_count, GL_UNSIGNED_INT, (void*)(int64_t)submodel->offset);
        }
    }

    glfwSwapBuffers(naxa_globals.window);

    render_queue_len = 0;

    return NAXA_E_SUCCESS;
}

int32_t render_enqueue(NaxaEntity_t* entity) {
    if (entity == NULL || entity->model == NULL) {
        report_error(NAXA_E_NULLPTR);
        return NAXA_E_NULLPTR;
    }
    if (render_queue_len >= render_queue_size) {
        render_queue_size *= 2;
        render_queue = realloc(render_queue, render_queue_size * sizeof(Renderable_t));
    }
    render_queue[render_queue_len].model = entity->model;
    glm_vec3_copy(entity->position, render_queue[render_queue_len].position);
    glm_vec4_copy(entity->rotation_quat, render_queue[render_queue_len].rotation_quat);
    render_queue_len++;
    
    return NAXA_E_SUCCESS;
}

int32_t bind_model(NaxaModel_t* model) {
    return NAXA_E_SUCCESS;
}