#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <naxa/err.h>
#include <naxa/gfx.h>
#include <naxa/log.h>
#include <naxa/naxa_internal.h>

int32_t render_all() {
    glClearColor(0.5f, 0.0f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glfwSwapBuffers(naxa_globals.window);

    return NAXA_E_SUCCESS;
}

int32_t render_enqueue(NaxaEntity_t* entity) {
    glBindVertexArray(entity->model->vao);
    for (int32_t i = 0; i < entity->model->submodel_count; i++) {
        NaxaSubmodel_t* submodel = &entity->model->submodels[i];
        glDrawElements(GL_TRIANGLES, submodel->vertex_count, GL_UNSIGNED_INT, (void*)(int64_t)submodel->offset);
    }
    return NAXA_E_SUCCESS;
}

int32_t bind_model(NaxaModel_t* model) {
    return NAXA_E_SUCCESS;
}