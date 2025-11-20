#ifndef __naxa_internal_h__
#define __naxa_internal_h__

#include <stdint.h>
#include <threads.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <naxa/naxa.h>

typedef struct {
    // Flags
    #define GLOBAL_FLAGS1_SEGFAULTED 0x1
    #define GLOBAL_FLAGS1_STDOUT_LOGGING 0x2
    int64_t flags1;

    // Threads
    #define GLOBAL_THREADFLAGS_LOG 0x1
    int64_t thread_flags;
    thrd_t thread_log;

    // GLFW context which is shared between graphics and input
    GLFWwindow* window;
    int window_width;
    int window_height;
} NaxaGlobals_t;

typedef struct {
    uint32_t type;
    char* path;
} NaxaShaderType_t;

extern NaxaGlobals_t naxa_globals;

// Generic functions
uint32_t hash_code(char* string);
char* read_file_into_buffer(FILE* fp, uint32_t* len);

// Graphics functions
int32_t init_gfx_context(int32_t window_width, int32_t window_height, char* window_name);
int32_t init_renderer();
int32_t init_loader_caches();
int32_t load_shader_program(uint32_t* dest, int32_t stages_len, NaxaShaderType_t* stages);
int32_t render_all();
int32_t render_enqueue(NaxaEntity_t* entity);

// Internal logging utilities
int32_t init_log_engine(char* log_file, int32_t stdout_logging);
int32_t await_log_thread();
int32_t teardown_log_engine();
#define internal_log(string) internal_logs(NAXA_SEVERITY_INFO, string)
int32_t internal_logs(int32_t severity, char* string);
int32_t internal_logn(int32_t severity, char* string, int32_t n);
int32_t internal_logf(int32_t severity, char* format, ...);
void set_log_severity(int32_t severity);
#define report_error(error) internal_logf(NAXA_SEVERITY_ERROR, "%s:%d (%s) - %s", \
    __FILE_NAME__, __LINE__, __func__, naxa_strerror(error))

#endif