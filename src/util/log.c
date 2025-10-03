#include "naxa/log.h"
#include "naxa/err.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <naxa/naxa.h>
#include <naxa/naxa_internal.h>
#include <threads.h>

#define MAX_MESSAGE_LENGTH 1000

static int32_t real_log_func(int32_t severity, char* string, int32_t len, int32_t is_engine) {
    // If the severity is too low, ignore
    if (severity < naxa_globals.log_level) {
        return NAXA_E_SUCCESS;
    }

    // Build the message header
    char* owner_string = "NAXA";
    if (!is_engine) {
        // TODO user definable
        owner_string = "APP";
    }
    char* sev_string = "????";
    switch (severity) {
        case NAXA_SEVERITY_TRACE:
            sev_string = "TRACE";
            break;
        case NAXA_SEVERITY_INFO:
            sev_string = "INFO";
            break;
        case NAXA_SEVERITY_WARN:
            sev_string = "WARN";
            break;
        case NAXA_SEVERITY_ERROR:
            sev_string = "ERROR";
            break;
        case NAXA_SEVERITY_FATAL:
            sev_string = "FATAL";
            break;
    }
    char time_string[20];
    time_t timer = time(NULL);
    struct tm* time_info = localtime(&timer);
    strftime(time_string, sizeof(time_string), "%Y-%m-%d %H:%M:%S", time_info); // Consistently 19 chars
    char message_header[40];
    int32_t header_len = snprintf(message_header, sizeof(message_header), "[%.8s/%-5s %19s]", // Max 36 chars
            owner_string, sev_string, time_string);

    // Print the message
    if (naxa_globals.thread_flags & GLOBAL_THREADFLAGS_LOG) {
        // If the logging thread is active, put the message on a ring buffer
        // to be consumed by the logging thread later
        mtx_lock(&naxa_globals.log_mutex);
        int32_t max_len = sizeof(naxa_globals.log_ring) - naxa_globals.log_ring_cursor;
        int32_t actual_len = header_len + 1 + len + 1;
        if (actual_len > max_len) {
            naxa_globals.log_ring_end = naxa_globals.log_ring_cursor;
            naxa_globals.log_ring_cursor = 0;
            max_len = sizeof(naxa_globals.log_ring) - naxa_globals.log_ring_cursor;
        }
        naxa_globals.log_ring_cursor += snprintf(&naxa_globals.log_ring[naxa_globals.log_ring_cursor], max_len, "%s %s\n", message_header, string);
        mtx_unlock(&naxa_globals.log_mutex);
        cnd_broadcast(&naxa_globals.log_condition);
    } else {
        // If the logging thread is not active, do the print ourselves
        if (naxa_globals.flags1 & GLOBAL_FLAGS1_STDOUT_LOGGING) {
            printf("%s %s\n", message_header, string);
        }
        if (naxa_globals.log_file != NULL) {
            fwrite(message_header, header_len, 1, naxa_globals.log_file);
            fwrite(" ", 1, 1, naxa_globals.log_file);
            fwrite(string, len, 1, naxa_globals.log_file);
            fwrite("\n", 1, 1, naxa_globals.log_file);
        }
    }
    return NAXA_E_SUCCESS;
}

static int log_thread_func(void* user) {
    while (!naxa_globals.log_thread_stop) {
        // Wait on a signal that there is data. If 100ms passes we check anyways
        cnd_timedwait(&naxa_globals.log_condition, &naxa_globals.log_condition_mutex, &(struct timespec){ .tv_nsec = 100000000 });
        while (naxa_globals.log_ring_start != naxa_globals.log_ring_cursor) {
            // There is new data, how long is it?
            int32_t saved_cursor = naxa_globals.log_ring_cursor;
            int32_t log_len = saved_cursor - naxa_globals.log_ring_start;
            if (log_len < 0) {
                log_len = naxa_globals.log_ring_end - naxa_globals.log_ring_start;
            } else if (log_len == 0) {
                continue;
            }

            // Write it out
            if (naxa_globals.flags1 & GLOBAL_FLAGS1_STDOUT_LOGGING) {
                fwrite(&naxa_globals.log_ring[naxa_globals.log_ring_start], log_len, 1, stdout);
            }
            if (naxa_globals.log_file != NULL) {
                fwrite(&naxa_globals.log_ring[naxa_globals.log_ring_start], log_len, 1, naxa_globals.log_file);
            }

            if (saved_cursor < naxa_globals.log_ring_start) {
                // If the cursor was before the start, the ring buffer wrapped
                naxa_globals.log_ring_start = 0;
            } else {
                naxa_globals.log_ring_start = saved_cursor;
            }
        }
    }
    return 0;
}

