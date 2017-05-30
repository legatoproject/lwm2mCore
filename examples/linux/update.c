/**
 * @file update.c
 *
 * Porting layer for Firmware Over The Air update
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#include <stdint.h>
#include <stddef.h>
#include <platform/types.h>
#include <lwm2mcore/lwm2mcore.h>
#include <lwm2mcore/update.h>

//--------------------------------------------------------------------------------------------------
/**
 * The server pushes a package to the LWM2M client
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
lwm2mcore_Sid_t lwm2mcore_PushUpdatePackage
(
    lwm2mcore_UpdateType_t type,    ///< [IN] Update type
    uint16_t instanceId,            ///< [IN] Instance Id (0 for FW, any value for SW)
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
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_SetUpdatePackageUri
(
    lwm2mcore_UpdateType_t type,    ///< [IN] Update type
    uint16_t instanceId,            ///< [IN] Instance Id (0 for FW, any value for SW)
    char* bufferPtr,                ///< [INOUT] data buffer
    size_t len                      ///< [IN] length of input buffer
)
{
    lwm2mcore_Sid_t sid;

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
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetUpdatePackageUri
(
    lwm2mcore_UpdateType_t type,    ///< [IN] Update type
    uint16_t instanceId,            ///< [IN] Instance Id (0 for FW, any value for SW)
    char* bufferPtr,                ///< [INOUT] data buffer
    size_t* lenPtr                  ///< [INOUT] length of input buffer and length of the returned
                                    ///< data
)
{
    lwm2mcore_Sid_t sid;

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
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_LaunchUpdate
(
    lwm2mcore_UpdateType_t type,    ///< [IN] Update type
    uint16_t instanceId,            ///< [IN] Instance Id (0 for FW, any value for SW)
    char* bufferPtr,                ///< [INOUT] data buffer
    size_t len                      ///< [IN] length of input buffer
)
{
    lwm2mcore_Sid_t sid;
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
lwm2mcore_Sid_t lwm2mcore_GetUpdateState
(
    lwm2mcore_UpdateType_t type,    ///< [IN] Update type
    uint16_t instanceId,            ///< [IN] Instance Id (0 for FW, any value for SW)
    uint8_t* updateStatePtr         ///< [OUT] Firmware update state
)
{
    lwm2mcore_Sid_t sid;
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
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetUpdateResult
(
    lwm2mcore_UpdateType_t type,    ///< [IN] Update type
    uint16_t instanceId,            ///< [IN] Instance Id (0 for FW, any value for SW)
    uint8_t* updateResultPtr        ///< [OUT] Firmware update result
)
{
    lwm2mcore_Sid_t sid;
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
lwm2mcore_Sid_t lwm2mcore_GetUpdatePackageName
(
    lwm2mcore_UpdateType_t type,    ///< [IN] Update type
    uint16_t instanceId,            ///< [IN] Instance Id (0 for FW, any value for SW)
    char* bufferPtr,                ///< [INOUT] data buffer
    uint32_t len                    ///< [IN] length of input buffer
)
{
    return LWM2MCORE_ERR_OP_NOT_SUPPORTED;
}

//--------------------------------------------------------------------------------------------------
/**
 * The server requires the package version
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
lwm2mcore_Sid_t lwm2mcore_GetUpdatePackageVersion
(
    lwm2mcore_UpdateType_t type,    ///< [IN] Update type
    uint16_t instanceId,            ///< [IN] Instance Id (0 for FW, any value for SW)
    char* bufferPtr,                ///< [INOUT] data buffer
    uint32_t len                    ///< [IN] length of input buffer
)
{
    return LWM2MCORE_ERR_OP_NOT_SUPPORTED;
}

//--------------------------------------------------------------------------------------------------
/**
 * The server sets the "update supported objects" field for software update
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
lwm2mcore_Sid_t lwm2mcore_SetSwUpdateSupportedObjects
(
    uint16_t instanceId,            ///< [IN] Instance Id (any value for SW)
    bool value                      ///< [IN] Update supported objects field value
)
{
    return LWM2MCORE_ERR_COMPLETED_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * The server requires the "update supported objects" field for software update
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
lwm2mcore_Sid_t lwm2mcore_GetSwUpdateSupportedObjects
(
    uint16_t instanceId,            ///< [IN] Instance Id (any value for SW)
    bool* valuePtr                  ///< [INOUT] Update supported objects field value
)
{
    if (NULL == valuePtr)
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }
    else
    {
        *valuePtr = true;
        return LWM2MCORE_ERR_COMPLETED_OK;
    }
}


//--------------------------------------------------------------------------------------------------
/**
 * The server requires the activation state for one embedded application
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
lwm2mcore_Sid_t lwm2mcore_GetSwUpdateActivationState
(
    uint16_t instanceId,            ///< [IN] Instance Id (any value for SW)
    bool* valuePtr                  ///< [INOUT] Activation state
)
{
    if (NULL == valuePtr)
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }
    else
    {
        *valuePtr = true;
        return LWM2MCORE_ERR_COMPLETED_OK;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * The server requires an embedded application to be uninstalled (only for software update)
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
lwm2mcore_Sid_t lwm2mcore_LaunchSwUpdateUninstall
(
    uint16_t instanceId,            ///< [IN] Instance Id (any value for SW)
    char* bufferPtr,                ///< [INOUT] data buffer
    size_t len                      ///< [IN] length of input buffer
)
{
    return LWM2MCORE_ERR_NOT_YET_IMPLEMENTED;
}

//--------------------------------------------------------------------------------------------------
/**
 * The server requires an embedded application to be activated or deactivated (only for software
 * update)
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
lwm2mcore_Sid_t lwm2mcore_ActivateSoftware
(
    bool activation,        ///< [IN] Requested activation (true: activate, false: deactivate)
    uint16_t instanceId,    ///< [IN] Instance Id (any value for SW)
    char* bufferPtr,        ///< [INOUT] data buffer
    size_t len              ///< [IN] length of input buffer
)
{
    if ((NULL == bufferPtr) && len)
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }
    else
    {
        return LWM2MCORE_ERR_NOT_YET_IMPLEMENTED;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * The server request to create or delete an object instance of object 9
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
lwm2mcore_Sid_t lwm2mcore_SoftwareUpdateInstance
(
    bool create,                ///<[IN] Create (true) or delete (false)
    uint16_t instanceId         ///<[IN] Object instance Id to create or delete
)
{
    return LWM2MCORE_ERR_NOT_YET_IMPLEMENTED;
}

//--------------------------------------------------------------------------------------------------
/**
 * Resume a package download if necessary
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
lwm2mcore_Sid_t lwm2mcore_ResumePackageDownload
(
    void
)
{
    return LWM2MCORE_ERR_OP_NOT_SUPPORTED;
}

//--------------------------------------------------------------------------------------------------
/**
 * Suspend a package download if necessary
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_SuspendPackageDownload
(
    void
)
{
    return LWM2MCORE_ERR_COMPLETED_OK;
}
