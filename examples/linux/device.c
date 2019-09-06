/**
 * @file device.c
 *
 * Porting layer for device parameters
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <platform/types.h>
#include <lwm2mcore/lwm2mcore.h>
#include <lwm2mcore/device.h>
#include <time.h>
#include "clientConfig.h"

//--------------------------------------------------------------------------------------------------
/**
 * Retrieve the device manufacturer
 * This API needs to have a procedural treatment
 *
 * @return
 *  - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *  - LWM2MCORE_ERR_OVERFLOW in case of buffer overflow
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetDeviceManufacturer
(
    char*   bufferPtr,  ///< [IN]    data buffer pointer
    size_t* lenPtr      ///< [INOUT] length of input buffer and length of the returned data
)
{
    char manufacturer[] = "Sierra Wireless";
    size_t manufacturerLen = strlen(manufacturer);

    if ((!bufferPtr) || (!lenPtr))
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    if (*lenPtr < manufacturerLen)
    {
        return LWM2MCORE_ERR_OVERFLOW;
    }

    memcpy(bufferPtr, manufacturer, manufacturerLen);
    *lenPtr = manufacturerLen;

    return LWM2MCORE_ERR_COMPLETED_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Retrieve the device model number
 * This API needs to have a procedural treatment
 *
 * @return
 *  - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *  - LWM2MCORE_ERR_OVERFLOW in case of buffer overflow
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetDeviceModelNumber
(
    char*   bufferPtr,  ///< [IN]    data buffer pointer
    size_t* lenPtr      ///< [INOUT] length of input buffer and length of the returned data
)
{
    char modelNumber[] = "Sierra device";
    size_t modelNumberLen = strlen(modelNumber);

    if ((!bufferPtr) || (!lenPtr))
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    if (*lenPtr < modelNumberLen)
    {
        return LWM2MCORE_ERR_OVERFLOW;
    }

    memcpy(bufferPtr, modelNumber, modelNumberLen);
    *lenPtr = modelNumberLen;

    return LWM2MCORE_ERR_COMPLETED_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Retrieve the device serial number
 * This API needs to have a procedural treatment
 *
 * @return
 *  - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - LWM2MCORE_ERR_GENERAL_ERROR if client configuration is not available
 *  - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *  - LWM2MCORE_ERR_OVERFLOW in case of buffer overflow
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetDeviceSerialNumber
(
    char*   bufferPtr,  ///< [IN]    data buffer pointer
    size_t* lenPtr      ///< [INOUT] length of input buffer and length of the returned data
)
{
    size_t serialNumberLen;
    clientConfig_t* config = ClientConfigGet();
    if (!config)
    {
        return LWM2MCORE_ERR_GENERAL_ERROR;
    }

    serialNumberLen = strlen(config->general.SN);

    if ((!bufferPtr) || (!lenPtr))
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    if (*lenPtr < serialNumberLen)
    {
        return LWM2MCORE_ERR_OVERFLOW;
    }

    memcpy(bufferPtr, config->general.SN, serialNumberLen);
    *lenPtr = serialNumberLen;

    return LWM2MCORE_ERR_COMPLETED_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Retrieve the firmware version
 * This API needs to have a procedural treatment
 *
 * @return
 *  - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *  - LWM2MCORE_ERR_OVERFLOW in case of buffer overflow
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetDeviceFirmwareVersion
(
    char*   bufferPtr,  ///< [IN]    data buffer pointer
    size_t* lenPtr      ///< [INOUT] length of input buffer and length of the returned data
)
{
    char fwVersion[] = "FW v1.0";
    size_t fwVersionLen = strlen(fwVersion);

    if ((!bufferPtr) || (!lenPtr))
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    if (*lenPtr < fwVersionLen)
    {
        return LWM2MCORE_ERR_OVERFLOW;
    }

    memcpy(bufferPtr, fwVersion, fwVersionLen);
    *lenPtr = fwVersionLen;

    return LWM2MCORE_ERR_COMPLETED_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Retrieve the battery level (percentage)
 * This API needs to have a procedural treatment
 *
 * @return
 *  - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetBatteryLevel
(
    uint8_t* valuePtr  ///< [INOUT] data buffer
)
{
    if (!valuePtr)
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    /* Battery level of 57% */
    *valuePtr = 57;

    return LWM2MCORE_ERR_COMPLETED_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Retrieve the device time (UNIX time in seconds)
 * This API needs to have a procedural treatment
 *
 * @return
 *  - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetDeviceCurrentTime
