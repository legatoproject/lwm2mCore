/**
 * @file objectsTable.c
 *
 * Object table and resource supported by the client
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

/* include files */
#include <platform/types.h>
#include <stdint.h>
#include <stdio.h>
#include <lwm2mcore/lwm2mcore.h>
#include "objects.h"
#include "handlers.h"
#include "clientConfig.h"
#include "clockTimeConfiguration.h"

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
        omanager_ReadSecurityObj,                   //.read
        omanager_WriteSecurityObj,                  //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_SECURITY_BOOTSTRAP_SERVER_RID,    //.id
        LWM2MCORE_RESOURCE_TYPE_BOOL,               //.type
        1,                                          //.maxResInstCnt
        omanager_ReadSecurityObj,                   //.read
        omanager_WriteSecurityObj,                  //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_SECURITY_MODE_RID,                //.id
        LWM2MCORE_RESOURCE_TYPE_INT,                //.type
        1,                                          //.maxResInstCnt
        omanager_ReadSecurityObj,                   //.read
        omanager_WriteSecurityObj,                  //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_SECURITY_PKID_RID,                //.id
        LWM2MCORE_RESOURCE_TYPE_OPAQUE,             //.type
        1,                                          //.maxResInstCnt
        omanager_ReadSecurityObj,                   //.read
        omanager_WriteSecurityObj,                  //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_SECURITY_SERVER_KEY_RID,          //.id
        LWM2MCORE_RESOURCE_TYPE_OPAQUE,             //.type
        1,                                          //.maxResInstCnt
        omanager_ReadSecurityObj,                   //.read
        omanager_WriteSecurityObj,                  //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_SECURITY_SECRET_KEY_RID,          //.id
        LWM2MCORE_RESOURCE_TYPE_OPAQUE,             //.type
        1,                                          //.maxResInstCnt
        omanager_ReadSecurityObj,                   //.read
        omanager_WriteSecurityObj,                  //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_SECURITY_SMS_SECURITY_MODE_RID,   //.id
        LWM2MCORE_RESOURCE_TYPE_INT,                //.type
        1,                                          //.maxResInstCnt
        NULL,                                       //.read
        omanager_SmsDummy,                          //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_SECURITY_SMS_BINDING_KEY_PAR_RID, //.id
        LWM2MCORE_RESOURCE_TYPE_OPAQUE,             //.type
        1,                                          //.maxResInstCnt
        NULL,                                       //.read
        omanager_SmsDummy,                          //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_SECURITY_SMS_BINDING_SEC_KEY_RID, //.id
        LWM2MCORE_RESOURCE_TYPE_OPAQUE,             //.type
        1,                                          //.maxResInstCnt
        NULL,                                       //.read
        omanager_SmsDummy,                          //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_SECURITY_SERVER_SMS_NUMBER_RID,   //.id
        LWM2MCORE_RESOURCE_TYPE_STRING,             //.type
        1,                                          //.maxResInstCnt
        NULL,                                       //.read
        omanager_SmsDummy,                          //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_SECURITY_SERVER_ID_RID,           //.id
        LWM2MCORE_RESOURCE_TYPE_INT,                //.type
        1,                                          //.maxResInstCnt
        omanager_ReadSecurityObj,                   //.read
        omanager_WriteSecurityObj,                  //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_SECURITY_CLIENT_HOLD_OFF_TIME_RID, //.id
        LWM2MCORE_RESOURCE_TYPE_INT,                 //.type
        1,                                           //.maxResInstCnt
        omanager_ReadSecurityObj,                    //.read
        omanager_WriteSecurityObj,                   //.write
        NULL,                                        //.exec
    },
    {
        LWM2MCORE_SECURITY_BS_ACCOUNT_TIMEOUT_RID,   //.id
        LWM2MCORE_RESOURCE_TYPE_INT,                 //.type
        1,                                           //.maxResInstCnt
        omanager_ReadSecurityObj,                    //.read
        omanager_WriteSecurityObj,                   //.write
        NULL,                                        //.exec
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
        omanager_ReadServerObj,                     //.read
        omanager_WriteServerObj,                    //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_SERVER_LIFETIME_RID,              //.id
        LWM2MCORE_RESOURCE_TYPE_INT,                //.type
        1,                                          //.maxResInstCnt
        omanager_ReadServerObj,                     //.read
        omanager_WriteServerObj,                    //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_SERVER_DEFAULT_MIN_PERIOD_RID,    //.id
        LWM2MCORE_RESOURCE_TYPE_INT,                //.type
        1,                                          //.maxResInstCnt
        omanager_ReadServerObj,                     //.read
        omanager_WriteServerObj,                    //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_SERVER_DEFAULT_MAX_PERIOD_RID,    //.id
        LWM2MCORE_RESOURCE_TYPE_INT,                //.type
        1,                                          //.maxResInstCnt
        omanager_ReadServerObj,                     //.read
        omanager_WriteServerObj,                    //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_SERVER_DISABLE_TIMEOUT_RID,       //.id
        LWM2MCORE_RESOURCE_TYPE_INT,                //.type
        1,                                          //.maxResInstCnt
        omanager_ReadServerObj,                     //.read
        omanager_WriteServerObj,                    //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_SERVER_STORE_NOTIF_WHEN_OFFLINE_RID,  //.id
        LWM2MCORE_RESOURCE_TYPE_BOOL,                   //.type
        1,                                              //.maxResInstCnt
        omanager_ReadServerObj,                         //.read
        omanager_WriteServerObj,                        //.write
        NULL,                                           //.exec
    },
    {
        LWM2MCORE_SERVER_BINDING_MODE_RID,          //.id
        LWM2MCORE_RESOURCE_TYPE_STRING,             //.type
        1,                                          //.maxResInstCnt
        omanager_ReadServerObj,                     //.read
        omanager_WriteServerObj,                    //.write
        NULL,                                       //.exec
    }
};

//--------------------------------------------------------------------------------------------------
/**
 * Acl_resources supported resources defined for LWM2M Access Control Lists object.
 * For each resource, the resource Id, the resource type, the resource instance number,
 * a READ, WRITE, EXEC callback can be defined.
 */
