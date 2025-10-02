#include "naxa/err.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <naxa/naxa.h>
#include <naxa/naxa_internal.h>

#define MAX_MESSAGE_LENGTH 1000

static void real_log_func(int32_t severity, char* string, int32_t len, int32_t is_engine) {
    printf("%d %d - %.*s\n", is_engine, severity, len, string);
}

static int log_thread_func(void* user) {
    return 0;
}

int32_t init_log_engine() {
    if (thrd_create(&naxa_globals.log_thread, log_thread_func, NULL) != thrd_success) {
        report_error(NAXA_E_INTERNAL);
        return NAXA_E_INTERNAL;
    }
    return NAXA_E_SUCCESS;
}

int32_t await_log_thread() {
    return NAXA_E_SUCCESS;
}

extern void naxa_logs(int32_t severity, char* string) {
    naxa_logn(severity, string, strlen(string));
}

extern void naxa_logn(int32_t severity, char* string, int32_t n) {
    real_log_func(severity, string, n, 0);
}

extern void naxa_logf(int32_t severity, char* format, ...) {
    va_list argptr;
    va_start(argptr, format);
    int32_t desired_len = vsnprintf("", 0, format, argptr);
    va_end(argptr);
    if (desired_len < 0 || desired_len >= MAX_MESSAGE_LENGTH) {
        report_error(NAXA_E_TOOLONG);
        return;
    }
    char* message_buffer = malloc(desired_len + 1);
    va_start(argptr, format);
    vsnprintf(message_buffer, 0, format, argptr);
    va_end(argptr);
    naxa_logn(severity, message_buffer, desired_len);
    free(message_buffer);
}

void internal_logs(int32_t severity, char* string) {
    internal_logn(severity, string, strlen(string));
}

void internal_logn(int32_t severity, char* string, int32_t n) {
    real_log_func(severity, string, n, 1);
}

void internal_logf(int32_t severity, char* format, ...) {
    va_list argptr;
    va_start(argptr, format);
    int32_t desired_len = vsnprintf("", 0, format, argptr);
    va_end(argptr);
    if (desired_len < 0 || desired_len >= MAX_MESSAGE_LENGTH) {
        report_error(NAXA_E_TOOLONG);
        return;
    }
    char* message_buffer = malloc(desired_len + 1);
    va_start(argptr, format);
    vsnprintf(message_buffer, desired_len + 1, format, argptr);
    va_end(argptr);
    internal_logn(severity, message_buffer, desired_len);
    free(message_buffer);
}