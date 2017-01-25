/**
 * @file lwm2mcoreObjectsTable.c
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
#include "lwm2mcore.h"
#include "../objectManager/lwm2mcoreObjects.h"
#include "../objectManager/lwm2mcoreHandlers.h"
#include "lwm2mcoreObjectHandler.h"


//--------------------------------------------------------------------------------------------------
/**
 * This file is used to indicate which objects are supported by the client.
 * In lwm2mcore_handlers table, the following parameters are indicated
 *  - obj_cnt: number of supported LWM2M objects (standard + proprietary)
 *  - objects: supported  LWM2M objects (standard + proprietary) table
 *             This table includes the supported objects: see ObjArray table.
 *             For each object, the supported resources needs to be indicated.
 *  - generic_UO_handler: callback for not supported LWM2M objects (standard + proprietary)
 */
//--------------------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------------------
/**
 * Security_resources supported resources defined for LWM2M security object.
 * For each resource, the resource Id, the resource type, the resource instance number,
 * a READ, WRITE, EXEC callback can be defined.
 */
//--------------------------------------------------------------------------------------------------
static lwm2mcore_resource_t SecurityResources[] =
{
    {
        .id = LWM2MCORE_SECURITY_SERVER_URI_RID,
        .type = LWM2MCORE_RESOURCE_TYPE_STRING,
        .max_res_inst_cnt = 1,
        .read = lwm2mcore_ReadSecurityObj,
        .write = lwm2mcore_WriteSecurityObj,
        .exec = NULL,
    },
    {
        .id = LWM2MCORE_SECURITY_BOOTSTRAP_SERVER_RID,
        .type = LWM2MCORE_RESOURCE_TYPE_BOOL,
        .max_res_inst_cnt = 1,
        .read = lwm2mcore_ReadSecurityObj,
        .write = lwm2mcore_WriteSecurityObj,
        .exec = NULL,
    },
    {
        .id = LWM2MCORE_SECURITY_MODE_RID,
        .type = LWM2MCORE_RESOURCE_TYPE_INT,
        .max_res_inst_cnt = 1,
        .read = lwm2mcore_ReadSecurityObj,
        .write = lwm2mcore_WriteSecurityObj,
        .exec = NULL,
    },
    {
        .id = LWM2MCORE_SECURITY_PKID_RID,
        .type = LWM2MCORE_RESOURCE_TYPE_OPAQUE,
        .max_res_inst_cnt = 1,
        .read = lwm2mcore_ReadSecurityObj,
        .write = lwm2mcore_WriteSecurityObj,
        .exec = NULL,
    },
    {
        .id = LWM2MCORE_SECURITY_SERVER_KEY_RID,
        .type = LWM2MCORE_RESOURCE_TYPE_OPAQUE,
        .max_res_inst_cnt = 1,
        .read = lwm2mcore_ReadSecurityObj,
        .write = lwm2mcore_WriteSecurityObj,
        .exec = NULL,
    },
    {
        .id = LWM2MCORE_SECURITY_SECRET_KEY_RID,
        .type = LWM2MCORE_RESOURCE_TYPE_OPAQUE,
        .max_res_inst_cnt = 1,
        .read = lwm2mcore_ReadSecurityObj,
        .write = lwm2mcore_WriteSecurityObj,
        .exec = NULL,
    },
    {
        .id = LWM2MCORE_SECURITY_SMS_SECURITY_MODE_RID,
        .type = LWM2MCORE_RESOURCE_TYPE_INT,
        .max_res_inst_cnt = 1,
        .read = NULL,   /* N/A */
        .write = OnLWM2MSecuritySMSDummy,
        .exec = NULL,
    },
    {
        .id = LWM2MCORE_SECURITY_SMS_BINDING_KEY_PAR_RID,
        .type = LWM2MCORE_RESOURCE_TYPE_OPAQUE,
        .max_res_inst_cnt = 1,
        .read = NULL,   /* N/A */
        .write = OnLWM2MSecuritySMSDummy,
        .exec = NULL,
    },
    {
        .id = LWM2MCORE_SECURITY_SMS_BINDING_SEC_KEY_RID,
        .type = LWM2MCORE_RESOURCE_TYPE_OPAQUE,
        .max_res_inst_cnt = 1,
        .read = NULL,   /* N/A */
        .write = OnLWM2MSecuritySMSDummy,
        .exec = NULL,
    },
    {
        .id = LWM2MCORE_SECURITY_SERVER_SMS_NUMBER_RID,
        .type = LWM2MCORE_RESOURCE_TYPE_INT,
        .max_res_inst_cnt = 1,
        .read = NULL,   /* N/A */
        .write = OnLWM2MSecuritySMSDummy,
        .exec = NULL,
    },
    {
        .id = LWM2MCORE_SECURITY_SERVER_ID_RID,
        .type = LWM2MCORE_RESOURCE_TYPE_INT,
        .max_res_inst_cnt = 1,
        .read = lwm2mcore_ReadSecurityObj,
        .write = lwm2mcore_WriteSecurityObj,
        .exec = NULL,
    },
    {
        .id = LWM2MCORE_SECURITY_CLIENT_HOLD_OFF_TIME_RID,
        .max_res_inst_cnt = 1,
        .type = LWM2MCORE_RESOURCE_TYPE_INT,
        .read = lwm2mcore_ReadSecurityObj,
        .write = lwm2mcore_WriteSecurityObj,
        .exec = NULL,
    }
};

