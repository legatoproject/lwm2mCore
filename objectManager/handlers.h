/**
 * @file handlers.h
 *
 * client of the LWM2M stack
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#ifndef __HANDLERS_H__
#define __HANDLERS_H__

//--------------------------------------------------------------------------------------------------
/**
 * @brief Define value for object instance of object 0 for bootstrap server
 */
//--------------------------------------------------------------------------------------------------
#define LWM2MCORE_BS_SERVER_OIID 0

//--------------------------------------------------------------------------------------------------
/**
 * @brief Define value for object instance of object 0 for Device Management server
 */
//--------------------------------------------------------------------------------------------------
#define LWM2MCORE_DM_SERVER_OIID 1

//--------------------------------------------------------------------------------------------------
/**
 * @brief Define default value for Pmin: Minimum period (resource 2 of object 1)
 */
//--------------------------------------------------------------------------------------------------
#define LWM2MCORE_PMIN_DEFAULT_VALUE 30

//--------------------------------------------------------------------------------------------------
/**
 * @brief Define default value for Pmax: Maximum period (resource 3 of object 1)
 */
//--------------------------------------------------------------------------------------------------
#define LWM2MCORE_PMAX_DEFAULT_VALUE 60

//--------------------------------------------------------------------------------------------------
/**
 * @brief Define possible values for binding mode (supported mode is UQ)
 */
//--------------------------------------------------------------------------------------------------
#define LWM2MCORE_BINDING_STR_MAX_LEN    3

//--------------------------------------------------------------------------------------------------
/**
 * @brief Binding mode: UDP
 */
//--------------------------------------------------------------------------------------------------
#define LWM2MCORE_BINDING_UDP           "U"

//--------------------------------------------------------------------------------------------------
/**
 * @brief Binding mode: UDP with Queue Mode
 */
//--------------------------------------------------------------------------------------------------
#define LWM2MCORE_BINDING_UDP_QUEUE     "UQ"

//--------------------------------------------------------------------------------------------------
/**
 * @brief Binding mode: SMS
 */
//--------------------------------------------------------------------------------------------------
#define LWM2MCORE_BINDING_SMS           "S"

//--------------------------------------------------------------------------------------------------
/**
 * @brief Binding mode: SMS with Queue Mode
 */
//--------------------------------------------------------------------------------------------------
#define LWM2MCORE_BINDING_SMS_QUEUE     "SQ"

//--------------------------------------------------------------------------------------------------
/**
 * @brief Binding mode: UDP and SMS
 */
//--------------------------------------------------------------------------------------------------
#define LWM2MCORE_BINDING_UDP_SMS       "US"

//--------------------------------------------------------------------------------------------------
/**
 * @brief Binding mode: UDP UDP with Queue Mode and SMS
 */
//--------------------------------------------------------------------------------------------------
#define LWM2MCORE_BINDING_UDP_QUEUE_SMS "UQS"

//--------------------------------------------------------------------------------------------------
/**
 *                                  OBJECT 0: SECURITY
 */
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to write a resource of object 0
 *
 * Object: 0 - Security
 * Resource: all
 *
 * @return
 *      - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - @ref LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - @ref LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int omanager_WriteSecurityObj
