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

/**
  * @addtogroup lwm2mcore_platform_adaptor_semaphore_IFS
  * @{
  */


//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to initialize semaphores
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
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
 * @brief Function to post a semaphore
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 */
//--------------------------------------------------------------------------------------------------
void lwm2mcore_SemPost
(
    void* semaphorePtr              ///< [IN] Pointer to the semaphore.
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to wait for a semaphore
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 */
//--------------------------------------------------------------------------------------------------
void lwm2mcore_SemWait
(
    void* semaphorePtr              ///< [IN] Pointer to the semaphore.
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to delete a semaphore
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 */
//--------------------------------------------------------------------------------------------------
void lwm2mcore_SemDelete
(
    void* semaphorePtr              ///< [IN] Pointer to the semaphore.
);

/**
  * @}
  */

#endif /* __LWM2MCORE_SEM_H__ */
