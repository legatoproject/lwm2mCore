/**
 * @file timer.h
 *
 * Header file for adaptation layer for timer management
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#ifndef __LWM2MCORE_TIMER_H__
#define __LWM2MCORE_TIMER_H__

#include <stdint.h>
#include <platform/types.h>

//--------------------------------------------------------------------------------------------------
/**
 * Timer definitions
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    LWM2MCORE_TIMER_STEP,      ///< Timer step
    LWM2MCORE_TIMER_MAX        ///< Maximum timer value (internal use)
}lwm2mcore_TimerType_t;

//--------------------------------------------------------------------------------------------------
/**
 * Adaptation API for timer launch
 */
//--------------------------------------------------------------------------------------------------
typedef void* (*lwm2mcore_TimerCallback_t)
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
bool lwm2mcore_TimerSet
(
    lwm2mcore_TimerType_t timer,    ///< [IN] Timer Id
    uint32_t time,                  ///< [IN] Timer value in seconds
    lwm2mcore_TimerCallback_t cb    ///< [IN] Timer callback
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
bool lwm2mcore_TimerStop
(
    lwm2mcore_TimerType_t timer    ///< [IN] Timer Id
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
bool lwm2mcore_TimerIsRunning
(
    lwm2mcore_TimerType_t timer    ///< [IN] Timer Id
);

#endif /* __LWM2MCORE_TIMER_H__ */
