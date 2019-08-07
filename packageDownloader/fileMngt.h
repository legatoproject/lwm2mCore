/**
 * @file fileMngt.h
 *
 * LwM2Mcore file transfer
 *
 * <HR>
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */


#ifndef  __LWM2MCORE_FILE_MNGT_H__
#define  __LWM2MCORE_FILE_MNGT_H__


/**
  * @addtogroup lwm2mcore_fileTransfer_int
  * @{
  */

//--------------------------------------------------------------------------------------------------
/**
 * Define file transfer failure cause: Download timer can not be launched
 */
//--------------------------------------------------------------------------------------------------
#define FILE_MNGT_ERROR_DOWNLOAD_TIMER_ISSUE "Download timer can not be launched"

//--------------------------------------------------------------------------------------------------
/**
 * Define file transfer failure cause: Error before file URI treatment
 */
//--------------------------------------------------------------------------------------------------
#define FILE_MNGT_ERROR_BEFORE_FILE_URI_TREATMENT "Error before file URI treatment"

//--------------------------------------------------------------------------------------------------
/**
 * Define file transfer failure cause: Invalid URI
 */
//--------------------------------------------------------------------------------------------------
#define FILE_MNGT_ERROR_DOWNLOAD_INVALID_URI "Invalid URI"

//--------------------------------------------------------------------------------------------------
/**
 * Define file transfer failure cause: Invalid parameter
 */
//--------------------------------------------------------------------------------------------------
#define FILE_MNGT_ERROR_DOWNLOAD_INVALID_PARAMETER "Invalid parameter"

//--------------------------------------------------------------------------------------------------
/**
 * Define file transfer failure cause: Buffer overflow
 */
//--------------------------------------------------------------------------------------------------
#define FILE_MNGT_ERROR_DOWNLOAD_BUFFER_OVERFLOW "Buffer overflow"

//--------------------------------------------------------------------------------------------------
/**
 * Define file transfer failure cause: Not enough memory
 */
//--------------------------------------------------------------------------------------------------
#define FILE_MNGT_ERROR_DOWNLOAD_NOT_ENOUGH_MEMORY "Not enough memory"

//--------------------------------------------------------------------------------------------------
/**
 * Define file transfer failure cause: Out of memory
 */
//--------------------------------------------------------------------------------------------------
#define FILE_MNGT_ERROR_DOWNLOAD_OUT_OF_MEMORY "Out of memory"

//--------------------------------------------------------------------------------------------------
/**
 * Define file transfer failure cause: Invalid file
 */
//--------------------------------------------------------------------------------------------------
#define FILE_MNGT_ERROR_DOWNLOAD_INVALID_FILE "Invalid file"

//--------------------------------------------------------------------------------------------------
/**
 * Define file transfer failure cause: CRC check failure
 */
//--------------------------------------------------------------------------------------------------
#define FILE_MNGT_ERROR_DOWNLOAD_CRC_CHECK_FAILURE "CRC check failure"

//--------------------------------------------------------------------------------------------------
/**
 * Define file transfer failure cause: CRC init error
 */
//--------------------------------------------------------------------------------------------------
#define FILE_MNGT_ERROR_DOWNLOAD_CRC_INIT "CRC init error"

//--------------------------------------------------------------------------------------------------
/**
 * Define file transfer failure cause: CRC process error
 */
//--------------------------------------------------------------------------------------------------
#define FILE_MNGT_ERROR_DOWNLOAD_CRC_PROCESS "CRC process error"

//--------------------------------------------------------------------------------------------------
/**
 * Define file transfer failure cause: CRC restore error
 */
//--------------------------------------------------------------------------------------------------
#define FILE_MNGT_ERROR_DOWNLOAD_CRC_RESTORE "CRC restore error"

//--------------------------------------------------------------------------------------------------
/**
 * Define file transfer failure cause: download abort
 */
//--------------------------------------------------------------------------------------------------
#define FILE_MNGT_ERROR_DOWNLOAD_ABORTED "Aborted transfer"

//--------------------------------------------------------------------------------------------------
/**
 * Define max number of stored files is reached
 */
//--------------------------------------------------------------------------------------------------
#define FILE_MNGT_ERROR_MAX_STORED_FILES "Maximum number of stored files was reached"

//--------------------------------------------------------------------------------------------------
/**
 * Define file transfer failure cause: Miscellenaous error
 */
//--------------------------------------------------------------------------------------------------
#define FILE_MNGT_ERROR_DOWNLOAD_MISC "Miscellaneous error"

//--------------------------------------------------------------------------------------------------
/**
 * @brief Supported version for file transfer workspace
 */
//--------------------------------------------------------------------------------------------------
#define FILE_TRANSFER_WORKSPACE_VERSION    1

//--------------------------------------------------------------------------------------------------
/**
 * @brief File transfer workspace structure
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    uint8_t                             version;            ///< Workspace version
    lwm2mcore_FileTransferState_t       transferState;      ///< File transfer state
    lwm2mcore_FileTransferResult_t      transferResult;     ///< File transfer result
    lwm2mcore_FileTransferDirection_t   transferDirection;  ///< File transfer direction
    char transferFailureReason[LWM2MCORE_FILE_TRANSFER_FAILURE_CAUSE_MAX_CHAR+1]; ///< Failure reason
}
FileTransferWorkspace_t;

//--------------------------------------------------------------------------------------------------
/**
 * Function to perform an immediate file information storage.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t fileTransfer_TreatInfo
(
    lwm2mcore_FileTransferRequest_t fileTransferInfo        ///< [IN] File transfer info
);

//--------------------------------------------------------------------------------------------------
/**
 * Function to check if a file transfer is possible
 */
//--------------------------------------------------------------------------------------------------
void fileTransfer_CheckFileTransferPossible
(
    void
);

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
);

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
);

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
);

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
);

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
);

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
);

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
    lwm2mcore_FileTransferResult_t* resultPtr       ///< [OUT] File transfer result
);

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
);

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
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Set the file transfer result
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
);

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
);

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
 *  - @ref LWM2MCORE_ERR_OVERFLOW on overflow
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR other failure
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t fileTransfer_GetFailureReason
(
    char*   bufferPtr,          ///< [OUT] Buffer
    size_t* bufferLengthPtr     ///< [OUT] Buffer length
);

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
);

/**
  * @}
  */

#endif /*  __LWM2MCORE_FILE_MNGT_H__ */
