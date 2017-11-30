/**
 * @file handlers.c
 *
 * client of the LWM2M stack
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#include <stdint.h>
#include <stddef.h>
#include <platform/types.h>
#include <lwm2mcore/lwm2mcore.h>
#include <lwm2mcore/connectivity.h>
#include <lwm2mcore/device.h>
#include <lwm2mcore/security.h>
#include <lwm2mcore/server.h>
#include <lwm2mcore/paramStorage.h>
#include <lwm2mcore/update.h>
#include <lwm2mcore/location.h>
#include "handlers.h"
#include "sessionManager.h"
#include "objects.h"
#include "internals.h"
#include "utils.h"
#include "liblwm2m.h"

//--------------------------------------------------------------------------------------------------
/**
 * Maximum number of bytes in the Universal Geographical Area Description of velocity
 * GAD is defined in the 3GPP 23.032 standard, section 8
 */
//--------------------------------------------------------------------------------------------------
#define LWM2MCORE_GAD_VELOCITY_MAX_BYTES    7

//--------------------------------------------------------------------------------------------------
/**
 * Default value for disable timeout
 */
//--------------------------------------------------------------------------------------------------
#define DEFAULT_DISABLE_TIMEOUT     86400

//--------------------------------------------------------------------------------------------------
/**
 * Default value for minimum period
 */
//--------------------------------------------------------------------------------------------------
#define DEFAULT_P_MIN               30

//--------------------------------------------------------------------------------------------------
/**
 * Default value for minimum period
 */
//--------------------------------------------------------------------------------------------------
#define DEFAULT_P_MAX               60

//--------------------------------------------------------------------------------------------------
/**
 * Default value for bootstrap short server Id
 */
//--------------------------------------------------------------------------------------------------
#define DEFAULT_BS_SERVER_ID        0

//--------------------------------------------------------------------------------------------------
/**
 * Default value for device management short server Id
 */
//--------------------------------------------------------------------------------------------------
#define DEFAULT_DM_SERVER_ID        1

//--------------------------------------------------------------------------------------------------
/**
 * Bootstrap file version 1
 */
//--------------------------------------------------------------------------------------------------
#define BS_CONFIG_VERSION_1         1

//--------------------------------------------------------------------------------------------------
/**
 * Bootstrap file version 2
 */
//--------------------------------------------------------------------------------------------------
#define BS_CONFIG_VERSION_2         2

//--------------------------------------------------------------------------------------------------
/**
 * Supported version for bootstrap file
 */
//--------------------------------------------------------------------------------------------------
#define BS_CONFIG_VERSION           BS_CONFIG_VERSION_2

//--------------------------------------------------------------------------------------------------
/**
 * Velocity type according to the Universal Geographical Area Description
 * Velocity type is defined in the 3GPP 23.032 standard, section 8.6
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    LWM2MCORE_VELOCITY_H                       = 0, ///< Horizontal Velocity
    LWM2MCORE_VELOCITY_H_AND_V                 = 1, ///< Horizontal with Vertical Velocity
    LWM2MCORE_VELOCITY_H_AND_UNCERTAINTY       = 2, ///< Horizontal Velocity with Uncertainty
    LWM2MCORE_VELOCITY_H_AND_V_AND_UNCERTAINTY = 3, ///< Horizontal with Vertical Velocity and
                                                    ///< Uncertainty
}
VelocityType_t;

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
}
ConfigSecurityObjectV01_t;

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
}
ConfigServerObjectV01_t;

//--------------------------------------------------------------------------------------------------
/**
 * Structure for bootstrap configuration to be stored in platform storage
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
 * Structure for bootstrap configuration: list of received bootstrap information
 * This structure needs to be stored in platform storage
 */
//--------------------------------------------------------------------------------------------------
static ConfigBootstrapFile_t BsConfigList;

//--------------------------------------------------------------------------------------------------
/**
 * Function to get the bootstrap information for a specific object instance Id of object 0
 * (security)
 *
 * @return
 *  - pointer on object instance structure on success
 *  - NULL if the object instance Id does not exist
 */
