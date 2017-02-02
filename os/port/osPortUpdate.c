/**
 * @file osPortUpdate.c
 *
 * Porting layer for Firmware Over The Air update
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "osDebug.h"
#include "osPortUpdate.h"

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
lwm2mcore_sid_t os_portUpdatePushPackage
(
    lwm2mcore_updateType_t type,    ///< [IN] Update type
    uint16_t instanceId,            ///< [IN] Intance Id (0 for FW, any value for SW)
    char* bufferPtr,                ///< [INOUT] data buffer
    size_t len                      ///< [IN] length of input buffer
)
{
    /* This function can be used to receive a package using CoAP
     * This is not supported by LWM2MCore
     */
    return LWM2MCORE_ERR_OP_NOT_SUPPORTED;
}


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
lwm2mcore_sid_t os_portUpdateSetPackageUri
(
    lwm2mcore_updateType_t type,    ///< [IN] Update type
    uint16_t instanceId,            ///< [IN] Intance Id (0 for FW, any value for SW)
    char* bufferPtr,                ///< [INOUT] data buffer
    size_t len                      ///< [IN] length of input buffer
)
{
    lwm2mcore_sid_t sid;

    if (0 == len)
    {
        /* If len is 0, then :
         * the Update State shall be set to default value: LWM2MCORE_FW_UPDATE_STATE_IDLE
         * the package URI is deleted from storage file
         * any active download is suspended
         */
        sid = LWM2MCORE_ERR_COMPLETED_OK;
    }
    else
    {
        /* Parameter check */
        if ((!bufferPtr)
         || (LWM2MCORE_PACKAGE_URI_MAX_LEN < len)
         || (LWM2MCORE_MAX_UPDATE_TYPE <= type))
        {
            sid = LWM2MCORE_ERR_INVALID_ARG;
        }
        else
        {
            /* Package URI: LWM2MCORE_PACKAGE_URI_MAX_LEN+1 for null byte: string format */
            uint8_t downloadUri[LWM2MCORE_PACKAGE_URI_MAX_LEN+1];
            memset(downloadUri, 0, LWM2MCORE_PACKAGE_URI_MAX_LEN+1);
            memcpy(downloadUri, bufferPtr, len);

            /* Call API to launch the package download
             * Advice: the package download needs to be made in a dedicated thread/task.
             */
            sid = LWM2MCORE_ERR_COMPLETED_OK;
        }
    }
    return sid;
}

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
lwm2mcore_sid_t os_portUpdateGetPackageUri
(
    lwm2mcore_updateType_t type,    ///< [IN] Update type
    uint16_t instanceId,            ///< [IN] Intance Id (0 for FW, any value for SW)
    char* bufferPtr,                ///< [INOUT] data buffer
    size_t* lenPtr                  ///< [INOUT] length of input buffer and length of the returned
                                    ///< data
)
{
    lwm2mcore_sid_t sid;

    if ((NULL == bufferPtr) || (NULL == lenPtr) || (LWM2MCORE_MAX_UPDATE_TYPE <= type))
    {
        sid = LWM2MCORE_ERR_INVALID_ARG;
    }
    else
    {
        sid = LWM2MCORE_ERR_NOT_YET_IMPLEMENTED;
    }
    return sid;
}

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
lwm2mcore_sid_t os_portUpdateLaunchUpdate
(
    lwm2mcore_updateType_t type,    ///< [IN] Update type
    uint16_t instanceId,            ///< [IN] Intance Id (0 for FW, any value for SW)
    char* bufferPtr,                ///< [INOUT] data buffer
    size_t len                      ///< [IN] length of input buffer
)
{
    lwm2mcore_sid_t sid;
    if (LWM2MCORE_MAX_UPDATE_TYPE <= type)
    {
        sid = LWM2MCORE_ERR_INVALID_ARG;
    }
    else
    {
        /* Call API to launch the update process
         * Set the update state to LWM2MCORE_FW_UPDATE_STATE_UPDATING
         */
        sid = LWM2MCORE_ERR_COMPLETED_OK;
    }
    return sid;
}

