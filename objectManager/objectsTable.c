/**
 * @file objectsTable.c
 *
 * Object table and resource supported by the client
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

/* include files */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <lwm2mcore/lwm2mcore.h>
#include "objects.h"
#include "handlers.h"

//--------------------------------------------------------------------------------------------------
/**
 * This file is used to indicate which objects are supported by the client.
 * In lwm2mcore_handlers table, the following parameters are indicated
 *  - objCnt: number of supported LWM2M objects (standard + proprietary)
 *  - objects: supported  LWM2M objects (standard + proprietary) table
 *             This table includes the supported objects: see ObjArray table.
 *             For each object, the supported resources needs to be indicated.
 *  - genericUOHandler: callback for not supported LWM2M objects (standard + proprietary)
 */
//--------------------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------------------
/**
 * Security_resources supported resources defined for LWM2M security object.
 * For each resource, the resource Id, the resource type, the resource instance number,
 * a READ, WRITE, EXEC callback can be defined.
 */
//--------------------------------------------------------------------------------------------------
static lwm2mcore_Resource_t SecurityResources[] =
{
    {
        LWM2MCORE_SECURITY_SERVER_URI_RID,          //.id
        LWM2MCORE_RESOURCE_TYPE_STRING,             //.type
        1,                                          //.maxResInstCnt
        ReadSecurityObj,                            //.read
        WriteSecurityObj,                           //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_SECURITY_BOOTSTRAP_SERVER_RID,    //.id
        LWM2MCORE_RESOURCE_TYPE_BOOL,               //.type
        1,                                          //.maxResInstCnt
        ReadSecurityObj,                            //.read
        WriteSecurityObj,                           //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_SECURITY_MODE_RID,                //.id
        LWM2MCORE_RESOURCE_TYPE_INT,                //.type
        1,                                          //.maxResInstCnt
        ReadSecurityObj,                            //.read
        WriteSecurityObj,                           //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_SECURITY_PKID_RID,                //.id
        LWM2MCORE_RESOURCE_TYPE_OPAQUE,             //.type
        1,                                          //.maxResInstCnt
        ReadSecurityObj,                            //.read
        WriteSecurityObj,                           //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_SECURITY_SERVER_KEY_RID,          //.id
        LWM2MCORE_RESOURCE_TYPE_OPAQUE,             //.type
        1,                                          //.maxResInstCnt
        ReadSecurityObj,                            //.read
        WriteSecurityObj,                           //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_SECURITY_SECRET_KEY_RID,          //.id
        LWM2MCORE_RESOURCE_TYPE_OPAQUE,             //.type
        1,                                          //.maxResInstCnt
        ReadSecurityObj,                            //.read
        WriteSecurityObj,                           //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_SECURITY_SMS_SECURITY_MODE_RID,   //.id
        LWM2MCORE_RESOURCE_TYPE_INT,                //.type
        1,                                          //.maxResInstCnt
        NULL,                                       //.read
        SmsDummy,                                   //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_SECURITY_SMS_BINDING_KEY_PAR_RID, //.id
        LWM2MCORE_RESOURCE_TYPE_OPAQUE,             //.type
        1,                                          //.maxResInstCnt
        NULL,                                       //.read
        SmsDummy,                                   //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_SECURITY_SMS_BINDING_SEC_KEY_RID, //.id
        LWM2MCORE_RESOURCE_TYPE_OPAQUE,             //.type
        1,                                          //.maxResInstCnt
        NULL,                                       //.read
        SmsDummy,                                   //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_SECURITY_SERVER_SMS_NUMBER_RID,   //.id
        LWM2MCORE_RESOURCE_TYPE_INT,                //.type
        1,                                          //.maxResInstCnt
        NULL,                                       //.read
        SmsDummy,                                   //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_SECURITY_SERVER_ID_RID,           //.id
        LWM2MCORE_RESOURCE_TYPE_INT,                //.type
        1,                                          //.maxResInstCnt
        ReadSecurityObj,                            //.read
        WriteSecurityObj,                           //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_SECURITY_CLIENT_HOLD_OFF_TIME_RID, //.id
        LWM2MCORE_RESOURCE_TYPE_INT,                //.type
        1,                                          //.maxResInstCnt
        ReadSecurityObj,                            //.read
        WriteSecurityObj,                           //.write
        NULL,                                       //.exec
    }
};

