/**
 * @file fileTransfer.c
 *
 * LwM2Mcore file transfer manager
 *
 * <HR>
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#ifdef LWM2M_OBJECT_33406

#include <stdio.h>
#include <internals.h>
#include <lwm2mcore/lwm2mcore.h>
#include "handlers.h"
#include <lwm2mcore/timer.h>
#include <lwm2mcore/fileTransfer.h>
#include "workspace.h"
#include "fileMngt.h"
#include "objects.h"
#include <lwm2mcore/lwm2mcorePackageDownloader.h>

//--------------------------------------------------------------------------------------------------
// Symbol and Enum definitions
//--------------------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------------------
// Static variables
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
// Static variables
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Default values of file transfer workspace
 */
//--------------------------------------------------------------------------------------------------
static const FileTransferWorkspace_t FileTransferWorkspace =
{
    .version =                  FILE_TRANSFER_WORKSPACE_VERSION ,
    .transferState =            LWM2MCORE_FILE_TRANSFER_STATE_IDLE,
    .transferResult =           LWM2MCORE_FILE_TRANSFER_RESULT_INITIAL,
    .transferDirection =        LWM2MCORE_FILE_TRANSFER_DIRECTION_DOWNLOAD,
    .transferFailureReason =    {0}
};


//--------------------------------------------------------------------------------------------------
// Internal functions
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Function to read the file transfer workspace from platform memory
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters is incorrect
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *  - @ref LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t ReadFileTransferWorkspace
(
    FileTransferWorkspace_t* fileTransferWorkspacePtr    ///< File transfer workspace
)
{
    lwm2mcore_Sid_t sid;
    size_t len = sizeof(FileTransferWorkspace_t);

    if (!fileTransferWorkspacePtr)
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    // Check if the file transfer workspace is stored
    sid = lwm2mcore_GetParam(LWM2MCORE_FILE_TRANSFER_WORKSPACE_PARAM,
                             (uint8_t*)fileTransferWorkspacePtr,
                             &len);
    //LOG_ARG("Read file transfer workspace: len = %zu, result = %d", len, sid);

    if (   (LWM2MCORE_ERR_COMPLETED_OK == sid)
        && (sizeof(FileTransferWorkspace_t) == len)
       )
    {
        //LOG_ARG("File transfer workspace version %d (only %d supported)",
                //fileTransferWorkspacePtr->version, FILE_TRANSFER_WORKSPACE_VERSION);

        // Check if the version is the supported one
        if (FILE_TRANSFER_WORKSPACE_VERSION == fileTransferWorkspacePtr->version)
        {
            return LWM2MCORE_ERR_COMPLETED_OK;
        }
    }

    LOG("Failed to read the download workspace");

    if (len)
    {
        // The workspace is present but the size is not correct, delete it
        LOG("Delete file transfer workspace");
        sid = lwm2mcore_DeleteParam(LWM2MCORE_FILE_TRANSFER_WORKSPACE_PARAM);
    }

    // Copy the default configuration
    memcpy(fileTransferWorkspacePtr, &FileTransferWorkspace, sizeof(FileTransferWorkspace_t));

    if(LWM2MCORE_ERR_COMPLETED_OK != WriteFileTransferWorkspace(fileTransferWorkspacePtr))
    {
        return LWM2MCORE_ERR_GENERAL_ERROR;
    }

    return LWM2MCORE_ERR_COMPLETED_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to write the file transfer workspace in platform memory
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters is incorrect
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *  - @ref LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t WriteFileTransferWorkspace
(
    FileTransferWorkspace_t* fileTransferWorkspacePtr    ///< File transfer workspace
)
{
    lwm2mcore_Sid_t sID;

    if (!fileTransferWorkspacePtr)
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    sID = lwm2mcore_SetParam(LWM2MCORE_FILE_TRANSFER_WORKSPACE_PARAM,
                             (uint8_t*)fileTransferWorkspacePtr,
                             sizeof(FileTransferWorkspace_t));
    if (LWM2MCORE_ERR_COMPLETED_OK != sID)
    {
        LOG_ARG("Save download workspace failed: %d", sID);
    }

    return sID;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to delete the package downloader workspace in platform memory
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters is incorrect
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *  - @ref LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t DeleteFileTransferWorkspace
(
    void
)
{
    lwm2mcore_Sid_t sID = lwm2mcore_DeleteParam(LWM2MCORE_FILE_TRANSFER_WORKSPACE_PARAM);
    if (LWM2MCORE_ERR_COMPLETED_OK != sID)
    {
        LOG_ARG("Delete file transfer workspace: error %d", sID);
    }
    return sID;
}

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function setting the failure when file transfer operation fails on request
 *
 * @note
 * This function is available only if @c LWM2M_OBJECT_33406 flag is embedded
 */
//--------------------------------------------------------------------------------------------------
void fileTransfer_PreOperationFailure
(
    lwm2mcore_Sid_t sID                                 ///< [IN] Error on resource check
)
{
    switch(sID)
    {
        case LWM2MCORE_ERR_COMPLETED_OK:
        break;

        case LWM2MCORE_ERR_ALREADY_PROCESSED:
            fileTransfer_SetResult(LWM2MCORE_FILE_TRANSFER_RESULT_ALREADY_EXISTS);
        break;

        case LWM2MCORE_ERR_INVALID_ARG:
        case LWM2MCORE_ERR_INCORRECT_RANGE:
            fileTransfer_SetResult(LWM2MCORE_FILE_TRANSFER_RESULT_FAILURE);
            fileTransfer_SetFailureReason(FILE_MNGT_ERROR_DOWNLOAD_INVALID_PARAMETER,
                                          strlen(FILE_MNGT_ERROR_DOWNLOAD_INVALID_PARAMETER));
        break;

        case LWM2MCORE_ERR_OVERFLOW:
            fileTransfer_SetResult(LWM2MCORE_FILE_TRANSFER_RESULT_FAILURE);
            fileTransfer_SetFailureReason(FILE_MNGT_ERROR_DOWNLOAD_BUFFER_OVERFLOW,
                                          strlen(FILE_MNGT_ERROR_DOWNLOAD_BUFFER_OVERFLOW));
        break;

        default:
            fileTransfer_SetResult(LWM2MCORE_FILE_TRANSFER_RESULT_FAILURE);
            fileTransfer_SetFailureReason(FILE_MNGT_ERROR_DOWNLOAD_MISC,
                                          strlen(FILE_MNGT_ERROR_DOWNLOAD_MISC));
        break;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to perform an immediate file information storage.
 *
 * @note
 * This function is available only if @c LWM2M_OBJECT_33406 flag is embedded
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if succeeds
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if parameter is invalid
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR other failure
 *  - @ref LWM2MCORE_ERR_ALREADY_PROCESSED if the file is already present
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t fileTransfer_TreatInfo
(
    lwm2mcore_FileTransferRequest_t fileTransferInfo        ///< [IN] File transfer info
)
{
    lwm2mcore_Sid_t sID = LWM2MCORE_ERR_COMPLETED_OK;
    bool couldDwnldBeLaunched = false;
    sID = lwm2mcore_FileTransferRequest(fileTransferInfo, &couldDwnldBeLaunched);
    LOG_ARG("lwm2mcore_FileTransferRequest result %d, couldDwnldBeLaunched %d",
            sID, couldDwnldBeLaunched);

    if ((LWM2MCORE_ERR_ALREADY_PROCESSED == sID) && (false == couldDwnldBeLaunched))
    {
        LOG("File is already in download phasis");
        return sID;
    }

    fileTransfer_SetState(LWM2MCORE_FILE_TRANSFER_STATE_IDLE);

    if (LWM2MCORE_ERR_COMPLETED_OK != sID)
    {
        fileTransfer_PreOperationFailure(sID);
        /* Check if the package download timer is running */
        if (lwm2mcore_TimerIsRunning(LWM2MCORE_TIMER_DOWNLOAD))
        {
            lwm2mcore_TimerStop(LWM2MCORE_TIMER_DOWNLOAD);
        }
    }
    else
    {
        // Treat the file download
        sID = omanager_SetUpdatePackageUri(LWM2MCORE_FILE_TRANSFER_TYPE,
                                           0,
                                           fileTransferInfo.fileUri,
                                           strlen(fileTransferInfo.fileUri));
        if (LWM2MCORE_ERR_COMPLETED_OK == sID)
        {
            fileTransfer_SetState(LWM2MCORE_FILE_TRANSFER_STATE_PROCESSING);
        }
        else
        {
            fileTransfer_SetFailureReason(FILE_MNGT_ERROR_BEFORE_FILE_URI_TREATMENT,
                                          strlen(FILE_MNGT_ERROR_BEFORE_FILE_URI_TREATMENT));
        }
    }
    return sID;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to check if a file transfer is possible
 */
//--------------------------------------------------------------------------------------------------
void fileTransfer_CheckFileTransferPossible
(
    void
)
{
    // Check if max number of files is already reached
    if (LWM2MCORE_FILE_TRANSFER_NUMBER_MAX == omanager_ObjectInstanceCount(LWM2MCORE_FILE_LIST_OID))
    {
        LOG("File transfer: maximum file number was already reached");
        lwm2mcore_DeletePackageDownloaderResumeInfo();
        fileTransfer_SetState(LWM2MCORE_FILE_TRANSFER_STATE_IDLE);
        fileTransfer_SetResult(LWM2MCORE_FILE_TRANSFER_RESULT_FAILURE);
        fileTransfer_SetFailureReason(FILE_MNGT_ERROR_MAX_STORED_FILES,
                                      strlen(FILE_MNGT_ERROR_MAX_STORED_FILES));
        /* Check if the package download timer is running */
        if (lwm2mcore_TimerIsRunning(LWM2MCORE_TIMER_DOWNLOAD))
        {
            lwm2mcore_TimerStop(LWM2MCORE_TIMER_DOWNLOAD);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * @brief Set the file transfer state
 *
 * @note
 * This function is available only if @c LWM2M_OBJECT_33406 flag is embedded
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if succeeds
 *  - @ref LWM2MCORE_ERR_INCORRECT_RANGE parameter out of range
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR other failure
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t fileTransfer_SetState
(
    lwm2mcore_FileTransferState_t state   ///< [IN] File transfer state
)
{
    FileTransferWorkspace_t fileTransferWorkspace;
    if(LWM2MCORE_FILE_TRANSFER_STATE_MAX <= state)
    {
        return LWM2MCORE_ERR_INCORRECT_RANGE;
    }

    if (LWM2MCORE_ERR_COMPLETED_OK != ReadFileTransferWorkspace(&fileTransferWorkspace))
    {
        return LWM2MCORE_ERR_GENERAL_ERROR;
    }

    LOG_ARG("Set file transfer state to %d", state);
    if (fileTransferWorkspace.transferState != state)
    {
        fileTransferWorkspace.transferState = state;
        return WriteFileTransferWorkspace(&fileTransferWorkspace);
    }

    return LWM2MCORE_ERR_COMPLETED_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * @brief Get the file transfer state
 *
 * @note
 * This function is available only if @c LWM2M_OBJECT_33406 flag is embedded
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if succeeds
 *  - @ref LWM2MCORE_ERR_INVALID_ARG on invalid parameter
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR other failure
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t fileTransfer_GetState
(
    lwm2mcore_FileTransferState_t* statePtr     ///< [OUT] File transfer state
)
{
    FileTransferWorkspace_t fileTransferWorkspace;

    if (!statePtr)
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    if (LWM2MCORE_ERR_COMPLETED_OK != ReadFileTransferWorkspace(&fileTransferWorkspace))
    {
        return LWM2MCORE_ERR_GENERAL_ERROR;
    }

    LOG_ARG("Get file transfer state: %d", fileTransferWorkspace.transferState);
    *statePtr = fileTransferWorkspace.transferState;

    return LWM2MCORE_ERR_COMPLETED_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * @brief Set the file transfer result
 *
 * @note
 * This function is available only if @c LWM2M_OBJECT_33406 flag is embedded
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if succeeds
 *  - @ref LWM2MCORE_ERR_INCORRECT_RANGE parameter out of range
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR other failure
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t fileTransfer_SetResult
(
    lwm2mcore_FileTransferResult_t result   ///< [IN] File transfer result
)
{
    FileTransferWorkspace_t fileTransferWorkspace;
    if(LWM2MCORE_FILE_TRANSFER_RESULT_MAX <= result)
    {
        return LWM2MCORE_ERR_INCORRECT_RANGE;
    }

    if (LWM2MCORE_ERR_COMPLETED_OK != ReadFileTransferWorkspace(&fileTransferWorkspace))
    {
        return LWM2MCORE_ERR_GENERAL_ERROR;
    }

    LOG_ARG("Set file transfer result to %d", result);
    if (fileTransferWorkspace.transferResult != result)
    {
        fileTransferWorkspace.transferResult = result;
        return WriteFileTransferWorkspace(&fileTransferWorkspace);
    }

    return LWM2MCORE_ERR_COMPLETED_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * @brief Get the file transfer result
 *
 * @note
 * This function is available only if @c LWM2M_OBJECT_33406 flag is embedded
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if succeeds
 *  - @ref LWM2MCORE_ERR_INVALID_ARG on invalid parameter
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR other failure
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t fileTransfer_GetResult
(
    lwm2mcore_FileTransferResult_t* resultPtr      ///< [OUT] File transfer result
)
{
    FileTransferWorkspace_t fileTransferWorkspace;

    if (!resultPtr)
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    if (LWM2MCORE_ERR_COMPLETED_OK != ReadFileTransferWorkspace(&fileTransferWorkspace))
    {
        return LWM2MCORE_ERR_GENERAL_ERROR;
    }

    LOG_ARG("Get file transfer result: %d", fileTransferWorkspace.transferResult);
    *resultPtr = fileTransferWorkspace.transferResult;

    return LWM2MCORE_ERR_COMPLETED_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * @brief Set the file transfer direction
 *
 * @note
 * This function is available only if @c LWM2M_OBJECT_33406 flag is embedded
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if succeeds
 *  - @ref LWM2MCORE_ERR_INCORRECT_RANGE parameter out of range
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR other failure
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t fileTransfer_SetDirection
(
    lwm2mcore_FileTransferDirection_t direction   ///< [IN] File transfer direction
)
{
    FileTransferWorkspace_t fileTransferWorkspace;
    if(LWM2MCORE_FILE_TRANSFER_DIRECTION_MAX <= direction)
    {
        return LWM2MCORE_ERR_INCORRECT_RANGE;
    }

    if (LWM2MCORE_ERR_COMPLETED_OK != ReadFileTransferWorkspace(&fileTransferWorkspace))
    {
        return LWM2MCORE_ERR_GENERAL_ERROR;
    }

    LOG_ARG("Set file transfer direction to %d", direction);
    if (fileTransferWorkspace.transferDirection != direction)
    {
        fileTransferWorkspace.transferDirection = direction;
        return WriteFileTransferWorkspace(&fileTransferWorkspace);
    }

    return LWM2MCORE_ERR_COMPLETED_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * @brief Get the file transfer direction
 *
 * @note
 * This function is available only if @c LWM2M_OBJECT_33406 flag is embedded
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if succeeds
 *  - @ref LWM2MCORE_ERR_INVALID_ARG on invalid parameter
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR other failure
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t fileTransfer_GetDirection
(
    lwm2mcore_FileTransferDirection_t* directionPtr     ///< [OUT] File transfer direction
)
{
    FileTransferWorkspace_t fileTransferWorkspace;

    if (!directionPtr)
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    if (LWM2MCORE_ERR_COMPLETED_OK != ReadFileTransferWorkspace(&fileTransferWorkspace))
    {
        return LWM2MCORE_ERR_GENERAL_ERROR;
    }

    LOG_ARG("Get file transfer direction: %d", fileTransferWorkspace.transferDirection);
    *directionPtr = fileTransferWorkspace.transferDirection;

    return LWM2MCORE_ERR_COMPLETED_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * @brief Get the file transfer progress
 *
 * @note
 * This function is available only if @c LWM2M_OBJECT_33406 flag is embedded
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if succeeds
 *  - @ref LWM2MCORE_ERR_INVALID_ARG on invalid parameter
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR other failure
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t fileTransfer_GetProgress
(
    uint8_t*    progressPtr           ///< [OUT] Transfer progress
)
{
    PackageDownloaderWorkspace_t workspace;

    if (!progressPtr)
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    // Update the package type.
    // This is the first step as error handling is dependent on update type.
    if (DWL_OK != ReadPkgDwlWorkspace(&workspace))
    {
        return LWM2MCORE_ERR_GENERAL_ERROR;
    }

    LOG_ARG("workspace.packageSize %"PRIu64" workspace.remainingBinaryData %"PRIu64,
    workspace.packageSize, workspace.remainingBinaryData);

    if (workspace.packageSize)
    {
        *progressPtr = (uint8_t)(((workspace.packageSize - workspace.remainingBinaryData)*100)/workspace.packageSize);
    }
    else
    {
        *progressPtr = 0;
    }
    LOG_ARG("progress %d", *progressPtr);
    return LWM2MCORE_ERR_COMPLETED_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * @brief Set the file transfer failure reason
 *
 * @note
 * This function is available only if @c LWM2M_OBJECT_33406 flag is embedded
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if succeeds
 *  - @ref LWM2MCORE_ERR_INVALID_ARG on invalid parameter
 *  - @ref LWM2MCORE_ERR_INCORRECT_RANGE parameter out of range
 *  - @ref LWM2MCORE_ERR_OVERFLOW on overflow
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR other failure
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t fileTransfer_SetFailureReason
(
    const char* bufferPtr,          ///< [IN] Buffer
    size_t      bufferLength        ///< [IN] Buffer length
)
{
    FileTransferWorkspace_t fileTransferWorkspace;
    size_t len;
    if (!bufferPtr)
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    if ((LWM2MCORE_FILE_TRANSFER_FAILURE_CAUSE_MAX_CHAR < bufferLength)
     || (LWM2MCORE_FILE_TRANSFER_FAILURE_CAUSE_MAX_CHAR < strlen(bufferPtr)))
    {
        return LWM2MCORE_ERR_OVERFLOW;
    }

    if (LWM2MCORE_ERR_COMPLETED_OK != ReadFileTransferWorkspace(&fileTransferWorkspace))
    {
        return LWM2MCORE_ERR_GENERAL_ERROR;
    }

    len = strlen(fileTransferWorkspace.transferFailureReason);
    //LOG_ARG("strlen of stored failure reason string %zd", len);

    if (bufferLength)
    {
        LOG_ARG("Set file transfer failure reason to %s", bufferPtr);
    }

    if (len < bufferLength)
    {
        len = bufferLength;
    }

    if (strncmp(fileTransferWorkspace.transferFailureReason, (char*)bufferPtr, len))
    {
        snprintf(fileTransferWorkspace.transferFailureReason,
                 LWM2MCORE_FILE_TRANSFER_FAILURE_CAUSE_MAX_CHAR+1,
                 "%s",
                 bufferPtr);
        return WriteFileTransferWorkspace(&fileTransferWorkspace);
    }

    return LWM2MCORE_ERR_COMPLETED_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * @brief Get the file transfer failure reason
 *
 * @note
 * This function is available only if @c LWM2M_OBJECT_33406 flag is embedded
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if succeeds
 *  - @ref LWM2MCORE_ERR_INVALID_ARG on invalid parameter
 *  - @ref LWM2MCORE_ERR_OVERFLOW on overflow
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR other failure
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t fileTransfer_GetFailureReason
(
    char*      bufferPtr,          ///< [OUT] Buffer
    size_t*    bufferLengthPtr     ///< [OUT] Buffer length
)
{
    FileTransferWorkspace_t fileTransferWorkspace;
    size_t len;

    if ((!bufferPtr) || (!bufferLengthPtr))
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    if (LWM2MCORE_ERR_COMPLETED_OK != ReadFileTransferWorkspace(&fileTransferWorkspace))
    {
        return LWM2MCORE_ERR_GENERAL_ERROR;
    }

    LOG_ARG("Get file transfer failure reason: %s", fileTransferWorkspace.transferFailureReason);
    len = strlen(fileTransferWorkspace.transferFailureReason);
    if (len > *bufferLengthPtr)
    {
        return LWM2MCORE_ERR_OVERFLOW;
    }
    snprintf(bufferPtr,
             *bufferLengthPtr,
             "%s",
             fileTransferWorkspace.transferFailureReason);
    *bufferLengthPtr = len;
    return LWM2MCORE_ERR_COMPLETED_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * @brief Set the file transfer state
 *
 * @remark Public function which can be called by the client.
 *
 * @note
 * This function is available only if @c LWM2M_OBJECT_33406 flag is embedded
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if succeeds
 *  - @ref LWM2MCORE_ERR_INCORRECT_RANGE parameter out of range
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR other failure
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_SetFileTransferState
(
    lwm2mcore_FileTransferState_t state   ///< [IN] File transfer state
)
{
    return fileTransfer_SetState(state);
}

//--------------------------------------------------------------------------------------------------
/**
 * @brief Set the file transfer result
 *
 * @remark Public function which can be called by the client.
 *
 * @note
 * This function is available only if @c LWM2M_OBJECT_33406 flag is embedded
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if succeeds
 *  - @ref LWM2MCORE_ERR_INCORRECT_RANGE parameter out of range
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR other failure
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_SetFileTransferResult
(
    lwm2mcore_FileTransferResult_t result      ///< [IN] File transfer result
)
{
    return fileTransfer_SetResult(result);
}

//--------------------------------------------------------------------------------------------------
/**
 * @brief Set the file transfer failure reason
 *
 * @remark Public function which can be called by the client.
 *
 * @note
 * This function is available only if @c LWM2M_OBJECT_33406 flag is embedded
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if succeeds
 *  - @ref LWM2MCORE_ERR_INCORRECT_RANGE parameter out of range
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR other failure
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_SetFileTransferFailureCause
(
    const char* bufferPtr,     ///< [IN] Buffer
    size_t      bufferLen      ///< [IN] Buffer length
)
{
    return fileTransfer_SetFailureReason(bufferPtr, bufferLen);
}

#endif /* LWM2M_OBJECT_33406*/
