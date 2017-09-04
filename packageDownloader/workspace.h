/**
 * @file workspace.h
 *
 * Header for the LWM2M Core package downloader workspace
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#ifndef __WORKSPACE_H__
#define __WORKSPACE_H__

#include <lwm2mcore/lwm2mcore.h>
#include <lwm2mcore/paramStorage.h>
#include "lwm2mcorePackageDownloader.h"

//--------------------------------------------------------------------------------------------------
// Symbol and Enum definitions
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Supported version for package downloader workspace
 */
//--------------------------------------------------------------------------------------------------
#define PKGDWL_WORKSPACE_VERSION    1

//--------------------------------------------------------------------------------------------------
/**
 * Maximal size of the SHA-1 context
 */
//--------------------------------------------------------------------------------------------------
#define SHA1_CTX_MAX_SIZE   512

//--------------------------------------------------------------------------------------------------
// Data structures
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Package downloader workspace structure
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    uint8_t  version;                       ///< Workspace version
    uint64_t offset;                        ///< Current package offset
    uint32_t section;                       ///< DWL section
    uint8_t  subsection;                    ///< DWL subsection
    uint32_t packageCRC;                    ///< Package CRC read in first DWL prolog
    uint64_t commentSize;                   ///< Comments size read in DWL prolog
    uint64_t binarySize;                    ///< Binary package size read in DWL prolog
    uint64_t paddingSize;                   ///< Binary padding size read in DWL prolog
    uint64_t remainingBinaryData;           ///< Remaining length of binary data to download
    uint64_t signatureSize;                 ///< Signature size read in DWL prolog
    uint32_t computedCRC;                   ///< CRC computed with downloaded data
    uint8_t  sha1Ctx[SHA1_CTX_MAX_SIZE];    ///< SHA-1 context
}
PackageDownloaderWorkspace_t;

//--------------------------------------------------------------------------------------------------
// Public functions
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Function to read the package downloader workspace from platform memory
 *
 * @return
 *  - DWL_OK    The function succeeded
 *  - DWL_FAULT The function failed
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_DwlResult_t ReadPkgDwlWorkspace
(
    PackageDownloaderWorkspace_t* pkgDwlWorkspacePtr    ///< Package downloader workspace
);

//--------------------------------------------------------------------------------------------------
/**
 * Function to write the package downloader workspace in platform memory
 *
 * @return
 *  - DWL_OK    The function succeeded
 *  - DWL_FAULT The function failed
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_DwlResult_t WritePkgDwlWorkspace
(
    PackageDownloaderWorkspace_t* pkgDwlWorkspacePtr    ///< Package downloader workspace
);

//--------------------------------------------------------------------------------------------------
/**
 * Function to delete the package downloader workspace in platform memory
 *
 * @return
 *  - DWL_OK    The function succeeded
 *  - DWL_FAULT The function failed
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_DwlResult_t DeletePkgDwlWorkspace
(
    void
);

#endif /* __WORKSPACE_H__ */