//--------------------------------------------------------------------------------------------------
static lwm2mcore_Resource_t AclResources[] =
{
    {
        LWM2MCORE_ACL_OBJECT_ID_RID,            //.id
        LWM2MCORE_RESOURCE_TYPE_INT,            //.type
        1,                                      //.maxResInstCnt
        omanager_ReadAclObj,                    //.read
        omanager_WriteAclObj,                   //.write
        NULL,                                   //.exec
    },
    {
        LWM2MCORE_ACL_OBJECT_INSTANCE_ID_RID,   //.id
        LWM2MCORE_RESOURCE_TYPE_INT,            //.type
        1,                                      //.maxResInstCnt
        omanager_ReadAclObj,                    //.read
        omanager_WriteAclObj,                   //.write
        NULL,                                   //.exec
    },
    {
        LWM2MCORE_ACL_ACCESS_CONTROL_ID,        //.id
        LWM2MCORE_RESOURCE_TYPE_INT,            //.type
        LWM2MCORE_ID_NONE,                      //.maxResInstCnt
        omanager_ReadAclObj,                    //.read
        omanager_WriteAclObj,                   //.write
        NULL,                                   //.exec
    },
    {
        LWM2MCORE_ACL_OWNER_RID,                //.id
        LWM2MCORE_RESOURCE_TYPE_INT,            //.type
        1,                                      //.maxResInstCnt
        omanager_ReadAclObj,                    //.read
        omanager_WriteAclObj,                   //.write
        NULL,                                   //.exec
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
        omanager_ReadDeviceObj,                     //.read
        NULL,                                       //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_DEVICE_MODEL_NUMBER_RID,          //.id
        LWM2MCORE_RESOURCE_TYPE_STRING,             //.type
        1,                                          //.maxResInstCnt
        omanager_ReadDeviceObj,                     //.read
        NULL,                                       //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_DEVICE_SERIAL_NUMBER_RID,         //.id
        LWM2MCORE_RESOURCE_TYPE_STRING,             //.type
        1,                                          //.maxResInstCnt
        omanager_ReadDeviceObj,                     //.read
        NULL,                                       //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_DEVICE_FIRMWARE_VERSION_RID,      //.id
        LWM2MCORE_RESOURCE_TYPE_STRING,             //.type
        1,                                          //.maxResInstCnt
        omanager_ReadDeviceObj,                     //.read
        NULL,                                       //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_DEVICE_REBOOT_RID,                //.id
        LWM2MCORE_RESOURCE_TYPE_UNKNOWN,            //.type
        1,                                          //.maxResInstCnt
        NULL,                                       //.read
        NULL,                                       //.write
        omanager_ExecDeviceObj,                     //.exec
    },
    {
        LWM2MCORE_DEVICE_AVAIL_POWER_SOURCES_RID,   //.id
        LWM2MCORE_RESOURCE_TYPE_INT,                //.type
        CONN_MONITOR_AVAIL_POWER_SOURCE_MAX_NB,     //.maxResInstCnt
        omanager_ReadDeviceObj,                     //.read
        NULL,                                       //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_DEVICE_AVAIL_POWER_VOLTAGES_RID,  //.id
        LWM2MCORE_RESOURCE_TYPE_INT,                //.type
        CONN_MONITOR_AVAIL_POWER_SOURCE_MAX_NB,     //.maxResInstCnt
        omanager_ReadDeviceObj,                     //.read
        NULL,                                       //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_DEVICE_AVAIL_POWER_CURRENTS_RID,  //.id
        LWM2MCORE_RESOURCE_TYPE_INT,                //.type
        CONN_MONITOR_AVAIL_POWER_SOURCE_MAX_NB,     //.maxResInstCnt
        omanager_ReadDeviceObj,                     //.read
        NULL,                                       //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_DEVICE_BATTERY_LEVEL_RID,         //.id
        LWM2MCORE_RESOURCE_TYPE_INT,                //.type
        1,                                          //.maxResInstCnt
        omanager_ReadDeviceObj,                     //.read
        NULL,                                       //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_DEVICE_CURRENT_TIME_RID,          //.id
        LWM2MCORE_RESOURCE_TYPE_TIME,               //.type
        1,                                          //.maxResInstCnt
        omanager_ReadDeviceObj,                     //.read
        omanager_WriteDeviceObj,                    //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_DEVICE_SUPPORTED_BINDING_MODE_RID, //.id
        LWM2MCORE_RESOURCE_TYPE_STRING,              //.type
        1,                                          //.maxResInstCnt
        omanager_ReadDeviceObj,                     //.read
        omanager_WriteDeviceObj,                    //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_DEVICE_BATTERY_STATUS_RID,        //.id
        LWM2MCORE_RESOURCE_TYPE_INT,                //.type
        1,                                          //.maxResInstCnt
        omanager_ReadDeviceObj,                     //.read
        NULL,                                       //.write
        NULL,                                       //.exec
    }
};

//--------------------------------------------------------------------------------------------------
/**
 * Supported resources defined for LWM2M Connectivity Monitoring object.
 * For each resource, the resource Id, the resource type, the resource instance number,
 * a READ, WRITE, EXEC callback can be defined.
 */
