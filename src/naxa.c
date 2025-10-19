#include "naxa/gfx.h"
#include <signal.h>
#include <stdlib.h>
#include <string.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <naxa/err.h>
#include <naxa/log.h>
#include <naxa/naxa.h>
#include <naxa/naxa_internal.h>

NaxaGlobals_t naxa_globals;

static void handle_segfault(int signum) {
    // If this is our second segfault, exit immediately
    if (naxa_globals.flags1 & GLOBAL_FLAGS1_SEGFAULTED) {
        exit(-1);
        return;
    }
    naxa_globals.flags1 |= GLOBAL_FLAGS1_SEGFAULTED;
    internal_logs(NAXA_SEVERITY_FATAL, "Segmentation fault");

    // Force the log thread to exit 
    await_log_thread();

    // Close log file and exit
    teardown_log_engine();
    exit(1);
}

extern int32_t naxa_init() {
    int32_t rc;

    // Clear out the global area
    memset(&naxa_globals, 0, sizeof(naxa_globals));

    // Segfault handler
    signal(SIGSEGV, handle_segfault);

    // Log to a file and to stdout
    // TODO let the application choose
    if ((rc = init_log_engine("latest.log", NAXA_TRUE)) != NAXA_E_SUCCESS) {
        return rc;
    }
    set_log_severity(NAXA_SEVERITY_INFO);
    internal_log("Started Naxa");

    // Set up the graphics context
    // TODO let the application choose
    if ((rc = init_gfx_context(720, 480, NULL)) != NAXA_E_SUCCESS) {
        return rc;
    }

    // Set up OpenGL stuff
    init_renderer();
    init_loader_caches();

    return NAXA_E_SUCCESS;
}

extern int32_t naxa_run() {
    internal_log("Entered game loop");

    NaxaEntity_t entity;
    naxa_load_model(&entity.model, "res/anaxa/model.pmx");
    entity.position[0] = 0.0f;
    entity.position[1] = -10.0f;
    entity.position[2] = -20.0f;

    while (!glfwWindowShouldClose(naxa_globals.window)) {
        render_enqueue(&entity);
        render_all();
        glfwPollEvents();
    }

    naxa_free_model(entity.model);

    return NAXA_E_SUCCESS;
}

extern int32_t naxa_stop() {
    // No need to get fancy, just use the window close signal
    glfwSetWindowShouldClose(naxa_globals.window, GLFW_TRUE);

    return NAXA_E_SUCCESS;
}

extern int32_t naxa_teardown() {
    internal_log("Tearing down Naxa");
    
    glfwTerminate();

    // The log engine should be torn down last because it will close the file
    teardown_log_engine();

    return NAXA_E_SUCCESS;
}