//--------------------------------------------------------------------------------------------------
static ConfigSecurityObject_t* FindSecurityInstance
(
    ConfigBootstrapFile_t   bsConfigList,               ///< [IN] Bootstrap information list
    uint16_t                securityObjectInstanceId    ///< [IN] Security object instance Id
)
{
    ConfigSecurityObject_t* bsInfoPtr = bsConfigList.securityPtr;

    while ((bsInfoPtr) && ((bsInfoPtr->data.securityObjectInstanceId) != securityObjectInstanceId))
    {
        bsInfoPtr = bsInfoPtr->nextPtr;
    }
    return bsInfoPtr;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to get the bootstrap information for a specific object instance Id of object 1
 * (server)
 *
 * @return
 *  - pointer on ConfigServerObject_t
 *  - NULL if the object instance Id does not exist
 */
//--------------------------------------------------------------------------------------------------
static ConfigServerObject_t* FindServerInstance
(
    ConfigBootstrapFile_t   bsConfigList,               ///< [IN] Bootstrap information list
    uint16_t                serverObjectInstanceId      ///< [IN] Server object instance Id
)
{
    ConfigServerObject_t* bsInfoPtr = bsConfigList.serverPtr;

    while (bsInfoPtr)
    {
        bsInfoPtr = bsInfoPtr->nextPtr;
    }

    bsInfoPtr = bsConfigList.serverPtr;


    while ((bsInfoPtr) && ((bsInfoPtr->data.serverObjectInstanceId) != serverObjectInstanceId))
    {
        bsInfoPtr = bsInfoPtr->nextPtr;
    }
    return bsInfoPtr;
}

//--------------------------------------------------------------------------------------------------
/**
 * Build the velocity, formated according to 3GPP 23.032 (Universal Geographical Area
 * Description)
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not available
 *      - LWM2MCORE_ERR_OVERFLOW in case of buffer overflow
 */
//--------------------------------------------------------------------------------------------------
static lwm2mcore_Sid_t BuildVelocity
(
    char*   bufferPtr,  ///< [INOUT] data buffer
    size_t* lenPtr      ///< [INOUT] length of input buffer and length of the returned data
)
{
    lwm2mcore_Sid_t sID, lsID, hsID, vsID;
    uint32_t direction;
    uint32_t hSpeed;
    int32_t vSpeed;
    uint8_t gadVelocity[LWM2MCORE_GAD_VELOCITY_MAX_BYTES];
    uint8_t gadVelocityLen = 0;

    /* Get the direction of movement */
    lsID = lwm2mcore_GetDirection(&direction);
    switch (lsID)
    {
        case LWM2MCORE_ERR_NOT_YET_IMPLEMENTED:
            /* No direction available, assume 0 */
            direction = 0;
            break;

        case LWM2MCORE_ERR_COMPLETED_OK:
            /* Direction successfully retrieved */
            break;

        default:
            /* Direction is necessary to build the velocity */
            return LWM2MCORE_ERR_NOT_YET_IMPLEMENTED;
    }

    /* Get the horizontal speed */
    hsID = lwm2mcore_GetHorizontalSpeed(&hSpeed);
    if (LWM2MCORE_ERR_COMPLETED_OK != hsID)
    {
        /* We need at least the horizontal speed to build the velocity */
        return LWM2MCORE_ERR_NOT_YET_IMPLEMENTED;
    }

    /* Velocity initialization */
    memset(gadVelocity, 0, sizeof(gadVelocity));

    /* Get the vertical speed */
    vsID = lwm2mcore_GetVerticalSpeed(&vSpeed);
    if (LWM2MCORE_ERR_COMPLETED_OK == vsID)
    {
        uint8_t vSpeedDir;

        /* Bits 5 to 8 of byte 1: Velocity type */
        gadVelocity[0] = (uint8_t)(((uint8_t)(LWM2MCORE_VELOCITY_H_AND_V)) << 4);
        gadVelocityLen++;
        /* Bit 2 of byte 1: Direction of vertical speed */
        /* 0 = upward, 1 = downward */
        vSpeedDir = (vSpeed < 0) ? 1 : 0;
        gadVelocity[0] |= (uint8_t)(vSpeedDir << 1);
        /* Last bit of byte 1 and byte 2: Bearing in degrees */
        gadVelocity[0] |= (uint8_t)(((uint16_t)(direction & 0x0100)) >> 8);
        gadVelocity[1] |= (uint8_t)(direction & 0xFF);
        gadVelocityLen++;
        /* Bytes 3 and 4: Horizontal speed in km/h */
        hSpeed = hSpeed * 3.6;
        gadVelocity[2] = (uint8_t)(((uint16_t)(hSpeed & 0xFF00)) >> 8);
        gadVelocityLen++;
        gadVelocity[3] = (uint8_t)(hSpeed & 0xFF);
        gadVelocityLen++;
        /* Byte 5: Vertical speed in km/h */
        vSpeed = abs(vSpeed) * 3.6;
        gadVelocity[4] = (uint8_t)vSpeed;
        gadVelocityLen++;

        sID = LWM2MCORE_ERR_COMPLETED_OK;
    }
    else
    {
        /* Bits 5 to 8 of byte 1: Velocity type */
        gadVelocity[0] = (uint8_t)(((uint8_t)(LWM2MCORE_VELOCITY_H)) << 4);
        gadVelocityLen++;
        /* Last bit of byte 1 and byte 2: Bearing in degrees */
        gadVelocity[0] |= (uint8_t)(((uint16_t)(direction & 0x0100)) >> 8);
        gadVelocity[1] |= (uint8_t)(direction & 0xFF);
        gadVelocityLen++;
        /* Bytes 3 and 4: Horizontal speed in km/h */
        hSpeed = hSpeed * 3.6;
        gadVelocity[2] = (uint8_t)(((uint16_t)(hSpeed & 0xFF00)) >> 8);
        gadVelocityLen++;
        gadVelocity[3] = (uint8_t)(hSpeed & 0xFF);
        gadVelocityLen++;

        sID = LWM2MCORE_ERR_COMPLETED_OK;
    }
    lwm2mcore_DataDump("Velocity buffer", gadVelocity, gadVelocityLen);

    /* Copy the velocity to the output buffer */
    if (*lenPtr < gadVelocityLen)
    {
        sID = LWM2MCORE_ERR_OVERFLOW;
    }
    else
    {
        memcpy(bufferPtr, gadVelocity, gadVelocityLen);
        *lenPtr = gadVelocityLen;
    }

    return sID;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to add an object instance of object 0 (security) in bootstrap information list
 */
//--------------------------------------------------------------------------------------------------
static void AddBootstrapInformationSecurity
(
    ConfigBootstrapFile_t* bsConfigListPtr,         ///< [IN] Bootstrap information list
    ConfigSecurityObject_t* securityInformationPtr  ///< [IN] Security object
)
{
    if (!bsConfigListPtr->securityPtr)
    {
        bsConfigListPtr->securityPtr = securityInformationPtr;
        bsConfigListPtr->securityPtr->nextPtr = NULL;
    }
    else
    {
        ConfigSecurityObject_t* tempPtr = bsConfigListPtr->securityPtr;

        while (tempPtr->nextPtr)
        {
            tempPtr = tempPtr->nextPtr;
        }
        tempPtr->nextPtr = securityInformationPtr;
        securityInformationPtr->nextPtr = NULL;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to add an object instance of object 1 (server) in bootstrap information list
 */
//--------------------------------------------------------------------------------------------------
static void AddBootstrapInformationServer
(
    ConfigBootstrapFile_t*  bsConfigListPtr,          ///< [IN] Bootstrap information list
    ConfigServerObject_t*   serverInformationPtr      ///< [IN] Server object
)
{
    if (!bsConfigListPtr->serverPtr)
    {
        bsConfigListPtr->serverPtr = serverInformationPtr;
        bsConfigListPtr->serverPtr->nextPtr = NULL;
    }
    else
    {
        ConfigServerObject_t* tempPtr = bsConfigListPtr->serverPtr;

        while (tempPtr->nextPtr)
        {
            tempPtr = tempPtr->nextPtr;
        }
        tempPtr->nextPtr = serverInformationPtr;
        serverInformationPtr->nextPtr = NULL;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to save the bootstrap configuration in platform memory
 *
 * @return
 *      - true in case of success
 *      - false in case of failure
 */
//--------------------------------------------------------------------------------------------------
static bool StoreBootstrapConfiguration
(
    ConfigBootstrapFile_t BsConfig              ///< [IN] Bootstrap configuration to store
)
{
    bool result = false;
    uint32_t lenToStore;
    uint32_t lenWritten = 0;
    uint32_t len = sizeof(lenToStore);
    uint16_t loop = 0;
    uint8_t* dataPtr;
    uint8_t* dataLenPtr;

    ConfigSecurityObject_t* securityPtr = BsConfig.securityPtr;
    ConfigServerObject_t* serverPtr = BsConfig.serverPtr;

    lenToStore = sizeof(BsConfig.version) +
                 sizeof(BsConfig.securityObjectNumber) +
                 sizeof(BsConfig.serverObjectNumber) +
                 sizeof(ConfigSecurityToStore_t) * BsConfig.securityObjectNumber +
                 sizeof(ConfigServerToStore_t) * BsConfig.serverObjectNumber;

    dataPtr = (uint8_t*)lwm2m_malloc(lenToStore);
    if (!dataPtr)
    {
        return false;
    }
    memset(dataPtr, 0, lenToStore);

    /* Copy the version */
    memcpy(dataPtr + lenWritten, &(BsConfig.version), sizeof(BsConfig.version));
    lenWritten += sizeof(BsConfig.version);

    /* Copy the number of security objects and server objects */
    memcpy(dataPtr + lenWritten,
           &(BsConfig.securityObjectNumber),
           sizeof(BsConfig.securityObjectNumber));
    lenWritten += sizeof(BsConfig.securityObjectNumber);

    memcpy(dataPtr + lenWritten,
           &(BsConfig.serverObjectNumber),
           sizeof(BsConfig.serverObjectNumber));
    lenWritten += sizeof(BsConfig.serverObjectNumber);

    /* Copy security objects data */
    loop = BsConfig.securityObjectNumber;
    while (loop && securityPtr)
    {
        memcpy(dataPtr + lenWritten, &(securityPtr->data), sizeof(ConfigSecurityToStore_t));
        lenWritten += sizeof(ConfigSecurityToStore_t);
        securityPtr = securityPtr->nextPtr;
        loop--;
    }

    /* Copy server objects data */
    loop = BsConfig.serverObjectNumber;
    while (loop && serverPtr)
    {
        memcpy(dataPtr + lenWritten, &(serverPtr->data), sizeof(ConfigServerToStore_t));
        lenWritten += sizeof(ConfigServerToStore_t);
        serverPtr = serverPtr->nextPtr;
        loop--;
    }
    lwm2mcore_DataDump("BS config data", dataPtr, lenToStore);
    dataLenPtr = (uint8_t*)&lenToStore;

    if ( (LWM2MCORE_ERR_COMPLETED_OK == lwm2mcore_SetParam(LWM2MCORE_BOOTSTRAP_INFO_SIZE_PARAM,
                                                           dataLenPtr,
                                                           len))
      && (LWM2MCORE_ERR_COMPLETED_OK == lwm2mcore_SetParam(LWM2MCORE_BOOTSTRAP_PARAM,
                                                           dataPtr,
                                                           lenToStore)))
    {
        result = true;
    }

    lwm2m_free(dataPtr);
    LOG_ARG("StoreBootstrapConfiguration result %d", result);
    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to free the bootstrap information list
 */
//--------------------------------------------------------------------------------------------------
static void FreeBootstrapInformation
(
    ConfigBootstrapFile_t* configPtr        ///< [INOUT] Configuration to free
)
{
    ConfigSecurityObject_t* securityInformationPtr;
    ConfigServerObject_t* serverInformationPtr;

    /* Free bootstrap information list */
    securityInformationPtr = configPtr->securityPtr;
    while (NULL != securityInformationPtr)
    {
        ConfigSecurityObject_t* nextPtr = securityInformationPtr->nextPtr;
        lwm2m_free(securityInformationPtr);
        securityInformationPtr = nextPtr;
    }
    configPtr->securityPtr = NULL;

    serverInformationPtr = configPtr->serverPtr;
    while (NULL != serverInformationPtr)
    {
        ConfigServerObject_t* nextPtr = serverInformationPtr->nextPtr;
        lwm2m_free(serverInformationPtr);
        serverInformationPtr = nextPtr;
    }
    configPtr->serverPtr = NULL;

    memset(configPtr, 0, sizeof(ConfigBootstrapFile_t));
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to set a default bootstrap information for bootstrap
 */
//--------------------------------------------------------------------------------------------------
static void SetDefaultBootstrapConfiguration
(
    ConfigBootstrapFile_t* configPtr        ///< [INOUT] configuration
)
{
    ConfigSecurityObject_t* securityInformationPtr;

    LOG("Set default BS configuration");

    FreeBootstrapInformation(configPtr);
    memset(configPtr, 0, sizeof(ConfigSecurityObject_t));
    configPtr->version = BS_CONFIG_VERSION;

    /* Allocation security object for bootstrap server */
    securityInformationPtr = (ConfigSecurityObject_t*)lwm2m_malloc(sizeof(ConfigSecurityObject_t));
    LWM2MCORE_ASSERT(securityInformationPtr);
    memset(securityInformationPtr, 0, sizeof(ConfigSecurityObject_t));
    configPtr->securityObjectNumber = 1;
    configPtr->serverObjectNumber = 0;

    /* Object instance of object 0 for bootstrap is 0 */
    securityInformationPtr->data.securityObjectInstanceId = 0;
    securityInformationPtr->data.isBootstrapServer = true;
    /* PSK support only */
    securityInformationPtr->data.securityMode = SEC_PSK;

    /* Default values */
    securityInformationPtr->data.serverId = 1;
    securityInformationPtr->data.clientHoldOffTime = 5;
    securityInformationPtr->data.bootstrapAccountTimeout = 0;

    /* Add the security object on the bootstrap configuration list */
    AddBootstrapInformationSecurity(configPtr, securityInformationPtr);
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to adapt bootstrap configuration file from previous version (v1) to current one (v2)
 *
 * @return
 *      - true in case of success
 *      - false in case of failure
 */
//--------------------------------------------------------------------------------------------------
static bool BootstrapConfigurationAdaptation
(
    void
)
{
    lwm2mcore_Sid_t sid;
    ConfigBootstrapFileV01_t bsConfig;
    size_t len = sizeof(ConfigBootstrapFileV01_t);

    LOG("Adapt bootstrap configuration");

    /* Check if the LwM2MCore configuration file is stored */
    sid = lwm2mcore_GetParam(LWM2MCORE_BOOTSTRAP_PARAM, (uint8_t*)&bsConfig, &len);
    if (LWM2MCORE_ERR_COMPLETED_OK != sid)
    {
        LOG("No bootstrap configuration");
        return false;
    }

    if (BS_CONFIG_VERSION_1 == bsConfig.version)
    {
        LOG("Stored file for BS is version 1");
        /* In BS version 1, only one DM server was supported
         * Check if at least one DM credentials set is stored
         */
        if (lwm2mcore_CheckCredential(LWM2MCORE_CREDENTIAL_DM_ADDRESS, LWM2MCORE_NO_SERVER_ID))
        {
            /* Adapt BS configuration file v1 to v2 */
            ConfigSecurityObject_t* securityInformationPtr;
            ConfigServerObject_t* serverInformationPtr;
            LOG("DM credentials are present");

            BsConfigList.version = BS_CONFIG_VERSION;
            BsConfigList.securityObjectNumber = 2;
            BsConfigList.serverObjectNumber = 1;

            /* Allocation security object for bootstrap server */
            securityInformationPtr = (ConfigSecurityObject_t*)
                                     lwm2m_malloc(sizeof(ConfigSecurityObject_t));
            LWM2MCORE_ASSERT(securityInformationPtr);
            memset(securityInformationPtr, 0, sizeof(ConfigSecurityObject_t));

            securityInformationPtr->data.securityObjectInstanceId = 0;
            securityInformationPtr->data.bootstrapAccountTimeout = bsConfig.security[0].bootstrapAccountTimeout;
            securityInformationPtr->data.clientHoldOffTime = bsConfig.security[0].clientHoldOffTime;
            securityInformationPtr->data.isBootstrapServer = bsConfig.security[0].isBootstrapServer;
            securityInformationPtr->data.securityMode = bsConfig.security[0].securityMode;
            securityInformationPtr->data.serverId = bsConfig.security[0].serverId;

            /* Add the security object on the bootstrap configuration list */
            AddBootstrapInformationSecurity(&BsConfigList, securityInformationPtr);

            /* Allocation security object for DM server */
            securityInformationPtr = (ConfigSecurityObject_t*)
                                     lwm2m_malloc(sizeof(ConfigSecurityObject_t));
            LWM2MCORE_ASSERT(securityInformationPtr);
            memset(securityInformationPtr, 0, sizeof(ConfigSecurityObject_t));

            securityInformationPtr->data.securityObjectInstanceId = 1;
            securityInformationPtr->data.bootstrapAccountTimeout = bsConfig.security[1].bootstrapAccountTimeout;
            securityInformationPtr->data.clientHoldOffTime = bsConfig.security[1].clientHoldOffTime;
            securityInformationPtr->data.isBootstrapServer = bsConfig.security[1].isBootstrapServer;
            securityInformationPtr->data.securityMode = bsConfig.security[1].securityMode;
            securityInformationPtr->data.serverId = bsConfig.security[1].serverId;

            /* Add the security object on the bootstrap configuration list */
            AddBootstrapInformationSecurity(&BsConfigList, securityInformationPtr);

            /* Allocation server object for DM server */
            serverInformationPtr = (ConfigServerObject_t*)
                                   lwm2m_malloc(sizeof(ConfigServerObject_t));
            LWM2MCORE_ASSERT(serverInformationPtr);
            memset(serverInformationPtr, 0, sizeof(ConfigServerObject_t));

            serverInformationPtr->data.serverObjectInstanceId = 0;
            serverInformationPtr->data.serverId = bsConfig.server.serverId;
            serverInformationPtr->data.lifetime = bsConfig.server.lifetime;
            serverInformationPtr->data.defaultPmin = bsConfig.server.defaultPmin;
            serverInformationPtr->data.defaultPmax = bsConfig.server.defaultPmax;
            serverInformationPtr->data.isDisable = bsConfig.server.isDisable;
            serverInformationPtr->data.disableTimeout = bsConfig.server.disableTimeout;
            serverInformationPtr->data.isNotifStored = bsConfig.server.isNotifStored;
            memcpy(serverInformationPtr->data.bindingMode,
                   bsConfig.server.bindingMode,
                   LWM2MCORE_BINDING_STR_MAX_LEN);

            /* Add the security object on the bootstrap configuration list */
            AddBootstrapInformationServer(&BsConfigList, serverInformationPtr);

            return true;
        }
        /* Else consider that no connection was made to bootstrap */
        LOG("DM credentials are NOT present");
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to read the bootstrap configuration from platform memory
 *
 * @return
 *      - true in case of success
 *      - false in case of failure
 */
//--------------------------------------------------------------------------------------------------
static bool GetBootstrapConfiguration
(
    ConfigBootstrapFile_t*  configPtr,      ///< [OUT] Bootstrap configuration
    bool                    storage         ///< [IN] Indicates if the configuration needs to be
                                            ///<      stored
)
{
    lwm2mcore_Sid_t sid;
    uint32_t lenWritten = 0;
    uint32_t loop;
    uint32_t fileSize = 0;
    uint32_t fileReadSize = 0;
    size_t len = sizeof(fileSize);
    uint8_t* dataPtr = (uint8_t*)configPtr;
    ConfigSecurityObject_t*     securityPtr;
    ConfigServerObject_t*       serverPtr;
    uint8_t* rawData;

    /* Free the configuration */
    FreeBootstrapInformation(configPtr);

    /* Get the bootstrap information file size */
    sid = lwm2mcore_GetParam(LWM2MCORE_BOOTSTRAP_INFO_SIZE_PARAM, (uint8_t*)&fileSize, &len);
    LOG_ARG("Get BS configuration size: %d result %d, len %d", fileSize, sid, len);
    if (LWM2MCORE_ERR_COMPLETED_OK != sid)
    {
        if (false == BootstrapConfigurationAdaptation())
        {
            /* Set a default configuration */
            SetDefaultBootstrapConfiguration(configPtr);
        }

        /* Store the configuration */
        if (storage)
        {
            StoreBootstrapConfiguration(*configPtr);
        }
        return false;
    }

    rawData = (uint8_t*)lwm2m_malloc(fileSize);
    LWM2MCORE_ASSERT(rawData);
    fileReadSize = fileSize;
    /* Get the bootstrap information file */
    sid = lwm2mcore_GetParam(LWM2MCORE_BOOTSTRAP_PARAM, rawData, (size_t*)&fileReadSize);
    LOG_ARG("Read BS configuration: fileReadSize %d result %d", fileReadSize, sid);

    if (LWM2MCORE_ERR_COMPLETED_OK != sid)
    {
        if (false == BootstrapConfigurationAdaptation())
        {
            /* Set a default configuration */
            SetDefaultBootstrapConfiguration(configPtr);
        }
        /* Store the configuration */
        if (storage)
        {
            StoreBootstrapConfiguration(*configPtr);
        }
        return false;
    }

    if (fileReadSize != fileSize)
    {
        LOG("Not same BS configuration file size");
        lwm2m_free(rawData);
        lwm2mcore_DeleteParam(LWM2MCORE_BOOTSTRAP_PARAM);
        lwm2mcore_DeleteParam(LWM2MCORE_BOOTSTRAP_INFO_SIZE_PARAM);

        /* Set a default configuration */
        SetDefaultBootstrapConfiguration(configPtr);

        /* Store the configuration */
        if (storage)
        {
            StoreBootstrapConfiguration(*configPtr);
        }
        return false;
    }

    if (LWM2MCORE_ERR_COMPLETED_OK != sid)
    {
        lwm2m_free(rawData);
        /* Set a default configuration */
        SetDefaultBootstrapConfiguration(configPtr);

        /* Store the configuration */
        if (storage)
        {
            StoreBootstrapConfiguration(*configPtr);
        }

        return false;
    }

    /* Copy the version */
    memcpy(dataPtr + lenWritten, rawData + lenWritten, sizeof(configPtr->version));
    lenWritten += sizeof(configPtr->version);

    /* Copy the number of security objects and server objects */
    memcpy(dataPtr + lenWritten, rawData + lenWritten, sizeof(configPtr->securityObjectNumber));
    lenWritten += sizeof(configPtr->securityObjectNumber);
    memcpy(dataPtr + lenWritten, rawData + lenWritten, sizeof(configPtr->serverObjectNumber));
    lenWritten += sizeof(configPtr->serverObjectNumber);

    /* Allocate security objects and copy related data */
    for (loop = 0; loop < configPtr->securityObjectNumber; loop++)
    {
        securityPtr = (ConfigSecurityObject_t*)lwm2m_malloc(sizeof(ConfigSecurityObject_t));
        if (securityPtr)
        {
            memset(securityPtr, 0, sizeof(ConfigSecurityObject_t));
            memcpy(securityPtr, rawData + lenWritten, sizeof(ConfigSecurityToStore_t));
            lenWritten += sizeof(ConfigSecurityToStore_t);

            /* Check if the security object instance Id is already stored */
            if (NULL == FindSecurityInstance(*configPtr, securityPtr->data.securityObjectInstanceId))
            {
                AddBootstrapInformationSecurity(configPtr, securityPtr);
            }
        }
    }

    /* Allocate server objects and copy related data */
    for (loop = 0; loop < configPtr->serverObjectNumber; loop++)
    {
        serverPtr = (ConfigServerObject_t*)lwm2m_malloc(sizeof(ConfigServerObject_t));
        if (serverPtr)
        {
            memset(serverPtr, 0, sizeof(ConfigServerObject_t));
            memcpy(serverPtr, rawData + lenWritten, sizeof(ConfigServerToStore_t));
            lenWritten += sizeof(ConfigServerToStore_t);

            /* Check if the server object instance Id is already stored */
            if (NULL == FindServerInstance(*configPtr, serverPtr->data.serverObjectInstanceId))
            {
                AddBootstrapInformationServer(configPtr, serverPtr);
            }
        }
    }

    lwm2m_free(rawData);

    if (BS_CONFIG_VERSION == configPtr->version)
    {
        return true;
    }

    /* Delete file if necessary and copy the default config */
    LOG_ARG("Failed to read the BS configuration: read result %d, len %d", sid, len);
    if (len)
    {
        /* The file is present but the size is not correct or the version is not correct
         * Delete it
         */
        LOG("Delete bootstrap configuration");
        sid = lwm2mcore_DeleteParam(LWM2MCORE_BOOTSTRAP_PARAM);
        if (LWM2MCORE_ERR_COMPLETED_OK != sid)
        {
            LOG("Error to delete BS configuration parameter");
        }

        sid = lwm2mcore_DeleteParam(LWM2MCORE_BOOTSTRAP_INFO_SIZE_PARAM);
        if (LWM2MCORE_ERR_COMPLETED_OK != sid)
        {
            LOG("Error to delete BS configuration size parameter");
        }
    }

    /* Set a default configuration */
    SetDefaultBootstrapConfiguration(configPtr);

    /* Store the configuration */
    if (storage)
    {
        StoreBootstrapConfiguration(*configPtr);
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to read the bootstrap configuration from platform memory
 *
 * @return
 *      - true in case of success
 *      - false in case of failure
 */
//--------------------------------------------------------------------------------------------------
bool omanager_GetBootstrapConfiguration
(
    void
)
{
    return GetBootstrapConfiguration(&BsConfigList, true);
}

//--------------------------------------------------------------------------------------------------
/**
 * Sets the lifetime in the server configuration and saves it to platform memory
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_INVALID_STATE if no device management server are configured
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t omanager_SetLifetime
(
    uint32_t    lifetime,   ///< [IN] lifetime in seconds
    bool        storage     ///< [IN] Indicates if the configuration needs to be stored
)
{
    ConfigServerObject_t* serverInformationPtr = BsConfigList.serverPtr;
    LOG_ARG("omanager_SetLifetime %d sec", lifetime);

    if (lwm2mcore_CheckLifetimeLimit(lifetime) != true)
    {
        LOG("Lifetime not in good range");
        return LWM2MCORE_ERR_GENERAL_ERROR;
    }

    LOG_ARG("BsConfigList.version %d", BsConfigList.version);

    if (!(BsConfigList.version) || (BS_CONFIG_VERSION != (BsConfigList.version)))
    {
        /* Load configuration */
        GetBootstrapConfiguration(&BsConfigList, false);

        serverInformationPtr = BsConfigList.serverPtr;
        if (!serverInformationPtr)
        {
            /* No DM server configuration */
            LOG("No DM server configuration");
            return LWM2MCORE_ERR_INVALID_STATE;
        }

        /* Set lifetime for all servers */
        while (serverInformationPtr)
        {
            serverInformationPtr->data.lifetime = lifetime;
            serverInformationPtr = serverInformationPtr->nextPtr;
        }

        if (false == storage)
        {
            return LWM2MCORE_ERR_COMPLETED_OK;
        }

        /* Save bootstrap configuration */
        if (StoreBootstrapConfiguration(BsConfigList))
        {
            return LWM2MCORE_ERR_COMPLETED_OK;
        }
    }
    else
    {
        if (!serverInformationPtr)
        {
            /* No DM server configuration */
            LOG("No DM server configuration");
            return LWM2MCORE_ERR_INVALID_STATE;
        }

        /* Set lifetime for all servers */
        while (serverInformationPtr)
        {
            serverInformationPtr->data.lifetime = lifetime;
            serverInformationPtr = serverInformationPtr->nextPtr;
        }

        if (false == storage)
        {
            return LWM2MCORE_ERR_COMPLETED_OK;
        }
        /* Save bootstrap configuration */
        if (StoreBootstrapConfiguration(BsConfigList))
        {
            return LWM2MCORE_ERR_COMPLETED_OK;
        }
    }

    LOG("Failed to store lifetime");
    return LWM2MCORE_ERR_GENERAL_ERROR;
}

//--------------------------------------------------------------------------------------------------
/**
 * Retrieves the lifetime from the server configuration
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_INVALID_STATE if no device management server are configured
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t omanager_GetLifetime
(
    uint32_t* lifetimePtr                 ///< [OUT] lifetime in seconds
)
{
    if (!(BsConfigList.version) || (BS_CONFIG_VERSION != (BsConfigList.version)))
    {
        memset(&BsConfigList, 0, sizeof(BsConfigList));
        GetBootstrapConfiguration(&BsConfigList, true);
        if (!BsConfigList.serverPtr)
        {
            /* No DM server configuration */
            LOG("No DM server configuration");
            return LWM2MCORE_ERR_INVALID_STATE;
        }
        *lifetimePtr = BsConfigList.serverPtr->data.lifetime;
    }
    else
    {
        ConfigServerObject_t* serverInformationPtr = BsConfigList.serverPtr;
        if (!serverInformationPtr)
        {
            /* No DM server configuration */
            LOG("No DM server configuration");
            return LWM2MCORE_ERR_INVALID_STATE;
        }

        /* Set lifetime to default (disabled) */
        *lifetimePtr = serverInformationPtr->data.lifetime;
    }


    LOG_ARG("Lifetime is %d seconds", *lifetimePtr);
    return LWM2MCORE_ERR_COMPLETED_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 *                                  OBJECT 0: SECURITY
 */
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Function to write a resource of object 0
 * Object: 0 - Security
 * Resource: all
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int omanager_WriteSecurityObj
(
    lwm2mcore_Uri_t* uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource
    char* bufferPtr,                    ///< [INOUT] data buffer for information
    size_t len                          ///< [IN] length of input buffer
)
{
    int sID = LWM2MCORE_ERR_GENERAL_ERROR;
    ConfigSecurityObject_t* securityInformationPtr;

    if ((NULL == uriPtr) || (NULL == bufferPtr))
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    if ((uriPtr->op & LWM2MCORE_OP_WRITE) == 0)
    {
        return LWM2MCORE_ERR_OP_NOT_SUPPORTED;
    }

    securityInformationPtr = FindSecurityInstance(BsConfigList, uriPtr->oiid);
    if (!securityInformationPtr)
    {
        /* Create new securityInformationPtr */
        securityInformationPtr =
                            (ConfigSecurityObject_t*)lwm2m_malloc(sizeof(ConfigSecurityObject_t));
        LWM2MCORE_ASSERT(securityInformationPtr);
        memset(securityInformationPtr, 0, sizeof(ConfigSecurityObject_t));
        BsConfigList.securityObjectNumber++;

        /* Set the security object instance Id */
        securityInformationPtr->data.securityObjectInstanceId = uriPtr->oiid;
        AddBootstrapInformationSecurity(&BsConfigList, securityInformationPtr);
    }

    switch (uriPtr->rid)
    {
        /* Resource 0: LWM2M server URI */
        case LWM2MCORE_SECURITY_SERVER_URI_RID:
            if ((LWM2MCORE_SERVER_URI_MAX_LEN < len) || (LWM2MCORE_BUFFER_MAX_LEN < len))
            {
                sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            }
            else
            {
                /* Copy the server address */
                snprintf((char*)securityInformationPtr->serverURI,
                         LWM2MCORE_SERVER_URI_MAX_LEN,
                         "%s",
                         bufferPtr);
                sID = LWM2MCORE_ERR_COMPLETED_OK;
            }
            break;

        /* Resource 1: Bootstrap server (true or false) */
        case LWM2MCORE_SECURITY_BOOTSTRAP_SERVER_RID:
#ifdef CREDENTIALS_DEBUG
            lwm2mcore_DataDump("server addr write", bufferPtr, len);
#endif
            securityInformationPtr->data.isBootstrapServer =
                                            (bool)omanager_BytesToInt((const char*)bufferPtr, len);
            sID = LWM2MCORE_ERR_COMPLETED_OK;
            break;

        /* Resource 2: Security mode */
        case LWM2MCORE_SECURITY_MODE_RID:
            securityInformationPtr->data.securityMode =
                                (SecurityMode_t)omanager_BytesToInt((const char*)bufferPtr, len);
            sID = LWM2MCORE_ERR_COMPLETED_OK;
            break;

        /* Resource 3: Public key or identity */
        case LWM2MCORE_SECURITY_PKID_RID:
            if (DTLS_PSK_MAX_CLIENT_IDENTITY_LEN < len)
            {
                sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            }
            else
            {
#ifdef CREDENTIALS_DEBUG
                lwm2mcore_DataDump("PSK ID write", bufferPtr, len);
#endif
                memset(securityInformationPtr->devicePKID, 0, DTLS_PSK_MAX_CLIENT_IDENTITY_LEN);
                memcpy(securityInformationPtr->devicePKID, bufferPtr, len);
                securityInformationPtr->pskIdLen = (uint16_t)len;
                sID = LWM2MCORE_ERR_COMPLETED_OK;
            }
            break;

        /* Resource 4: Server public key */
        case LWM2MCORE_SECURITY_SERVER_KEY_RID:
            sID = LWM2MCORE_ERR_COMPLETED_OK;
            break;

        /* Resource 5: Secret key */
        case LWM2MCORE_SECURITY_SECRET_KEY_RID:
            if ((DTLS_PSK_MAX_CLIENT_IDENTITY_LEN < len) || (DTLS_PSK_MAX_KEY_LEN < len))
            {
                sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            }
            else
            {
#ifdef CREDENTIALS_DEBUG
                lwm2mcore_DataDump("PSK secret write", bufferPtr, len);
#endif
                memcpy(securityInformationPtr->secretKey, bufferPtr, len);
                securityInformationPtr->pskLen = (uint16_t)len;
                sID = LWM2MCORE_ERR_COMPLETED_OK;
            }
            break;

        /* Resource 6: SMS security mode */
        case LWM2MCORE_SECURITY_SMS_SECURITY_MODE_RID:
            sID = LWM2MCORE_ERR_NOT_YET_IMPLEMENTED;
            break;

        /* Resource 7: SMS binding key parameters */
        case LWM2MCORE_SECURITY_SMS_BINDING_KEY_PAR_RID:
            sID = LWM2MCORE_ERR_NOT_YET_IMPLEMENTED;
            break;

        /* Resource 8: SMS binding secret key(s) */
        case LWM2MCORE_SECURITY_SMS_BINDING_SEC_KEY_RID:
            sID = LWM2MCORE_ERR_NOT_YET_IMPLEMENTED;
            break;

        /* Resource 9: LWM2M server SMS number */
        case LWM2MCORE_SECURITY_SERVER_SMS_NUMBER_RID:
            sID = LWM2MCORE_ERR_NOT_YET_IMPLEMENTED;
            break;

        /* Resource 10: Short server ID */
        case LWM2MCORE_SECURITY_SERVER_ID_RID:
                securityInformationPtr->data.serverId =
                                        (uint16_t)omanager_BytesToInt((const char*)bufferPtr, len);
                sID = LWM2MCORE_ERR_COMPLETED_OK;
            break;

        /* Resource 11: Client hold of time */
        case LWM2MCORE_SECURITY_CLIENT_HOLD_OFF_TIME_RID:
            securityInformationPtr->data.clientHoldOffTime =
                                        (uint16_t)omanager_BytesToInt((const char*)bufferPtr, len);
            sID = LWM2MCORE_ERR_COMPLETED_OK;
            break;

        /* Resource 12: Bootstrap-Server Account Timeout */
        case LWM2MCORE_SECURITY_BS_ACCOUNT_TIMEOUT_RID:
            sID = LWM2MCORE_ERR_COMPLETED_OK;
            break;

        default:
            sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            break;
    }
    return sID;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to read a resource of object 0
 * Object: 0 - Security
 * Resource: All
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int omanager_ReadSecurityObj
(
    lwm2mcore_Uri_t* uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource
    char* bufferPtr,                    ///< [INOUT] data buffer for information
    size_t* lenPtr,                     ///< [INOUT] length of input buffer and length of the
                                        ///< returned data
    valueChangedCallback_t changedCb    ///< [IN] callback for notification
)
{
    int sID = LWM2MCORE_ERR_GENERAL_ERROR;
    ConfigSecurityObject_t* securityInformationPtr;

    (void)changedCb;

    if ((NULL == uriPtr) || (NULL == bufferPtr) || (NULL == lenPtr))
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    if ((uriPtr->op & LWM2MCORE_OP_READ) == 0)
    {
        return LWM2MCORE_ERR_OP_NOT_SUPPORTED;
    }

    securityInformationPtr = FindSecurityInstance(BsConfigList, uriPtr->oiid);
    if (!securityInformationPtr)
    {
        return LWM2MCORE_ERR_INCORRECT_RANGE;
    }

    switch (uriPtr->rid)
    {
        /* Resource 0: LWM2M server URI */
        case LWM2MCORE_SECURITY_SERVER_URI_RID:
            if (securityInformationPtr->data.isBootstrapServer)
            {
                /* Bootstrap server */
                sID = lwm2mcore_GetCredential(LWM2MCORE_CREDENTIAL_BS_ADDRESS,
                                              securityInformationPtr->data.serverId,
                                              bufferPtr,
                                              lenPtr);
            }
            else
            {
                /* Device Management server */
                sID = lwm2mcore_GetCredential(LWM2MCORE_CREDENTIAL_DM_ADDRESS,
                                              securityInformationPtr->data.serverId,
                                              bufferPtr,
                                              lenPtr);
            }
            break;

        /* Resource 1: Bootstrap server (true or false) */
        case LWM2MCORE_SECURITY_BOOTSTRAP_SERVER_RID:
            *lenPtr = omanager_FormatValueToBytes((uint8_t*) bufferPtr,
                                  &(securityInformationPtr->data.isBootstrapServer),
                                  sizeof(securityInformationPtr->data.isBootstrapServer),
                                  false);
            sID = LWM2MCORE_ERR_COMPLETED_OK;
            break;

        /* Resource 2: Security mode */
        case LWM2MCORE_SECURITY_MODE_RID:
            *lenPtr = omanager_FormatValueToBytes((uint8_t*) bufferPtr,
                                        &(securityInformationPtr->data.securityMode),
                                        sizeof(securityInformationPtr->data.securityMode),
                                        false);
            sID = LWM2MCORE_ERR_COMPLETED_OK;
            break;

        /* Resource 3: Public key or identity */
        case LWM2MCORE_SECURITY_PKID_RID:
            if (securityInformationPtr->data.isBootstrapServer)
            {
                /* Bootstrap server */
                sID = lwm2mcore_GetCredential(LWM2MCORE_CREDENTIAL_BS_PUBLIC_KEY,
                                              securityInformationPtr->data.serverId,
                                              bufferPtr,
                                              lenPtr);
            }
            else
            {
                /* Device Management server */
                sID = lwm2mcore_GetCredential(LWM2MCORE_CREDENTIAL_DM_PUBLIC_KEY,
                                              securityInformationPtr->data.serverId,
                                              bufferPtr,
                                              lenPtr);
            }
#ifdef CREDENTIALS_DEBUG
            lwm2mcore_DataDump("PSK ID read", bufferPtr, *lenPtr);
#endif
            break;

        /* Resource 4: Server public key */
        case LWM2MCORE_SECURITY_SERVER_KEY_RID:
            sID = LWM2MCORE_ERR_COMPLETED_OK;
            break;

        /* Resource 5: Secret key */
        case LWM2MCORE_SECURITY_SECRET_KEY_RID:
            if (securityInformationPtr->data.isBootstrapServer)
            {
                /* Bootstrap server */
                sID = lwm2mcore_GetCredential(LWM2MCORE_CREDENTIAL_BS_SECRET_KEY,
                                              securityInformationPtr->data.serverId,
                                              bufferPtr,
                                              lenPtr);
            }
            else
            {
                /* Device Management server */
                sID = lwm2mcore_GetCredential(LWM2MCORE_CREDENTIAL_DM_SECRET_KEY,
                                              securityInformationPtr->data.serverId,
                                              bufferPtr,
                                              lenPtr);
            }
#ifdef CREDENTIALS_DEBUG
            lwm2mcore_DataDump("PSK secret read", bufferPtr, *lenPtr);
#endif
            break;

        /* Resource 6: SMS security mode */
        case LWM2MCORE_SECURITY_SMS_SECURITY_MODE_RID:
            sID = LWM2MCORE_ERR_NOT_YET_IMPLEMENTED;
            break;

        /* Resource 7: SMS binding key parameters */
        case LWM2MCORE_SECURITY_SMS_BINDING_KEY_PAR_RID:
            sID = LWM2MCORE_ERR_NOT_YET_IMPLEMENTED;
            break;

        /* Resource 8: SMS binding secret key(s) */
        case LWM2MCORE_SECURITY_SMS_BINDING_SEC_KEY_RID:
            sID = LWM2MCORE_ERR_NOT_YET_IMPLEMENTED;
            break;

        /* Resource 9: LWM2M server SMS number */
        case LWM2MCORE_SECURITY_SERVER_SMS_NUMBER_RID:
            sID = LWM2MCORE_ERR_NOT_YET_IMPLEMENTED;
            break;

        /* Resource 10: Short server ID */
        case LWM2MCORE_SECURITY_SERVER_ID_RID:
            *lenPtr = omanager_FormatValueToBytes((uint8_t*) bufferPtr,
                                                  &(securityInformationPtr->data.serverId),
                                                  sizeof(securityInformationPtr->data.serverId),
                                                  false);
            sID = LWM2MCORE_ERR_COMPLETED_OK;
            break;

        /* Resource 11: Client hold of time */
        case LWM2MCORE_SECURITY_CLIENT_HOLD_OFF_TIME_RID:
            *lenPtr = omanager_FormatValueToBytes((uint8_t*) bufferPtr,
                                            &(securityInformationPtr->data.clientHoldOffTime),
                                            sizeof(securityInformationPtr->data.clientHoldOffTime),
                                            false);
            sID = LWM2MCORE_ERR_COMPLETED_OK;
            break;

        /* Resource 12: Bootstrap-Server Account Timeout */
        case LWM2MCORE_SECURITY_BS_ACCOUNT_TIMEOUT_RID:
            sID = LWM2MCORE_ERR_NOT_YET_IMPLEMENTED;
            break;

        default:
            sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            break;
    }
    return sID;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to store credentials in non volatile memory
 *
 * @return
 *      - true in case of success
 *      - false in case of failure
 */
//--------------------------------------------------------------------------------------------------
bool omanager_StoreCredentials
(
    void
)
{
    bool result = false;
    int storageResult = LWM2MCORE_ERR_COMPLETED_OK;

    ConfigSecurityObject_t* securityInformationPtr = BsConfigList.securityPtr;

    while (securityInformationPtr)
    {
        if (securityInformationPtr->data.isBootstrapServer)
        {
            LOG_ARG("Bootstrap: PskIdLen %d PskLen %d Addr len %d",
                    securityInformationPtr->pskIdLen,
                    securityInformationPtr->pskLen,
                    strlen((const char *)securityInformationPtr->serverURI));

            if (   (securityInformationPtr->pskIdLen)
                && (securityInformationPtr->pskLen)
                && (strlen((const char *)securityInformationPtr->serverURI))
                )
            {
                storageResult = lwm2mcore_SetCredential(LWM2MCORE_CREDENTIAL_BS_PUBLIC_KEY,
                                                        LWM2MCORE_BS_SERVER_ID,
                                                        (char*)securityInformationPtr->devicePKID,
                                                        securityInformationPtr->pskIdLen);
                LOG_ARG("Store Bootstrap PskId result %d", storageResult);

                storageResult = lwm2mcore_SetCredential(LWM2MCORE_CREDENTIAL_BS_SECRET_KEY,
                                                        LWM2MCORE_BS_SERVER_ID,
                                                        (char*)securityInformationPtr->secretKey,
                                                        securityInformationPtr->pskLen);
                LOG_ARG("Store Bootstrap Psk result %d", storageResult);

                storageResult = lwm2mcore_SetCredential(LWM2MCORE_CREDENTIAL_BS_ADDRESS,
                                                        LWM2MCORE_BS_SERVER_ID,
                                                        (char*)securityInformationPtr->serverURI,
                                                        strlen((const char *)securityInformationPtr->serverURI));
                LOG_ARG("Store Bootstrap Addr result %d", storageResult);
            }
        }
        else
        {
            LOG_ARG("Device management: PskIdLen %d PskLen %d Addr len %d",
                    securityInformationPtr->pskIdLen,
                    securityInformationPtr->pskLen,
                    strlen((const char *)securityInformationPtr->serverURI));

            /* In case of non-secure connection, pskIdLen and pskLen can be 0 */
            if ((securityInformationPtr->pskIdLen) && (LWM2MCORE_ERR_COMPLETED_OK == storageResult))
            {
                storageResult = lwm2mcore_SetCredential(LWM2MCORE_CREDENTIAL_DM_PUBLIC_KEY,
                                                        securityInformationPtr->data.serverId,
                                                        (char*)securityInformationPtr->devicePKID,
                                                        securityInformationPtr->pskIdLen);
                LOG_ARG("Store Device management (%d) PskId result %d",
                        securityInformationPtr->data.serverId, storageResult);
            }

            if ((securityInformationPtr->pskLen) && (LWM2MCORE_ERR_COMPLETED_OK == storageResult))
            {
                storageResult = lwm2mcore_SetCredential(LWM2MCORE_CREDENTIAL_DM_SECRET_KEY,
                                                        securityInformationPtr->data.serverId,
                                                        (char*)securityInformationPtr->secretKey,
                                                        securityInformationPtr->pskLen);
                LOG_ARG("Store Device management (%d) Psk result %d",
                        securityInformationPtr->data.serverId, storageResult);
            }

            if ((strlen((const char *)securityInformationPtr->serverURI))
             && (LWM2MCORE_ERR_COMPLETED_OK == storageResult))
            {
                storageResult = lwm2mcore_SetCredential(LWM2MCORE_CREDENTIAL_DM_ADDRESS,
                                                        securityInformationPtr->data.serverId,
                                                        (char*)securityInformationPtr->serverURI,
                                                        strlen((const char *)
                                                               securityInformationPtr->serverURI));
                LOG_ARG("Store Device management (%d) Addr result %d",
                        securityInformationPtr->data.serverId, storageResult);
            }
        }

        securityInformationPtr = securityInformationPtr->nextPtr;
    }

    if (LWM2MCORE_ERR_COMPLETED_OK == storageResult)
    {
        result = true;
    }
    LOG_ARG("credentials storage: %d", result);

    /* Set the bootstrap configuration */
    StoreBootstrapConfiguration(BsConfigList);
    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function for the server SMS parameters
 * Object: 0 - Security
 * Resources: 6, 7, 8, 9
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int omanager_SmsDummy
(
    lwm2mcore_Uri_t* uriPtr,                ///< [IN] uriPtr represents the requested operation and
                                            ///< object/resource
    char* bufferPtr,                        ///< [INOUT] data buffer for information
    size_t len                             ///< [IN] length of input buffer
)
{
    (void)uriPtr;
    (void)bufferPtr;
    (void)len;

    if ((NULL == uriPtr) || (NULL == bufferPtr))
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    if ((uriPtr->op & (LWM2MCORE_OP_READ | LWM2MCORE_OP_WRITE)) == 0)
    {
        return LWM2MCORE_ERR_OP_NOT_SUPPORTED;
    }

    return LWM2MCORE_ERR_COMPLETED_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 *                                  OBJECT 1: SERVER
 */
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Function to write a resource of object 1
 * Object: 1 - Server
 * Resource: all
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int omanager_WriteServerObj
(
    lwm2mcore_Uri_t* uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource
    char* bufferPtr,                    ///< [INOUT] data buffer for information
    size_t len                          ///< [IN] length of input buffer
)
{
    int sID = LWM2MCORE_ERR_GENERAL_ERROR;
    uint32_t lifetime;
    ConfigServerObject_t* serverInformationPtr;

    if ((NULL == uriPtr) || (NULL == bufferPtr))
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    if ((uriPtr->op & LWM2MCORE_OP_WRITE) == 0)
    {
        return LWM2MCORE_ERR_OP_NOT_SUPPORTED;
    }

    serverInformationPtr = FindServerInstance(BsConfigList, uriPtr->oiid);
    if (!serverInformationPtr)
    {
        /* Create new serverInformationPtr */
        serverInformationPtr = (ConfigServerObject_t*)lwm2m_malloc(sizeof(ConfigServerObject_t));
        LWM2MCORE_ASSERT(serverInformationPtr);
        memset(serverInformationPtr, 0, sizeof(ConfigServerObject_t));
        BsConfigList.serverObjectNumber++;

        serverInformationPtr->data.serverObjectInstanceId = uriPtr->oiid;
        AddBootstrapInformationServer(&BsConfigList, serverInformationPtr);
    }

    switch (uriPtr->rid)
    {
        /* Resource 0: Server short ID */
        case LWM2MCORE_SERVER_SHORT_ID_RID:
            serverInformationPtr->data.serverId =
                                (uint16_t)omanager_BytesToInt((const char*)bufferPtr, len);
            sID = LWM2MCORE_ERR_COMPLETED_OK;
            break;

        /* Resource 1: Server lifetime */
        case LWM2MCORE_SERVER_LIFETIME_RID:
            lifetime = (uint32_t)omanager_BytesToInt((const char*)bufferPtr, len);
            LOG_ARG("set lifetime %d", lifetime);
            if (!smanager_IsBootstrapConnection())
            {
                sID = omanager_SetLifetime(lifetime, false);
                if (LWM2MCORE_ERR_COMPLETED_OK == sID)
                {
                    serverInformationPtr->data.lifetime = lifetime;
                }
            }
            else
            {
                /* In case of bootstrap, the configuration will be stored at the bootstrap end */
                serverInformationPtr->data.lifetime = lifetime;
                sID = LWM2MCORE_ERR_COMPLETED_OK;
            }
            break;

        /* Resource 2: Server default minimum period */
        case LWM2MCORE_SERVER_DEFAULT_MIN_PERIOD_RID:
            serverInformationPtr->data.defaultPmin =
                                (uint16_t)omanager_BytesToInt((const char*)bufferPtr, len);
            sID = LWM2MCORE_ERR_COMPLETED_OK;
            break;

        /* Resource 3: Server default minimum period */
        case LWM2MCORE_SERVER_DEFAULT_MAX_PERIOD_RID:
            serverInformationPtr->data.defaultPmax =
                                    (uint16_t)omanager_BytesToInt((const char*)bufferPtr, len);
            sID = LWM2MCORE_ERR_COMPLETED_OK;
            break;

        /* Resource 5: Disable timeout */
        case LWM2MCORE_SERVER_DISABLE_TIMEOUT_RID:
            serverInformationPtr->data.disableTimeout =
                                    (uint32_t)omanager_BytesToInt((const char*)bufferPtr, len);
            sID = LWM2MCORE_ERR_COMPLETED_OK;
            break;

        /* Resource 6: Notification storing when disabled or offline */
        case LWM2MCORE_SERVER_STORE_NOTIF_WHEN_OFFLINE_RID:
            serverInformationPtr->data.isNotifStored =
                                    (bool)omanager_BytesToInt((const char*)bufferPtr, len);
            sID = LWM2MCORE_ERR_COMPLETED_OK;
            break;

        /* Resource 7: Binding */
        case LWM2MCORE_SERVER_BINDING_MODE_RID:
            memset(serverInformationPtr->data.bindingMode, 0, LWM2MCORE_BINDING_STR_MAX_LEN);
            memcpy(serverInformationPtr->data.bindingMode, (uint8_t*)bufferPtr, len);
            sID = LWM2MCORE_ERR_COMPLETED_OK;
            break;

        default:
            sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            break;
    }

    /* Write server object in platform storage only in case of device management
     * For bootstrap, the configuration is stored at the end of bootstap
     */
    if (false == smanager_IsBootstrapConnection())
    {
        StoreBootstrapConfiguration(BsConfigList);
    }

    return sID;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to read a resource of object 1
 * Object: 1 - Server
 * Resource: All
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int omanager_ReadServerObj
(
    lwm2mcore_Uri_t* uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource
    char* bufferPtr,                    ///< [INOUT] data buffer for information
    size_t* lenPtr,                     ///< [INOUT] length of input buffer and length of the
                                        ///< returned data
    valueChangedCallback_t changedCb    ///< [IN] callback for notification
)
{
    int sID = LWM2MCORE_ERR_GENERAL_ERROR;
    ConfigServerObject_t* serverInformationPtr;

    (void)changedCb;

    if ((NULL == uriPtr) || (NULL == bufferPtr) || (NULL == lenPtr))
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    if ((uriPtr->op & LWM2MCORE_OP_READ) == 0)
    {
        return LWM2MCORE_ERR_OP_NOT_SUPPORTED;
    }

    serverInformationPtr = FindServerInstance(BsConfigList, uriPtr->oiid);
    if (!serverInformationPtr)
    {
        LOG("serverInformationPtr NULL");
        return LWM2MCORE_ERR_GENERAL_ERROR;
    }

    switch (uriPtr->rid)
    {
        /* Resource 0: Server short ID */
        case LWM2MCORE_SERVER_SHORT_ID_RID:
            *lenPtr = omanager_FormatValueToBytes((uint8_t*) bufferPtr,
                                         &(serverInformationPtr->data.serverId),
                                         sizeof(serverInformationPtr->data.serverId),
                                         false);
            sID = LWM2MCORE_ERR_COMPLETED_OK;
            break;

        /* Resource 1: Server lifetime */
        case LWM2MCORE_SERVER_LIFETIME_RID:
        {
            *lenPtr = omanager_FormatValueToBytes((uint8_t*) bufferPtr,
                                         &(serverInformationPtr->data.lifetime),
                                         sizeof(serverInformationPtr->data.lifetime),
                                         false);
            serverInformationPtr->data.lifetime = (serverInformationPtr->data.lifetime == 0)
                                       ? LWM2MCORE_LIFETIME_VALUE_DISABLED
                                       : serverInformationPtr->data.lifetime;
            LOG_ARG("lifetime write %d", serverInformationPtr->data.lifetime);
            sID = LWM2MCORE_ERR_COMPLETED_OK;
            break;
        }

        /* Resource 2: Server default minimum period */
        case LWM2MCORE_SERVER_DEFAULT_MIN_PERIOD_RID:
            *lenPtr = omanager_FormatValueToBytes((uint8_t*) bufferPtr,
                                         &(serverInformationPtr->data.defaultPmin),
                                         sizeof(serverInformationPtr->data.defaultPmin),
                                         false);
            sID = LWM2MCORE_ERR_COMPLETED_OK;
            break;

        /* Resource 3: Server default minimum period */
        case LWM2MCORE_SERVER_DEFAULT_MAX_PERIOD_RID:
            *lenPtr = omanager_FormatValueToBytes((uint8_t*) bufferPtr,
                                         &(serverInformationPtr->data.defaultPmax),
                                         sizeof(serverInformationPtr->data.defaultPmax),
                                         false);
            sID = LWM2MCORE_ERR_COMPLETED_OK;
            break;

        /* Resource 5: Disable timeout */
        case LWM2MCORE_SERVER_DISABLE_TIMEOUT_RID:
            *lenPtr = omanager_FormatValueToBytes((uint8_t*) bufferPtr,
                                         &(serverInformationPtr->data.disableTimeout),
                                         sizeof(serverInformationPtr->data.disableTimeout),
                                         false);
            sID = LWM2MCORE_ERR_COMPLETED_OK;
            break;

        /* Resource 6: Notification storing when disabled or offline */
        case LWM2MCORE_SERVER_STORE_NOTIF_WHEN_OFFLINE_RID:
            *lenPtr = omanager_FormatValueToBytes((uint8_t*) bufferPtr,
                                         &(serverInformationPtr->data.isNotifStored),
                                         sizeof(serverInformationPtr->data.isNotifStored),
                                         false);
            sID = LWM2MCORE_ERR_COMPLETED_OK;
            break;

        /* Resource 7: Binding */
        case LWM2MCORE_SERVER_BINDING_MODE_RID:
            *lenPtr = snprintf(bufferPtr, *lenPtr, "%s", serverInformationPtr->data.bindingMode);
            sID = LWM2MCORE_ERR_COMPLETED_OK;
            break;

        default:
            sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            break;
    }
    return sID;
}

//--------------------------------------------------------------------------------------------------
/**
 *                                  OBJECT 3: DEVICE
 */
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Function to write a resource of object 3
 * Object: 3 - Device
 * Resource: All with write operation
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
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int omanager_WriteDeviceObj
(
    lwm2mcore_Uri_t* uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource
    char* bufferPtr,                    ///< [INOUT] data buffer for information
    size_t len                          ///< [IN] length of input buffer
)
{
    int sID = LWM2MCORE_ERR_GENERAL_ERROR;

    (void)len;

    if ((!uriPtr) || (!bufferPtr))
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    /* Check that the object instance Id is in the correct range (only one object instance) */
    if (0 < uriPtr->oiid)
    {
        return LWM2MCORE_ERR_INCORRECT_RANGE;
    }

    /* Check that the operation is coherent */
    if (0 ==(uriPtr->op & LWM2MCORE_OP_WRITE))
    {
        return LWM2MCORE_ERR_OP_NOT_SUPPORTED;
    }

    switch (uriPtr->rid)
    {
        /* Resource 13: Current time */
        case LWM2MCORE_DEVICE_CURRENT_TIME_RID:
            sID = LWM2MCORE_ERR_NOT_YET_IMPLEMENTED;
            break;


        /* Resource 16: Supported binding mode */
        case LWM2MCORE_DEVICE_SUPPORTED_BINDING_MODE_RID:
            sID = LWM2MCORE_ERR_NOT_YET_IMPLEMENTED;
            break;

        default:
            sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            break;
    }

    return sID;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to read a resource of object 3
 * Object: 3 - Device
 * Resource: All with read operation
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
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int omanager_ReadDeviceObj
(
    lwm2mcore_Uri_t* uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource
    char* bufferPtr,                    ///< [INOUT] data buffer for information
    size_t* lenPtr,                     ///< [INOUT] length of input buffer and length of the
                                        ///< returned data
    valueChangedCallback_t changedCb    ///< [IN] callback for notification
)
{
    int sID = LWM2MCORE_ERR_GENERAL_ERROR;

    (void)changedCb;

    if ((!uriPtr) || (!bufferPtr) || (!lenPtr))
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    /* Check that the object instance Id is in the correct range (only one object instance) */
    if (0 < uriPtr->oiid)
    {
        return LWM2MCORE_ERR_INCORRECT_RANGE;
    }

    /* Check that the operation is coherent */
    if (0 == (uriPtr->op & LWM2MCORE_OP_READ))
    {
        return LWM2MCORE_ERR_OP_NOT_SUPPORTED;
    }

    switch (uriPtr->rid)
    {
        /* Resource 0: Manufacturer */
        case LWM2MCORE_DEVICE_MANUFACTURER_RID:
            sID = lwm2mcore_GetDeviceManufacturer(bufferPtr, lenPtr);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = strlen(bufferPtr);
            }
            break;

        /* Resource 1: Device number */
        case LWM2MCORE_DEVICE_MODEL_NUMBER_RID:
            sID = lwm2mcore_GetDeviceModelNumber(bufferPtr, lenPtr);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = strlen(bufferPtr);
            }
            break;

        /* Resource 2: Serial number */
        case LWM2MCORE_DEVICE_SERIAL_NUMBER_RID:
            sID = lwm2mcore_GetDeviceSerialNumber(bufferPtr, lenPtr);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = strlen(bufferPtr);
            }
            break;

        /* Resource 3: Firmware */
        case LWM2MCORE_DEVICE_FIRMWARE_VERSION_RID:
            sID = lwm2mcore_GetDeviceFirmwareVersion(bufferPtr, lenPtr);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = strlen(bufferPtr);
            }
            break;

        /* Resource 9: Battery level */
        case LWM2MCORE_DEVICE_BATTERY_LEVEL_RID:
        {
            uint8_t batteryLevel = 0;
            sID = lwm2mcore_GetBatteryLevel(&batteryLevel);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = omanager_FormatValueToBytes((uint8_t*)bufferPtr,
                                             &batteryLevel,
                                             sizeof(batteryLevel),
                                             false);
            }
        }
        break;

        /* Resource 13: Current time */
        case LWM2MCORE_DEVICE_CURRENT_TIME_RID:
        {
            uint64_t currentTime = 0;
            sID = lwm2mcore_GetDeviceCurrentTime(&currentTime);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = omanager_FormatValueToBytes((uint8_t*)bufferPtr,
                                             &currentTime,
                                             sizeof(currentTime),
                                             false);
            }
        }
        break;

        /* Resource 16: Supported binding mode */
        case LWM2MCORE_DEVICE_SUPPORTED_BINDING_MODE_RID:
            *lenPtr = snprintf(bufferPtr, *lenPtr, LWM2MCORE_BINDING_UDP_QUEUE);
            sID = LWM2MCORE_ERR_COMPLETED_OK;
            break;

        default:
            sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            break;
    }

    return sID;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to execute a resource of object 3
 * Object: 3 - Device
 * Resource: All with execute operation
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int omanager_ExecDeviceObj
(
    lwm2mcore_Uri_t* uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource
    char* bufferPtr,                    ///< [INOUT] data buffer for information
    size_t len                          ///< [IN] length of input buffer
)
{
    int sID = LWM2MCORE_ERR_GENERAL_ERROR;

    if ((NULL == uriPtr) || (len && (NULL == bufferPtr)))
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    /* Check that the object instance Id is in the correct range (only one object instance) */
    if (0 != uriPtr->oiid)
    {
        return LWM2MCORE_ERR_INCORRECT_RANGE;
    }

    /* Check that the operation is coherent */
    if (0 == (uriPtr->op & LWM2MCORE_OP_EXECUTE))
    {
        return LWM2MCORE_ERR_OP_NOT_SUPPORTED;
    }

    switch (uriPtr->rid)
    {
        /* Resource 4: Reboot */
        case LWM2MCORE_DEVICE_REBOOT_RID:
            sID = lwm2mcore_RebootDevice();
            break;

        default:
            sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            break;
    }

    return sID;
}

//--------------------------------------------------------------------------------------------------
/**
 *                              OBJECT 4: CONNECTIVITY MONITORING
 */
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Function to read a resource of object 4
 * Object: 4 - Connectivity monitoring
 * Resource: All
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
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int omanager_ReadConnectivityMonitoringObj
(
    lwm2mcore_Uri_t* uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource
    char* bufferPtr,                    ///< [INOUT] data buffer for information
    size_t* lenPtr,                     ///< [INOUT] length of input buffer and length of the
                                        ///< returned data
    valueChangedCallback_t changedCb    ///< [IN] callback for notification
)
{
    int sID = LWM2MCORE_ERR_GENERAL_ERROR;

    (void)changedCb;

    if ((!uriPtr) || (!bufferPtr) || (!lenPtr))
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    /* Check that the object instance Id is in the correct range (only one object instance) */
    if (0 < uriPtr->oiid)
    {
        return LWM2MCORE_ERR_INCORRECT_RANGE;
    }

    /* Check that the operation is coherent */
    if (0 == (uriPtr->op & LWM2MCORE_OP_READ))
    {
        return LWM2MCORE_ERR_OP_NOT_SUPPORTED;
    }

    switch (uriPtr->rid)
    {
        /* Resource 0: Network bearer */
        case LWM2MCORE_CONN_MONITOR_NETWORK_BEARER_RID:
        {
            lwm2mcore_networkBearer_enum_t networkBearer;
            sID = lwm2mcore_GetNetworkBearer(&networkBearer);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = omanager_FormatValueToBytes((uint8_t*)bufferPtr,
                                                      &networkBearer,
                                                      sizeof(networkBearer),
                                                      false);
            }
        }
        break;

        /* Resource 1: Available network bearer */
        case LWM2MCORE_CONN_MONITOR_AVAIL_NETWORK_BEARER_RID:
            /* Check that the resource instance Id is in the correct range */
            if (uriPtr->riid < CONN_MONITOR_AVAIL_NETWORK_BEARER_MAX_NB)
            {
                static lwm2mcore_networkBearer_enum_t
                       bearersList[CONN_MONITOR_AVAIL_NETWORK_BEARER_MAX_NB];
                static uint16_t bearersNb = 0;

                if (0 == uriPtr->riid)
                {
                    /* Reset the available network bearers list */
                    memset(bearersList, 0, sizeof(bearersList));
                    bearersNb = 0;

                    /* Retrieve the list */
                    sID = lwm2mcore_GetAvailableNetworkBearers(bearersList, &bearersNb);
                }
                else
                {
                    sID = LWM2MCORE_ERR_COMPLETED_OK;
                }

                if (LWM2MCORE_ERR_COMPLETED_OK == sID)
                {
                    if (uriPtr->riid < bearersNb)
                    {
                        *lenPtr = omanager_FormatValueToBytes((uint8_t*)bufferPtr,
                                                              &bearersList[uriPtr->riid],
                                                              sizeof(bearersList[uriPtr->riid]),
                                                              false);
                    }
                    else
                    {
                        *lenPtr = 0;
                    }
                }
            }
            else
            {
                sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            }
            break;

        /* Resource 2: Radio signal strength */
        case LWM2MCORE_CONN_MONITOR_RADIO_SIGNAL_STRENGTH_RID:
        {
            int32_t signalStrength;
            sID = lwm2mcore_GetSignalStrength(&signalStrength);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = omanager_FormatValueToBytes((uint8_t*)bufferPtr,
                                                      &signalStrength,
                                                      sizeof(signalStrength),
                                                      true);
            }
        }
        break;

        /* Resource 3: Link quality */
        case LWM2MCORE_CONN_MONITOR_LINK_QUALITY_RID:
        {
            int linkQuality;
            sID = lwm2mcore_GetLinkQuality(&linkQuality);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = omanager_FormatValueToBytes((uint8_t*)bufferPtr,
                                                      &linkQuality,
                                                      sizeof(linkQuality),
                                                      true);
            }
        }
        break;

        /* Resource 4: IP addresses */
        case LWM2MCORE_CONN_MONITOR_IP_ADDRESSES_RID:
            /* Check that the resource instance Id is in the correct range */
            if (uriPtr->riid < CONN_MONITOR_IP_ADDRESSES_MAX_NB)
            {
                static char ipAddrList[CONN_MONITOR_IP_ADDRESSES_MAX_NB]
                                      [CONN_MONITOR_IP_ADDR_MAX_BYTES];
                static uint16_t ipAddrNb = 0;

                if (0 == uriPtr->riid)
                {
                    /* Reset the IP addresses list */
                    memset(ipAddrList, 0, sizeof(ipAddrList));
                    ipAddrNb = 0;

                    /* Retrieve the list */
                    sID = lwm2mcore_GetIpAddresses(ipAddrList, &ipAddrNb);
                }
                else
                {
                    sID = LWM2MCORE_ERR_COMPLETED_OK;
                }

                if (LWM2MCORE_ERR_COMPLETED_OK == sID)
                {
                    if (uriPtr->riid < ipAddrNb)
                    {
                        *lenPtr = snprintf(bufferPtr,
                                           CONN_MONITOR_IP_ADDR_MAX_BYTES,
                                           "%s",
                                           ipAddrList[uriPtr->riid]);
                    }
                    else
                    {
                        *lenPtr = 0;
                    }
                }
            }
            else
            {
                sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            }
            break;

        /* Resource 5: Router IP addresses */
        case LWM2MCORE_CONN_MONITOR_ROUTER_IP_ADDRESSES_RID:
            /* Check that the resource instance Id is in the correct range */
            if (uriPtr->riid < CONN_MONITOR_ROUTER_IP_ADDRESSES_MAX_NB)
            {
                static char ipAddrList[CONN_MONITOR_ROUTER_IP_ADDRESSES_MAX_NB]
                                      [CONN_MONITOR_IP_ADDR_MAX_BYTES];
                static uint16_t ipAddrNb = 0;

                if (0 == uriPtr->riid)
                {
                    /* Reset the router IP addresses list */
                    memset(ipAddrList, 0, sizeof(ipAddrList));
                    ipAddrNb = 0;

                    /* Retrieve the list */
                    sID = lwm2mcore_GetRouterIpAddresses(ipAddrList, &ipAddrNb);
                }
                else
                {
                    sID = LWM2MCORE_ERR_COMPLETED_OK;
                }

                if (LWM2MCORE_ERR_COMPLETED_OK == sID)
                {
                    if (uriPtr->riid < ipAddrNb)
                    {
                        *lenPtr = snprintf(bufferPtr,
                                           CONN_MONITOR_IP_ADDR_MAX_BYTES,
                                           "%s",
                                           ipAddrList[uriPtr->riid]);
                    }
                    else
                    {
                        *lenPtr = 0;
                    }
                }
            }
            else
            {
                sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            }
            break;

        /* Resource 6: Link utilization */
        case LWM2MCORE_CONN_MONITOR_LINK_UTILIZATION_RID:
        {
            uint8_t linkUtilization;
            sID = lwm2mcore_GetLinkUtilization(&linkUtilization);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = omanager_FormatValueToBytes((uint8_t*)bufferPtr,
                                                      &linkUtilization,
                                                      sizeof(linkUtilization),
                                                      false);
            }
        }
        break;

        /* Resource 7: Access Point Name */
        case LWM2MCORE_CONN_MONITOR_APN_RID:
            /* Check that the resource instance Id is in the correct range */
            if (uriPtr->riid < CONN_MONITOR_APN_MAX_NB)
            {
                static char apnList[CONN_MONITOR_APN_MAX_NB][CONN_MONITOR_APN_MAX_BYTES];
                static uint16_t apnNb = 0;

                if (0 == uriPtr->riid)
                {
                    /* Reset the APN list */
                    memset(apnList, 0, sizeof(apnList));
                    apnNb = 0;

                    /* Retrieve the list */
                    sID = lwm2mcore_GetAccessPointNames(apnList, &apnNb);
                }
                else
                {
                    sID = LWM2MCORE_ERR_COMPLETED_OK;
                }

                if (LWM2MCORE_ERR_COMPLETED_OK == sID)
                {
                    if (uriPtr->riid < apnNb)
                    {
                        *lenPtr = snprintf(bufferPtr,
                                           CONN_MONITOR_APN_MAX_BYTES,
                                           "%s",
                                           apnList[uriPtr->riid]);
                    }
                    else
                    {
                        *lenPtr = 0;
                    }
                }
            }
            else
            {
                sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            }
            break;

        /* Resource 8: Cell ID */
        case LWM2MCORE_CONN_MONITOR_CELL_ID_RID:
        {
            uint32_t cellId;
            sID = lwm2mcore_GetCellId(&cellId);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = omanager_FormatValueToBytes((uint8_t*)bufferPtr,
                                                      &cellId,
                                                      sizeof(cellId),
                                                      false);
            }
        }
        break;

        /* Resource 9: Serving Mobile Network Code */
        case LWM2MCORE_CONN_MONITOR_SMNC_RID:
        {
            uint16_t mnc;
            sID = lwm2mcore_GetMncMcc(&mnc, NULL);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = omanager_FormatValueToBytes((uint8_t*)bufferPtr,
                                                      &mnc,
                                                      sizeof(mnc),
                                                      false);
            }
        }
        break;

        /* Resource 10: Serving Mobile Country Code */
        case LWM2MCORE_CONN_MONITOR_SMCC_RID:
        {
            uint16_t mcc;
            sID = lwm2mcore_GetMncMcc(NULL, &mcc);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = omanager_FormatValueToBytes((uint8_t*)bufferPtr,
                                                      &mcc,
                                                      sizeof(mcc),
                                                      false);
            }
        }
        break;

        default:
            sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            break;
    }

    return sID;
}

//--------------------------------------------------------------------------------------------------
/**
 *                                  OBJECT 5: FIRMWARE UPDATE
 */
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Function to write a resource of object 5
 * Object: 5 - Firmware update
 * Resource: all with write operation
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int omanager_WriteFwUpdateObj
(
    lwm2mcore_Uri_t* uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource
    char* bufferPtr,                    ///< [INOUT] data buffer for information
    size_t len                          ///< [IN] length of input buffer
)
{
    int sID = LWM2MCORE_ERR_GENERAL_ERROR;
    if ((NULL == uriPtr) || (NULL == bufferPtr))
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    /* Only one object instance */
    if (0 < uriPtr->oiid)
    {
        return LWM2MCORE_ERR_INCORRECT_RANGE;
    }

    if (0 == (uriPtr->op & LWM2MCORE_OP_WRITE))
    {
        return LWM2MCORE_ERR_OP_NOT_SUPPORTED;
    }

    switch (uriPtr->rid)
    {
        /* Resource 1: Package URI */
        case LWM2MCORE_FW_UPDATE_PACKAGE_URI_RID:
            if (LWM2MCORE_BUFFER_MAX_LEN < len)
            {
                sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            }
            else
            {
                sID = lwm2mcore_SetUpdatePackageUri(LWM2MCORE_FW_UPDATE_TYPE,
                                                    uriPtr->oid,
                                                    bufferPtr,
                                                    len);
            }
            break;

        default:
            sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            break;
    }
    return sID;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to read a resource of object 5
 * Object: 5 - Firmware update
 * Resource: all with read operation
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int omanager_ReadFwUpdateObj
(
    lwm2mcore_Uri_t* uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource
    char* bufferPtr,                    ///< [INOUT] data buffer for information
    size_t* lenPtr,                     ///< [INOUT] length of input buffer and length of the
                                        ///< returned data
    valueChangedCallback_t changedCb    ///< [IN] not used for READ operation but for WRITE one
)
{
    int sID = LWM2MCORE_ERR_GENERAL_ERROR;

    (void)changedCb;

    if ((NULL == uriPtr) || (NULL == bufferPtr) || (NULL == lenPtr))
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    /* Only one object instance */
    if (0 < uriPtr->oiid)
    {
        return LWM2MCORE_ERR_INCORRECT_RANGE;
    }

    if (0 == (uriPtr->op & LWM2MCORE_OP_READ))
    {
        return LWM2MCORE_ERR_OP_NOT_SUPPORTED;
    }

    switch (uriPtr->rid)
    {
        /* Resource 1: Package URI */
        case LWM2MCORE_FW_UPDATE_PACKAGE_URI_RID:
            sID = lwm2mcore_GetUpdatePackageUri(LWM2MCORE_FW_UPDATE_TYPE,
                                                uriPtr->oid,
                                                bufferPtr,
                                                lenPtr);
            break;

        /* Resource 3: Update state */
        case LWM2MCORE_FW_UPDATE_UPDATE_STATE_RID:
        {
            uint8_t updateState;
            sID = lwm2mcore_GetUpdateState(LWM2MCORE_FW_UPDATE_TYPE, uriPtr->oiid, &updateState);
            if (sID == LWM2MCORE_ERR_COMPLETED_OK)
            {
                *lenPtr = omanager_FormatValueToBytes((uint8_t*) bufferPtr,
                                             &updateState,
                                             sizeof(updateState),
                                             false);
            }
        }
        break;

        /* Resource 5: Update result */
        case LWM2MCORE_FW_UPDATE_UPDATE_RESULT_RID:
        {
            uint8_t updateResult;
            sID = lwm2mcore_GetUpdateResult(LWM2MCORE_FW_UPDATE_TYPE, uriPtr->oiid, &updateResult);
            if (sID == LWM2MCORE_ERR_COMPLETED_OK)
            {
                *lenPtr = omanager_FormatValueToBytes((uint8_t*) bufferPtr,
                                            &updateResult,
                                            sizeof(updateResult),
                                            false);
            }
        }
        break;

        /* Resource 6: Package name */
        case LWM2MCORE_FW_UPDATE_PACKAGE_NAME_RID:
            sID = LWM2MCORE_ERR_OP_NOT_SUPPORTED;
            break;

        /* Resource 7: Package version */
        case LWM2MCORE_FW_UPDATE_PACKAGE_VERSION_RID:
            sID = LWM2MCORE_ERR_OP_NOT_SUPPORTED;
            break;

        default:
            sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            break;
    }
    return sID;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to execute a resource of object 5
 * Object: 5 - Firmware update
 * Resource: all with execute operation
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int omanager_ExecFwUpdate
(
    lwm2mcore_Uri_t* uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource
    char* bufferPtr,                    ///< [INOUT] data buffer for information
    size_t len                          ///< [IN] length of input buffer
)
{
    int sID = LWM2MCORE_ERR_GENERAL_ERROR;

    /* BufferPtr can be null as per spec (OMA-TS-LightweightM2M-V1_0-20151214-C.pdf, E.6) */
    if (NULL == uriPtr)
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    if (0 < uriPtr->oiid)
    {
        /* Only one object instance */
        return LWM2MCORE_ERR_INCORRECT_RANGE;
    }

    /* Check if the related command is EXECUTE */
    if (0 == (uriPtr->op & LWM2MCORE_OP_EXECUTE))
    {
        return LWM2MCORE_ERR_OP_NOT_SUPPORTED;
    }

    switch (uriPtr->rid)
    {
        /* Resource 2: Update */
        case LWM2MCORE_FW_UPDATE_UPDATE_RID:
            sID = lwm2mcore_LaunchUpdate(LWM2MCORE_FW_UPDATE_TYPE, uriPtr->oiid, bufferPtr, len);
            break;

        default:
            sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            break;
    }

    return sID;
}

//--------------------------------------------------------------------------------------------------
/**
 *                                  OBJECT 6: LOCATION
 */
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Function to read a resource of object 6
 * Object: 6 - Location
 * Resource: All
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
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int omanager_ReadLocationObj
(
    lwm2mcore_Uri_t* uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource
    char* bufferPtr,                    ///< [INOUT] data buffer for information
    size_t* lenPtr,                     ///< [INOUT] length of input buffer and length of the
                                        ///< returned data
    valueChangedCallback_t changedCb    ///< [IN] callback for notification
)
{
    int sID = LWM2MCORE_ERR_GENERAL_ERROR;

    (void)changedCb;

    if ((!uriPtr) || (!bufferPtr) || (!lenPtr))
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    /* Check that the object instance Id is in the correct range (only one object instance) */
    if (0 < uriPtr->oiid)
    {
        return LWM2MCORE_ERR_INCORRECT_RANGE;
    }

    /* Check that the operation is coherent */
    if (0 == (uriPtr->op & LWM2MCORE_OP_READ))
    {
        return LWM2MCORE_ERR_OP_NOT_SUPPORTED;
    }

    switch (uriPtr->rid)
    {
        /* Resource 0: Latitude */
        case LWM2MCORE_LOCATION_LATITUDE_RID:
            sID = lwm2mcore_GetLatitude(bufferPtr, lenPtr);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = strlen(bufferPtr);
            }
            break;

        /* Resource 1: Longitude */
        case LWM2MCORE_LOCATION_LONGITUDE_RID:
            sID = lwm2mcore_GetLongitude(bufferPtr, lenPtr);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = strlen(bufferPtr);
            }
            break;

        /* Resource 2: Altitude */
        case LWM2MCORE_LOCATION_ALTITUDE_RID:
            sID = lwm2mcore_GetAltitude(bufferPtr, lenPtr);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = strlen(bufferPtr);
            }
            break;

        /* Resource 4: Velocity */
        case LWM2MCORE_LOCATION_VELOCITY_RID:
            /* Build the velocity with direction, horizontal and vertical speeds */
            sID = BuildVelocity(bufferPtr, lenPtr);
            break;

        /* Resource 5: Timestamp */
        case LWM2MCORE_LOCATION_TIMESTAMP_RID:
        {
            uint64_t timestamp = 0;
            sID = lwm2mcore_GetLocationTimestamp(&timestamp);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = omanager_FormatValueToBytes((uint8_t*)bufferPtr,
                                             &timestamp,
                                             sizeof(timestamp),
                                             false);
            }
        }
        break;

        default:
            sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            break;
    }

    return sID;
}

//--------------------------------------------------------------------------------------------------
/**
 *                              OBJECT 7: CONNECTIVITY STATISTICS
 */
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Function to read a resource of object 7
 * Object: 7 - Connectivity statistics
 * Resource: All with read operation
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
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int omanager_ReadConnectivityStatisticsObj
(
    lwm2mcore_Uri_t* uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource
    char* bufferPtr,                    ///< [INOUT] data buffer for information
    size_t* lenPtr,                     ///< [INOUT] length of input buffer and length of the
                                        ///< returned data
    valueChangedCallback_t changedCb    ///< [IN] callback for notification
)
{
    int sID = LWM2MCORE_ERR_GENERAL_ERROR;

    (void)changedCb;

    if ((!uriPtr) || (!bufferPtr) || (!lenPtr))
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    /* Check that the object instance Id is in the correct range (only one object instance) */
    if (0 < uriPtr->oiid)
    {
        return LWM2MCORE_ERR_INCORRECT_RANGE;
    }

    /* Check that the operation is coherent */
    if (0 == (uriPtr->op & LWM2MCORE_OP_READ))
    {
        return LWM2MCORE_ERR_OP_NOT_SUPPORTED;
    }

    switch (uriPtr->rid)
    {
        /* Resource 0: SMS Tx counter */
        case LWM2MCORE_CONN_STATS_TX_SMS_COUNT_RID:
        {
            uint64_t smsTxCount;
            sID = lwm2mcore_GetSmsTxCount(&smsTxCount);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = omanager_FormatValueToBytes((uint8_t*)bufferPtr,
                                             &smsTxCount,
                                             sizeof(smsTxCount),
                                             false);
            }
        }
        break;

        /* Resource 1: SMS Rx counter */
        case LWM2MCORE_CONN_STATS_RX_SMS_COUNT_RID:
        {
            uint64_t smsRxCount;
            sID = lwm2mcore_GetSmsRxCount(&smsRxCount);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = omanager_FormatValueToBytes((uint8_t*)bufferPtr,
                                             &smsRxCount,
                                             sizeof(smsRxCount),
                                             false);
            }
        }
        break;

        /* Resource 2: Tx data */
        case LWM2MCORE_CONN_STATS_TX_DATA_COUNT_RID:
        {
            uint64_t txData;
            sID = lwm2mcore_GetTxData(&txData);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = omanager_FormatValueToBytes((uint8_t*)bufferPtr,
                                             &txData,
                                             sizeof(txData),
                                             false);
            }
        }
        break;

        /* Resource 3: Rx data */
        case LWM2MCORE_CONN_STATS_RX_DATA_COUNT_RID:
        {
            uint64_t rxData;
            sID = lwm2mcore_GetRxData(&rxData);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = omanager_FormatValueToBytes((uint8_t*)bufferPtr,
                                             &rxData,
                                             sizeof(rxData),
                                             false);
            }
        }
        break;

        default:
            sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            break;
    }

    return sID;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to execute a resource of object 7
 * Object: 7 - Connectivity statistics
 * Resource: All with execute operation
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int omanager_ExecConnectivityStatistics
(
    lwm2mcore_Uri_t* uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource
    char* bufferPtr,                    ///< [INOUT] data buffer for information
    size_t len                          ///< [IN] length of input buffer
)
{
    int sID = LWM2MCORE_ERR_GENERAL_ERROR;

    if ((NULL == uriPtr) || (len && (NULL == bufferPtr)))
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    /* Check that the object instance Id is in the correct range (only one object instance) */
    if (0 < uriPtr->oiid)
    {
        return LWM2MCORE_ERR_INCORRECT_RANGE;
    }

    /* Check that the operation is coherent */
    if (0 == (uriPtr->op & LWM2MCORE_OP_EXECUTE))
    {
        return LWM2MCORE_ERR_OP_NOT_SUPPORTED;
    }

    switch (uriPtr->rid)
    {
        /* Resource 6: Start */
        case LWM2MCORE_CONN_STATS_START_RID:
            sID = lwm2mcore_StartConnectivityCounters();
            break;

        /* Resource 7: Stop */
        case LWM2MCORE_CONN_STATS_STOP_RID:
            sID = lwm2mcore_StopConnectivityCounters();
            break;

        default:
            sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            break;
    }

    return sID;
}