//--------------------------------------------------------------------------------------------------
static lwm2mcore_Resource_t ConnectivityMonitoringResources[] =
{
    {
        LWM2MCORE_CONN_MONITOR_NETWORK_BEARER_RID,          //.id
        LWM2MCORE_RESOURCE_TYPE_INT,                        //.type
        1,                                                  //.maxResInstCnt
        omanager_ReadConnectivityMonitoringObj,             //.read
        NULL,                                               //.write
        NULL,                                               //.exec
    },
    {
        LWM2MCORE_CONN_MONITOR_AVAIL_NETWORK_BEARER_RID,    //.id
        LWM2MCORE_RESOURCE_TYPE_INT,                        //.type
        CONN_MONITOR_AVAIL_NETWORK_BEARER_MAX_NB,           //.maxResInstCnt
        omanager_ReadConnectivityMonitoringObj,                      //.read
        NULL,                                               //.write
        NULL,                                               //.exec
    },
    {
        LWM2MCORE_CONN_MONITOR_RADIO_SIGNAL_STRENGTH_RID,   //.id
        LWM2MCORE_RESOURCE_TYPE_INT,                        //.type
        1,                                                  //.maxResInstCnt
        omanager_ReadConnectivityMonitoringObj,             //.read
        NULL,                                               //.write
        NULL,                                               //.exec
    },
    {
        LWM2MCORE_CONN_MONITOR_LINK_QUALITY_RID,            //.id
        LWM2MCORE_RESOURCE_TYPE_INT,                        //.type
        1,                                                  //.maxResInstCnt
        omanager_ReadConnectivityMonitoringObj,             //.read
        NULL,                                               //.write
        NULL,                                               //.exec
    },
    {
        LWM2MCORE_CONN_MONITOR_IP_ADDRESSES_RID,            //.id
        LWM2MCORE_RESOURCE_TYPE_STRING,                     //.type
        CONN_MONITOR_IP_ADDRESSES_MAX_NB,                   //.maxResInstCnt
        omanager_ReadConnectivityMonitoringObj,             //.read
        NULL,                                               //.write
        NULL,                                               //.exec
    },
    {
        LWM2MCORE_CONN_MONITOR_ROUTER_IP_ADDRESSES_RID,     //.id
        LWM2MCORE_RESOURCE_TYPE_STRING,                     //.type
        CONN_MONITOR_ROUTER_IP_ADDRESSES_MAX_NB,            //.maxResInstCnt
        omanager_ReadConnectivityMonitoringObj,             //.read
        NULL,                                               //.write
        NULL,                                               //.exec
    },
    {
        LWM2MCORE_CONN_MONITOR_LINK_UTILIZATION_RID,        //.id
        LWM2MCORE_RESOURCE_TYPE_INT,                        //.type
        1,                                                  //.maxResInstCnt
        omanager_ReadConnectivityMonitoringObj,             //.read
        NULL,                                               //.write
        NULL,                                               //.exec
    },
    {
        LWM2MCORE_CONN_MONITOR_APN_RID,                     //.id
        LWM2MCORE_RESOURCE_TYPE_STRING,                     //.type
        CONN_MONITOR_APN_MAX_NB,                            //.maxResInstCnt
        omanager_ReadConnectivityMonitoringObj,             //.read
        NULL,                                               //.write
        NULL,                                               //.exec
    },
    {
        LWM2MCORE_CONN_MONITOR_CELL_ID_RID,                 //.id
        LWM2MCORE_RESOURCE_TYPE_INT,                        //.type
        1,                                                  //.maxResInstCnt
        omanager_ReadConnectivityMonitoringObj,             //.read
        NULL,                                               //.write
        NULL,                                               //.exec
    },
    {
        LWM2MCORE_CONN_MONITOR_SMNC_RID,                    //.id
        LWM2MCORE_RESOURCE_TYPE_INT,                        //.type
        1,                                                  //.maxResInstCnt
        omanager_ReadConnectivityMonitoringObj,             //.read
        NULL,                                               //.write
        NULL,                                               //.exec
    },
    {
        LWM2MCORE_CONN_MONITOR_SMCC_RID,                    //.id
        LWM2MCORE_RESOURCE_TYPE_INT,                        //.type
        1,                                                  //.maxResInstCnt
        omanager_ReadConnectivityMonitoringObj,             //.read
        NULL,                                               //.write
        NULL,                                               //.exec
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
        omanager_WriteFwUpdateObj,                  //.write
        NULL                                        //.exec
    },
    {
        LWM2MCORE_FW_UPDATE_PACKAGE_URI_RID,        //.id
        LWM2MCORE_RESOURCE_TYPE_STRING,             //.type
        1,                                          //.maxResInstCnt
        omanager_ReadFwUpdateObj,                   //.read
        omanager_WriteFwUpdateObj,                  //.write
        NULL                                        //.exec
    },
    {
        LWM2MCORE_FW_UPDATE_UPDATE_RID,             //.id
        LWM2MCORE_RESOURCE_TYPE_UNKNOWN,            //.type
        1,                                          //.maxResInstCnt
        NULL,                                       //.read
        NULL,                                       //.write
        omanager_ExecFwUpdate,                      //.exec
    },
    {
        LWM2MCORE_FW_UPDATE_UPDATE_STATE_RID,       //.id
        LWM2MCORE_RESOURCE_TYPE_INT,                //.type
        1,                                          //.maxResInstCnt
        omanager_ReadFwUpdateObj,                   //.read
        NULL,                                       //.write
        NULL                                        //.exec
    },
    {
        LWM2MCORE_FW_UPDATE_UPDATE_RESULT_RID,      //.id
        LWM2MCORE_RESOURCE_TYPE_INT,                //.type
        1,                                          //.maxResInstCnt
        omanager_ReadFwUpdateObj,                   //.read
        NULL,                                       //.write
        NULL                                        //.exec
    },
    {
        LWM2MCORE_FW_UPDATE_PROTO_SUPPORT_RID,      //.id
        LWM2MCORE_RESOURCE_TYPE_INT,                //.type
        2,                                          //.maxResInstCnt
        omanager_ReadFwUpdateObj,                   //.read
        NULL,                                       //.write
        NULL                                        //.exec
    },
    {
        LWM2MCORE_FW_UPDATE_DELIVERY_METHOD_RID,    //.id
        LWM2MCORE_RESOURCE_TYPE_INT,                //.type
        1,                                          //.maxResInstCnt
        omanager_ReadFwUpdateObj,                   //.read
        NULL,                                       //.write
        NULL                                        //.exec
    }
};

//--------------------------------------------------------------------------------------------------
/**
 * Supported resources defined for LWM2M Location object.
 * For each resource, the resource Id, the resource type, the resource instance number,
 * a READ, WRITE, EXEC callback can be defined.
 */
//--------------------------------------------------------------------------------------------------
static lwm2mcore_Resource_t LocationResources[] =
{
    {
        LWM2MCORE_LOCATION_LATITUDE_RID,            //.id
#ifdef LWM2M_LOCATION_FLOAT
        LWM2MCORE_RESOURCE_TYPE_FLOAT,              //.type
#else
        LWM2MCORE_RESOURCE_TYPE_STRING,             //.type
#endif
        1,                                          //.maxResInstCnt
        omanager_ReadLocationObj,                   //.read
        NULL,                                       //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_LOCATION_LONGITUDE_RID,           //.id
#ifdef LWM2M_LOCATION_FLOAT
        LWM2MCORE_RESOURCE_TYPE_FLOAT,              //.type
#else
        LWM2MCORE_RESOURCE_TYPE_STRING,             //.type
#endif
        1,                                          //.maxResInstCnt
        omanager_ReadLocationObj,                   //.read
        NULL,                                       //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_LOCATION_ALTITUDE_RID,            //.id
#ifdef LWM2M_LOCATION_FLOAT
        LWM2MCORE_RESOURCE_TYPE_FLOAT,              //.type
#else
        LWM2MCORE_RESOURCE_TYPE_STRING,             //.type
#endif
        1,                                          //.maxResInstCnt
        omanager_ReadLocationObj,                   //.read
        NULL,                                       //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_LOCATION_VELOCITY_RID,            //.id
        LWM2MCORE_RESOURCE_TYPE_OPAQUE,             //.type
        1,                                          //.maxResInstCnt
        omanager_ReadLocationObj,                   //.read
        NULL,                                       //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_LOCATION_TIMESTAMP_RID,           //.id
        LWM2MCORE_RESOURCE_TYPE_TIME,               //.type
        1,                                          //.maxResInstCnt
        omanager_ReadLocationObj,                   //.read
        NULL,                                       //.write
        NULL,                                       //.exec
    }
};

