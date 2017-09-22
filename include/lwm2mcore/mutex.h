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
  @defgroup lwm2mcore_platform_adaptor_mutex_IFS Mutex
  @ingroup lwm2mcore_platform_adaptor_IFS
  @brief Adaptation layer for Mutex
  */

/**
  * @addtogroup lwm2mcore_platform_adaptor_mutex_IFS
  * @{
  */

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

/**
  * @}
  */

#endif /* __LWM2MCORE_THREAD_H__ */

