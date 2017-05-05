/**
 * @file sem.h
 *
 * Header file for adaptation layer for semaphores
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#ifndef __LWM2MCORE_SEM_H__
#define __LWM2MCORE_SEM_H__

//--------------------------------------------------------------------------------------------------
/**
 * Function to initialize semaphores
 * @return:
 *   - Reference to the created semaphore
 */
//--------------------------------------------------------------------------------------------------
void* lwm2mcore_SemCreate
(
    const char* name,               ///< [IN] Name of the semaphore
    int32_t initialCount            ///< [IN] initial number of semaphore
);

//--------------------------------------------------------------------------------------------------
/**
 * Function to post a semaphore
 */
//--------------------------------------------------------------------------------------------------
void lwm2mcore_SemPost
(
    void* semaphorePtr              ///< [IN] Pointer to the semaphore.
);

//--------------------------------------------------------------------------------------------------
/**
 * Function to wait for a semaphore
 */
//--------------------------------------------------------------------------------------------------
void lwm2mcore_SemWait
(
    void* semaphorePtr              ///< [IN] Pointer to the semaphore.
);

//--------------------------------------------------------------------------------------------------
/**
 * Function to delete a semaphore
 */
//--------------------------------------------------------------------------------------------------
void lwm2mcore_SemDelete
(
    void* semaphorePtr              ///< [IN] Pointer to the semaphore.
);

#endif /* __LWM2MCORE_SEM_H__ */