//--------------------------------------------------------------------------------------------------
/**
 *                                  OBJECT 9: SOFTWARE UPDATE
 */
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Function to write a resource of object 9
 * Object: 9 - software update
 * Resource: all with write operation
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int omanager_WriteSwUpdateObj
(
    lwm2mcore_Uri_t* uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource
    char* bufferPtr,                    ///< [INOUT] data buffer for information
    size_t len                          ///< [IN] length of input buffer
)
{
    int sID = LWM2MCORE_ERR_GENERAL_ERROR;
    if ((NULL == uriPtr) || (NULL == bufferPtr))
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    /* Check if the related command is WRITE */
    if (0 == (uriPtr->op & LWM2MCORE_OP_WRITE))
    {
        return LWM2MCORE_ERR_OP_NOT_SUPPORTED;
    }

    switch (uriPtr->rid)
    {
        /* Resource 3: Package URI */
        case LWM2MCORE_SW_UPDATE_PACKAGE_URI_RID:
            if (LWM2MCORE_BUFFER_MAX_LEN < len)
            {
                sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            }
            else
            {
                sID = lwm2mcore_SetUpdatePackageUri(LWM2MCORE_SW_UPDATE_TYPE,
                                                    uriPtr->oiid,
                                                    bufferPtr,
                                                    len);
            }
            break;

        /* Resource 8: Update Supported Objects */
        case LWM2MCORE_SW_UPDATE_UPDATE_SUPPORTED_OBJ_RID:
            if (LWM2MCORE_BUFFER_MAX_LEN < len)
            {
                sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            }
            else
            {
                sID = lwm2mcore_SetSwUpdateSupportedObjects(uriPtr->oiid,
                                (bool)omanager_BytesToInt((const char*)bufferPtr, len));
            }
            break;

        default:
            sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            break;
    }
    return sID;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to read a resource of object 9
 * Object: 9 - Software update
 * Resource: all with read operation
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int omanager_ReadSwUpdateObj
(
    lwm2mcore_Uri_t* uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource
    char* bufferPtr,                    ///< [INOUT] data buffer for information
    size_t* lenPtr,                     ///< [INOUT] length of input buffer and length of the
                                        ///< returned data
    valueChangedCallback_t changedCb    ///< [IN] not used for READ operation but for WRITE one
)
{
    int sID = LWM2MCORE_ERR_GENERAL_ERROR;

    (void)changedCb;

    if ((NULL == uriPtr) || (NULL == bufferPtr) || (NULL == lenPtr))
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    /* Check if the related command is READ */
    if (0 == (uriPtr->op & LWM2MCORE_OP_READ))
    {
        return LWM2MCORE_ERR_OP_NOT_SUPPORTED;
    }

    switch (uriPtr->rid)
    {
        /* Resource 0: package name */
        case LWM2MCORE_SW_UPDATE_PACKAGE_NAME_RID:
            sID = lwm2mcore_GetUpdatePackageName(LWM2MCORE_SW_UPDATE_TYPE,
                                                 uriPtr->oiid,
                                                 bufferPtr,
                                                 (uint32_t)*lenPtr);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = strlen(bufferPtr);
            }
            break;

        /* Resource 1: package version */
        case LWM2MCORE_SW_UPDATE_PACKAGE_VERSION_RID:
            sID = lwm2mcore_GetUpdatePackageVersion(LWM2MCORE_SW_UPDATE_TYPE,
                                                    uriPtr->oiid,
                                                    bufferPtr,
                                                    (uint32_t)*lenPtr);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = strlen(bufferPtr);
            }
            break;

        /* Resource 7: Update State */
        case LWM2MCORE_SW_UPDATE_UPDATE_STATE_RID:
        {
            uint8_t updateResult;
            sID = lwm2mcore_GetUpdateState(LWM2MCORE_SW_UPDATE_TYPE, uriPtr->oiid, &updateResult);
            if (sID == LWM2MCORE_ERR_COMPLETED_OK)
            {
                *lenPtr = omanager_FormatValueToBytes((uint8_t*) bufferPtr,
                                             &updateResult,
                                             sizeof(updateResult),
                                             false);
            }
        }
        break;

        /* Resource 8: Update Supported Objects */
        case LWM2MCORE_SW_UPDATE_UPDATE_SUPPORTED_OBJ_RID:
        {
            bool value;
            sID = lwm2mcore_GetSwUpdateSupportedObjects(uriPtr->oiid, &value);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = omanager_FormatValueToBytes((uint8_t*) bufferPtr,
                                             &value,
                                             sizeof(value),
                                             false);
            }
        }
        break;

        /* Resource 9: Update result */
        case LWM2MCORE_SW_UPDATE_UPDATE_RESULT_RID:
        {
            uint8_t updateResult;
            sID = lwm2mcore_GetUpdateResult(LWM2MCORE_SW_UPDATE_TYPE, uriPtr->oiid, &updateResult);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = omanager_FormatValueToBytes((uint8_t*) bufferPtr,
                                             &updateResult,
                                             sizeof(updateResult),
                                             false);
            }
        }
        break;

        /* Resource 12: Activation state */
        case LWM2MCORE_SW_UPDATE_ACTIVATION_STATE_RID:
        {
            bool value;
            sID = lwm2mcore_GetSwUpdateActivationState(uriPtr->oiid, &value);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = omanager_FormatValueToBytes((uint8_t*) bufferPtr,
                                             &value,
                                             sizeof(value),
                                             false);
            }
        }
        break;

        default:
            sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            break;
    }
    return sID;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to execute a resource of object 9
 * Object: 9 - Software update
 * Resource: all with execute operation
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int omanager_ExecSwUpdate
(
    lwm2mcore_Uri_t* uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource
    char* bufferPtr,                    ///< [INOUT] data buffer for information
    size_t len                          ///< [IN] length of input buffer
)
{
    int sID = LWM2MCORE_ERR_GENERAL_ERROR;

    if ((NULL == uriPtr) || ((NULL == bufferPtr) && len))
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    /* Check if the related command is EXECUTE */
    if (0 == (uriPtr->op & LWM2MCORE_OP_EXECUTE))
    {
        return LWM2MCORE_ERR_OP_NOT_SUPPORTED;
    }

    switch (uriPtr->rid)
    {
        /* Resource 4: Install */
        case LWM2MCORE_SW_UPDATE_INSTALL_RID:
            sID = lwm2mcore_LaunchUpdate(LWM2MCORE_SW_UPDATE_TYPE, uriPtr->oiid, bufferPtr, len);
            break;

        /* Resource 6: Uninstall */
        case LWM2MCORE_SW_UPDATE_UNINSTALL_RID:
            sID = lwm2mcore_LaunchSwUpdateUninstall(uriPtr->oiid, bufferPtr, len);
            break;

        /* Resource 10: Activate */
        case LWM2MCORE_SW_UPDATE_ACTIVATE_RID:
            sID = lwm2mcore_ActivateSoftware(true, uriPtr->oiid, bufferPtr, len);
            break;

        /* Resource 11: Deactivate */
        case LWM2MCORE_SW_UPDATE_DEACTIVATE_RID:
            sID = lwm2mcore_ActivateSoftware(false, uriPtr->oiid, bufferPtr, len);
            break;

        default:
            sID = LWM2MCORE_ERR_INCORRECT_RANGE;
        break;
    }

    return sID;
}

