/**
 * @file downloader.c
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#include <liblwm2m.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <lwm2mcore/lwm2mcore.h>
#include <lwm2mcore/update.h>
#include <lwm2mcore/security.h>
#include <lwm2mcore/memory.h>
#include <lwm2mcore/lwm2mcorePackageDownloader.h>
#include "downloader.h"
#include <internals.h>
#include "http.h"
#include "handlers.h"

#ifndef LWM2M_EXTERNAL_DOWNLOADER

//--------------------------------------------------------------------------------------------------
/**
 * Define value for HTTP protocol in HTTP header
 */
//--------------------------------------------------------------------------------------------------
#define HTTP_PROTOCOL "http"

//--------------------------------------------------------------------------------------------------
/**
 * Define value for HTTPS protocol in HTTP header
 */
//--------------------------------------------------------------------------------------------------
#define HTTPS_PROTOCOL "https"

//--------------------------------------------------------------------------------------------------
/**
 * Define value for GET command in HTTP header (including space)
 */
//--------------------------------------------------------------------------------------------------
#define GET "GET "

//--------------------------------------------------------------------------------------------------
/**
 * Define value for HEAD command in HTTP header (including space)
 */
//--------------------------------------------------------------------------------------------------
#define HEAD "HEAD "

//--------------------------------------------------------------------------------------------------
/**
 * Define value for HTTP in HTTP header (including space)
 */
//--------------------------------------------------------------------------------------------------
#define HTTP " HTTP/1.1"

//--------------------------------------------------------------------------------------------------
/**
 * Define value for Host field in HTTP header (including space)
 */
//--------------------------------------------------------------------------------------------------
#define HOST "Host: "

//--------------------------------------------------------------------------------------------------
/**
 * Define value for Range field in HTTP header (including space)
 */
//--------------------------------------------------------------------------------------------------
#define RANGE "Range: bytes="

//--------------------------------------------------------------------------------------------------
/**
 * Define value for content-length field in HTTP header response
 */
//--------------------------------------------------------------------------------------------------
#define CONTENT_LENGTH "content-length"

//--------------------------------------------------------------------------------------------------
/**
 * Define value for HTTP port
 */
//--------------------------------------------------------------------------------------------------
#define HTTP_PORT 80

//--------------------------------------------------------------------------------------------------
/**
 * Define value for HTTPS port
 */
//--------------------------------------------------------------------------------------------------
#define HTTPS_PORT 443

//--------------------------------------------------------------------------------------------------
/**
 * Define value for base 10
 */
//--------------------------------------------------------------------------------------------------
#define BASE10 10

//--------------------------------------------------------------------------------------------------
/**
 * Define value for the download buffer size
 */
//--------------------------------------------------------------------------------------------------
#ifndef LWM2MCORE_DWNLD_BUFFER_SIZE
#define LWM2MCORE_DWNLD_BUFFER_SIZE 4096
#endif

//--------------------------------------------------------------------------------------------------
/**
 * Define value for CR/LF length
 */
//--------------------------------------------------------------------------------------------------
#define CR_LF_LENGTH 2

//--------------------------------------------------------------------------------------------------
/**
 * Current download status.
 */
//--------------------------------------------------------------------------------------------------
uint8_t DownloadStatus = DWL_OK;

//--------------------------------------------------------------------------------------------------
/**
 * Static structure for package download
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_PackageDownloadContext_t* PackageDownloadContextPtr;

//--------------------------------------------------------------------------------------------------
/**
 * Global value for last HTTP(S) error code.
 */
//--------------------------------------------------------------------------------------------------
uint16_t HttpErrorCode = 0;

//--------------------------------------------------------------------------------------------------
/**
 * Enumeration for HTTP command
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    HTTP_HEAD,      ///< Enumeration for HTTP HEAD
    HTTP_GET        ///< Enumeration for HTTP GET
}
HttpCommand_t;

//--------------------------------------------------------------------------------------------------
/**
 * Structure used to parse an URI and package information
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    lwm2mcore_FwUpdateProtocolSupport_t protocol;           ///< Protocol to be used: HTTP or HTTPS
    char*                               hostPtr;            ///< Host
    char*                               pathPtr;            ///< Package path
    uint32_t                            packageSize;        ///< Package size
    uint32_t                            downloadedBytes;    ///< Downloaded bytes
    uint32_t                            range;              ///< Range for HTTP GET
    int                                 httpCode;           ///< Last HTTP error code
    void*                               opaquePtr;          ///< Opaque pointer;
    uint16_t                            port;               ///< Port
    bool                                isHead;             ///< true for HEAD command, false else
}
PackageUriDetails_t;

//--------------------------------------------------------------------------------------------------
/**
 * Static structure for package details
 */
//--------------------------------------------------------------------------------------------------
static PackageUriDetails_t PackageUriDetails;

