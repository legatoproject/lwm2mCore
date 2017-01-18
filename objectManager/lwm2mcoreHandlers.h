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
 * Function to read/write the server URI
 * Object: 0 - Security
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
int OnLWM2MSecurityServerURI
(
    lwm2mcore_uri_t *uri,               ///< [IN] uri represents the requested operation and
                                        ///< object/resource.
    char *buffer,                       ///< [INOUT] data buffer for information
    size_t *len,                        ///< [INOUT] length of input buffer and length of the
                                        ///< returned data
    value_changed_callback_t changed_cb ///< [IN] not used for READ operation but for WRITE one
);

//--------------------------------------------------------------------------------------------------
/**
 * Function to read/write the server type
 * Object: 0 - Security
 * Resource: 1
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
int OnLWM2MSecurityServerType
(
    lwm2mcore_uri_t *uri,               ///< [IN] uri represents the requested operation and
                                        ///< object/resource.
    char *buffer,                       ///< [INOUT] data buffer for information
    size_t *len,                        ///< [INOUT] length of input buffer and length of the
                                        ///< returned data
    value_changed_callback_t changed_cb ///< [IN] not used for READ operation but for WRITE one
);

//--------------------------------------------------------------------------------------------------
/**
 * Function to read/write the server security mode
 * Object: 0 - Security
 * Resource: 2
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
int OnLWM2MSecurityMode
(
    lwm2mcore_uri_t *uri,               ///< [IN] uri represents the requested operation and
                                        ///< object/resource.
    char *buffer,                       ///< [INOUT] data buffer for information
    size_t *len,                        ///< [INOUT] length of input buffer and length of the
                                        ///< returned data
    value_changed_callback_t changed_cb ///< [IN] not used for READ operation but for WRITE one
);

//--------------------------------------------------------------------------------------------------
/**
 * Function to read/write the device PSK Identity
 * Object: 0 - Security
 * Resource: 3
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
int OnLWM2MSecurityDevicePKID
(
    lwm2mcore_uri_t *uri,               ///< [IN] uri represents the requested operation and
                                        ///< object/resource.
    char *buffer,                       ///< [INOUT] data buffer for information
    size_t *len,                        ///< [INOUT] length of input buffer and length of the
                                        ///< returned data
    value_changed_callback_t changed_cb ///< [IN] not used for READ operation but for WRITE one
);

//--------------------------------------------------------------------------------------------------
/**
 * Function to read/write the server public key
 * Object: 0 - Security
 * Resource: 4
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
int OnLWM2MSecurityServerKey
(
    lwm2mcore_uri_t *uri,               ///< [IN] uri represents the requested operation and
                                        ///< object/resource.
    char *buffer,                       ///< [INOUT] data buffer for information
    size_t *len,                        ///< [INOUT] length of input buffer and length of the
                                        ///< returned data
    value_changed_callback_t changed_cb ///< [IN] not used for READ operation but for WRITE one
);

//--------------------------------------------------------------------------------------------------
/**
 * Function to read/write the device secret key
 * Object: 0 - Security
 * Resource: 5
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
int OnLWM2MSecuritySecretKey
(
    lwm2mcore_uri_t *uri,               ///< [IN] uri represents the requested operation and
                                        ///< object/resource.
    char *buffer,                       ///< [INOUT] data buffer for information
    size_t *len,                        ///< [INOUT] length of input buffer and length of the
                                        ///< returned data
    value_changed_callback_t changed_cb ///< [IN] not used for READ operation but for WRITE one
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
    char *buffer,                       ///< [INOUT] data buffer for information
    size_t *len,                        ///< [INOUT] length of input buffer and length of the
                                        ///< returned data
    value_changed_callback_t changed_cb ///< [IN] not used for READ operation but for WRITE one
);

//--------------------------------------------------------------------------------------------------
/**
 * Function for the short server ID
 * Object: 0 - Security
 * Resource: 10
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
int OnLWM2MSecurityServerID
(
    lwm2mcore_uri_t *uri,               ///< [IN] uri represents the requested operation and
                                        ///< object/resource.
    char *buffer,                       ///< [INOUT] data buffer for information
    size_t *len,                        ///< [INOUT] length of input buffer and length of the
                                        ///< returned data
    value_changed_callback_t changed_cb ///< [IN] not used for READ operation but for WRITE one
);

//--------------------------------------------------------------------------------------------------
/**
 * Function for the Client Hold Off Time
 * Object: 0 - Security
 * Resource: 11
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
int OnLWM2MSecurityClientHoldOffTime
(
    lwm2mcore_uri_t *uri,               ///< [IN] uri represents the requested operation and
                                        ///< object/resource.
    char *buffer,                       ///< [INOUT] data buffer for information
    size_t *len,                        ///< [INOUT] length of input buffer and length of the
                                        ///< returned data
    value_changed_callback_t changed_cb ///< [IN] not used for READ operation but for WRITE one
);





//--------------------------------------------------------------------------------------------------
/**
 *                  OBJECT 1: SERVER
 */
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Function to read/write the short server ID
 * Object: 1 - Server
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
int OnLWM2MServerID
(
    lwm2mcore_uri_t *uri,               ///< [IN] uri represents the requested operation and
                                        ///< object/resource.
    char *buffer,                       ///< [INOUT] data buffer for information
    size_t *len,                        ///< [INOUT] length of input buffer and length of the
                                        ///< returned data
    value_changed_callback_t changed_cb ///< [IN] not used for READ operation but for WRITE one
);

