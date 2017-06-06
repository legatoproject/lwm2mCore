/**
 * @file mutex.c
 *
 * Adaptation layer for mutex
 *
 * <hr>
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#include "pthread.h"

//--------------------------------------------------------------------------------------------------
/**
 * Function to create a mutex
 *
 * @return:
 *   - 0 on success; error code otherwise
 */
//--------------------------------------------------------------------------------------------------
void* lwm2mcore_MutexCreate
(
    const char* mutexNamePtr         ///< mutex name
)
{
    // To be implemented later
    return NULL;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to lock a mutex
 */
//--------------------------------------------------------------------------------------------------
void lwm2mcore_MutexLock
(
    void* mutexPtr              ///< [IN] mutex
)
{
    // To be implemented later
    return;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to unlock a mutex
 *
 */
//--------------------------------------------------------------------------------------------------
void lwm2mcore_MutexUnlock
(
    void* mutexPtr              ///< [IN] mutex
)
{
    // To be implemented later
    return;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to delete a mutex
 */
//--------------------------------------------------------------------------------------------------
void lwm2mcore_MutexDelete
(
    void* mutexPtr              ///< [IN] mutex
)
{
    // To be implemented later
    return;
}


