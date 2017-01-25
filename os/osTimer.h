/**
 * @file osTimer.h
 *
 * Header file for adaptation layer for timer management
 *
 * <hr>
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#ifndef OS_TIMER_H
#define OS_TIMER_H

#include <stdbool.h>

//--------------------------------------------------------------------------------------------------
/**
 * Timer definitions
 */
//--------------------------------------------------------------------------------------------------
typedef enum _os_timerType
{
    OS_TIMER_STEP,      ///< Timer step
    OS_TIMER_MAX        ///< Maximum timer value (internal use)
}os_timerType_t;

//--------------------------------------------------------------------------------------------------
/**
 * Adaptation API for timer launch
 */
//--------------------------------------------------------------------------------------------------
typedef void* (*os_timerCallback_t)
(
    void* data
);

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
);

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
);

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
);

#endif //OS_TIMER_H

