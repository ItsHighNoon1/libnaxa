/**
 * @file naxa.h
 * @author ItsHighNoon
 * @brief Core Naxa APIs.
 * @date 09-19-2025
 * 
 * @copyright Copyright (c) 2025
 */

#ifndef __naxa_h__
#define __naxa_h__
#ifdef __cplusplus
extern "C" {
#endif

#include <naxa/err.h>
#include <naxa/log.h>

/**
 * @brief Initialize Naxa.
 * 
 * This function initializes Naxa. This should be the first call to Naxa
 * and calling other Naxa APIs before naxa_init is unsupported behavior.
 */
extern void naxa_init();

/**
 * @brief Pass control to Naxa.
 * 
 * Invoking this function will start the Naxa game loop. When this function
 * returns, it is because we received a signal from the operating system
 * to close or naxa_stop was invoked.
 */
extern void naxa_run();

/**
 * @brief Stop running Naxa.
 * 
 * This function will stop Naxa as if the application window was closed.
 * This will not interrupt the current frame. 
 */
extern void naxa_stop();

/**
 * @brief Tear down Naxa.
 * 
 * Clean up Naxa systems and check for leakage. Calling this function on
 * application exit is not strictly necessary as the operating system
 * should clean up the memory space and graphics context for us. However,
 * naxa_teardown may produce helpful warnings if it detects bad states
 * during the cleanup proces.
 */
extern void naxa_teardown();

#ifdef __cplusplus
}
#endif
#endif