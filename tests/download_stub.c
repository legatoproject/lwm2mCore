/**
 * @file download_stub.c
 *
 * stub for package download
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lwm2mcore/lwm2mcore.h>
#include <lwm2mcore/lwm2mcorePackageDownloader.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "download_stub.h"
#include "download_test.h"

//--------------------------------------------------------------------------------------------------
/**
 * Global for setting downloader stub error codes
 */
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Global to know if a command is a HEAD or a GET command
 */
//--------------------------------------------------------------------------------------------------
bool IsHead = true;

//--------------------------------------------------------------------------------------------------
/**
 * Static file descriptor to store the downloaded data
 */
//--------------------------------------------------------------------------------------------------
static int FdOutput = -1;

//--------------------------------------------------------------------------------------------------
/**
 * Static file descriptor to read simulated data (HEAD/GET response)
 */
//--------------------------------------------------------------------------------------------------
static int FdReadFile = -1;

//--------------------------------------------------------------------------------------------------
/**
 * Define for maximum file name length for simulated data
 */
//--------------------------------------------------------------------------------------------------
#define FILE_NAME_MAX_LENGTH 255

//--------------------------------------------------------------------------------------------------
/**
 * Define for file name suffix (HEAD case)
 */
//--------------------------------------------------------------------------------------------------
#define HEAD_FILE_SUFFIX "_HEAD_response.txt"

//--------------------------------------------------------------------------------------------------
/**
 * Define for file name suffix (GET case)
 */
//--------------------------------------------------------------------------------------------------
#define GET_FILE_SUFFIX "_GET_response.txt"

//--------------------------------------------------------------------------------------------------
/**
 * Define for file location
 */
//--------------------------------------------------------------------------------------------------
#define FILE_PREFIX "../data/"

//--------------------------------------------------------------------------------------------------
/**
 * File name variable
 */
//--------------------------------------------------------------------------------------------------
char FileNameForPackageDownload[FILE_NAME_MAX_LENGTH] = {0};

//--------------------------------------------------------------------------------------------------
/**
  * Find the path containing the currently-running program executable
  */
