/**
 * @file lwm2mcorePackageDownloader.h
 *
 * LWM2M Package Downloader and DWL parser definitions
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#ifndef LWM2MCORE_PACKAGEDOWNLOADER_H
#define LWM2MCORE_PACKAGEDOWNLOADER_H

#include <stddef.h>
#include <stdint.h>
#include <lwm2mcore/lwm2mcore.h>
#include <lwm2mcore/update.h>

/**
  * @addtogroup lwm2mcore_package_downloader_IFS
  * @{
  */


//--------------------------------------------------------------------------------------------------
/**
 * @brief Package downloader result codes
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    DWL_OK              = 0,    ///< Successful
    DWL_FAULT           = 1,    ///< Internal error
    DWL_SUSPEND         = 2,    ///< Download suspended
    DWL_ABORTED         = 3,    ///< Download aborted
    DWL_MEM_ERROR       = 4,    ///< Memory allocation error (download is suspended)
    DWL_NETWORK_ERROR   = 5,    ///< Network error (download is suspended)
    DWL_BAD_ADDR        = 6,    ///< Incorrect URL or server can not be reached
    DWL_RETRY_FAILED    = 7,    ///< Download retry failed
}
lwm2mcore_DwlResult_t;

//--------------------------------------------------------------------------------------------------
// Data structures
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * @brief Package downloader data structure
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    uint64_t packageSize;                                   ///< Package size given by server
    lwm2mcore_UpdateType_t updateType;                      ///< FW or SW update
    bool     isResume;                                      ///< Is this a resume operation?
    uint64_t updateOffset;                                  ///< Update offset for download resume
}
lwm2mcore_PackageDownloaderData_t;

//--------------------------------------------------------------------------------------------------
/**
 * @brief Structure for package download connection
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    bool    isInitMade;     ///< true if the lwm2mcore_InitForDownload was called
    bool    isSecure;       ///< true for HTTPS, false for HTTP
}
lwm2mcore_PackageDownloadContext_t;

//--------------------------------------------------------------------------------------------------
// Data structures
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * @brief Package downloader structure
 *
 * @warning Unimplemented callbacks should be explicitly set to NULL
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    lwm2mcore_PackageDownloaderData_t data;                 ///< Package downloader data
    void*                             ctxPtr;               ///< Context pointer
}
lwm2mcore_PackageDownloader_t;

#ifndef LWM2M_EXTERNAL_DOWNLOADER

//--------------------------------------------------------------------------------------------------
// Public functions
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * @brief Process the downloaded data.
 *
 * Downloaded data should be sequentially transmitted to the package downloader with this function.
 *
 * @remark Public function which can be called by the client.
 *
 * @note
 * This function is not available if @c LWM2M_EXTERNAL_DOWNLOADER compilation flag is embedded
 *
 * @return
 *  - @ref DWL_OK    The function succeeded
 *  - @ref DWL_FAULT The function failed
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_DwlResult_t lwm2mcore_PackageDownloaderReceiveData
(
    uint8_t*    bufPtr,     ///< [IN] Received data
    size_t      bufSize,    ///< [IN] Size of received data
    void*       opaquePtr   ///< [IN] Opaque pointer
);

//--------------------------------------------------------------------------------------------------
/**
 * Request a download retry.
 *
 * @remark Public function which can be called by the client.
 *
 * @note
 * This function is not available if @c LWM2M_EXTERNAL_DOWNLOADER compilation flag is embedded
 *
 * @return
 *  - LWM2MCORE_ERR_COMPLETED_OK on success
 *  - LWM2MCORE_ERR_GENERAL_ERROR if unable to request a retry
 *  - LWM2MCORE_ERR_RETRY_FAILED if retry attempt failed
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_RequestDownloadRetry
(
    void
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Initialize the package downloader.
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * This function is called to initialize the package downloader: the associated workspace is
 * deleted if necessary to be able to start a new download.
 *
 * @remark Public function which can be called by the client.
 *
 * @note
 * This function is not available if @c LWM2M_EXTERNAL_DOWNLOADER compilation flag is embedded
 *
 */
