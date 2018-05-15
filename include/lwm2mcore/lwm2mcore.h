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
#include <platform/types.h>
#include <stddef.h>

/**
  * @addtogroup lwm2mcore_config_IFS
  * @{
  */
//--------------------------------------------------------------------------------------------------
/**
 * @brief Maximum length of a resource name
 */
//--------------------------------------------------------------------------------------------------
#define LWM2MCORE_NAME_LEN                      64

//--------------------------------------------------------------------------------------------------
/**
 * @brief Define the server URI max length
 */
//--------------------------------------------------------------------------------------------------
#define LWM2MCORE_SERVER_URI_MAX_LEN            255

//--------------------------------------------------------------------------------------------------
/**
 * @brief Maximum length of a device endpoint
 *
 * Endpoint can be:
 * IMEI: 15 digits
 * ESN: 8 digits
 * MEID: 14 digits
 */
//--------------------------------------------------------------------------------------------------
#define LWM2MCORE_ENDPOINT_LEN                  16

//--------------------------------------------------------------------------------------------------
/**
 * @brief Define values to indicate that an object can be supported without any defined ressource
 */
//--------------------------------------------------------------------------------------------------
#define LWM2MCORE_ID_NONE                       0xFFFF

//--------------------------------------------------------------------------------------------------
/**
 * @brief Define values for security maximum length (object 0)
 */
//--------------------------------------------------------------------------------------------------
#define LWM2MCORE_SECURITY_KEY_LEN              128

//--------------------------------------------------------------------------------------------------
/**
 * @brief Define values for SMS binding key length (object 0)
 */
//--------------------------------------------------------------------------------------------------
#define LWM2MCORE_SBK_LEN                       6

//--------------------------------------------------------------------------------------------------
/**
 * @brief Define values for SMS binding secret key length (object 0)
 */
//--------------------------------------------------------------------------------------------------
#define LWM2MCORE_SBSK_LEN                      48

//--------------------------------------------------------------------------------------------------
/**
 * @brief Define values for phone number length
 */
//--------------------------------------------------------------------------------------------------
#define LWM2MCORE_PHONENUMBER_MAX_LEN           64

//--------------------------------------------------------------------------------------------------
/**
 * Lifetime value to indicate that the lifetime is deactivated
 * This is compliant with the LWM2M specification and a 0-value has no sense
 * 630720000 = 20 years
 * This is used if the customer does not wan any "automatic" connection to the server
 */
//--------------------------------------------------------------------------------------------------
#define LWM2MCORE_LIFETIME_VALUE_DISABLED       630720000
/**
  * @}
  */

/**
  * @addtogroup lwm2mcore_update_IFS
  * @{
  */
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
/**
  * @}
  */

/**
  * @addtogroup lwm2mcore_init_IFS
  * @{
  */
