#include <signal.h>
#include <stdlib.h>
#include <string.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <naxa/naxa.h>
#include <naxa/naxa_internal.h>

Naxa_Globals_t naxa_globals;

void handle_segfault(int signum) {
    //TODO flush log queue
    if (naxa_globals.flags1 & GLOBAL_FLAGS1_SEGFAULTED) {
        exit(EXIT_FAILURE);
        return;
    }
    report_error(1);
    naxa_globals.flags1 |= GLOBAL_FLAGS1_SEGFAULTED;
    exit(EXIT_FAILURE);
}

extern void naxa_init() {
    memset(&naxa_globals, 0, sizeof(naxa_globals));
    signal(SIGSEGV, handle_segfault);
    glfwInit();
}

extern void naxa_run() {
    while (glfwWindowShouldClose(naxa_globals.window) != GLFW_TRUE) {
        glfwPollEvents();
        glfwSwapBuffers(naxa_globals.window);
    }
}

extern void naxa_stop() {
    glfwSetWindowShouldClose(naxa_globals.window, GLFW_TRUE);
}

extern void naxa_teardown() {
    glfwTerminate();
}
