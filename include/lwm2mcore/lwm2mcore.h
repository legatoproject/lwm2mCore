/**
 * @file lwm2mcore.h
 *
 * Header file for LWM2M Core
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#ifndef  __LWM2MCORE_H__
#define  __LWM2MCORE_H__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

//--------------------------------------------------------------------------------------------------
/**
 * Maximum LWM2M server supported, though only one used at any time
 */
//--------------------------------------------------------------------------------------------------
#define LWM2MCORE_DM_SERVER_MAX_COUNT           1

//--------------------------------------------------------------------------------------------------
/**
 * Maximum LWM2M bootstrap server supported
 */
//--------------------------------------------------------------------------------------------------
#define LWM2MCORE_BOOTSRAP_SERVER_MAX_COUNT     1

//--------------------------------------------------------------------------------------------------
/**
 * Maximum length of a resource name
 */
//--------------------------------------------------------------------------------------------------
#define LWM2MCORE_NAME_LEN                      64

//--------------------------------------------------------------------------------------------------
/**
 * Maximum length of a device endpoint
 * Endpoint can be:
 * IMEI: 15 digits
 * ESN: 8 digits
 * MEID: 14 digits
 */
//--------------------------------------------------------------------------------------------------
#define LWM2MCORE_ENDPOINT_LEN                  16

//--------------------------------------------------------------------------------------------------
/**
 * Define values to indicate that an object can be supported without any defined ressource
 */
//--------------------------------------------------------------------------------------------------
#define LWM2MCORE_ID_NONE                       0xFFFF

//--------------------------------------------------------------------------------------------------
/**
 * Specific defines for OMA FUMO (Firmware Update Management Object)
 * Values defined in ETSI 118 105 Table 5.4.2.1-1
 */
//--------------------------------------------------------------------------------------------------
#define LWM2MCORE_FUMO_CORRUPTED_PKG            402     ///< Corrupted package
#define LWM2MCORE_FUMO_FAILED_VALIDATION        404     ///< Failed package validation
#define LWM2MCORE_FUMO_UNSUPPORTED_PKG          405     ///< Unsupported package
#define LWM2MCORE_FUMO_INVALID_URI              411     ///< Invalid URI
#define LWM2MCORE_FUMO_ALTERNATE_DL_ERROR       500     ///< Download error
#define LWM2MCORE_FUMO_NO_SUFFICIENT_MEMORY     501     ///< Insufficient memory

