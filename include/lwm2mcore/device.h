/**
 * @file device.h
 *
 * Porting layer for device parameters
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#ifndef __LWM2MCORE_DEVICE_H__
#define __LWM2MCORE_DEVICE_H__

#include <lwm2mcore/lwm2mcore.h>

/**
  * @addtogroup lwm2mcore_device_monitoring_IFS
  * @{
  */

//--------------------------------------------------------------------------------------------------
/**
 * @brief Retrieve the device manufacturer
 * This API needs to have a procedural treatment
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *  - @ref LWM2MCORE_ERR_OVERFLOW in case of buffer overflow
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetDeviceManufacturer
(
    char*   bufferPtr,  ///< [IN]    data buffer pointer
    size_t* lenPtr      ///< [INOUT] length of input buffer and length of the returned data
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Retrieve the device model number
 * This API needs to have a procedural treatment
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *  - @ref LWM2MCORE_ERR_OVERFLOW in case of buffer overflow
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetDeviceModelNumber
(
    char*   bufferPtr,  ///< [IN]    data buffer pointer
    size_t* lenPtr      ///< [INOUT] length of input buffer and length of the returned data
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Retrieve the device serial number
 * This API needs to have a procedural treatment
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *  - @ref LWM2MCORE_ERR_OVERFLOW in case of buffer overflow
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetDeviceSerialNumber
(
    char*   bufferPtr,  ///< [IN]    data buffer pointer
    size_t* lenPtr      ///< [INOUT] length of input buffer and length of the returned data
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Retrieve the device firmware version
 * This API needs to have a procedural treatment
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *  - @ref LWM2MCORE_ERR_OVERFLOW in case of buffer overflow
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetDeviceFirmwareVersion
(
    char*   bufferPtr,  ///< [IN]    data buffer pointer
    size_t* lenPtr      ///< [INOUT] length of input buffer and length of the returned data
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Retrieve the battery level (percentage)
 * This API needs to have a procedural treatment
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *  - @ref LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetBatteryLevel
(
    uint8_t* valuePtr  ///< [INOUT] data buffer
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Retrieve the device time (UNIX time in seconds)
 * This API needs to have a procedural treatment
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *  - @ref LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetDeviceCurrentTime
(
    uint64_t* valuePtr  ///< [INOUT] data buffer
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Retrieve the module identity (IMEI)
 * This API needs to have a procedural treatment
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *  - @ref LWM2MCORE_ERR_OVERFLOW in case of buffer overflow
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetDeviceImei
(
    char*   bufferPtr,  ///< [IN]    data buffer pointer
    size_t* lenPtr      ///< [INOUT] length of input buffer and length of the returned data
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Retrieve the SIM card identifier (ICCID)
 * This API needs to have a procedural treatment
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *  - @ref LWM2MCORE_ERR_OVERFLOW in case of buffer overflow
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetIccid
(
    char*   bufferPtr,  ///< [IN]    data buffer pointer
    size_t* lenPtr      ///< [INOUT] length of input buffer and length of the returned data
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Retrieve the subscription identity (MEID/ESN/IMSI)
 * This API needs to have a procedural treatment
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *  - @ref LWM2MCORE_ERR_OVERFLOW in case of buffer overflow
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetSubscriptionIdentity
(
    char*   bufferPtr,  ///< [IN]    data buffer pointer
    size_t* lenPtr      ///< [INOUT] length of input buffer and length of the returned data
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Retrieve the currently used SIM card
 * This API needs to have a procedural treatment
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetCurrentSimCard
(
    uint8_t*   currentSimPtr  ///< [OUT]    Currently used SIM card
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Set SIM mode
 * This API needs to have a procedural treatment
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_SetSimMode
(
    char*   bufferPtr,  ///< [IN]    data buffer pointer
    size_t* lenPtr      ///< [INOUT] length of input buffer and length of the returned data
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Retrieve the current SIM mode
 * This API needs to have a procedural treatment
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetCurrentSimMode
(
    uint8_t*   simModePtr  ///< [OUT]    SIM mode pointer
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Retrieve the SIM switch status
 * This API needs to have a procedural treatment
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetLastSimSwitchStatus
(
    uint8_t*   switchStatusPtr  ///< [OUT]    SIM switch status
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Retrieve the phone number (MSISDN)
 * This API needs to have a procedural treatment
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *  - @ref LWM2MCORE_ERR_OVERFLOW in case of buffer overflow
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetMsisdn
(
    char*   bufferPtr,  ///< [IN]    data buffer pointer
    size_t* lenPtr      ///< [INOUT] length of input buffer and length of the returned data
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Retrieve the device temperature (in °C)
 * This API needs to have a procedural treatment
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetDeviceTemperature
(
    int32_t* valuePtr   ///< [INOUT] data buffer
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Retrieve the number of unexpected resets
 * This API needs to have a procedural treatment
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetDeviceUnexpectedResets
(
    uint32_t* valuePtr  ///< [INOUT] data buffer
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Retrieve the total number of resets
 * This API needs to have a procedural treatment
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetDeviceTotalResets
(
    uint32_t* valuePtr  ///< [INOUT] data buffer
);

//--------------------------------------------------------------------------------------------------
/**
 * Request to reboot the device
 * This API needs to have a procedural treatment
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @note When LwM2MCore receives a reboot request from the server (EXECUTE on /3/0/4), a 2-second
 * timer is launched and this function is called by LwM2MCore when the timer expires. The client
 * needs to manage the platform reboot by implementing this function.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_RebootDevice
(
    void
);

/**
  * @}
  */

#endif /* __LWM2MCORE_DEVICE_H__ */
