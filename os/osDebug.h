/**
 * @file osDebug.h
 *
 * Header file for adaptation layer for log management
 *
 * <hr>
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#ifndef OS_DEBUG_H
#define OS_DEBUG_H

//--------------------------------------------------------------------------------------------------
/**
 * Function for assert
 */
//--------------------------------------------------------------------------------------------------
void os_assert
(
    bool condition,         /// [IN] Condition to be checked
    char* function,         /// [IN] Function name which calls the assert function
    uint32_t line           /// [IN] Function line which calls the assert function
);

//--------------------------------------------------------------------------------------------------
/**
 * Macro for assertion
 */
//--------------------------------------------------------------------------------------------------
#define OS_ASSERT(X) os_assert(X, __func__, __LINE__)

//--------------------------------------------------------------------------------------------------
/**
 * Adaptation function for log: dump data
 */
//--------------------------------------------------------------------------------------------------
void os_debug_data_dump
(
    char *descPtr,              ///< [IN] data description
    void *addrPtr,              ///< [IN] Data address to be dumped
    int len                     ///< [IN] Data length
);

#endif //OS_DEBUG_H

