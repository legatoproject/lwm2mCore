/**
 * @file lwm2mcorePortUpdate.h
 *
 * Porting layer for firmware update
 *
 * Copyright (C) Sierra Wireless Inc. Use of this work is subject to license.
 *
 */

#ifndef LWM2MCORE_PORT_UPDATE_H_
#define LWM2MCORE_PORT_UPDATE_H_

#include "lwm2mcore.h"
#include "lwm2mcoreObjectHandler.h"

//--------------------------------------------------------------------------------------------------
/**
 * Define the maximum length for a package URI
 */
//--------------------------------------------------------------------------------------------------
#define LWM2MCORE_PACKAGE_URI_MAX_LEN   255

//--------------------------------------------------------------------------------------------------
/**
 * Enumeration to indicates if an update is linked to a firmware update or a software update
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    LWM2MCORE_FW_UPDATE_TYPE,    ///< Firmware update
    LWM2MCORE_SW_UPDATE_TYPE,    ///< Software update
    LWM2MCORE_MAX_UPDATE_TYPE    ///< Internal usage
}lwm2mcore_update_type_t;

//--------------------------------------------------------------------------------------------------
/**
 * Enumeration for firmware update state (object 5 (firmware update), resource 3)
 * These values are defined in the LWM2M specification
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
}lwm2mcore_fw_update_state_t;

//--------------------------------------------------------------------------------------------------
/**
 * Enumeration for firmware update result (object 5 (firmware update), resource 5)
 * These values are defined in the LWM2M specification
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    LWM2MCORE_FW_UPDATE_RESULT_DEFAULT_NORMAL = 0,      ///< FW update default result
    LWM2MCORE_FW_UPDATE_RESULT_INSTALLED_SUCCESSFUL,    ///< FW update result: success
    LWM2MCORE_FW_UPDATE_RESULT_NO_STORAGE_SPACE,        ///< FW update result: failure - not enough
                                                        ///< space
    LWM2MCORE_FW_UPDATE_RESULT_OUT_OF_MEMORY,           ///< FW update result: out of memory
    LWM2MCORE_FW_UPDATE_RESULT_COMMUNICATION_ERROR,     ///< FW update result: failure -
                                                        ///< communication error
    LWM2MCORE_FW_UPDATE_RESULT_VERIFY_ERROR,            ///< FW update result: failure - package
                                                        ///< check error
    LWM2MCORE_FW_UPDATE_RESULT_UNSUPPORTED_PKG_TYPE,    ///< FW update result: failure - unsupported
                                                        ///< package
    LWM2MCORE_FW_UPDATE_RESULT_INVALID_URI,             ///< FW update result: failure - invalid URI
    LWM2MCORE_FW_UPDATE_RESULT_INSTALL_FAILURE,         ///< FW update result: failure - install
                                                        ///< failure
    LWM2MCORE_FW_UPDATE_RESULT_UNSUPPORTED_PROTOCOL,    ///< FW update result: failure - unsupported
                                                        ///< protocol
    /* Sierra defined UD_RESULT code */
    LWM2MCORE_FW_UPDATE_RESULT_CLIENT_CANCEL = 0xF000   ///< internal usage
}lwm2mcore_fw_update_result_t;

//--------------------------------------------------------------------------------------------------
/**
 * The server pushes a package to the LWM2M client
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treament succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_sid_t os_portUpdate_PushPackage
(
    lwm2mcore_update_type_t type,   ///< [IN] Update type
    uint16_t instanceId,            ///< [IN] Intance Id (0 for FW, any value for SW)
    char* bufferPtr,                ///< [INOUT] data buffer
    size_t len                      ///< [IN] length of input buffer
);

//--------------------------------------------------------------------------------------------------
/**
 * The server sends a package URI to the LWM2M client
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treament succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_sid_t os_portUpdate_SetPackageUri
(
    lwm2mcore_update_type_t type,   ///< [IN] Update type
    uint16_t instanceId,            ///< [IN] Intance Id (0 for FW, any value for SW)
    char* bufferPtr,                ///< [INOUT] data buffer
    size_t len                      ///< [IN] length of input buffer
);

//--------------------------------------------------------------------------------------------------
/**
 * The server requires the current package URI stored in the LWM2M client
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treament succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_sid_t os_portUpdate_GetPackageUri
(
    lwm2mcore_update_type_t type,   ///< [IN] Update type
    uint16_t instanceId,            ///< [IN] Intance Id (0 for FW, any value for SW)
    char* bufferPtr,                ///< [INOUT] data buffer
    size_t* lenPtr                  ///< [INOUT] length of input buffer and length of the returned
                                    ///< data
);

//--------------------------------------------------------------------------------------------------
/**
 * The server requests to launch an update
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treament succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_sid_t os_portUpdate_LaunchUpdate
(
    lwm2mcore_update_type_t type,   ///< [IN] Update type
    uint16_t instanceId,            ///< [IN] Intance Id (0 for FW, any value for SW)
    char* bufferPtr,                ///< [INOUT] data buffer
    size_t len                      ///< [IN] length of input buffer
);

//--------------------------------------------------------------------------------------------------
/**
 * The server requires the update state
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treament succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_sid_t os_portUpdate_GetUpdateState
(
    lwm2mcore_update_type_t type,   ///< [IN] Update type
    uint16_t instanceId,            ///< [IN] Intance Id (0 for FW, any value for SW)
    uint8_t* updateStatePtr         ///< [OUT] update state
);

//--------------------------------------------------------------------------------------------------
/**
 * The server requires the update result
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treament succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_sid_t os_portUpdate_GetUpdateResult
(
    lwm2mcore_update_type_t type,   ///< [IN] Update type
    uint16_t instanceId,            ///< [IN] Intance Id (0 for FW, any value for SW)
    uint8_t* updateResultPtr        ///< [OUT] update result
);

//--------------------------------------------------------------------------------------------------
/**
 * The server requires the package name
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treament succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_sid_t os_portUpdate_GetPackageName
(
    lwm2mcore_update_type_t type,   ///< [IN] Update type
    uint16_t instanceId,            ///< [IN] Intance Id (0 for FW, any value for SW)
    char* bufferPtr,                ///< [INOUT] data buffer
    size_t* lenPtr                  ///< [INOUT] length of input buffer and length of the returned
                                    ///< data
);

//--------------------------------------------------------------------------------------------------
/**
 * The server requires the package version
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treament succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_sid_t os_portUpdate_GetPackageVersion
(
    lwm2mcore_update_type_t type,   ///< [IN] Update type
    uint16_t instanceId,            ///< [IN] Intance Id (0 for FW, any value for SW)
    char* bufferPtr,                ///< [INOUT] data buffer
    size_t* lenPtr                  ///< [INOUT] length of input buffer and length of the returned
                                    ///< data
);

#endif /* LWM2MCORE_PORT_UPDATE_H_ */

