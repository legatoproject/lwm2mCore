/**
 * @file clockTimeConfiguration.h
 *
 * Clock time configuration management header
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#ifndef __CLOCK_TIME_CONFIG_H__
#define __CLOCK_TIME_CONFIG_H__

/**
  * @addtogroup lwm2mcore_clockTimeConfiguration_int
  * @{
  */


//--------------------------------------------------------------------------------------------------
/**
 * Clock source types defined: TP, NTP, GPS
 * CLOCK_SOURCE_IS_VALID(source) validates whether the given source is within the valid range
 */
//--------------------------------------------------------------------------------------------------
#define CLOCK_TIME_CONFIG_SOURCE_MAX 3
#define CLOCK_SOURCE_IS_VALID(source) ((source) < CLOCK_TIME_CONFIG_SOURCE_MAX)

/**
  * @}
  */

#endif /* __CLOCK_TIME_CONFIG_H__ */