//--------------------------------------------------------------------------------------------------
/**
 * Supported resources defined for LWM2M Connectivity Statistics object.
 * For each resource, the resource Id, the resource type, the resource instance number,
 * a READ, WRITE, EXEC callback can be defined.
 */
//--------------------------------------------------------------------------------------------------
static lwm2mcore_Resource_t ConnectivityStatisticsResources[] =
{
    {
        LWM2MCORE_CONN_STATS_TX_SMS_COUNT_RID,      //.id
        LWM2MCORE_RESOURCE_TYPE_INT,                //.type
        1,                                          //.maxResInstCnt
        omanager_ReadConnectivityStatisticsObj,     //.read
        NULL,                                       //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_CONN_STATS_RX_SMS_COUNT_RID,      //.id
        LWM2MCORE_RESOURCE_TYPE_INT,                //.type
        1,                                          //.maxResInstCnt
        omanager_ReadConnectivityStatisticsObj,     //.read
        NULL,                                       //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_CONN_STATS_TX_DATA_COUNT_RID,     //.id
        LWM2MCORE_RESOURCE_TYPE_INT,                //.type
        1,                                          //.maxResInstCnt
        omanager_ReadConnectivityStatisticsObj,     //.read
        NULL,                                       //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_CONN_STATS_RX_DATA_COUNT_RID,     //.id
        LWM2MCORE_RESOURCE_TYPE_INT,                //.type
        1,                                          //.maxResInstCnt
        omanager_ReadConnectivityStatisticsObj,     //.read
        NULL,                                       //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_CONN_STATS_START_RID,             //.id
        LWM2MCORE_RESOURCE_TYPE_UNKNOWN,            //.type
        1,                                          //.maxResInstCnt
        NULL,                                       //.read
        NULL,                                       //.write
        omanager_ExecConnectivityStatistics,        //.exec
    },
    {
        LWM2MCORE_CONN_STATS_STOP_RID,              //.id
        LWM2MCORE_RESOURCE_TYPE_UNKNOWN,            //.type
        1,                                          //.maxResInstCnt
        NULL,                                       //.read
        NULL,                                       //.write
        omanager_ExecConnectivityStatistics,        //.exec
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
        omanager_ReadSwUpdateObj,                   //.read
        NULL,                                       //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_SW_UPDATE_PACKAGE_VERSION_RID,    //.id
        LWM2MCORE_RESOURCE_TYPE_STRING,             //.type
        1,                                          //.maxResInstCnt
        omanager_ReadSwUpdateObj,                   //.read
        NULL,                                       //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_SW_UPDATE_PACKAGE_URI_RID,        //.id
        LWM2MCORE_RESOURCE_TYPE_STRING,             //.type
        1,                                          //.maxResInstCnt
        NULL,                                       //.read
        omanager_WriteSwUpdateObj,                  //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_SW_UPDATE_INSTALL_RID,            //.id
        LWM2MCORE_RESOURCE_TYPE_UNKNOWN,            //.type
        1,                                          //.maxResInstCnt
        NULL,                                       //.read
        NULL,                                       //.write
        omanager_ExecSwUpdate,                      //.exec
    },
    {
        LWM2MCORE_SW_UPDATE_UNINSTALL_RID,          //.id
        LWM2MCORE_RESOURCE_TYPE_INT,                //.type
        1,                                          //.maxResInstCnt
        NULL,                                       //.read
        NULL,                                       //.write
        omanager_ExecSwUpdate,                      //.exec
    },
    {
        LWM2MCORE_SW_UPDATE_UPDATE_STATE_RID,       //.id
        LWM2MCORE_RESOURCE_TYPE_INT,                //.type
        1,                                          //.maxResInstCnt
        omanager_ReadSwUpdateObj,                   //.read
        NULL,                                       //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_SW_UPDATE_UPDATE_SUPPORTED_OBJ_RID, //.id
        LWM2MCORE_RESOURCE_TYPE_BOOL,               //.type
        1,                                          //.maxResInstCnt
        omanager_ReadSwUpdateObj,                   //.read
        omanager_WriteSwUpdateObj,                  //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_SW_UPDATE_UPDATE_RESULT_RID,      //.id
        LWM2MCORE_RESOURCE_TYPE_INT,                //.type
        1,                                          //.maxResInstCnt
        omanager_ReadSwUpdateObj,                   //.read
        NULL,                                       //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_SW_UPDATE_ACTIVATE_RID,           //.id
        LWM2MCORE_RESOURCE_TYPE_UNKNOWN,            //.type
        1,                                          //.maxResInstCnt
        NULL,                                       //.read
        NULL,                                       //.write
        omanager_ExecSwUpdate,                      //.exec
    },
    {
        LWM2MCORE_SW_UPDATE_DEACTIVATE_RID,         //.id
        LWM2MCORE_RESOURCE_TYPE_UNKNOWN,            //.type
        1,                                          //.maxResInstCnt
        NULL,                                       //.read
        NULL,                                       //.write
        omanager_ExecSwUpdate,                      //.exec
    },
    {
        LWM2MCORE_SW_UPDATE_ACTIVATION_STATE_RID,   //.id
        LWM2MCORE_RESOURCE_TYPE_INT,                //.type
        1,                                          //.maxResInstCnt
        omanager_ReadSwUpdateObj,                   //.read
        NULL,                                       //.write
        NULL,                                       //.exec
    }
};

//--------------------------------------------------------------------------------------------------
/**
 * Supported resources defined for LWM2M Cellular connectivity object (10).
 * For each resource, the resource Id, the resource type, the resource instance number,
 * a READ, WRITE, EXEC callback can be defined.
 */