//--------------------------------------------------------------------------------------------------
/**
 * The server requires the update state
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_sid_t os_portUpdateGetUpdateState
(
    lwm2mcore_updateType_t type,    ///< [IN] Update type
    uint16_t instanceId,            ///< [IN] Intance Id (0 for FW, any value for SW)
    uint8_t* updateStatePtr         ///< [OUT] Firmware update state
)
{
    lwm2mcore_sid_t sid;
    if ((NULL == updateStatePtr) || (LWM2MCORE_MAX_UPDATE_TYPE <= type))
    {
        sid = LWM2MCORE_ERR_INVALID_ARG;
    }
    else
    {
        /* Call API to get the update state
         * Default value is LWM2MCORE_FW_UPDATE_RESULT_DEFAULT_NORMAL.
         * When the update process succeeds, the update result needs to be set to
         * LWM2MCORE_FW_UPDATE_RESULT_INSTALLED_SUCCESSFUL
         * Others values (see lwm2mcore_fw_update_result_t) are related to update process error.
         */
        if (LWM2MCORE_FW_UPDATE_TYPE == type)
        {
            /* Firmware update */
            *updateStatePtr = LWM2MCORE_FW_UPDATE_STATE_IDLE;
            sid = LWM2MCORE_ERR_COMPLETED_OK;
        }
        else
        {
            /* Software update */
            sid = LWM2MCORE_ERR_NOT_YET_IMPLEMENTED;
        }
    }
    return sid;
}

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
lwm2mcore_sid_t os_portUpdateGetUpdateResult
(
    lwm2mcore_updateType_t type,    ///< [IN] Update type
    uint16_t instanceId,            ///< [IN] Intance Id (0 for FW, any value for SW)
    uint8_t* updateResultPtr        ///< [OUT] Firmware update result
)
{
    lwm2mcore_sid_t sid;
    if ((NULL == updateResultPtr) || (LWM2MCORE_MAX_UPDATE_TYPE <= type))
    {
        sid = LWM2MCORE_ERR_INVALID_ARG;
    }
    else
    {
        /* Call API to get the update result
         * For the moment, hard-coded to LWM2MCORE_FW_UPDATE_RESULT_DEFAULT_NORMAL
         */
         if (LWM2MCORE_FW_UPDATE_TYPE == type)
        {
            /* Firmware update */
            *updateResultPtr = LWM2MCORE_FW_UPDATE_RESULT_DEFAULT_NORMAL;
            sid = LWM2MCORE_ERR_COMPLETED_OK;
        }
        else
        {
            /* Software update */
            sid = LWM2MCORE_ERR_NOT_YET_IMPLEMENTED;
        }
    }
    return sid;
}

//--------------------------------------------------------------------------------------------------
/**
 * The server requires the package name
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_sid_t os_portUpdateGetPackageName
(
    lwm2mcore_updateType_t type,    ///< [IN] Update type
    uint16_t instanceId,            ///< [IN] Intance Id (0 for FW, any value for SW)
    char* bufferPtr,                ///< [INOUT] data buffer
    size_t* lenPtr                  ///< [INOUT] length of input buffer and length of the returned
                                    ///< data
)
{
    return LWM2MCORE_ERR_OP_NOT_SUPPORTED;
}

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
lwm2mcore_sid_t os_portUpdateGetPackageVersion
(
    lwm2mcore_updateType_t type,    ///< [IN] Update type
    uint16_t instanceId,            ///< [IN] Intance Id (0 for FW, any value for SW)
    char* bufferPtr,                ///< [INOUT] data buffer
    size_t* lenPtr                  ///< [INOUT] length of input buffer and length of the returned
                                    ///< data
)
{
    return LWM2MCORE_ERR_OP_NOT_SUPPORTED;
}

