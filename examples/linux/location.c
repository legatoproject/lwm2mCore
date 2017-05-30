/**
 * @file location.c
 *
 * Porting layer for location parameters
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#include <stdint.h>
#include <stddef.h>
#include <platform/types.h>
#include <lwm2mcore/lwm2mcore.h>
#include <lwm2mcore/location.h>

//--------------------------------------------------------------------------------------------------
/**
 * Retrieve the WSG84 latitude
 * This API treatment needs to have a procedural treatment
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - LWM2MCORE_ERR_OVERFLOW in case of buffer overflow
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetLatitude
(
    char*   bufferPtr,  ///< [IN]    data buffer pointer
    size_t* lenPtr      ///< [INOUT] length of input buffer and length of the returned data
)
{
    char latitude[] = "48.822880";
    size_t latitudeLen = strlen(latitude);

    if ((!bufferPtr) || (!lenPtr))
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    if (*lenPtr < latitudeLen)
    {
        return LWM2MCORE_ERR_OVERFLOW;
    }

    memcpy(bufferPtr, latitude, latitudeLen);
    *lenPtr = latitudeLen;

    return LWM2MCORE_ERR_COMPLETED_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Retrieve the WSG84 longitude
 * This API treatment needs to have a procedural treatment
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - LWM2MCORE_ERR_OVERFLOW in case of buffer overflow
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetLongitude
(
    char*   bufferPtr,  ///< [IN]    data buffer pointer
    size_t* lenPtr      ///< [INOUT] length of input buffer and length of the returned data
)
{
    char longitude[] = "2.249210";
    size_t longitudeLen = strlen(longitude);

    if ((!bufferPtr) || (!lenPtr))
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    if (*lenPtr < longitudeLen)
    {
        return LWM2MCORE_ERR_OVERFLOW;
    }

    memcpy(bufferPtr, longitude, longitudeLen);
    *lenPtr = longitudeLen;

    return LWM2MCORE_ERR_COMPLETED_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Retrieve the altitude (meters above sea level)
 * This API treatment needs to have a procedural treatment
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - LWM2MCORE_ERR_OVERFLOW in case of buffer overflow
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetAltitude
(
    char*   bufferPtr,  ///< [IN]    data buffer pointer
    size_t* lenPtr      ///< [INOUT] length of input buffer and length of the returned data
)
{
    char altitude[] = "41";
    size_t altitudeLen = strlen(altitude);

    if ((!bufferPtr) || (!lenPtr))
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    if (*lenPtr < altitudeLen)
    {
        return LWM2MCORE_ERR_OVERFLOW;
    }

    memcpy(bufferPtr, altitude, altitudeLen);
    *lenPtr = altitudeLen;

    return LWM2MCORE_ERR_COMPLETED_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Retrieve the direction of movement (range 0-359 degrees, where 0 is True North)
 * This API treatment needs to have a procedural treatment
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - LWM2MCORE_ERR_OVERFLOW in case of buffer overflow
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetDirection
(
    uint32_t* valuePtr  ///< [INOUT] data buffer
)
{
    if (!valuePtr)
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    *valuePtr = 257;

    return LWM2MCORE_ERR_COMPLETED_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Retrieve the horizontal speed in m/s
 * This API treatment needs to have a procedural treatment
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - LWM2MCORE_ERR_OVERFLOW in case of buffer overflow
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetHorizontalSpeed
(
    uint32_t* valuePtr  ///< [INOUT] data buffer
)
{
    if (!valuePtr)
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    // 92 m/s = 331 km/h
    *valuePtr = 92;

    return LWM2MCORE_ERR_COMPLETED_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Retrieve the vertical speed in m/s, positive up
 * This API treatment needs to have a procedural treatment
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - LWM2MCORE_ERR_OVERFLOW in case of buffer overflow
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetVerticalSpeed
(
    int32_t* valuePtr   ///< [INOUT] data buffer
)
{
    if (!valuePtr)
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    // -9 m/s = 32 km/h down
    *valuePtr = -9;

    return LWM2MCORE_ERR_COMPLETED_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Retrieve the timestamp of when the location measurement was performed
 * This API treatment needs to have a procedural treatment
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetLocationTimestamp
(
    uint64_t* valuePtr  ///< [INOUT] data buffer
)
{
    if (!valuePtr)
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    // UNIX time: 04/06/2017 @ 2:37pm (UTC)
    *valuePtr = 1491489442;

    return LWM2MCORE_ERR_COMPLETED_OK;
}
