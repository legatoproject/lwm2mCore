/**
 * @file time.c
 *
 * Adaptation layer for time
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#include <stdio.h>
#include <sys/time.h>
#include <liblwm2m.h>

//--------------------------------------------------------------------------------------------------
/**
 * Function to retreive the device time
 *
 * @return
 *      - device time (UNIX time: seconds since January 01, 1970)
 */
//--------------------------------------------------------------------------------------------------
time_t lwm2m_gettime(void)
{
    struct timeval tv;

    if (0 != gettimeofday(&tv, NULL))
    {
        return -1;
    }

    return tv.tv_sec;
}