//--------------------------------------------------------------------------------------------------
/**
 * tinyHTTP callback for realloc
 */
//--------------------------------------------------------------------------------------------------
static void* TinyHttpReallocCb
(
    void*   opaquePtr,      ///< [IN] User data context
    void*   ptr,            ///< [IN] Data pointer
    int     size            ///< [IN] New size allocation
);

//--------------------------------------------------------------------------------------------------
/**
 * tinyHTTP callback for received data in HTTP body response
 */
//--------------------------------------------------------------------------------------------------
static void TinyHttpBodyRspCb
(
    void*       opaquePtr,  ///< [IN] User data context
    const char* dataPtr,    ///< [IN] HTTP body data
    int         size        ///< [IN] Data length
);

//--------------------------------------------------------------------------------------------------
/**
 * tinyHTTP callback for received data in HTTP header response
 */
//--------------------------------------------------------------------------------------------------
static void TinyHttpHeaderRspCb
(
    void*       opaquePtr,  ///< [IN] User data context
    const char* keyPtr,     ///< [IN] Key field in the HTTP header response
    int         nkey,       ///< [IN] Key field length
    const char* valuePtr,   ///< [IN] Key value in the HTTP header response
    int         nvalue      ///< [IN] Key value length
);

//--------------------------------------------------------------------------------------------------
/**
 * tinyHTTP callback for received HTTP error code
 */
//--------------------------------------------------------------------------------------------------
static void TinyHttpErrorCodeCb
(
    void*   opaquePtr,      ///< [IN] User data context
    int     code            ///< [IN] HTTP error code
);

//--------------------------------------------------------------------------------------------------
/**
 * Structure for tinyHTTP callbacks
 */
//--------------------------------------------------------------------------------------------------
struct http_funcs responseFuncs = {
    TinyHttpReallocCb,
    TinyHttpBodyRspCb,
    TinyHttpHeaderRspCb,
    TinyHttpErrorCodeCb
};


//--------------------------------------------------------------------------------------------------
/**
 * Convert string to integer
 *
 * @return
 *    - DOWNLOADER_INVALID_ARG invalid input parameter
 *    - DOWNLOADER_ERROR on failure
 *    - DOWNLOADER_OK on success
 */
