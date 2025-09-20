/**
 * @file naxa.h
 * @author ItsHighNoon
 * @brief Core naxa APIs.
 * @date 09-19-2025
 * 
 * @copyright Copyright (c) 2025
 */

#ifndef __naxa_h__
#define __naxa_h__

/**
 * @brief Initialize naxa.
 * 
 * TODO add docs when I am actually doing anything
 */
extern void naxa_init();

/**
 * @brief Tear down naxa.
 * 
 * Clean up naxa systems and check for leakage. Calling this function on
 * application exit is not strictly necessary as the operating system
 * should clean up the memory space and graphics context for us. However,
 * naxa_teardown may produce helpful warnings if it detects bad states
 * during the cleanup proces.
 */
extern void naxa_teardown();

#endif