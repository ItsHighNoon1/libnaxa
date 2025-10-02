#include "naxa/err.h"
#include <signal.h>
#include <stdlib.h>
#include <string.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <naxa/naxa.h>
#include <naxa/naxa_internal.h>

Naxa_Globals_t naxa_globals;

void handle_segfault(int signum) {
    if (naxa_globals.flags1 & GLOBAL_FLAGS1_SEGFAULTED) {
        exit(-1);
        return;
    }
    report_error(1);
    naxa_globals.flags1 |= GLOBAL_FLAGS1_SEGFAULTED;
    await_log_thread();
    if (naxa_globals.log_file) {
        fclose(naxa_globals.log_file);
    }
    exit(1);
}

extern int32_t naxa_init() {
    memset(&naxa_globals, 0, sizeof(naxa_globals));
    signal(SIGSEGV, handle_segfault);
    init_log_engine("latest.log", NAXA_TRUE);
    internal_log("Started Naxa");
    glfwInit();

    return NAXA_E_SUCCESS;
}

extern int32_t naxa_run() {
    while (glfwWindowShouldClose(naxa_globals.window) != GLFW_TRUE) {
        glfwPollEvents();
        glfwSwapBuffers(naxa_globals.window);
    }

    return NAXA_E_SUCCESS;
}

extern int32_t naxa_stop() {
    glfwSetWindowShouldClose(naxa_globals.window, GLFW_TRUE);

    return NAXA_E_SUCCESS;
}

extern int32_t naxa_teardown() {
    internal_log("Tearing down Naxa");
    
    glfwTerminate();

    teardown_log_engine();

    return NAXA_E_SUCCESS;
}
