/**
 * @file log.h
 * @author ItsHighNoon
 * @brief Naxa logging functionality.
 * @date 10-01-2025
 * 
 * @copyright Copyright (c) 2025
 */

#ifndef __naxa_log_h__
#define __naxa_log_h__
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define NAXA_SEVERITY_TRACE 0
#define NAXA_SEVERITY_INFO 1
#define NAXA_SEVERITY_WARN 2
#define NAXA_SEVERITY_ERROR 3
#define NAXA_SEVERITY_FATAL 4

/**
 * @brief Log message.
 * 
 * @param string The message to log.
 * @return int32_t NAXA_E_SUCCESS or an error code.
 *
 * Shorthand for naxa_logs(NAXA_SEVERITY_INFO, string).
 */
#define naxa_log(string) naxa_logs(NAXA_SEVERITY_INFO, string)

/**
 * @brief Log message.
 * 
 * @param severity The severity of the message. See NAXA_SEVERITY_*.
 * @param string The message to log.
 * @return int32_t NAXA_E_SUCCESS or an error code.
 *
 * Shorthand for naxa_logn(severity, string, strlen(string)).
 */
extern int32_t naxa_logs(int32_t severity, char* string);

/**
 * @brief Log a message with a given length and a specified severity.
 * 
 * @param severity The severity of the message. See NAXA_SEVERITY_*.
 * @param string The message to log.
 * @param n The length of the message to log.
 * @return int32_t NAXA_E_SUCCESS or an error code.
 *
 * naxa_logn will append the given message to the logging queue, which
 * is periodically serviced by the log thread. In the event of a
 * SIGSEGV (but not a SIGKILL) the rest of the log queue will be
 * flushed by the main thread before the program exits. If a second
 * SIGSEGV occurs while the log queue is being flushed the program
 * will exit immediately and the log queue may not be flushed.
 */
extern int32_t naxa_logn(int32_t severity, char* string, int32_t n);

/**
 * @brief Log formatted.
 * 
 * @param severity The severity of the message. See NAXA_SEVERITY_*.
 * @param format The format specifier for the message. See printf.
 * @param ... The parameters for the format specifier.
 * @return int32_t NAXA_E_SUCCESS or an error code.
 *
 * naxa_logf is a printf-like interface for naxa_logn. It is equivalent
 * to calling sprintf and then naxa_logn.
 */
extern int32_t naxa_logf(int32_t severity, char* format, ...);

#ifdef __cplusplus
}
#endif
#endif