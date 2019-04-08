/**
 * @file update.h
 *
 * Porting layer for firmware update
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#ifndef __LWM2MCORE_UPDATE_H__
#define __LWM2MCORE_UPDATE_H__

#include <lwm2mcore/lwm2mcore.h>

/**
  * @addtogroup lwm2mcore_update_IFS
  * @{
  */
//--------------------------------------------------------------------------------------------------
/**
 * @brief Define the maximum length for a package URI
 */
//--------------------------------------------------------------------------------------------------
#define LWM2MCORE_PACKAGE_URI_MAX_LEN               255

//--------------------------------------------------------------------------------------------------
/**
 * @brief Define the maximum bytes number for a package URI, including the null-terminator
 */
//--------------------------------------------------------------------------------------------------
#define LWM2MCORE_PACKAGE_URI_MAX_BYTES             (LWM2MCORE_PACKAGE_URI_MAX_LEN + 1)

//--------------------------------------------------------------------------------------------------
/**
 * @brief Define the maximum length for the software objects
 */
//--------------------------------------------------------------------------------------------------
#ifdef LWM2M_OBJECT_9
#define LWM2MCORE_SW_OBJECT_INSTANCE_LIST_MAX_LEN   4032
#else
#define LWM2MCORE_SW_OBJECT_INSTANCE_LIST_MAX_LEN   0
#endif

//--------------------------------------------------------------------------------------------------
/**
 * @brief Enumeration for firmware update state (object 5 (firmware update), resource 3)
 *
 * @note These values are defined in the LWM2M specification
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    LWM2MCORE_FW_UPDATE_STATE_WAITDOWNLOAD      = -1,   ///< A package download for FW update will
                                                        ///< be launched
    LWM2MCORE_FW_UPDATE_STATE_IDLE              = 0,    ///< FW update default state (LWM2M
                                                        ///< specification)
    LWM2MCORE_FW_UPDATE_STATE_DOWNLOADING       = 1,    ///< FW update downloading state (LWM2M
                                                        ///< specification)
    LWM2MCORE_FW_UPDATE_STATE_DOWNLOADED        = 2,    ///< FW update downloaded state (LWM2M
                                                        ///< specification)
    LWM2MCORE_FW_UPDATE_STATE_UPDATING          = 3,    ///< FW update updating state (LWM2M
                                                        ///< specification)
    LWM2MCORE_FW_UPDATE_STATE_WAITINSTALL       = 4,    ///< FW update: wait for install
    LWM2MCORE_FW_UPDATE_STATE_WAITINSTALLRESULT = 5     ///< FW update: install result
}lwm2mcore_FwUpdateState_t;

//--------------------------------------------------------------------------------------------------
/**
 * @brief Enumeration for firmware update result (object 5 (firmware update), resource 5)
 *
 * @note These values are defined in the LWM2M specification
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    LWM2MCORE_FW_UPDATE_RESULT_DEFAULT_NORMAL       = 0,    ///< FW update default result
    LWM2MCORE_FW_UPDATE_RESULT_INSTALLED_SUCCESSFUL = 1,    ///< FW update result: success
    LWM2MCORE_FW_UPDATE_RESULT_NO_STORAGE_SPACE     = 2,    ///< FW update result: failure - not
                                                            ///< enough space
    LWM2MCORE_FW_UPDATE_RESULT_OUT_OF_MEMORY        = 3,    ///< FW update result: out of memory
    LWM2MCORE_FW_UPDATE_RESULT_COMMUNICATION_ERROR  = 4,    ///< FW update result: failure -
                                                            ///< communication error
    LWM2MCORE_FW_UPDATE_RESULT_VERIFY_ERROR         = 5,    ///< FW update result: failure - package
                                                            ///< check error
    LWM2MCORE_FW_UPDATE_RESULT_UNSUPPORTED_PKG_TYPE = 6,    ///< FW update result: failure -
                                                            ///< unsupported package
    LWM2MCORE_FW_UPDATE_RESULT_INVALID_URI          = 7,    ///< FW update result: failure - invalid
                                                            ///< URI
    LWM2MCORE_FW_UPDATE_RESULT_INSTALL_FAILURE      = 8,    ///< FW update result: failure - install
                                                            ///< failure
    LWM2MCORE_FW_UPDATE_RESULT_UNSUPPORTED_PROTOCOL = 9,    ///< FW update result: failure -
                                                            ///< unsupported protocol
    /* Sierra defined UD_RESULT code */
    LWM2MCORE_FW_UPDATE_RESULT_CLIENT_CANCEL        = 0xF000 ///< internal usage
}lwm2mcore_FwUpdateResult_t;

