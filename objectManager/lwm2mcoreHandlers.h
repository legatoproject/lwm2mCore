/**
 * @file lwm2mcoreHandlers.h
 *
 * client of the LWM2M stack
 *
 * Copyright (C) Sierra Wireless Inc. Use of this work is subject to license.
 *
 */

#ifndef LWM2MCORE_HANDLERS_H
#define LWM2MCORE_HANDLERS_H

//--------------------------------------------------------------------------------------------------
/**
 * Define value for object instance of object 0 for bootstrap server
 */
//--------------------------------------------------------------------------------------------------
#define LWM2MCORE_BS_SERVER_OIID 0

//--------------------------------------------------------------------------------------------------
/**
 * Define value for object instance of object 0 for Device Management server
 */
//--------------------------------------------------------------------------------------------------
#define LWM2MCORE_DM_SERVER_OIID 1

//--------------------------------------------------------------------------------------------------
/**
 *                  OBJECT 0: SECURITY
 */
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Function to write a resource of object 0
 * Object: 0 - Security
 * Resource: all
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treament succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int lwm2mcore_WriteSecurityObj
(
    lwm2mcore_uri_t *uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource.
    char *bufferPtr,                    ///< [INOUT] data buffer for information
    size_t len                          ///< [IN] length of input buffer
);

//--------------------------------------------------------------------------------------------------
/**
 * Function to read a resource of object 0
 * Object: 0 - Security
 * Resource: All
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treament succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int lwm2mcore_ReadSecurityObj
(
    lwm2mcore_uri_t *uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource.
    char *bufferPtr,                    ///< [INOUT] data buffer for information
    size_t *lenPtr,                     ///< [INOUT] length of input buffer and length of the
                                        ///< returned data
    value_changed_callback_t changed_cb ///< [IN] callback for notification
);

//--------------------------------------------------------------------------------------------------
/**
 * Function for the server SMS parameters
 * Object: 0 - Security
 * Resources: 6, 7, 8, 9
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treament succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int OnLWM2MSecuritySMSDummy
(
    lwm2mcore_uri_t *uri,               ///< [IN] uri represents the requested operation and
                                        ///< object/resource.
    char *bufferPtr,                    ///< [INOUT] data buffer for information
    size_t len,                         ///< [IN] length of input buffer
    value_changed_callback_t changed_cb ///< [IN] not used for READ operation but for WRITE one
);


//--------------------------------------------------------------------------------------------------
/**
 *                  OBJECT 1: SERVER
 */
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Define default value for Pmin: Minimum period (resource 2 of object 1)
 */
//--------------------------------------------------------------------------------------------------
#define LWM2MCORE_PMIN_DEFAULT_VALUE 30

//--------------------------------------------------------------------------------------------------
/**
 * Define default value for Pmax: Maximum period (resource 3 of object 1)
 */
//--------------------------------------------------------------------------------------------------
#define LWM2MCORE_PMAX_DEFAULT_VALUE 60

//--------------------------------------------------------------------------------------------------
/**
 * Define possible values for binding mode (supported mode is UQ)
 */
//--------------------------------------------------------------------------------------------------
#define LWM2MCORE_BINDING_UDP "U"
#define LWM2MCORE_BINDING_UDP_QUEUE "UQ"
#define LWM2MCORE_BINDING_SMS "S"
#define LWM2MCORE_BINDING_SMS_QUEUE "SQ"
#define LWM2MCORE_BINDING_UDP_SMS "US"
#define LWM2MCORE_BINDING_UDP_QUEUE_SMS "UQS"

