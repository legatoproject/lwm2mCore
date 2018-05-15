/**
 * @file mutex.h
 *
 * Header file for adaptation layer for mutex related api
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#ifndef __LWM2MCORE_THREAD_H__
#define __LWM2MCORE_THREAD_H__

/**
  * @addtogroup lwm2mcore_platform_adaptor_mutex_IFS
  * @{
  */

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to create a mutex
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *   - @c 0 on success; error code otherwise
 */
//--------------------------------------------------------------------------------------------------
void* lwm2mcore_MutexCreate
(
    const char* mutexNamePtr          ///< mutex name
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to lock a mutex
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 */
//--------------------------------------------------------------------------------------------------
void lwm2mcore_MutexLock
(
    void* mutexPtr              ///< [IN] mutex
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to unlock a mutex
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 */
//--------------------------------------------------------------------------------------------------
void lwm2mcore_MutexUnlock
(
    void* mutexPtr              ///< [IN] mutex

);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to delete a mutex
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 */
//--------------------------------------------------------------------------------------------------
void lwm2mcore_MutexDelete
(
    void* mutexPtr             ///< [IN] mutex
);

/**
  * @}
  */

#endif /* __LWM2MCORE_THREAD_H__ */