//--------------------------------------------------------------------------------------------------
/**
 *                              OBJECT 10241: SUBSCRIPTION
 */
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Function to read a resource of object 10241
 * Object: 10241 - Subscription
 * Resource: All with read operation
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
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int omanager_ReadSubscriptionObj
(
    lwm2mcore_Uri_t* uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource
    char* bufferPtr,                    ///< [INOUT] data buffer for information
    size_t* lenPtr,                     ///< [INOUT] length of input buffer and length of the
                                        ///< returned data
    valueChangedCallback_t changedCb    ///< [IN] callback for notification
)
{
    int sID = LWM2MCORE_ERR_GENERAL_ERROR;

    (void)changedCb;

    if ((!uriPtr) || (!bufferPtr) || (!lenPtr))
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    /* Check that the object instance Id is in the correct range (only one object instance) */
    if (0 < uriPtr->oiid)
    {
        return LWM2MCORE_ERR_INCORRECT_RANGE;
    }

    /* Check that the operation is coherent */
    if (0 == (uriPtr->op & LWM2MCORE_OP_READ))
    {
        return LWM2MCORE_ERR_OP_NOT_SUPPORTED;
    }

    switch (uriPtr->rid)
    {
        /* Resource 0: Module identity */
        case LWM2MCORE_SUBSCRIPTION_IMEI_RID:
            sID = lwm2mcore_GetDeviceImei(bufferPtr, lenPtr);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = strlen(bufferPtr);
            }
            break;

        /* Resource 1: SIM card identifier */
        case LWM2MCORE_SUBSCRIPTION_ICCID_RID:
            sID = lwm2mcore_GetIccid(bufferPtr, lenPtr);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = strlen(bufferPtr);
            }
            break;

        /* Resource 2: Subscription identity */
        case LWM2MCORE_SUBSCRIPTION_IDENTITY_RID:
            sID = lwm2mcore_GetSubscriptionIdentity(bufferPtr, lenPtr);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = strlen(bufferPtr);
            }
            break;

        /* Resource 3: Subscription phone number */
        case LWM2MCORE_SUBSCRIPTION_MSISDN_RID:
            sID = lwm2mcore_GetMsisdn(bufferPtr, lenPtr);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = strlen(bufferPtr);
            }
            break;

        /* Resource 5: Currently used SIM */
        case LWM2MCORE_SUBSCRIPTION_CURRENT_SIM_RID:
        {
            int currentSim = 0;
            sID = lwm2mcore_GetCurrentSimCard(&currentSim);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = omanager_FormatValueToBytes((uint8_t*)bufferPtr,
                                                      &currentSim,
                                                      sizeof(currentSim),
                                                      false);
            }
        }
        break;

        /* Resource 6: Switch SIM */
        case LWM2MCORE_SUBSCRIPTION_SWITCH_SIM_RID:
        {
            int status = 0;
            sID = lwm2mcore_GetSimSwitchStatus(&status);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = omanager_FormatValueToBytes((uint8_t*)bufferPtr,
                                                      &status,
                                                      sizeof(status),
                                                      false);
            }
        }
        break;

        default:
            sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            break;
    }

    return sID;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to exec a resource of object 10241
 * Object: 10241 - Subscription
 * Resource: 4
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
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int omanager_ExecSubscriptionObj
(
    lwm2mcore_Uri_t* uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource
    char* bufferPtr,                    ///< [INOUT] data buffer for information
    size_t len                          ///< [IN] length of input buffer
)
{
    int sID = LWM2MCORE_ERR_GENERAL_ERROR;

    if ((NULL == uriPtr) || (len && (NULL == bufferPtr)))
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    /* Check that the object instance Id is in the correct range (only one object instance) */
    if (0 != uriPtr->oiid)
    {
        return LWM2MCORE_ERR_INCORRECT_RANGE;
    }

    /* Check that the operation is coherent */
    if (0 == (uriPtr->op & LWM2MCORE_OP_EXECUTE))
    {
        return LWM2MCORE_ERR_OP_NOT_SUPPORTED;
    }

    switch (uriPtr->rid)
    {
        /* Resource 4: SIM mode */
        case LWM2MCORE_SUBSCRIPTION_SIM_MODE_RID:
            sID = lwm2mcore_SetSimMode(bufferPtr, &len);
            break;

        default:
            sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            break;
    }

    return sID;
}