//--------------------------------------------------------------------------------------------------
/**
 * Server_resources supported resources defined for LWM2M server object.
 * For each resource, the resource Id, the resource type, the resource instance number,
 * a READ, WRITE, EXEC callback can be defined.
 */
//--------------------------------------------------------------------------------------------------
static lwm2mcore_Resource_t ServerResources[] =
{
    {
        LWM2MCORE_SERVER_SHORT_ID_RID,              //.id
        LWM2MCORE_RESOURCE_TYPE_INT,                //.type
        1,                                          //.maxResInstCnt
        ReadServerObj,                              //.read
        WriteServerObj,                             //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_SERVER_LIFETIME_RID,              //.id
        LWM2MCORE_RESOURCE_TYPE_INT,                //.type
        1,                                          //.maxResInstCnt
        ReadServerObj,                              //.read
        WriteServerObj,                             //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_SERVER_DEFAULT_MIN_PERIOD_RID,    //.id
        LWM2MCORE_RESOURCE_TYPE_INT,                //.type
        1,                                          //.maxResInstCnt
        ReadServerObj,                              //.read
        WriteServerObj,                             //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_SERVER_DEFAULT_MAX_PERIOD_RID,    //.id
        LWM2MCORE_RESOURCE_TYPE_INT,                //.type
        1,                                          //.maxResInstCnt
        ReadServerObj,                              //.read
        WriteServerObj,                             //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_SERVER_STORE_NOTIF_WHEN_OFFLINE_RID, //.id
        LWM2MCORE_RESOURCE_TYPE_BOOL,                  //.type
        1,                                          //.maxResInstCnt
        ReadServerObj,                              //.read
        WriteServerObj,                             //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_SERVER_BINDING_MODE_RID,          //.id
        LWM2MCORE_RESOURCE_TYPE_STRING,             //.type
        1,                                          //.maxResInstCnt
        ReadServerObj,                              //.read
        WriteServerObj,                             //.write
        NULL,                                       //.exec
    }
};

//--------------------------------------------------------------------------------------------------
/**
 * Device_resources supported resources defined for LWM2M device object.
 * For each resource, the resource Id, the resource type, the resource instance number,
 * a READ, WRITE, EXEC callback can be defined.
 */
//--------------------------------------------------------------------------------------------------
static lwm2mcore_Resource_t DeviceResources[] =
{
    {
        LWM2MCORE_DEVICE_MANUFACTURER_RID,          //.id
        LWM2MCORE_RESOURCE_TYPE_STRING,             //.type
        1,                                          //.maxResInstCnt
        ReadDeviceObj,                              //.read
        NULL,                                       //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_DEVICE_MODEL_NUMBER_RID,          //.id
        LWM2MCORE_RESOURCE_TYPE_STRING,             //.type
        1,                                          //.maxResInstCnt
        ReadDeviceObj,                              //.read
        NULL,                                       //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_DEVICE_SERIAL_NUMBER_RID,         //.id
        LWM2MCORE_RESOURCE_TYPE_STRING,             //.type
        1,                                          //.maxResInstCnt
        ReadDeviceObj,                              //.read
        NULL,                                       //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_DEVICE_FIRMWARE_VERSION_RID,      //.id
        LWM2MCORE_RESOURCE_TYPE_STRING,             //.type
        1,                                          //.maxResInstCnt
        ReadDeviceObj,                              //.read
        NULL,                                       //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_DEVICE_BATTERY_LEVEL_RID,         //.id
        LWM2MCORE_RESOURCE_TYPE_INT,                //.type
        1,                                          //.maxResInstCnt
        ReadDeviceObj,                              //.read
        NULL,                                       //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_DEVICE_CURRENT_TIME_RID,          //.id
        LWM2MCORE_RESOURCE_TYPE_TIME,               //.type
        1,                                          //.maxResInstCnt
        ReadDeviceObj,                              //.read
        WriteDeviceObj,                             //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_DEVICE_SUPPORTED_BINDING_MODE_RID, //.id
        LWM2MCORE_RESOURCE_TYPE_STRING,              //.type
        1,                                          //.maxResInstCnt
        ReadDeviceObj,                              //.read
        WriteDeviceObj,                             //.write
        NULL,                                       //.exec
    }
};


//--------------------------------------------------------------------------------------------------
/**
 * Firmware update supported resources defined for LWM2M object (5)
 * For each resource, the resource Id, the resource type, the resource instance number,
 * a READ, WRITE, EXEC callback can be defined.
 */
