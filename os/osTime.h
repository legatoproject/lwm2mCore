/**
 * @file osTime.h
 *
 * Header file for adaptation layer for device time management
 *
 * <hr>
 *
 * Copyright (C) Sierra Wireless Inc. Use of this work is subject to license.
 *
 */

#ifndef OS_TIME_H
#define OS_TIME_H

#include <stdbool.h>

//--------------------------------------------------------------------------------------------------
/**
 * Function to retreive the device time
 *
 * @return
 *      - device time (UNIX time: seconds since January 01, 1970)
 */
//--------------------------------------------------------------------------------------------------
time_t lwm2m_gettime
(
    void
);

#endif //OS_TIME_H

