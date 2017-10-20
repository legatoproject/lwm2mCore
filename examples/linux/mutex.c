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

#include <stdio.h>

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
    (void)mutexNamePtr;
    printf("Mutex to be implemented later\n");
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
    (void)mutexPtr;
    printf("Mutex to be implemented later\n");
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
    (void)mutexPtr;
    printf("Mutex to be implemented later\n");
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
    (void)mutexPtr;
    printf("Mutex to be implemented later\n");
}


