#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <naxa/err.h>
#include <naxa/log.h>
#include <naxa/naxa_internal.h>

int32_t render_all() {
    glClearColor(0.5f, 0.0f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glfwSwapBuffers(naxa_globals.window);

    return NAXA_E_SUCCESS;
}