//--------------------------------------------------------------------------------------------------
/**
 * Server_resources supported resources defined for LWM2M server object.
 * For each resource, the resource Id, the resource type, the resource instance number,
 * a READ, WRITE, EXEC callback can be defined.
 */
//--------------------------------------------------------------------------------------------------
static lwm2mcore_resource_t ServerResources[] =
{
    {
        .id = LWM2MCORE_SERVER_SHORT_ID_RID,
        .type = LWM2MCORE_RESOURCE_TYPE_INT,
        .max_res_inst_cnt = 1,
        .read = lwm2mcore_ReadServerObj,
        .write = lwm2mcore_WriteServerObj,
        .exec = NULL,
    },
    {
        .id = LWM2MCORE_SERVER_LIFETIME_RID,
        .type = LWM2MCORE_RESOURCE_TYPE_INT,
        .max_res_inst_cnt = 1,
        .read = lwm2mcore_ReadServerObj,
        .write = lwm2mcore_WriteServerObj,
        .exec = NULL,
    },
    {
        .id = LWM2MCORE_SERVER_DEFAULT_MIN_PERIOD_RID,
        .type = LWM2MCORE_RESOURCE_TYPE_INT,
        .max_res_inst_cnt = 1,
        .read = lwm2mcore_ReadServerObj,
        .write = lwm2mcore_WriteServerObj,
        .exec = NULL,
    },
    {
        .id = LWM2MCORE_SERVER_DEFAULT_MAX_PERIOD_RID,
        .type = LWM2MCORE_RESOURCE_TYPE_INT,
        .max_res_inst_cnt = 1,
        .read = lwm2mcore_ReadServerObj,
        .write = lwm2mcore_WriteServerObj,
        .exec = NULL,
    },
    {
        .id = LWM2MCORE_SERVER_STORE_NOTIF_WHEN_OFFLINE_RID,
        .max_res_inst_cnt = 1,
        .type = LWM2MCORE_RESOURCE_TYPE_BOOL,
        .read = lwm2mcore_ReadServerObj,
        .write = lwm2mcore_WriteServerObj,
        .exec = NULL,
    },
    {
        .id = LWM2MCORE_SERVER_BINDING_MODE_RID,
        .type = LWM2MCORE_RESOURCE_TYPE_STRING,
        .max_res_inst_cnt = 1,
        .read = lwm2mcore_ReadServerObj,
        .write = lwm2mcore_WriteServerObj,
        .exec = NULL,
    }
};

