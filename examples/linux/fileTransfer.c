/**
 * @file fileTransfer.c
 *
 * Porting layer for file transfer
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#ifdef LWM2M_OBJECT_33406

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include "internals.h"
#include <platform/types.h>
#include <lwm2mcore/lwm2mcore.h>
#include <lwm2mcore/fileTransfer.h>
#include "fileMngt.h"

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
)
{
    LOG("File info for transfer");
    LOG_ARG("Name: %s", fileTransferInfo.fileName);
    LOG_ARG("Class: %s", fileTransferInfo.fileClass);
    LOG_ARG("Hash: %s", fileTransferInfo.fileHash);
    LOG_ARG("Direction: %d", fileTransferInfo.direction);
    (void)couldDwnldBeLaunchedPtr;
    return LWM2MCORE_ERR_COMPLETED_OK;
}

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
)
{
    #define FILE_TRANSFER_CHECKSUM "ABCDEF0123456789"
    if ((!bufferPtr) || (!bufferSizePtr))
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    if ((strlen(FILE_TRANSFER_CHECKSUM) + 5) > (*bufferSizePtr))
    {
        return LWM2MCORE_ERR_OVERFLOW;
    }

    snprintf(bufferPtr, *bufferSizePtr, "%s", FILE_TRANSFER_CHECKSUM);
    *bufferSizePtr = strlen(bufferPtr);

    return LWM2MCORE_ERR_COMPLETED_OK;
}


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
)
{
    #define FILE_NAME "FileName"
    if ((!bufferPtr) || (!bufferSizePtr))
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    if ((strlen(FILE_NAME) + 5) > *bufferSizePtr)
    {
        return LWM2MCORE_ERR_OVERFLOW;
    }

    snprintf(bufferPtr, *bufferSizePtr, "%s%d", FILE_NAME, instanceId);
    *bufferSizePtr = strlen(bufferPtr);

    return LWM2MCORE_ERR_COMPLETED_OK;
}


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
)
{
    #define FILE_CLASS "FileClass"
    if ((!bufferPtr) || (!bufferSizePtr))
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    if ((strlen(FILE_CLASS) + 5) > *bufferSizePtr)
    {
        return LWM2MCORE_ERR_OVERFLOW;
    }

    snprintf(bufferPtr, *bufferSizePtr, "%s%d", FILE_CLASS, instanceId);
    *bufferSizePtr = strlen(bufferPtr);

    return LWM2MCORE_ERR_COMPLETED_OK;
}

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
)
{
    #define FILE_HASH "010203040506070809"

    if ((!bufferPtr) || (!bufferSizePtr))
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    if ((strlen(FILE_HASH) + 5) > *bufferSizePtr)
    {
        return LWM2MCORE_ERR_OVERFLOW;
    }

    snprintf(bufferPtr, *bufferSizePtr, "%s%d", FILE_HASH, instanceId);
    *bufferSizePtr = strlen(bufferPtr);

    return LWM2MCORE_ERR_COMPLETED_OK;
}

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
)
{
    if (!originPtr)
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }
    *originPtr = LWM2MCORE_FILE_LIST_ORIGIN_SERVER;
    (void)instanceId;
    return LWM2MCORE_ERR_COMPLETED_OK;
}

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
)
{
    (void)instanceId;
    return LWM2MCORE_ERR_COMPLETED_OK;
}

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
)
{
    if (!availableSpacePtr)
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }
    *availableSpacePtr = 100;
    return LWM2MCORE_ERR_COMPLETED_OK;
}

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
)
{
    return LWM2MCORE_ERR_COMPLETED_OK;
}
#endif /* LWM2M_OBJECT_33406 */
