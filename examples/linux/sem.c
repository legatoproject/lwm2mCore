/**
 * @file sem.c
 *
 * Adaptation layer for semaphores
 *
 * <hr>
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#include <stdio.h>
#include <stdint.h>

//--------------------------------------------------------------------------------------------------
/**
 * Function to initialize semaphores
 *
 * @return:
 *    - Reference to the created semaphore
 */
//--------------------------------------------------------------------------------------------------
void* lwm2mcore_SemCreate
(
    const char* namePtr,            ///< [IN] Name of the semaphore
    int32_t initialCount            ///< [IN] initial number of semaphore
)
{
    (void)namePtr;
    (void)initialCount;
    printf("Semaphore to be implemented later\n");
    return NULL;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to post a semaphore
 */
//--------------------------------------------------------------------------------------------------
void lwm2mcore_SemPost
(
    void* semaphorePtr              ///< [IN] Pointer to the semaphore.
)
{
    (void)semaphorePtr;
    printf("Semaphore to be implemented later\n");
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to wait for a semaphore
 */
//--------------------------------------------------------------------------------------------------
void lwm2mcore_SemWait
(
    void* semaphorePtr              ///< [IN] Pointer to the semaphore.
)
{
    (void)semaphorePtr;
    printf("Semaphore to be implemented later\n");
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to delete a semaphore
 */
//--------------------------------------------------------------------------------------------------
void lwm2mcore_SemDelete
(
    void* semaphorePtr              ///< [IN] Pointer to the semaphore.
)
{
    (void)semaphorePtr;
    printf("Semaphore to be implemented later\n");
}