//--------------------------------------------------------------------------------------------------
bool GetExecPath
(
    char* bufferPtr
)
{
    int length;
    char* pathEndPtr = NULL;

    length = readlink("/proc/self/exe", bufferPtr, FILE_NAME_MAX_LENGTH - 1);
    if (length <= 0)
    {
        return false;
    }
    bufferPtr[length] = '\0';

    // Delete the binary name from the path
    pathEndPtr = strrchr(bufferPtr, '/');
    if (NULL == pathEndPtr)
    {
        return false;
    }
    *(pathEndPtr+1) = '\0';

    return true;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to get the whole file name for a package download
 *
 * @return
 *  - true  on success
 *  - false on failure
 */
//--------------------------------------------------------------------------------------------------
static bool GetFileNameForPackageDownload
(
    const char*     prefixPtr,      ///< [IN] File prefix
    bool            isHeadCmd,      ///< [IN] true for a HEAD command, else for GET command
    char*           fileNamePtr,    ///< [INOUT] File name to be used
    size_t          fileNameSize    ///< [IN] fileNamePtr size
)
{
    if (false == GetExecPath(fileNamePtr))
    {
        printf("Error on getting path\n");
        return false;
    }

    if (isHeadCmd)
    {
        if ((fileNameSize - strlen(fileNamePtr))
                                            < (strlen(prefixPtr) + strlen(HEAD_FILE_SUFFIX) - 1))
        {
            printf("Error to get file name for HEAD: prefix %s\n", prefixPtr);
            return false;
        }
        snprintf(fileNamePtr + strlen(fileNamePtr),
                 fileNameSize - strlen(fileNamePtr),
                 "%s%s%s",
                 FILE_PREFIX,
                 prefixPtr,
                 HEAD_FILE_SUFFIX);
    }
    else
    {
        if ((fileNameSize - strlen (fileNamePtr))
                                            < (strlen(prefixPtr) + strlen(GET_FILE_SUFFIX) -1 ))
        {
            printf("Error to get file name for GET: prefix %s\n", prefixPtr);
            return false;
        }
        snprintf(fileNamePtr + strlen(fileNamePtr),
                 fileNameSize - strlen(fileNamePtr),
                 "%s%s%s",
                 FILE_PREFIX,
                 prefixPtr,
                 GET_FILE_SUFFIX);
    }
    printf("File name for package download: %s\n", fileNamePtr);
    return true;
}

//--------------------------------------------------------------------------------------------------
/**
 * Test function is called by tests.c in order to select a file name
 *
 * @return
 *  - true  on success
 *  - false on failure
 */
//--------------------------------------------------------------------------------------------------
bool test_setFileNameForPackageDownload
(
    const char*     fileNamePrefixPtr   ///<[IN] File name prefix
)
{
    if (!fileNamePrefixPtr)
    {
        return false;
    }
    memset(FileNameForPackageDownload, 0, FILE_NAME_MAX_LENGTH);
    strncpy(FileNameForPackageDownload, fileNamePrefixPtr, strlen(fileNamePrefixPtr));
    return true;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to initialize a package download
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @note The returned pointer needs to be deallocated on client side
 *
 * @return
 *  - Package download context
 *  - @c NULL on failure
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_PackageDownloadContext_t* lwm2mcore_InitForDownload
(
    bool    isHttps     ///< [IN] true if HTTPS is requested, else HTTP is requested
)
{
    lwm2mcore_PackageDownloadContext_t* contextPtr =
                                                malloc(sizeof(lwm2mcore_PackageDownloadContext_t));

    if (!contextPtr)
    {
        return NULL;
    }
    contextPtr->isInitMade = false;

    if (isHttps)
    {
        contextPtr->isSecure = true;
    }
    else
    {
        contextPtr->isSecure = false;
    }
    contextPtr->isInitMade = true;
    return contextPtr;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to initiate the connection for package download
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - LWM2MCORE_ERR_COMPLETED_OK on success
 *  - LWM2MCORE_ERR_INVALID_ARG if the parameter is invalid
 *  - LWM2MCORE_ERR_GENERAL_ERROR on failure
 *  - LWM2MCORE_ERR_MEMORY on memory allocation issue
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_ConnectForDownload
(
    lwm2mcore_PackageDownloadContext_t* contextPtr, ///< [IN] Package donwload context
    char*                               hostPtr,    ///< [IN] Host to connect on
    uint16_t                            port        ///< [IN] Port to connect on
)
{
    if ((!contextPtr) || (!hostPtr))
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }
    (void)port;
    return test_getConnectForDownloadResult();
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to disconnect the connection for package download
 *
 * @return
 *  - LWM2MCORE_ERR_COMPLETED_OK on success
 *  - LWM2MCORE_ERR_GENERAL_ERROR on failure
 *  - LWM2MCORE_ERR_INVALID_ARG if the parameter is invalid
 *  - LWM2MCORE_ERR_INVALID_STATE if no connection was initiated for package download
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_DisconnectForDownload
(
    lwm2mcore_PackageDownloadContext_t* contextPtr  ///< [IN] Package donwload context
)
{
    if (!contextPtr)
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    close (FdReadFile);
    FdReadFile = -1;
    return LWM2MCORE_ERR_COMPLETED_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to free the connection for package download
 *
 * @return
 *  - LWM2MCORE_ERR_COMPLETED_OK on success
 *  - LWM2MCORE_ERR_GENERAL_ERROR on failure
 *  - LWM2MCORE_ERR_INVALID_ARG if the parameter is invalid
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_FreeForDownload
(
    lwm2mcore_PackageDownloadContext_t* contextPtr  ///< [IN] Package donwload context
)
{
    if (!contextPtr)
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    free(contextPtr);
    return LWM2MCORE_ERR_COMPLETED_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to send a HTTP request for package download
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - LWM2MCORE_ERR_COMPLETED_OK on success
 *  - LWM2MCORE_ERR_INVALID_ARG if the request is invalid
 *  - LWM2MCORE_ERR_GENERAL_ERROR on failure
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_SendForDownload
(
    lwm2mcore_PackageDownloadContext_t* contextPtr,         ///< [IN] Package donwload context
    char*                               serverRequestPtr    ///< [IN] HTTP(S) request
)
{
    if ((!contextPtr) || (!serverRequestPtr))
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    printf("Request sent to the server:\n%s\n", serverRequestPtr);

    if (!strncmp(serverRequestPtr, "HEAD", strlen("HEAD")))
    {
        printf("HEAD received\n");
        IsHead = true;
    }
    else if (!strncmp(serverRequestPtr, "GET", strlen("GET")))
    {
        printf("GET received\n");
        IsHead = false;
    }
    else
    {
        printf("Unsupported command");
        return LWM2MCORE_ERR_GENERAL_ERROR;
    }
    return LWM2MCORE_ERR_COMPLETED_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to read received data for package download
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - LWM2MCORE_ERR_COMPLETED_OK on success
 *  - LWM2MCORE_ERR_INVALID_ARG if the request is invalid
 *  - LWM2MCORE_ERR_GENERAL_ERROR on failure
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_ReadForDownload
(
    lwm2mcore_PackageDownloadContext_t* contextPtr,     ///< [IN] Package donwload context
    char*                               bufferPtr,      ///< [INOUT] Buffer
    int*                                lenPtr          ///< [IN] Buffer length
)
{
    int readLen = 0;
    lwm2mcore_Sid_t result;

    if ((!contextPtr) || (!bufferPtr) || (!lenPtr))
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    result = test_getReadForDownloadResult();
    if (LWM2MCORE_ERR_COMPLETED_OK != result)
    {
        return result;
    }

    if (-1 == FdReadFile)
    {
        char fileName[FILE_NAME_MAX_LENGTH] = {0};
        if (!strlen(FileNameForPackageDownload))
        {
            printf("Error on file name for package download\n");
            return LWM2MCORE_ERR_GENERAL_ERROR;
        }
        TEST_ASSERT(true == GetFileNameForPackageDownload(FileNameForPackageDownload,
                                                          IsHead,
                                                          fileName,
                                                          sizeof(fileName)));
        FdReadFile = open(fileName, O_RDONLY, S_IRUSR | S_IWUSR);

        if (-1 == FdReadFile)
        {
            fprintf(stderr, "Error to open file\n");
            return LWM2MCORE_ERR_GENERAL_ERROR;
        }
    }

    readLen = read(FdReadFile, bufferPtr, *lenPtr);
    if (-1 == readLen)
    {
        fprintf(stderr, "Read error %m\n");
        return LWM2MCORE_ERR_GENERAL_ERROR;
    }
    *lenPtr = readLen;
    return LWM2MCORE_ERR_COMPLETED_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Write data
 *
 * This function is called in a dedicated thread/task.
 *
 * @return
 *  - LWM2MCORE_ERR_COMPLETED_OK on success
 *  - LWM2MCORE_ERR_INVALID_STATE if no package download is suspended
 *  - LWM2MCORE_ERR_GENERAL_ERROR on failure
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_WritePackageData
(
    uint8_t* bufferPtr,     ///< [IN] Data to be written
    uint32_t length,        ///< [IN] Data length
    void*    opaquePtr      ///< [IN] Opaque pointer
)
{
    struct stat sb;
    int lwrite;

    (void)opaquePtr;

    if (-1 == stat("download.bin", &sb))
    {
        printf("Create the output file to store downloaded data\n");
        FdOutput = open("download.bin", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    }
    else
    {
        if(-1 == FdOutput)
        {
            FdOutput = open("download.bin", O_WRONLY | O_APPEND, S_IRUSR | S_IWUSR);
        }
    }
    lwrite = write(FdOutput, bufferPtr, length);
    if (-1 == lwrite)
    {
        fprintf(stderr, "Write error %m\n");
        return LWM2MCORE_ERR_GENERAL_ERROR;
    }
    return LWM2MCORE_ERR_COMPLETED_OK;
}