//--------------------------------------------------------------------------------------------------
/**
 *  @brief Enumeration for handlers status ID code (returned value)
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
 * @brief Events for status callback
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    LWM2MCORE_EVENT_INITIALIZED                    = 0,     ///< LwM2MCore is initialized and ready to be used.
    LWM2MCORE_EVENT_AGREEMENT_CONNECTION           = 1,     ///< The device wants a user agreementto make a connection to the server.
    LWM2MCORE_EVENT_AGREEMENT_DOWNLOAD             = 2,     ///< The device wants a user agreement to make a connection to the server.
    LWM2MCORE_EVENT_AGREEMENT_UPDATE               = 3,     ///< The device wants a user agreement to install a downloaded package.
    LWM2MCORE_EVENT_AUTHENTICATION_STARTED         = 4,     ///< The OTA update client has started authentication with the server.
    LWM2MCORE_EVENT_AUTHENTICATION_FAILED          = 5,     ///< The OTA update client failed to authenticate with the server.
    LWM2MCORE_EVENT_SESSION_STARTED                = 6,     ///< The OTA update client succeeded in authenticating with the server and has started the session.
    LWM2MCORE_EVENT_SESSION_FAILED                 = 7,     ///< The session with the server failed.
    LWM2MCORE_EVENT_SESSION_FINISHED               = 8,     ///< The session with the server finished successfully.
    LWM2MCORE_EVENT_PACKAGE_DOWNLOAD_DETAILS       = 9,     ///< A descriptor was downloaded with the package size.
    LWM2MCORE_EVENT_PACKAGE_DOWNLOAD_FINISHED      = 10,    ///< The OTA update package downloaded successfully.
    LWM2MCORE_EVENT_PACKAGE_DOWNLOAD_FAILED        = 11,    ///< The OTA update package downloaded successfully, but could not be stored in flash.
    LWM2MCORE_EVENT_PACKAGE_CERTIFICATION_OK       = 12,    ///< The OTA update package was certified to have been sent by a trusted server.
    LWM2MCORE_EVENT_PACKAGE_CERTIFICATION_NOT_OK   = 13,    ///< The OTA update package was not certified to have been sent by a trusted server.
    LWM2MCORE_EVENT_UPDATE_STARTED                 = 14,    ///< An update package is being applied.
    LWM2MCORE_EVENT_UPDATE_FAILED                  = 15,    ///< The update failed.
    LWM2MCORE_EVENT_UPDATE_FINISHED                = 16,    ///< The update succeeded.
    LWM2MCORE_EVENT_FALLBACK_STARTED               = 17,    ///< A fallback mechanism was started.
    LWM2MCORE_EVENT_DOWNLOAD_PROGRESS              = 18,    ///< Indicate the download %
    LWM2MCORE_EVENT_LWM2M_SESSION_TYPE_START       = 23,    ///< LWM2M Event to know if the session is a Bootstrap or a Device Management one
    LWM2MCORE_EVENT_LWM2M_SESSION_INACTIVE         = 24,    ///< LWM2M Event to know if the session is inactive for 20 sec
    /* NEW EVENT TO BE ADDED BEFORE THIS COMMENT */
    LWM2MCORE_EVENT_LAST                           = 25     ///< Internal usage
}lwm2mcore_StatusType_t;
/**
  * @}
  */

/**
  * @addtogroup lwm2mcore_config_IFS
  * @{
  */
//--------------------------------------------------------------------------------------------------
/**
 * @brief Enumration for LWM2M operation
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    LWM2MCORE_OP_READ         = 0x01,             ///< Read command
    LWM2MCORE_OP_DISCOVER     = 0x02,             ///< Discover command
    LWM2MCORE_OP_WRITE        = 0x04,             ///< Write command
    LWM2MCORE_OP_WRITE_ATTR   = 0x08,             ///< Write attributes command
    LWM2MCORE_OP_EXECUTE      = 0x10,             ///< Execute command
    LWM2MCORE_OP_CREATE       = 0x20,             ///< Create command
    LWM2MCORE_OP_DELETE       = 0x40,             ///< Delete command
    LWM2MCORE_OP_OBSERVE      = 0x80,             ///< Observe command
    LWM2MCORE_OP_QUERY_INSTANCE_COUNT = 0x100     ///< Custom: query resource instance count
}lwm2mcore_OpType_t;

//--------------------------------------------------------------------------------------------------
/**
 * @brief Enumeration for LWM2M resource data type
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    LWM2MCORE_RESOURCE_TYPE_INT = 0,        ///< Integer resource type
    LWM2MCORE_RESOURCE_TYPE_BOOL,           ///< Boolean resource type
    LWM2MCORE_RESOURCE_TYPE_STRING,         ///< String resource type
    LWM2MCORE_RESOURCE_TYPE_OPAQUE,         ///< Opaque resource type
    LWM2MCORE_RESOURCE_TYPE_FLOAT,          ///< Float resource type
    LWM2MCORE_RESOURCE_TYPE_TIME,           ///< Time resource type
    LWM2MCORE_RESOURCE_TYPE_UNKNOWN         ///< Unknown resource type
}lwm2mcore_ResourceType_t;
/**
  * @}
  */

/**
  * @addtogroup lwm2mcore_security_IFS
  * @{
  */
