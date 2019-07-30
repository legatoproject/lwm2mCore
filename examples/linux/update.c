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
#include <stdio.h>
#include <string.h>
#include <platform/types.h>
#include <lwm2mcore/lwm2mcore.h>
#include <lwm2mcore/update.h>
#include "downloader.h"
#include <pthread.h>
#include "workspace.h"
#include <unistd.h>
#include <unistd.h>
#include <sys/stat.h>
#include "update.h"

#ifndef LWM2M_EXTERNAL_DOWNLOADER
//--------------------------------------------------------------------------------------------------
/**
 * Static package downloader structure
 */
//--------------------------------------------------------------------------------------------------
static lwm2mcore_PackageDownloader_t PkgDwl;
#endif /* !LWM2M_EXTERNAL_DOWNLOADER */

//--------------------------------------------------------------------------------------------------
/**
 * Static thread for package download
 */
//--------------------------------------------------------------------------------------------------
static pthread_t DownloadThread = 0;

//--------------------------------------------------------------------------------------------------
/**
 * Download thread data
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
   lwm2mcore_UpdateType_t   updateType;     ///< Update type
   uint64_t                 packageSize;    ///< Package size
   char*                    urlPtr;         ///< Package URL
   int                      result;         ///< Package download result
   bool                     isResume;       ///< Indicates if it's a download resume
}
DownloadThreadData_t;

DownloadThreadData_t downloadThreadData;

//--------------------------------------------------------------------------------------------------
/**
 * Function to treat a download start (DownloadThread)
 */
