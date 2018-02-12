/**
 * @file aclConfiguration.c
 *
 * ACL management
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#include <stdint.h>
#include <stddef.h>
#include <platform/types.h>
#include <lwm2mcore/lwm2mcore.h>
#include <lwm2mcore/paramStorage.h>
#include "handlers.h"
#include "objects.h"
#include "internals.h"
#include "utils.h"
#include "aclConfiguration.h"
#include "liblwm2m.h"

//--------------------------------------------------------------------------------------------------
/**
 * Structure for ACL configuration: list of received ACLs
 * This structure needs to be stored in platform storage
 */
//--------------------------------------------------------------------------------------------------
static ConfigAclFile_t AclConfigList;

//--------------------------------------------------------------------------------------------------
/**
 * Function to deallocate an object instance in the ACL configuration list
 */
//--------------------------------------------------------------------------------------------------
static void DeleteObjectInstance
(
    AclObjectInstance_t** aclObjectInstancePtr  ///< [IN] Object instance pointer to deallocate
)
{
    /* Remove the object instance */
    Acl_t* aclPtr;
    Acl_t* nextAclPtr;
    AclObjectInstance_t* nextPtr;

    if(!(*aclObjectInstancePtr))
    {
        return;
    }

    nextPtr = (*aclObjectInstancePtr)->nextPtr;
    aclPtr = (*aclObjectInstancePtr)->aclListPtr;

    /* Remove all ACL resources */
    while (aclPtr)
    {
        nextAclPtr = aclPtr->nextPtr;
        LOG_ARG("/2/%d/2/%d ACL 0x%x",
                (*aclObjectInstancePtr)->aclObjectData.objInstId,
                aclPtr->acl.resInstId,
                aclPtr->acl.accCtrlValue);
        (*aclObjectInstancePtr)->aclObjectData.aclInstanceNumber--;
        lwm2m_free(aclPtr);
        aclPtr = nextAclPtr;
    }

    lwm2m_free(*aclObjectInstancePtr);
    *aclObjectInstancePtr = nextPtr;
}

//--------------------------------------------------------------------------------------------------
/**
 * Static function to free the ACL configuration list
 */