//--------------------------------------------------------------------------------------------------
static lwm2mcore_Resource_t CellularConnectivityResources[] =
{
    {
        LWM2MCORE_CELL_CONN_SMSC_ADDR_RID,          //.id
        LWM2MCORE_RESOURCE_TYPE_STRING,             //.type
        1,                                          //.maxResInstCnt
        omanager_ReadCellularConnectivityObj,       //.read
        omanager_WriteCellularConnectivityObj,      //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_CELL_CONN_DISABLE_RADIO_PERIOD_RID, //.id
        LWM2MCORE_RESOURCE_TYPE_INT,                //.type
        1,                                          //.maxResInstCnt
        omanager_ReadCellularConnectivityObj,       //.read
        omanager_WriteCellularConnectivityObj,      //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_CELL_CONN_MOD_ACTIVATION_CODE_RID,  //.id
        LWM2MCORE_RESOURCE_TYPE_STRING,             //.type
        1,                                          //.maxResInstCnt
        omanager_ReadCellularConnectivityObj,       //.read
        omanager_WriteCellularConnectivityObj,      //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_CELL_CONN_PSM_TIMER_RID,          //.id
        LWM2MCORE_RESOURCE_TYPE_INT,                //.type
        1,                                          //.maxResInstCnt
        omanager_ReadCellularConnectivityObj,       //.read
        omanager_WriteCellularConnectivityObj,      //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_CELL_CONN_ACTIVE_TIMER_RID,       //.id
        LWM2MCORE_RESOURCE_TYPE_INT,                //.type
        1,                                          //.maxResInstCnt
        omanager_ReadCellularConnectivityObj,       //.read
        omanager_WriteCellularConnectivityObj,      //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_CELL_CONN_SERV_PLMN_RATE_CTRL_RID,  //.id
        LWM2MCORE_RESOURCE_TYPE_INT,                //.type
        1,                                          //.maxResInstCnt
        omanager_ReadCellularConnectivityObj,       //.read
        NULL,                                       //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_CELL_CONN_EDRX_PARAM_IU_MODE_RID,   //.id
        LWM2MCORE_RESOURCE_TYPE_OPAQUE,             //.type
        1,                                          //.maxResInstCnt
        omanager_ReadCellularConnectivityObj,       //.read
        omanager_WriteCellularConnectivityObj,      //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_CELL_CONN_EDRX_PARAM_WB_S1_MODE_RID, //.id
        LWM2MCORE_RESOURCE_TYPE_OPAQUE,             //.type
        1,                                          //.maxResInstCnt
        omanager_ReadCellularConnectivityObj,       //.read
        omanager_WriteCellularConnectivityObj,      //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_CELL_CONN_EDRX_PARAM_NB_S1_MODE_RID, //.id
        LWM2MCORE_RESOURCE_TYPE_OPAQUE,             //.type
        1,                                          //.maxResInstCnt
        omanager_ReadCellularConnectivityObj,       //.read
        omanager_WriteCellularConnectivityObj,      //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_CELL_CONN_EDRX_PARAM_AGB_MODE_RID,  //.id
        LWM2MCORE_RESOURCE_TYPE_OPAQUE,             //.type
        1,                                          //.maxResInstCnt
        omanager_ReadCellularConnectivityObj,       //.read
        omanager_WriteCellularConnectivityObj,      //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_CELL_CONN_ACTIVATED_PROFILE_NAME_RID, //.id
        LWM2MCORE_RESOURCE_TYPE_STRING,             //.type
        1,                                          //.maxResInstCnt
        omanager_ReadCellularConnectivityObj,       //.read
        NULL,                                       //.write
        NULL,                                       //.exec
    }
};

//--------------------------------------------------------------------------------------------------
/**
 * Supported resources defined for Subscription, a Sierra Wireless proprietary object (10241)
 * For each resource, the resource Id, the resource type, the resource instance number,
 * a READ, WRITE, EXEC callback can be defined.
 */
//--------------------------------------------------------------------------------------------------
static lwm2mcore_Resource_t SubscriptionResources[] =
{
    {
        LWM2MCORE_SUBSCRIPTION_IMEI_RID,            //.id
        LWM2MCORE_RESOURCE_TYPE_STRING,             //.type
        1,                                          //.maxResInstCnt
        omanager_ReadSubscriptionObj,               //.read
        NULL,                                       //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_SUBSCRIPTION_ICCID_RID,           //.id
        LWM2MCORE_RESOURCE_TYPE_STRING,             //.type
        1,                                          //.maxResInstCnt
        omanager_ReadSubscriptionObj,               //.read
        NULL,                                       //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_SUBSCRIPTION_IDENTITY_RID,        //.id
        LWM2MCORE_RESOURCE_TYPE_STRING,             //.type
        1,                                          //.maxResInstCnt
        omanager_ReadSubscriptionObj,               //.read
        NULL,                                       //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_SUBSCRIPTION_MSISDN_RID,          //.id
        LWM2MCORE_RESOURCE_TYPE_STRING,             //.type
        1,                                          //.maxResInstCnt
        omanager_ReadSubscriptionObj,               //.read
        NULL,                                       //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_SUBSCRIPTION_SIM_MODE_RID,        //.id
        LWM2MCORE_RESOURCE_TYPE_STRING,             //.type
        1,                                          //.maxResInstCnt
        NULL,                                       //.read
        NULL,                                       //.write
        omanager_ExecSubscriptionObj,               //.exec
    },
    {
        LWM2MCORE_SUBSCRIPTION_CURRENT_SIM_RID,     //.id
        LWM2MCORE_RESOURCE_TYPE_INT,                //.type
        1,                                          //.maxResInstCnt
        omanager_ReadSubscriptionObj,               //.read
        NULL,                                       //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_SUBSCRIPTION_CURRENT_SIM_MODE_RID,//.id
        LWM2MCORE_RESOURCE_TYPE_INT,                //.type
        1,                                          //.maxResInstCnt
        omanager_ReadSubscriptionObj,               //.read
        NULL,                                       //.write
        NULL,                                       //.exec
    },
    {
        LWM2MCORE_SUBSCRIPTION_SIM_SWITCH_STATUS_RID,//.id
        LWM2MCORE_RESOURCE_TYPE_INT,                //.type
        1,                                          //.maxResInstCnt
        omanager_ReadSubscriptionObj,               //.read
        NULL,                                       //.write
        NULL,                                       //.exec
    }
};

//--------------------------------------------------------------------------------------------------
/**
 * Supported resources defined for Extended connectivity statistics, a Sierra Wireless proprietary
 * object (10242)
 * For each resource, the resource Id, the resource type, the resource instance number,
 * a READ, WRITE, EXEC callback can be defined.
 */