//--------------------------------------------------------------------------------------------------
/**
 * Device_resources supported resources defined for LWM2M device object.
 * For each resource, the resource Id, the resource type, the resource instance number,
 * a READ, WRITE, EXEC callback can be defined.
 */
//--------------------------------------------------------------------------------------------------
static lwm2mcore_resource_t DeviceResources[] =
{
    {
        .id = LWM2MCORE_DEVICE_MANUFACTURER_RID,
        .type = LWM2MCORE_RESOURCE_TYPE_STRING,
        .max_res_inst_cnt = 1,
        .read = lwm2mcore_ReadDeviceObj,
        .write = NULL,
        .exec = NULL,
    },
    {
        .id = LWM2MCORE_DEVICE_MODEL_NUMBER_RID,
        .type = LWM2MCORE_RESOURCE_TYPE_STRING,
        .max_res_inst_cnt = 1,
        .read = lwm2mcore_ReadDeviceObj,
        .write = NULL,
        .exec = NULL,
    },
    {
        .id = LWM2MCORE_DEVICE_SERIAL_NUMBER_RID,
        .type = LWM2MCORE_RESOURCE_TYPE_STRING,
        .max_res_inst_cnt = 1,
        .read = lwm2mcore_ReadDeviceObj,
        .write = NULL,
        .exec = NULL,
    },
    {
        .id = LWM2MCORE_DEVICE_FIRMWARE_VERSION_RID,
        .type = LWM2MCORE_RESOURCE_TYPE_STRING,
        .max_res_inst_cnt = 1,
        .read = lwm2mcore_ReadDeviceObj,
        .write = NULL,
        .exec = NULL,
    },
    {
        .id = LWM2MCORE_DEVICE_CURRENT_TIME_RID,
        .max_res_inst_cnt = 1,
        .type = LWM2MCORE_RESOURCE_TYPE_TIME,
        .read = lwm2mcore_ReadDeviceObj,
        .write = lwm2mcore_WriteDeviceObj,
        .exec = NULL,
    },
    {
        .id = LWM2MCORE_DEVICE_SUPPORTED_BINDING_MODE_RID,
        .max_res_inst_cnt = 1,
        .type = LWM2MCORE_RESOURCE_TYPE_STRING,
        .read = lwm2mcore_ReadDeviceObj,
        .write = lwm2mcore_WriteDeviceObj,
        .exec = NULL,
    }
};


//--------------------------------------------------------------------------------------------------
/**
 * Firmware update supported resources defined for LWM2M object (5)
 * For each resource, the resource Id, the resource type, the resource instance number,
 * a READ, WRITE, EXEC callback can be defined.
 */
//--------------------------------------------------------------------------------------------------
static lwm2mcore_resource_t FirmwareUpdateResources[] =
{
    {
        .id = LWM2MCORE_FW_UPDATE_PACKAGE_RID,
        .max_res_inst_cnt = 1,
        .type = LWM2MCORE_RESOURCE_TYPE_OPAQUE,
        .read = NULL,
        .write = lwm2mcore_WriteFwUpdate,
        .exec = NULL
    },
    {
        .id = LWM2MCORE_FW_UPDATE_PACKAGE_URI_RID,
        .max_res_inst_cnt = 1,
        .type = LWM2MCORE_RESOURCE_TYPE_STRING,
        .read = lwm2mcore_ReadFwUpdate,
        .write = lwm2mcore_WriteFwUpdate,
        .exec = NULL
    },
    {
        .id = LWM2MCORE_FW_UPDATE_UPDATE_RID,
        .max_res_inst_cnt = 1,
        .type = LWM2MCORE_RESOURCE_TYPE_UNKNOWN,
        .read = NULL,
        .write = NULL,
        .exec = lwm2mcore_ExecFwUpdate,
    },
    {
        .id = LWM2MCORE_FW_UPDATE_UPDATE_STATE_RID,
        .max_res_inst_cnt = 1,
        .type = LWM2MCORE_RESOURCE_TYPE_INT,
        .read = lwm2mcore_ReadFwUpdate,
        .write = NULL,
        .exec = NULL
    },
    {
        .id = LWM2MCORE_FW_UPDATE_UPDATE_RESULT_RID,
        .max_res_inst_cnt = 1,
        .type = LWM2MCORE_RESOURCE_TYPE_INT,
        .read = lwm2mcore_ReadFwUpdate,
        .write = NULL,
        .exec = NULL
    },
    {
        .id = LWM2MCORE_FW_UPDATE_PACKAGE_NAME_RID,
        .max_res_inst_cnt = 1,
        .type = LWM2MCORE_RESOURCE_TYPE_STRING,
        .read = lwm2mcore_ReadFwUpdate,
        .write = NULL,
        .exec = NULL
    },
    {
        .id = LWM2MCORE_FW_UPDATE_PACKAGE_VERSION_RID,
        .max_res_inst_cnt = 1,
        .type = LWM2MCORE_RESOURCE_TYPE_STRING,
        .read = lwm2mcore_ReadFwUpdate,
        .write = NULL,
        .exec = NULL
    }
};



