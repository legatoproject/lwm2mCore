/**
 * @file update.c
 *
 * LwM2Mcore update manager
 *
 * <HR>
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#include <stdio.h>
#include <internals.h>
#include <lwm2mcore/lwm2mcore.h>
#include <lwm2mcore/update.h>
#include "workspace.h"
#include "updateAgent.h"

//--------------------------------------------------------------------------------------------------
// Symbol and Enum definitions
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Static variables
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Internal functions
//--------------------------------------------------------------------------------------------------

#ifndef LWM2M_EXTERNAL_DOWNLOADER

//--------------------------------------------------------------------------------------------------
/**
 * Function to initialize a package update
 *
 * @note
 * This function is not available if @c LWM2M_EXTERNAL_DOWNLOADER compilation flag is embedded
 *
 * @return
 *  - LWM2MCORE_ERR_COMPLETED_OK on success
 *  - LWM2MCORE_ERR_INVALID_ARG when the package URL is not valid
 *  - LWM2MCORE_ERR_GENERAL_ERROR on failure
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t downloader_InitializeDownload
(
    lwm2mcore_UpdateType_t type,    ///< [IN] Update type
    uint16_t instanceId,            ///< [IN] Instance Id (0 for FW, any value for SW)
    char* bufferPtr,                ///< [INOUT] data buffer
    size_t len                      ///< [IN] length of input buffer
)
{
    PackageDownloaderWorkspace_t workspace;

    (void)instanceId;

    if (DWL_OK != ReadPkgDwlWorkspace(&workspace))
    {
        return LWM2MCORE_ERR_GENERAL_ERROR;
    }

    // Copy the URL in the workspace
    memset(workspace.url, 0, LWM2MCORE_PACKAGE_URI_MAX_BYTES);

    // Copy the updateType
    workspace.updateType = type;

    if ( (!bufferPtr)
      || (LWM2MCORE_PACKAGE_URI_MAX_LEN < len))
    {
        workspace.fwState = LWM2MCORE_FW_UPDATE_STATE_IDLE;
        workspace.fwResult = LWM2MCORE_FW_UPDATE_RESULT_INVALID_URI;
        // Store the workspace
        if (DWL_OK != WritePkgDwlWorkspace(&workspace))
        {
            LOG("Error on saving workspace");
            return LWM2MCORE_ERR_GENERAL_ERROR;
        }
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    // Copy the URL in the workspace
    snprintf(workspace.url, LWM2MCORE_PACKAGE_URI_MAX_BYTES, "%s", bufferPtr);

    // Set update result and state fields to initial values
    switch (type)
    {
        case LWM2MCORE_FW_UPDATE_TYPE:
            workspace.fwState = LWM2MCORE_FW_UPDATE_STATE_IDLE;
            workspace.fwResult = LWM2MCORE_FW_UPDATE_RESULT_DEFAULT_NORMAL;
            break;

        case LWM2MCORE_SW_UPDATE_TYPE:
            LOG("Init downloader for SOTA: TODO");
            break;

        default:
            return LWM2MCORE_ERR_INVALID_ARG;
    }

    // Store the workspace
    if (DWL_OK != WritePkgDwlWorkspace(&workspace))
    {
        LOG("Error on saving workspace");
        return LWM2MCORE_ERR_GENERAL_ERROR;
    }
    LOG_ARG("Stored url %s", workspace.url);

    return LWM2MCORE_ERR_COMPLETED_OK;
}

#endif /* !LWM2M_EXTERNAL_DOWNLOADER */

