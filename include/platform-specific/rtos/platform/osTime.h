/**
 * @file osTime.h
 *
 * Header file for adaptation layer for device time management
 *
 * <hr>
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#ifndef __OSTIME_H__
#define __OSTIME_H__

#include <time.h>

//--------------------------------------------------------------------------------------------------
/**
 * Function to retrieve the device time
 *
 * @return
 *      - device time (UNIX time: seconds since January 01, 1970)
 */
//--------------------------------------------------------------------------------------------------
time_t lwm2m_gettime
(
    void
);

#endif //__OSTIME_H__