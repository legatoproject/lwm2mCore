/**
 * @file downloader.h
 *
 * LWM2M Package Downloader and DWL parser definitions
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include <stddef.h>
#include <stdint.h>
#include <lwm2mcore/lwm2mcore.h>
#include <lwm2mcore/lwm2mcorePackageDownloader.h>
#include <lwm2mcore/update.h>

#ifndef LWM2M_EXTERNAL_DOWNLOADER

//--------------------------------------------------------------------------------------------------
// Symbol and Enum definitions
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Start offset to begin a new download
 */
//--------------------------------------------------------------------------------------------------
#define START_OFFSET 0

//--------------------------------------------------------------------------------------------------
/**
 * Define HTTP error code: 401 Unauthorized
 */
//--------------------------------------------------------------------------------------------------
#define HTTP_401    401

//--------------------------------------------------------------------------------------------------
/**
 * Define HTTP error code: 403 Forbidden
 */
//--------------------------------------------------------------------------------------------------
#define HTTP_403    403

//--------------------------------------------------------------------------------------------------
/**
 * Define HTTP error code: 404 Not found
 */
//--------------------------------------------------------------------------------------------------
#define HTTP_404    404

//--------------------------------------------------------------------------------------------------
/**
 * Define HTTP error code: 414  URI Too Long
 */
//--------------------------------------------------------------------------------------------------
#define HTTP_414    414

//--------------------------------------------------------------------------------------------------
/**
 * Define HTTP error code: 500 Internal Server Error
 */
//--------------------------------------------------------------------------------------------------
#define HTTP_500    500

//--------------------------------------------------------------------------------------------------
/**
 * Define HTTP error code: 599 Last error code 5xx
 */
//--------------------------------------------------------------------------------------------------
#define HTTP_599    599

//--------------------------------------------------------------------------------------------------
/**
 * Define HTTP error code: 200 OK
 */
//--------------------------------------------------------------------------------------------------
#define HTTP_200    200

//--------------------------------------------------------------------------------------------------
/**
 * Define HTTP error code: 206 Partial Content
 */
//--------------------------------------------------------------------------------------------------
#define HTTP_206    206

//--------------------------------------------------------------------------------------------------
/**
 * Define value for download retry maximum number
 */
//--------------------------------------------------------------------------------------------------
#define DWL_RETRIES 5

//--------------------------------------------------------------------------------------------------
/**
 * Downloader error code enumeration
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    DOWNLOADER_OK,                ///< Command success
    DOWNLOADER_INVALID_ARG,       ///< Invalid arguments
    DOWNLOADER_CONNECTION_ERROR,  ///< Error on connection
    DOWNLOADER_PARTIAL_FILE,      ///< Command success but partial file was received
    DOWNLOADER_RECV_ERROR,        ///< Error on receiving data
    DOWNLOADER_SEND_ERROR,        ///< Error on sending data
    DOWNLOADER_ERROR,             ///< Command failure
    DOWNLOADER_TIMEOUT,           ///< Command success but not data read during the dedicated time
    DOWNLOADER_MEMORY_ERROR,      ///< Memory allocation issue
    DOWNLOADER_CERTIF_ERROR       ///< Certificate failure
}
downloaderResult_t;

//--------------------------------------------------------------------------------------------------
// Function definitions
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Start a package download in downloader
 *
 * This function is called in a dedicated thread/task.
 *
 * @note
 * This function is not available if @c LWM2M_EXTERNAL_DOWNLOADER compilation flag is embedded
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
downloaderResult_t downloader_StartDownload
(
    char*                   packageUriPtr,  ///< [IN] Package URI
    uint64_t                offset,         ///< [IN] Offset for the download
    void*                   opaquePtr       ///< [IN] Opaque pointer
);

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
);

//--------------------------------------------------------------------------------------------------
/**
 * Check if the current download should be aborted
 *
 * @note
 * This function is not available if @c LWM2M_EXTERNAL_DOWNLOADER compilation flag is embedded
 *
 * @return
 *  - @c true    Download abort is requested
 *  - @c false   Download can continue
 */
//--------------------------------------------------------------------------------------------------
bool downloader_CheckDownloadToAbort
(
    void
);

//--------------------------------------------------------------------------------------------------
/**
 * Check if the current download should be suspended
 *
 * @note
 * This function is not available if @c LWM2M_EXTERNAL_DOWNLOADER compilation flag is embedded
 *
 * @return
 *  - @c true    Download suspend is requested
 *  - @c false   Download can continue
 */
//--------------------------------------------------------------------------------------------------
bool downloader_CheckDownloadToSuspend
(
    void
);

//--------------------------------------------------------------------------------------------------
/**
 * Get download status
 *
 * @note
 * This function is not available if @c LWM2M_EXTERNAL_DOWNLOADER compilation flag is embedded
 *
 * @return
    - @ref DWL_OK on success
    - @ref DWL_FAULT on failure
    - @ref DWL_SUSPEND if download suspended
    - @ref DWL_ABORTED if download aborted
    - @ref DWL_MEM_ERROR on memory allocation error (download is suspended)
    - @ref DWL_NETWORK_ERROR on network error (download is suspended)
    - @ref DWL_BAD_ADDR if incorrect URL or server can not be reached
    - @ref DWL_RETRY_FAILED on download retry failure
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_DwlResult_t downloader_GetDownloadStatus
(
    void
);

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
 *  - @ref DOWNLOADER_MEMORY_ERROR in case of memory allocation
 */
//--------------------------------------------------------------------------------------------------
downloaderResult_t downloader_GetPackageSize
(
    char*                   packageUriPtr,      ///< [IN] Package URI
    uint64_t*               packageSizePtr      ///< [OUT] Package size
);

//--------------------------------------------------------------------------------------------------
/**
 * Abort current download
 */
//--------------------------------------------------------------------------------------------------
void downloader_AbortDownload
(
    void
);

//--------------------------------------------------------------------------------------------------
/**
 * Supend current download
 */
//--------------------------------------------------------------------------------------------------
void downloader_SuspendDownload
(
    void
);

//--------------------------------------------------------------------------------------------------
/**
 *  Indicate package update has started.
 */
//--------------------------------------------------------------------------------------------------
void downloader_PackageUpdateStarted
(
    void
);

#endif /* !LWM2M_EXTERNAL_DOWNLOADER */

#endif /* DOWNLOADER_H */