//--------------------------------------------------------------------------------------------------
/**
 * Function to get an update state
 *
 * @return
 *  - LWM2MCORE_ERR_COMPLETED_OK on success
 *  - LWM2MCORE_ERR_INVALID_ARG when parameter is invalid
 *  - LWM2MCORE_ERR_GENERAL_ERROR on failure
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t downloader_GetFwUpdateState
(
    lwm2mcore_FwUpdateState_t*  statePtr    ///< [IN] FW update state
)
{
    PackageDownloaderWorkspace_t workspace;

    if (!statePtr)
    {
        LOG("statePtr NULL");
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    if (DWL_OK != ReadPkgDwlWorkspace(&workspace))
    {
        LOG("Error to read workspace");
        return LWM2MCORE_ERR_GENERAL_ERROR;
    }

    *statePtr = workspace.fwState;

    // state cannot exceed LWM2MCORE_FW_UPDATE_STATE_WAITINSTALLRESULT
    if (*statePtr > LWM2MCORE_FW_UPDATE_STATE_WAITINSTALLRESULT)
    {
        LOG_ARG("Reset invalid fw update state(%d) from workspace", *statePtr);
        *statePtr = LWM2MCORE_FW_UPDATE_STATE_IDLE;
    }

    LOG_ARG("fw State %d", *statePtr);
    return LWM2MCORE_ERR_COMPLETED_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to get an update result
 *
 * @return
 *  - LWM2MCORE_ERR_COMPLETED_OK on success
 *  - LWM2MCORE_ERR_INVALID_ARG when parameter is invalid
 *  - LWM2MCORE_ERR_GENERAL_ERROR on failure
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t downloader_GetFwUpdateResult
(
    lwm2mcore_FwUpdateResult_t* resultPtr   ///< [IN] FW update result
)
{
    PackageDownloaderWorkspace_t workspace;

    if (!resultPtr)
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    if (DWL_OK != ReadPkgDwlWorkspace(&workspace))
    {
        LOG("Error to read workspace");
        return LWM2MCORE_ERR_GENERAL_ERROR;
    }

    *resultPtr = workspace.fwResult;

    // result cannot exceed LWM2MCORE_FW_UPDATE_RESULT_UNSUPPORTED_PROTOCOL
    if (*resultPtr > LWM2MCORE_FW_UPDATE_RESULT_UNSUPPORTED_PROTOCOL)
    {
        LOG_ARG("Reset invalid fw update result(%d) from workspace", *resultPtr);
        *resultPtr = LWM2MCORE_FW_UPDATE_RESULT_DEFAULT_NORMAL;
    }

    LOG_ARG("fw Result %d", *resultPtr);
    return LWM2MCORE_ERR_COMPLETED_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to set an update state
 *
 * @return
 *  - LWM2MCORE_ERR_COMPLETED_OK on success
 *  - LWM2MCORE_ERR_GENERAL_ERROR on failure
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t downloader_SetFwUpdateState
(
    lwm2mcore_FwUpdateState_t   state   ///< [IN] FW update state
)
{
    PackageDownloaderWorkspace_t workspace;

    if (DWL_OK != ReadPkgDwlWorkspace(&workspace))
    {
        return LWM2MCORE_ERR_GENERAL_ERROR;
    }

    // state cannot exceed LWM2MCORE_FW_UPDATE_STATE_WAITINSTALLRESULT
    if (state > LWM2MCORE_FW_UPDATE_STATE_WAITINSTALLRESULT)
    {
        LOG("Invalid Fw update state");
        return LWM2MCORE_ERR_GENERAL_ERROR;
    }

    LOG_ARG("Set Fw state %d -> %d", workspace.fwState, state);

    if (workspace.fwState != state)
    {
        workspace.fwState = state;

        // Store the workspace
        if (DWL_OK != WritePkgDwlWorkspace(&workspace))
        {
            return LWM2MCORE_ERR_GENERAL_ERROR;
        }
    }

    return LWM2MCORE_ERR_COMPLETED_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to set an update result
 *
 * @return
 *  - LWM2MCORE_ERR_COMPLETED_OK on success
 *  - LWM2MCORE_ERR_GENERAL_ERROR on failure
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t downloader_SetFwUpdateResult
(
    lwm2mcore_FwUpdateResult_t   result ///< [IN] FW update result
)
{
    PackageDownloaderWorkspace_t workspace;

    if (DWL_OK != ReadPkgDwlWorkspace(&workspace))
    {
        return LWM2MCORE_ERR_GENERAL_ERROR;
    }

    // result cannot exceed LWM2MCORE_FW_UPDATE_RESULT_UNSUPPORTED_PROTOCOL
    if (result > LWM2MCORE_FW_UPDATE_RESULT_UNSUPPORTED_PROTOCOL)
    {
        LOG("Invalid firmware update result");
        return LWM2MCORE_ERR_GENERAL_ERROR;
    }

    LOG_ARG("Set Fw result %d -> %d", workspace.fwResult, result);

    if (workspace.fwResult != result)
    {
        workspace.fwResult = result;

        // Store the workspace
        if (DWL_OK != WritePkgDwlWorkspace(&workspace))
        {
            return LWM2MCORE_ERR_GENERAL_ERROR;
        }
    }

    return LWM2MCORE_ERR_COMPLETED_OK;
}

//--------------------------------------------------------------------------------------------------
// Public functions
//--------------------------------------------------------------------------------------------------

#ifndef LWM2M_EXTERNAL_DOWNLOADER
//--------------------------------------------------------------------------------------------------
/**
 * Function to indicate that a package download/install failed on client side
 *
 * @note
 * This function is not available if @c LWM2M_EXTERNAL_DOWNLOADER compilation flag is embedded
 *
 * @return
 *  - LWM2MCORE_ERR_COMPLETED_OK on success
 *  - LWM2MCORE_ERR_GENERAL_ERROR on failure
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_SetDownloadError
(
    lwm2mcore_UpdateError_t error   ///< [IN] Update error
)
{
    PackageDownloaderWorkspace_t workspace;
    lwm2mcore_FwUpdateResult_t fwResult = LWM2MCORE_FW_UPDATE_RESULT_INVALID_URI;
    lwm2mcore_SwUpdateResult_t swResult = LWM2MCORE_SW_UPDATE_RESULT_INVALID_URI;

    if (DWL_OK != ReadPkgDwlWorkspace(&workspace))
    {
        return LWM2MCORE_ERR_GENERAL_ERROR;
    }

    if (LWM2MCORE_MAX_UPDATE_TYPE == workspace.updateType)
    {
        return LWM2MCORE_ERR_GENERAL_ERROR;
    }

    // Erase URL
    memset(workspace.url, 0, LWM2MCORE_PACKAGE_URI_MAX_BYTES);
    workspace.packageSize = 0;

    if (DWL_OK != WritePkgDwlWorkspace(&workspace))
    {
        LOG("Error when updating workspace");
    }

    LOG_ARG("Set package download error %d", error);

    switch (error)
    {
        case LWM2MCORE_UPDATE_ERROR_NO_STORAGE_SPACE:
            if (LWM2MCORE_FW_UPDATE_TYPE == workspace.updateType)
            {
                fwResult = LWM2MCORE_FW_UPDATE_RESULT_NO_STORAGE_SPACE;
            }
            else if (LWM2MCORE_SW_UPDATE_TYPE == workspace.updateType)
            {
                swResult = LWM2MCORE_SW_UPDATE_RESULT_NOT_ENOUGH_MEMORY;
            }
            break;

        case LWM2MCORE_UPDATE_ERROR_OUT_OF_MEMORY:
            if (LWM2MCORE_FW_UPDATE_TYPE == workspace.updateType)
            {
                fwResult = LWM2MCORE_FW_UPDATE_RESULT_OUT_OF_MEMORY;
            }
            else if (LWM2MCORE_SW_UPDATE_TYPE == workspace.updateType)
            {
                swResult = LWM2MCORE_SW_UPDATE_RESULT_OUT_OF_MEMORY;
            }
            break;

        case LWM2MCORE_UPDATE_ERROR_CONNECTION_LOST:
            if (LWM2MCORE_FW_UPDATE_TYPE == workspace.updateType)
            {
                fwResult = LWM2MCORE_FW_UPDATE_RESULT_COMMUNICATION_ERROR;
            }
            else if (LWM2MCORE_SW_UPDATE_TYPE == workspace.updateType)
            {
                swResult = LWM2MCORE_SW_UPDATE_RESULT_CONNECTION_LOST;
            }
            break;

        case LWM2MCORE_UPDATE_ERROR_UNSUPPORTED_PACKAGE:
            if (LWM2MCORE_FW_UPDATE_TYPE == workspace.updateType)
            {
                fwResult = LWM2MCORE_FW_UPDATE_RESULT_UNSUPPORTED_PKG_TYPE;
            }
            else if (LWM2MCORE_SW_UPDATE_TYPE == workspace.updateType)
            {
                swResult = LWM2MCORE_SW_UPDATE_RESULT_UNSUPPORTED_TYPE;
            }
            break;

        case LWM2MCORE_UPDATE_ERROR_DEVICE_SPECIFIC:
            if (LWM2MCORE_FW_UPDATE_TYPE == workspace.updateType)
            {
                // No specific result for FW update
                fwResult = LWM2MCORE_FW_UPDATE_RESULT_VERIFY_ERROR;
            }
            else if (LWM2MCORE_SW_UPDATE_TYPE == workspace.updateType)
            {
                swResult = LWM2MCORE_SW_UPDATE_RESULT_DEVICE_ERROR;
            }
            break;

        default:
        return LWM2MCORE_ERR_GENERAL_ERROR;
    }


    if (LWM2MCORE_FW_UPDATE_TYPE == workspace.updateType)
    {
        downloader_SetFwUpdateState(LWM2MCORE_FW_UPDATE_STATE_IDLE);
        downloader_SetFwUpdateResult(fwResult);
    }
    else if (LWM2MCORE_SW_UPDATE_TYPE == workspace.updateType)
    {
        lwm2mcore_SetSwUpdateState(LWM2MCORE_SW_UPDATE_STATE_INITIAL);
        lwm2mcore_SetSwUpdateResult(swResult);
    }
    return LWM2MCORE_ERR_COMPLETED_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Indicates that the Firmware update is accepted
 *
 * @note
 * This function is not available if @c LWM2M_EXTERNAL_DOWNLOADER compilation flag is embedded
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
)
{
    PackageDownloaderWorkspace_t workspace;
    lwm2mcore_Sid_t result = LWM2MCORE_ERR_GENERAL_ERROR;

    if (DWL_OK != ReadPkgDwlWorkspace(&workspace))
    {
        LOG("Error to read workspace");
        return LWM2MCORE_ERR_GENERAL_ERROR;
    }

    LOG_ARG("lwm2mcore_SetUpdateAccepted workspace update type %d", workspace.updateType);
    switch (workspace.updateType)
    {
        case LWM2MCORE_FW_UPDATE_TYPE:
        {
            if( (LWM2MCORE_FW_UPDATE_STATE_DOWNLOADED == workspace.fwState)
             && (LWM2MCORE_FW_UPDATE_RESULT_DEFAULT_NORMAL == workspace.fwResult))
            {
                result = downloader_SetFwUpdateState(LWM2MCORE_FW_UPDATE_STATE_UPDATING);

#ifdef LEGACY_FW_STATUS
                // Previous package downloader design saves the firmware update state and result in
                // dedicated files.
                // We save update state and result in old style in case the downloaded firmware
                // containts an old package downloader design.
                lwm2mcore_SetLegacyFwUpdateState(LWM2MCORE_FW_UPDATE_STATE_UPDATING);
                lwm2mcore_SetLegacyFwUpdateResult(LWM2MCORE_FW_UPDATE_RESULT_DEFAULT_NORMAL);
#endif
                return result;
            }

            if( (LWM2MCORE_FW_UPDATE_STATE_UPDATING == workspace.fwState)
             && (LWM2MCORE_FW_UPDATE_RESULT_DEFAULT_NORMAL == workspace.fwResult))
            {
                LOG("FW update state already set to UPDATING");
                return LWM2MCORE_ERR_COMPLETED_OK;
            }
            else
            {
                LOG_ARG("Invalid FW update state %d, result %d",
                        workspace.fwState, workspace.fwResult);
                return LWM2MCORE_ERR_INVALID_STATE;
            }
        }
        break;

        case LWM2MCORE_SW_UPDATE_TYPE:
            LOG("Nothing to do in SW update case");
            return LWM2MCORE_ERR_COMPLETED_OK;

        default:
            LOG("Invalid update type");
            return LWM2MCORE_ERR_INVALID_STATE;
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Indicates that the Firmware update succeeds
 *
 * @note
 * This function is not available if @c LWM2M_EXTERNAL_DOWNLOADER compilation flag is embedded
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
)
{
    PackageDownloaderWorkspace_t workspace;
    lwm2mcore_Sid_t result = LWM2MCORE_ERR_GENERAL_ERROR;

    if (DWL_OK != ReadPkgDwlWorkspace(&workspace))
    {
        LOG("Error to read workspace");
        return LWM2MCORE_ERR_GENERAL_ERROR;
    }

    switch (workspace.updateType)
    {
        case LWM2MCORE_FW_UPDATE_TYPE:
        {
            lwm2mcore_FwUpdateState_t fwUpdateState = LWM2MCORE_FW_UPDATE_STATE_IDLE;
            lwm2mcore_FwUpdateResult_t fwUpdateResult = LWM2MCORE_FW_UPDATE_RESULT_DEFAULT_NORMAL;

            // Check if a FW update was ongoing
            if (   (LWM2MCORE_ERR_COMPLETED_OK != downloader_GetFwUpdateState(&fwUpdateState))
                || (LWM2MCORE_ERR_COMPLETED_OK != downloader_GetFwUpdateResult(&fwUpdateResult)))
            {
                LOG("Error to get FW update state/result");
                return LWM2MCORE_ERR_GENERAL_ERROR;
            }

            if( (LWM2MCORE_FW_UPDATE_STATE_UPDATING != fwUpdateState)
             || (LWM2MCORE_FW_UPDATE_RESULT_DEFAULT_NORMAL != fwUpdateResult))
            {
                LOG_ARG("Invalid FW update state %d, result %d", fwUpdateState, fwUpdateResult);
                return LWM2MCORE_ERR_INVALID_STATE;
            }

            if (isSuccess)
            {
                result = downloader_SetFwUpdateState(LWM2MCORE_FW_UPDATE_STATE_IDLE);
                if (LWM2MCORE_ERR_COMPLETED_OK == result)
                {
                    result = downloader_SetFwUpdateResult(
                                                LWM2MCORE_FW_UPDATE_RESULT_INSTALLED_SUCCESSFUL);
                }
            }
            else
            {
                result = downloader_SetFwUpdateResult(LWM2MCORE_FW_UPDATE_RESULT_INSTALL_FAILURE);
            }
        }
        break;

        case LWM2MCORE_SW_UPDATE_TYPE:
            LOG("Nothing to do in SW update case");
            return LWM2MCORE_ERR_COMPLETED_OK;

        default:
            LOG("Invalid update type");
            return LWM2MCORE_ERR_INVALID_STATE;
    }

    return result;
}

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
)
{
    PackageDownloaderWorkspace_t workspace;

    if (!IsFwUpdateOnGoingPtr)
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    *IsFwUpdateOnGoingPtr = false;

    if (DWL_OK != ReadPkgDwlWorkspace(&workspace))
    {
        LOG("Error to read workspace");
        return LWM2MCORE_ERR_GENERAL_ERROR;
    }

    if (LWM2MCORE_FW_UPDATE_TYPE != workspace.updateType)
    {
        return LWM2MCORE_ERR_INVALID_STATE;
    }

    // Check if a FW update was ongoing
    if ((LWM2MCORE_FW_UPDATE_STATE_UPDATING == workspace.fwState)
     && (LWM2MCORE_FW_UPDATE_RESULT_DEFAULT_NORMAL == workspace.fwResult))
    {
        *IsFwUpdateOnGoingPtr = true;
    }
    return LWM2MCORE_ERR_COMPLETED_OK;
}

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
)
{
    PackageDownloaderWorkspace_t workspace;

    if (!IsFwUpdateInstallWaitedPtr)
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    *IsFwUpdateInstallWaitedPtr = false;

    if (DWL_OK != ReadPkgDwlWorkspace(&workspace))
    {
        LOG("Error to read workspace");
        return LWM2MCORE_ERR_GENERAL_ERROR;
    }

    if (LWM2MCORE_FW_UPDATE_TYPE != workspace.updateType)
    {
        return LWM2MCORE_ERR_INVALID_STATE;
    }

    // Check if a FW update was ongoing
    if ((LWM2MCORE_FW_UPDATE_STATE_DOWNLOADED == workspace.fwState)
     && (LWM2MCORE_FW_UPDATE_RESULT_DEFAULT_NORMAL == workspace.fwResult))
    {
        *IsFwUpdateInstallWaitedPtr = true;
    }
    return LWM2MCORE_ERR_COMPLETED_OK;
}

#endif /* !LWM2M_EXTERNAL_DOWNLOADER */