//--------------------------------------------------------------------------------------------------
static void* StartDownload
(
    void*   argPtr
)
{
    DownloadThreadData_t* data = (DownloadThreadData_t*)argPtr;


#ifndef LWM2M_EXTERNAL_DOWNLOADER
    //uint64_t packagesize;
    lwm2mcore_PackageDownloaderData_t dataPkg;
    PackageDownloaderWorkspace_t workspace;

    if (DWL_OK != ReadPkgDwlWorkspace(&workspace))
    {
        return 0;
    }

    // Set the package downloader data structure
    dataPkg.updateOffset = 0;
    dataPkg.isResume = data->isResume;
    PkgDwl.data = dataPkg;
    printf("StartDownload type %d: %s\n", data->updateType, data->urlPtr);

    if (LWM2MCORE_ERR_COMPLETED_OK != lwm2mcore_StartPackageDownloader(NULL))
    {
        printf("packageDownloadRun failed\n");
    }
#endif /* LWM2M_EXTERNAL_DOWNLOADER */
    data->result = 0;
    printf("Exit download thread");
    return 0;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function for setting software update state
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_SetSwUpdateState
(
    lwm2mcore_SwUpdateState_t swUpdateState     ///< [IN] New SW update state
)
{
    (void)swUpdateState;

    printf("update.c to be implemented\n");
    return LWM2MCORE_ERR_NOT_YET_IMPLEMENTED;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function for setting software update result
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_SetSwUpdateResult
(
    lwm2mcore_SwUpdateResult_t swUpdateResult   ///< [IN] New SW update result
)
{
    (void)swUpdateResult;

    printf("update.c to be implemented\n");
    return LWM2MCORE_ERR_NOT_YET_IMPLEMENTED;
}

#ifdef LEGACY_FW_STATUS
//--------------------------------------------------------------------------------------------------
/**
 * Set legacy firmware update state
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_SetLegacyFwUpdateState
(
    lwm2mcore_FwUpdateState_t fwUpdateState     ///< [IN] New FW update state
)
{
    (void)fwUpdateState;

    printf("update.c to be implemented\n");
    return LWM2MCORE_ERR_NOT_YET_IMPLEMENTED;
}

//--------------------------------------------------------------------------------------------------
/**
 * Set legacy firmware update result
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_SetLegacyFwUpdateResult
(
    lwm2mcore_FwUpdateResult_t fwUpdateResult   ///< [IN] New FW update result
)
{
    (void)fwUpdateResult;

    printf("update.c to be implemented\n");
    return LWM2MCORE_ERR_NOT_YET_IMPLEMENTED;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get legacy firmware update state
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetLegacyFwUpdateState
(
    lwm2mcore_FwUpdateState_t* fwUpdateStatePtr     ///< [INOUT] FW update state
)
{
    (void)fwUpdateStatePtr;

    printf("update.c to be implemented\n");
    return LWM2MCORE_ERR_NOT_YET_IMPLEMENTED;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get legacy firmware update result
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetLegacyFwUpdateResult
(
    lwm2mcore_FwUpdateResult_t* fwUpdateResultPtr   ///< [INOUT] FW update result
)
{
    (void)fwUpdateResultPtr;

    printf("update.c to be implemented\n");
    return LWM2MCORE_ERR_NOT_YET_IMPLEMENTED;
}

#endif // LEGACY_FW_STATUS

//--------------------------------------------------------------------------------------------------
/**
 * The server pushes a package to the LWM2M client
 *
 * @return
 *  - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
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
    if ((NULL == bufferPtr) || (LWM2MCORE_MAX_UPDATE_TYPE <= type))
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    (void)type;
    (void)instanceId;
    (void)bufferPtr;
    (void)len;

    printf("update.c to be implemented\n");
    return LWM2MCORE_ERR_NOT_YET_IMPLEMENTED;
}

//--------------------------------------------------------------------------------------------------
/**
 * The server requires the current package URI stored in the LWM2M client
 *
 * @return
 *  - LWM2MCORE_ERR_COMPLETED_OK on success
 *  - LWM2MCORE_ERR_INVALID_STATE if no package download is suspended
 *  - LWM2MCORE_ERR_GENERAL_ERROR on failure
 *  - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
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
    lwm2mcore_Sid_t result = LWM2MCORE_ERR_GENERAL_ERROR;
    PackageDownloaderWorkspace_t workspace;

    if ((NULL == bufferPtr) || (NULL == lenPtr) || (LWM2MCORE_MAX_UPDATE_TYPE <= type))
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    (void)instanceId;

    // Read the workspace
    if (DWL_OK != ReadPkgDwlWorkspace(&workspace))
    {
        return LWM2MCORE_ERR_GENERAL_ERROR;
    }

    if (type != workspace.updateType)
    {
        printf("Curent URL is not linked to the required type");
        return LWM2MCORE_ERR_INVALID_STATE;
    }

    // Set update result and state fields to initial values
    switch (type)
    {
        case LWM2MCORE_FW_UPDATE_TYPE:
                snprintf(bufferPtr, LWM2MCORE_PACKAGE_URI_MAX_BYTES, "%s", workspace.url);
                *lenPtr = strlen(bufferPtr);
                result = LWM2MCORE_ERR_COMPLETED_OK;
            break;

        case LWM2MCORE_SW_UPDATE_TYPE:
            printf("SOTA to be implemented\n");
            break;

        default:
            return LWM2MCORE_ERR_INVALID_ARG;
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * The server requests to launch an update
 *
 * @warning The client MUST store a parameter in non-volatile memory in order to keep in memory that
 * an install request was received and launch a timer (value could be decided by the client
 * implementation) in order to treat the install request.
 *
 * @return
 *  - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
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
    lwm2mcore_Sid_t result;
    (void)type;
    (void)instanceId;
    (void)bufferPtr;
    (void)len;

    if (LWM2MCORE_MAX_UPDATE_TYPE <= type)
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

     result = lwm2mcore_SetUpdateAccepted();
     if (LWM2MCORE_ERR_COMPLETED_OK != result)
     {
         return result;
     }
     return lwm2mcore_SetUpdateResult(true);
}

//--------------------------------------------------------------------------------------------------
/**
 * The server requires the software update state
 *
 * @return
 *  - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetSwUpdateState
(
    uint16_t instanceId,            ///< [IN] Instance Id (0 for FW, any value for SW)
    uint8_t* updateStatePtr         ///< [OUT] Firmware update state
)
{
    if (NULL == updateStatePtr)
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    (void)instanceId;
    (void)updateStatePtr;

    printf("update.c to be implemented\n");
    return LWM2MCORE_ERR_NOT_YET_IMPLEMENTED;
}

//--------------------------------------------------------------------------------------------------
/**
 * The server requires the software update result
 *
 * @return
 *  - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetSwUpdateResult
(
    uint16_t instanceId,            ///< [IN] Instance Id (0 for FW, any value for SW)
    uint8_t* updateResultPtr        ///< [OUT] Firmware update result
)
{
    if (NULL == updateResultPtr)
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    (void)instanceId;
    (void)updateResultPtr;

    printf("update.c to be implemented\n");
    return LWM2MCORE_ERR_NOT_YET_IMPLEMENTED;
}

//--------------------------------------------------------------------------------------------------
/**
 * The server requires the package name
 *
 * @return
 *  - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
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
    if ((NULL == bufferPtr) || (LWM2MCORE_MAX_UPDATE_TYPE <= type))
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    (void)type;
    (void)instanceId;
    (void)bufferPtr;
    (void)len;

    printf("update.c to be implemented\n");
    return LWM2MCORE_ERR_NOT_YET_IMPLEMENTED;
}

//--------------------------------------------------------------------------------------------------
/**
 * The server requires the package version
 *
 * @return
 *  - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
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
    if ((NULL == bufferPtr) || (LWM2MCORE_MAX_UPDATE_TYPE <= type))
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    (void)type;
    (void)instanceId;
    (void)bufferPtr;
    (void)len;

    printf("update.c to be implemented\n");
    return LWM2MCORE_ERR_NOT_YET_IMPLEMENTED;
}

//--------------------------------------------------------------------------------------------------
/**
 * Clean the stale workspace of aborted SOTA/FOTA job
 */
//--------------------------------------------------------------------------------------------------
void lwm2mcore_CleanStaleData
(
    lwm2mcore_UpdateType_t type     ///< [IN] Update type
)
{
    (void)type;
    printf("function to be implemented");
}

//--------------------------------------------------------------------------------------------------
/**
 * The server sets the "update supported objects" field for software update
 *
 * @return
 *  - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_SetSwUpdateSupportedObjects
(
    uint16_t instanceId,            ///< [IN] Instance Id (any value for SW)
    bool value                      ///< [IN] Update supported objects field value
)
{
    (void)instanceId;
    (void)value;

    printf("update.c to be implemented\n");
    return LWM2MCORE_ERR_NOT_YET_IMPLEMENTED;
}

//--------------------------------------------------------------------------------------------------
/**
 * The server requires the "update supported objects" field for software update
 *
 * @return
 *  - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
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

    (void)instanceId;
    (void)valuePtr;

    printf("update.c to be implemented\n");
    return LWM2MCORE_ERR_NOT_YET_IMPLEMENTED;
}


//--------------------------------------------------------------------------------------------------
/**
 * The server requires the activation state for one embedded application
 *
 * @return
 *  - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
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

    (void)instanceId;
    (void)valuePtr;

    printf("update.c to be implemented\n");
    return LWM2MCORE_ERR_NOT_YET_IMPLEMENTED;
}

//--------------------------------------------------------------------------------------------------
/**
 * The server requires an embedded application to be uninstalled (only for software update)
 *
 * @return
 *  - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_LaunchSwUpdateUninstall
(
    uint16_t instanceId,            ///< [IN] Instance Id (any value for SW)
    char* bufferPtr,                ///< [INOUT] data buffer
    size_t len                      ///< [IN] length of input buffer
)
{
    if (NULL == bufferPtr)
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    (void)instanceId;
    (void)bufferPtr;
    (void)len;

    printf("update.c to be implemented\n");
    return LWM2MCORE_ERR_NOT_YET_IMPLEMENTED;
}

//--------------------------------------------------------------------------------------------------
/**
 * The server requires an embedded application to be activated or deactivated (only for software
 * update)
 *
 * @return
 *  - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handlerler
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
    if (NULL == bufferPtr)
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }
    (void)activation;
    (void)instanceId;
    (void)bufferPtr;
    (void)len;

    printf("update.c to be implemented\n");
    return LWM2MCORE_ERR_NOT_YET_IMPLEMENTED;
}

//--------------------------------------------------------------------------------------------------
/**
 * The server request to create or delete an object instance of object 9
 *
 * @return
 *  - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_SoftwareUpdateInstance
(
    bool create,                ///<[IN] Create (true) or delete (false)
    uint16_t instanceId         ///<[IN] Object instance Id to create or delete
)
{
    (void)create;
    (void)instanceId;
    printf("update.c to be implemented\n");
    return LWM2MCORE_ERR_NOT_YET_IMPLEMENTED;
}

#ifndef LWM2M_EXTERNAL_DOWNLOADER
//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to get the package offset on client side
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @note
 * This function is not available if @c LWM2M_EXTERNAL_DOWNLOADER compilation flag is embedded
 *
 * @note
 * When a package started to be downloaded, the client stores the downloaded data in memory.
 * When the download is suspended, LwM2MCore needs to know the package offset which is stored in
 * client side in order to resume the download to the correct offset.
 *
 * @return
 *  - LWM2MCORE_ERR_COMPLETED_OK on success
 *  - LWM2MCORE_ERR_INVALID_STATE if no package download is on-going
 *  - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *  - LWM2MCORE_ERR_GENERAL_ERROR on failure
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetPackageOffsetStorage
(
    lwm2mcore_UpdateType_t  updateType,     ///< [IN] Update type
    uint64_t*               offsetPtr       ///< [IN] Package offset
)
{
    struct stat sb;

    if (!offsetPtr)
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    (void)updateType;

    if (-1 == stat("download.bin", &sb))
    {
        return LWM2MCORE_ERR_GENERAL_ERROR;
    }

    *offsetPtr = (uint64_t)sb.st_size;
    return LWM2MCORE_ERR_COMPLETED_OK;
}
#endif

//--------------------------------------------------------------------------------------------------
/**
 * Start the download
 */
//--------------------------------------------------------------------------------------------------
void ClientStartDownload
(
    lwm2mcore_UpdateType_t  type,           ///< [IN] Update type
    uint64_t                packageSize,    ///< [IN] Package size
    bool                    isResume        ///< [IN] Indicates if it's a download resume
)
{
    downloadThreadData.updateType = type;
    downloadThreadData.packageSize = packageSize;
    downloadThreadData.isResume = isResume;

    printf("Start Download type %d, isResume %d\n", type, isResume);

    unlink("download.bin");

    if(pthread_create(&DownloadThread, NULL, StartDownload, &downloadThreadData) == -1)
    {
        perror("pthread_create");
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * @brief Resume a package download
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @note
 * This function is not available if @c LWM2M_EXTERNAL_DOWNLOADER compilation flag is embedded
 *
 */
//--------------------------------------------------------------------------------------------------
void lwm2mcore_ResumePackageDownloader
(
    lwm2mcore_UpdateType_t updateType   ///< [IN] Update type (FW/SW)
)
{
    PackageDownloaderWorkspace_t workspace;

    if (DWL_OK != ReadPkgDwlWorkspace(&workspace))
    {
        fprintf(stderr, "Error to read workspace\n");
        return;
    }

    downloadThreadData.updateType = updateType;
    downloadThreadData.packageSize = workspace.packageSize;
    downloadThreadData.isResume = true;

    printf("Resume Download type %d", updateType);

    if(pthread_create(&DownloadThread, NULL, StartDownload, &downloadThreadData) == -1)
    {
        perror("pthread_create");
    }
}
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
)
{
    if (NULL == statePtr)
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

   return LWM2MCORE_ERR_COMPLETED_OK;
}