//--------------------------------------------------------------------------------------------------
/**
 *                          OBJECT 10242: EXTENDED CONNECTIVITY STATISTICS
 */
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Function to read a resource of object 10242
 * Object: 10242 - Extended connectivity statistics
 * Resource: All
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
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int omanager_ReadExtConnectivityStatsObj
(
    lwm2mcore_Uri_t* uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource
    char* bufferPtr,                    ///< [INOUT] data buffer for information
    size_t* lenPtr,                     ///< [INOUT] length of input buffer and length of the
                                        ///< returned data
    valueChangedCallback_t changedCb    ///< [IN] callback for notification
)
{
    int sID = LWM2MCORE_ERR_GENERAL_ERROR;

    (void)changedCb;

    if ((!uriPtr) || (!bufferPtr) || (!lenPtr))
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    /* Check that the object instance Id is in the correct range (only one object instance) */
    if (0 < uriPtr->oiid)
    {
        return LWM2MCORE_ERR_INCORRECT_RANGE;
    }

    /* Check that the operation is coherent */
    if (0 == (uriPtr->op & LWM2MCORE_OP_READ))
    {
        return LWM2MCORE_ERR_OP_NOT_SUPPORTED;
    }

    switch (uriPtr->rid)
    {
        /* Resource 0: Signal bars */
        case LWM2MCORE_EXT_CONN_STATS_SIGNAL_BARS_RID:
        {
            uint8_t signalBars = 0;
            sID = lwm2mcore_GetSignalBars(&signalBars);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = omanager_FormatValueToBytes((uint8_t*)bufferPtr,
                                             &signalBars,
                                             sizeof(signalBars),
                                             false);
            }
        }
        break;

        /* Resource 1: Currently used cellular technology */
        case LWM2MCORE_EXT_CONN_STATS_CELLULAR_TECH_RID:
            sID = lwm2mcore_GetCellularTechUsed((char*)bufferPtr, lenPtr);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = strlen((char*)bufferPtr);
            }
            break;

        /* Resource 2: Roaming indicator */
        case LWM2MCORE_EXT_CONN_STATS_ROAMING_RID:
        {
            uint8_t roamingIndicator = 0;
            sID = lwm2mcore_GetRoamingIndicator(&roamingIndicator);
            {
                *lenPtr = omanager_FormatValueToBytes((uint8_t*)bufferPtr,
                                             &roamingIndicator,
                                             sizeof(roamingIndicator),
                                             false);
            }
        }
        break;

        /* Resource 3: Ec/Io */
        case LWM2MCORE_EXT_CONN_STATS_ECIO_RID:
        {
            int32_t ecIo = 0;
            sID = lwm2mcore_GetEcIo(&ecIo);
            {
                *lenPtr = omanager_FormatValueToBytes((uint8_t*)bufferPtr,
                                             &ecIo,
                                             sizeof(ecIo),
                                             true);
            }
        }
        break;

        /* Resource 4: RSRP */
        case LWM2MCORE_EXT_CONN_STATS_RSRP_RID:
        {
            int32_t rsrp = 0;
            sID = lwm2mcore_GetRsrp(&rsrp);
            {
                *lenPtr = omanager_FormatValueToBytes((uint8_t*)bufferPtr,
                                             &rsrp,
                                             sizeof(rsrp),
                                             true);
            }
        }
        break;

        /* Resource 5: RSRQ */
        case LWM2MCORE_EXT_CONN_STATS_RSRQ_RID:
        {
            int32_t rsrq = 0;
            sID = lwm2mcore_GetRsrq(&rsrq);
            {
                *lenPtr = omanager_FormatValueToBytes((uint8_t*)bufferPtr,
                                             &rsrq,
                                             sizeof(rsrq),
                                             true);
            }
        }
        break;

        /* Resource 6: RSCP */
        case LWM2MCORE_EXT_CONN_STATS_RSCP_RID:
        {
            int32_t rscp = 0;
            sID = lwm2mcore_GetRscp(&rscp);
            {
                *lenPtr = omanager_FormatValueToBytes((uint8_t*)bufferPtr,
                                             &rscp,
                                             sizeof(rscp),
                                             true);
            }
        }
        break;

        /* Resource 7: Device temperature */
        case LWM2MCORE_EXT_CONN_STATS_TEMPERATURE_RID:
        {
            int32_t deviceTemperature = 0;
            sID = lwm2mcore_GetDeviceTemperature(&deviceTemperature);
            {
                *lenPtr = omanager_FormatValueToBytes((uint8_t*)bufferPtr,
                                             &deviceTemperature,
                                             sizeof(deviceTemperature),
                                             true);
            }
        }
        break;

        /* Resource 8: Unexpected reset counter */
        case LWM2MCORE_EXT_CONN_STATS_UNEXPECTED_RESETS_RID:
        {
            uint32_t unexpectedResets = 0;
            sID = lwm2mcore_GetDeviceUnexpectedResets(&unexpectedResets);
            {
                *lenPtr = omanager_FormatValueToBytes((uint8_t*)bufferPtr,
                                             &unexpectedResets,
                                             sizeof(unexpectedResets),
                                             false);
            }
        }
        break;

        /* Resource 9: Total reset counter */
        case LWM2MCORE_EXT_CONN_STATS_TOTAL_RESETS_RID:
        {
            uint32_t totalResets = 0;
            sID = lwm2mcore_GetDeviceTotalResets(&totalResets);
            {
                *lenPtr = omanager_FormatValueToBytes((uint8_t*)bufferPtr,
                                             &totalResets,
                                             sizeof(totalResets),
                                             false);
            }
        }
        break;

        /* Resource 10: Location Area Code */
        case LWM2MCORE_EXT_CONN_STATS_LAC_RID:
        {
            uint32_t lac = 0;
            sID = lwm2mcore_GetLac(&lac);
            {
                *lenPtr = omanager_FormatValueToBytes((uint8_t*)bufferPtr,
                                             &lac,
                                             sizeof(lac),
                                             false);
            }
        }
        break;

        default:
            sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            break;
    }

    return sID;
}