//--------------------------------------------------------------------------------------------------
/**
 * Function to write a resource of object 1
 * Object: 1 - Server
 * Resource: all
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treament succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int lwm2mcore_WriteServerObj
(
    lwm2mcore_uri_t *uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource.
    char *bufferPtr,                    ///< [INOUT] data buffer for information
    size_t len                          ///< [IN] length of input buffer
);

//--------------------------------------------------------------------------------------------------
/**
 * Function to read a resource of object 1
 * Object: 1 - Server
 * Resource: All
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treament succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int lwm2mcore_ReadServerObj
(
    lwm2mcore_uri_t *uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource.
    char *bufferPtr,                    ///< [INOUT] data buffer for information
    size_t *lenPtr,                     ///< [INOUT] length of input buffer and length of the
                                        ///< returned data
    value_changed_callback_t changed_cb ///< [IN] callback for notification
);


//--------------------------------------------------------------------------------------------------
/**
 *                  OBJECT 3: DEVICE
 */
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Function to write a resource of object 3
 * Object: 3 - Device
 * Resource: all
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treament succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int lwm2mcore_WriteDeviceObj
(
    lwm2mcore_uri_t *uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource.
    char *bufferPtr,                    ///< [INOUT] data buffer for information
    size_t len                          ///< [IN] length of input buffer
);

//--------------------------------------------------------------------------------------------------
/**
 * Function to read a resource of object 3
 * Object: 3 - Device
 * Resource: All
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treament succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int lwm2mcore_ReadDeviceObj
(
    lwm2mcore_uri_t *uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource.
    char *bufferPtr,                    ///< [INOUT] data buffer for information
    size_t *lenPtr,                     ///< [INOUT] length of input buffer and length of the
                                        ///< returned data
    value_changed_callback_t changed_cb ///< [IN] callback for notification
);

//--------------------------------------------------------------------------------------------------
/**
 * Function to store credentials in non volatile memory
 *
 * @return
 *      - true in case of success
 *      - false in case of failure
 */
//--------------------------------------------------------------------------------------------------
bool lwm2mcore_StoreCredentials
(
    void
);

//--------------------------------------------------------------------------------------------------
/**
 *                  OBJECT 10243: SSL certificates
 */
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Function to read/write the SSL certificates
 * Object: 10243 - SSL certificates
 * Resource: 0
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treament succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int OnSslCertif
(
    lwm2mcore_uri_t *uri,               ///< [IN] uri represents the requested operation and
                                        ///< object/resource.
    char *buffer,                       ///< [INOUT] data buffer for information
    size_t *len,                        ///< [INOUT] length of input buffer and length of the
                                        ///< returned data
    value_changed_callback_t changed_cb ///< [IN] not used for READ operation but for WRITE one
);


int OnUnlistedObject
(
    lwm2mcore_uri_t *uri,               ///< [IN] uri represents the requested operation and
                                        ///< object/resource.
    char *buffer,                       ///< [INOUT] data buffer for information
    size_t *len,                        ///< [INOUT] length of input buffer and length of the
                                        ///< returned data
    value_changed_callback_t changed_cb ///< [IN] callback function pointer for OBSERVE operation
);


//--------------------------------------------------------------------------------------------------
/**
 *                  OBJECT 5: FIRMWARE UPDATE
 */
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Function to write a ressource of object 5
 * Object: 5 - Firmware update
 * Resource: all with write operation
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treament succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int lwm2mcore_WriteFwUpdate
(
    lwm2mcore_uri_t *uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource.
    char *bufferPtr,                    ///< [INOUT] data buffer for information
    size_t len                          ///< [IN] length of input buffer
);

//--------------------------------------------------------------------------------------------------
/**
 * Function to read a ressource of object 5
 * Object: 5 - Firmware update
 * Resource: all with read operation
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treament succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int lwm2mcore_ReadFwUpdate
(
    lwm2mcore_uri_t *uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource.
    char *bufferPtr,                    ///< [INOUT] data buffer for information
    size_t *lenPtr,                     ///< [INOUT] length of input buffer and length of the
                                        ///< returned data
    value_changed_callback_t changed_cb ///< [IN] not used for READ operation but for WRITE one
);

//--------------------------------------------------------------------------------------------------
/**
 * Function to execute a ressource of object 5
 * Object: 5 - Firwmare update
 * Resource: all
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treament succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int lwm2mcore_ExecFwUpdate
(
    lwm2mcore_uri_t *uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource.
    char *bufferPtr,                    ///< [INOUT] data buffer for information
    size_t len                          ///< [INOUT] length of input buffer
);

#endif /* LWM2MCORE_HANDLERS_H */
