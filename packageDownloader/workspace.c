/**
 * @file workspace.c
 *
 * LWM2M Core package downloader workspace
 *
 * <HR>
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#include <stdio.h>
#include <liblwm2m.h>
#include <internals.h>
#include <lwm2mcore/lwm2mcore.h>
#include <lwm2mcore/paramStorage.h>
#include <lwm2mcore/update.h>
#include "workspace.h"

//--------------------------------------------------------------------------------------------------
// Symbol and Enum definitions
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Static variables
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Default values of package downloader workspace
 */
//--------------------------------------------------------------------------------------------------
static const PackageDownloaderWorkspace_t PkgDwlDefaultWorkspace =
{
    .version    = PKGDWL_WORKSPACE_VERSION,
    .updateType = LWM2MCORE_MAX_UPDATE_TYPE,
    .fwState    = LWM2MCORE_FW_UPDATE_STATE_IDLE,
    .fwResult   = LWM2MCORE_FW_UPDATE_RESULT_DEFAULT_NORMAL,
};

//--------------------------------------------------------------------------------------------------
// Public functions
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Function to read the package downloader workspace from platform memory
 *
 * @return
 *  - DWL_OK    The function succeeded
 *  - DWL_FAULT The function failed
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_DwlResult_t ReadPkgDwlWorkspace
(
    PackageDownloaderWorkspace_t* pkgDwlWorkspacePtr    ///< Package downloader workspace
)
{
    lwm2mcore_Sid_t sid;
    size_t len = sizeof(PackageDownloaderWorkspace_t);

#ifdef LEGACY_FW_STATUS
    lwm2mcore_FwUpdateState_t updateState;
    lwm2mcore_FwUpdateResult_t updateResult;
#endif

    if (!pkgDwlWorkspacePtr)
    {
        return DWL_FAULT;
    }

    // Check if the package downloader workspace is stored
    sid = lwm2mcore_GetParam(LWM2MCORE_DWL_WORKSPACE_PARAM, (uint8_t*)pkgDwlWorkspacePtr, &len);
    LOG_ARG("Read download workspace: len = %zu, result = %d", len, sid);

    if (   (LWM2MCORE_ERR_COMPLETED_OK == sid)
        && (sizeof(PackageDownloaderWorkspace_t) == len)
       )
    {
        LOG_ARG("Package downloader workspace version %d (only %d supported)",
                pkgDwlWorkspacePtr->version, PKGDWL_WORKSPACE_VERSION);

        // Check if the version is the supported one
        if (PKGDWL_WORKSPACE_VERSION == pkgDwlWorkspacePtr->version)
        {
            return DWL_OK;
        }
    }

    LOG("Failed to read the download workspace");

    if (len)
    {
        // The workspace is present but the size is not correct, delete it
        LOG("Delete download workspace");
        sid = lwm2mcore_DeleteParam(LWM2MCORE_DWL_WORKSPACE_PARAM);
    }

    // Copy the default configuration
    memcpy(pkgDwlWorkspacePtr, &PkgDwlDefaultWorkspace, sizeof(PackageDownloaderWorkspace_t));

#ifdef LEGACY_FW_STATUS
    // Previous package downloader design saves the firmware update state and result in dedicated
    // files. To ensure compatibility with current design, we need to check if these parameters
    // exist in storage
    if (LWM2MCORE_ERR_COMPLETED_OK == lwm2mcore_GetLegacyFwUpdateState(&updateState)
        && (LWM2MCORE_ERR_COMPLETED_OK == lwm2mcore_GetLegacyFwUpdateResult(&updateResult)))
    {
        LOG("Firmware update state and result found");
        pkgDwlWorkspacePtr->fwState = updateState;
        pkgDwlWorkspacePtr->fwResult = updateResult;
        pkgDwlWorkspacePtr->updateType = LWM2MCORE_FW_UPDATE_TYPE;
    }

    if(DWL_OK != WritePkgDwlWorkspace(pkgDwlWorkspacePtr))
    {
        return DWL_FAULT;
    }
#endif

    return DWL_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to write the package downloader workspace in platform memory
 *
 * @return
 *  - DWL_OK    The function succeeded
 *  - DWL_FAULT The function failed
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_DwlResult_t WritePkgDwlWorkspace
(
    PackageDownloaderWorkspace_t* pkgDwlWorkspacePtr    ///< Package downloader workspace
)
{
    lwm2mcore_DwlResult_t result = DWL_FAULT;
    lwm2mcore_Sid_t sid;

    if (!pkgDwlWorkspacePtr)
    {
        return DWL_FAULT;
    }

    sid = lwm2mcore_SetParam(LWM2MCORE_DWL_WORKSPACE_PARAM,
                             (uint8_t*)pkgDwlWorkspacePtr,
                             sizeof(PackageDownloaderWorkspace_t));
    if (LWM2MCORE_ERR_COMPLETED_OK == sid)
    {
        result = DWL_OK;
    }
    else
    {
        LOG_ARG("Save download workspace failed: sid = %d", sid);
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get TPF mode state
 *
 * @return
 *  - DWL_OK    The function succeeded
 *  - DWL_FAULT The function failed
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_DwlResult_t GetTpfWorkspace
(
    bool* isTpfEnabledPtr                   ///< [OUT] True if TFP mode is enabled, false otherwise
)
{
    lwm2mcore_Sid_t sid;
    sid = lwm2mcore_GetTpfState(isTpfEnabledPtr);
    if(LWM2MCORE_ERR_COMPLETED_OK == sid)
    {
        return DWL_OK;
    }
    return DWL_FAULT;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to delete the package downloader workspace in platform memory
 *
 * @return
 *  - DWL_OK    The function succeeded
 *  - DWL_FAULT The function failed
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_DwlResult_t DeletePkgDwlWorkspace
(
    void
)
{
    lwm2mcore_DwlResult_t result = DWL_FAULT;
    lwm2mcore_Sid_t sid = lwm2mcore_DeleteParam(LWM2MCORE_DWL_WORKSPACE_PARAM);
    if (LWM2MCORE_ERR_COMPLETED_OK == sid)
    {
        result = DWL_OK;
    }

    LOG_ARG("Delete download workspace: result = %d", result);

    return result;
}