//--------------------------------------------------------------------------------------------------
/**
 *                              OBJECT 10243: SSL certificates
 */
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Function to update the SSL certificates
 * Object: 10243 - SSL certificates
 * Resource: 0
 *
 * @note To delete the saved certificate, set the length to 0
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the update succeeds
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the size of the certificate is > 4000 bytes
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the update fails
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid
 */
//--------------------------------------------------------------------------------------------------
int omanager_WriteSslCertif
(
    lwm2mcore_Uri_t* uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource
    char* bufferPtr,                    ///< [INOUT] data buffer for information
    size_t len                          ///< [IN] length of input buffer
)
{
    if (!uriPtr)
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    if ( (!(uriPtr->op & LWM2MCORE_OP_WRITE)) || (uriPtr->oiid) )
    {
        return LWM2MCORE_ERR_OP_NOT_SUPPORTED;
    }

    return lwm2mcore_UpdateSslCertificate(bufferPtr, len);
}

//--------------------------------------------------------------------------------------------------
/**
 * Function for not registered objects
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int omanager_OnUnlistedObject
(
    lwm2mcore_Uri_t* uriPtr,                ///< [IN] uriPtr represents the requested operation and
                                            ///< object/resource
    char* bufferPtr,                        ///< [INOUT] data buffer for information
    size_t* lenPtr,                         ///< [INOUT] length of input buffer and length of the
                                            ///< returned data
    valueChangedCallback_t changedCb        ///< [IN] callback function pointer for OBSERVE
                                            ///< operation
)
{
    (void)uriPtr;
    (void)bufferPtr;
    (void)lenPtr;
    (void)changedCb;

    return LWM2MCORE_ERR_NOT_YET_IMPLEMENTED;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to get the number of security and server objects in the bootstrap information
 *
 * @return
 *  - true on success
 *  - false on failure
 */
