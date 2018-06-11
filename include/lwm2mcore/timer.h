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

/**
  * @addtogroup lwm2mcore_platform_adaptor_timer_IFS
  * @{
  */

//--------------------------------------------------------------------------------------------------
/**
 * @brief Timer definitions
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    LWM2MCORE_TIMER_STEP,           ///< Timer step
    LWM2MCORE_TIMER_INACTIVITY,     ///< Inactivity timer
    LWM2MCORE_TIMER_REBOOT,         ///< Reboot expiration timer
    LWM2MCORE_TIMER_MAX             ///< Maximum timer value (internal use)
}lwm2mcore_TimerType_t;

//--------------------------------------------------------------------------------------------------
/**
 * @brief Callback when a timer expires
 */
//--------------------------------------------------------------------------------------------------
typedef void (*lwm2mcore_TimerCallback_t)
(
    void
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Adaptation function for timer start
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @c true  on success
 *  - @c false on failure
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
 * @brief Adaptation function for timer stop
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @c true  on success
 *  - @c false on failure
 */
//--------------------------------------------------------------------------------------------------
bool lwm2mcore_TimerStop
(
    lwm2mcore_TimerType_t timer    ///< [IN] Timer Id
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Adaptation function for timer state
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @c true  if the timer is running
 *  - @c false if the timer is stopped
 */
//--------------------------------------------------------------------------------------------------
bool lwm2mcore_TimerIsRunning
(
    lwm2mcore_TimerType_t timer    ///< [IN] Timer Id
);

/**
  * @}
  */

#endif /* __LWM2MCORE_TIMER_H__ */
