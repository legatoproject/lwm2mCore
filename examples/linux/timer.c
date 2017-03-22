/**
 * @file timer.c
 *
 * Adaptation layer for timer management
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */


#include <stdbool.h>
#include <stdint.h>
#include <lwm2mcore/timer.h>

//--------------------------------------------------------------------------------------------------
/**
 * Adaptation function for timer launch
 *
 * @return
 *      - true  on success
 *      - false on failure
 */
//--------------------------------------------------------------------------------------------------
bool lwm2mcore_TimerSet
(
    lwm2mcore_TimerType_t timer,    ///< [IN] Timer Id
    uint32_t time,                  ///< [IN] Timer value in seconds
    lwm2mcore_TimerCallback_t cb    ///< [IN] Timer callback
)
{
    bool result = false;
    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Adaptation function for timer stop
 *
 * @return
 *      - true  on success
 *      - false on failure
 */
//--------------------------------------------------------------------------------------------------
bool lwm2mcore_TimerStop
(
    lwm2mcore_TimerType_t timer    ///< [IN] Timer Id
)
{
    bool result = false;
    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Adaptation function for timer state
 *
 * @return
 *      - true  if the timer is running
 *      - false if the timer is stopped
 */
//--------------------------------------------------------------------------------------------------
bool lwm2mcore_TimerIsRunning
(
    lwm2mcore_TimerType_t timer    ///< [IN] Timer Id
)
{
    bool isRunning = false;
    return isRunning;
}