//--------------------------------------------------------------------------------------------------
static lwm2mcore_Resource_t ExtConnectivityStatsResources[] =
{
    {
        LWM2MCORE_EXT_CONN_STATS_SIGNAL_BARS_RID,       //.id
        LWM2MCORE_RESOURCE_TYPE_INT,                    //.type
        1,                                              //.maxResInstCnt
        omanager_ReadExtConnectivityStatsObj,           //.read
        NULL,                                           //.write
        NULL,                                           //.exec
    },
    {
        LWM2MCORE_EXT_CONN_STATS_CELLULAR_TECH_RID,     //.id
        LWM2MCORE_RESOURCE_TYPE_STRING,                 //.type
        1,                                              //.maxResInstCnt
        omanager_ReadExtConnectivityStatsObj,           //.read
        NULL,                                           //.write
        NULL,                                           //.exec
    },
    {
        LWM2MCORE_EXT_CONN_STATS_ROAMING_RID,           //.id
        LWM2MCORE_RESOURCE_TYPE_INT,                    //.type
        1,                                              //.maxResInstCnt
        omanager_ReadExtConnectivityStatsObj,           //.read
        NULL,                                           //.write
        NULL,                                           //.exec
    },
    {
        LWM2MCORE_EXT_CONN_STATS_ECIO_RID,              //.id
        LWM2MCORE_RESOURCE_TYPE_INT,                    //.type
        1,                                              //.maxResInstCnt
        omanager_ReadExtConnectivityStatsObj,           //.read
        NULL,                                           //.write
        NULL,                                           //.exec
    },
    {
        LWM2MCORE_EXT_CONN_STATS_RSRP_RID,              //.id
        LWM2MCORE_RESOURCE_TYPE_INT,                    //.type
        1,                                              //.maxResInstCnt
        omanager_ReadExtConnectivityStatsObj,           //.read
        NULL,                                           //.write
        NULL,                                           //.exec
    },
    {
        LWM2MCORE_EXT_CONN_STATS_RSRQ_RID,              //.id
        LWM2MCORE_RESOURCE_TYPE_INT,                    //.type
        1,                                              //.maxResInstCnt
        omanager_ReadExtConnectivityStatsObj,           //.read
        NULL,                                           //.write
        NULL,                                           //.exec
    },
    {
        LWM2MCORE_EXT_CONN_STATS_RSCP_RID,              //.id
        LWM2MCORE_RESOURCE_TYPE_INT,                    //.type
        1,                                              //.maxResInstCnt
        omanager_ReadExtConnectivityStatsObj,           //.read
        NULL,                                           //.write
        NULL,                                           //.exec
    },
    {
        LWM2MCORE_EXT_CONN_STATS_TEMPERATURE_RID,       //.id
        LWM2MCORE_RESOURCE_TYPE_INT,                    //.type
        1,                                              //.maxResInstCnt
        omanager_ReadExtConnectivityStatsObj,           //.read
        NULL,                                           //.write
        NULL,                                           //.exec
    },
    {
        LWM2MCORE_EXT_CONN_STATS_UNEXPECTED_RESETS_RID, //.id
        LWM2MCORE_RESOURCE_TYPE_INT,                    //.type
        1,                                              //.maxResInstCnt
        omanager_ReadExtConnectivityStatsObj,           //.read
        NULL,                                           //.write
        NULL,                                           //.exec
    },
    {
        LWM2MCORE_EXT_CONN_STATS_TOTAL_RESETS_RID,      //.id
        LWM2MCORE_RESOURCE_TYPE_INT,                    //.type
        1,                                              //.maxResInstCnt
        omanager_ReadExtConnectivityStatsObj,           //.read
        NULL,                                           //.write
        NULL,                                           //.exec
    },
    {
        LWM2MCORE_EXT_CONN_STATS_LAC_RID,               //.id
        LWM2MCORE_RESOURCE_TYPE_INT,                    //.type
        1,                                              //.maxResInstCnt
        omanager_ReadExtConnectivityStatsObj,           //.read
        NULL,                                           //.write
        NULL,                                           //.exec
    },
    {
        LWM2MCORE_EXT_CONN_STATS_TAC_RID,               //.id
        LWM2MCORE_RESOURCE_TYPE_INT,                    //.type
        1,                                              //.maxResInstCnt
        omanager_ReadExtConnectivityStatsObj,           //.read
        NULL,                                           //.write
        NULL,                                           //.exec
    }
};

#ifdef LWM2M_OBJECT_33406
//--------------------------------------------------------------------------------------------------
/**
 * Supported resources defined for file transfer management object, a Sierra Wireless proprietary
 * object (33406)
 * For each resource, the resource Id, the resource type, the resource instance number,
 * a READ, WRITE, EXEC callback can be defined.
 */
//--------------------------------------------------------------------------------------------------
static lwm2mcore_Resource_t FileTransferResources[] =
{
   {
        LWM2MCORE_FILE_TRANSFER_NAME_RID,               //.id
        LWM2MCORE_RESOURCE_TYPE_STRING,                 //.type
        1,                                              //.maxResInstCnt
        NULL,                                           //.read
        omanager_WriteFileTransferObj,                  //.write
        NULL,                                           //.exec
    },
    {
        LWM2MCORE_FILE_TRANSFER_CLASS_RID,              //.id
        LWM2MCORE_RESOURCE_TYPE_STRING,                 //.type
        1,                                              //.maxResInstCnt
        NULL                        ,                   //.read
        omanager_WriteFileTransferObj,                  //.write
        NULL,                                           //.exec
    },
    {
        LWM2MCORE_FILE_TRANSFER_URI_RID,                //.id
        LWM2MCORE_RESOURCE_TYPE_STRING,                 //.type
        1,                                              //.maxResInstCnt
        NULL,                                           //.read
        omanager_WriteFileTransferObj,                  //.write
        NULL,                                           //.exec
    },
    {
        LWM2MCORE_FILE_TRANSFER_CHECKSUM_RID,             //.id
        LWM2MCORE_RESOURCE_TYPE_STRING,                 //.type
        1,                                              //.maxResInstCnt
        omanager_ReadFileTransferObj,                   //.read
        omanager_WriteFileTransferObj,                  //.write
        NULL,                                           //.exec
    },
    {
        LWM2MCORE_FILE_TRANSFER_DIRECTION_RID,          //.id
        LWM2MCORE_RESOURCE_TYPE_INT,                    //.type
        1,                                              //.maxResInstCnt
        NULL,                                           //.read
        omanager_WriteFileTransferObj,                    //.write
        NULL,                                           //.exec
    },
    {
        LWM2MCORE_FILE_TRANSFER_STATE_RID,              //.id
        LWM2MCORE_RESOURCE_TYPE_INT,                    //.type
        1,                                              //.maxResInstCnt
        omanager_ReadFileTransferObj,                   //.read
        NULL,                                           //.write
        NULL,                                           //.exec
    },
    {
        LWM2MCORE_FILE_TRANSFER_RESULT_RID,             //.id
        LWM2MCORE_RESOURCE_TYPE_INT,                    //.type
        1,                                              //.maxResInstCnt
        omanager_ReadFileTransferObj,                   //.read
        NULL,                                           //.write
        NULL,                                           //.exec
    },
    {
        LWM2MCORE_FILE_TRANSFER_PROGRESS_RID,           //.id
        LWM2MCORE_RESOURCE_TYPE_INT,                    //.type
        1,                                              //.maxResInstCnt
        omanager_ReadFileTransferObj,                   //.read
        NULL,                                           //.write
        NULL,                                           //.exec
    },
    {
        LWM2MCORE_FILE_TRANSFER_FAILURE_REASON_RID,     //.id
        LWM2MCORE_RESOURCE_TYPE_STRING,                 //.type
        1,                                              //.maxResInstCnt
        omanager_ReadFileTransferObj,                   //.read
        NULL,                                           //.write
        NULL,                                           //.exec
    }
};