//--------------------------------------------------------------------------------------------------
static lwm2mcore_Resource_t FirmwareUpdateResources[] =
{
    {
        LWM2MCORE_FW_UPDATE_PACKAGE_RID,            //.id
        LWM2MCORE_RESOURCE_TYPE_OPAQUE,             //.type
        1,                                          //.maxResInstCnt
        NULL,                                       //.read
        WriteFwUpdateObj,                           //.write
        NULL                                        //.exec
    },
    {
        LWM2MCORE_FW_UPDATE_PACKAGE_URI_RID,        //.id
        LWM2MCORE_RESOURCE_TYPE_STRING,             //.type
        1,                                          //.maxResInstCnt
        ReadFwUpdateObj,                            //.read
        WriteFwUpdateObj,                           //.write
        NULL                                        //.exec
    },
    {
        LWM2MCORE_FW_UPDATE_UPDATE_RID,             //.id
        LWM2MCORE_RESOURCE_TYPE_UNKNOWN,            //.type
        1,                                          //.maxResInstCnt
        NULL,                                       //.read
        NULL,                                       //.write
        ExecFwUpdate,                               //.exec
    },
    {
        LWM2MCORE_FW_UPDATE_UPDATE_STATE_RID,       //.id
        LWM2MCORE_RESOURCE_TYPE_INT,                //.type
        1,                                          //.maxResInstCnt
        ReadFwUpdateObj,                            //.read
        NULL,                                       //.write
        NULL                                        //.exec
    },
    {
        LWM2MCORE_FW_UPDATE_UPDATE_RESULT_RID,      //.id
        LWM2MCORE_RESOURCE_TYPE_INT,                //.type
        1,                                          //.maxResInstCnt
        ReadFwUpdateObj,                            //.read
        NULL,                                       //.write
        NULL                                        //.exec
    }
};

//--------------------------------------------------------------------------------------------------
/**
 * SoftwareResources: supported resources defined for LWM2M software update object (9)
 * For each resource, the resource Id, the resource type, the resource instance number,
 * a READ, WRITE, EXEC callback can be defined.
 */
//--------------------------------------------------------------------------------------------------
static lwm2mcore_Resource_t SoftwareUpdateResources[] =
{
    {
        LWM2MCORE_SW_UPDATE_PACKAGE_NAME_RID,       //.id
        LWM2MCORE_RESOURCE_TYPE_STRING,             //.type
        1,                                          //.maxResInstCnt
        ReadSwUpdateObj,                            //.read
        NULL,                                       //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_SW_UPDATE_PACKAGE_VERSION_RID,    //.id
        LWM2MCORE_RESOURCE_TYPE_STRING,             //.type
        1,                                          //.maxResInstCnt
        ReadSwUpdateObj,                            //.read
        NULL,                                       //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_SW_UPDATE_PACKAGE_URI_RID,        //.id
        LWM2MCORE_RESOURCE_TYPE_STRING,             //.type
        1,                                          //.maxResInstCnt
        NULL,                                       //.read
        WriteSwUpdateObj,                           //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_SW_UPDATE_INSTALL_RID,            //.id
        LWM2MCORE_RESOURCE_TYPE_UNKNOWN,            //.type
        1,                                          //.maxResInstCnt
        NULL,                                       //.read
        NULL,                                       //.write
        ExecSwUpdate,                               //.exec
    },
    {
        LWM2MCORE_SW_UPDATE_UNINSTALL_RID,          //.id
        LWM2MCORE_RESOURCE_TYPE_UNKNOWN,            //.type
        1,                                          //.maxResInstCnt
        NULL,                                       //.read
        NULL,                                       //.write
        ExecSwUpdate,                               //.exec
    },
    {
        LWM2MCORE_SW_UPDATE_UPDATE_STATE_RID,       //.id
        LWM2MCORE_RESOURCE_TYPE_INT,                //.type
        1,                                          //.maxResInstCnt
        ReadSwUpdateObj,                            //.read
        NULL,                                       //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_SW_UPDATE_UPDATE_SUPPORTED_OBJ_RID, //.id
        LWM2MCORE_RESOURCE_TYPE_BOOL,               //.type
        1,                                          //.maxResInstCnt
        ReadSwUpdateObj,                            //.read
        WriteSwUpdateObj,                           //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_SW_UPDATE_UPDATE_RESULT_RID,      //.id
        LWM2MCORE_RESOURCE_TYPE_INT,                //.type
        1,                                          //.maxResInstCnt
        ReadSwUpdateObj,                            //.read
        NULL,                                       //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_SW_UPDATE_ACTIVATE_RID,           //.id
        LWM2MCORE_RESOURCE_TYPE_UNKNOWN,            //.type
        1,                                          //.maxResInstCnt
        NULL,                                       //.read
        NULL,                                       //.write
        ExecSwUpdate,                               //.exec
    },
    {
        LWM2MCORE_SW_UPDATE_DEACTIVATE_RID,         //.id
        LWM2MCORE_RESOURCE_TYPE_UNKNOWN,            //.type
        1,                                          //.maxResInstCnt
        NULL,                                       //.read
        NULL,                                       //.write
        ExecSwUpdate,                               //.exec
    },
    {
        LWM2MCORE_SW_UPDATE_ACTIVATION_STATE_RID,   //.id
        LWM2MCORE_RESOURCE_TYPE_INT,                //.type
        1,                                          //.maxResInstCnt
        ReadSwUpdateObj,                            //.read
        NULL,                                       //.write
        NULL,                                       //.exec
    }
};

