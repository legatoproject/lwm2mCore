/**
 * @file lwm2mcoreObjects.h
 *
 * Object manager header file
 *
 *
 * Copyright (C) Sierra Wireless Inc. Use of this work is subject to license.
 *
 */

#ifndef LWM2MCORE_OBJECTS_H
#define LWM2MCORE_OBJECTS_H

#include "lwm2mcore.h"
#include "liblwm2m.h"

//--------------------------------------------------------------------------------------------------
/** Define entry for the list inside user data structure for list element
 *
 * Example:
 *
 * typedef struct mydata {
 *      DLIST_ENTRY(mydata) list;
 *      int value;
 * } mydata_t;
 *
 * @param[in] type data type of the user data structure.
 */
//--------------------------------------------------------------------------------------------------
#define DLIST_ENTRY(type) struct { struct type *p_next, *p_prev; const void *p_head;}

//--------------------------------------------------------------------------------------------------
/** Define a new data type for the list head
 *
 * Example:
 *
 * DLIST_HEAD(mylist_head, mydata) mylist;
 *
 * Which defines a list head variable "mylist" whose type is "struct mylist_head", and the
 * list element consists of "struct mydata" as defined by DLIST_ENTRY() above.
 *
 * @param[in] name name of the list head data type.
 * @param[in] type name of the list element data type.
 */
//--------------------------------------------------------------------------------------------------
#define DLIST_HEAD(name, type) struct name{ struct type *p_first, *p_last;}

//--------------------------------------------------------------------------------------------------
/** Macro to initialize the list
 *
 * @param[in] head pointer to the list head.
 */
//--------------------------------------------------------------------------------------------------
#define DLIST_INIT(head) ((head)->p_first = (head)->p_last = NULL)

//--------------------------------------------------------------------------------------------------
/** Insert new element into the tail of the list
 *
 * @param[in] head pointer to the list head.
 * @param[in] new_elem element to be inserted.
 * @param[in] field the field name of the user data structure used as link element.
 */
//--------------------------------------------------------------------------------------------------
#define DLIST_INSERT_TAIL(head, new_elem, field) do { \
    (new_elem)->field.p_head = (const void *)head; \
    (new_elem)->field.p_next = NULL; \
    (new_elem)->field.p_prev = (head)->p_last; \
    if ((head)->p_last) (head)->p_last->field.p_next = (new_elem); \
    else (head)->p_first = (new_elem); \
    (head)->p_last = (new_elem); \
}while(0)

//--------------------------------------------------------------------------------------------------
/** Return first element from the list
 *
 * @param[in] head pointer to the list head.
 */
//--------------------------------------------------------------------------------------------------
#define DLIST_FIRST(head) ((head)->p_first)

//--------------------------------------------------------------------------------------------------
/** Return next element from the list
 *
 * @param[in] pointer to the list element whose next element will be returned.
 * @param[in] field the field name of the user data structure used as link element.
 */
//--------------------------------------------------------------------------------------------------
#define DLIST_NEXT(elem, field) ((elem)->field.p_next)

//--------------------------------------------------------------------------------------------------
/** Remove element from the list
 *
 * @param[in] head pointer to the list head.
 * @param[in] elem element to be removed.
 * @param[in] field the field name of the user data structure used as link element.
 */
//--------------------------------------------------------------------------------------------------
#define DLIST_REMOVE(head, elem, field) do { \
    if ((elem)->field.p_next) (elem)->field.p_next->field.p_prev = (elem)->field.p_prev; \
    else (head)->p_last = (elem)->field.p_prev; \
    if ((elem)->field.p_prev) (elem)->field.p_prev->field.p_next = (elem)->field.p_next; \
    else (head)->p_first = (elem)->field.p_next; \
} while(0)

//--------------------------------------------------------------------------------------------------
/** Remove element from the head of the list
 *
 * @param[in] head pointer to the list head.
 * @param[in] field the field name of the user data structure used as link element.
 */
//--------------------------------------------------------------------------------------------------
#define DLIST_REMOVE_HEAD(head, field) do { \
    (head)->p_first = (head)->p_first->field.p_next; \
    if ((head)->p_first) (head)->p_first->field.p_prev = NULL; \
    else (head)->p_last = NULL; \
} while(0)


//--------------------------------------------------------------------------------------------------
/*! \struct _lwm2m_attribute
 *  \brief data structure represent the attribute.
 */
