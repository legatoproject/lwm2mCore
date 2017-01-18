/**
 * @file lwm2mcoreObjectHandler.h
 *
 * Header file for object resources handlers
 *
 * Copyright (C) Sierra Wireless Inc. Use of this work is subject to license.
 *
 */


#ifndef LWM2CORE_OBJECTHANDLER_H_
#define LWM2CORE_OBJECTHANDLER_H_

//--------------------------------------------------------------------------------------------------
/** Lifetime value to indicate that the lifetime is deactivated
 * This is compliant with the LWM2M specification and a 0-value has no sense
 * 630720000 = 20 years
 * This is used if the customer does not wan any "automatic" connection to the server
 */
//--------------------------------------------------------------------------------------------------
#define LWM2MCORE_LIFETIME_VALUE_DISABLED       630720000

//--------------------------------------------------------------------------------------------------
/**
 * Enumeration for handlers status ID code (returned value)
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    LWM2MCORE_ERR_COMPLETED_OK = 0,             ///< Handler treatment is OK
    LWM2MCORE_ERR_GENERAL_ERROR = -1,           ///< Handler treatment failed
    LWM2MCORE_ERR_INCORRECT_RANGE = -2,         ///< Bad parameter range (WRITE operation)
    LWM2MCORE_ERR_NOT_YET_IMPLEMENTED = -3,     ///< Not yet implemented resource
    LWM2MCORE_ERR_OP_NOT_SUPPORTED = -4,        ///< Not supported resource
    LWM2MCORE_ERR_INVALID_ARG = -5,             ///< Invalid parameter in resource handler
    LWM2MCORE_ERR_INVALID_STATE = -6,           ///< Invalid state to treat the resource handler
    LWM2MCORE_ERR_OVERFLOW = -7                 ///< Buffer overflow
}lwm2mcore_sid_t;

//--------------------------------------------------------------------------------------------------
/**
 * Enum for security mode for LWM2M connection (object 0 (security); resource 2)
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    LWM2MCORE_SEC_PSK = 0,          ///< PSK
    LWM2MCORE_SEC_RAW_PK,           ///< Raw PSK
    LWM2MCORE_SEC_CERTIFICATE,      ///< Certificate
    LWM2MCORE_SEC_NONE,             ///< No security
    LWM2MCORE_SEC_MODE_MAX          ///<internal use only
}lwm2mcore_security_mode_t;

//--------------------------------------------------------------------------------------------------
/**
 * Enum for security mode for SMS (object 0 (security); resources 6)
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
  LWM2MCORE_SSM_RESERVED = 0,                   ///< Default value
  LWM2MCORE_SSM_SECURE_PKT_STRUCT_DEVICE,       ///<
  LWM2MCORE_SSM_SECURE_PKT_STRUCT_SMARTCARD,    ///<
  LWM2MCORE_SSM_NOSEC,                          ///< no security
  LWM2MCORE_SSM_PROPRIETARY = 255,              ///< Internal use
}lwm2mcore_sms_security_mode_t;

#endif /* LWM2CORE_OBJECTHANDLER_H_ */