//--------------------------------------------------------------------------------------------------
/**
 * SSL certificate supported resources defined for LWM2M certificate object (10243)
 * For each resource, the resource Id, the resource type, the resource instance number,
 * a READ, WRITE, EXEC callback can be defined.
 */
//--------------------------------------------------------------------------------------------------
static lwm2mcore_Resource_t SslCertificateResources[] =
{
    {
        LWM2MCORE_SSL_CERTIFICATE_CERTIF,           //.id
        LWM2MCORE_RESOURCE_TYPE_OPAQUE,             //.type
        1,                                          //.maxResInstCnt
        OnSslCertif,                                //.read
        OnSslCertif,                                //.write
        NULL,                                       //.exec
    }
};

//--------------------------------------------------------------------------------------------------
/**
 * List of objects which are supported by the client
 *
 * For each object, the following parameters needs to be filled:
 *  - id: the object Id
 *  - maxObjInstCnt: maximum object instance number
 *  - resCnt: resources number which are supported for this object
 *  - resources: supported resources table
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Object_t ObjArray[] =
{
    /* object 0, LWM2M security */
    {
        LWM2MCORE_SECURITY_OID,                                                 //.id
        LWM2MCORE_DM_SERVER_MAX_COUNT + LWM2MCORE_BOOTSRAP_SERVER_MAX_COUNT,    //..maxObjInstCnt
        ARRAYSIZE(SecurityResources),                                           //.resCnt
        SecurityResources                                                       //.resources
    },
    /* object 1, LWM2M DM server */
    {
        LWM2MCORE_SERVER_OID,                                                   //.id
        LWM2MCORE_DM_SERVER_MAX_COUNT,                                          //.maxObjInstCnt
        ARRAYSIZE(ServerResources),                                             //.resCnt
        ServerResources                                                         //.resources
    },
    /* object 3, device */
    {
        LWM2MCORE_DEVICE_OID,                                                   //.id
        1,                                                                      //.maxObjInstCnt
        ARRAYSIZE(DeviceResources),                                             //.resCnt
        DeviceResources                                                         //.resources
    },
    /* object 5, firmware update */
    {
        LWM2MCORE_FIRMWARE_UPDATE_OID,                                          //.id
        1,                                                                      //.maxObjInstCnt
        ARRAYSIZE(FirmwareUpdateResources),                                     //.resCnt
        FirmwareUpdateResources                                                 //.resources
    },
    /* object 9, software update */
    {
        LWM2MCORE_SOFTWARE_UPDATE_OID,                                          //.id
        LWM2MCORE_ID_NONE,                                                      //.maxObjInstCnt
        ARRAYSIZE(SoftwareUpdateResources),                                     //.resCnt
        SoftwareUpdateResources                                                 //.resources
    },
    /* object 10243, SSL certificate */
    {
        LWM2M_SWI_SSL_CERTIFS_OID,                                              //.id
        1,                                                                      //.maxObjInstCnt
        ARRAYSIZE(SslCertificateResources),                                     //.resCnt
        SslCertificateResources                                                 //.resources
    }
};

//--------------------------------------------------------------------------------------------------
/**
 * Handler indicating the supported objects list and default callback for not registered objects
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Handler_t Lwm2mcoreHandlers =
{
    ARRAYSIZE(ObjArray),                        //.objCnt
    ObjArray,                                   //.objects
    OnUnlistedObject                            //.genericUOHandler
};

