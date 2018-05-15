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

/**
  * @}
  */

#endif /*  __LWM2MCORE_SERVER_H__ */
