/**
 * @file bootstrapConfiguration.c
 *
 * Bootstrap information management
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#include <stdint.h>
#include <stddef.h>
#include <platform/types.h>
#include <lwm2mcore/lwm2mcore.h>
#include <lwm2mcore/security.h>
#include <lwm2mcore/server.h>
#include <lwm2mcore/paramStorage.h>
#include "handlers.h"
#include "objects.h"
#include "internals.h"
#include "bootstrapConfiguration.h"
#include "liblwm2m.h"

//--------------------------------------------------------------------------------------------------
/**
 * Structure for bootstrap configuration: list of received bootstrap information
 * This structure needs to be stored in platform storage
 */
//--------------------------------------------------------------------------------------------------
static ConfigBootstrapFile_t BsConfigList;

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

    if (!configPtr)
    {
        return;
    }

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

    if (!configPtr)
    {
        return;
    }

    FreeBootstrapInformation(configPtr);
    memset(configPtr, 0, sizeof(ConfigBootstrapFile_t));
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
    omanager_AddBootstrapConfigurationSecurity(configPtr, securityInformationPtr);
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
            omanager_AddBootstrapConfigurationSecurity(&BsConfigList, securityInformationPtr);

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
            omanager_AddBootstrapConfigurationSecurity(&BsConfigList, securityInformationPtr);

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
            omanager_AddBootstrapConfigurationServer(&BsConfigList, serverInformationPtr);

            return true;
        }
        /* Else consider that no connection was made to bootstrap */
        LOG("DM credentials are NOT present");
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to get the bootstrap information from RAM
 *
 * @return
 *  - Boostrap configuration structure pointer
 */