//--------------------------------------------------------------------------------------------------
/**
 * @brief Enumeration for the status of credential provisioning
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    LWM2MCORE_NO_CREDENTIAL_PROVISIONED   = 0x01,   ///< Neither DM nor BS key provisioned
    LWM2MCORE_BS_CREDENTIAL_PROVISIONED   = 0x02,   ///< BS key provisioned
    LWM2MCORE_DM_CREDENTIAL_PROVISIONED   = 0x03    ///< DM key provisioned

}lwm2mcore_CredentialStatus_t;

//--------------------------------------------------------------------------------------------------
/**
 * @brief Enumeration of supported LWM2M credentials
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    LWM2MCORE_CREDENTIAL_FW_KEY = 0,            ///< FW public key
    LWM2MCORE_CREDENTIAL_SW_KEY,                ///< SW public key
    LWM2MCORE_CREDENTIAL_CERTIFICATE,           ///< Certificate for HTTPS
    LWM2MCORE_CREDENTIAL_BS_PUBLIC_KEY,         ///< LwM2M Client’s Certificate (Certificate mode), public key (RPK mode) or PSK Identity (PSK mode)
    LWM2MCORE_CREDENTIAL_BS_SERVER_PUBLIC_KEY,  ///< LwM2M Server’s or LWM2M Bootstrap Server’s Certificate (Certificate mode), public key (RPK mode)
    LWM2MCORE_CREDENTIAL_BS_SECRET_KEY,         ///< secret key or private key of the security mode
    LWM2MCORE_CREDENTIAL_BS_ADDRESS,            ///< BS server address
    LWM2MCORE_CREDENTIAL_DM_PUBLIC_KEY,         ///< LwM2M Client’s Certificate (Certificate mode), public key (RPK mode) or PSK Identity (PSK mode)
    LWM2MCORE_CREDENTIAL_DM_SERVER_PUBLIC_KEY,  ///< LWM2M Server’s or LWM2M Bootstrap Server’s Certificate (Certificate mode), public key (RPK mode)
    LWM2MCORE_CREDENTIAL_DM_SECRET_KEY,         ///< secret key or private key of the security mode
    LWM2MCORE_CREDENTIAL_DM_ADDRESS,            ///< DM server address
    LWM2MCORE_CREDENTIAL_MAX                    ///< Internal usage
}lwm2mcore_Credentials_t;
/**
  * @}
  */

/**
  * @addtogroup lwm2mcore_init_IFS
  * @{
  */
//--------------------------------------------------------------------------------------------------
/**
 * @brief Enumeration for session type
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    LWM2MCORE_SESSION_BOOTSTRAP,            ///< Bootstrap session
    LWM2MCORE_SESSION_DEVICE_MANAGEMENT     ///< Device management session
}lwm2mcore_SessionType_t;

//--------------------------------------------------------------------------------------------------
/**
 * @brief Enum for package type
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
 * @brief LwM2MCore instance reference
 */
//--------------------------------------------------------------------------------------------------
typedef struct ClientData_s* lwm2mcore_Ref_t;

/**
  * @}
  */

/**
  * @addtogroup lwm2mcore_asset_IFS
  * @{
  */
//--------------------------------------------------------------------------------------------------
/**
 * @brief Enum for LwM2M push ack callback
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    LWM2MCORE_ACK_RECEIVED = 0,     ///< Data transferred successfully.
    LWM2MCORE_ACK_TIMEOUT           ///< Transaction time out
} lwm2mcore_AckResult_t;

//--------------------------------------------------------------------------------------------------
/**
 * @brief Enum for LwM2M push result
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    LWM2MCORE_PUSH_INITIATED = 0,    ///< Push successfully initiated.
    LWM2MCORE_PUSH_BUSY,             ///< Busy doing a block transfer.
    LWM2MCORE_PUSH_FAILED            ///< Failed to initiate push.
} lwm2mcore_PushResult_t;

//--------------------------------------------------------------------------------------------------
/**
 * @brief Enum for LwM2M push content type
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    LWM2MCORE_PUSH_CONTENT_CBOR = 60,    ///< Uncompressed CBOR
    LWM2MCORE_PUSH_CONTENT_ZCBOR = 12118 ///< Proprietary compressed CBOR (zlib + CBOR)
} lwm2mcore_PushContent_t;

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function pointer of push callback function
 */
