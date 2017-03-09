/**
 * @file lwm2mcorePackageDownloader.h
 *
 * LWM2M Package Downloader and DWL parser definitions
 *
 * <HR>
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#ifndef LWM2MCORE_PACKAGEDOWNLOADER_H
#define LWM2MCORE_PACKAGEDOWNLOADER_H

#include <stddef.h>
#include <stdint.h>
#include "osPortUpdate.h"

//--------------------------------------------------------------------------------------------------
// Symbol and Enum definitions
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Package downloader result codes
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    DWL_OK    = 0,  ///< Successful
    DWL_FAULT = -1  ///< Internal error
}
lwm2mcore_DwlResult_t;

//--------------------------------------------------------------------------------------------------
/**
 * Package downloader data structure
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    char     packageUri[LWM2MCORE_PACKAGE_URI_MAX_LEN]; ///< URI of package to download
    uint64_t packageSize;                               ///< Package size given by server
    lwm2mcore_updateType_t updateType;                  ///< FW or SW update
}
lwm2mcore_PackageDownloaderData_t;

//--------------------------------------------------------------------------------------------------
/**
 * Callback for package download initialization
 *
 * This callback should prepare the download of a package located at the given URI.
 * The context pointer will then be transmitted in each other callbacks.
 *
 * @return
 *  - DWL_OK    The function succeeded
 *  - DWL_FAULT The function failed
 *
 * @warning This callback should be set to NULL if not implemented
 */
//--------------------------------------------------------------------------------------------------
typedef lwm2mcore_DwlResult_t (*lwm2mcore_InitDownload_t)
(
    char* uriPtr,   ///< URI to use for the download
    void* ctxPtr    ///< Context pointer
);

//--------------------------------------------------------------------------------------------------
/**
 * Callback to get information about the package to download
 *
 * This callback should fill the available information about the package to download.
 * This is currently limited to the package size.
 *
 * @return
 *  - DWL_OK    The function succeeded
 *  - DWL_FAULT The function failed
 *
 * @warning This callback should be set to NULL if not implemented
 */
//--------------------------------------------------------------------------------------------------
typedef lwm2mcore_DwlResult_t (*lwm2mcore_GetPackageInfo_t)
(
    lwm2mcore_PackageDownloaderData_t* dataPtr, ///< Information about the package
    void* ctxPtr                                ///< Context pointer
);

//--------------------------------------------------------------------------------------------------
/**
 * Callback to set the firmware update state
 *
 * This callback should set the new given firmware update state.
 *
 * @return
 *  - DWL_OK    The function succeeded
 *  - DWL_FAULT The function failed
 *
 * @warning This callback should be set to NULL if not implemented
 */
//--------------------------------------------------------------------------------------------------
typedef lwm2mcore_DwlResult_t (*lwm2mcore_SetFwUpdateState_t)
(
    lwm2mcore_fwUpdateState_t updateState       ///< New update state
);

//--------------------------------------------------------------------------------------------------
/**
 * Callback to set the firmware update result
 *
 * This callback should set the new given firmware update result.
 *
 * @return
 *  - DWL_OK    The function succeeded
 *  - DWL_FAULT The function failed
 *
 * @warning This callback should be set to NULL if not implemented
 */
//--------------------------------------------------------------------------------------------------
typedef lwm2mcore_DwlResult_t (*lwm2mcore_SetFwUpdateResult_t)
(
    lwm2mcore_fwUpdateResult_t updateResult     ///< New update result
);

//--------------------------------------------------------------------------------------------------
/**
 * Callback to set the software update state
 *
 * This callback should set the new given software update state.
 *
 * @return
 *  - DWL_OK    The function succeeded
 *  - DWL_FAULT The function failed
 *
 * @warning This callback should be set to NULL if not implemented
 */
//--------------------------------------------------------------------------------------------------
typedef lwm2mcore_DwlResult_t (*lwm2mcore_SetSwUpdateState_t)
(
    lwm2mcore_swUpdateState_t updateState       ///< New update state
);

//--------------------------------------------------------------------------------------------------
/**
 * Callback to set the software update result
 *
 * This callback should set the new given software update result.
 *
 * @return
 *  - DWL_OK    The function succeeded
 *  - DWL_FAULT The function failed
 *
 * @warning This callback should be set to NULL if not implemented
 */