//--------------------------------------------------------------------------------------------------
/**
 * Supported resources defined for stream sync object, a Sierra Wireless proprietary object (33407)
 * For each resource, the resource Id, the resource type, the resource instance number,
 * a READ, WRITE, EXEC callback can be defined.
 */
//--------------------------------------------------------------------------------------------------
static lwm2mcore_Resource_t FileListResources[] =
{
    {
        LWM2MCORE_FILE_LIST_NAME_RID,                   //.id
        LWM2MCORE_RESOURCE_TYPE_STRING,                 //.type
        1,                                              //.maxResInstCnt
        omanager_ReadFileListObj,                       //.read
        NULL,                                           //.write
        NULL,                                           //.exec
    },
    {
        LWM2MCORE_FILE_LIST_CLASS_RID,                  //.id
        LWM2MCORE_RESOURCE_TYPE_STRING,                 //.type
        1,                                              //.maxResInstCnt
        omanager_ReadFileListObj,                       //.read
        NULL,                                           //.write
        NULL,                                           //.exec
    },
    {
        LWM2MCORE_FILE_LIST_CHECKSUM_RID,               //.id
        LWM2MCORE_RESOURCE_TYPE_STRING,                 //.type
        1,                                              //.maxResInstCnt
        omanager_ReadFileListObj,                       //.read
        NULL,                                           //.write
        NULL,                                           //.exec
    },
    {
        LWM2MCORE_FILE_LIST_DIRECTION_RID,              //.id
        LWM2MCORE_RESOURCE_TYPE_INT,                    //.type
        1,                                              //.maxResInstCnt
        omanager_ReadFileListObj,                       //.read
        NULL,                                           //.write
        NULL,                                           //.exec
    }
};
#endif

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
        NULL,                                       //.read
        omanager_WriteSslCertif,                    //.write
        NULL,                                       //.exec
    }
};

//--------------------------------------------------------------------------------------------------
/**
 * Clock time configuration resources defined for LWM2M certificate object (33405)
 * For each resource, the resource Id, the resource type, the resource instance number,
 * a READ, WRITE, EXEC callback can be defined.
 */
//--------------------------------------------------------------------------------------------------
static lwm2mcore_Resource_t ClockTimeConfigResources[] =
{
    {
        LWM2MCORE_CLOCK_TIME_CONFIG_SOURCE_PRIORITY_RID,    //.id
        LWM2MCORE_RESOURCE_TYPE_INT,                        //.type
        1,                                                  //.maxResInstCnt
        omanager_ReadClockTimeConfigObj,                    //.read
        omanager_WriteClockTimeConfigObj,                   //.write
        NULL,                                               //.exec
    },
    {
        LWM2MCORE_CLOCK_TIME_CONFIG_SOURCE_CONFIG_RID,      //.id
        LWM2MCORE_RESOURCE_TYPE_STRING,                     //.type
        2,                                                  //.maxResInstCnt
        omanager_ReadClockTimeConfigObj,                    //.read
        omanager_WriteClockTimeConfigObj,                   //.write
        NULL,                                               //.exec
    },
    {
        LWM2MCORE_CLOCK_TIME_CONFIG_UPDATE_RID,             //.id
        LWM2MCORE_RESOURCE_TYPE_UNKNOWN,                    //.type
        1,                                                  //.maxResInstCnt
        NULL,                                               //.read
        NULL,                                               //.write
        omanager_ExecClockTimeConfigObj,                    //.exec
    },
    {
        LWM2MCORE_CLOCK_TIME_CONFIG_STATUS_RID,             //.id
        LWM2MCORE_RESOURCE_TYPE_INT,                        //.type
        1,                                                  //.maxResInstCnt
        omanager_ReadClockTimeConfigObj,                    //.read
        NULL,                                               //.write
        NULL,                                               //.exec
    }
};

//--------------------------------------------------------------------------------------------------
/**
 * SIM APDU config resources (33408)
 * For each resource, the resource Id, the resource type, the resource instance number,
 * a READ, WRITE, EXEC callback can be defined.
 */