(
    uint64_t* valuePtr  ///< [INOUT] data buffer
)
{
    if (!valuePtr)
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    *valuePtr = (uint64_t)time(NULL);

    return LWM2MCORE_ERR_COMPLETED_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Set the device time (UNIX time in seconds) into its system clock
 * This API needs to have a procedural treatment
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_SetDeviceCurrentTime
(
    uint64_t inputTime  ///< [IN] Current clock time given
)
{
    (void)inputTime;
    return LWM2MCORE_ERR_COMPLETED_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Retrieve the module identity (IMEI)
 * This API needs to have a procedural treatment
 *
 * @return
 *  - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - LWM2MCORE_ERR_GENERAL_ERROR if client configuration is not available
 *  - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *  - LWM2MCORE_ERR_OVERFLOW in case of buffer overflow
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetDeviceImei
(
    char*   bufferPtr,  ///< [IN]    data buffer pointer
    size_t* lenPtr      ///< [INOUT] length of input buffer and length of the returned data
)
{
    size_t imeiLen;
    clientConfig_t* config = ClientConfigGet();
    if (!config)
    {
        return LWM2MCORE_ERR_GENERAL_ERROR;
    }

    imeiLen = strlen(config->general.IMEI);

    if ((!bufferPtr) || (!lenPtr))
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    if (*lenPtr < imeiLen)
    {
        return LWM2MCORE_ERR_OVERFLOW;
    }

    memcpy(bufferPtr, config->general.IMEI, imeiLen);
    *lenPtr = imeiLen;

    return LWM2MCORE_ERR_COMPLETED_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Retrieve the SIM card identifier (ICCID)
 * This API needs to have a procedural treatment
 *
 * @return
 *  - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *  - LWM2MCORE_ERR_OVERFLOW in case of buffer overflow
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetIccid
(
    char*   bufferPtr,  ///< [IN]    data buffer pointer
    size_t* lenPtr      ///< [INOUT] length of input buffer and length of the returned data
)
{
    char iccid[] = "01234567890123456789";
    size_t iccidLen = strlen(iccid);

    if ((!bufferPtr) || (!lenPtr))
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    if (*lenPtr < iccidLen)
    {
        return LWM2MCORE_ERR_OVERFLOW;
    }

    memcpy(bufferPtr, iccid, iccidLen);
    *lenPtr = iccidLen;

    return LWM2MCORE_ERR_COMPLETED_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Retrieve the subscription identity (MEID/ESN/IMSI)
 * This API needs to have a procedural treatment
 *
 * @return
 *  - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *  - LWM2MCORE_ERR_OVERFLOW in case of buffer overflow
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetSubscriptionIdentity
(
    char*   bufferPtr,  ///< [IN]    data buffer pointer
    size_t* lenPtr      ///< [INOUT] length of input buffer and length of the returned data
)
{
    char identity[] = "0123456789012345";
    size_t identityLen = strlen(identity);

    if ((!bufferPtr) || (!lenPtr))
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    if (*lenPtr < identityLen)
    {
        return LWM2MCORE_ERR_OVERFLOW;
    }

    memcpy(bufferPtr, identity, identityLen);
    *lenPtr = identityLen;

    return LWM2MCORE_ERR_COMPLETED_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Retrieve the currently used SIM card
 * This API needs to have a procedural treatment
 *
 * @note
 *      This function is stubbed
 * @return
 *  - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetCurrentSimCard
(
    uint8_t*   currentSimPtr  ///< [OUT]    Currently used SIM card
)
{
    if (!currentSimPtr)
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    *currentSimPtr = 0;
    return LWM2MCORE_ERR_COMPLETED_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Set SIM mode
 * This API needs to have a procedural treatment
 *
 * @note
 *      This function is stubbed
 * @return
 *  - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_SetSimMode
(
    char*   bufferPtr,  ///< [IN]    data buffer pointer
    size_t* lenPtr      ///< [INOUT] length of input buffer and length of the returned data
)
{
    if ((!bufferPtr) || (!lenPtr))
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    return LWM2MCORE_ERR_COMPLETED_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Retrieve the current SIM mode
 * This API needs to have a procedural treatment
 *
 * @return
 *  - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetCurrentSimMode
(
    uint8_t*   simModePtr  ///< [OUT]    SIM mode pointer
)
{
    if (!simModePtr)
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    *simModePtr = 0;
    return LWM2MCORE_ERR_COMPLETED_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Retrieve the last SIM switch status
 * This API needs to have a procedural treatment
 *
 * @note
 *      This function is stubbed
 * @return
 *  - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetLastSimSwitchStatus
(
    uint8_t*   switchStatusPtr  ///< [OUT]    SIM switch status
)
{
    if (!switchStatusPtr)
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    *switchStatusPtr = 0;
    return LWM2MCORE_ERR_COMPLETED_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Retrieve the phone number (MSISDN)
 * This API needs to have a procedural treatment
 *
 * @return
 *  - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *  - LWM2MCORE_ERR_OVERFLOW in case of buffer overflow
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetMsisdn
(
    char*   bufferPtr,  ///< [IN]    data buffer pointer
    size_t* lenPtr      ///< [INOUT] length of input buffer and length of the returned data
)
{
    char msisdn[] = "+33123456789";
    size_t msisdnLen = strlen(msisdn);

    if ((!bufferPtr) || (!lenPtr))
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    if (*lenPtr < msisdnLen)
    {
        return LWM2MCORE_ERR_OVERFLOW;
    }

    memcpy(bufferPtr, msisdn, msisdnLen);
    *lenPtr = msisdnLen;

    return LWM2MCORE_ERR_COMPLETED_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Retrieve the device temperature (in °C)
 * This API needs to have a procedural treatment
 *
 * @return
 *  - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetDeviceTemperature
(
    int32_t* valuePtr   ///< [INOUT] data buffer
)
{
    if (!valuePtr)
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    *valuePtr = 26;

    return LWM2MCORE_ERR_COMPLETED_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Retrieve the number of unexpected resets
 * This API needs to have a procedural treatment
 *
 * @return
 *  - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetDeviceUnexpectedResets
(
    uint32_t* valuePtr  ///< [INOUT] data buffer
)
{
    if (!valuePtr)
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    *valuePtr = 2;

    return LWM2MCORE_ERR_COMPLETED_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Retrieve the total number of resets
 * This API needs to have a procedural treatment
 *
 * @return
 *  - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetDeviceTotalResets
(
    uint32_t* valuePtr  ///< [INOUT] data buffer
)
{
    if (!valuePtr)
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    *valuePtr = 10;

    return LWM2MCORE_ERR_COMPLETED_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Request to reboot the device
 * This API needs to have a procedural treatment
 *
 * @warning The client MUST acknowledge this function before treating the reboot request, in order
 * to allow LwM2MCore to acknowledge the LwM2M server that the reboot request is correctly taken
 * into account.
 * Advice: launch a timer (value could be decided by the client implementation) in order to treat
 * the reboot request.
 *
 * @return
 *  - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *  - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *  - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *  - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_RebootDevice
(
    void
)
{
    return LWM2MCORE_ERR_NOT_YET_IMPLEMENTED;
}
