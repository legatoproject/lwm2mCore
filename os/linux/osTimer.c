/**
 * @file osTimer.c
 *
 * Adaptation layer for timer management
 *
 * <hr>
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */


#include <stdbool.h>
#include <stdint.h>
#include "osTimer.h"

//--------------------------------------------------------------------------------------------------
/**
 * Adaptation function for timer launch
 *
 * @return
 *      - true  on success
 *      - false on failure
 */
//--------------------------------------------------------------------------------------------------
bool os_timerSet
(
    os_timerType_t timer,   ///< [IN] Timer Id
    uint32_t time,          ///< [IN] Timer value in seconds
    os_timerCallback_t cb   ///< [IN] Timer callback
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
bool os_timerStop
(
    os_timerType_t timer    ///< [IN] Timer Id
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
bool os_timerIsRunning
(
    os_timerType_t timer    ///< [IN] Timer Id
)
{
    bool isRunning = false;
    return isRunning;
}

