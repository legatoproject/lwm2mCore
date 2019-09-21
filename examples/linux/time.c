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
#include <lwm2mcore/lwm2mcore.h>

//--------------------------------------------------------------------------------------------------
/**
 * Function to retreive the device time
 *
 * @return
 *  - device time (UNIX time: seconds since January 01, 1970)
 */
//--------------------------------------------------------------------------------------------------
time_t lwm2m_gettime(void)
{
    return time(NULL);
}


//--------------------------------------------------------------------------------------------------
/**
 * Function to retrieve the configured priority of the given clock time source from the config tree
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the info retrieval has succeeded
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the info retrieval has failed
 *      - LWM2MCORE_ERR_INVALID_ARG if a config parameter is invalid
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the retrieved value is out of the proper range
 */
//--------------------------------------------------------------------------------------------------
int lwm2mcore_GetClockTimeSourcePriority
(
    uint16_t source,
    int16_t* priority
)
{
    *priority = 0;
    printf("Clock source %d with priority %d\n", source, *priority);
    return LWM2MCORE_ERR_COMPLETED_OK;
}


//--------------------------------------------------------------------------------------------------
/**
 * Function to set the priority of the given clock time source provided in the input onto the
 * config tree
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the info retrieval has succeeded
 *      - LWM2MCORE_ERR_INVALID_ARG if the provided input is invalid
 */
//--------------------------------------------------------------------------------------------------
int lwm2mcore_SetClockTimeSourcePriority
(
    uint16_t source,
    int16_t priority
)
{
    printf("Clock source %d with priority %d\n", source, priority);
    return LWM2MCORE_ERR_COMPLETED_OK;
}


//--------------------------------------------------------------------------------------------------
/**
 * Function to retrieve the clock time source config as server name, IPv4/v6 address, etc., from
 * the config tree
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the info retrieval has succeeded
 *      - LWM2MCORE_ERR_INVALID_ARG if there is no source config configured to be retrieved
 */
//--------------------------------------------------------------------------------------------------
int lwm2mcore_GetClockTimeSourceConfig
(
    uint16_t source,
    char* bufferPtr,
    size_t* lenPtr
)
{
    printf("Clock source %d with bufferPtr %p, length %d\n", source, bufferPtr, (int)*lenPtr);
    return LWM2MCORE_ERR_COMPLETED_OK;
}


//--------------------------------------------------------------------------------------------------
/**
 * Function to set the clock time source config as server name, IPv4/v6 address, etc., onto the
 * config tree
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the input has been successfully set
 *      - LWM2MCORE_ERR_INVALID_ARG if the input is invalid
 */
//--------------------------------------------------------------------------------------------------
int lwm2mcore_SetClockTimeSourceConfig
(
    uint16_t source,
    char* bufferPtr,
    size_t length
)
{
    printf("Clock source %d with bufferPtr %p, length %d\n", source, bufferPtr, (int)length);
    return LWM2MCORE_ERR_COMPLETED_OK;
}


//--------------------------------------------------------------------------------------------------
/**
 * Function to execute the device's system clock update by acquiring it from the clock source(s)
 * configured and, if successful, setting it in
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if successful
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED if this functionality is not supported
 */
//--------------------------------------------------------------------------------------------------
int lwm2mcore_ExecuteClockTimeUpdate
(
    char* bufferPtr,
    size_t length
)
{
    printf("bufferPtr: %p, length: %d\n", bufferPtr, (int)length);
    return LWM2MCORE_ERR_COMPLETED_OK;
}


//--------------------------------------------------------------------------------------------------
/**
 * Function to retrieve the status of the last execution of clock time update
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the status retrieval has succeeded
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the status retrieval has failed
 *      - LWM2MCORE_ERR_INVALID_ARG if there is no status to be retrieved
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the retrieved status is out of the proper range
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED if this functionality is not supported
 */
//--------------------------------------------------------------------------------------------------
int lwm2mcore_GetClockTimeStatus
(
    uint16_t source,
    int16_t* status
)
{
    *status = 0;
    printf("Clock source %d with status %d\n", source, *status);
    return LWM2MCORE_ERR_COMPLETED_OK;
}



//--------------------------------------------------------------------------------------------------
/**
 * Function to return a boolean to reveal whether the Clock Service is the process of doing a
 * system clock update.
 *
 * @return
 *      - true if a system clock update is in progress
 *      - false otherwise
 */
//--------------------------------------------------------------------------------------------------
bool lwm2mcore_UpdateSystemClockInProgress
(
    void
)
{
    return false;
}
