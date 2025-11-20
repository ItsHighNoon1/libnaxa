#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <naxa/err.h>
#include <naxa/log.h>
#include <naxa/naxa_internal.h>

static void error_callback(int error, const char* desc) {
    internal_logf(NAXA_SEVERITY_ERROR, "GLFW error %d: %s", error, desc);
}

static void gl_error_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* user_parm) {
    switch (severity) {
        case GL_DEBUG_SEVERITY_NOTIFICATION:
            internal_logn(NAXA_SEVERITY_TRACE, (char*)message, length);
            break;
        case GL_DEBUG_SEVERITY_LOW:
            internal_logn(NAXA_SEVERITY_INFO, (char*)message, length);
            break;
        case GL_DEBUG_SEVERITY_MEDIUM:
            internal_logn(NAXA_SEVERITY_WARN, (char*)message, length);
            break;
        case GL_DEBUG_SEVERITY_HIGH:
            internal_logn(NAXA_SEVERITY_ERROR, (char*)message, length);
            break;
    }
}

static void window_resize_callback(GLFWwindow* window, int width, int height) {
    if (naxa_globals.window == window) {
        glViewport(0, 0, width, height);
    }
    naxa_globals.window_width = width;
    naxa_globals.window_height = height;
}

int32_t init_gfx_context(int32_t window_width, int32_t window_height, char* window_name) {
    if (window_width < 0 || window_height < 0) {
        report_error(NAXA_E_BOUNDS);
        return NAXA_E_BOUNDS;
    }
    if (window_name == NULL) {
        window_name = "Naxa";
    }

    // Standard GLFW init
    if (glfwInit() == GLFW_FALSE) {
        report_error(NAXA_E_INTERNAL);
        return NAXA_E_INTERNAL;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    naxa_globals.window = glfwCreateWindow(window_width, window_height, window_name, NULL, NULL);
    if (naxa_globals.window == NULL) {
        report_error(NAXA_E_INTERNAL);
        return NAXA_E_INTERNAL;
    }

    // Standard OpenGL init
    glfwMakeContextCurrent(naxa_globals.window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        report_error(NAXA_E_INTERNAL);
        return NAXA_E_INTERNAL;
    }
    naxa_globals.window_width = window_width;
    naxa_globals.window_height = window_height;
    glViewport(0, 0, window_width, window_height);

    // Set callbacks
    glfwSetErrorCallback(error_callback);
    glfwSetFramebufferSizeCallback(naxa_globals.window, window_resize_callback);
    glDebugMessageCallback(gl_error_callback, NULL);
    
    return NAXA_E_SUCCESS;
}