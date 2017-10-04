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
static PackageDownloaderWorkspace_t PkgDwlDefaultWorkspace =
{
    .version             = PKGDWL_WORKSPACE_VERSION,
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

    if (!pkgDwlWorkspacePtr)
    {
        return DWL_FAULT;
    }

    // Check if the package downloader workspace is stored
    sid = lwm2mcore_GetParam(LWM2MCORE_DWL_WORKSPACE_PARAM, (uint8_t*)pkgDwlWorkspacePtr, &len);
    LOG_ARG("Read download workspace: len=%zu, result=%d", len, sid);

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

    return DWL_FAULT;
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
    lwm2mcore_Sid_t sid = lwm2mcore_SetParam(LWM2MCORE_DWL_WORKSPACE_PARAM,
                                             (uint8_t*)pkgDwlWorkspacePtr,
                                             sizeof(PackageDownloaderWorkspace_t));
    if (LWM2MCORE_ERR_COMPLETED_OK == sid)
    {
        result = DWL_OK;
    }
    else
    {
        LOG_ARG("Save download workspace failed: sid=%d", sid);
    }

    return result;
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

    LOG_ARG("Delete download workspace: result=%d", result);

    return result;
}
