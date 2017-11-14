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

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to check if the lifetime is within acceptable limits
 *
 * @return
 *      - true if lifetime is within limits
 *      - false else
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