//--------------------------------------------------------------------------------------------------
/**
 * Enumeration for handlers status ID code (returned value)
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    LWM2MCORE_ERR_ASYNC_OPERATION       = 1,    ///< Handler will respond asynchronously.
    LWM2MCORE_ERR_COMPLETED_OK          = 0,    ///< Handler treatment is OK
    LWM2MCORE_ERR_GENERAL_ERROR         = -1,   ///< Handler treatment failed
    LWM2MCORE_ERR_INCORRECT_RANGE       = -2,   ///< Bad parameter range (WRITE operation)
    LWM2MCORE_ERR_NOT_YET_IMPLEMENTED   = -3,   ///< Not yet implemented resource
    LWM2MCORE_ERR_OP_NOT_SUPPORTED      = -4,   ///< Not supported resource
    LWM2MCORE_ERR_INVALID_ARG           = -5,   ///< Invalid parameter in resource handler
    LWM2MCORE_ERR_INVALID_STATE         = -6,   ///< Invalid state to treat the resource handler
    LWM2MCORE_ERR_OVERFLOW              = -7    ///< Buffer overflow
}lwm2mcore_Sid_t;

//--------------------------------------------------------------------------------------------------
/**
 * Events for status callback
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    LWM2MCORE_EVENT_INITIALIZED                    = 0,     ///< The AL subsystem is initialized and
                                                            ///< ready to be used.
    LWM2MCORE_EVENT_AGREEMENT_CONNECTION           = 1,     ///< The device wants a user agreement
                                                            ///< to make a connection to the server.
    LWM2MCORE_EVENT_AGREEMENT_DOWNLOAD             = 2,     ///< The device wants a user agreement
                                                            ///< to make a connection to the server.
    LWM2MCORE_EVENT_AGREEMENT_UPDATE               = 3,     ///< The device wants a user agreement
                                                            ///< to install a downloaded package.
    LWM2MCORE_EVENT_AUTHENTICATION_STARTED         = 4,     ///< The OTA update client has started
                                                            ///< authentication with the server.
    LWM2MCORE_EVENT_AUTHENTICATION_FAILED          = 5,     ///< The OTA update client failed to
                                                            ///< authenticate with the server.
    LWM2MCORE_EVENT_SESSION_STARTED                = 6,     ///< The OTA update client succeeded in
                                                            ///< authenticating with the server and
                                                            ///< has started the session.
    LWM2MCORE_EVENT_SESSION_FAILED                 = 7,     ///< The session with the server failed.
    LWM2MCORE_EVENT_SESSION_FINISHED               = 8,     ///< The with the server finished
                                                            ///< successfully.
    LWM2MCORE_EVENT_PACKAGE_DOWNLOAD_DETAILS       = 9,     ///< A descriptor was downloaded with
                                                            ///< the package size.
    LWM2MCORE_EVENT_PACKAGE_DOWNLOAD_FINISHED      = 10,    ///< The OTA update package downloaded
                                                            ///< successfully.
    LWM2MCORE_EVENT_PACKAGE_DOWNLOAD_FAILED        = 11,    ///< The OTA update package downloaded
                                                            ///< successfully, but could not be
                                                            ///< stored in flash.
    LWM2MCORE_EVENT_PACKAGE_CERTIFICATION_OK       = 12,    ///< The OTA update package was
                                                            ///< certified to have come from right
                                                            ///< server.
    LWM2MCORE_EVENT_PACKAGE_CERTIFICATION_NOT_OK   = 13,    ///< The OTA update package was not
                                                            ///< certified to have come from right
                                                            ///<server.
    LWM2MCORE_EVENT_UPDATE_STARTED                 = 14,    ///< An update package is being applied.
    LWM2MCORE_EVENT_UPDATE_FAILED                  = 15,    ///< The update failed.
    LWM2MCORE_EVENT_UPDATE_FINISHED                = 16,    ///< The update succeeded.
    LWM2MCORE_EVENT_FALLBACK_STARTED               = 17,    ///< A fallback mechanism was started.
    LWM2MCORE_EVENT_DOWNLOAD_PROGRESS              = 18,    ///< Indicate the download %
    LWM2MCORE_EVENT_LWM2M_SESSION_TYPE_START       = 23,    ///< LWM2M Event to know if the session
                                                            ///< is a Bootstrap or a Device
                                                            ///< Management one
    /* NEW EVENT TO BE ADDED BEFORE THIS COMMENT */
    LWM2MCORE_EVENT_LAST                           = 24     ///< Internal usage
}lwm2mcore_StatusType_t;