(
    lwm2mcore_Uri_t *uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource.
    char *bufferPtr,                    ///< [INOUT] data buffer for information
    size_t len                          ///< [IN] length of input buffer
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to read a resource of object 0
 *
 * Object: 0 - Security
 * Resource: All
 *
 * @return
 *      - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - @ref LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - @ref LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int omanager_ReadSecurityObj
(
    lwm2mcore_Uri_t *uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource.
    char *bufferPtr,                    ///< [INOUT] data buffer for information
    size_t *lenPtr,                     ///< [INOUT] length of input buffer and length of the
                                        ///< returned data
    valueChangedCallback_t changedCb    ///< [IN] callback for notification
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function for the server SMS parameters
 *
 * Object: 0 - Security
 * Resources: 6, 7, 8, 9
 *
 * @return
 *      - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - @ref LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - @ref LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int omanager_SmsDummy
(
    lwm2mcore_Uri_t *uri,               ///< [IN] uri represents the requested operation and
                                        ///< object/resource.
    char *bufferPtr,                    ///< [INOUT] data buffer for information
    size_t len                         ///< [IN] length of input buffer
);

//--------------------------------------------------------------------------------------------------
/**
 *                                  OBJECT 1: SERVER
 */
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to write a resource of object 1
 *
 * Object: 1 - Server
 * Resource: all
 *
 * @return
 *      - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - @ref LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - @ref LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int omanager_WriteServerObj
(
    lwm2mcore_Uri_t *uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource.
    char *bufferPtr,                    ///< [INOUT] data buffer for information
    size_t len                          ///< [IN] length of input buffer
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to read a resource of object 1
 *
 * Object: 1 - Server
 * Resource: All
 *
 * @return
 *      - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - @ref LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - @ref LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int omanager_ReadServerObj
(
    lwm2mcore_Uri_t *uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource.
    char *bufferPtr,                    ///< [INOUT] data buffer for information
    size_t *lenPtr,                     ///< [INOUT] length of input buffer and length of the
                                        ///< returned data
    valueChangedCallback_t changedCb    ///< [IN] callback for notification
);

//--------------------------------------------------------------------------------------------------
/**
 *                                  OBJECT 3: DEVICE
 */
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to write a resource of object 3
 *
 * Object: 3 - Device
 * Resource: all
 *
 * @return
 *      - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - @ref LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - @ref LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int omanager_WriteDeviceObj
(
    lwm2mcore_Uri_t *uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource.
    char *bufferPtr,                    ///< [INOUT] data buffer for information
    size_t len                          ///< [IN] length of input buffer
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to read a resource of object 3
 *
 * Object: 3 - Device
 * Resource: All
 *
 * @return
 *      - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - @ref LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - @ref LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int omanager_ReadDeviceObj
(
    lwm2mcore_Uri_t *uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource.
    char *bufferPtr,                    ///< [INOUT] data buffer for information
    size_t *lenPtr,                     ///< [INOUT] length of input buffer and length of the
                                        ///< returned data
    valueChangedCallback_t changedCb    ///< [IN] callback for notification
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to execute a resource of object 3
 *
 * Object: 3 - Device
 * Resource: All with execute operation
 *
 * @return
 *      - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - @ref LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - @ref LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int omanager_ExecDeviceObj
(
    lwm2mcore_Uri_t* uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource
    char* bufferPtr,                    ///< [INOUT] data buffer for information
    size_t len                          ///< [IN] length of input buffer
);

//--------------------------------------------------------------------------------------------------
/**
 *                              OBJECT 4: CONNECTIVITY MONITORING
 */
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to read a resource of object 4
 *
 * Object: 4 - Connectivity monitoring
 * Resource: All
 *
 * @return
 *      - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - @ref LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - @ref LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - @ref LWM2MCORE_ERR_OVERFLOW in case of buffer overflow
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int omanager_ReadConnectivityMonitoringObj
(
    lwm2mcore_Uri_t* uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource
    char* bufferPtr,                    ///< [INOUT] data buffer for information
    size_t* lenPtr,                     ///< [INOUT] length of input buffer and length of the
                                        ///< returned data
    valueChangedCallback_t changedCb    ///< [IN] callback for notification
);

//--------------------------------------------------------------------------------------------------
/**
 *                                  OBJECT 5: FIRMWARE UPDATE
 */
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to write a resource of object 5
 *
 * Object: 5 - Firmware update
 * Resource: all with write operation
 *
 * @return
 *      - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - @ref LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - @ref LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int omanager_WriteFwUpdateObj
(
    lwm2mcore_Uri_t *uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource.
    char *bufferPtr,                    ///< [INOUT] data buffer for information
    size_t len                          ///< [IN] length of input buffer
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to read a resource of object 5
 *
 * Object: 5 - Firmware update
 * Resource: all with read operation
 *
 * @return
 *      - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - @ref LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - @ref LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int omanager_ReadFwUpdateObj
(
    lwm2mcore_Uri_t *uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource.
    char *bufferPtr,                    ///< [INOUT] data buffer for information
    size_t *lenPtr,                     ///< [INOUT] length of input buffer and length of the
                                        ///< returned data
    valueChangedCallback_t changedCb    ///< [IN] not used for READ operation but for WRITE one
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to execute a resource of object 5
 *
 * Object: 5 - Firwmare update
 * Resource: all
 *
 * @return
 *      - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - @ref LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - @ref LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int omanager_ExecFwUpdate
(
    lwm2mcore_Uri_t *uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource.
    char *bufferPtr,                    ///< [INOUT] data buffer for information
    size_t len                          ///< [INOUT] length of input buffer
);


//--------------------------------------------------------------------------------------------------
/**
 *                                  OBJECT 6: LOCATION
 */
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to read a resource of object 6
 *
 * Object: 6 - Location
 * Resource: All
 *
 * @return
 *      - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - @ref LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - @ref LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - @ref LWM2MCORE_ERR_OVERFLOW in case of buffer overflow
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int omanager_ReadLocationObj
(
    lwm2mcore_Uri_t* uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource
    char* bufferPtr,                    ///< [INOUT] data buffer for information
    size_t* lenPtr,                     ///< [INOUT] length of input buffer and length of the
                                        ///< returned data
    valueChangedCallback_t changedCb    ///< [IN] callback for notification
);

//--------------------------------------------------------------------------------------------------
/**
 *                              OBJECT 7: CONNECTIVITY STATISTICS
 */
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to read a resource of object 7
 *
 * Object: 7 - Connectivity statistics
 * Resource: All with read operation
 *
 * @return
 *      - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - @ref LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - @ref LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - @ref LWM2MCORE_ERR_OVERFLOW in case of buffer overflow
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int omanager_ReadConnectivityStatisticsObj
(
    lwm2mcore_Uri_t* uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource
    char* bufferPtr,                    ///< [INOUT] data buffer for information
    size_t* lenPtr,                     ///< [INOUT] length of input buffer and length of the
                                        ///< returned data
    valueChangedCallback_t changedCb    ///< [IN] callback for notification
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to execute a resource of object 7
 *
 * Object: 7 - Connectivity statistics
 * Resource: All with execute operation
 *
 * @return
 *      - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - @ref LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - @ref LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int omanager_ExecConnectivityStatistics
(
    lwm2mcore_Uri_t* uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource
    char* bufferPtr,                    ///< [INOUT] data buffer for information
    size_t len                          ///< [IN] length of input buffer
);

//--------------------------------------------------------------------------------------------------
/**
 *                                  OBJECT 9: SOFTWARE UPDATE
 */
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to write a resource of object 9
 *
 * Object: 9 - software update
 * Resource: all with write operation
 *
 * @return
 *      - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - @ref LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - @ref LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int omanager_WriteSwUpdateObj
(
    lwm2mcore_Uri_t* uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource
    char* bufferPtr,                    ///< [INOUT] data buffer for information
    size_t len                          ///< [IN] length of input buffer
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to read a resource of object 9
 *
 * Object: 9 - Software update
 * Resource: all with read operation
 *
 * @return
 *      - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - @ref LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - @ref LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int omanager_ReadSwUpdateObj
(
    lwm2mcore_Uri_t* uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource
    char* bufferPtr,                    ///< [INOUT] data buffer for information
    size_t* lenPtr,                     ///< [INOUT] length of input buffer and length of the
                                        ///< returned data
    valueChangedCallback_t changedCb    ///< [IN] not used for READ operation but for WRITE one
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to execute a resource of object 9
 *
 * Object: 9 - Software update
 * Resource: all with execute operation
 *
 * @return
 *      - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - @ref LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - @ref LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int omanager_ExecSwUpdate
(
    lwm2mcore_Uri_t* uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource
    char* bufferPtr,                    ///< [INOUT] data buffer for information
    size_t len                          ///< [IN] length of input buffer
);

//--------------------------------------------------------------------------------------------------
/**
 *                              OBJECT 10241: SUBSCRIPTION
 */
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to read a resource of object 10241
 *
 * Object: 10241 - Subscription
 * Resource: All
 *
 * @return
 *      - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - @ref LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - @ref LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - @ref LWM2MCORE_ERR_OVERFLOW in case of buffer overflow
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int omanager_ReadSubscriptionObj
(
    lwm2mcore_Uri_t* uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource
    char* bufferPtr,                    ///< [INOUT] data buffer for information
    size_t* lenPtr,                     ///< [INOUT] length of input buffer and length of the
                                        ///< returned data
    valueChangedCallback_t changedCb    ///< [IN] callback for notification
);

//--------------------------------------------------------------------------------------------------
/**
 *                          OBJECT 10242: EXTENDED CONNECTIVITY STATISTICS
 */
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to read a resource of object 10242
 *
 * Object: 10242 - Extended connectivity statistics
 * Resource: All
 *
 * @return
 *      - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - @ref LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - @ref LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - @ref LWM2MCORE_ERR_OVERFLOW in case of buffer overflow
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int omanager_ReadExtConnectivityStatsObj
(
    lwm2mcore_Uri_t* uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource
    char* bufferPtr,                    ///< [INOUT] data buffer for information
    size_t* lenPtr,                     ///< [INOUT] length of input buffer and length of the
                                        ///< returned data
    valueChangedCallback_t changedCb    ///< [IN] callback for notification
);

//--------------------------------------------------------------------------------------------------
/**
 *                              OBJECT 10243: SSL certificates
 */
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to update the SSL certificates
 *
 * Object: 10243 - SSL certificates
 * Resource: 0
 *
 * @note To delete the saved certificate, set the length to 0
 *
 * @return
 *      - @ref LWM2MCORE_ERR_COMPLETED_OK if the update succeeds
 *      - @ref LWM2MCORE_ERR_INCORRECT_RANGE if the size of the certificate is > 4000 bytes
 *      - @ref LWM2MCORE_ERR_GENERAL_ERROR if the update fails
 *      - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid
 */
//--------------------------------------------------------------------------------------------------
int omanager_WriteSslCertif
(
    lwm2mcore_Uri_t* uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource
    char* bufferPtr,                    ///< [INOUT] data buffer for information
    size_t len                          ///< [IN] length of input buffer
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function for not registered objects
 *
 * @return
 *      - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - @ref LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - @ref LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int omanager_OnUnlistedObject
(
    lwm2mcore_Uri_t* uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource
    char* bufferPtr,                    ///< [INOUT] data buffer for information
    size_t* lenPtr,                     ///< [INOUT] length of input buffer and length of the
                                        ///< returned data
    valueChangedCallback_t changedCb    ///< [IN] not used for READ operation but for WRITE one
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to store credentials in non volatile memory
 *
 * @return
 *      - @c true in case of success
 *      - @c false in case of failure
 */
//--------------------------------------------------------------------------------------------------
bool omanager_StoreCredentials
(
    void
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to read the bootstrap configuration from platform memory
 *
 * @return
 *      - @c true in case of success
 *      - @c false in case of failure
 */
//--------------------------------------------------------------------------------------------------
bool omanager_GetBootstrapConfiguration
(
    void
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to save the bootstrap configuration in platform memory
 *
 * @return
 *      - @c true in case of success
 *      - @c false in case of failure
 */
//--------------------------------------------------------------------------------------------------
bool omanager_SetBootstrapConfiguration
(
    void
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Sets the lifetime in the server configuration and saves it to file system
 *
 * @return
 *      - @c true in case of success
 *      - @c false in case of failure
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t omanager_SetLifetime
(
    uint32_t lifetime                               ///< [IN] lifetime in seconds
);

//--------------------------------------------------------------------------------------------------
/**
 * Retrieves the lifetime from the server configuration
 *
 * @return
 *      - @c true in case of success
 *      - @c false in case of failure
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t omanager_GetLifetime
(
    uint32_t* lifetimePtr                           ///< [OUT] lifetime in seconds
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to know if the connection is secured or not
 *
 * @return
 *      - @c true in case of secured connection
 *      - @c false else
 */
//--------------------------------------------------------------------------------------------------
bool omanager_IsSecuredMode
(
    void
);
#endif /* __HANDLERS_H__ */
