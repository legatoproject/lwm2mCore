/**
 * @file fileTransfer.h
 *
 * Header file for LWM2M Core file transfer management object
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#ifndef  __LWM2MCORE_FILE_TRANSFER_H__
#define  __LWM2MCORE_FILE_TRANSFER_H__

#include <lwm2mcore/lwm2mcore.h>
#include <lwm2mcore/update.h>

/**
  * @addtogroup lwm2mcore_fileTransfer_IFS
  * @{
  */

//--------------------------------------------------------------------------------------------------
/**
 * Define value for maximum supported files
 */
//--------------------------------------------------------------------------------------------------
#define LWM2MCORE_FILE_TRANSFER_NUMBER_MAX              50

//--------------------------------------------------------------------------------------------------
/**
 * Define value for file name max length (\0 excluded)
 */
//--------------------------------------------------------------------------------------------------
#define LWM2MCORE_FILE_TRANSFER_NAME_MAX_CHAR           128

//--------------------------------------------------------------------------------------------------
/**
 * Define value for file class max length (\0 excluded)
 */
//--------------------------------------------------------------------------------------------------
#define LWM2MCORE_FILE_TRANSFER_CLASS_MAX_CHAR          255

//--------------------------------------------------------------------------------------------------
/**
 * Define value for file URI (\0 excluded)
 */
//--------------------------------------------------------------------------------------------------
#define LWM2MCORE_FILE_TRANSFER_URI_MAX_CHAR            LWM2MCORE_PACKAGE_URI_MAX_LEN

//--------------------------------------------------------------------------------------------------
/**
 * Define value for failure cause (\0 excluded)
 */
//--------------------------------------------------------------------------------------------------
#define LWM2MCORE_FILE_TRANSFER_FAILURE_CAUSE_MAX_CHAR  255

//--------------------------------------------------------------------------------------------------
/**
 * Define value for failure cause (\0 excluded)
 */
//--------------------------------------------------------------------------------------------------
#define LWM2MCORE_FILE_TRANSFER_HASH_MAX_CHAR           64

//--------------------------------------------------------------------------------------------------
/**
 * @brief Define the maximum length for one object instance definition for the registration
 * (update) message: </lwm2m/33406/xxxxx>, where xxxxx is in the [0-65535] range
 */
//--------------------------------------------------------------------------------------------------
#define LWM2MCORE_FILE_TRANSFER_OBJECT_INSTANCE_LEN    21

//--------------------------------------------------------------------------------------------------
/**
 * @brief Define the maximum length for the file transfer objects
 */
//--------------------------------------------------------------------------------------------------
#define LWM2MCORE_FILE_TRANSFER_OBJECT_INSTANCE_LIST_MAX_LEN   LWM2MCORE_FILE_TRANSFER_OBJECT_INSTANCE_LEN * LWM2MCORE_FILE_TRANSFER_NUMBER_MAX

//--------------------------------------------------------------------------------------------------
/**
 * @brief Enumeration for file transfer state (object 33406, resource "state")
 *
 *  @note These values are defined in the LWM2M specification
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    LWM2MCORE_FILE_TRANSFER_STATE_IDLE             = 0,   ///< No download
    LWM2MCORE_FILE_TRANSFER_STATE_PROCESSING       = 1,   ///< File transfer is processing
    LWM2MCORE_FILE_TRANSFER_STATE_TRANSFERRING     = 2,   ///< File transfer is on-going
    LWM2MCORE_FILE_TRANSFER_STATE_SUSPENDED        = 3,   ///< File transfer was suspended
    LWM2MCORE_FILE_TRANSFER_STATE_MAX              = 4    ///< Internal usage
}lwm2mcore_FileTransferState_t;

//--------------------------------------------------------------------------------------------------
/**
 * @brief Enumeration for file transfer state (object 33406, resource "result")
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    LWM2MCORE_FILE_TRANSFER_RESULT_INITIAL          = 0,   ///< Initial value
    LWM2MCORE_FILE_TRANSFER_RESULT_ALREADY_EXISTS   = 1,   ///< File already exists
    LWM2MCORE_FILE_TRANSFER_RESULT_SUCCESS          = 2,   ///< File transfer success
    LWM2MCORE_FILE_TRANSFER_RESULT_FAILURE          = 3,   ///< File transfer failure
    LWM2MCORE_FILE_TRANSFER_RESULT_MAX              = 4    ///< Internal usage
}lwm2mcore_FileTransferResult_t;

//--------------------------------------------------------------------------------------------------
/**
 * @brief Enumeration for file transfer direction (object 33406, resource "direction")
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    LWM2MCORE_FILE_TRANSFER_DIRECTION_DOWNLOAD  = 0,  ///< File download
    LWM2MCORE_FILE_TRANSFER_DIRECTION_MAX             ///< Internal usage
}lwm2mcore_FileTransferDirection_t;


//--------------------------------------------------------------------------------------------------
/**
 * @brief Enumeration for file transfer origin (object 33406, resource "origin")
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    LWM2MCORE_FILE_LIST_ORIGIN_SERVER  = 0,     ///< File from the server
    LWM2MCORE_FILE_LIST_ORIGIN_MAX              ///< Internal usage
}lwm2mcore_FileListOrigin_t;

//--------------------------------------------------------------------------------------------------
/**
 * @brief Structure for a file transfer
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    char        fileName[LWM2MCORE_FILE_TRANSFER_NAME_MAX_CHAR+1];      ///< File name
    char        fileClass[LWM2MCORE_FILE_TRANSFER_CLASS_MAX_CHAR+1];    ///< File class
    char        fileUri[LWM2MCORE_FILE_TRANSFER_URI_MAX_CHAR+1];        ///< File URI
    char        fileHash[LWM2MCORE_FILE_TRANSFER_HASH_MAX_CHAR+1];      ///< File hash
    lwm2mcore_FileTransferDirection_t direction;                        ///< File direction
}lwm2mcore_FileTransferRequest_t;



/**
  * @}
  */