//--------------------------------------------------------------------------------------------------
/**
 * Enumration for LWM2M operation
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    LWM2MCORE_OP_READ         = 0x01,             ///< standard DM: read
    LWM2MCORE_OP_DISCOVER     = 0x02,             ///< standard DM: discover
    LWM2MCORE_OP_WRITE        = 0x04,             ///< standard DM: write
    LWM2MCORE_OP_WRITE_ATTR   = 0x08,             ///< standard DM: write attributes
    LWM2MCORE_OP_EXECUTE      = 0x10,             ///< standard DM: execute
    LWM2MCORE_OP_CREATE       = 0x20,             ///< standard DM: create
    LWM2MCORE_OP_DELETE       = 0x40,             ///< standard DM: delete
    LWM2MCORE_OP_OBSERVE      = 0x80,             ///< observe
    LWM2MCORE_OP_QUERY_INSTANCE_COUNT = 0x100     ///< custom: query resource instance count
}lwm2mcore_OpType_t;

//--------------------------------------------------------------------------------------------------
/**
 * Enumeration for LWM2M resource data type
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    LWM2MCORE_RESOURCE_TYPE_INT = 0,        ///< resource type: integer
    LWM2MCORE_RESOURCE_TYPE_BOOL,           ///< resource type: boolean
    LWM2MCORE_RESOURCE_TYPE_STRING,         ///< resource type: string
    LWM2MCORE_RESOURCE_TYPE_OPAQUE,         ///< resource type: opaque
    LWM2MCORE_RESOURCE_TYPE_FLOAT,          ///< resource type: float
    LWM2MCORE_RESOURCE_TYPE_TIME,           ///< resource type: time
    LWM2MCORE_RESOURCE_TYPE_UNKNOWN         ///< resource type: unknown
}lwm2m_ResourceType_t;

//--------------------------------------------------------------------------------------------------
/**
 * Enumeration of supported LWM2M credentials
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    LWM2MCORE_CREDENTIAL_FW_KEY = 0,            ///< FW public key
    LWM2MCORE_CREDENTIAL_SW_KEY,                ///< SW public key
    LWM2MCORE_CREDENTIAL_CERTIFICATE,           ///< Certificate for HTTPS
    LWM2MCORE_CREDENTIAL_BS_PUBLIC_KEY,         ///< LWM2M Client’s Certificate (Certificate mode),
                                                ///< public key (RPK mode) or PSK Identity (PSK
                                                ///< mode)
    LWM2MCORE_CREDENTIAL_BS_SERVER_PUBLIC_KEY,  ///< LWM2M Server’s or LWM2M Bootstrap Server’s
                                                ///< Certificate (Certificate mode), public key
                                                ///< (RPK mode)
    LWM2MCORE_CREDENTIAL_BS_SECRET_KEY,         ///< secret key or private key of the security mode
    LWM2MCORE_CREDENTIAL_BS_ADDRESS,            ///< BS server address
    LWM2MCORE_CREDENTIAL_DM_PUBLIC_KEY,         ///< LWM2M Client’s Certificate (Certificate mode),
                                                ///< public key (RPK mode) or PSK Identity (PSK
                                                ///< mode)
    LWM2MCORE_CREDENTIAL_DM_SERVER_PUBLIC_KEY,  ///< LWM2M Server’s or LWM2M Bootstrap Server’s
                                                ///< Certificate (Certificate mode), public key
                                                ///< (RPK mode)
    LWM2MCORE_CREDENTIAL_DM_SECRET_KEY,         ///< secret key or private key of the security mode
    LWM2MCORE_CREDENTIAL_DM_ADDRESS,            ///< DM server address
    LWM2MCORE_CREDENTIAL_MAX                    ///< Internal usage
}lwm2mcore_Credentials_t;

//--------------------------------------------------------------------------------------------------
/**
 * Enumeration for device endpoint urn format: LWM2M TS v1.0 6.2.1
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    UUID_URN,       ///< UUID
    OPS_URN,        ///< QPS
    OS_URN,         ///< OS
    IMEI_URN,       ///< IMEI
    ESN_URN,        ///< ESN
    MEID_URN        ///< MEID
}lwm2mcore_EpnType_t;

//--------------------------------------------------------------------------------------------------
/**
 * Enumeration for session type
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    LWM2MCORE_SESSION_BOOTSTRAP,            ///< Bootstrap session
    LWM2MCORE_SESSION_DEVICE_MANAGEMENT     ///< Device management session
}lwm2mcore_SessionType_t;

//--------------------------------------------------------------------------------------------------
/**
 * Enum for package type
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    LWM2MCORE_PKG_NONE,   ///< Default value
    LWM2MCORE_PKG_FW,     ///< Package for firmware
    LWM2MCORE_PKG_SW      ///< Package for software
}lwm2mcore_PkgDwlType_t;

//--------------------------------------------------------------------------------------------------
/**
 * CoAP URI representation
 * \brief data structure represent the lwm2m request URI, obtained by parsing CoAP URI options.
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    lwm2mcore_OpType_t op;                 ///< operation type
    int contentType;                       ///< payload content type, a value of -1 indicates
                                           ///< content type is not specified
    int acceptContentType;                 ///< server accept content type, a value of -1 indicates
                                           ///< its not specified from server
    bool observe;                          ///< if the request come with OBSERVE option
    int observeValue;                      ///< OBSERVE option value
    uint16_t oid;                          ///< object Id
    uint16_t oiid;                         ///< object instance Id
    uint16_t rid;                          ///< resource Id
    uint16_t riid;                         ///< resource instance Id
    uint32_t blockNum;                     ///< block number for CoAP block xfer
    uint16_t blockSize;                    ///< size of block for CoAP block xfer
    bool lastBlock;                        ///< if this is last block in CoAP block xfer
    int pathLen;                           ///< Len of alternative path (CoRE Link Format RFC6690),
                                           ///< 0 for default LWM2M standard object
    bool standardPath;                     ///< flag indicate if the path name is lwm2m std path
                                           ///< "lwm2m"
    uint8_t  pathName[LWM2MCORE_NAME_LEN]; ///< alternative path name
}lwm2mcore_Uri_t;

//--------------------------------------------------------------------------------------------------
/**
 * Structure for a package download status
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    lwm2mcore_PkgDwlType_t pkgType;     ///< Package type
    uint32_t numBytes;                  ///< For package download, downloaded bytes
    uint32_t progress;                  ///< For package download, package download progress in %
    uint32_t errorCode;                 ///< For package download, error code
}lwm2mcore_PkgDwlStatus_t;

//--------------------------------------------------------------------------------------------------
/**
 * Structure for session event
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    lwm2mcore_SessionType_t type;           ///< Session type for
                                            ///< LWM2MCORE_EVENT_LWM2M_SESSION_TYPE_START event
}lwm2mcore_SessionStatus_t;

//--------------------------------------------------------------------------------------------------
/**
 * Structure for events (session and package download)
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    lwm2mcore_StatusType_t event;               ///< Event
    union
    {
        lwm2mcore_SessionStatus_t session;      ///< Session information
        lwm2mcore_PkgDwlStatus_t pkgStatus;     ///< Package download status
    }u;                                         ///< Union
}lwm2mcore_Status_t;

//--------------------------------------------------------------------------------------------------
/**
 * function pointer of resource value changed callback function.
 *
 * @return
 *      - 0 on success
 *      - negative value on failure.
 */