//--------------------------------------------------------------------------------------------------
static lwm2mcore_Resource_t SimApduConfigResources[] =
{
    {
        LWM2MCORE_SIM_APDU_CONFIG_COMMAND_RID,              //.id
        LWM2MCORE_RESOURCE_TYPE_OPAQUE,                     //.type
        1,                                                  //.maxResInstCnt
        NULL,                                               //.read
        omanager_WriteSimApduConfigObj,                     //.write
        NULL,                                               //.exec
    },
    {
        LWM2MCORE_SIM_APDU_CONFIG_EXEC_RID,                 //.id
        LWM2MCORE_RESOURCE_TYPE_UNKNOWN,                    //.type
        1,                                                  //.maxResInstCnt
        NULL,                                               //.read
        NULL,                                               //.write
        omanager_ExecSimApduConfigObj,                      //.exec
    },
    {
        LWM2MCORE_SIM_APDU_CONFIG_RESPONSE_RID,             //.id
        LWM2MCORE_RESOURCE_TYPE_OPAQUE,                     //.type
        1,                                                  //.maxResInstCnt
        omanager_ReadSimApduConfigObj,                      //.read
        NULL,                                               //.write
        NULL,                                               //.exec
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
static lwm2mcore_Object_t ObjArray[] =
{
    /* Object 0: LWM2M security */
    {
        LWM2MCORE_SECURITY_OID,                                                 //.id
        LWM2MCORE_ID_NONE,                                                      //.maxObjInstCnt
        ARRAYSIZE(SecurityResources),                                           //.resCnt
        SecurityResources                                                       //.resources
    },
    /* Object 1: LWM2M DM server */
    {
        LWM2MCORE_SERVER_OID,                                                   //.id
        LWM2MCORE_ID_NONE,                                                      //.maxObjInstCnt
        ARRAYSIZE(ServerResources),                                             //.resCnt
        ServerResources                                                         //.resources
    },
    /* Object 2: ACL */
    {
        LWM2MCORE_ACL_OID,                                                      //.id
        LWM2MCORE_ID_NONE,                                                      //.maxObjInstCnt
        ARRAYSIZE(AclResources),                                                //.resCnt
        AclResources                                                            //.resources
    },
    /* Object 3: device */
    {
        LWM2MCORE_DEVICE_OID,                                                   //.id
        1,                                                                      //.maxObjInstCnt
        ARRAYSIZE(DeviceResources),                                             //.resCnt
        DeviceResources                                                         //.resources
    },
    /* Object 4: connectivity monitoring */
    {
        LWM2MCORE_CONN_MONITOR_OID,                                             //.id
        1,                                                                      //.maxObjInstCnt
        ARRAYSIZE(ConnectivityMonitoringResources),                             //.resCnt
        ConnectivityMonitoringResources                                         //.resources
    },
    /* Object 5: firmware update */
    {
        LWM2MCORE_FIRMWARE_UPDATE_OID,                                          //.id
        1,                                                                      //.maxObjInstCnt
        ARRAYSIZE(FirmwareUpdateResources),                                     //.resCnt
        FirmwareUpdateResources                                                 //.resources
    },
    /* Object 6: location */
    {
        LWM2MCORE_LOCATION_OID,                                                 //.id
        1,                                                                      //.maxObjInstCnt
        ARRAYSIZE(LocationResources),                                           //.resCnt
        LocationResources                                                       //.resources
    },
    /* Object 7: connectivity statistics */
    {
        LWM2MCORE_CONN_STATS_OID,                                               //.id
        1,                                                                      //.maxObjInstCnt
        ARRAYSIZE(ConnectivityStatisticsResources),                             //.resCnt
        ConnectivityStatisticsResources                                         //.resources
    },
    /* Object 9: software update */
    {
        LWM2MCORE_SOFTWARE_UPDATE_OID,                                          //.id
        LWM2MCORE_ID_NONE,                                                      //.maxObjInstCnt
        ARRAYSIZE(SoftwareUpdateResources),                                     //.resCnt
        SoftwareUpdateResources                                                 //.resources
    },
    /* Object 10: cellular connectivity */
    {
        LWM2MCORE_CELL_CONN_OID,                                                //.id
        1,                                                                      //.maxObjInstCnt
        ARRAYSIZE(CellularConnectivityResources),                               //.resCnt
        CellularConnectivityResources                                           //.resources
    },
    /* Object 10241: subscription */
    {
        LWM2MCORE_SUBSCRIPTION_OID,                                             //.id
        1,                                                                      //.maxObjInstCnt
        ARRAYSIZE(SubscriptionResources),                                       //.resCnt
        SubscriptionResources                                                   //.resources
    },
    /* Object 10242: extended connectivity statistics */
    {
        LWM2MCORE_EXT_CONN_STATS_OID,                                           //.id
        1,                                                                      //.maxObjInstCnt
        ARRAYSIZE(ExtConnectivityStatsResources),                               //.resCnt
        ExtConnectivityStatsResources                                           //.resources
    },
    /* Object 10243: SSL certificate */
    {
        LWM2MCORE_SSL_CERTIFS_OID,                                              //.id
        1,                                                                      //.maxObjInstCnt
        ARRAYSIZE(SslCertificateResources),                                     //.resCnt
        SslCertificateResources                                                 //.resources
    },
    /* Object 33405: Clock Time configuration */
    {
        LWM2MCORE_CLOCK_TIME_CONFIG_OID,                                        //.id
        CLOCK_TIME_CONFIG_SOURCE_MAX,                                           //.maxObjInstCnt
        ARRAYSIZE(ClockTimeConfigResources),                                    //.resCnt
        ClockTimeConfigResources                                                //.resources
    },
#ifdef LWM2M_OBJECT_33406
    /* Object 33406: File transfer */
    {
        LWM2MCORE_FILE_TRANSFER_OID,                                            //.id
        1,                                                                      //.maxObjInstCnt
        ARRAYSIZE(FileTransferResources),                                       //.resCnt
        FileTransferResources                                                   //.resources
    },
    /* Object 33407: File list */
    {
        LWM2MCORE_FILE_LIST_OID,                                                //.id
        LWM2MCORE_ID_NONE,                                                      //.maxObjInstCnt
        ARRAYSIZE(FileListResources),                                           //.resCnt
        FileListResources                                                       //.resources
    },
#endif
    /* Object 33408: SIM APDU config */
    {
        LWM2MCORE_SIM_APDU_CONFIG_OID,                                          //.id
        1,                                                                      //.maxObjInstCnt
        ARRAYSIZE(SimApduConfigResources),                                      //.resCnt
        SimApduConfigResources                                                  //.resources
    }
};

//--------------------------------------------------------------------------------------------------
/**
 * Handler indicating the supported objects list and default callback for not registered objects
 */
//--------------------------------------------------------------------------------------------------
static lwm2mcore_Handler_t Lwm2mcoreHandlers =
{
    ARRAYSIZE(ObjArray),                        //.objCnt
    ObjArray,                                   //.objects
    omanager_OnUnlistedObject                   //.genericUOHandler
};

//--------------------------------------------------------------------------------------------------
/**
 *  Get the registered objects and resources
 *
 * @return
 *      - Registered handlers table
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Handler_t* omanager_GetHandlers
(
     void
)
{
    return &Lwm2mcoreHandlers;
}