//--------------------------------------------------------------------------------------------------
/**
 * Function to read/write the server lifetime
 * Object: 1 - Server
 * Resource: 1
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
int OnLWM2MServerLifeTime
(
    lwm2mcore_uri_t *uri,               ///< [IN] uri represents the requested operation and
                                        ///< object/resource.
    char *buffer,                       ///< [INOUT] data buffer for information
    size_t *len,                        ///< [INOUT] length of input buffer and length of the
                                        ///< returned data
    value_changed_callback_t changed_cb ///< [IN] not used for READ operation but for WRITE one
);

//--------------------------------------------------------------------------------------------------
/**
 * Function to read/write the server Pmin
 * Object: 1 - Server
 * Resource: 2
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
int OnLWM2MServerMinPeriod
(
    lwm2mcore_uri_t *uri,               ///< [IN] uri represents the requested operation and
                                        ///< object/resource.
    char *buffer,                       ///< [INOUT] data buffer for information
    size_t *len,                        ///< [INOUT] length of input buffer and length of the
                                        ///< returned data
    value_changed_callback_t changed_cb ///< [IN] not used for READ operation but for WRITE one
);

//--------------------------------------------------------------------------------------------------
/**
 * Function to read/write the server Pmax
 * Object: 1 - Server
 * Resource: 3
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
int OnLWM2MServerMaxPeriod
(
    lwm2mcore_uri_t *uri,               ///< [IN] uri represents the requested operation and
                                        ///< object/resource.
    char *buffer,                       ///< [INOUT] data buffer for information
    size_t *len,                        ///< [INOUT] length of input buffer and length of the
                                        ///< returned data
    value_changed_callback_t changed_cb ///< [IN] not used for READ operation but for WRITE one
);

//--------------------------------------------------------------------------------------------------
/**
 * Function to read/write the server Notification Storing When Disabled or Offline
 * Object: 1 - Server
 * Resource: 6
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
int OnLWM2MServerQueueUpNotification
(
    lwm2mcore_uri_t *uri,               ///< [IN] uri represents the requested operation and
                                        ///< object/resource.
    char *buffer,                       ///< [INOUT] data buffer for information
    size_t *len,                        ///< [INOUT] length of input buffer and length of the
                                        ///< returned data
    value_changed_callback_t changed_cb ///< [IN] not used for READ operation but for WRITE one
);

//--------------------------------------------------------------------------------------------------
/**
 * Function to read/write the server binding mode
 * Object: 1 - Server
 * Resource: 7
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
int OnLWM2MServerBindingMode
(
    lwm2mcore_uri_t *uri,               ///< [IN] uri represents the requested operation and
                                        ///< object/resource.
    char *buffer,                       ///< [INOUT] data buffer for information
    size_t *len,                        ///< [INOUT] length of input buffer and length of the
                                        ///< returned data
    value_changed_callback_t changed_cb ///< [IN] not used for READ operation but for WRITE one
);

//--------------------------------------------------------------------------------------------------
/**
 * Function to send a REGISTRATION UPDATE to the LWM2M server (DM server)
 * Object: 1 - Server
 * Resource: 8
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
int OnExecLWM2MServerRegUpdate
(
    lwm2mcore_uri_t *uri,               ///< [IN] uri represents the requested operation and
                                        ///< object/resource.
    char *buffer,                       ///< [INOUT] contain arguments
    size_t *len                         ///< [INOUT] length of buffer
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
 *                  OBJECT 3: DEVICE
 */
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Function to read the device manufacturer
 * Object: 3 - Device
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
int OnManufacturer
(
    lwm2mcore_uri_t *uri,               ///< [IN] uri represents the requested operation and
                                        ///< object/resource.
    char *buffer,                       ///< [INOUT] data buffer for information
    size_t *len,                        ///< [INOUT] length of input buffer and length of the
                                        ///< returned data
    value_changed_callback_t changed_cb ///< [IN] not used for READ operation but for WRITE one
);