//--------------------------------------------------------------------------------------------------
typedef int (*valueChangedCallback_t)
(
    lwm2mcore_Uri_t *uri,               ///< [IN] uri represents the resource whose value is changed
    char *buffer,                       ///< [INOUT] buffer of the notification data
    int len                             ///< [IN] llength of the notification data
);

//--------------------------------------------------------------------------------------------------
/**
 * function pointer of resource READ function.
 *
 * @return
 *      - 0 on success
 *      - negative value on failure
 *      - > 0 values for asynchronous operations
 */
//--------------------------------------------------------------------------------------------------
typedef int (*lwm2mcore_ReadCallback_t)
(
    lwm2mcore_Uri_t *uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource.
    char *bufferPtr,                    ///< [INOUT] data buffer for information
    size_t *lenPtr,                     ///< [INOUT] length of input buffer and length of the
                                        ///< returned data
    valueChangedCallback_t changedCb    ///< [IN] not used for READ operation
);

//--------------------------------------------------------------------------------------------------
/**
 * function pointer of resource WRITE/OBSERVE function.
 *
 * @return
 *      - 0 on success
 *      - negative value on failure
 *      - > 0 values for asynchronous operations
 */
//--------------------------------------------------------------------------------------------------
typedef int (*lwm2mcore_WriteCallback_t)
(
    lwm2mcore_Uri_t *uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource.
    char *bufferPtr,                    ///< [INOUT] data buffer for information
    size_t len                          ///< [IN] length of input buffer
);

