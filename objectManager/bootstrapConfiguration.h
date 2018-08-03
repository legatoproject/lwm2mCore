/**
 * @file bootstrapConfiguration.h
 *
 * Bootstrap management header
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#ifndef __BS_CONFIG_H__
#define __BS_CONFIG_H__

/**
  * @addtogroup lwm2mcore_bootstrap_int
  * @{
  */

//--------------------------------------------------------------------------------------------------
/**
 * @brief Bootstrap file version 1
 */
//--------------------------------------------------------------------------------------------------
#define BS_CONFIG_VERSION_1         1

//--------------------------------------------------------------------------------------------------
/**
 * @brief Bootstrap file version 2
 */
//--------------------------------------------------------------------------------------------------
#define BS_CONFIG_VERSION_2         2

//--------------------------------------------------------------------------------------------------
/**
 * @brief Supported version for bootstrap file
 */
//--------------------------------------------------------------------------------------------------
#define BS_CONFIG_VERSION           BS_CONFIG_VERSION_2

//--------------------------------------------------------------------------------------------------
/**
 * @brief Enum for security mode for LWM2M connection (object 0 (security); resource 2)
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    SEC_PSK,                ///< PSK
    SEC_RAW_PK,             ///< Raw PSK
    SEC_CERTIFICATE,        ///< Certificate
    SEC_NONE,               ///< No security
    SEC_MODE_MAX            ///< Internal use only
}
SecurityMode_t;

//--------------------------------------------------------------------------------------------------
/**
 * @brief Structure for the security object (object 0) for bootstrap configuration file version 1
 * Serveur URI and credentials (PSKID, PSK) are managed as credentials
 * SMS parameters are not supported
 *
 * @note Concerns bootstrap file version 1
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
  bool              isBootstrapServer;          ///< Is bootstrap server?
  SecurityMode_t    securityMode;               ///< Security mode
  uint16_t          serverId;                   ///< Short server ID
  uint16_t          clientHoldOffTime;          ///< Client hold off time
  uint32_t          bootstrapAccountTimeout;    ///< Bootstrap server account timeout
}
ConfigSecurityObjectV01_t;

//--------------------------------------------------------------------------------------------------
/**
 * @brief Structure for the server object (object 1) for bootstrap configuration file version 1
 *
 * @note Concerns bootstrap file version 1
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
}
ConfigServerObjectV01_t;

//--------------------------------------------------------------------------------------------------
/**
 * @brief Structure for bootstrap configuration file version 1 to be stored in platform storage
 *
 * @note Concerns bootstrap file version 1
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
  uint32_t                  version;                    ///< Configuration version
  ConfigSecurityObjectV01_t security[2];                ///< DM + BS server: security resources
  ConfigServerObjectV01_t   server;                     ///< one DM server resources
}
ConfigBootstrapFileV01_t;

//--------------------------------------------------------------------------------------------------
/**
 * @brief Structure for security objects (object 0) for bootstrap configuration file version 2 which
 * are stored in platform memory
 *
 * @note Concerns bootstrap file version 2
 */
//--------------------------------------------------------------------------------------------------
struct ConfigSecurityToStoreV02_t
{
    uint16_t        securityObjectInstanceId;               ///< Object instance Id of object 0
                                                            ///< (security)
    bool            isBootstrapServer;                      ///< Is bootstrap server?
    SecurityMode_t  securityMode;                           ///< Security mode
    uint16_t        serverId;                               ///< Short server ID
    uint16_t        clientHoldOffTime;                      ///< Client hold off time
    uint32_t        bootstrapAccountTimeout;                ///< Bootstrap server account timeout
};

//--------------------------------------------------------------------------------------------------
/**
 * @brief Structure for server objects (object 1) for bootstrap configuration file version 2 which
 * are stored in platform memory
 *
 * @note Concerns bootstrap file version 2
 */
