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
    #define GLOBAL_FLAGS1_IMMEDIATE_LOGGING 0x2
    int64_t flags1;

    // Threads
    thrd_t log_thread;

    // Graphics context
    GLFWwindow* window;

    // Log context
    #define N_LOG_AREAS 2
    int32_t log_thread_log_area;
    int32_t current_log_area;
    struct {
        int32_t len;
        char content[2048];
    } log_area[N_LOG_AREAS];
} Naxa_Globals_t;

extern Naxa_Globals_t naxa_globals;

// Internal logging utilities
int32_t init_log_engine();
int32_t await_log_thread();
#define internal_log(string) internal_logs(NAXA_SEVERITY_INFO, string)
void internal_logs(int32_t severity, char* string);
void internal_logn(int32_t severity, char* string, int32_t n);
void internal_logf(int32_t severity, char* format, ...);
#define report_error(error) internal_logf(NAXA_SEVERITY_ERROR, "%s:%d (%s) - %s", \
    __FILE_NAME__, __LINE__, __func__, naxa_strerror(error))

#endif