//--------------------------------------------------------------------------------------------------
/**
 * Function pointer of resource EXECUTE function.
 *
 * @return
 *      - 0 on success
 *      - negative value on failure
 *      - > 0 values for asynchronous ops.
 */
//--------------------------------------------------------------------------------------------------
typedef int (*lwm2mcore_ExecuteCallback_t)
(
    lwm2mcore_Uri_t *uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource.
    char *bufferPtr,                    ///< [INOUT] contain arguments
    size_t len                          ///< [IN] length of buffer
);

//--------------------------------------------------------------------------------------------------
/**
 * Structure for an object resource
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    uint16_t id;                        ///< resource id: one of the LWM2M_xyz_RID above
    lwm2m_ResourceType_t type;          ///< resource data type
    uint16_t maxResInstCnt;             ///< maximum number of resource instance
                                        ///< count. 1 means single instance
    lwm2mcore_ReadCallback_t read;      ///< operation handler: READ handler
    lwm2mcore_WriteCallback_t write;    ///< operation handler: WRITE handler
    lwm2mcore_ExecuteCallback_t exec;   ///< operation handler: EXECUTE handler
}lwm2mcore_Resource_t;

//--------------------------------------------------------------------------------------------------
/**
 * Structure for an object
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    uint16_t id;                            ///< one of the defined object id
    uint16_t maxObjInstCnt;                 ///< maximum number of object instance
                                            ///< count. 1 means single instance.
    uint16_t resCnt;                        ///< number of resource count under this object
    lwm2mcore_Resource_t * resources;       ///< pointer to the list of resource under this object
}lwm2mcore_Object_t;

//--------------------------------------------------------------------------------------------------
/**
 * function pointer of resource READ/WRITE/OBSERVE function.
 *
 * @return
 *      - 0 on success
 *      - negative value on failure
 *      -  > 0 values for asynchronous ops.
*/
//--------------------------------------------------------------------------------------------------
typedef int (*genericReadWriteApi_t)
(
    lwm2mcore_Uri_t *uri,               ///< [IN] uri represents the requested operation and
                                        ///< object/resource.
    char *buffer,                       ///< [INOUT] data buffer for information
    size_t *len,                        ///< [INOUT] length of input buffer and length of the
                                        ///< returned data
    valueChangedCallback_t changedCb    ///< [IN] callback function pointer for OBSERVE operation
);

//--------------------------------------------------------------------------------------------------
/**
 * Structure for a handler
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    uint16_t                objCnt;                 ///< Numbers of supported pre-defined objects
    lwm2mcore_Object_t*     objects;                ///< Actual supported pre-defined objects
    genericReadWriteApi_t   genericUOHandler;       ///< Generic handler for unidentified object
}lwm2mcore_Handler_t;

//--------------------------------------------------------------------------------------------------
/**
 * Callback for event status
 *
 * @return
 *      - 0 on success
 *      - negative value on failure.
 */
//--------------------------------------------------------------------------------------------------
typedef int (*lwm2mcore_StatusCb_t)
(
    lwm2mcore_Status_t eventStatus              ///< [IN] event status
);

//--------------------------------------------------------------------------------------------------
/**
 * Initialize the LWM2M core
 *
 * @return
 *  - context address
 *  - -1 in case of error
 */
//--------------------------------------------------------------------------------------------------
int lwm2mcore_Init
(
    lwm2mcore_StatusCb_t eventCb    ///< [IN] event callback
);

//--------------------------------------------------------------------------------------------------
/**
 * Free the LWM2M core
 *
 */
//--------------------------------------------------------------------------------------------------
void lwm2mcore_Free
(
    int context     ///< [IN] context
);

//--------------------------------------------------------------------------------------------------
/**
 *  Register the object table and service API
 *
 * @note If handlerPtr parameter is NULL, LWM2MCore registers it's own "standard" object list
 *
 * @return
 *      - number of registered objects
 */