/**
  * @addtogroup lwm2mcore_platform_adaptor_fileTransfer_IFS
  * @{
  */

//--------------------------------------------------------------------------------------------------
/**
 *                              Platform adaptor functions
 */
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * @brief File transfer request
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @note
 * This function is available only if @c LWM2M_OBJECT_33406 flag is embedded
 *
 * @note
 * For CoAP retry reason, this function treatment needs to be synchronous
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if succeeds
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if parameter is invalid
 *  - @ref LWM2MCORE_ERR_OVERFLOW if the buffer is too long
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR other failure
 *  - @ref LWM2MCORE_ERR_ALREADY_PROCESSED if the file is already present
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_FileTransferRequest
(
    lwm2mcore_FileTransferRequest_t fileTransferInfo,       ///< [IN] File transfer info
    bool*                           couldDwnldBeLaunchedPtr ///< [OUT] Could download be launched?
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Get the file checksum for the file transfer
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @note
 * This function is available only if @c LWM2M_OBJECT_33406 flag is embedded
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if succeeds
 *  - @ref LWM2MCORE_ERR_OVERFLOW on buffer overflow
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if parameter is invalid
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR other failure
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetFileTransferChecksum
(
    char*       bufferPtr,          ///< [OUT] Buffer
    size_t*     bufferSizePtr       ///< [INOUT] Buffer size
);

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
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Get the file name from its intance Id
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @note
 * This function is available only if @c LWM2M_OBJECT_33406 flag is embedded
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if succeeds
 *  - @ref LWM2MCORE_ERR_OVERFLOW on buffer overflow
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if parameter is invalid
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR other failure
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetFileNameByInstance
(
    uint16_t    instanceId,         ///< [IN] Instance Id of object 33407
    char*       bufferPtr,          ///< [OUT] Buffer
    size_t*     bufferSizePtr       ///< [INOUT] Buffer size
);


//--------------------------------------------------------------------------------------------------
/**
 * @brief Get the file class from its intance Id
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @note
 * This function is available only if @c LWM2M_OBJECT_33406 flag is embedded
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if succeeds
 *  - @ref LWM2MCORE_ERR_OVERFLOW on buffer overflow
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if parameter is invalid
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR other failure
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetFileClassByInstance
(
    uint16_t    instanceId,         ///< [IN] Instance Id of object 33407
    char*       bufferPtr,          ///< [OUT] Buffer
    size_t*     bufferSizePtr       ///< [INOUT] Buffer size
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Get the file hashcode from its intance Id
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @note
 * This function is available only if @c LWM2M_OBJECT_33406 flag is embedded
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if succeeds
 *  - @ref LWM2MCORE_ERR_OVERFLOW on buffer overflow
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if parameter is invalid
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR other failure
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetFileChecksumByInstance
(
    uint16_t    instanceId,         ///< [IN] Instance Id of object 33407
    char*       bufferPtr,          ///< [OUT] Buffer
    size_t*     bufferSizePtr       ///< [INOUT] Buffer size
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Get the file origin from its intance Id
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @note
 * This function is available only if @c LWM2M_OBJECT_33406 flag is embedded
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if succeeds
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if parameter is invalid
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR other failure
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetFileOriginByInstance
(
    uint16_t                        instanceId,     ///< [IN] Instance Id of object 33407
    lwm2mcore_FileListOrigin_t*     originPtr       ///< [OUT] File origin
);


//--------------------------------------------------------------------------------------------------
/**
 * @brief Delete a file by its instance Id
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @note
 * This function is available only if @c LWM2M_OBJECT_33406 flag is embedded
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if succeeds
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if parameter is invalid
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR other failure
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_DeleteFileByInstance
(
    uint16_t instanceId                                 ///< [IN] Instance Id of object 33407
);

/**
  * @}
  */

/**
  * @addtogroup lwm2mcore_fileTransfer_IFS
  * @{
  */

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to notify LwM2MCore of supported object instance list for file transfer
 *
 * @remark Public function which can be called by the client.
 *
 * @return
 *  - @c true if the list was successfully treated
 *  - else @c false
 */
//--------------------------------------------------------------------------------------------------
bool lwm2mcore_UpdateFileTransferList
(
    lwm2mcore_Ref_t instanceRef,    ///< [IN] Instance reference (Set to NULL if this API is used if
                                    ///< lwm2mcore_init API was no called)
    const char* listPtr,            ///< [IN] Formatted list
    size_t listLen                  ///< [IN] Size of the update list
);

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
);

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
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Get available space for file storage
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @note
 * This function is available only if @c LWM2M_OBJECT_33406 flag is embedded
 *
 * @note
 * For CoAP retry reason, this function treatment needs to be synchronous
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if succeeds
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if parameter is invalid
 *  - @ref LWM2MCORE_ERR_OVERFLOW if the buffer is too long
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR other failure
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_FileTransferAvailableSpace
(
    uint64_t*   availableSpacePtr           ///< [OUT] Available space
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief The file transfer is aborted
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @note
 * This function is available only if @c LWM2M_OBJECT_33406 flag is embedded
 *
 * @note
 * For CoAP retry reason, this function treatment needs to be synchronous
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR other failure
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_FileTransferAbort
(
    void
);

/**
  * @}
  */

#endif /*  __LWM2MCORE_FILE_TRANSFER_H__ */
