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

#define NAXA_FALSE 0
#define NAXA_TRUE 1

/**
 * @brief Initialize Naxa.
 *
 * @return int32_t NAXA_E_SUCCESS or an error code.
 * 
 * This function initializes Naxa. This should be the first call to Naxa
 * and calling other Naxa APIs before naxa_init is unsupported behavior.
 */
extern int32_t naxa_init();

/**
 * @brief Pass control to Naxa.
 * 
 * @return int32_t NAXA_E_SUCCESS or an error code.
 *
 * Invoking this function will start the Naxa game loop. When this function
 * returns, it is because we received a signal from the operating system
 * to close or naxa_stop was invoked.
 */
extern int32_t naxa_run();

/**
 * @brief Stop running Naxa.
 *
 * @return int32_t NAXA_E_SUCCESS or an error code.
 * 
 * This function will stop Naxa as if the application window was closed.
 * This will not interrupt the current frame. 
 */
extern int32_t naxa_stop();

/**
 * @brief Tear down Naxa.
 *
 * @return int32_t NAXA_E_SUCCESS or an error code.
 * 
 * Clean up Naxa systems and check for leakage. Calling this function on
 * application exit is not strictly necessary as the operating system
 * should clean up the memory space and graphics context for us. However,
 * naxa_teardown may produce helpful warnings if it detects bad states
 * during the cleanup proces.
 */
extern int32_t naxa_teardown();

#ifdef __cplusplus
}
#endif
#endif