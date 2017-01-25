/**
 * @file lwm2mcorePortDeviceDefault.c
 *
 * Porting layer for device parameters
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "../inc/lwm2mcoreObjectHandler.h"
#include "lwm2mcorePortDevice.h"

//--------------------------------------------------------------------------------------------------
/**
 *                  OBJECT 3: DEVICE
 */
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Retrieve the device manufacturer
 * This API treatment needs to have a procedural treatment
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treament succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_sid_t os_portDeviceManufacturer
(
    char *bufferPtr,                        ///< [INOUT] data buffer
    size_t *lenPtr                          ///< [INOUT] length of input buffer and length of the
                                            ///< returned data
)
{
    lwm2mcore_sid_t result = LWM2MCORE_ERR_OP_NOT_SUPPORTED;

    if ((bufferPtr == NULL) || (lenPtr == NULL))
    {
        result = LWM2MCORE_ERR_INVALID_ARG;
    }
    else
    {
        memcpy (bufferPtr, "Sierra", 6);
        *lenPtr = 6;
        result = LWM2MCORE_ERR_COMPLETED_OK;
    }
    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Retrieve the device model number
 * This API treatment needs to have a procedural treatment
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treament succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_sid_t os_portDeviceModelNumber
(
    char *bufferPtr,                        ///< [INOUT] data buffer
    size_t *lenPtr                          ///< [INOUT] length of input buffer and length of the
                                            ///< returned data
)
{
    lwm2mcore_sid_t result = LWM2MCORE_ERR_OP_NOT_SUPPORTED;

    if ((bufferPtr == NULL) || (lenPtr == NULL))
    {
        result = LWM2MCORE_ERR_INVALID_ARG;
    }
    else
    {
        memcpy (bufferPtr, "Sierra device", 13);
        *lenPtr = 13;
        result = LWM2MCORE_ERR_COMPLETED_OK;
    }
    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Retrieve the device serial number
 * This API treatment needs to have a procedural treatment
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treament succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_sid_t os_portDeviceSerialNumber
(
    char *bufferPtr,                        ///< [INOUT] data buffer
    size_t *lenPtr                          ///< [INOUT] length of input buffer and length of the
                                            ///< returned data
)
{
    lwm2mcore_sid_t result = LWM2MCORE_ERR_OP_NOT_SUPPORTED;

    if ((bufferPtr == NULL) || (lenPtr == NULL))
    {
        result = LWM2MCORE_ERR_INVALID_ARG;
    }
    else
    {
        memcpy (bufferPtr, "0123456789", 10);
        *lenPtr = 10;
        result = LWM2MCORE_ERR_COMPLETED_OK;
    }
    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Retrieve the firmware version
 * This API treatment needs to have a procedural treatment
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treament succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_sid_t os_portDeviceFirmwareVersion
(
    char *bufferPtr,                        ///< [INOUT] data buffer
    size_t *lenPtr                          ///< [INOUT] length of input buffer and length of the
                                            ///< returned data
)
{
    lwm2mcore_sid_t result = LWM2MCORE_ERR_OP_NOT_SUPPORTED;

    if ((bufferPtr == NULL) || (lenPtr == NULL))
    {
        result = LWM2MCORE_ERR_INVALID_ARG;
    }
    else
    {
        memcpy (bufferPtr, "FW v1.0", 7);
        *lenPtr = 7;
        result = LWM2MCORE_ERR_COMPLETED_OK;
    }
    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Retrieve the device time: UNIX time in seconds
 * This API treatment needs to have a procedural treatment
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treament succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_sid_t os_portDeviceCurrentTime
(
    uint64_t* valuePtr                      ///< [INOUT] data buffer
)
{
    lwm2mcore_sid_t result = LWM2MCORE_ERR_OP_NOT_SUPPORTED;

    if (valuePtr == NULL)
    {
        result = LWM2MCORE_ERR_INVALID_ARG;
    }
    else
    {
        /* UNIX time: December 25th, 2016, 11:59:59 pm */
        *valuePtr = 1482710399;
        result = LWM2MCORE_ERR_COMPLETED_OK;
    }
    return result;
}

