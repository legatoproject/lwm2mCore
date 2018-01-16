/**
 * @file aclConfiguration.h
 *
 * ACL management header
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#ifndef __ACL_CONFIG_H__
#define __ACL_CONFIG_H__

//--------------------------------------------------------------------------------------------------
/**
 * Supported version for ACL file
 */
//--------------------------------------------------------------------------------------------------
#define ACL_CONFIG_VERSION          1

//--------------------------------------------------------------------------------------------------
/**
 * Structure for ACL storage in platform
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    uint16_t         resInstId;         ///< Resource instance number = server Id
    uint16_t         accCtrlValue;      ///< ACL
}
AclStorage_t;

//--------------------------------------------------------------------------------------------------
/**
 * Structure for ACL definition
 */
//--------------------------------------------------------------------------------------------------
typedef struct _Acl_t
{
    AclStorage_t     acl;               ///< ACL data
    struct _Acl_t*   nextPtr;           ///< Next entry in the list
}
Acl_t;

//--------------------------------------------------------------------------------------------------
/**
 * Structure for one object instance of object 2 (ACL) for platform storage
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    uint16_t                        objInstId;          ///< Object instance Id of object 2
    uint16_t                        objectId;           ///< Object Id on which ACL applies
    uint16_t                        objectInstId;       ///< Object instance Id on which ACL applies
    uint16_t                        aclOwner;           ///< ACL owner
    uint16_t                        aclInstanceNumber;  ///< ACL resource instance number
}
AclObjectInstanceStorage_t;

//--------------------------------------------------------------------------------------------------
/**
 * Structure for one object instance of object 2 (ACL)
 */
//--------------------------------------------------------------------------------------------------
typedef struct _AclObjectInstance_t
{
    AclObjectInstanceStorage_t      aclObjectData;      ///< ACL Object data
    Acl_t*                          aclListPtr;         ///< ACL list
    struct _AclObjectInstance_t*    nextPtr;            ///< Next entry in the list
}
AclObjectInstance_t;

//--------------------------------------------------------------------------------------------------
/**
 * Structure for ACL configuration (object 2) to be stored in platform storage
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    uint32_t                version;                    ///< File version
    uint16_t                instanceNumber;             ///< Object instance number
    AclObjectInstance_t*    aclObjectInstanceListPtr;   ///< Object instance list
}
ConfigAclFile_t;

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
);

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
);

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
);

//--------------------------------------------------------------------------------------------------
/**
 * Function to add an object instance of object 2 (ACL) in ACL configuration list
 */
//--------------------------------------------------------------------------------------------------
void omanager_AddAclObjectInstance
(
    ConfigAclFile_t*        aclConfigPtr,           ///< [IN] ACL Configuration
    AclObjectInstance_t*    aclObjectInstancePtr    ///< [IN] Object instance
);

//--------------------------------------------------------------------------------------------------
/**
 * Function to remove object instance in object 2 (ACL) for a specific object Id, object instance Id
 */
//--------------------------------------------------------------------------------------------------
void omanager_RemoveAclForOidOiid
(
    uint16_t    oid,        ///< [IN] Object Id
    uint16_t    oiid        ///< [IN] Object instance Id
);

//--------------------------------------------------------------------------------------------------
/**
 * Function to remove an object instance in object 2 (ACL)
 */
//--------------------------------------------------------------------------------------------------
void omanager_RemoveAclObjectInstance
(
    uint16_t    oiid        ///< [IN] Object instance Id
);

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
    AclObjectInstance_t*    aclObjectInstancePtr,   ///< [IN] Object instance
    uint16_t                resourceInstanceId      ///< [IN] Resource instance Id
);

//--------------------------------------------------------------------------------------------------
/**
 * Function to add access rights list in an object instance of object 2
 */
//--------------------------------------------------------------------------------------------------
void omanager_AddAclAccessRights
(
    AclObjectInstance_t*    aclObjectInstancePtr,   ///< [IN] Object instance
    Acl_t*                  aclPtr                  ///< [IN] ACL
);

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
);

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
);

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
);

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
);

//--------------------------------------------------------------------------------------------------
/**
 * Function to free the ACL configuration list
 */
//--------------------------------------------------------------------------------------------------
void omanager_FreeAclConfiguration
(
    void
);

#endif /* __ACL_CONFIG_H__ */
