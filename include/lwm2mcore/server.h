/**
 * @file server.h
 *
 * Header file for LWM2M Core server object parameters
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#ifndef  __LWM2MCORE_SERVER_H__
#define  __LWM2MCORE_SERVER_H__

#include <lwm2mcore/lwm2mcore.h>

/**
  * @addtogroup lwm2mcore_server_IFS
  * @{
  */

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to check if the lifetime is within acceptable limits
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @c true if lifetime is within limits
 *  - @c false else
 */
//--------------------------------------------------------------------------------------------------
bool lwm2mcore_CheckLifetimeLimit
(
    uint32_t lifetime                  ///< [IN] Lifetime in seconds
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Set the Polling Timer interval
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if succeeds
 *  - @ref LWM2MCORE_ERR_INCORRECT_RANGE parameter out of range
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR other failure
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_SetPollingTimer
(
    uint32_t interval               ///< [IN] Polling Timer interval in seconds
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Set the EDM Polling Timer interval
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if succeeds
 *  - @ref LWM2MCORE_ERR_INCORRECT_RANGE parameter out of range
 *  - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED if not supported
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR other failure
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_SetEdmPollingTimer
(
    uint32_t interval               ///< [IN] Polling Timer interval in seconds
);

/**
  * @}
  */

#endif /*  __LWM2MCORE_SERVER_H__ */