//--------------------------------------------------------------------------------------------------
void lwm2mcore_PackageDownloaderInit
(
    void
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Delete the package downloader resume info.
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * This function is called to delete resume related information from the package downloader
 * workspace.
 *
 * @remark Public function which can be called by the client.
 *
 * @note
 * This function is not available if @c LWM2M_EXTERNAL_DOWNLOADER compilation flag is embedded
 *
 */
//--------------------------------------------------------------------------------------------------
void lwm2mcore_DeletePackageDownloaderResumeInfo
(
    void
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to initialize a package download
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @note The returned pointer needs to be deallocated on client side
 * This function is not available if @c LWM2M_EXTERNAL_DOWNLOADER compilation flag is embedded
 *
 * @return
 *  - Package download context
 *  - @c NULL on failure
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_PackageDownloadContext_t* lwm2mcore_InitForDownload
(
    bool    isHttps     ///< [IN] true if HTTPS is requested, else HTTP is requested
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to free the connection for package download
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @note
 * This function is not available if @c LWM2M_EXTERNAL_DOWNLOADER compilation flag is embedded
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK on success
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if the parameter is invalid
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR on failure
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_FreeForDownload
(
    lwm2mcore_PackageDownloadContext_t* contextPtr  ///< [IN] Package donwload context
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to initiate the connection for package download
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @note
 * This function is not available if @c LWM2M_EXTERNAL_DOWNLOADER compilation flag is embedded
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK on success
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if the parameter is invalid
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR on failure
 *  - @ref LWM2MCORE_ERR_NET_RECV_FAILED if reading information from the socket failed
 *  - @ref LWM2MCORE_ERR_NET_SEND_FAILED if sending information through the socket failed
 *  - @ref LWM2MCORE_ERR_MEMORY on memory allocation issue
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_ConnectForDownload
(
    lwm2mcore_PackageDownloadContext_t* contextPtr, ///< [IN] Package donwload context
    char*                               hostPtr,    ///< [IN] Host to connect on
    uint16_t                            port        ///< [IN] Port to connect on
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to read received for package download
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @note
 * This function is not available if @c LWM2M_EXTERNAL_DOWNLOADER compilation flag is embedded
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK on success
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if the request is invalid
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR on failure
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_ReadForDownload
(
    lwm2mcore_PackageDownloadContext_t* contextPtr,     ///< [IN] Package donwload context
    char*                               bufferPtr,      ///< [INOUT] Buffer
    int*                                lenPtr          ///< [IN] Buffer length
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to send a HTTP request for package download
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @note
 * This function is not available if @c LWM2M_EXTERNAL_DOWNLOADER compilation flag is embedded
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK on success
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if the request is invalid
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR on failure
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_SendForDownload
(
    lwm2mcore_PackageDownloadContext_t* contextPtr,         ///< [IN] Package donwload context
    char*                               serverRequestPtr    ///< [IN] HTTP(S) request
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Store downloaded data for update package
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @note
 * This function is not available if @c LWM2M_EXTERNAL_DOWNLOADER compilation flag is embedded
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK on success
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if the parameter is invalid
 *  - @ref LWM2MCORE_ERR_INVALID_STATE if package download is suspended
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR on failure
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_WritePackageData
(
    uint8_t* bufferPtr,     ///< [IN] Data to be written
    uint32_t length,        ///< [IN] Data length
    void*    opaquePtr      ///< [IN] Opaque pointer
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to disconnect the connection for package download
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @note
 * This function is not available if @c LWM2M_EXTERNAL_DOWNLOADER compilation flag is embedded
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK on success
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if the parameter is invalid
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR on failure
 *  - @ref LWM2MCORE_ERR_INVALID_STATE if no connection was initiated for package download
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_DisconnectForDownload
(
    lwm2mcore_PackageDownloadContext_t* contextPtr  ///< [IN] Package donwload context
);

//--------------------------------------------------------------------------------------------------
/**
 * Start package downloading
 *
 * @note If the returned error is @ref LWM2MCORE_ERR_NET_ERROR, the download is suspended and can be
 * resumed
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK on success
 *  - @ref LWM2MCORE_ERR_INVALID_ARG when the package URL is not valid
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR on failure
 *  - @ref LWM2MCORE_ERR_MEMORY on memory allocation issue
 *  - @ref LWM2MCORE_ERR_NET_ERROR on network issue (socket)
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_StartPackageDownloader
(
    void*   ctxPtr      ///< [IN] Context pointer
);

//--------------------------------------------------------------------------------------------------
/**
 * Handle package download state machine
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK on success
 *  - @ref LWM2MCORE_ERR_INVALID_ARG when the package URL is not valid
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR on failure
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_HandlePackageDownloader
(
    void
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Resume a package download
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 * The platform needs to launch a decicated thread/task and call @ref
 * lwm2mcore_StartPackageDownloader (updateType, true)
 *
 * @note
 * This function is not available if @c LWM2M_EXTERNAL_DOWNLOADER compilation flag is embedded
 *
 */
//--------------------------------------------------------------------------------------------------
void lwm2mcore_ResumePackageDownloader
(
    lwm2mcore_UpdateType_t updateType   ///< [IN] Update type (FW/SW)
);

//--------------------------------------------------------------------------------------------------
/**
 * Function to suspend a download
 *
 * @note
 * This function could be called by the client in order to abort a download if any issue happens
 * on client side.
 *
 * @warning
 * This function is called in a dedicated thread/task.
 *
 * @return
 *  - LWM2MCORE_ERR_COMPLETED_OK on success
 *  - LWM2MCORE_ERR_INVALID_STATE if no package download is on-going
 *  - LWM2MCORE_ERR_GENERAL_ERROR on failure
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_SuspendDownload
(
    void
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to abort a download
 *
 * @remark Public function which can be called by the client.
 *
 * @note
 * This function could be called by the client in order to abort a download if any issue happens
 * on client side.
 *
 * @note
 * This function is not available if @c LWM2M_EXTERNAL_DOWNLOADER compilation flag is embedded
 *
 * @warning
 * This function is called in a dedicated thread/task.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK on success
 *  - @ref LWM2MCORE_ERR_INVALID_STATE if no package download is on-going
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR on failure
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_AbortDownload
(
    void
);


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
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK on success
 *  - @ref LWM2MCORE_ERR_INVALID_STATE if no package download is on-going
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR on failure
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetPackageOffsetStorage
(
    lwm2mcore_UpdateType_t  updateType,     ///< [IN] Update type
    uint64_t*               offsetPtr       ///< [IN] Package offset
);

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
);

#endif /* !LWM2M_EXTERNAL_DOWNLOADER */

/**
  * @}
  */

#endif /* LWM2MCORE_PACKAGEDOWNLOADER_H */