//--------------------------------------------------------------------------------------------------
typedef void (*lwm2mcore_PushAckCallback_t)
(
    lwm2mcore_AckResult_t result,       ///< [IN] result of the transaction
    uint16_t midPtr                     ///< [IN] message id
);
/**
  * @}
  */

/**
  * @addtogroup lwm2mcore_config_IFS
  * @{
  */
//--------------------------------------------------------------------------------------------------
/**
 * @brief CoAP URI representation.@n
 *        Data structure representing the LwM2M request URI, obtained by parsing CoAP URI options.
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    lwm2mcore_OpType_t op;                 ///< operation type
    int contentType;                       ///< payload content type, a value of -1 indicates content type is not specified
    int acceptContentType;                 ///< server accept content type, a value of -1 indicates its not specified from server
    bool observe;                          ///< if the request come with OBSERVE option
    int observeValue;                      ///< OBSERVE option value
    uint16_t oid;                          ///< object Id
    uint16_t oiid;                         ///< object instance Id
    uint16_t rid;                          ///< resource Id
    uint16_t riid;                         ///< resource instance Id
    uint32_t blockNum;                     ///< block number for CoAP block xfer
    uint16_t blockSize;                    ///< size of block for CoAP block xfer
    bool lastBlock;                        ///< if this is last block in CoAP block xfer
    int pathLen;                           ///< Len of alternative path (CoRE Link Format RFC6690),  0 for default LWM2M standard object
    bool standardPath;                     ///< flag indicate if the path name is LwM2M std path "lwm2m"
    uint8_t  pathName[LWM2MCORE_NAME_LEN]; ///< alternative path name
}lwm2mcore_Uri_t;
/**
  * @}
  */

/**
  * @addtogroup lwm2mcore_init_IFS
  * @{
  */

//--------------------------------------------------------------------------------------------------
/**
 * @brief Structure for a package download status
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    lwm2mcore_PkgDwlType_t pkgType;     ///< Package type
    uint32_t numBytes;                  ///< For package download, num of bytes to be downloaded
    uint32_t progress;                  ///< For package download, package download progress in %
    uint32_t errorCode;                 ///< For package download, error code
}lwm2mcore_PkgDwlStatus_t;

//--------------------------------------------------------------------------------------------------
/**
 * @brief Structure for session event
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    lwm2mcore_SessionType_t type;           ///< Session type for
                                            ///< @ref LWM2MCORE_EVENT_LWM2M_SESSION_TYPE_START event
}lwm2mcore_SessionStatus_t;

//--------------------------------------------------------------------------------------------------
/**
 * @brief Structure for events (session and package download)
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    lwm2mcore_StatusType_t event;               ///< Event
    union
    {
        lwm2mcore_SessionStatus_t   session;    ///< Session information
        lwm2mcore_PkgDwlStatus_t    pkgStatus;  ///< Package download status
    }u;                                         ///< Union
}lwm2mcore_Status_t;
/**
  * @}
  */

/**
  * @addtogroup lwm2mcore_config_IFS
  * @{
  */
//--------------------------------------------------------------------------------------------------
/**
 * @brief Function pointer of resource value changed callback function.
 *
 * @return
 *  - @c 0 on success
 *  - negative value on failure.
 */