//--------------------------------------------------------------------------------------------------
static downloaderResult_t GetPortNumber
(
    char*        strPtr,          ///< [IN] URI to be converted on integer
    uint16_t*    valPtr           ///< [OUT] conversion result
)
{
    long rc;
    char* endPtr = NULL;

    if (!strPtr || !valPtr)
    {
        return DOWNLOADER_INVALID_ARG;
    }

    if ('\0' == *strPtr)
    {
        return DOWNLOADER_INVALID_ARG;
    }
    errno = 0;
    rc = strtoul(strPtr, &endPtr, BASE10);

    if (errno)
    {
        return DOWNLOADER_ERROR;
    }

    if (*endPtr != '\0')
    {
        return DOWNLOADER_ERROR;
    }

    if ((1 > rc) || (USHRT_MAX < rc))
    {
        return DOWNLOADER_ERROR;
    }

    *valPtr = rc;

    return DOWNLOADER_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Parse an URI and fill information in packageUriDetails parameter
 *
 * @return
 *  - true on success
 *  - false on failure
 */
//--------------------------------------------------------------------------------------------------
static bool ParsePackageURI
(
    char*                   packageUriPtr,          ///< [IN] Package URI
    PackageUriDetails_t*    packageUriDetailsPtr    ///< [INOUT] Package URI details

)
{
    char* tokenPtr;
    char* savedTokenPtr;
    char* subTokenPtr;
    char* subSavedTokenPtr;
    bool  isHttpProtocol = false;

    if ((!packageUriPtr) || (!packageUriDetailsPtr))
    {
        return false;
    }

    if (!strlen(packageUriPtr))
    {
        LOG("Empty URL");
        return false;
    }

    LOG_ARG("Parse URL: packageUriPtr %s", packageUriPtr);

    /* Get the protocol */
    tokenPtr = strtok_r(packageUriPtr, ":", &savedTokenPtr);

    if (!tokenPtr)
    {
        return false;
    }
    /* Check if the protocol is HTTP or HTTPS */
    if (0 == strncasecmp(tokenPtr, HTTPS_PROTOCOL, strlen(HTTPS_PROTOCOL)))
    {
        LOG("HTTPS");
        packageUriDetailsPtr->protocol = LWM2MCORE_FW_UPDATE_HTTPS_1_1_PROTOCOL;
    }
    else if (0 == strncasecmp(tokenPtr, HTTP_PROTOCOL, strlen(HTTP_PROTOCOL)))
    {
        LOG("HTTP");
        packageUriDetailsPtr->protocol = LWM2MCORE_FW_UPDATE_HTTP_1_1_PROTOCOL;
        isHttpProtocol = true ;
    }
    else
    {
        LOG("ERROR in uri");
        return false;
    }

    /* Get host */
    tokenPtr = strtok_r(NULL, "/", &savedTokenPtr);
    if (!tokenPtr)
    {
        return false;
    }
    // Check if a specific port is selected
    subTokenPtr = strtok_r(tokenPtr, ":", &subSavedTokenPtr);
    if (!subTokenPtr)
    {
        return false;
    }
    packageUriDetailsPtr->hostPtr = subTokenPtr;
    LOG_ARG("hostPtr %s", packageUriDetailsPtr->hostPtr);

    // Get the port number if it exists
    subTokenPtr = strtok_r(NULL, "/", &subSavedTokenPtr);
    if (!subTokenPtr)
    {
        LOG("Port number is not provided so use http(s) default port");
        if(isHttpProtocol)
        {
            packageUriDetailsPtr->port = HTTP_PORT;
        }
        else
        {
            packageUriDetailsPtr->port = HTTPS_PORT;
        }
    }
    else
    {
        if(DOWNLOADER_OK != GetPortNumber(subTokenPtr, &(packageUriDetailsPtr->port)))
        {
            return false;
        }
        LOG_ARG("Port number : %"PRIu16 "", packageUriDetailsPtr->port);
    }
    /* Get path */
    tokenPtr = strtok_r(NULL, "?", &savedTokenPtr);
    if (!tokenPtr)
    {
        return false;
    }

    packageUriDetailsPtr->pathPtr = tokenPtr;
    LOG_ARG("pathPtr %s", packageUriDetailsPtr->pathPtr);

    return true;
}

//--------------------------------------------------------------------------------------------------
/**
 * tinyHTTP callback for realloc
 */
//--------------------------------------------------------------------------------------------------
static void* TinyHttpReallocCb
(
    void*   opaquePtr,      ///< [IN] User data context
    void*   ptr,            ///< [IN] Data pointer
    int     size            ///< [IN] New size allocation
)
{
    (void)opaquePtr;
    return lwm2mcore_realloc(ptr, size);
}

//--------------------------------------------------------------------------------------------------
/**
 * tinyHTTP callback for received data in HTTP body response
 */
//--------------------------------------------------------------------------------------------------
static void TinyHttpBodyRspCb
(
    void*       opaquePtr,  ///< [IN] User data context
    const char* dataPtr,    ///< [IN] HTTP body data
    int         size        ///< [IN] Data length
)
{
    lwm2mcore_DwlResult_t dwlResult;
    PackageUriDetails_t* packageDetailsPtr = (PackageUriDetails_t*)opaquePtr;
    packageDetailsPtr->downloadedBytes += (uint32_t)size;
    dwlResult = lwm2mcore_PackageDownloaderReceiveData((uint8_t*)dataPtr,
                                                       (size_t)size,
                                                       packageDetailsPtr->opaquePtr);
    if (DWL_OK != dwlResult)
    {
        LOG("Error on treated received data");
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * tinyHTTP callback for received data in HTTP header response
 */
//--------------------------------------------------------------------------------------------------
static void TinyHttpHeaderRspCb
(
    void*       opaquePtr,  ///< [IN] User data context
    const char* keyPtr,     ///< [IN] Key field in the HTTP header response
    int         nkey,       ///< [IN] Key field length
    const char* valuePtr,   ///< [IN] Key value in the HTTP header response
    int         nvalue      ///< [IN] Key value length
)
{
    char printKey[nkey + 1];
    char printVal[nvalue + 1];
    PackageUriDetails_t* packageDetailsPtr = (PackageUriDetails_t*)opaquePtr;
    memcpy(printKey, keyPtr, nkey);
    printKey[nkey] = '\0';
    memcpy(printVal, valuePtr, nvalue);
    printVal[nvalue] = '\0';

    if (!strncmp(CONTENT_LENGTH, printKey, nkey))
    {
        if (!(packageDetailsPtr->packageSize))
        {
            LOG_ARG("key: %s - value: %s", printKey, printVal);
            packageDetailsPtr->packageSize = strtoul(printVal, NULL, BASE10);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * tinyHTTP callback for received HTTP error code
 */
//--------------------------------------------------------------------------------------------------
static void TinyHttpErrorCodeCb
(
    void*   opaquePtr,      ///< [IN] User data context
    int     code            ///< [IN] HTTP error code
)
{
    PackageUriDetails_t* packageDetailsPtr = (PackageUriDetails_t*)opaquePtr;
    packageDetailsPtr->httpCode = code;
    HttpErrorCode = code;
    LOG_ARG("HTTP code: %d", code);
}

//--------------------------------------------------------------------------------------------------
/**
 * Set download status
 */
//--------------------------------------------------------------------------------------------------
static void SetDownloadStatus
(
    lwm2mcore_DwlResult_t newDownloadStatus   ///< New download status to set
)
{
    //(); //TODO
    DownloadStatus = newDownloadStatus;
    //UNLOCK();
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to prepare the HTTP request
 *
 * @return
 *  - @ref serverRequest on success
 *  - @ref NULL on failure
 */
//--------------------------------------------------------------------------------------------------
static char* ConstructServerRequest
(
    lwm2mcore_PackageDownloadContext_t* downloadContextPtr, ///< [IN] Context
    HttpCommand_t                       command,            ///< [IN] HTTP command to be sent
    PackageUriDetails_t*                packageDetailsPtr,  ///< [IN] Package details
    bool                                isResume            ///< [IN] Indicates a resume request
                                                            ///< (only available for HTTP GET)
)
{
    int serverRequestLen;
    char* serverRequestPtr;

    if ((!downloadContextPtr) || (!packageDetailsPtr))
    {
        return NULL;
    }

    /* Calculate the request length to be sent */
    serverRequestLen = strlen(packageDetailsPtr->pathPtr) +
                       strlen(HTTP) +
                       strlen(HOST) +
                       strlen(packageDetailsPtr->hostPtr);

    switch(command)
    {
        case HTTP_HEAD:
            packageDetailsPtr->isHead = true;
            /* Add 3 \r\n, one / and final char */
            serverRequestLen += strlen(HEAD) + (3 * CR_LF_LENGTH) + 1 + 1;
            break;

        case HTTP_GET:
            packageDetailsPtr->isHead = false;
            /* Add 3 \r\n, one / and final char */
            serverRequestLen += strlen(GET) + (3 * CR_LF_LENGTH) + 1 + 1;

            if (isResume)
            {
                /* Add  1 \r\n, 1 '-'
                 * and length for maximum range length: aaaaaaaaa-bbbbbbbbb
                 * and final char
                 */
                serverRequestLen += strlen(RANGE) + CR_LF_LENGTH + 1 +19 + 1;
            }
            break;

        default:
            return NULL;
    }

    /* Fill the request to be sent */
    serverRequestPtr = lwm2m_malloc(sizeof(char)*serverRequestLen);

    if(!serverRequestPtr)
    {
        return NULL;
    }

    memset(serverRequestPtr, 0, sizeof(char)*serverRequestLen);

    snprintf(serverRequestPtr,
             serverRequestLen,
             "%s/%s%s\r\n%s%s",
             command == HTTP_HEAD ? HEAD : GET,
             packageDetailsPtr->pathPtr,
             HTTP,
             HOST,
             packageDetailsPtr->hostPtr);

    if ( (false == (packageDetailsPtr->isHead))
      && (isResume))
    {
        /* In case of HTTP GET and resume, add Range field */
        snprintf(serverRequestPtr + strlen(serverRequestPtr),
                 serverRequestLen - strlen(serverRequestPtr),
                 "\r\n%s%"PRIu32"-",
                 RANGE,
                 packageDetailsPtr->range);
    }

    snprintf(serverRequestPtr + strlen(serverRequestPtr),
             serverRequestLen - strlen(serverRequestPtr),
             "\r\n\r\n");
    return serverRequestPtr;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to send data on stream
 *
 * @return
 *  - @ref DOWNLOADER_OK on success
 *  - @ref DOWNLOADER_INVALID_ARG when the package URL is not valid
 *  - @ref DOWNLOADER_CONNECTION_ERROR when the host can not be reached
 *  - @ref DOWNLOADER_PARTIAL_FILE when partial file is received even if HTTP request succeeds
 *  - @ref DOWNLOADER_RECV_ERROR when error occurs on data receipt
 *  - @ref DOWNLOADER_ERROR on failure
 *  - @ref DOWNLOADER_TIMEOUT if any timer expires for a LwM2MCore called function
 */
//--------------------------------------------------------------------------------------------------
static downloaderResult_t SendHttpRequest
(
    lwm2mcore_PackageDownloadContext_t* downloadContextPtr, ///< [IN] Context
    HttpCommand_t                       command,            ///< [IN] HTTP command to be sent
    PackageUriDetails_t*                packageDetailsPtr,  ///< [IN] Package details
    bool                                isResume            ///< [IN] Indicates a resume request
                                                            ///< (only available for HTTP GET)
)
{
    int len;
    bool loop = true;
    int read;
    int needmore;
    struct http_roundtripper rt;
    char buffer[LWM2MCORE_DWNLD_BUFFER_SIZE];
    char* serverRequestPtr;
    lwm2mcore_DwlResult_t dwlStatus;
    downloaderResult_t result = DOWNLOADER_OK;
    lwm2mcore_Sid_t readResult;

    if ((!downloadContextPtr) || (!packageDetailsPtr))
    {
        return DOWNLOADER_INVALID_ARG;
    }

    memset(buffer, 0, LWM2MCORE_DWNLD_BUFFER_SIZE);

    serverRequestPtr = ConstructServerRequest(downloadContextPtr, command,
                                              packageDetailsPtr, isResume);
    if(serverRequestPtr == NULL)
    {
        return DOWNLOADER_INVALID_ARG;
    }

    /* Send HTTP command */
    http_init(&rt, (struct http_funcs) responseFuncs, packageDetailsPtr);
    LOG("################");
    LOG(" HTTP REQUEST");
    if (LWM2MCORE_ERR_COMPLETED_OK != lwm2mcore_SendForDownload(downloadContextPtr,
                                                                serverRequestPtr))
    {
        LOG("Error on send data");
        lwm2m_free(serverRequestPtr);
        http_free(&rt);
        SetDownloadStatus(DWL_FAULT);
        return DOWNLOADER_SEND_ERROR;
    }

    LOG("################");
    LOG(" HTTP RESPONSE");
    LOG_ARG("downloader_GetDownloadStatus %d", downloader_GetDownloadStatus());
    while ((loop)
        && (DWL_OK == downloader_GetDownloadStatus()))
    {
        len = LWM2MCORE_DWNLD_BUFFER_SIZE;
        readResult = lwm2mcore_ReadForDownload(downloadContextPtr, buffer, &len);
        if (LWM2MCORE_ERR_COMPLETED_OK == readResult)
        {
            if (len > 0)
            {
                needmore = http_data(&rt, buffer, len, &read);
                if (!needmore)
                {
                    loop = false;
                }
                if (!read)
                {
                    loop = false;

                }
            }
            else
            {
                loop = false;
                result = DOWNLOADER_RECV_ERROR;
            }
        }
        else if (LWM2MCORE_ERR_TIMEOUT == readResult)
        {
            loop = false;
            result = DOWNLOADER_TIMEOUT;
        }
        else
        {
            loop = false;
            result = DOWNLOADER_RECV_ERROR;
        }
    }
    LOG("################");
    LOG_ARG("lwm2mcore_ReadForDownload ended -> downloader result %d", result);


    LOG_ARG("downloadedBytes %lu\nRange \t\t%lu\nHTTP code \t%d",
            packageDetailsPtr->downloadedBytes,
            packageDetailsPtr->range,
            packageDetailsPtr->httpCode);

    dwlStatus = downloader_GetDownloadStatus();

    if ( (false == (packageDetailsPtr->isHead))
      && (DWL_OK != dwlStatus))
    {
        LOG_ARG("Download stopped after %lu bytes", packageDetailsPtr->downloadedBytes);
    }

    LOG_ARG("Package details:\nprotocol \t%s\nhost \t\t%s\npath \t\t%s\nSize \t\t%lu\n"\
            "downloadedBytes %lu\nHead \t\t%d\nHTTP code \t%d",
            (packageDetailsPtr->protocol == LWM2MCORE_FW_UPDATE_HTTP_1_1_PROTOCOL)?"HTTP":"HTTPS",
            packageDetailsPtr->hostPtr,
            packageDetailsPtr->pathPtr,
            packageDetailsPtr->packageSize,
            packageDetailsPtr->downloadedBytes,
            packageDetailsPtr->isHead,
            packageDetailsPtr->httpCode);

    lwm2m_free(serverRequestPtr);
    http_free(&rt);

    if (result == DOWNLOADER_TIMEOUT)
    {
        LOG("Download time out");
        return DOWNLOADER_TIMEOUT;
    }

    if ((DWL_SUSPEND == dwlStatus) || (DWL_ABORTED == dwlStatus))
    {
        LOG("Download suspended/aborted");
        return DOWNLOADER_OK;
    }

    if (((HTTP_200 == packageDetailsPtr->httpCode) || (HTTP_206 == packageDetailsPtr->httpCode))
     && (HTTP_GET == command)
     && ((packageDetailsPtr->packageSize + packageDetailsPtr->range) !=
                                                (packageDetailsPtr->downloadedBytes)))
    {
        LOG("Download status is OK but all bytes were not downloaded");
        downloader_SuspendDownload();
        return DOWNLOADER_PARTIAL_FILE;
    }

    if (!(packageDetailsPtr->httpCode))
    {
        return DOWNLOADER_ERROR;
    }

    if ((HTTP_200 != packageDetailsPtr->httpCode) && (HTTP_206 != packageDetailsPtr->httpCode))
    {
        return DOWNLOADER_ERROR;
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to send a HTTP command on stream
 *
 * @return
 *  - @ref DOWNLOADER_OK on success
 *  - @ref DOWNLOADER_INVALID_ARG when the package URL is not valid
 *  - @ref DOWNLOADER_CONNECTION_ERROR when the host can not be reached
 *  - @ref DOWNLOADER_PARTIAL_FILE when partial file is received even if HTTP request succeeds
 *  - @ref DOWNLOADER_RECV_ERROR when error occurs on data receipt
 *  - @ref DOWNLOADER_ERROR on failure
 *  - @ref DOWNLOADER_TIMEOUT if any timer expires for a LwM2MCore called function
 *  - @ref DOWNLOADER_MEMORY_ERROR on memory allocation issue
 */
//--------------------------------------------------------------------------------------------------
static downloaderResult_t SendRequest
(
    HttpCommand_t           command,        ///< [IN] HTTP command to be sent
    char*                   packageUriPtr,  ///< [IN] Package URI
    uint64_t                offset,         ///< [IN] Offset for the download
    uint64_t*               packageSizePtr, ///< [INOUT] Package size
    void*                   opaquePtr       ///< [IN] Opaque pointer
)
{
    bool isSuccess = true;
    bool isTimeOut = false;
    bool isSecure;
    downloaderResult_t result;
    bool isResume = false;
    lwm2mcore_Sid_t connectResult;
    lwm2mcore_PackageDownloadContext_t* packageDownloadCtxPtr;

    if ((!packageUriPtr) || (!packageSizePtr))
    {
        LOG("Invalid arg");
        return DOWNLOADER_INVALID_ARG;
    }

    if (!strlen(packageUriPtr))
    {
        LOG("Empty URL");
        return DOWNLOADER_INVALID_ARG;
    }

    if (LWM2MCORE_PACKAGE_URI_MAX_LEN < strlen(packageUriPtr))
    {
        LOG("Too long URL");
        return DOWNLOADER_INVALID_ARG;
    }

    if (offset && (HTTP_HEAD == command))
    {
        return DOWNLOADER_INVALID_ARG;
    }

    memset(&PackageUriDetails, 0, sizeof(PackageUriDetails_t));
    PackageUriDetails.opaquePtr = opaquePtr;

    LOG_ARG("Package uri %s", packageUriPtr);

    /* Parse the package URL */
    if (ParsePackageURI(packageUriPtr, &PackageUriDetails))
    {
        LOG_ARG("Package URL details: \nprotocol \t%s\nhost \t\t%s\npath \t\t%s\nport \t\t%"PRIu16 "",
                (LWM2MCORE_FW_UPDATE_HTTP_1_1_PROTOCOL == PackageUriDetails.protocol)?"HTTP":
                                                                                      "HTTPS",
                PackageUriDetails.hostPtr,
                PackageUriDetails.pathPtr,
                PackageUriDetails.port);
    }
    else
    {
        LOG("Error on package URL parsing");
        return DOWNLOADER_INVALID_ARG;
    }

    if ((!PackageUriDetails.pathPtr) || (!PackageUriDetails.hostPtr))
    {
        LOG("Error on URL parsing");
        return DOWNLOADER_INVALID_ARG;
    }

    switch(PackageUriDetails.protocol)
    {
        case LWM2MCORE_FW_UPDATE_HTTP_1_1_PROTOCOL:
        {
            isSecure = false;
        }
        break;

        case LWM2MCORE_FW_UPDATE_HTTPS_1_1_PROTOCOL:
        {
            isSecure = true;
        }
        break;

        default:
            LOG("Unsupported protocol");
            return DOWNLOADER_INVALID_ARG;
    }

    /* Initialize the download */
    packageDownloadCtxPtr = lwm2mcore_InitForDownload(isSecure);
    if (NULL == packageDownloadCtxPtr)
    {
        LOG("Error on download initialization");
        return DOWNLOADER_ERROR;
    }

    LOG("Download init done");

    /* Connect */
    connectResult = lwm2mcore_ConnectForDownload(packageDownloadCtxPtr,
                                                 PackageUriDetails.hostPtr,
                                                 PackageUriDetails.port);

    if (LWM2MCORE_ERR_COMPLETED_OK != connectResult)
    {
        if (LWM2MCORE_ERR_COMPLETED_OK != lwm2mcore_DisconnectForDownload(packageDownloadCtxPtr))
        {
            LOG("Error on download disconnection");
            if (LWM2MCORE_ERR_COMPLETED_OK != lwm2mcore_FreeForDownload(packageDownloadCtxPtr))
            {
                LOG("Error on download free");
            }
        }
    }

    switch (connectResult)
    {
        case LWM2MCORE_ERR_NET_RECV_FAILED:
            LOG("Error on download connection receive data");
            if (LWM2MCORE_ERR_COMPLETED_OK != lwm2mcore_FreeForDownload(packageDownloadCtxPtr))
            {
                LOG("Error on download free");
            }
            return DOWNLOADER_RECV_ERROR;

        case LWM2MCORE_ERR_NET_SEND_FAILED:
            LOG("Error on download connection send data");
            if (LWM2MCORE_ERR_COMPLETED_OK != lwm2mcore_FreeForDownload(packageDownloadCtxPtr))
            {
                LOG("Error on download free");
            }
            return DOWNLOADER_SEND_ERROR;

        case LWM2MCORE_ERR_MEMORY:
            LOG("Memory allocation issue on download connection");
            return DOWNLOADER_MEMORY_ERROR;

        case LWM2MCORE_ERR_NET_ERROR:
            LOG("Error on connection");
            return DOWNLOADER_CONNECTION_ERROR;

        case LWM2MCORE_ERR_COMPLETED_OK:
            break;

        default:
            LOG("Error on download connection");
            return DOWNLOADER_CONNECTION_ERROR;
    }

    if (offset)
    {
        PackageUriDetails.downloadedBytes = offset;
        PackageUriDetails.range = offset;
    }

    if (offset)
    {
        isResume = true;
    }

    result = SendHttpRequest(packageDownloadCtxPtr, command, &PackageUriDetails, isResume);
    switch (result)
    {
        case DOWNLOADER_OK:
            LOG("Command succeeds");
            break;
        case DOWNLOADER_TIMEOUT:
            LOG("Command succeeds but time out on reading");
            isTimeOut = true;
            break;
        case DOWNLOADER_INVALID_ARG:
        case DOWNLOADER_CONNECTION_ERROR:
        case DOWNLOADER_PARTIAL_FILE:
        case DOWNLOADER_RECV_ERROR:
        case DOWNLOADER_SEND_ERROR:
        case DOWNLOADER_ERROR:
        default:
            LOG("Error on command");
            isSuccess = false;
            break;
    }

    /* Disconnect */
    if (LWM2MCORE_ERR_COMPLETED_OK != lwm2mcore_DisconnectForDownload(packageDownloadCtxPtr))
    {
        LOG("Error on download disconnection");
        if (LWM2MCORE_ERR_COMPLETED_OK != lwm2mcore_FreeForDownload(packageDownloadCtxPtr))
        {
           LOG("Error on download free");
        }
        return DOWNLOADER_ERROR;
    }

    if (LWM2MCORE_ERR_COMPLETED_OK != lwm2mcore_FreeForDownload(packageDownloadCtxPtr))
    {
        LOG("Error on download free");
        return DOWNLOADER_ERROR;
    }

    if (false == isSuccess)
    {
        return result;
    }

    if (true == isTimeOut)
    {
        return DOWNLOADER_TIMEOUT;
    }

    if (packageSizePtr)
    {
        *packageSizePtr = PackageUriDetails.packageSize;
    }

    return DOWNLOADER_OK;
}



//--------------------------------------------------------------------------------------------------
// Internal functions
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Get download status
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_DwlResult_t downloader_GetDownloadStatus
(
    void
)
{
    lwm2mcore_DwlResult_t currentDownloadStatus;
    //LOCK(); //TODO
    currentDownloadStatus = DownloadStatus;
    //UNLOCK();

    return currentDownloadStatus;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get package size to be downloaded from the server
 *
 * @note
 * This function is not available if @c LWM2M_EXTERNAL_DOWNLOADER compilation flag is embedded
 *
 * @note
 * The client can call this function if it requested to know the package size before downloading it.
 *
 * @warning
 * This function is called in a dedicated thread/task.
 *
 * @return
 *  - @ref DOWNLOADER_OK on success
 *  - @ref DOWNLOADER_INVALID_ARG when the package URL is not valid
 *  - @ref DOWNLOADER_CONNECTION_ERROR when the host can not be reached
 *  - @ref DOWNLOADER_RECV_ERROR when error occurs on data receipt
 *  - @ref DOWNLOADER_ERROR on failure
 *  - @ref DOWNLOADER_TIMEOUT if any timer expires for a LwM2MCore called function
 *  - @ref DOWNLOADER_MEMORY_ERROR in case of memory allocation
 */
//--------------------------------------------------------------------------------------------------
downloaderResult_t downloader_GetPackageSize
(
    char*       packageUriPtr,      ///< [IN] Package URI
    uint64_t*   packageSizePtr      ///< [OUT] Package size
)
{
    if ((!packageUriPtr) || (!packageSizePtr))
    {
        return DOWNLOADER_INVALID_ARG;
    }

    if (!strlen(packageUriPtr))
    {
        LOG("Empty URL");
        return DOWNLOADER_INVALID_ARG;
    }

    if (LWM2MCORE_PACKAGE_URI_MAX_LEN < strlen(packageUriPtr))
    {
        LOG("Too long URL");
        return DOWNLOADER_INVALID_ARG;
    }

    SetDownloadStatus(DWL_OK);

    return SendRequest(HTTP_HEAD, packageUriPtr, START_OFFSET, packageSizePtr, NULL);
}

//--------------------------------------------------------------------------------------------------
/**
 * Start a package download in downloader
 *
 * This function is called in a dedicated thread/task.
 *
 * @return
 *  - @ref DOWNLOADER_OK on success
 *  - @ref DOWNLOADER_INVALID_ARG when the package URL is not valid
 *  - @ref DOWNLOADER_CONNECTION_ERROR when the host can not be reached
 *  - @ref DOWNLOADER_PARTIAL_FILE when partial file is received even if HTTP request succeeds
 *  - @ref DOWNLOADER_RECV_ERROR when error occurs on data receipt
 *  - @ref DOWNLOADER_ERROR on failure
 *  - @ref DOWNLOADER_TIMEOUT if any timer expires for a LwM2MCore called function
 *  - @ref DOWNLOADER_MEMORY_ERROR in case of memory allocation
 */
//--------------------------------------------------------------------------------------------------
downloaderResult_t downloader_StartDownload
(
    char*                   packageUriPtr,  ///< [IN] Package URI
    uint64_t                offset,         ///< [IN] Offset for the download
    void*                   opaquePtr       ///< [IN] Opaque pointer
)
{
    uint64_t packageSize = 0;

    if (!packageUriPtr)
    {
        LOG("URL NULL");
        return DOWNLOADER_INVALID_ARG;
    }

    if (!strlen(packageUriPtr))
    {
        LOG("Empty URL");
        return DOWNLOADER_INVALID_ARG;
    }

    if (LWM2MCORE_PACKAGE_URI_MAX_LEN < strlen(packageUriPtr))
    {
        LOG("Too long URL");
        return DOWNLOADER_INVALID_ARG;
    }

    SetDownloadStatus(DWL_OK);

    return SendRequest(HTTP_GET, packageUriPtr, offset, &packageSize, opaquePtr);
}

//--------------------------------------------------------------------------------------------------
/**
 * Abort current download
 */
//--------------------------------------------------------------------------------------------------
void downloader_AbortDownload
(
    void
)
{
    LOG_ARG("Abort download, download status was %d", downloader_GetDownloadStatus());

    /* Suspend ongoing download */
    SetDownloadStatus(DWL_ABORTED);
}

//--------------------------------------------------------------------------------------------------
/**
 * Supend current download
 */
//--------------------------------------------------------------------------------------------------
void downloader_SuspendDownload
(
    void
)
{
    LOG_ARG("Suspend download, download status was %d", downloader_GetDownloadStatus());

    /* Suspend ongoing download */
    SetDownloadStatus(DWL_SUSPEND);
}

//--------------------------------------------------------------------------------------------------
/**
 * Check if the current download should be aborted
 *
 * @return
 *      True    Download abort is requested
 *      False   Download can continue
 */
//--------------------------------------------------------------------------------------------------
bool downloader_CheckDownloadToAbort
(
    void
)
{
    if (DWL_ABORTED == downloader_GetDownloadStatus())
    {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/**
 * Check if the current download should be suspended
 *
 * @return
 *      True    Download suspend is requested
 *      False   Download can continue
 */
//--------------------------------------------------------------------------------------------------
bool downloader_CheckDownloadToSuspend
(
    void
)
{
    if (DWL_SUSPEND == downloader_GetDownloadStatus())
    {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to get the last HTTP(S) error code on a package download.
 *
 * @remark Public function which can be called by the client.
 *
 * @note
 * This function is not available if @c LWM2M_EXTERNAL_DOWNLOADER compilation flag is embedded
 *
 * @note
 * If a package download error happens, this funciton could be called in order to get the last
 * HTTP(S) error code related to the package download after package URI retrieval from the server.
 * This function only concerns the package download.
 * The value is not persitent to reset.
 * If no package download was made, the error code is set to 0.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK on success
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetLastHttpErrorCode
(
    uint16_t*   errorCode       ///< [IN] HTTP(S) error code
)
{
    if (!errorCode)
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    *errorCode = HttpErrorCode;
    return LWM2MCORE_ERR_COMPLETED_OK;
}


//--------------------------------------------------------------------------------------------------
/**
 * Get last downloader error
 *
 * This function is called in a dedicated thread/task.
 *
 * @note
 * This function is not available if @c LWM2M_EXTERNAL_DOWNLOADER compilation flag is embedded
 *
 * @note
 * This function is called when the downloader tries to download @c DWL_RETRIES time a package.
 *
 * @return
 *  - @ref DOWNLOADER_OK on success
 *  - @ref DOWNLOADER_INVALID_ARG when the package URL is not valid
 *  - @ref DOWNLOADER_CONNECTION_ERROR when the host can not be reached
 *  - @ref DOWNLOADER_PARTIAL_FILE when partial file is received even if HTTP request succeeds
 *  - @ref DOWNLOADER_RECV_ERROR when error occurs on data receipt
 *  - @ref DOWNLOADER_ERROR on failure
 *  - @ref DOWNLOADER_MEMORY_ERROR in case of memory allocation
 */
//--------------------------------------------------------------------------------------------------
downloaderResult_t downloader_GetLastDownloadError
(
    void
)
{
    return DOWNLOADER_OK;
}

#endif /* !LWM2M_EXTERNAL_DOWNLOADER */