//--------------------------------------------------------------------------------------------------
bool ConfigGetObjectsNumber
(
    uint16_t* securityObjectNumberPtr,  ///< [IN] Number of security objects in the bootstrap
                                        ///< information
    uint16_t* serverObjectNumberPtr     ///< [IN] Number of server objects in the bootstrap
                                        ///< information
)
{
    if ((!securityObjectNumberPtr) || (!serverObjectNumberPtr))
    {
        return false;
    }

    *securityObjectNumberPtr = BsConfigList.securityObjectNumber;
    *serverObjectNumberPtr = BsConfigList.serverObjectNumber;

    return true;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to free the bootstrap information list
 */
//--------------------------------------------------------------------------------------------------
void omanager_FreeBootstrapInformation
(
    void
)
{
    FreeBootstrapInformation(&BsConfigList);
}

//--------------------------------------------------------------------------------------------------
/**
 * Delete all device management credentials
 */
//--------------------------------------------------------------------------------------------------
void omanager_DeleteDmCredentials
(
    void
)
{
    ConfigSecurityObject_t* securityInformationPtr = BsConfigList.securityPtr;
    ConfigSecurityObject_t* newSecurityInformationPtr = NULL;
    ConfigServerObject_t* serverInformationPtr;

    while (securityInformationPtr)
    {
        lwm2mcore_DeleteCredential(LWM2MCORE_CREDENTIAL_DM_PUBLIC_KEY,
                                   securityInformationPtr->data.serverId);
        lwm2mcore_DeleteCredential(LWM2MCORE_CREDENTIAL_DM_SERVER_PUBLIC_KEY,
                                   securityInformationPtr->data.serverId);
        lwm2mcore_DeleteCredential(LWM2MCORE_CREDENTIAL_DM_SECRET_KEY,
                                   securityInformationPtr->data.serverId);
        lwm2mcore_DeleteCredential(LWM2MCORE_CREDENTIAL_DM_ADDRESS,
                                   securityInformationPtr->data.serverId);

        /* Delete bootstrap information related to DM servers */
        if (false == (securityInformationPtr->data.isBootstrapServer))
        {
            ConfigSecurityObject_t* nextPtr = securityInformationPtr->nextPtr;
            omanager_FreeObjectByInstanceId(LWM2MCORE_SECURITY_OID,
                                            securityInformationPtr->data.securityObjectInstanceId);
            lwm2m_free(securityInformationPtr);
            securityInformationPtr = nextPtr;
            BsConfigList.securityObjectNumber--;
        }
        else
        {
            newSecurityInformationPtr = securityInformationPtr;
            securityInformationPtr = securityInformationPtr->nextPtr;
            newSecurityInformationPtr->nextPtr = NULL;
        }
    }
    BsConfigList.securityPtr = newSecurityInformationPtr;

    /* Delete all information about servers */
    serverInformationPtr = BsConfigList.serverPtr;
    while (NULL != serverInformationPtr)
    {
        ConfigServerObject_t* nextPtr = serverInformationPtr->nextPtr;
        lwm2m_free(serverInformationPtr);
        serverInformationPtr = nextPtr;
        BsConfigList.serverObjectNumber--;
    }
    BsConfigList.serverObjectNumber = 0;
    BsConfigList.serverPtr = NULL;

    /* Unregister all object instances of object 1 in Wakaama */
    omanager_FreeObjectById(LWM2MCORE_SERVER_OID);

    /* Store the new configuration */
    StoreBootstrapConfiguration(BsConfigList);
}
