/**
 * @file server.c
 *
 * Porting layer for server object parameters
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <platform/types.h>
#include <lwm2mcore/lwm2mcore.h>
#include <lwm2mcore/server.h>


//--------------------------------------------------------------------------------------------------
/**
 * Lifetime maximum value
 * 31536000 seconds = 1 year in seconds
 */
//--------------------------------------------------------------------------------------------------
#define LIFETIME_VALUE_MAX            31536000

//--------------------------------------------------------------------------------------------------
/**
 * Lifetime minimum value
 */
//--------------------------------------------------------------------------------------------------
#define LIFETIME_VALUE_MIN            1

//--------------------------------------------------------------------------------------------------
/**
 * Function to check if the lifetime is within aceptable limits
 *
 * @return
 *  - true if lifetime is within limits
 *  - false else
 */
//--------------------------------------------------------------------------------------------------
bool lwm2mcore_CheckLifetimeLimit
(
    uint32_t lifetime                  ///< [IN] Lifetime in seconds
)
{
    // Check only when enabling lifetime
    if (lifetime != LWM2MCORE_LIFETIME_VALUE_DISABLED)
    {
        if ( (LIFETIME_VALUE_MIN > lifetime)
          || (LIFETIME_VALUE_MAX < lifetime))
        {
            printf("Lifetime not within limit\n");
            return false;
        }
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the Polling Timer interval
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if succeeds
 *      - LWM2MCORE_ERR_INCORRECT_RANGE parameter out of range
 *      - LWM2MCORE_ERR_GENERAL_ERROR other failure
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_SetPollingTimer
(
    uint32_t interval   ///< [IN] Polling Timer interval in seconds
)
{
    if (false == lwm2mcore_CheckLifetimeLimit(interval))
    {
        return LWM2MCORE_ERR_INCORRECT_RANGE;
    }

    return LWM2MCORE_ERR_COMPLETED_OK;
}