int32_t init_log_engine(char* log_file, int32_t stdout_logging) {
    if (stdout_logging) {
        naxa_globals.flags1 |= GLOBAL_FLAGS1_STDOUT_LOGGING;
    } else {
        naxa_globals.flags1 &= ~GLOBAL_FLAGS1_STDOUT_LOGGING;
    }

    // If a log file was already open for some reason, close it first
    if (naxa_globals.log_file) {
        fclose(naxa_globals.log_file);
        naxa_globals.log_file = NULL;
    }

    naxa_globals.log_file = fopen(log_file, "w");
    if (naxa_globals.log_file == NULL) {
        report_error(NAXA_E_FILE);
        return NAXA_E_FILE;
    }

    // Init logging thread
    naxa_globals.log_thread_stop = 0;
    mtx_init(&naxa_globals.log_mutex, mtx_plain);
    mtx_init(&naxa_globals.log_condition_mutex, mtx_plain);
    cnd_init(&naxa_globals.log_condition);
    if (thrd_create(&naxa_globals.thread_log, log_thread_func, NULL) != thrd_success) {
        report_error(NAXA_E_INTERNAL);
        return NAXA_E_INTERNAL;
    }
    naxa_globals.thread_flags |= ~GLOBAL_FLAGS1_STDOUT_LOGGING;
    return NAXA_E_SUCCESS;
}

int32_t await_log_thread() {
    if (naxa_globals.thread_flags & GLOBAL_THREADFLAGS_LOG) {
        // Tell the thread to stop and wait for it
        naxa_globals.log_thread_stop = 1;
        thrd_join(naxa_globals.thread_log, NULL);

        // Make sure everyone knows the thread is gone
        memset(&naxa_globals.thread_log, 0, sizeof(thrd_t));
        naxa_globals.thread_flags &= ~GLOBAL_FLAGS1_STDOUT_LOGGING;
    }
    return NAXA_E_SUCCESS;
}

int32_t teardown_log_engine() {
    await_log_thread();
    if (naxa_globals.log_file) {
        fclose(naxa_globals.log_file);
        naxa_globals.log_file = NULL;
    }
    return NAXA_E_SUCCESS;
}

extern int32_t naxa_logs(int32_t severity, char* string) {
    return naxa_logn(severity, string, strlen(string));
}

extern int32_t naxa_logn(int32_t severity, char* string, int32_t n) {
    return real_log_func(severity, string, n, 0);
}

extern int32_t naxa_logf(int32_t severity, char* format, ...) {
    // Figure out how long of a buffer we need
    va_list argptr;
    va_start(argptr, format);
    int32_t desired_len = vsnprintf("", 0, format, argptr);
    va_end(argptr);
    if (desired_len < 0 || desired_len >= MAX_MESSAGE_LENGTH) {
        report_error(NAXA_E_TOOLONG);
        return NAXA_E_TOOLONG;
    }
    char* message_buffer = malloc(desired_len + 1);

    // vsnprintf
    va_start(argptr, format);
    vsnprintf(message_buffer, 0, format, argptr);
    va_end(argptr);

    // Delegate to logn
    int32_t rc = naxa_logn(severity, message_buffer, desired_len);
    free(message_buffer);
    return rc;
}

int32_t internal_logs(int32_t severity, char* string) {
    return internal_logn(severity, string, strlen(string));
}

int32_t internal_logn(int32_t severity, char* string, int32_t n) {
    return real_log_func(severity, string, n, 1);
}

int32_t internal_logf(int32_t severity, char* format, ...) {
    va_list argptr;
    va_start(argptr, format);
    int32_t desired_len = vsnprintf("", 0, format, argptr);
    va_end(argptr);
    if (desired_len < 0 || desired_len >= MAX_MESSAGE_LENGTH) {
        report_error(NAXA_E_TOOLONG);
        return NAXA_E_TOOLONG;
    }
    char* message_buffer = malloc(desired_len + 1);
    va_start(argptr, format);
    vsnprintf(message_buffer, desired_len + 1, format, argptr);
    va_end(argptr);
    int32_t rc = internal_logn(severity, message_buffer, desired_len);
    free(message_buffer);
    return rc;
}