//--------------------------------------------------------------------------------------------------
/**
 * SSL certificate supported resources defined for LWM2M certificate object (10243)
 * For each resource, the resource Id, the resource type, the resource instance number,
 * a READ, WRITE, EXEC callback can be defined.
 */
//--------------------------------------------------------------------------------------------------
static lwm2mcore_resource_t SslCertificateResources[] =
{
    {
        .id = LWM2MCORE_SSL_CERTIFICATE_CERTIF,
        .max_res_inst_cnt = 1,
        .type = LWM2MCORE_RESOURCE_TYPE_OPAQUE,
        .read = OnSslCertif,
        .write = OnSslCertif,
        .exec = NULL,
    }
};

//--------------------------------------------------------------------------------------------------
/**
 * List of objects which are supported by the client
 *
 * For each object, the following parameters needs to be filled:
 *  - id: the object Id
 *  - max_obj_inst_cnt: maximum object instance number
 *  - res_cnt: resources number which are supported for this object
 *  - resources: supported resources table
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_object_t ObjArray[] =
{
    /* object 0, LWM2M security */
    {
        .id = LWM2MCORE_SECURITY_OID,
        .max_obj_inst_cnt = LWM2MCORE_DM_SERVER_MAX_COUNT + LWM2MCORE_BOOTSRAP_SERVER_MAX_COUNT,
        .res_cnt = ARRAYSIZE(SecurityResources),
        .resources = SecurityResources,
    },
    /* object 1, LWM2M DM server */
    {
        .id = LWM2MCORE_SERVER_OID,
        .max_obj_inst_cnt = LWM2MCORE_DM_SERVER_MAX_COUNT,
        .res_cnt = ARRAYSIZE(ServerResources),
        .resources = ServerResources,
    },
    /* object 3, device */
    {
        .id = LWM2MCORE_DEVICE_OID,
        .max_obj_inst_cnt = 1,
        .res_cnt = ARRAYSIZE(DeviceResources),
        .resources = DeviceResources,
    },
    /* object 5, firmware update */
    {
        .id = LWM2MCORE_FIRMWARE_UPDATE_OID,
        .max_obj_inst_cnt = 1,
        .res_cnt = ARRAYSIZE(FirmwareUpdateResources),
        .resources = FirmwareUpdateResources,
    },
    /* object 10243, SSL certificate */
    {
        .id = LWM2M_SWI_SSL_CERTIFS_OID,
        .max_obj_inst_cnt = 1,
        .res_cnt = ARRAYSIZE(SslCertificateResources),
        .resources = SslCertificateResources
    }
};

//--------------------------------------------------------------------------------------------------
/**
 * Handler indicating the supported objects list and default callback for not registered objects
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_handler_t Lwm2mcoreHandlers =
{
    .obj_cnt = ARRAYSIZE(ObjArray),
    .objects = ObjArray,
    .generic_UO_handler = OnUnlistedObject,
};

