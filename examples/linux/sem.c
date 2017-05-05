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
#include <stdlib.h>
#include <semaphore.h>

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
    const char* name,               ///< [IN] Name of the semaphore
    int32_t initialCount            ///< [IN] initial number of semaphore
)
{
    // To be implemented later
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
    // To be implemented later
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
    // To be implemented later
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
    // To be implemented later
}


