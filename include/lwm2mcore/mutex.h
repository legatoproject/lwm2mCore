/**
 * @file thread.h
 *
 * Header file for adaptation layer for thread related api
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#ifndef __LWM2MCORE_THREAD_H__
#define __LWM2MCORE_THREAD_H__


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
    const char* mutexNamePtr          ///< mutex name
);

//--------------------------------------------------------------------------------------------------
/**
 * Function to lock a mutex
 *
 * @return:
 *   - 0 on success; error code otherwise
 */
//--------------------------------------------------------------------------------------------------
int lwm2mcore_MutexLock
(
    void* mutexPtr              ///< [IN] mutex
);

//--------------------------------------------------------------------------------------------------
/**
 * Function to unlock a mutex
 *
 * @return:
 *   - 0 on success; error code otherwise
 */
//--------------------------------------------------------------------------------------------------
int lwm2mcore_MutexUnlock
(
    void* mutexPtr              ///< [IN] mutex

);

//--------------------------------------------------------------------------------------------------
/**
 * Function to delete a mutex
 */
//--------------------------------------------------------------------------------------------------
void lwm2mcore_MutexDelete
(
    void* mutexPtr             ///< [IN] mutex
);

#endif /* __LWM2MCORE_THREAD_H__ */