//--------------------------------------------------------------------------------------------------
/**
 * Function to read the model number
 * Object: 3 - Device
 * Resource: 1
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
int OnModelNumber
(
    lwm2mcore_uri_t *uri,               ///< [IN] uri represents the requested operation and
                                        ///< object/resource.
    char *buffer,                       ///< [INOUT] data buffer for information
    size_t *len,                        ///< [INOUT] length of input buffer and length of the
                                        ///< returned data
    value_changed_callback_t changed_cb ///< [IN] not used for READ operation but for WRITE one
);

//--------------------------------------------------------------------------------------------------
/**
 * Function to read the device serial number
 * Object: 3 - Device
 * Resource: 2
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
int OnSerialNumber
(
    lwm2mcore_uri_t *uri,               ///< [IN] uri represents the requested operation and
                                        ///< object/resource.
    char *buffer,                       ///< [INOUT] data buffer for information
    size_t *len,                        ///< [INOUT] length of input buffer and length of the
                                        ///< returned data
    value_changed_callback_t changed_cb ///< [IN] not used for READ operation but for WRITE one
);

//--------------------------------------------------------------------------------------------------
/**
 * Function to read the firmware version
 * Object: 3 - Device
 * Resource: 3
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
int OnFirmwareVersion
(
    lwm2mcore_uri_t *uri,               ///< [IN] uri represents the requested operation and
                                        ///< object/resource.
    char *buffer,                       ///< [INOUT] data buffer for information
    size_t *len,                        ///< [INOUT] length of input buffer and length of the
                                        ///< returned data
    value_changed_callback_t changed_cb ///< [IN] not used for READ operation but for WRITE one
);


//--------------------------------------------------------------------------------------------------
/**
 * Function to read/write the device time
 * Object: 3 - Device
 * Resource: 13
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
int OnCurrentTime
(
    lwm2mcore_uri_t *uriPtr,               ///< [IN] uriPtr represents the requested operation and
                                            ///< object/resource
    char *buffer,                       ///< [INOUT] data buffer for information
    size_t *len,                        ///< [INOUT] length of input buffer and length of the
                                            ///< returned data
    value_changed_callback_t changed_cb     ///< [IN] not used for READ operation but for WRITE one
);

//--------------------------------------------------------------------------------------------------
/**
 * Function to read/write the device UTC offset
 * Object: 3 - Device
 * Resource: 14
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
int OnUTCOffset
(
    lwm2mcore_uri_t *uriPtr,               ///< [IN] uriPtr represents the requested operation and
                                            ///< object/resource
    char *buffer,                       ///< [INOUT] data buffer for information
    size_t *len,                        ///< [INOUT] length of input buffer and length of the
                                            ///< returned data
    value_changed_callback_t changed_cb     ///< [IN] not used for READ operation but for WRITE one
);

//--------------------------------------------------------------------------------------------------
/**
 * Function to read/write the device Supported Binding and Modes
 * Object: 3 - Device
 * Resource: 15
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
int OnClientSupportedBindingMode
(
    lwm2mcore_uri_t *uri,               ///< [IN] uri represents the requested operation and
                                        ///< object/resource.
    char *buffer,                       ///< [INOUT] data buffer for information
    size_t *len,                        ///< [INOUT] length of input buffer and length of the
                                        ///< returned data
    value_changed_callback_t changed_cb ///< [IN] not used for READ operation but for WRITE one
);

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

#endif /* LWM2MCORE_HANDLERS_H */
