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
    glDrawElements(GL_TRIANGLES, entity->model->vertex_count, GL_UNSIGNED_INT, (void*)0);
    return NAXA_E_SUCCESS;
}

int32_t bind_model(NaxaModel_t* model) {
    return NAXA_E_SUCCESS;
}