//--------------------------------------------------------------------------------------------------
typedef lwm2mcore_DwlResult_t (*lwm2mcore_SetSwUpdateResult_t)
(
    lwm2mcore_swUpdateResult_t updateResult     ///< New update result
);

//--------------------------------------------------------------------------------------------------
/**
 * Callback to download a package
 *
 * This callback should download the package, starting at offset startOffset.
 * Downloaded data should then be sequentially transmitted to the package downloader using
 * the lwm2mcore_PackageDownloaderReceiveData() function.
 *
 * @return
 *  - DWL_OK    The function succeeded
 *  - DWL_FAULT The function failed
 *
 * @warning This callback should be set to NULL if not implemented
 */
//--------------------------------------------------------------------------------------------------
typedef lwm2mcore_DwlResult_t (*lwm2mcore_Download_t)
(
    uint64_t startOffset,       ///< Offset indicating where to start the download
    void* ctxPtr                ///< Context pointer
);

//--------------------------------------------------------------------------------------------------
/**
 * Callback to store a package range after treatment
 *
 * This callback should store the data given in the buffer for a firmware update:
 * - bufSize indicates the length of the buffer to store
 * - offset indicates the offset where the data should be stored, starting from file beginning.
 *
 * @return
 *  - DWL_OK    The function succeeded
 *  - DWL_FAULT The function failed
 *
 * @warning This callback should be set to NULL if not implemented
 */
//--------------------------------------------------------------------------------------------------
typedef lwm2mcore_DwlResult_t (*lwm2mcore_StoreRange_t)
(
    uint8_t* bufPtr,        ///< Buffer of data to store
    size_t bufSize,         ///< Size of buffer to store
    uint64_t offset,        ///< Offset indicating where to start storing data
    void* ctxPtr            ///< Context pointer
);

//--------------------------------------------------------------------------------------------------
/**
 * Callback for package download end
 *
 * This callback should end the download and clean up what is necessary after a download.
 *
 * @return
 *  - DWL_OK    The function succeeded
 *  - DWL_FAULT The function failed
 *
 * @warning This callback should be set to NULL if not implemented
 */
//--------------------------------------------------------------------------------------------------
typedef lwm2mcore_DwlResult_t (*lwm2mcore_EndDownload_t)
(
    void* ctxPtr            ///< Context pointer
);

//--------------------------------------------------------------------------------------------------
// Data structures
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Package downloader structure
 *
 * @warning Unimplemented callbacks should be explicitly set to NULL
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    lwm2mcore_PackageDownloaderData_t data;                 ///< Package downloader data
    lwm2mcore_InitDownload_t          initDownload;         ///< Initialization callback
    lwm2mcore_GetPackageInfo_t        getInfo;              ///< Get package information
    lwm2mcore_SetFwUpdateState_t      setFwUpdateState;     ///< Set firmware update state
    lwm2mcore_SetFwUpdateResult_t     setFwUpdateResult;    ///< Set firmware update result
    lwm2mcore_SetSwUpdateState_t      setSwUpdateState;     ///< Set software update state
    lwm2mcore_SetSwUpdateResult_t     setSwUpdateResult;    ///< Set software update result
    lwm2mcore_Download_t              download;             ///< Download callback
    lwm2mcore_StoreRange_t            storeRange;           ///< Storing callback
    lwm2mcore_EndDownload_t           endDownload;          ///< Ending callback
    void*                             ctxPtr;               ///< Context pointer
}
lwm2mcore_PackageDownloader_t;

//--------------------------------------------------------------------------------------------------
// Public functions
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Run the package downloader.
 *
 * This function is called to launch the package downloader.
 *
 * @return
 *  - DWL_OK    The function succeeded
 *  - DWL_FAULT The function failed
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_DwlResult_t lwm2mcore_PackageDownloaderRun
(
    lwm2mcore_PackageDownloader_t* packageDownloaderPtr ///< Pointer on package downloader structure
);

//--------------------------------------------------------------------------------------------------
/**
 * Process the downloaded data.
 *
 * Downloaded data should be sequentially transmitted to the package downloader with this function.
 *
 * @return
 *  - DWL_OK    The function succeeded
 *  - DWL_FAULT The function failed
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_DwlResult_t lwm2mcore_PackageDownloaderReceiveData
(
    uint8_t* bufPtr,    ///< Received data
    size_t   bufSize    ///< Size of received data
);
#endif /* LWM2MCORE_PACKAGEDOWNLOADER_H */
