/**
 * @file timer.c
 *
 * Adaptation layer for timer management
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

#include <sys/time.h>
#include <signal.h>
#include <time.h>

#include <lwm2mcore/lwm2mcore.h>
#include <lwm2mcore/timer.h>
#include "internals.h"


//--------------------------------------------------------------------------------------------------
/**
 * Structure for timers
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    lwm2mcore_TimerType_t       timerType;  ///< Timer type
    lwm2mcore_TimerCallback_t   timerCb;    ///< Timer callback
    timer_t                     timerId;    ///< Timer Id
}
Lwm2mTimer_t;

//--------------------------------------------------------------------------------------------------
/**
 * Timer table
 */
//--------------------------------------------------------------------------------------------------
static Lwm2mTimer_t TimerTable[LWM2MCORE_TIMER_MAX];

//--------------------------------------------------------------------------------------------------
/**
 * Timer handler
 */
//--------------------------------------------------------------------------------------------------
static void TimerSigHandler
(
    int         signal,         ///< [IN] Signal number
    siginfo_t*  siPtr,          ///< [IN] New action for signal is installed from si
    void*       contextPtr      ///< [IN] Context
)
{
    (void)signal;
    (void)contextPtr;
    if (TimerTable[siPtr->si_value.sival_int].timerCb)
    {
        TimerTable[siPtr->si_value.sival_int].timerCb();
    }
    else
    {
        printf("No timer callback\n");
    }
}

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
    lwm2mcore_TimerType_t       timerType,  ///< [IN] Timer Id
    uint32_t                    time,       ///< [IN] Timer value in seconds
    lwm2mcore_TimerCallback_t   cb          ///< [IN] Timer callback
)
{
    bool result = false;
    struct sigevent sev;
    struct sigaction sa;
    printf("lwm2mcore_TimerSet time %d\n", time);

    if(lwm2mcore_TimerIsRunning(timerType))
    {
        lwm2mcore_TimerStop(timerType);
    }

    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = TimerSigHandler;
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGRTMIN, &sa, NULL) < 0)
    {
        printf("failed to set signal handler\n");
    }
    else
    {
        timer_t timerId;

        TimerTable[timerType].timerType = timerType;
        TimerTable[timerType].timerCb = cb;

        memset(&sev, 0, sizeof(struct sigevent));
        sev.sigev_notify = SIGEV_SIGNAL;
        sev.sigev_signo = SIGRTMIN;
        sev.sigev_value.sival_int = timerType;

        if (timer_create(CLOCK_REALTIME, &sev, &timerId))
        {
            printf("failed to create timer\n");

            // remove signal handler
            sa.sa_handler = SIG_DFL;
            if (sigaction(SIGRTMIN, &sa, NULL))
            {
                printf("failed to remove signal handler\n");
            }
        }
        else
        {
            struct itimerspec its;
            TimerTable[timerType].timerId = timerId;
            printf("set timer %d\n", time);

            if (!time)
            {
                its.it_value.tv_sec = 1;
                its.it_value.tv_nsec = 0;
            }
            else
            {
                its.it_value.tv_sec = time;
                its.it_value.tv_nsec = 0;
            }
            its.it_interval.tv_sec = 0;
            its.it_interval.tv_nsec = 0;

            LOG_ARG("timer sec %d", its.it_value.tv_sec);
            if (timer_settime(timerId, 0, &its, NULL) < 0)
            {
                printf("failed to set timer\n");
            }
            else
            {
                result = true;
            }
        }
    }
    printf("set timer result %d\n", result);
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
    lwm2mcore_TimerType_t timerType     ///< [IN] Timer Id
)
{
    if (TimerTable[timerType].timerId)
    {
        timer_delete(TimerTable[timerType].timerId);
        TimerTable[timerType].timerCb = NULL;
        return true;
    }
    return false;
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
    lwm2mcore_TimerType_t timerType     ///< [IN] Timer Id
)
{
    if (!(TimerTable[timerType].timerCb))
    {
        return false;
    }
    return true;
}