//--------------------------------------------------------------------------------------------------
typedef struct _lwm2m_attribute
{
    int pmin;       ///< min value
    int pmax;       ///< max value
    int gt;         ///< greater than
    int lt;         ///< less than
    int st;         ///< step
    int cancel;     ///< cancel observe
    int mask;       ///< bitmask indicates what attributes are set
}lwm2m_attribute_t;

//--------------------------------------------------------------------------------------------------
/*! \struct _lwm2m_resource
 *  \brief data structure reprents a lwm2m resource.
 */
//--------------------------------------------------------------------------------------------------
typedef struct _lwm2mcore_internal_resource
{
    DLIST_ENTRY(_lwm2mcore_internal_resource) list;  ///< list entry for resource linked list
    uint16_t id;                        ///< resource id
    uint16_t iid;                       ///< resource instance id
    lwm2m_resource_type_t type;         ///< resource data type
    bool multiple;                      ///< flag indicate if this is single or multiple instances
    lwm2m_attribute_t attr;             ///< resource attributes

    /* operation handler */
    lwm2mcore_read_callback_t read;     ///< READ handler
    lwm2mcore_write_callback_t write;   ///< WRITE handler
    lwm2mcore_execute_callback_t exec;  ///< EXECUTE handler

    /* used by async notification*/
    char *cache;                        ///< cache value for OBSERVE
}lwm2mcore_internal_resource_t;

//--------------------------------------------------------------------------------------------------
/** LWM2M Core resource list data type
 */
//--------------------------------------------------------------------------------------------------
DLIST_HEAD(_lwm2m_resource_list, _lwm2mcore_internal_resource);

//--------------------------------------------------------------------------------------------------
/*! \struct _lwm2m_object
 *  \brief data structure reprents a lwm2m object.
 */
//--------------------------------------------------------------------------------------------------
typedef struct _lwm2mcore_internal_object
{
    DLIST_ENTRY(_lwm2mcore_internal_object) list;   ///< list entry for object linked list
    uint16_t id;                                    ///< object id
    uint16_t iid;                                   ///< object instance id
    bool multiple;                                  ///< flag indicate if this is single or multiple
                                                    ///< instances
    lwm2m_attribute_t attr;                         ///< object attributes
    struct _lwm2m_resource_list resource_list;      ///< resource linked list
}lwm2mcore_internal_object_t;

//--------------------------------------------------------------------------------------------------
/**
 * LWM2M Core object list data type
 */
//--------------------------------------------------------------------------------------------------
DLIST_HEAD(_lwm2mcore_objects_list, _lwm2mcore_internal_object);

//--------------------------------------------------------------------------------------------------
/**
 * Function to convert bytes(in network byte order) to integer
 *
 * @return
 *      - converted data
 */
//--------------------------------------------------------------------------------------------------
int64_t bytesToInt
(
    const uint8_t *bytes,
    size_t len
);

//--------------------------------------------------------------------------------------------------
/**
 * Macro for array size
 */
//--------------------------------------------------------------------------------------------------
#if !defined(ARRAYSIZE)
#define ARRAYSIZE(a) sizeof(a)/sizeof(a[0])
#endif

//--------------------------------------------------------------------------------------------------
/**
 * Maximum buffer length from CoAP
 */
//--------------------------------------------------------------------------------------------------
#define LWM2MCORE_BUFFER_MAX_LEN 1024

//--------------------------------------------------------------------------------------------------
/**
 * Define the server URI max length
 */
//--------------------------------------------------------------------------------------------------
#define LWM2MCORE_SERVER_URI_MAX_LEN    255

