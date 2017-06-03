/**
 * @file AvcConfig.c
 *
 * Adaptation layer for Configuration
 *
 * <hr>
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#include <lwm2mcore/lwm2mcore.h>

//--------------------------------------------------------------------------------------------------
/**
 * Function to read the polling timer.
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetPollingTimer
(
    uint32_t* pollingTimerPtr  ///< [OUT] Polling timer
)
{
    *pollingTimerPtr = 0;
    return LWM2MCORE_ERR_COMPLETED_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to set the polling timer.
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_SetPollingTimer
(
    uint32_t pollingTimer ///< [IN] Polling timer
)
{
    return LWM2MCORE_ERR_COMPLETED_OK;
}