//--------------------------------------------------------------------------------------------------
ConfigBootstrapFile_t* omanager_GetBootstrapConfiguration
(
    void
)
{
    return &BsConfigList;
}

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
ConfigSecurityObject_t* omanager_GetBootstrapConfigurationSecurityInstance
(
    ConfigBootstrapFile_t*  bsConfigListPtr,            ///< [IN] Bootstrap information list
    uint16_t                securityObjectInstanceId    ///< [IN] Security object instance Id
)
{
    ConfigSecurityObject_t* bsInfoPtr;

    if(!bsConfigListPtr)
    {
        return NULL;
    }

    bsInfoPtr = bsConfigListPtr->securityPtr;

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
ConfigServerObject_t* omanager_GetBootstrapConfigurationServerInstance
(
    ConfigBootstrapFile_t*  bsConfigListPtr,            ///< [IN] Bootstrap information list
    uint16_t                serverObjectInstanceId      ///< [IN] Server object instance Id
)
{
    ConfigServerObject_t* bsInfoPtr;

    if(!bsConfigListPtr)
    {
        return NULL;
    }

    bsInfoPtr = bsConfigListPtr->serverPtr;

    while ((bsInfoPtr) && ((bsInfoPtr->data.serverObjectInstanceId) != serverObjectInstanceId))
    {
        bsInfoPtr = bsInfoPtr->nextPtr;
    }
    return bsInfoPtr;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to add an object instance of object 0 (security) in bootstrap information list
 */
//--------------------------------------------------------------------------------------------------
void omanager_AddBootstrapConfigurationSecurity
(
    ConfigBootstrapFile_t* bsConfigListPtr,         ///< [IN] Bootstrap information list
    ConfigSecurityObject_t* securityInformationPtr  ///< [IN] Security object
)
{
    if(!bsConfigListPtr || !securityInformationPtr)
    {
        return;
    }

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
void omanager_AddBootstrapConfigurationServer
(
    ConfigBootstrapFile_t*  bsConfigListPtr,          ///< [IN] Bootstrap information list
    ConfigServerObject_t*   serverInformationPtr      ///< [IN] Server object
)
{
    if((!bsConfigListPtr) || (!serverInformationPtr))
    {
        return;
    }

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
bool omanager_StoreBootstrapConfiguration
(
    ConfigBootstrapFile_t* bsConfigPtr          ///< [IN] Bootstrap configuration to store
)
{
    bool result = false;
    uint32_t lenToStore;
    uint32_t lenWritten = 0;
    uint32_t len = sizeof(lenToStore);
    uint16_t loop = 0;
    uint8_t* dataPtr;
    uint8_t* dataLenPtr;

    ConfigSecurityObject_t* securityPtr;
    ConfigServerObject_t* serverPtr;

    if (!bsConfigPtr)
    {
        return false;
    }

    securityPtr = bsConfigPtr->securityPtr;
    serverPtr = bsConfigPtr->serverPtr;

    lenToStore = sizeof(bsConfigPtr->version) +
                 sizeof(bsConfigPtr->securityObjectNumber) +
                 sizeof(bsConfigPtr->serverObjectNumber) +
                 sizeof(ConfigSecurityToStore_t) * bsConfigPtr->securityObjectNumber +
                 sizeof(ConfigServerToStore_t) * bsConfigPtr->serverObjectNumber;

    LOG_ARG("Store BS config securityObjectNumber %d serverObjectNumber %d",
            bsConfigPtr->securityObjectNumber,
            bsConfigPtr->serverObjectNumber);
    LOG_ARG("lenToStore %d", lenToStore);

    dataPtr = (uint8_t*)lwm2m_malloc(lenToStore);
    if (!dataPtr)
    {
        return false;
    }
    memset(dataPtr, 0, lenToStore);

    /* Copy the version */
    memcpy(dataPtr + lenWritten, &(bsConfigPtr->version), sizeof(bsConfigPtr->version));
    lenWritten += sizeof(bsConfigPtr->version);

    /* Copy the number of security objects and server objects */
    memcpy(dataPtr + lenWritten,
           &(bsConfigPtr->securityObjectNumber),
           sizeof(bsConfigPtr->securityObjectNumber));
    lenWritten += sizeof(bsConfigPtr->securityObjectNumber);

    memcpy(dataPtr + lenWritten,
           &(bsConfigPtr->serverObjectNumber),
           sizeof(bsConfigPtr->serverObjectNumber));
    lenWritten += sizeof(bsConfigPtr->serverObjectNumber);

    /* Copy security objects data */
    loop = bsConfigPtr->securityObjectNumber;
    while (loop && securityPtr)
    {
        memcpy(dataPtr + lenWritten, &(securityPtr->data), sizeof(ConfigSecurityToStore_t));
        lenWritten += sizeof(ConfigSecurityToStore_t);
        securityPtr = securityPtr->nextPtr;
        loop--;
    }

    /* Copy server objects data */
    loop = bsConfigPtr->serverObjectNumber;
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
    LOG_ARG("Result %d", result);
    return result;
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
bool omanager_LoadBootstrapConfiguration
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
    size_t fileReadSize = 0;
    size_t len = sizeof(fileSize);
    ConfigSecurityObject_t*     securityPtr;
    ConfigServerObject_t*       serverPtr;
    uint8_t* rawData;

    if (!configPtr)
    {
        return false;
    }

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
            omanager_StoreBootstrapConfiguration(configPtr);
        }

        LOG("Loaded default BS configuration");
        return true;
    }

    rawData = (uint8_t*)lwm2m_malloc(fileSize);
    LWM2MCORE_ASSERT(rawData);
    fileReadSize = (size_t)fileSize;
    /* Get the bootstrap information file */
    sid = lwm2mcore_GetParam(LWM2MCORE_BOOTSTRAP_PARAM, rawData, &fileReadSize);
    LOG_ARG("Read BS configuration: fileReadSize %ld result %d", fileReadSize, sid);

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
            omanager_StoreBootstrapConfiguration(configPtr);
        }
        lwm2m_free(rawData);
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
            omanager_StoreBootstrapConfiguration(configPtr);
        }
        return false;
    }

    if (fileSize < (sizeof(configPtr->version) +
                    sizeof(configPtr->securityObjectNumber) +
                    sizeof(configPtr->serverObjectNumber)))
    {
        lwm2m_free(rawData);
        return false;
    }

    /* Copy the version */
    configPtr->version = (uint32_t)(((ConfigBootstrapFile_t*)((void*)rawData))->version);
    lenWritten += sizeof(configPtr->version);

    /* Copy the number of security objects and server objects */
    configPtr->securityObjectNumber = (uint16_t)(((ConfigBootstrapFile_t*)((void*)rawData))->securityObjectNumber);
    lenWritten += sizeof(configPtr->securityObjectNumber);
    configPtr->serverObjectNumber = (uint16_t)(((ConfigBootstrapFile_t*)((void*)rawData))->serverObjectNumber);
    lenWritten += sizeof(configPtr->serverObjectNumber);

    /* Allocate security objects and copy related data */
    for (loop = 0; loop < configPtr->securityObjectNumber; loop++)
    {
        if (fileSize >= (lenWritten + sizeof(ConfigSecurityToStore_t)))
        {
            securityPtr = (ConfigSecurityObject_t*)lwm2m_malloc(sizeof(ConfigSecurityObject_t));
            if (securityPtr)
            {
                memset(securityPtr, 0, sizeof(ConfigSecurityObject_t));
                memcpy(securityPtr, rawData + lenWritten, sizeof(ConfigSecurityToStore_t));
                lenWritten += sizeof(ConfigSecurityToStore_t);

                /* Check if the security object instance Id is already stored */
                if (!omanager_GetBootstrapConfigurationSecurityInstance(configPtr,
                                                                        securityPtr->data.securityObjectInstanceId))
                {
                    omanager_AddBootstrapConfigurationSecurity(configPtr, securityPtr);
                }
                else
                {
                    lwm2m_free(securityPtr);
                }
            }
            else
            {
                lwm2m_free(rawData);
                omanager_FreeBootstrapInformation();
                return false;
            }
        }
        else
        {
            lwm2m_free(rawData);
            omanager_FreeBootstrapInformation();
            return false;
        }
    }

    /* Allocate server objects and copy related data */
    for (loop = 0; loop < configPtr->serverObjectNumber; loop++)
    {
        if (fileSize >= (lenWritten + sizeof(ConfigServerToStore_t)))
        {
            serverPtr = (ConfigServerObject_t*)lwm2m_malloc(sizeof(ConfigServerObject_t));
            if (serverPtr)
            {
                memset(serverPtr, 0, sizeof(ConfigServerObject_t));
                memcpy(serverPtr, rawData + lenWritten, sizeof(ConfigServerToStore_t));
                lenWritten += sizeof(ConfigServerToStore_t);

                /* Check if the server object instance Id is already stored */
                if (!omanager_GetBootstrapConfigurationServerInstance(configPtr,
                                                                      serverPtr->data.serverObjectInstanceId))
                {
                    omanager_AddBootstrapConfigurationServer(configPtr, serverPtr);
                }
                else
                {
                    lwm2m_free(serverPtr);
                }
            }
            else
            {
                lwm2m_free(rawData);
                omanager_FreeBootstrapInformation();
                return false;
            }
        }
        else
        {
            lwm2m_free(rawData);
            omanager_FreeBootstrapInformation();
            return false;
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
        omanager_StoreBootstrapConfiguration(configPtr);
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
bool omanager_LoadBootstrapConfigurationFile
(
    void
)
{
    return omanager_LoadBootstrapConfiguration(&BsConfigList, true);
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
bool omanager_GetBootstrapConfigObjectsNumber
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
 *
 * @return
 *  - true if DM credentials were deleted
 *  - false if DM were not present
 */
//--------------------------------------------------------------------------------------------------
bool omanager_DeleteDmCredentials
(
    void
)
{
    ConfigSecurityObject_t* securityInformationPtr = BsConfigList.securityPtr;
    ConfigSecurityObject_t* newSecurityInformationPtr = NULL;
    ConfigServerObject_t* serverInformationPtr;
    bool result = false;

    while (securityInformationPtr)
    {
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
        result = true;

        lwm2mcore_DeleteCredential(LWM2MCORE_CREDENTIAL_DM_PUBLIC_KEY,
                                   serverInformationPtr->data.serverId);
        lwm2mcore_DeleteCredential(LWM2MCORE_CREDENTIAL_DM_SERVER_PUBLIC_KEY,
                                   serverInformationPtr->data.serverId);
        lwm2mcore_DeleteCredential(LWM2MCORE_CREDENTIAL_DM_SECRET_KEY,
                                   serverInformationPtr->data.serverId);
        lwm2mcore_DeleteCredential(LWM2MCORE_CREDENTIAL_DM_ADDRESS,
                                   serverInformationPtr->data.serverId);

        lwm2m_free(serverInformationPtr);
        serverInformationPtr = nextPtr;
        BsConfigList.serverObjectNumber--;
    }
    BsConfigList.serverObjectNumber = 0;
    BsConfigList.serverPtr = NULL;

    /* Unregister all object instances of object 1 in Wakaama */
    omanager_FreeObjectById(LWM2MCORE_SERVER_OID);

    /* Store the new configuration */
    omanager_StoreBootstrapConfiguration(&BsConfigList);

    return result;
}