//--------------------------------------------------------------------------------------------------
typedef int (*valueChangedCallback_t)
(
    lwm2mcore_Uri_t* uriPtr,            ///< [IN] uri represents the resource whose value is changed
    char* bufferPtr,                    ///< [INOUT] buffer of the notification data
    int len                             ///< [IN] llength of the notification data
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function pointer of resource READ function.
 *
 * @return
 *  - @c 0 on success
 *  - negative value on failure
 *  - > @c 0 values for asynchronous operations
 */
//--------------------------------------------------------------------------------------------------
typedef int (*lwm2mcore_ReadCallback_t)
(
    lwm2mcore_Uri_t* uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource.
    char* bufferPtr,                    ///< [INOUT] data buffer for information
    size_t* lenPtr,                     ///< [INOUT] length of input buffer and length of the
                                        ///< returned data
    valueChangedCallback_t changedCb    ///< [IN] not used for READ operation
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function pointer of resource WRITE/OBSERVE function.
 *
 * @return
 *  - @c 0 on success
 *  - negative value on failure
 *  - > @c 0 values for asynchronous operations
 */
//--------------------------------------------------------------------------------------------------
typedef int (*lwm2mcore_WriteCallback_t)
(
    lwm2mcore_Uri_t* uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource.
    char* bufferPtr,                    ///< [INOUT] data buffer for information
    size_t len                          ///< [IN] length of input buffer
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function pointer of resource EXECUTE function.
 *
 * @return
 *  - @c 0 on success
 *  - negative value on failure
 *  - > @c 0 values for asynchronous ops.
 */
//--------------------------------------------------------------------------------------------------
typedef int (*lwm2mcore_ExecuteCallback_t)
(
    lwm2mcore_Uri_t* uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource.
    char* bufferPtr,                    ///< [INOUT] contain arguments
    size_t len                          ///< [IN] length of buffer
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Structure for an object resource
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    uint16_t id;                        ///< resource id: one of the LWM2M_xyz_RID above
    lwm2mcore_ResourceType_t type;      ///< resource data type
    uint16_t maxResInstCnt;             ///< maximum number of resource instance count. 1 means single instance
    lwm2mcore_ReadCallback_t read;      ///< operation handler: READ handler
    lwm2mcore_WriteCallback_t write;    ///< operation handler: WRITE handler
    lwm2mcore_ExecuteCallback_t exec;   ///< operation handler: EXECUTE handler
}lwm2mcore_Resource_t;

//--------------------------------------------------------------------------------------------------
/**
 * @brief Structure for an object
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    uint16_t id;                            ///< one of the defined object id
    uint16_t maxObjInstCnt;                 ///< maximum number of object instance count. 1 means single instance.
    uint16_t resCnt;                        ///< number of resource count under this object
    lwm2mcore_Resource_t* resources;        ///< pointer to the list of resource under this object
}lwm2mcore_Object_t;

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function pointer of resource READ/WRITE/OBSERVE function.
 *
 * @return
 *  - @c 0 on success
 *  - negative value on failure
 *  -  > @c 0 values for asynchronous ops.
*/
//--------------------------------------------------------------------------------------------------
typedef int (*genericReadWriteApi_t)
(
    lwm2mcore_Uri_t* uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource.
    char* bufferPtr,                    ///< [INOUT] data buffer for information
    size_t* lenPtr,                     ///< [INOUT] length of input buffer and length of the
                                        ///< returned data
    valueChangedCallback_t changedCb    ///< [IN] callback function pointer for OBSERVE operation
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Structure for a handler
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    uint16_t                objCnt;                 ///< Numbers of supported pre-defined objects
    lwm2mcore_Object_t*     objects;                ///< Actual supported pre-defined objects
    genericReadWriteApi_t   genericUOHandler;       ///< Generic handler for unidentified object
}
lwm2mcore_Handler_t;

/**
  * @}
  */

/**
  * @addtogroup lwm2mcore_init_IFS
  * @{
  */

//--------------------------------------------------------------------------------------------------
/**
 * @brief Callback for event status
 *
 * @return
 *  - @c 0 on success
 *  - negative value on failure.
 */
//--------------------------------------------------------------------------------------------------
typedef int (*lwm2mcore_StatusCb_t)
(
    lwm2mcore_Status_t eventStatus              ///< [IN] event status
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Initialize LwM2MCore
 *
 * @remark Public function which can be called by the client.
 *
 * @return
 *  - instance reference
 *  - @c NULL in case of error
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Ref_t lwm2mcore_Init
(
    lwm2mcore_StatusCb_t eventCb    ///< [IN] event callback
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Free LwM2MCore
 *
 * @remark Public function which can be called by the client.
 */
//--------------------------------------------------------------------------------------------------
void lwm2mcore_Free
(
    lwm2mcore_Ref_t instanceRef     ///< [IN] instance reference
);

/**
  * @}
  */

/**
  * @addtogroup lwm2mcore_config_IFS
  * @{
  */

//--------------------------------------------------------------------------------------------------
/**
 * @brief Register the object table and service API
 *
 * @note If handlerPtr parameter is NULL, LwM2MCore registers it's own "standard" object list
 *
 * @remark Public function which can be called by the client.
 *
 * @return
 *  - number of registered objects
 */
//--------------------------------------------------------------------------------------------------
uint16_t lwm2mcore_ObjectRegister
(
    lwm2mcore_Ref_t instanceRef,            ///< [IN] instance reference
    char* endpoint,                         ///< [IN] Device endpoint
    lwm2mcore_Handler_t* const handlerPtr,  ///< [IN] List of supported object/resource by client
                                            ///< This parameter can be set to NULL
    void* const servicePtr                  ///< [IN] Client service API table
);

//--------------------------------------------------------------------------------------------------
/**
 * Read a resource from the object table
 *
 * @remark Public function which can be called by the client.
 *
 * @return
 *  - @c true if resource is found and read succeeded
 *  - else @c false
 */
//--------------------------------------------------------------------------------------------------
bool lwm2mcore_ResourceRead
(
    uint16_t objectId,                 ///< [IN] object identifier
    uint16_t objectInstanceId,         ///< [IN] object instance identifier
    uint16_t resourceId,               ///< [IN] resource identifier
    uint16_t resourceInstanceId,       ///< [IN] resource instance identifier
    char*    dataPtr,                  ///< [OUT] Array of requested resources to be read
    size_t*  dataSizePtr               ///< [IN/OUT] Size of the array
);

/**
  * @}
  */

/**
  * @addtogroup lwm2mcore_cnx_IFS
  * @{
  */

//--------------------------------------------------------------------------------------------------
/**
 * @brief LwM2M client entry point to initiate a connection
 *
 * @remark Public function which can be called by the client.
 *
 * @return
 *  - @c true if the treatment is launched
 *  - else @c false
 */
//--------------------------------------------------------------------------------------------------
bool lwm2mcore_Connect
(
    lwm2mcore_Ref_t instanceRef     ///< [IN] instance reference
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to close a connection
 *
 * @remark Public function which can be called by the client.
 *
 * @return
 *  - @c true if the treatment is launched
 *  - else @c false
 */
//--------------------------------------------------------------------------------------------------
bool lwm2mcore_Disconnect
(
    lwm2mcore_Ref_t instanceRef     ///< [IN] instance reference
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to send an update message to the Device Management server.
 *
 * This API can be used when the application wants to send a notification or during a firmware/app
 * update in order to be able to fully treat the scheduled update job
 *
 * @remark Public function which can be called by the client.
 *
 * @return
 *  - @c true if the treatment is launched
 *  - else @c false
 */
//--------------------------------------------------------------------------------------------------
bool lwm2mcore_Update
(
    lwm2mcore_Ref_t instanceRef     ///< [IN] instance reference
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to retrieve the status and the type of the current connection
 *
 * @remark Public function which can be called by the client.
 *
 * @return
 *  - @c true if the device is connected to any server
 *  - else @c false
 */
//--------------------------------------------------------------------------------------------------
bool lwm2mcore_ConnectionGetType
(
    lwm2mcore_Ref_t instanceRef,    ///< [IN] instance reference
    bool* isDeviceManagement        ///< [INOUT] Session type (false: bootstrap,
                                    ///< true: device management)
);

/**
  * @}
  */

/**
  * @addtogroup lwm2mcore_asset_IFS
  * @{
  */
//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to push data to lwm2mCore
 *
 * @remark Public function which can be called by the client.
 *
 * @return
 *  - @c true if a data push transaction is initiated
 *  - else @c false
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_PushResult_t lwm2mcore_Push
(
    lwm2mcore_Ref_t instanceRef,            ///< [IN] instance reference
    uint8_t* payloadPtr,                    ///< [IN] payload
    size_t payloadLength,                   ///< [IN] payload length
    lwm2mcore_PushContent_t content,       ///< [IN] content type
    uint16_t* midPtr                        ///< [OUT] message id
);


//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to set the push callback handler
 */
//--------------------------------------------------------------------------------------------------
void lwm2mcore_SetPushCallback
(
    lwm2mcore_PushAckCallback_t callbackP  ///< [IN] push callback pointer
);

/**
  * @}
  */

/**
  * @addtogroup lwm2mcore_sota_IFS
  * @{
  */
//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to notify LwM2MCore of supported object instance list for software and asset data
 *
 * @remark Public function which can be called by the client.
 *
 * @return
 *  - @c true if the list was successfully treated
 *  - else @c false
 */
//--------------------------------------------------------------------------------------------------
bool lwm2mcore_UpdateSwList
(
    lwm2mcore_Ref_t instanceRef,    ///< [IN] Instance reference (Set to 0 if this API is used if
                                    ///< lwm2mcore_init API was no called)
    const char* listPtr,            ///< [IN] Formatted list
    size_t listLen                  ///< [IN] Size of the update list
);

/**
  * @}
  */

/**
  * @addtogroup lwm2mcore_log_IFS
  * @{
  */

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function for assert
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 */
//--------------------------------------------------------------------------------------------------
void lwm2mcore_Assert
(
    bool condition,         ///< [IN] Condition to be checked
    const char* function,   ///< [IN] Function name which calls the assert function
    uint32_t line           ///< [IN] Function line which calls the assert function
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Macro for assertion
 */
//--------------------------------------------------------------------------------------------------
#define LWM2MCORE_ASSERT(X) lwm2mcore_Assert(X, __func__, __LINE__)

//--------------------------------------------------------------------------------------------------
/**
 * @brief Adaptation function for log: dump data
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 */
//--------------------------------------------------------------------------------------------------
void lwm2mcore_DataDump
(
    const char* descPtr,        ///< [IN] data description
    void* addrPtr,              ///< [IN] Data address to be dumped
    int len                     ///< [IN] Data length
);

/**
  * @}
  */

/**
  * @addtogroup lwm2mcore_config_IFS
  * @{
  */

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to read the lifetime from the server object.
 * The lifetime is retrieved from the @ref LWM2MCORE_BOOTSTRAP_PARAM parameter (see @ref
 * lwm2mcore_platform_adaptor_storage_IFS).
 *
 * @remark Public function which can be called by the client.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetLifetime
(
    uint32_t* lifetimePtr       ///< [OUT] Lifetime in seconds
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to set the lifetime in the server object and save to platform storage.
 * The lifetime is then stored in the @ref LWM2MCORE_BOOTSTRAP_PARAM parameter (see @ref
 * lwm2mcore_platform_adaptor_storage_IFS).
 *
 * @remark Public function which can be called by the client.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_INCORRECT_RANGE if the lifetime is not correct
 *  - @ref LWM2MCORE_ERR_INVALID_STATE if no device management server are configured
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_SetLifetime
(
    uint32_t lifetime           ///< [IN] Lifetime in seconds
);

//--------------------------------------------------------------------------------------------------
/**
 * Function to report UDP error code.
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 */
//--------------------------------------------------------------------------------------------------
void lwm2mcore_ReportUdpErrorCode
(
    int code    ///< [IN] UDP custom error code defined in udp.h
);

//--------------------------------------------------------------------------------------------------
/**
 * Function to report CoAP response code.
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 */
//--------------------------------------------------------------------------------------------------
void lwm2mcore_ReportCoapResponseCode
(
    int code    ///< [IN] CoAP error code as defined in RFC 7252 section 12.1.2
);

/**
  * @}
  */

#endif /* __LWM2MCORE_H__ */