//--------------------------------------------------------------------------------------------------
/**
 * Enumeration for LWM2M objects
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    LWM2MCORE_SECURITY_OID = 0,             ///< Security
    LWM2MCORE_SERVER_OID,                   ///< Server
    LWM2MCORE_ACL_OID,                      ///< Access Control
    LWM2MCORE_DEVICE_OID,                   ///< Device
    LWM2MCORE_CONN_MONITOR_OID,             ///< Connectivity monitoring
    LWM2MCORE_FIRMWARE_UPDATE_OID,          ///< Firmware update
    LWM2MCORE_LOCATION_OID,                 ///< Location
    LWM2MCORE_CONN_STATS_OID,               ///< Connectivity statistics
    LWM2MCORE_APP_UPDATE_OID = 9,           ///< Application update
    LWM2M_SWI_SUBSCRIPTION_OID = 10241,     ///< Sierra Wireless proprietary object ID: Subscription
    LWM2M_SWI_EXT_CONN_STATS_OID,           ///< Sierra Wireless proprietary object ID: Extended
                                            ///< connectivity statistics
    LWM2M_SWI_SSL_CERTIFS_OID               ///< Sierra Wireless proprietary object ID: SSL
                                            ///< certificate
} lwm2mcore_object_enum_t;

//--------------------------------------------------------------------------------------------------
/**
 * Enumeration for LWM2M object 0 (security) resources
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    LWM2MCORE_SECURITY_SERVER_URI_RID = 0,          ///< LWM2M server URI
    LWM2MCORE_SECURITY_BOOTSTRAP_SERVER_RID,        ///< Bootstrap server (true or false)
    LWM2MCORE_SECURITY_MODE_RID,                    ///< Security mode
    LWM2MCORE_SECURITY_PKID_RID,                    ///< Public key or identity
    LWM2MCORE_SECURITY_SERVER_KEY_RID,              ///< Server public key
    LWM2MCORE_SECURITY_SECRET_KEY_RID,              ///< Secret key
    LWM2MCORE_SECURITY_SMS_SECURITY_MODE_RID,       ///< SMS security mode
    LWM2MCORE_SECURITY_SMS_BINDING_KEY_PAR_RID,     ///< SMS binding key parameters
    LWM2MCORE_SECURITY_SMS_BINDING_SEC_KEY_RID,     ///< SMS binding secret key(s)
    LWM2MCORE_SECURITY_SERVER_SMS_NUMBER_RID,       /// <LWM2M server SMS number
    LWM2MCORE_SECURITY_SERVER_ID_RID,               ///< Short server ID
    LWM2MCORE_SECURITY_CLIENT_HOLD_OFF_TIME_RID     ///< Client hold of time
}lwm2mcore_security_res_enum_t;

//--------------------------------------------------------------------------------------------------
/**
 * Enumeration for LWM2M object 1 (server) resources
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    LWM2MCORE_SERVER_SHORT_ID_RID = 0,              ///< Server short ID
    LWM2MCORE_SERVER_LIFETIME_RID,                  ///< Server lifetime
    LWM2MCORE_SERVER_DEFAULT_MIN_PERIOD_RID,        ///< Server default minimum period
    LWM2MCORE_SERVER_DEFAULT_MAX_PERIOD_RID,        ///< Server default minimum period
    LWM2MCORE_SERVER_DISABLE_RID,                   ///< Disable the device
    LWM2MCORE_SERVER_DISABLE_TIMEOUT_RID,           ///< Disable timeout
    LWM2MCORE_SERVER_STORE_NOTIF_WHEN_OFFLINE_RID,  ///< Notification storing when disabled
                                                    ///< or offline
    LWM2MCORE_SERVER_BINDING_MODE_RID,              ///< Binding
    LWM2MCORE_SERVER_REG_UPDATE_TRIGGER_RID,        ///< Registration update trigger
}lwm2mcore_server_res_enum_t;

//--------------------------------------------------------------------------------------------------
/**
* Enumeration for LWM2M object 3 (device) resources
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    LWM2MCORE_DEVICE_MANUFACTURER_RID = 0,          ///< Manufacturer
    LWM2MCORE_DEVICE_MODEL_NUMBER_RID,              ///< Model number
    LWM2MCORE_DEVICE_SERIAL_NUMBER_RID,             ///< Serial number
    LWM2MCORE_DEVICE_FIRMWARE_VERSION_RID,          ///< Firmware version
    LWM2MCORE_DEVICE_REBOOT_RID,                    ///< Reboot the device
    LWM2MCORE_DEVICE_FACTORY_RESET_RID,             ///< Factory reset request
    LWM2MCORE_DEVICE_AVAIL_POWER_SOURCES_RID,       ///< Available power sources
    LWM2MCORE_DEVICE_AVAIL_POWER_VOLTAGES_RID,      ///< Power source voltage
    LWM2MCORE_DEVICE_AVAIL_POWER_CURRENTS_RID,      ///< Power source current
    LWM2MCORE_DEVICE_BATTERY_LEVEL_RID,             ///< Battery Level
    LWM2MCORE_DEVICE_MEMORY_FREE_RID,               ///< Memory free
    LWM2MCORE_DEVICE_ERROR_CODES_RID,               ///< Error code
    LWM2MCORE_DEVICE_RESET_ERROR_CODE_RID,          ///< Reset error code
    LWM2MCORE_DEVICE_CURRENT_TIME_RID,              ///< Current time
    LWM2MCORE_DEVICE_UTC_OFFSET_RID,                ///< UTC offset
    LWM2MCORE_DEVICE_TIMEZONE_RID,                  ///< Timezone
    LWM2MCORE_DEVICE_SUPPORTED_BINDING_MODE_RID,    ///< Supported binding and modes
}lwm2mcore_device_res_enum_t;


//--------------------------------------------------------------------------------------------------
/**
* Enumeration for LWM2M object 4 (connectivity monitoring) resources
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    LWM2MCORE_CONN_MONITOR_BEARER_RID = 0,          ///< Network bearer
    LWM2MCORE_CONN_MONITOR_AVAILABLE_BEARER_RID,    ///< Available network bearer
    LWM2MCORE_CONN_MONITOR_RSSI_RID,                ///< Radio Signal Strength
    LWM2MCORE_CONN_MONITOR_LINK_QUALITY_RID,        ///< Link quality
    LWM2MCORE_CONN_MONITOR_IP_ADDR_RID,             ///< IP addresses
    LWM2MCORE_CONN_MONITOR_ROUTER_IP_ADDR_RID,      ///< Router IP addresses
    LWM2MCORE_CONN_MONITOR_LINK_UTILIZATION_RID,    ///< Link utilization
    LWM2MCORE_CONN_MONITOR_APN_RID,                 ///< APN
    LWM2MCORE_CONN_MONITOR_CELL_ID_RID,             ///< Cell ID
    LWM2MCORE_CONN_MONITOR_SMNC_RID,                ///< SMNC
    LWM2MCORE_CONN_MONITOR_SMCC_RID,                ///< SMCC
}lwm2mcore_conn_monitor_res_enum_t;


//--------------------------------------------------------------------------------------------------
/**
* Enumeration for LWM2M object 5 (firmware update) resources
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    LWM2MCORE_FW_UPDATE_PACKAGE_RID = 0,        ///< Package
    LWM2MCORE_FW_UPDATE_PACKAGE_URI_RID,        ///< Package URI
    LWM2MCORE_FW_UPDATE_UPDATE_RID,             ///< Update
    LWM2MCORE_FW_UPDATE_UPDATE_STATE_RID,       ///< State
    LWM2MCORE_FW_UPDATE_SUPPPORTED_OBJ_RID,     ///< Update supported objects
    LWM2MCORE_FW_UPDATE_UPDATE_RESULT_RID,      ///< Update result
    LWM2MCORE_FW_UPDATE_PACKAGE_NAME_RID,       ///< Package name
    LWM2MCORE_FW_UPDATE_PACKAGE_VERSION_RID,    ///< Package version
    LWM2MCORE_FW_UPDATE_PROTO_SUPPORT_RID,      ///< Fw update protocol support
    LWM2MCORE_FW_UPDATE_DELIVERY_METHOD_RID,    ///< Fw update delivery method
}lwm2mcore_fw_update_res_enum_t;

#if LWM2MCORE_GPS
//--------------------------------------------------------------------------------------------------
/**
* Enumeration for LWM2M object 6 (location) resources
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    LWM2MCORE_LOCATION_LATITUDE_RID = 0,    ///< Latitude
    LWM2MCORE_LOCATION_LONGITUDE_RID,       ///< Longitude
    LWM2MCORE_LOCATION_ALTITUDE_RID,        ///< Altitude
    LWM2MCORE_LOCATION_RADIUS_RID,          ///< Radius
    LWM2MCORE_LOCATION_VELOCITY_RID,        ///< Velocity
    LWM2MCORE_LOCATION_TIMESTAMP_RID,       ///< Timestamp
    LWM2MCORE_LOCATION_SPEED_RID,           ///< Speed
}lwm2mcore_location_res_enum_t;
#endif /* LWM2MCORE_GPS */