//--------------------------------------------------------------------------------------------------
struct ConfigServerToStoreV02_t
{
    uint16_t    serverObjectInstanceId;                     ///< Object instance Id of object 1
                                                            ///< (server)
    uint16_t    serverId;                                   ///< Short server ID
    uint32_t    lifetime;                                   ///< lifetime in seconds
    uint32_t    defaultPmin;                                ///< Default minimum period in seconds
    uint32_t    defaultPmax;                                ///< Default maximum period in seconds
    bool        isDisable;                                  ///< Is device disabled?
    uint32_t    disableTimeout;                             ///< Disable timeout in seconds
    bool        isNotifStored;                              ///< Notification storing
    uint8_t     bindingMode[LWM2MCORE_BINDING_STR_MAX_LEN]; ///< Binding mode
};

//--------------------------------------------------------------------------------------------------
/**
 * @brief Structure for the security object (object 0)
 * Serveur URI and credentials (PSKID, PSK) are managed as credentials
 * SMS parameters are not supported
 *
 * @note Concerns bootstrap file version 2
 */
//--------------------------------------------------------------------------------------------------
struct ConfigSecurityObjectV02_t
{
    struct ConfigSecurityToStoreV02_t data;                         ///< Security data
    uint8_t         devicePKID[DTLS_PSK_MAX_CLIENT_IDENTITY_LEN];   ///< PSK identity
    uint16_t        pskIdLen;                                       ///< PSK identity length
    uint8_t         secretKey[DTLS_PSK_MAX_KEY_LEN];                ///< PSK secret
    uint16_t        pskLen;                                         ///< PSK secret length
    uint8_t         serverURI[LWM2MCORE_SERVER_URI_MAX_LEN];        ///< Server address
    struct ConfigSecurityObjectV02_t* nextPtr;                      ///< Next entry in the list
};

//--------------------------------------------------------------------------------------------------
/**
 * @brief Structure for the server object (object 1)
 *
 * @note Concerns bootstrap file version 2
 */
//--------------------------------------------------------------------------------------------------
struct ConfigServerObjectV02_t
{
    struct ConfigServerToStoreV02_t     data;                   ///< Server data
    struct ConfigServerObjectV02_t*     nextPtr;                ///< Next entry in the list
};

//--------------------------------------------------------------------------------------------------
/**
 * @brief Structure for bootstrap configuration file (version 2) to be stored in platform storage
 *
 * @note Concerns bootstrap file version 2
 */
//--------------------------------------------------------------------------------------------------
struct ConfigBootstrapFileV02_t
{
    uint32_t                            version;                ///< Configuration version
    uint16_t                            securityObjectNumber;   ///< Security objects number
    uint16_t                            serverObjectNumber;     ///< Server objects number
    struct ConfigSecurityObjectV02_t*   securityPtr;            ///< DM + BS server: security
                                                                ///< resources
    struct ConfigServerObjectV02_t*     serverPtr;              ///< DM servers resources
};

//--------------------------------------------------------------------------------------------------
/**
 * @brief Generic structure for bootstrap configuration to be stored in platform storage
 */
//--------------------------------------------------------------------------------------------------
typedef struct ConfigBootstrapFileV02_t     ConfigBootstrapFile_t;

//--------------------------------------------------------------------------------------------------
/**
 * @brief Generic structure for security objects for bootstrap configuration
 */
//--------------------------------------------------------------------------------------------------
typedef struct ConfigSecurityObjectV02_t    ConfigSecurityObject_t;

//--------------------------------------------------------------------------------------------------
/**
 * @brief Generic structure for server objects for bootstrap configuration
 */
//--------------------------------------------------------------------------------------------------
typedef struct ConfigServerObjectV02_t      ConfigServerObject_t;

//--------------------------------------------------------------------------------------------------
/**
 * @brief Generic structure for data linked to security objects for bootstrap configuration to be
 * stored in the platform memory
 */
//--------------------------------------------------------------------------------------------------
typedef struct ConfigSecurityToStoreV02_t   ConfigSecurityToStore_t;

//--------------------------------------------------------------------------------------------------
/**
 * @brief Generic structure for data linked to server objects for bootstrap configuration to be
 * stored in the platform memory
 */
//--------------------------------------------------------------------------------------------------
typedef struct ConfigServerToStoreV02_t     ConfigServerToStore_t;

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to get the bootstrap information from RAM
 *
 * @return
 *  - Boostrap configuration structure pointer
 */