//--------------------------------------------------------------------------------------------------
static void FreeAclConfiguration
(
    ConfigAclFile_t*  aclConfigPtr          ///< [IN] ACL configuration
)
{
    AclObjectInstance_t*    aclObjectInstancePtr;
    Acl_t*                  aclPtr;

    if (!aclConfigPtr)
    {
        return;
    }

    aclObjectInstancePtr = aclConfigPtr->aclObjectInstanceListPtr;
    while (NULL != aclObjectInstancePtr)
    {
        AclObjectInstance_t* nextPtr = aclObjectInstancePtr->nextPtr;
        aclPtr = aclObjectInstancePtr->aclListPtr;

        while (NULL != aclPtr)
        {
            Acl_t* nextAclPtr = aclPtr->nextPtr;
            lwm2m_free(aclPtr);
            aclPtr = nextAclPtr;
        }

        lwm2m_free(aclObjectInstancePtr);
        aclObjectInstancePtr = nextPtr;
    }
    aclConfigPtr->aclObjectInstanceListPtr = NULL;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to set a default ACL
 */
//--------------------------------------------------------------------------------------------------
static void SetDefaultAclConfiguration
(
    ConfigAclFile_t*  aclConfigPtr          ///< [INOUT] ACL configuration
)
{
    if (!aclConfigPtr)
    {
        return;
    }

    aclConfigPtr->version = ACL_CONFIG_VERSION;
    aclConfigPtr->instanceNumber = 0;

    FreeAclConfiguration(aclConfigPtr);
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to save the ACL configuration in platform memory
 *
 * @return
 *      - true in case of success
 *      - false in case of failure
 */
//--------------------------------------------------------------------------------------------------
static bool StoreAclConfiguration
(
    void
)
{
    bool result = false;
    uint32_t lenToStore;
    uint32_t lenWritten = 0;
    uint8_t* dataPtr;

    AclObjectInstance_t* aclObjectInstancePtr = AclConfigList.aclObjectInstanceListPtr;

    lenToStore = sizeof(AclConfigList.version) +
                 sizeof(AclConfigList.instanceNumber) +
                 sizeof(AclObjectInstanceStorage_t) * AclConfigList.instanceNumber;

    while (aclObjectInstancePtr)
    {
        lenToStore += aclObjectInstancePtr->aclObjectData.aclInstanceNumber * sizeof(AclStorage_t);
        aclObjectInstancePtr = aclObjectInstancePtr->nextPtr;
    }

    aclObjectInstancePtr = AclConfigList.aclObjectInstanceListPtr;

    dataPtr = (uint8_t*)lwm2m_malloc(lenToStore);
    if (!dataPtr)
    {
        return false;
    }
    memset(dataPtr, 0, lenToStore);

    /* Copy the version */
    memcpy(dataPtr + lenWritten, &(AclConfigList.version), sizeof(AclConfigList.version));
    lenWritten += sizeof(AclConfigList.version);

    /* Copy the number of object instances */
    memcpy(dataPtr + lenWritten,
           &(AclConfigList.instanceNumber),
           sizeof(AclConfigList.instanceNumber));
    lenWritten += sizeof(AclConfigList.instanceNumber);
    LOG_ARG("AclConfigList.instanceNumber %d", AclConfigList.instanceNumber);

    /* Copy object instances data */
    while (aclObjectInstancePtr)
    {
        Acl_t* aclPtr = aclObjectInstancePtr->aclListPtr;

        LOG_ARG("AclConfigList.aclInstanceNumber %d",
                aclObjectInstancePtr->aclObjectData.aclInstanceNumber);
        LOG_ARG("/2/%d for /%d/%d",
                aclObjectInstancePtr->aclObjectData.objInstId,
                aclObjectInstancePtr->aclObjectData.objectId,
                aclObjectInstancePtr->aclObjectData.objectInstId);

        /* Copy the object instance data: see AclObjectInstanceStorage_t */
        memcpy(dataPtr + lenWritten,
               aclObjectInstancePtr,
               sizeof(AclObjectInstanceStorage_t));
        lenWritten += sizeof(AclObjectInstanceStorage_t);

        while (aclPtr)
        {
            /* Copy the resource instance Id (server Id) and access rights*/
            LOG_ARG("ACL server Id Id %d, access rights 0x%x",
                    aclPtr->acl.resInstId, aclPtr->acl.accCtrlValue);
            memcpy(dataPtr + lenWritten, aclPtr, sizeof(AclStorage_t));
            lenWritten += sizeof(AclStorage_t);
            aclPtr = aclPtr->nextPtr;
        }

        aclObjectInstancePtr = aclObjectInstancePtr->nextPtr;
    }

    lwm2mcore_DataDump("ACL config data", dataPtr, lenToStore);

    if ( (LWM2MCORE_ERR_COMPLETED_OK == lwm2mcore_SetParam(LWM2MCORE_ACCESS_RIGHTS_SIZE_PARAM,
                                                           (uint8_t*)&lenToStore,
                                                           sizeof(lenToStore)))
      && (LWM2MCORE_ERR_COMPLETED_OK == lwm2mcore_SetParam(LWM2MCORE_ACCESS_RIGHTS_PARAM,
                                                           dataPtr,
                                                           lenToStore)))
    {
        result = true;
    }
    lwm2m_free(dataPtr);
    LOG_ARG("Set ACL configuration %d", result);
    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to read the ACL configuration from platform memory
 *
 * @return
 *      - true in case of success
 *      - false in case of failure
 */
//--------------------------------------------------------------------------------------------------
static bool LoadAclConfiguration
(
    ConfigAclFile_t* aclConfigPtr           ///< [INOUT] ACL Configuration
)
{
    lwm2mcore_Sid_t sid;
    uint32_t lenWritten = 0;
    uint32_t loop;
    uint32_t loop2;
    uint32_t fileSize;
    size_t fileReadSize = 0;
    size_t len = sizeof(fileSize);
    AclObjectInstance_t* aclObjectInstanceListPtr;
    uint8_t* rawData;

    /* Get the ACL configuration file size */
    sid = lwm2mcore_GetParam(LWM2MCORE_ACCESS_RIGHTS_SIZE_PARAM, (uint8_t*)&fileSize, &len);
    LOG_ARG("Get ACL configuration size: %d result %d, len %d", fileSize, sid, len);
    if ((LWM2MCORE_ERR_COMPLETED_OK != sid) || (!fileSize))
    {
        /* Set a default configuration */
        SetDefaultAclConfiguration(aclConfigPtr);
        StoreAclConfiguration();
        return false;
    }

    rawData = (uint8_t*)lwm2m_malloc(fileSize);
    LWM2MCORE_ASSERT(rawData);
    fileReadSize = (size_t)fileSize;
    /* Get the ACL information file */
    sid = lwm2mcore_GetParam(LWM2MCORE_ACCESS_RIGHTS_PARAM, rawData, &fileReadSize);
    LOG_ARG("Read ACL configuration: len %ld result %d", fileReadSize, sid);

    if ((LWM2MCORE_ERR_COMPLETED_OK != sid)
     || (fileSize != fileReadSize))
    {
        lwm2m_free(rawData);
        /* Set a default configuration */
        SetDefaultAclConfiguration(aclConfigPtr);
        StoreAclConfiguration();
        return false;
    }

    if (fileSize < (sizeof(aclConfigPtr->version) + sizeof(aclConfigPtr->instanceNumber)))
    {
        lwm2m_free(rawData);
        return false;
    }

    /* Copy the version */
    aclConfigPtr->version = (uint32_t)(((ConfigAclFile_t*)((void*)rawData))->version);
    lenWritten += sizeof(aclConfigPtr->version);

    /* Copy the number of object instance of object 2 */
    aclConfigPtr->instanceNumber = (uint16_t)(((ConfigAclFile_t*)((void*)rawData))->instanceNumber);
    lenWritten += sizeof(aclConfigPtr->instanceNumber);

    LOG_ARG("Object 2: number of object instances: %d", aclConfigPtr->instanceNumber);

    /* Allocate object instances and copy related data */
    for (loop = 0; loop < (aclConfigPtr->instanceNumber); loop++)
    {
        if (fileSize >= (lenWritten + sizeof(AclObjectInstanceStorage_t)))
        {
            aclObjectInstanceListPtr = (AclObjectInstance_t*)lwm2m_malloc(sizeof(AclObjectInstance_t));
            LWM2MCORE_ASSERT(aclObjectInstanceListPtr);
            memset(aclObjectInstanceListPtr, 0, sizeof(AclObjectInstance_t));
            aclObjectInstanceListPtr->nextPtr = NULL;

            /* Copy the object instance data */
            memcpy(aclObjectInstanceListPtr,
                   rawData + lenWritten,
                   sizeof(AclObjectInstanceStorage_t));
            lenWritten += sizeof(AclObjectInstanceStorage_t);

            LOG_ARG("/2/%d: oid %d, oiid %d, owner %d, ACL resource instance nb %d",
                    aclObjectInstanceListPtr->aclObjectData.objInstId,
                    aclObjectInstanceListPtr->aclObjectData.objectId,
                    aclObjectInstanceListPtr->aclObjectData.objectInstId,
                    aclObjectInstanceListPtr->aclObjectData.aclOwner,
                    aclObjectInstanceListPtr->aclObjectData.aclInstanceNumber);

            omanager_AddAclObjectInstance(aclConfigPtr, aclObjectInstanceListPtr);
        }
        else
        {
            lwm2m_free(rawData);
            omanager_FreeAclConfiguration();
            return false;
        }

        for (loop2 = 0;
             loop2 < (aclObjectInstanceListPtr->aclObjectData.aclInstanceNumber);
             loop2++)
        {
            if (fileSize >= (lenWritten + sizeof(AclStorage_t)))
            {
                /* Allocate Acl resource instances list */
                Acl_t* aclListPtr = (Acl_t*)lwm2m_malloc(sizeof(Acl_t));
                LWM2MCORE_ASSERT(aclListPtr);
                memset(aclListPtr, 0, sizeof(Acl_t));
                aclListPtr->nextPtr = NULL;

                /* Copy the server Id (resource instance Id and access rights */
                memcpy(aclListPtr, rawData + lenWritten, sizeof(AclStorage_t));
                lenWritten += sizeof(AclStorage_t);

                LOG_ARG("ACL server Id %d access 0x%x",
                        aclListPtr->acl.resInstId,
                        aclListPtr->acl.accCtrlValue);

                omanager_AddAclAccessRights(aclObjectInstanceListPtr, aclListPtr);
            }
            else
            {
                lwm2m_free(rawData);
                omanager_FreeAclConfiguration();
                return false;
            }
        }
    }

    lwm2m_free(rawData);

    if (ACL_CONFIG_VERSION == aclConfigPtr->version)
    {
        return true;
    }

    /* Delete file if necessary and copy the default config */
    LOG_ARG("Failed to read the ACL configuration: read result %d, len %d", sid, len);
    if (len)
    {
        /* The file is present but the size is not correct or the version is not correct
         * Delete it
         */
        LOG("Delete ACL configuration");
        sid = lwm2mcore_DeleteParam(LWM2MCORE_ACCESS_RIGHTS_PARAM);
        if (LWM2MCORE_ERR_COMPLETED_OK != sid)
        {
            LOG("Failed to delete ACL configuration parameter");
        }

        sid = lwm2mcore_DeleteParam(LWM2MCORE_ACCESS_RIGHTS_SIZE_PARAM);
        if (LWM2MCORE_ERR_COMPLETED_OK != sid)
        {
            LOG("Failed to delete ACL configuration size parameter");
        }
    }

    /* Set a default configuration */
    SetDefaultAclConfiguration(aclConfigPtr);
    StoreAclConfiguration();

    return false;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to get the ACL from RAM
 *
 * @return
 *  - ACL configuration structure pointer
 */
//--------------------------------------------------------------------------------------------------
ConfigAclFile_t* omanager_GetAclConfiguration
(
    void
)
{
    return &AclConfigList;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to add an object instance of object 2 (ACL) in ACL configuration list
 */
//--------------------------------------------------------------------------------------------------
void omanager_AddAclObjectInstance
(
    ConfigAclFile_t*        aclConfigPtr,           ///< [IN] ACL Configuration
    AclObjectInstance_t*    aclObjectInstancePtr    ///< [IN] Object instance
)
{
    if ((!aclConfigPtr) || (!aclObjectInstancePtr))
    {
        return;
    }

    if (!aclConfigPtr->aclObjectInstanceListPtr)
    {
        aclConfigPtr->aclObjectInstanceListPtr = aclObjectInstancePtr;
        aclConfigPtr->aclObjectInstanceListPtr->nextPtr = NULL;
    }
    else
    {
        AclObjectInstance_t* tempPtr = aclConfigPtr->aclObjectInstanceListPtr;

        while (tempPtr->nextPtr)
        {
            tempPtr = tempPtr->nextPtr;
        }
        tempPtr->nextPtr = aclObjectInstancePtr;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to remove an object instance in object 2 (ACL)
 */
//--------------------------------------------------------------------------------------------------
void omanager_RemoveAclObjectInstance
(
    uint16_t    oiid        ///< [IN] Object instance Id
)
{
    AclObjectInstance_t* aclObjectInstancePtr = AclConfigList.aclObjectInstanceListPtr;
    AclObjectInstance_t* aclObjectInstanceTargetPtr = NULL;

    LOG_ARG("omanager_RemoveAclObjectInstance /2/%d", oiid);
    LOG_ARG("ACL object instance Number %d", AclConfigList.instanceNumber);

    if( aclObjectInstancePtr )
    {
        aclObjectInstanceTargetPtr = aclObjectInstancePtr->nextPtr;
        if ( (aclObjectInstancePtr->aclObjectData.objInstId) == oiid )
        {
            DeleteObjectInstance(&aclObjectInstancePtr);
            AclConfigList.aclObjectInstanceListPtr = aclObjectInstanceTargetPtr;
            AclConfigList.instanceNumber--;
        }
    }

    while (aclObjectInstanceTargetPtr)
    {
        if ((aclObjectInstanceTargetPtr->aclObjectData.objInstId) == oiid)
        {
            if(aclObjectInstancePtr)
            {
                aclObjectInstancePtr->nextPtr = aclObjectInstanceTargetPtr->nextPtr;
            }
            DeleteObjectInstance(&aclObjectInstanceTargetPtr);
            AclConfigList.instanceNumber--;
        }
        else
        {
            aclObjectInstanceTargetPtr = aclObjectInstanceTargetPtr->nextPtr;
            if(aclObjectInstancePtr)
            {
                aclObjectInstancePtr = (AclObjectInstance_t*)aclObjectInstancePtr->nextPtr;
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to remove object instance in object 2 (ACL) for a specific object Id, object instance Id
 */
//--------------------------------------------------------------------------------------------------
void omanager_RemoveAclForOidOiid
(
    uint16_t    oid,        ///< [IN] Object Id
    uint16_t    oiid        ///< [IN] Object instance Id
)
{
    AclObjectInstance_t* aclObjectInstancePtr = AclConfigList.aclObjectInstanceListPtr;
    Acl_t*               aclPtr;

    while (aclObjectInstancePtr)
    {
        if ( ( (aclObjectInstancePtr->aclObjectData.objectId) != oid)
          && ( (aclObjectInstancePtr->aclObjectData.objectInstId) != oiid) )
        {
            /* Remove the object instance */
            AclObjectInstance_t* nextPtr = aclObjectInstancePtr->nextPtr;
            aclPtr = aclObjectInstancePtr->aclListPtr;

            while (NULL != aclPtr)
            {
                Acl_t* nextAclPtr = aclPtr->nextPtr;
                lwm2m_free(aclPtr);
                aclPtr = nextAclPtr;
            }

            lwm2m_free(aclObjectInstancePtr);
            aclObjectInstancePtr = nextPtr;
        }
        else
        {
            aclObjectInstancePtr = aclObjectInstancePtr->nextPtr;
        }
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to get the ACL resource instance for a specific object instance of object 2 for a
 * specific resource instance Id (server Id)
 *
 * @return
 *      - pointer on resource instance
 *      - NULL on failure or if the resource instance is not present
 */
//--------------------------------------------------------------------------------------------------
Acl_t* omanager_GetAclFromAclOiidAndRiid
(
    AclObjectInstance_t*    aclObjectInstancePtr,   ///< [IN] Object instance Id
    uint16_t                resourceInstanceId      ///< [IN] Resource instance Id
)
{
    Acl_t* aclRiidPtr;
    if (!aclObjectInstancePtr)
    {
        return NULL;
    }

    aclRiidPtr = aclObjectInstancePtr->aclListPtr;

    while (aclRiidPtr && (aclRiidPtr->acl.resInstId != resourceInstanceId))
    {
        aclRiidPtr = aclRiidPtr->nextPtr;
    }

    if (!aclRiidPtr)
    {
        return NULL;
    }

    return aclRiidPtr;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to get the ACL configuration for a specific object instance Id of object 2 (ACL)
 *
 * @return
 *  - pointer on object instance structure on success
 *  - NULL if the object instance Id does not exist
 */
//--------------------------------------------------------------------------------------------------
AclObjectInstance_t* omanager_GetAclObjectInstance
(
    ConfigAclFile_t*    aclConfigPtr,       ///< [IN] ACL configuration
    uint16_t            objectInstanceId    ///< [IN] Object instance Id
)
{
    AclObjectInstance_t* AclObjectInstancePtr;

    if (!aclConfigPtr)
    {
        return NULL;
    }

    AclObjectInstancePtr = aclConfigPtr->aclObjectInstanceListPtr;

    while ( (AclObjectInstancePtr)
       && ( (AclObjectInstancePtr->aclObjectData.objInstId) != objectInstanceId))
    {
        AclObjectInstancePtr = AclObjectInstancePtr->nextPtr;
    }
    return AclObjectInstancePtr;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to get the ACL value for a ACL resource instance of a specific object instance Id of
 * object 2 (ACL)
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t omanager_GetAclValueFromResourceInstance
(
    AclObjectInstance_t*    aclOiidPtr,             ///< [IN] ACL object instance pointer
    uint16_t*               resourceInstanceIdPtr,  ///< [IN] Object instance Id
    uint16_t*               aclValuePtr             ///< [IN] ACL value
)
{
    /* Resource instance Id for ACL resource is linked to server Id.
     * According to LwM2M specification, the range for server ID [1-65535].
     * In order to not check all 65535 possibilities, the resource instance Id is overwritten in
     * this function.
     * If *resourceInstanceIdPtr = 0, this function will read the 1st value ot ACL in aclOiidPtr.
     * If *resourceInstanceIdPtr = 1, this function will read the 2nd value ot ACL in aclOiidPtr.
     * If *resourceInstanceIdPtr = 2, this function will read the 3rd value ot ACL in aclOiidPtr.
     * etc...
     * If ACL is present, this function returns LWM2MCORE_ERR_COMPLETED_OK, else
     * LWM2MCORE_ERR_GENERAL_ERROR is returned.
     * Example:
     * One object instance ID of object 2 gets 2 resources instances for resource 2 (ACL):
     * 1st resource instance has instance ID = 1
     * 2nd resource instance has instance ID = 1000
     * This function will be called 3 times:
     * 1) 1st time: *resourceInstanceIdPtr = 0, overwritten by 1 for resource instance Id = 1
     * 2) 2nd time: *resourceInstanceIdPtr = 1, overwritten by 1000 for resource instance Id = 1000
     * 3) 3rd time: *resourceInstanceIdPtr = 2, LWM2MCORE_ERR_INVALID_ARG is returned
     */
    Acl_t* aclPtr;
    uint16_t loop;

    if ((!aclOiidPtr) || (!resourceInstanceIdPtr) || (!aclValuePtr))
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    aclPtr = aclOiidPtr->aclListPtr;
    for (loop = 0; loop < (*resourceInstanceIdPtr); loop++)
    {
        aclPtr = aclPtr->nextPtr;
    }

    if (!aclPtr)
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    *aclValuePtr = aclPtr->acl.accCtrlValue;
    *resourceInstanceIdPtr = aclPtr->acl.resInstId;

    return LWM2MCORE_ERR_COMPLETED_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to add access rights list in an object instance of object 2
 */
//--------------------------------------------------------------------------------------------------
void omanager_AddAclAccessRights
(
    AclObjectInstance_t*    aclObjectInstancePtr,   ///< [IN] Object instance
    Acl_t*                  aclPtr                  ///< [IN] ACL
)
{
    if ((!aclObjectInstancePtr) || (!aclPtr))
    {
        return;
    }

    LOG_ARG("Add ACL: resource Instance Id %d, rights 0x%x",
            aclPtr->acl.resInstId, aclPtr->acl.accCtrlValue);

    if (!aclObjectInstancePtr->aclListPtr)
    {
        aclObjectInstancePtr->aclListPtr = aclPtr;
        aclObjectInstancePtr->aclListPtr->nextPtr = NULL;
    }
    else
    {
        Acl_t* tempPtr = aclObjectInstancePtr->aclListPtr;

        while (tempPtr->nextPtr)
        {
            tempPtr = tempPtr->nextPtr;
        }
        tempPtr->nextPtr = aclPtr;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to indicate how many object instances are defined in object 2 (ACL)
 *
 * @return
 *      - object instance number
 */
//--------------------------------------------------------------------------------------------------
uint16_t omanager_GetObject2InstanceNumber
(
    void
)
{
    return AclConfigList.instanceNumber;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to indicate how many resource instances are defined in a specific object instance of
 * object 2 (ACL)
 *
 * @return
 *      - resource instance number
 */
//--------------------------------------------------------------------------------------------------
uint16_t omanager_GetAclInstanceNumber
(
    uint16_t    oiid    ///< [IN] Object instance of object 2
)
{
    ConfigAclFile_t* AclConfigPtr;
    AclObjectInstance_t* aclOiidPtr;

    AclConfigPtr = omanager_GetAclConfiguration();
    if (!AclConfigPtr)
    {
        return 0;
    }
    aclOiidPtr = omanager_GetAclObjectInstance(AclConfigPtr, oiid);
    if (!aclOiidPtr)
    {
        return 0;
    }

    return aclOiidPtr->aclObjectData.aclInstanceNumber;
}

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to read the ACL configuration from platform memory
 *
 * @return
 *      - @c true in case of success
 *      - @c false in case of failure
 */
//--------------------------------------------------------------------------------------------------
bool omanager_LoadAclConfiguration
(
    void
)
{
    return LoadAclConfiguration(&AclConfigList);
}

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to write the ACL configuration in platform memory
 *
 * @return
 *      - @c true in case of success
 *      - @c false in case of failure
 */
//--------------------------------------------------------------------------------------------------
bool omanager_StoreAclConfiguration
(
    void
)
{
    return StoreAclConfiguration();
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to free the ACL configuration list
 */
//--------------------------------------------------------------------------------------------------
void omanager_FreeAclConfiguration
(
    void
)
{
    FreeAclConfiguration(&AclConfigList);
}