//--------------------------------------------------------------------------------------------------
uint16_t lwm2mcore_ObjectRegister
(
    int context,                            ///< [IN] Context
    char* endpoint,                         ///< [IN] Device endpoint
    lwm2mcore_Handler_t* const handlerPtr,  ///< [IN] List of supported object/resource by client
                                            ///< This parameter can be set to NULL
    void* const servicePtr                  ///< [IN] Client service API table
);

//--------------------------------------------------------------------------------------------------
/**
 * LWM2M client entry point to initiate a connection
 *
 * @return
 *      - true if the treatment is launched
 *      - else false
 */
//--------------------------------------------------------------------------------------------------
bool lwm2mcore_Connect
(
    int context     ///< [IN] context
);

//--------------------------------------------------------------------------------------------------
/**
 * Function to close a connection
 *
 * @return
 *      - true if the treatment is launched
 *      - else false
 */
//--------------------------------------------------------------------------------------------------
bool lwm2mcore_Disconnect
(
    int context     ///< [IN] context
);

//--------------------------------------------------------------------------------------------------
/**
 * Function to send an update message to the Device Management server
 * This API can be used when the application wants to send a notification or during a firmware/app
 * update in order to be able to fully treat the scheduled update job
 *
 * @return
 *      - true if the treatment is launched
 *      - else false
 */
//--------------------------------------------------------------------------------------------------
bool lwm2mcore_Update
(
    int context     ///< [IN] context
);

//--------------------------------------------------------------------------------------------------
/**
 * Function to know what is the current connection
 *
 * @return
 *      - true if the device is connected to any server
 *      - else false
 */
//--------------------------------------------------------------------------------------------------
bool lwm2mcore_ConnectionGetType
(
    int context,                ///< [IN] context
    bool* isDeviceManagement    ///< [INOUT] Session type (false: bootstrap,
                                ///< true: device management)
);


//--------------------------------------------------------------------------------------------------
/**
 * Function to push data to lwm2mCore
 *
 * @return
 *      - true if a data push transaction is initiated
 *      - else false
 */
//--------------------------------------------------------------------------------------------------
bool lwm2mcore_Push
(
    int context,                ///< [IN] context
    uint8_t* payloadPtr,        ///< [IN] payload
    size_t payloadLength,       ///< [IN] payload length
    void* callbackPtr           ///< [IN] callback for payload
);

//--------------------------------------------------------------------------------------------------
/**
 * Function to notify LWM2MCore of supported object instance list for software and asset data
 *
 * @return
 *      - true if the list was successfully treated
 *      - else false
 */
//--------------------------------------------------------------------------------------------------
bool lwm2mcore_UpdateSwList
(
    int context,                    ///< [IN] Context (Set to 0 if this API is used if
                                    ///< lwm2mcore_init API was no called)
    const char* ListPtr,            ///< [IN] Formatted list
    size_t listLen                  ///< [IN] Size of the update list
);

//--------------------------------------------------------------------------------------------------
/**
 * Function for assert
 */
//--------------------------------------------------------------------------------------------------
void lwm2mcore_Assert
(
    bool condition,         /// [IN] Condition to be checked
    const char* function,   /// [IN] Function name which calls the assert function
    uint32_t line           /// [IN] Function line which calls the assert function
);

//--------------------------------------------------------------------------------------------------
/**
 * Macro for assertion
 */
//--------------------------------------------------------------------------------------------------
#define LWM2MCORE_ASSERT(X) lwm2mcore_Assert(X, __func__, __LINE__)

//--------------------------------------------------------------------------------------------------
/**
 * Adaptation function for log: dump data
 */
//--------------------------------------------------------------------------------------------------
void lwm2mcore_DataDump
(
    char* descPtr,              ///< [IN] data description
    void* addrPtr,              ///< [IN] Data address to be dumped
    int len                     ///< [IN] Data length
);

#endif /*  __LWM2MCORE_H__ */
