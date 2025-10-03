#ifndef __naxa_internal_h__
#define __naxa_internal_h__

#include <stdint.h>
#include <stdio.h>
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

    // Graphics context
    GLFWwindow* window;

    // Log context
    int32_t log_level;
    int32_t log_ring_start;
    int32_t log_ring_cursor;
    int32_t log_ring_end;
    int32_t log_thread_stop;
    mtx_t log_mutex;
    mtx_t log_condition_mutex;
    cnd_t log_condition;
    FILE* log_file;
    char log_ring[2048];
} Naxa_Globals_t;

extern Naxa_Globals_t naxa_globals;

// Graphics functions
int32_t init_gfx_context(int32_t window_width, int32_t window_height, char* window_name);
int32_t render_all();

// Internal logging utilities
int32_t init_log_engine(char* log_file, int32_t stdout_logging);
int32_t await_log_thread();
int32_t teardown_log_engine();
#define internal_log(string) internal_logs(NAXA_SEVERITY_INFO, string)
int32_t internal_logs(int32_t severity, char* string);
int32_t internal_logn(int32_t severity, char* string, int32_t n);
int32_t internal_logf(int32_t severity, char* format, ...);
#define report_error(error) internal_logf(NAXA_SEVERITY_ERROR, "%s:%d (%s) - %s", \
    __FILE_NAME__, __LINE__, __func__, naxa_strerror(error))

#endif