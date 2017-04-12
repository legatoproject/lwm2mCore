/**
 * @file paramContent.h
 *
 * Header for LWM2MCore parameter content
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */


#ifndef  __PARAMCONTENT_H__
#define  __PARAMCONTENT_H__

#include <lwm2mcore/lwm2mcore.h>
#include <lwm2mcore/update.h>

//--------------------------------------------------------------------------------------------------
/**
 * Define for the number of supported server
 */
//--------------------------------------------------------------------------------------------------
#define SERVER_NUMBER LWM2MCORE_DM_SERVER_MAX_COUNT + LWM2MCORE_BOOTSRAP_SERVER_MAX_COUNT

//--------------------------------------------------------------------------------------------------
/**
 * Enum for security mode for LWM2M connection (object 0 (security); resource 2)
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    SEC_PSK,                ///< PSK
    SEC_RAW_PK,             ///< Raw PSK
    SEC_CERTIFICATE,        ///< Certificate
    SEC_NONE,               ///< No security
    SEC_MODE_MAX            ///< Internal use only
}SecurityMode_t;

//--------------------------------------------------------------------------------------------------
/**
 * Structure for the security object (object 0)
 * Serveur URI and credentials (PSKID, PSK) are managed as credentials
 * SMS parameters are not supported
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
  bool              isBootstrapServer;          ///< Is bootstrap server?
  SecurityMode_t    securityMode;               ///< Security mode
  uint16_t          serverId;                   ///< Short server ID
  uint16_t          clientHoldOffTime;          ///< Client hold off time
  uint32_t          bootstrapAccountTimeout;    ///< Bootstrap server account timeout
}ConfigSecurityObject_t;

//--------------------------------------------------------------------------------------------------
/**
 * Structure for the server object (object 1)
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
  uint16_t  serverId;                                    ///< Short server ID
  uint32_t  lifetime;                                    ///< lifetime in seconds
  uint16_t  defaultPmin;                                 ///< Default minimum period in seconds
  uint16_t  defaultPmax;                                 ///< Default maximum period in seconds
  bool      isDisable;                                   ///< Is device disabled?
  uint32_t  disableTimeout;                              ///< Disable timeout in seconds
  bool      isNotifStored;                               ///< Notification storing
  uint8_t   bindingMode[LWM2MCORE_BINDING_STR_MAX_LEN];  ///< Binding mode
}ConfigServerObject_t;

//--------------------------------------------------------------------------------------------------
/**
 * Structure for bootstrap configuration to be stored in platform storage
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
  uint32_t                  version;                    ///< Configuration version
  ConfigSecurityObject_t    security[SERVER_NUMBER];    ///< DM + BS server: security resources
  ConfigServerObject_t      server;                     ///< one DM server resources
}ConfigBootstrapFile_t;

#endif /*  __PARAMCONTENT_H__ */