//--------------------------------------------------------------------------------------------------
/**
 * @brief Enumeration for software update state (object 9 (firmware update), resource 7)
 *
 *  @note These values are defined in the LWM2M specification
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    LWM2MCORE_SW_UPDATE_STATE_INITIAL           = 0,    ///< Before downloading (LWM2M
                                                        ///< specification)
    LWM2MCORE_SW_UPDATE_STATE_DOWNLOAD_STARTED  = 1,    ///< The downloading process has started and
                                                        ///< is on-going (LWM2M specification)
    LWM2MCORE_SW_UPDATE_STATE_DOWNLOADED        = 2,    ///< The package has been completely
                                                        ///< downloaded (LWM2M specification)
    LWM2MCORE_SW_UPDATE_STATE_DELIVERED         = 3,    ///< In that state, the package has been
                                                        ///< correctly downloaded and is ready to be
                                                        ///< installed.(LWM2M specification)
    LWM2MCORE_SW_UPDATE_STATE_INSTALLED         = 4,    ///< In that state the software is correctly
                                                        ///< installed and can be activated or
                                                        ///< deactivated (LWM2M specification)
    LWM2MCORE_SW_UPDATE_STATE_WAITINSTALLRESULT = 5     ///< FW update: install result
}lwm2mcore_SwUpdateState_t;

//--------------------------------------------------------------------------------------------------
/**
 * @brief Enumeration for software update result (object 9 (firmware update), resource 9)
 *
 *  @note These values are defined in the LWM2M specification
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    LWM2MCORE_SW_UPDATE_RESULT_INITIAL          = 0,    ///< Prior to download any new package in
                                                        ///< the Device, Update Result MUST be reset
                                                        ///< to this initial value
    LWM2MCORE_SW_UPDATE_RESULT_DOWNLOADING      = 1,    ///< Downloading. The package downloading
                                                        ///< process is on-going
    LWM2MCORE_SW_UPDATE_RESULT_INSTALLED        = 2,    ///< Software successfully installed
    LWM2MCORE_SW_UPDATE_RESULT_DOWNLOADED       = 3,    ///< Successfully Downloaded and package
                                                        ///< integrity verified
    LWM2MCORE_SW_UPDATE_RESULT_NOT_ENOUGH_MEMORY= 50,   ///< Not enough storage for the new software
                                                        ///< package
    LWM2MCORE_SW_UPDATE_RESULT_OUT_OF_MEMORY    = 51,   ///< Out of memory during downloading
                                                        ///< process
    LWM2MCORE_SW_UPDATE_RESULT_CONNECTION_LOST  = 52,   ///< Connection lost during downloading
                                                        ///< process
    LWM2MCORE_SW_UPDATE_RESULT_CHECK_FAILURE    = 53,   ///< Package integrity check failure
    LWM2MCORE_SW_UPDATE_RESULT_UNSUPPORTED_TYPE = 54,   ///< Unsupported package type
    LWM2MCORE_SW_UPDATE_RESULT_INVALID_URI      = 56,   ///< Invalid URI
    LWM2MCORE_SW_UPDATE_RESULT_DEVICE_ERROR     = 57,   ///< Device defined update error
    LWM2MCORE_SW_UPDATE_RESULT_INSTALL_FAILURE  = 58,   ///< Software installation failure
    LWM2MCORE_SW_UPDATE_RESULT_UNINSTALL_FAILURE= 59    ///< Uninstallation Failure
}lwm2mcore_SwUpdateResult_t;


//--------------------------------------------------------------------------------------------------
/**
 * Enumeration for update errors (generic values)
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    LWM2MCORE_UPDATE_ERROR_NO_STORAGE_SPACE,        ///< Failure - not enough space
    LWM2MCORE_UPDATE_ERROR_OUT_OF_MEMORY,           ///< Not enough storage for the new package
    LWM2MCORE_UPDATE_ERROR_CONNECTION_LOST,         ///< Connection lost during downloading process
    LWM2MCORE_UPDATE_ERROR_UNSUPPORTED_PACKAGE,     ///< Unsupported package type
    LWM2MCORE_UPDATE_ERROR_DEVICE_SPECIFIC          ///< Device defined update error
}
lwm2mcore_UpdateError_t;

//--------------------------------------------------------------------------------------------------
/**
 * @brief The server pushes a package to the LWM2M client
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters is incorrect
 *  - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *  - @ref LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_PushUpdatePackage
(
    lwm2mcore_UpdateType_t type,    ///< [IN] Update type
    uint16_t instanceId,            ///< [IN] Instance Id (0 for FW, any value for SW)
    char* bufferPtr,                ///< [INOUT] data buffer
    size_t len                      ///< [IN] length of input buffer
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief The server sends a package URI to the LWM2M client
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters is incorrect
 *  - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *  - @ref LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_SetUpdatePackageUri //TODO: intenral now
(
    lwm2mcore_UpdateType_t type,    ///< [IN] Update type
    uint16_t instanceId,            ///< [IN] Instance Id (0 for FW, any value for SW)
    char* bufferPtr,                ///< [INOUT] data buffer
    size_t len                      ///< [IN] length of input buffer
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief The server requires the current package URI stored in the LWM2M client
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters is incorrect
 *  - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *  - @ref LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *  - @ref LWM2MCORE_ERR_OVERFLOW in case of buffer overflow
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetUpdatePackageUri //TODO: intenral now
(
    lwm2mcore_UpdateType_t type,    ///< [IN] Update type
    uint16_t instanceId,            ///< [IN] Instance Id (0 for FW, any value for SW)
    char* bufferPtr,                ///< [INOUT] data buffer
    size_t* lenPtr                  ///< [INOUT] length of input buffer and length of the returned
                                    ///< data
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief The server requests to launch an update
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @warning The client MUST store a parameter in non-volatile memory in order to keep in memory that
 * an install request was received and launch a timer (value could be decided by the client
 * implementation) in order to treat the install request.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters is incorrect
 *  - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *  - @ref LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_LaunchUpdate
(
    lwm2mcore_UpdateType_t type,    ///< [IN] Update type
    uint16_t instanceId,            ///< [IN] Instance Id (0 for FW, any value for SW)
    char* bufferPtr,                ///< [INOUT] data buffer
    size_t len                      ///< [IN] length of input buffer
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief The server requires the software update state
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters is incorrect
 *  - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *  - @ref LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetSwUpdateState
(
    uint16_t instanceId,            ///< [IN] Instance Id (0 for FW, any value for SW)
    uint8_t* updateStatePtr         ///< [OUT] update state
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief The server requires the software update result
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters is incorrect
 *  - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *  - @ref LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetSwUpdateResult
(
    uint16_t instanceId,            ///< [IN] Instance Id (0 for FW, any value for SW)
    uint8_t* updateResultPtr        ///< [OUT] update result
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function for setting software update state
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters is incorrect
 *  - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *  - @ref LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_SetSwUpdateState
(
    lwm2mcore_SwUpdateState_t swUpdateState     ///< [IN] New SW update state
);

//--------------------------------------------------------------------------------------------------
/**
 * Function for setting software update result
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters is incorrect
 *  - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *  - @ref LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_SetSwUpdateResult
(
    lwm2mcore_SwUpdateResult_t swUpdateResult   ///< [IN] New SW update result
);

#ifdef LEGACY_FW_STATUS
//--------------------------------------------------------------------------------------------------
/**
 * Function for setting legacy firmware update state
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters is incorrect
 *  - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *  - @ref LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_SetLegacyFwUpdateState
(
    lwm2mcore_FwUpdateState_t fwUpdateState     ///< [IN] New FW update state
);

//--------------------------------------------------------------------------------------------------
/**
 * Function for setting legacy firmware update result
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters is incorrect
 *  - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *  - @ref LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_SetLegacyFwUpdateResult
(
    lwm2mcore_FwUpdateResult_t fwUpdateResult   ///< [IN] New FW update result
);

//--------------------------------------------------------------------------------------------------
/**
 * Function for getting legacy firmware update state
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters is incorrect
 *  - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *  - @ref LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetLegacyFwUpdateState
(
    lwm2mcore_FwUpdateState_t* fwUpdateStatePtr     ///< [INOUT] FW update state
);

//--------------------------------------------------------------------------------------------------
/**
 * Function for getting legacy firmware update result
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters is incorrect
 *  - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *  - @ref LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetLegacyFwUpdateResult
(
    lwm2mcore_FwUpdateResult_t* fwUpdateResultPtr   ///< [INOUT] FW update result
);

#endif // LEGACY_FW_STATUS

//--------------------------------------------------------------------------------------------------
/**
 * @brief The server requires the package name
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters is incorrect
 *  - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *  - @ref LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetUpdatePackageName
(
    lwm2mcore_UpdateType_t type,    ///< [IN] Update type
    uint16_t instanceId,            ///< [IN] Instance Id (0 for FW, any value for SW)
    char* bufferPtr,                ///< [INOUT] data buffer
    uint32_t len                    ///< [IN] length of input buffer
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief The server requires the package version
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters is incorrect
 *  - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *  - @ref LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *  - @ref LWM2MCORE_ERR_OVERFLOW in case of buffer overflow
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetUpdatePackageVersion
(
    lwm2mcore_UpdateType_t type,    ///< [IN] Update type
    uint16_t instanceId,            ///< [IN] Instance Id (0 for FW, any value for SW)
    char* bufferPtr,                ///< [INOUT] data buffer
    uint32_t len                    ///< [IN] length of input buffer
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief The server sets the "update supported objects" field for software update
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters is incorrect
 *  - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *  - @ref LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_SetSwUpdateSupportedObjects
(
    uint16_t instanceId,            ///< [IN] Instance Id (any value for SW)
    bool value                      ///< [IN] Update supported objects field value
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief The server requires the "update supported objects" field for software update
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters is incorrect
 *  - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *  - @ref LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetSwUpdateSupportedObjects
(
    uint16_t instanceId,            ///< [IN] Instance Id (any value for SW)
    bool* valuePtr                  ///< [INOUT] Update supported objects field value
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief The server requires the activation state for one embedded application
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters is incorrect
 *  - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *  - @ref LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetSwUpdateActivationState
(
    uint16_t instanceId,            ///< [IN] Instance Id (any value for SW)
    bool* valuePtr                  ///< [INOUT] Activation state
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief The server requires an embedded application to be uninstalled (only for software update)
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters is incorrect
 *  - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *  - @ref LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_LaunchSwUpdateUninstall
(
    uint16_t instanceId,            ///< [IN] Instance Id (any value for SW)
    char* bufferPtr,                ///< [INOUT] data buffer
    size_t len                      ///< [IN] length of input buffer
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief The server requires an embedded application to be activated or deactivated (only for
 * software update)
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters is incorrect
 *  - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *  - @ref LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_ActivateSoftware
(
    bool activation,        ///< [IN] Requested activation (true: activate, false: deactivate)
    uint16_t instanceId,    ///< [IN] Instance Id (any value for SW)
    char* bufferPtr,        ///< [INOUT] data buffer
    size_t len              ///< [IN] length of input buffer
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief The server request to create or delete an object instance of object 9
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters is incorrect
 *  - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *  - @ref LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_SoftwareUpdateInstance
(
    bool create,                ///<[IN] Create (true) or delete (false)
    uint16_t instanceId         ///<[IN] Object instance Id to create or delete
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Resume a package download if necessary
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *  - @ref LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_SetDownloadError
(
    lwm2mcore_UpdateError_t error   ///< [IN] Update error
);

//--------------------------------------------------------------------------------------------------
/**
 * Indicates that the Firmware update is accepted
 *
 * @remark Public function which can be called by the client.
 *
 * @note
 * This function is not available if @c LWM2M_EXTERNAL_DOWNLOADER compilation flag is embedded
 *
 * @note
 * This API needs to be called when the client accepts the Firmware update request.
 * When the Firmware update request is received by LwM2MCore from the server, the @ref
 * lwm2mcore_LaunchUpdate function is called.
 * The client can totally handle this request on its side, the common response to this update
 * request should be a platform reset.
 * Before resetting the platform, the client MUST call this function in order to allow LwM2MCore
 * knowning that the update requested was accepted.
 *
 * @return
 *      - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - @ref LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the request
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_SetUpdateAccepted
(
    void
);

//lwm2mcore_Sid_t lwm2mcore_SetUpdateAccepted(type) type à mettre? type dans workspace

//--------------------------------------------------------------------------------------------------
/**
 * Indicates that the Firmware update succeeds
 *
 * @remark Public function which can be called by the client.
 *
 * @note
 * This function is not available if @c LWM2M_EXTERNAL_DOWNLOADER compilation flag is embedded
 *
 * @note
 * This API needs to be called by the client when a Firmware update was treated (so usually at
 * platform initialization). According to the Firmware update result, the client sets the @c
 * isSuccess parameter to the right value.
 *
 * @return
 *      - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - @ref LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the request
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_SetUpdateResult
(
    bool    isSuccess   ///< [IN] true to indicate the update success, else failure
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to get download information
 *
 * @remark Public function which can be called by the client.
 *
 * @return
 *  - LWM2MCORE_ERR_COMPLETED_OK on success
 *  - LWM2MCORE_ERR_INVALID_ARG when at least one parameter is invalid
 *  - LWM2MCORE_ERR_INVALID_STATE if no package download is on-going
 *  - LWM2MCORE_ERR_GENERAL_ERROR on failure
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetDownloadInfo
(
    lwm2mcore_UpdateType_t* updateTypePtr,  ///< [OUT] Update type
    uint64_t*               packageSizePtr  ///< [OUT] Package size
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to check if a FW update is on-going
 * This function returns true if the FW upate install was accepted (@ref
 * lwm2mcore_SetUpdateAccepted) and before final FW update (@ref lwm2mcore_SetUpdateResult)
 *
 * @remark Public function which can be called by the client.
 *
 * @return
 *  - LWM2MCORE_ERR_COMPLETED_OK on success
 *  - LWM2MCORE_ERR_INVALID_ARG when at least one parameter is invalid
 *  - LWM2MCORE_ERR_INVALID_STATE if no package download is on-going
 *  - LWM2MCORE_ERR_GENERAL_ERROR on failure
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_IsFwUpdateOnGoing
(
    bool*   IsFwUpdateOnGoingPtr    ///< [INOUT] True if a FW update is ongoing, false otherwise
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to check if a package download for FW update is over and if the install request
 * was not received.
 * This function can be called by the client when a connection is closed to the server, or at client
 * initialization to know if the client needs to initiate a connection to the server in order to
 * receive the FW update install request from the server (a package was fully downloaded but the
 * install request was not received).
 *
 * @remark Public function which can be called by the client.
 *
 * @return
 *  - LWM2MCORE_ERR_COMPLETED_OK on success
 *  - LWM2MCORE_ERR_INVALID_ARG when at least one parameter is invalid
 *  - LWM2MCORE_ERR_INVALID_STATE if no package download was ended
 *  - LWM2MCORE_ERR_GENERAL_ERROR on failure
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_IsFwUpdateInstallWaited
(
    bool*   IsFwUpdateInstallWaitedPtr    ///< [INOUT] True if a FW update install request is waited
);

//--------------------------------------------------------------------------------------------------
/**
 * Get TPF mode state
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetTpfState
(
    bool*  statePtr        ///< [OUT] true if third party FOTA service is started
);

/**
  * @}
  */

#endif /* __LWM2MCORE_UPDATE_H__ */