//--------------------------------------------------------------------------------------------------
ConfigBootstrapFile_t* omanager_GetBootstrapConfiguration
(
    void
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to read the bootstrap configuration from platform memory
 *
 * @return
 *  - @c true in case of success
 *  - @c false in case of failure
 */
//--------------------------------------------------------------------------------------------------
bool omanager_LoadBootstrapConfiguration
(
    ConfigBootstrapFile_t*  configPtr,      ///< [OUT] Bootstrap configuration
    bool                    storage         ///< [IN] Indicates if the configuration needs to be
                                            ///<      stored
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to add an object instance of object 0 (security) in bootstrap information list
 */
//--------------------------------------------------------------------------------------------------
void omanager_AddBootstrapConfigurationSecurity
(
    ConfigBootstrapFile_t* bsConfigListPtr,         ///< [IN] Bootstrap information list
    ConfigSecurityObject_t* securityInformationPtr  ///< [IN] Security object
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to add an object instance of object 1 (server) in bootstrap information list
 */
//--------------------------------------------------------------------------------------------------
void omanager_AddBootstrapConfigurationServer
(
    ConfigBootstrapFile_t*  bsConfigListPtr,          ///< [IN] Bootstrap information list
    ConfigServerObject_t*   serverInformationPtr      ///< [IN] Server object
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to get the bootstrap information for a specific object instance Id of object 0
 * (security)
 *
 * @return
 *  - pointer on object instance structure on success
 *  - @c NULL if the object instance Id does not exist
 */
//--------------------------------------------------------------------------------------------------
ConfigSecurityObject_t* omanager_GetBootstrapConfigurationSecurityInstance
(
    ConfigBootstrapFile_t*  bsConfigListPtr,            ///< [IN] Bootstrap information list
    uint16_t                securityObjectInstanceId    ///< [IN] Security object instance Id
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to get the bootstrap information for a specific object instance Id of object 1
 * (server)
 *
 * @return
 *  - pointer on ConfigServerObject_t
 *  - @c NULL if the object instance Id does not exist
 */
//--------------------------------------------------------------------------------------------------
ConfigServerObject_t* omanager_GetBootstrapConfigurationServerInstance
(
    ConfigBootstrapFile_t*  bsConfigListPtr,            ///< [IN] Bootstrap information list
    uint16_t                serverObjectInstanceId      ///< [IN] Server object instance Id
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to save the bootstrap configuration in platform memory
 *
 * @return
 *  - @c true in case of success
 *  - @c false in case of failure
 */
//--------------------------------------------------------------------------------------------------
bool omanager_StoreBootstrapConfiguration
(
    ConfigBootstrapFile_t* bsConfigPtr              ///< [IN] Bootstrap configuration to store
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to free the bootstrap information list
 */
//--------------------------------------------------------------------------------------------------
void omanager_FreeBootstrapInformation
(
    void
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Delete all device management credentials
 *
 * @return
 *  - true if DM credentials were deleted
 *  - false if DM were not present
 */
//--------------------------------------------------------------------------------------------------
bool omanager_DeleteDmCredentials
(
    void
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to get the number of security and server objects in the bootstrap information
 *
 * @return
 *  - @c true on success
 *  - @c false on failure
 */
//--------------------------------------------------------------------------------------------------
bool omanager_GetBootstrapConfigObjectsNumber
(
    uint16_t* securityObjectNumberPtr,  ///< [IN] Number of security objects in the bootstrap
                                        ///< information
    uint16_t* serverObjectNumberPtr     ///< [IN] Number of server objects in the bootstrap
                                        ///< information
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to read the bootstrap configuration from platform memory
 *
 * @return
 *  - @c true in case of success
 *  - @c false in case of failure
 */
//--------------------------------------------------------------------------------------------------
bool omanager_LoadBootstrapConfigurationFile
(
    void
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to save the bootstrap configuration in platform memory
 *
 * @return
 *  - @c true in case of success
 *  - @c false in case of failure
 */
//--------------------------------------------------------------------------------------------------
bool omanager_SetBootstrapConfiguration
(
    void
);
/**
  * @}
  */

#endif /* __BS_CONFIG_H__ */
