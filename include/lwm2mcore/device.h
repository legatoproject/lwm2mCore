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
 * @brief Enumeration for resource 6 (available power sources) of LwM2M object 3 (device)
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    LWM2MCORE_DEVICE_PWR_SRC_TYPE_DC_POWER = 0,     ///< DC power
    LWM2MCORE_DEVICE_PWR_SRC_TYPE_BAT_INT,          ///< Internal battery
    LWM2MCORE_DEVICE_PWR_SRC_TYPE_BAT_EXT,          ///< External battery
    LWM2MCORE_DEVICE_PWR_SRC_TYPE_UNUSED,           ///< Unused
    LWM2MCORE_DEVICE_PWR_SRC_TYPE_PWR_OVER_ETH,     ///< Power over Ethernet
    LWM2MCORE_DEVICE_PWR_SRC_TYPE_USB,              ///< USB
    LWM2MCORE_DEVICE_PWR_SRC_TYPE_AC_POWER,         ///< AC power
    LWM2MCORE_DEVICE_PWR_SRC_TYPE_SOLAR             ///< Solar
}lwm2mcore_powerSource_enum_t;

//--------------------------------------------------------------------------------------------------
/**
 * @brief Enumeration for resource 20 (battery status) of LwM2M object 3 (device)
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    LWM2MCORE_DEVICE_BATTERY_NORMAL = 0,   ///< The battery is operating normally and not on power
    LWM2MCORE_DEVICE_BATTERY_CHARGING,     ///< The battery is currently charging
    LWM2MCORE_DEVICE_BATTERY_CHARGE_COMPLETE, ///< The battery is fully charged and still on power
    LWM2MCORE_DEVICE_BATTERY_DAMAGED,      ///< The battery has some problem
    LWM2MCORE_DEVICE_BATTERY_LOW,          ///< The battery is low on charge
    LWM2MCORE_DEVICE_BATTERY_NOT_INSTALL,  ///< The battery is not installed
    LWM2MCORE_DEVICE_BATTERY_UNKNOWN       ///< The battery information is not available
}lwm2mcore_batteryStatus_enum_t;

//--------------------------------------------------------------------------------------------------
/**
 * @brief Data structure representing the power source information.
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    lwm2mcore_powerSource_enum_t source;          ///< power source
    uint32_t    voltage;                          ///< power voltage
    uint16_t    current;                          ///< power current
    uint8_t     level;                            ///< battery level
    lwm2mcore_batteryStatus_enum_t status;        ///< battery status
}lwm2mcore_powerInfo_t;

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
LWM2MCORE_SHARED lwm2mcore_Sid_t lwm2mcore_GetDeviceManufacturer
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
LWM2MCORE_SHARED lwm2mcore_Sid_t lwm2mcore_GetDeviceModelNumber
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
LWM2MCORE_SHARED lwm2mcore_Sid_t lwm2mcore_GetDeviceSerialNumber
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
LWM2MCORE_SHARED lwm2mcore_Sid_t lwm2mcore_GetDeviceFirmwareVersion
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
LWM2MCORE_SHARED lwm2mcore_Sid_t lwm2mcore_GetBatteryLevel
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
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 */
//--------------------------------------------------------------------------------------------------
LWM2MCORE_SHARED lwm2mcore_Sid_t lwm2mcore_GetDeviceCurrentTime
(
    uint64_t* valuePtr  ///< [INOUT] data buffer
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Set the device time (UNIX time in seconds) into its system clock
 * This API needs to have a procedural treatment
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_SetDeviceCurrentTime
(
    uint64_t inputTime  ///< [IN] Current clock time given
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
LWM2MCORE_SHARED lwm2mcore_Sid_t lwm2mcore_GetDeviceImei
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
LWM2MCORE_SHARED lwm2mcore_Sid_t lwm2mcore_GetIccid
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
LWM2MCORE_SHARED lwm2mcore_Sid_t lwm2mcore_GetSubscriptionIdentity
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
LWM2MCORE_SHARED lwm2mcore_Sid_t lwm2mcore_GetCurrentSimCard
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
LWM2MCORE_SHARED lwm2mcore_Sid_t lwm2mcore_SetSimMode
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
LWM2MCORE_SHARED lwm2mcore_Sid_t lwm2mcore_GetCurrentSimMode
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
LWM2MCORE_SHARED lwm2mcore_Sid_t lwm2mcore_GetLastSimSwitchStatus
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
LWM2MCORE_SHARED lwm2mcore_Sid_t lwm2mcore_GetMsisdn
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
LWM2MCORE_SHARED lwm2mcore_Sid_t lwm2mcore_GetDeviceTemperature
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
LWM2MCORE_SHARED lwm2mcore_Sid_t lwm2mcore_GetDeviceUnexpectedResets
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
LWM2MCORE_SHARED lwm2mcore_Sid_t lwm2mcore_GetDeviceTotalResets
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
 * @warning The client MUST acknowledge this function before treating the reboot request, in order
 * to allow LwM2MCore to acknowledge the LwM2M server that the reboot request is correctly taken
 * into account.
 * Advice: launch a timer (value could be decided by the client implementation) in order to treat
 * the reboot request.
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

//--------------------------------------------------------------------------------------------------
/**
 * Retrieve the available power source
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 */
//--------------------------------------------------------------------------------------------------
LWM2MCORE_SHARED lwm2mcore_Sid_t lwm2mcore_GetAvailablePowerInfo
(
    lwm2mcore_powerInfo_t* powerInfoPtr,  ///< [INOUT] power source list
    size_t* powerNbPtr                    ///< [INOUT] power source nubmer
);

/**
  * @}
  */

#endif /* __LWM2MCORE_DEVICE_H__ */