//--------------------------------------------------------------------------------------------------
/**
* Enumeration for LWM2M object 7 (connectivity statistics) resources
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    LWM2MCORE_CONN_STATS_SMS_TX_COUNT_RID = 0,  ///< SMS Tx counter
    LWM2MCORE_CONN_STATS_SMS_RX_COUNT_RID,      ///< SMS Rx counter
    LWM2MCORE_CONN_STATS_DATA_TX_COUNT_RID,     ///< Tx data
    LWM2MCORE_CONN_STATS_DATA_RX_COUNT_RID,     ///< Rx data
    LWM2MCORE_CONN_STATS_MAX_MSG_SIZE_RID,      ///< Max message size
    LWM2MCORE_CONN_STATS_AVERAGE_MSG_SIZE_RID,  ///< Average message size
    LWM2MCORE_CONN_STATS_START_RID,             ///< Start
    LWM2MCORE_CONN_STATS_STOP_RID,              ///< Stop
    LWM2MCORE_CONN_STATS_COLLECT_PERIOD_RID,    ///< Collection period
    LWM2MCORE_CONN_STATS_COLLECT_DURATION_RID,  ///< Collection duration
}lwm2mcore_conn_stats_res_enum_t;

//--------------------------------------------------------------------------------------------------
/**
* Enumeration for LWM2M object 9 (software update) resources
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    LWM2MCORE_SW_UPDATE_PACKAGE_NAME_RID = 0,   ///< Package name
    LWM2MCORE_SW_UPDATE_PACKAGE_VERSION_RID,    ///< Package version
    LWM2MCORE_SW_UPDATE_PACKAGE_RID,            ///< Package
    LWM2MCORE_SW_UPDATE_PACKAGE_URI_RID,        ///< Package URI
    LWM2MCORE_SW_UPDATE_INSTALL_RID,            ///< Install
    LWM2MCORE_SW_UPDATE_CHECKPOINT_RID,         ///< Checkpoint
    LWM2MCORE_SW_UPDATE_UNINSTALL_RID,          ///< Uninstall
    LWM2MCORE_SW_UPDATE_UPDATE_STATE_RID,       ///< Update state
    LWM2MCORE_SW_UPDATE_UPDATE_SUPP_OBJ_RID,    ///< Update supported objects
    LWM2MCORE_SW_UPDATE_UPDATE_RESULT_RID,      ///< Update result
    LWM2MCORE_SW_UPDATE_ACTIVATE_RID,           ///< Activate
    LWM2MCORE_SW_UPDATE_DEACTIVATE_RID,         ///< Deactivate
    LWM2MCORE_SW_UPDATE_ACTIVATION_STATE_RID,   ///< Activation state
    LWM2MCORE_SW_UPDATE_PACKAGE_SETTINGS_RID,   ///< Package settings
}lwm2mcore_sw_update_res_enum_t;

//--------------------------------------------------------------------------------------------------
/**
* Enumeration for LWM2M object 10241 (subscription) resources
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    LWM2MCORE_SUBSCRIPTION_MODULE_IDENTITY_RID = 0, ///< Module identity (IMEI)
    LWM2MCORE_SUBSCRIPTION_ICCID_RID,               ///< ICCID
    LWM2MCORE_SUBSCRIPTION_IDENTITY_RID,            ///< Subscription identity
    LWM2MCORE_SUBSCRIPTION_NUMBER_RID,              ///< Subscription number
}lwm2mcore_subscription_res_enum_t;

//--------------------------------------------------------------------------------------------------
/**
* Enumeration for LWM2M object 10242 (extended connectivity statistics) resources
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    LWM2MCORE_EXT_CONN_STATS_SIGNAL_BAR_RID = 0,        ///< Signal bar
    LWM2MCORE_EXT_CONN_STATS_CELLULAR_TECH_USED_RID,    ///< Cellular technology used
    LWM2MCORE_EXT_CONN_STATS_ROAMING_INDICATOR_RID,     ///< Roaming indicator
    LWM2MCORE_EXT_CONN_STATS_ECIO_RID,                  ///< Ec/Io
    LWM2MCORE_EXT_CONN_STATS_RSRP_RID,                  ///< RSRP
    LWM2MCORE_EXT_CONN_STATS_RSRQ_RID,                  ///< RSRQ
    LWM2MCORE_EXT_CONN_STATS_RSCP_RID,                  ///< RSCP
    LWM2MCORE_EXT_CONN_DEVICE_TEMPERATURE_RID,          ///< Device temperature
    LWM2MCORE_EXT_CONN_UNEXPECTED_RESET_COUNT_RID,      ///< Unexpected reset counter
    LWM2MCORE_EXT_CONN_TOTAL_RESET_COUNT_RID,           ///< Total reset counter
}lwm2mcore_ext_conn_stats_res_enum_t;

//--------------------------------------------------------------------------------------------------
/**
* Enumeration for LWM2M object 10243 (SSL certificates) resources
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    LWM2MCORE_SSL_CERTIFICATE_CERTIF         = 0    ///< SSL certificates
} lwm2mcore_ssl_certificate_res_enum_t;


#endif /* LWM2MCORE_OBJECTS_H */

