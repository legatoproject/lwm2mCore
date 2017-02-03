/**
 * @file lwm2mcorePackageDownloader.c
 *
 * LWM2M Package Downloader and DWL parser
 *
 * A simple DWL package is composed of the following sections:
 * - UPCK (Update Package): general information about the DWL package
 * - BINA (Binary): binary data used to update the software
 * - SIGN (Signature): signature of the package
 *
 * Each DWL section starts with a DWL prolog containing information about the section
 * (e.g. type, size...). Depending on the section type, it is followed by several subsections:
 * - UPCK (Update Package):
 *      - DWL comments: optional subsection containing comments about the package
 *      - UPCK header: general information about the Update Package, e.g. update type
 * - BINA (Binary):
 *      - DWL comments: optional subsection containing comments about the package
 *      - BINA header: general information about the Binary data, e.g. destination baseband
 *      - Binary data: useful binary data for the update
 *      - Padding data
 * - SIGN (Signature):
 *      - DWL comments: optional subsection containing comments about the package
 *      - Signature: package signature
 *
 * The package CRC is retrieved in the first DWL prolog. A CRC is then computed with all data
 * from the package, starting from the first byte after the package CRC until the end of the file.
 * The CRC is computed using the crc32 function from zlib.
 *
 * @Note The zlib library should be present on your platform to use the package downloader.
 *
 * <HR>
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#include <stdio.h>
#include <liblwm2m.h>
#include <string.h>
#include <zlib.h>
#include <lwm2mcore.h>
#include <internals.h>
#include "lwm2mcorePackageDownloader.h"
#include "sessionManager.h"

//--------------------------------------------------------------------------------------------------
// Symbol and Enum definitions
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Maximal length of the error string
 */
//--------------------------------------------------------------------------------------------------
#define ERROR_STR_MAX_LEN   50

//--------------------------------------------------------------------------------------------------
/**
 * Magic number identifying a DWL prolog
 */
//--------------------------------------------------------------------------------------------------
#define DWL_MAGIC_NUMBER  0x464c5744  ///< DWLF

//--------------------------------------------------------------------------------------------------
/**
 * Possible types of DWL sections
 */
//--------------------------------------------------------------------------------------------------
#define DWL_TYPE_UPCK     0x4b435055  ///< UpdatePackage
#define DWL_TYPE_SIGN     0x4e474953  ///< Signature
#define DWL_TYPE_BINA     0x414e4942  ///< Binary
#define DWL_TYPE_COMP     0x504d4f43  ///< CompBinary
#define DWL_TYPE_XDWL     0x4c574458  ///< Downloader
#define DWL_TYPE_E2PR     0x52503245  ///< EEPROM
#define DWL_TYPE_DIFF     0x46464944  ///< Patch
#define DWL_TYPE_DOTA     0x41544f44  ///< DotaCell
#define DWL_TYPE_RAM_     0x5f4d4152  ///< Ram
#define DWL_TYPE_BOOT     0x544f4f42  ///< Bootstrap

//--------------------------------------------------------------------------------------------------
/**
 * Length of different DWL section headers
 */
//--------------------------------------------------------------------------------------------------
#define LWM2MCORE_UPCK_HEADER_SIZE  128         ///< Size of the Update Package header
#define LWM2MCORE_BINA_HEADER_SIZE  128         ///< Size of the Binary header
#define LWM2MCORE_COMP_HEADER_SIZE  128         ///< Size of the Compressed Binary header
#define LWM2MCORE_XDWL_HEADER_SIZE  128         ///< Size of the X-modem downloader binary header
#define LWM2MCORE_E2PR_HEADER_SIZE  32          ///< Size of the EEPROM binary header

//--------------------------------------------------------------------------------------------------
/**
 * Possible types of DWL subsections
 * Each DWL section is composed of one or more subsections
 */
//--------------------------------------------------------------------------------------------------
#define DWL_SUB_PROLOG        0x00    ///< DLW prolog
#define DWL_SUB_COMMENTS      0x01    ///< DLW comments
#define DWL_SUB_HEADER        0x02    ///< DLW header
#define DWL_SUB_BINARY        0x03    ///< DLW binary data
#define DWL_SUB_PADDING       0x04    ///< DLW padding data
#define DWL_SUB_SIGNATURE     0x05    ///< DLW signature

//--------------------------------------------------------------------------------------------------
/**
 * Possible types of Update Package
 */
//--------------------------------------------------------------------------------------------------
#define LWM2MCORE_UPCK_TYPE_FW      0x00000001
#define LWM2MCORE_UPCK_TYPE_OAT     0x00000002
#define LWM2MCORE_UPCK_TYPE_AMSS    0x00000003
#define LWM2MCORE_UPCK_TYPE_HYPER   0x00000004
#define LWM2MCORE_UPCK_TYPE_BOOT    0x00000005

//--------------------------------------------------------------------------------------------------
/**
 * Package downloader states
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    PKG_DWL_INIT,       ///< Package downloader initialization
    PKG_DWL_INFO,       ///< Retrieve information about the package
    PKG_DWL_DOWNLOAD,   ///< Download file chunk
    PKG_DWL_PARSE,      ///< Parse downloaded file chunk
    PKG_DWL_STORE,      ///< Store downloaded file chunk
    PKG_DWL_END,        ///< Download closing and clean up
    PKG_DWL_ERROR       ///< Package downloader error
}
PackageDownloaderState_t;

//--------------------------------------------------------------------------------------------------
/**
 * Event types for the package downloader
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    PKG_DWL_EVENT_DETAILS,          ///< Package details (size)
    PKG_DWL_EVENT_DL_START,         ///< Package download start
    PKG_DWL_EVENT_DL_PROGRESS,      ///< Package download progress
    PKG_DWL_EVENT_DL_END,           ///< Package download end (success or failure)
    PKG_DWL_EVENT_SIGN_OK,          ///< Package signature check is OK
    PKG_DWL_EVENT_SIGN_KO,          ///< Package signature is KO
    PKG_DWL_EVENT_UPDATE_START,     ///< Package update is launched
    PKG_DWL_EVENT_UPDATE_FAILURE,   ///< Package update fails
    PKG_DWL_EVENT_UPDATE_SUCCESS    ///< Package update succeeds
}
PackageDownloaderEvent_t;

//--------------------------------------------------------------------------------------------------
// Data structures
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Package downloader object structure
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    PackageDownloaderState_t state;                 ///< State of package downloader state machine
    bool                     endOfProcessing;       ///< End of package processing
    lwm2mcore_DwlResult_t    result;                ///< Current result of package downloader
    lwm2mcore_fwUpdateResult_t updateResult;        ///< Current package update result
    bool                     firstDownload;         ///< Indicates if next download is the first one
    lwm2mcore_PkgDwlType_t   packageType;           ///< Package type (FW or SW)
    uint64_t                 offset;                ///< Current offset in the package
    size_t                   lenToDownload;         ///< Length of next buffer to download
    uint8_t dwlData[MAX_DATA_BUFFER_CHUNK];         ///< Buffer of downloaded data
    size_t                   downloadedLen;         ///< Length of data really downloaded
    double                   downloadProgress;      ///< Overall download progress
    uint64_t                 storageOffset;         ///< Current offset in data storage
}
PackageDownloaderObj_t;

//--------------------------------------------------------------------------------------------------
/**
 * DWL parser object structure
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    uint32_t section;               ///< Current DWL section
    uint8_t  subsection;            ///< Current DWL subsection
    uint32_t packageCRC;            ///< Package CRC read in first DWL prolog
    uint32_t computedCRC;           ///< CRC computed with downloaded data
    uint64_t commentSize;           ///< Comments size read in DWL prolog
    uint64_t binarySize;            ///< Binary package size read in DWL prolog
    uint64_t paddingSize;           ///< Binary padding size read in DWL prolog
    uint64_t remainingBinaryData;   ///< Remaining length of binary data to download
    uint64_t signatureSize;         ///< Signature size read in DWL prolog
}
DwlParserObj_t;

//--------------------------------------------------------------------------------------------------
/**
 * DWL package prolog structure
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
  uint32_t magicNumber;     ///< Constant ID tag
  uint32_t statusBitfield;  ///< Status bit field that can be set in place (0xFFFFFFFF by default)
  uint32_t crc32;           ///< CRC32 of the following data (from fileSize to the end of the file)
  uint32_t fileSize;        ///< Total file size counting number of bytes (not counting padding)
  uint64_t timeStamp;       ///< Time stamp (in BCD format)
  uint32_t dataType;        ///< Data type of the DWL file
  uint16_t typeVersion;     ///< Version of this data type
  uint16_t commentSize;     ///< Size of comment part
}
DwlProlog_t;

//--------------------------------------------------------------------------------------------------
/**
 * UPCK header structure
 */
//--------------------------------------------------------------------------------------------------
typedef union
{
    struct
    {
        uint32_t upckType;                          ///< Update Package type
        uint32_t srcCks;                            ///< Source checksum
        uint32_t dstCks;                            ///< Destination checksum
        uint32_t dstBaseAddress;                    ///< Destination base address
    } structHeader;
    uint8_t rawHeader[LWM2MCORE_UPCK_HEADER_SIZE];  ///< Raw UPCK header
}
UpckHeader_t;

//--------------------------------------------------------------------------------------------------
// Static functions
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Function to notify package downloader events
 */
//--------------------------------------------------------------------------------------------------
static void PkgDwlEvent
(
    PackageDownloaderEvent_t eventId,           ///< Package downloader event
    lwm2mcore_PackageDownloader_t* pkgDwlPtr,   ///< Package downloader
    PackageDownloaderObj_t* pkgDwlObjPtr        ///< Package downloader object
)
{
    lwm2mcore_status_t status;

    // Create the status event
    switch (eventId)
    {
        case PKG_DWL_EVENT_DETAILS:
            LOG_ARG("Package download size: %llu bytes", pkgDwlPtr->data.packageSize);
            status.event = LWM2MCORE_EVENT_PACKAGE_DOWNLOAD_DETAILS;
            status.u.pkgStatus.pkgType = pkgDwlObjPtr->packageType;
            status.u.pkgStatus.numBytes = (uint32_t)pkgDwlPtr->data.packageSize;
            status.u.pkgStatus.progress = 0;
            status.u.pkgStatus.errorCode = 0;

            // TODO Check that the device has sufficient space to store the package
            // Need a new porting layer function
            break;

        case PKG_DWL_EVENT_DL_START:
            LOG("Package download start");
            status.event = LWM2MCORE_EVENT_DOWNLOAD_PROGRESS;
            status.u.pkgStatus.pkgType = pkgDwlObjPtr->packageType;
            status.u.pkgStatus.numBytes = 0;
            status.u.pkgStatus.progress = 0;
            status.u.pkgStatus.errorCode = 0;
            break;

        case PKG_DWL_EVENT_DL_PROGRESS:
            LOG_ARG("Package download progress: %llu bytes, %.2f%%",
                    pkgDwlObjPtr->offset, pkgDwlObjPtr->downloadProgress);

            if (   (100 < pkgDwlObjPtr->downloadProgress)
                || (pkgDwlPtr->data.packageSize < pkgDwlObjPtr->offset)
               )
            {
                // Incoherent download progress
                return;
            }

            status.event = LWM2MCORE_EVENT_DOWNLOAD_PROGRESS;
            status.u.pkgStatus.pkgType = pkgDwlObjPtr->packageType;
            status.u.pkgStatus.numBytes = (uint32_t)pkgDwlObjPtr->offset;
            status.u.pkgStatus.progress = (uint32_t)pkgDwlObjPtr->downloadProgress;
            status.u.pkgStatus.errorCode = 0;
            break;

        case PKG_DWL_EVENT_DL_END:
            // Determine download status with update result
            switch (pkgDwlObjPtr->updateResult)
            {
                case LWM2MCORE_FW_UPDATE_RESULT_DEFAULT_NORMAL:
                    status.event = LWM2MCORE_EVENT_PACKAGE_DOWNLOAD_FINISHED;
                    status.u.pkgStatus.errorCode = 0;
                    break;

                case LWM2MCORE_FW_UPDATE_RESULT_NO_STORAGE_SPACE:
                case LWM2MCORE_FW_UPDATE_RESULT_OUT_OF_MEMORY:
                    status.event = LWM2MCORE_EVENT_PACKAGE_DOWNLOAD_FAILED;
                    status.u.pkgStatus.errorCode = LWM2MCORE_FUMO_NO_SUFFICIENT_MEMORY;
                    break;

                case LWM2MCORE_FW_UPDATE_RESULT_VERIFY_ERROR:
                    status.event = LWM2MCORE_EVENT_PACKAGE_DOWNLOAD_FAILED;
                    status.u.pkgStatus.errorCode = LWM2MCORE_FUMO_FAILED_VALIDATION;
                    break;

                case LWM2MCORE_FW_UPDATE_RESULT_UNSUPPORTED_PKG_TYPE:
                    status.event = LWM2MCORE_EVENT_PACKAGE_DOWNLOAD_FAILED;
                    status.u.pkgStatus.errorCode = LWM2MCORE_FUMO_UNSUPPORTED_PKG;
                    break;

                case LWM2MCORE_FW_UPDATE_RESULT_INVALID_URI:
                    status.event = LWM2MCORE_EVENT_PACKAGE_DOWNLOAD_FAILED;
                    status.u.pkgStatus.errorCode = LWM2MCORE_FUMO_INVALID_URI;
                    break;

                case LWM2MCORE_FW_UPDATE_RESULT_COMMUNICATION_ERROR:
                case LWM2MCORE_FW_UPDATE_RESULT_UNSUPPORTED_PROTOCOL:
                    status.event = LWM2MCORE_EVENT_PACKAGE_DOWNLOAD_FAILED;
                    status.u.pkgStatus.errorCode = LWM2MCORE_FUMO_ALTERNATE_DL_ERROR;
                    break;

                default:
                    LOG_ARG("Unknown update result %d", pkgDwlObjPtr->updateResult);
                    status.event = LWM2MCORE_EVENT_PACKAGE_DOWNLOAD_FAILED;
                    status.u.pkgStatus.errorCode = LWM2MCORE_FUMO_ALTERNATE_DL_ERROR;
                    break;
            }
            status.u.pkgStatus.pkgType = pkgDwlObjPtr->packageType;
            status.u.pkgStatus.numBytes = (uint32_t)pkgDwlObjPtr->offset;
            status.u.pkgStatus.progress = (uint32_t)pkgDwlObjPtr->downloadProgress;

            LOG_ARG("Package download end: event %d, errorCode %d",
                    status.event, status.u.pkgStatus.errorCode);
            break;

        case PKG_DWL_EVENT_SIGN_OK:
            LOG("Signature check successful");
            status.event = LWM2MCORE_EVENT_PACKAGE_CERTIFICATION_OK;
            status.u.pkgStatus.pkgType = pkgDwlObjPtr->packageType;
            break;

        case PKG_DWL_EVENT_SIGN_KO:
            LOG("Signature check failed");
            status.event = LWM2MCORE_EVENT_PACKAGE_CERTIFICATION_NOT_OK;
            status.u.pkgStatus.pkgType = pkgDwlObjPtr->packageType;
            break;

        case PKG_DWL_EVENT_UPDATE_START:
            LOG("Package update is launched");
            status.event = LWM2MCORE_EVENT_UPDATE_STARTED;
            status.u.pkgStatus.pkgType = pkgDwlObjPtr->packageType;
            break;

        case PKG_DWL_EVENT_UPDATE_SUCCESS:
            LOG("Package update successful");
            status.event = LWM2MCORE_EVENT_UPDATE_FINISHED;
            status.u.pkgStatus.pkgType = pkgDwlObjPtr->packageType;
            break;

        case PKG_DWL_EVENT_UPDATE_FAILURE:
            LOG("Package update failed");
            status.event = LWM2MCORE_EVENT_UPDATE_FAILED;
            status.u.pkgStatus.pkgType = pkgDwlObjPtr->packageType;
            break;

        default:
            LOG_ARG("Unknown eventId %d", eventId);
            return;
    }

    // Send the status event
    SendStatusEvent(status);
}

//--------------------------------------------------------------------------------------------------
/**
 * Parse DWL prolog containing information about the next DWL section: type, size, CRC, comments...
 * See @ref DwlProlog_t structure for further details.
 *
 * @return
 *  - DWL_OK      The function succeeded
 *  - DWL_FAULT   The function failed
 */
//--------------------------------------------------------------------------------------------------
static lwm2mcore_DwlResult_t ParseDwlProlog
(
    PackageDownloaderObj_t* pkgDwlObjPtr,   ///< Package downloader object
    DwlParserObj_t* dwlParserObjPtr         ///< DWL parser object
)
{
    lwm2mcore_DwlResult_t result = DWL_OK;
    DwlProlog_t* dwlPrologPtr;

    // Check DWL prolog size
    if (pkgDwlObjPtr->downloadedLen < sizeof(DwlProlog_t))
    {
        LOG_ARG("DWL prolog is too short, %u < %u",
                pkgDwlObjPtr->downloadedLen, sizeof(DwlProlog_t));
        pkgDwlObjPtr->updateResult = LWM2MCORE_FW_UPDATE_RESULT_UNSUPPORTED_PKG_TYPE;
        return DWL_FAULT;
    }

    dwlPrologPtr = (DwlProlog_t*)pkgDwlObjPtr->dwlData;

    // Check DWL magic number
    if (DWL_MAGIC_NUMBER != dwlPrologPtr->magicNumber)
    {
        LOG_ARG("Unknown package format, magic number 0x%08x", dwlPrologPtr->magicNumber);
        pkgDwlObjPtr->updateResult = LWM2MCORE_FW_UPDATE_RESULT_UNSUPPORTED_PKG_TYPE;
        return DWL_FAULT;
    }

    // Store current DWL section
    dwlParserObjPtr->section = dwlPrologPtr->dataType;
    LOG_ARG("Parse new DWL section 0x%08x", dwlParserObjPtr->section);

    // Store necessary data and determine next awaited subsection
    switch (dwlParserObjPtr->section)
    {
        case DWL_TYPE_UPCK:
            dwlParserObjPtr->commentSize = (dwlPrologPtr->commentSize << 3);

            // Download DWL comments
            pkgDwlObjPtr->state = PKG_DWL_DOWNLOAD;
            dwlParserObjPtr->subsection = DWL_SUB_COMMENTS;
            pkgDwlObjPtr->lenToDownload = dwlParserObjPtr->commentSize;

            // Store package CRC from first DWL prolog for further comparison
            dwlParserObjPtr->packageCRC = dwlPrologPtr->crc32;
            LOG_ARG("Package CRC: 0x%08x", dwlParserObjPtr->packageCRC);

            // Initialize computed CRC
            dwlParserObjPtr->computedCRC = crc32(0L, NULL, 0);

            // Compute CRC starting from fileSize in first DWL prolog (ignore magic, file size, CRC)
            size_t prologSizeForCrc = sizeof(DwlProlog_t) - (3 * sizeof(uint32_t));
            dwlParserObjPtr->computedCRC = crc32(dwlParserObjPtr->computedCRC,
                                                 (uint8_t*)&dwlPrologPtr->fileSize,
                                                 prologSizeForCrc);
            LOG_ARG("New computed CRC: 0x%08x", dwlParserObjPtr->computedCRC);
            break;

        case DWL_TYPE_BINA:
            dwlParserObjPtr->commentSize = (dwlPrologPtr->commentSize << 3);
            dwlParserObjPtr->binarySize = dwlPrologPtr->fileSize
                                          - dwlParserObjPtr->commentSize
                                          - LWM2MCORE_BINA_HEADER_SIZE
                                          - sizeof(DwlProlog_t);
            dwlParserObjPtr->paddingSize = ((dwlPrologPtr->fileSize + 7) & 0xFFFFFFF8)
                                           - dwlPrologPtr->fileSize;

            // Download DWL comments
            pkgDwlObjPtr->state = PKG_DWL_DOWNLOAD;
            dwlParserObjPtr->subsection = DWL_SUB_COMMENTS;
            pkgDwlObjPtr->lenToDownload = dwlParserObjPtr->commentSize;

            // Not first DWL prolog, compute CRC with whole prolog
            dwlParserObjPtr->computedCRC = crc32(dwlParserObjPtr->computedCRC,
                                                 pkgDwlObjPtr->dwlData,
                                                 pkgDwlObjPtr->downloadedLen);
            LOG_ARG("New computed CRC: 0x%08x", dwlParserObjPtr->computedCRC);
            break;

        case DWL_TYPE_SIGN:
            dwlParserObjPtr->commentSize = (dwlPrologPtr->commentSize << 3);
            dwlParserObjPtr->signatureSize = dwlPrologPtr->fileSize
                                             - dwlParserObjPtr->commentSize
                                             - sizeof(DwlProlog_t);

            // Download DWL comments
            pkgDwlObjPtr->state = PKG_DWL_DOWNLOAD;
            dwlParserObjPtr->subsection = DWL_SUB_COMMENTS;
            pkgDwlObjPtr->lenToDownload = dwlParserObjPtr->commentSize;

            // Whole signature section is ignored for CRC computation
            break;

        default:
            LOG_ARG("Unexpected DWL prolog for section type 0x%08x", dwlParserObjPtr->section);
            pkgDwlObjPtr->updateResult = LWM2MCORE_FW_UPDATE_RESULT_UNSUPPORTED_PKG_TYPE;
            return DWL_FAULT;
            break;
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Parse DWL comments located after the DWL prolog. The comments length is given by the DWL prolog.
 *
 * @return
 *  - DWL_OK      The function succeeded
 *  - DWL_FAULT   The function failed
 */
//--------------------------------------------------------------------------------------------------
static lwm2mcore_DwlResult_t ParseDwlComments
(
    PackageDownloaderObj_t* pkgDwlObjPtr,   ///< Package downloader object
    DwlParserObj_t* dwlParserObjPtr         ///< DWL parser object
)
{
    lwm2mcore_DwlResult_t result = DWL_OK;

    LOG_ARG("Parse DWL comments, length %u", pkgDwlObjPtr->downloadedLen);

    // Check if the comment section is not empty
    if (0 != pkgDwlObjPtr->downloadedLen)
    {
        LOG_ARG("DWL comments: %s", pkgDwlObjPtr->dwlData);

        // Update CRC
        dwlParserObjPtr->computedCRC = crc32(dwlParserObjPtr->computedCRC,
                                             pkgDwlObjPtr->dwlData,
                                             pkgDwlObjPtr->downloadedLen);
        LOG_ARG("New computed CRC: 0x%08x", dwlParserObjPtr->computedCRC);
    }

    // Determine next awaited subsection
    switch (dwlParserObjPtr->section)
    {
        case DWL_TYPE_UPCK:
            // Download UPCK header
            pkgDwlObjPtr->state = PKG_DWL_DOWNLOAD;
            dwlParserObjPtr->subsection = DWL_SUB_HEADER;
            pkgDwlObjPtr->lenToDownload = LWM2MCORE_UPCK_HEADER_SIZE;
            break;

        case DWL_TYPE_BINA:
            // Download BINA header
            pkgDwlObjPtr->state = PKG_DWL_DOWNLOAD;
            dwlParserObjPtr->subsection = DWL_SUB_HEADER;
            pkgDwlObjPtr->lenToDownload = LWM2MCORE_BINA_HEADER_SIZE;
            break;

        case DWL_TYPE_SIGN:
            // Download signature
            pkgDwlObjPtr->state = PKG_DWL_DOWNLOAD;
            dwlParserObjPtr->subsection = DWL_SUB_SIGNATURE;
            pkgDwlObjPtr->lenToDownload = dwlParserObjPtr->signatureSize;
            break;

        default:
            LOG_ARG("Unexpected DWL prolog for section type 0x%08x", dwlParserObjPtr->section);
            pkgDwlObjPtr->updateResult = LWM2MCORE_FW_UPDATE_RESULT_UNSUPPORTED_PKG_TYPE;
            return DWL_FAULT;
            break;
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Parse DWL header data. The header length is fixed and depends on the section type.
 *
 * @return
 *  - DWL_OK      The function succeeded
 *  - DWL_FAULT   The function failed
 */
//--------------------------------------------------------------------------------------------------
static lwm2mcore_DwlResult_t ParseDwlHeader
(
    PackageDownloaderObj_t* pkgDwlObjPtr,   ///< Package downloader object
    DwlParserObj_t* dwlParserObjPtr         ///< DWL parser object
)
{
    lwm2mcore_DwlResult_t result = DWL_OK;

    LOG_ARG("Parse DWL header, length %u", pkgDwlObjPtr->downloadedLen);

    // Parse header and determine next awaited subsection
    switch (dwlParserObjPtr->section)
    {
        case DWL_TYPE_UPCK:
        {
            // Check UPCK type
            uint32_t upckType = ((UpckHeader_t*)pkgDwlObjPtr->dwlData)->structHeader.upckType;
            if (   (LWM2MCORE_UPCK_TYPE_FW != upckType)
                && (LWM2MCORE_UPCK_TYPE_AMSS != upckType)
               )
            {
                LOG_ARG("Incorrect Update Package type %u", upckType);
                pkgDwlObjPtr->updateResult = LWM2MCORE_FW_UPDATE_RESULT_UNSUPPORTED_PKG_TYPE;
                return DWL_FAULT;
            }

            // Set package type
            pkgDwlObjPtr->packageType = LWM2MCORE_PKG_FW;

            // Download next DWL prolog
            pkgDwlObjPtr->state = PKG_DWL_DOWNLOAD;
            dwlParserObjPtr->subsection = DWL_SUB_PROLOG;
            pkgDwlObjPtr->lenToDownload = sizeof(DwlProlog_t);
        }
        break;

        case DWL_TYPE_BINA:
            // Download DWL binary data
            pkgDwlObjPtr->state = PKG_DWL_DOWNLOAD;
            dwlParserObjPtr->subsection = DWL_SUB_BINARY;
            dwlParserObjPtr->remainingBinaryData = dwlParserObjPtr->binarySize;
            if (dwlParserObjPtr->remainingBinaryData > MAX_DATA_BUFFER_CHUNK)
            {
                pkgDwlObjPtr->lenToDownload = MAX_DATA_BUFFER_CHUNK;
            }
            else
            {
                pkgDwlObjPtr->lenToDownload = dwlParserObjPtr->remainingBinaryData;
            }
            break;

        default:
            LOG_ARG("Unexpected DWL prolog for section type 0x%08x", dwlParserObjPtr->section);
            pkgDwlObjPtr->updateResult = LWM2MCORE_FW_UPDATE_RESULT_UNSUPPORTED_PKG_TYPE;
            return DWL_FAULT;
            break;
    }

    // Update CRC
    dwlParserObjPtr->computedCRC = crc32(dwlParserObjPtr->computedCRC,
                                         pkgDwlObjPtr->dwlData,
                                         pkgDwlObjPtr->downloadedLen);
    LOG_ARG("New computed CRC: 0x%08x", dwlParserObjPtr->computedCRC);

    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Parse DWL binary data and store it. The binary data length is given by the DWL prolog.
 *
 * @return
 *  - DWL_OK      The function succeeded
 *  - DWL_FAULT   The function failed
 */
//--------------------------------------------------------------------------------------------------
static lwm2mcore_DwlResult_t ParseDwlBinary
(
    PackageDownloaderObj_t* pkgDwlObjPtr,   ///< Package downloader object
    DwlParserObj_t* dwlParserObjPtr         ///< DWL parser object
)
{
    lwm2mcore_DwlResult_t result = DWL_OK;

    LOG_ARG("Parse DWL binary data, length %u", pkgDwlObjPtr->downloadedLen);

    // Check if subsection is expected in current DWL section
    if (DWL_TYPE_BINA != dwlParserObjPtr->section)
    {
        LOG_ARG("Unexpected DWL binary data for section type 0x%08x", dwlParserObjPtr->section);
        pkgDwlObjPtr->updateResult = LWM2MCORE_FW_UPDATE_RESULT_UNSUPPORTED_PKG_TYPE;
        return DWL_FAULT;
    }

    // Update remaining length to download
    if (pkgDwlObjPtr->downloadedLen <= dwlParserObjPtr->remainingBinaryData)
    {
        dwlParserObjPtr->remainingBinaryData -= pkgDwlObjPtr->downloadedLen;
    }
    else
    {
        LOG_ARG("Received too much binary data: %u > %llu",
                pkgDwlObjPtr->downloadedLen, dwlParserObjPtr->remainingBinaryData);
        pkgDwlObjPtr->updateResult = LWM2MCORE_FW_UPDATE_RESULT_COMMUNICATION_ERROR;
        return DWL_FAULT;
    }

    // Store downloaded binary data
    pkgDwlObjPtr->state = PKG_DWL_STORE;

    // Check if there are more binary data
    if (dwlParserObjPtr->remainingBinaryData)
    {
        // Prepare download of next binary data
        dwlParserObjPtr->subsection = DWL_SUB_BINARY;
        if (dwlParserObjPtr->remainingBinaryData > MAX_DATA_BUFFER_CHUNK)
        {
            pkgDwlObjPtr->lenToDownload = MAX_DATA_BUFFER_CHUNK;
        }
        else
        {
            pkgDwlObjPtr->lenToDownload = dwlParserObjPtr->remainingBinaryData;
        }
    }
    else
    {
        // End of binary data, prepare download of DWL padding data
        dwlParserObjPtr->subsection = DWL_SUB_PADDING;
        pkgDwlObjPtr->lenToDownload = dwlParserObjPtr->paddingSize;
    }

    // Update CRC
    dwlParserObjPtr->computedCRC = crc32(dwlParserObjPtr->computedCRC,
                                         pkgDwlObjPtr->dwlData,
                                         pkgDwlObjPtr->downloadedLen);
    LOG_ARG("New computed CRC: 0x%08x", dwlParserObjPtr->computedCRC);

    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Parse DWL padding data. The padding data length is computed with the DWL prolog data.
 *
 * @return
 *  - DWL_OK      The function succeeded
 *  - DWL_FAULT   The function failed
 */
//--------------------------------------------------------------------------------------------------
static lwm2mcore_DwlResult_t ParseDwlPadding
(
    PackageDownloaderObj_t* pkgDwlObjPtr,   ///< Package downloader object
    DwlParserObj_t* dwlParserObjPtr         ///< DWL parser object
)
{
    lwm2mcore_DwlResult_t result = DWL_OK;

    LOG_ARG("Parse DWL padding, length %u", pkgDwlObjPtr->downloadedLen);

    // Check if subsection is expected in current DWL section
    if (DWL_TYPE_BINA != dwlParserObjPtr->section)
    {
        LOG_ARG("Unexpected DWL padding data for section type 0x%08x", dwlParserObjPtr->section);
        pkgDwlObjPtr->updateResult = LWM2MCORE_FW_UPDATE_RESULT_UNSUPPORTED_PKG_TYPE;
        return DWL_FAULT;
    }

    // Download next DWL prolog
    pkgDwlObjPtr->state = PKG_DWL_DOWNLOAD;
    dwlParserObjPtr->subsection = DWL_SUB_PROLOG;
    pkgDwlObjPtr->lenToDownload = sizeof(DwlProlog_t);

    // Update CRC
    dwlParserObjPtr->computedCRC = crc32(dwlParserObjPtr->computedCRC,
                                         pkgDwlObjPtr->dwlData,
                                         pkgDwlObjPtr->downloadedLen);
    LOG_ARG("New computed CRC: 0x%08x", dwlParserObjPtr->computedCRC);

    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Parse DWL signature data. The signature length is computed with the DWL prolog data.
 *
 * @return
 *  - DWL_OK      The function succeeded
 *  - DWL_FAULT   The function failed
 */
//--------------------------------------------------------------------------------------------------
static lwm2mcore_DwlResult_t ParseDwlSignature
(
    PackageDownloaderObj_t* pkgDwlObjPtr,   ///< Package downloader object
    DwlParserObj_t* dwlParserObjPtr         ///< DWL parser object
)
{
    lwm2mcore_DwlResult_t result = DWL_OK;

    LOG_ARG("Parse DWL signature, length %u", pkgDwlObjPtr->downloadedLen);

    // Check if subsection is expected in current DWL section
    if (DWL_TYPE_SIGN != dwlParserObjPtr->section)
    {
        LOG_ARG("Unexpected DWL signature for section type 0x%08x", dwlParserObjPtr->section);
        pkgDwlObjPtr->updateResult = LWM2MCORE_FW_UPDATE_RESULT_UNSUPPORTED_PKG_TYPE;
        return DWL_FAULT;
    }

    // Whole signature section is ignored for CRC computation, no need to update it.
    // Compare package CRC retrieved from first DWL prolog and computed CRC.
    if (dwlParserObjPtr->packageCRC != dwlParserObjPtr->computedCRC)
    {
        LOG_ARG("Incorrect file CRC: expected 0x%08x, computed 0x%08x",
                dwlParserObjPtr->packageCRC, dwlParserObjPtr->computedCRC);
        pkgDwlObjPtr->updateResult = LWM2MCORE_FW_UPDATE_RESULT_VERIFY_ERROR;
        return DWL_FAULT;
    }

    // End of file
    pkgDwlObjPtr->state = PKG_DWL_END;

    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * DWL package parser.
 *
 * This function parses the downloaded data as a DWL file, until the end of the package is reached.
 *
 * @return
 *  - DWL_OK      The function succeeded
 *  - DWL_FAULT   The function failed
 */
//--------------------------------------------------------------------------------------------------
static lwm2mcore_DwlResult_t DwlParser
(
    PackageDownloaderObj_t* pkgDwlObjPtr    ///< Package downloader object
)
{
    lwm2mcore_DwlResult_t result;
    static DwlParserObj_t dwlParserObj =
    {
        .section = 0,
        .subsection = DWL_SUB_PROLOG,
        .packageCRC = 0,
        .computedCRC = 0,
        .commentSize = 0,
        .binarySize = 0,
        .paddingSize = 0,
        .remainingBinaryData = 0,
        .signatureSize = 0
    };

    // Check if data was downloaded
    if (!pkgDwlObjPtr->dwlData)
    {
        LOG("NULL data pointer in DWL parser");
        pkgDwlObjPtr->updateResult = LWM2MCORE_FW_UPDATE_RESULT_COMMUNICATION_ERROR;
        return DWL_FAULT;
    }

    // Parse the downloaded data based on the current subsection
    switch (dwlParserObj.subsection)
    {
        case DWL_SUB_PROLOG:
            result = ParseDwlProlog(pkgDwlObjPtr, &dwlParserObj);
            break;

        case DWL_SUB_COMMENTS:
            result = ParseDwlComments(pkgDwlObjPtr, &dwlParserObj);
            break;

        case DWL_SUB_HEADER:
            result = ParseDwlHeader(pkgDwlObjPtr, &dwlParserObj);
            break;

        case DWL_SUB_BINARY:
            result = ParseDwlBinary(pkgDwlObjPtr, &dwlParserObj);
            break;

        case DWL_SUB_PADDING:
            result = ParseDwlPadding(pkgDwlObjPtr, &dwlParserObj);
            break;

        case DWL_SUB_SIGNATURE:
            result = ParseDwlSignature(pkgDwlObjPtr, &dwlParserObj);
            break;

        default:
            LOG_ARG("Unknown DWL subsection %u", dwlParserObj.subsection);
            pkgDwlObjPtr->updateResult = LWM2MCORE_FW_UPDATE_RESULT_UNSUPPORTED_PKG_TYPE;
            result = DWL_FAULT;
            break;
    }

    // Check if the DWL parsing is finished
    if ((DWL_OK != result) || (PKG_DWL_END == pkgDwlObjPtr->state))
    {
        // Reset the DWL parser object for next use
        memset(&dwlParserObj, 0, sizeof(DwlParserObj_t));
        dwlParserObj.subsection = DWL_SUB_PROLOG;
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Initialize the package download and determine next state
 */
//--------------------------------------------------------------------------------------------------
static void PkgDwlInit
(
    lwm2mcore_PackageDownloader_t* pkgDwlPtr,   ///< Package downloader
    PackageDownloaderObj_t* pkgDwlObjPtr        ///< Package downloader object
)
{
    // Initialize download
    pkgDwlObjPtr->result = pkgDwlPtr->initDownload(pkgDwlPtr->data.packageUri, pkgDwlPtr->ctxPtr);
    if (DWL_OK != pkgDwlObjPtr->result)
    {
        LOG("Error during download initialization");
        pkgDwlObjPtr->updateResult = LWM2MCORE_FW_UPDATE_RESULT_COMMUNICATION_ERROR;
        pkgDwlObjPtr->state = PKG_DWL_ERROR;
        return;
    }

    // Set update result to 'Normal' when the updating process is initiated
    pkgDwlObjPtr->updateResult = LWM2MCORE_FW_UPDATE_RESULT_DEFAULT_NORMAL;
    pkgDwlObjPtr->result=pkgDwlPtr->setUpdateResult(pkgDwlObjPtr->updateResult);
    if (DWL_OK != pkgDwlObjPtr->result)
    {
        LOG("Unable to set update result");
        pkgDwlObjPtr->updateResult = LWM2MCORE_FW_UPDATE_RESULT_COMMUNICATION_ERROR;
        pkgDwlObjPtr->state = PKG_DWL_ERROR;
        return;
    }

    // Retrieve package information
    pkgDwlObjPtr->state = PKG_DWL_INFO;
}

//--------------------------------------------------------------------------------------------------
/**
 * Retrieve information about the package to download and determine next state
 */
//--------------------------------------------------------------------------------------------------
static void PkgDwlGetInfo
(
    lwm2mcore_PackageDownloader_t* pkgDwlPtr,   ///< Package downloader
    PackageDownloaderObj_t* pkgDwlObjPtr        ///< Package downloader object
)
{
    // Get information about the package
    pkgDwlObjPtr->result = pkgDwlPtr->getInfo(&pkgDwlPtr->data, pkgDwlPtr->ctxPtr);
    if (DWL_OK != pkgDwlObjPtr->result)
    {
        LOG("Error while getting the package information");
        pkgDwlObjPtr->updateResult = LWM2MCORE_FW_UPDATE_RESULT_COMMUNICATION_ERROR;
        pkgDwlObjPtr->state = PKG_DWL_ERROR;
        return;
    }

    // Notify the application of the package size
    PkgDwlEvent(PKG_DWL_EVENT_DETAILS, pkgDwlPtr, pkgDwlObjPtr);

    // Download first package chunk
    pkgDwlObjPtr->state = PKG_DWL_DOWNLOAD;
    // Download length of DWL prolog, enough to determine file type
    pkgDwlObjPtr->lenToDownload = sizeof(DwlProlog_t);
}

//--------------------------------------------------------------------------------------------------
/**
 * Download a data chunk and determine next state
 */
//--------------------------------------------------------------------------------------------------
static void PkgDwlDownload
(
    lwm2mcore_PackageDownloader_t* pkgDwlPtr,   ///< Package downloader
    PackageDownloaderObj_t* pkgDwlObjPtr        ///< Package downloader object
)
{
    size_t   remainingLen = pkgDwlObjPtr->lenToDownload;
    size_t   readLen = 0;
    uint32_t bufOffset = 0;

    // Reset download buffer
    pkgDwlObjPtr->downloadedLen = 0;
    memset(&pkgDwlObjPtr->dwlData, 0, sizeof(pkgDwlObjPtr->dwlData));

    // Check if the download will not go beyond the end of the file
    if ((pkgDwlObjPtr->offset + pkgDwlObjPtr->lenToDownload) > pkgDwlPtr->data.packageSize)
    {
        LOG_ARG("Download after end of file: offset %llu, to download %u, file size %llu",
                 pkgDwlObjPtr->offset, pkgDwlObjPtr->lenToDownload, pkgDwlPtr->data.packageSize);
        pkgDwlObjPtr->updateResult = LWM2MCORE_FW_UPDATE_RESULT_COMMUNICATION_ERROR;
        pkgDwlObjPtr->result = DWL_FAULT;
        pkgDwlObjPtr->state = PKG_DWL_ERROR;
        return;
    }

    // Notify the download beginning
    if (true == pkgDwlObjPtr->firstDownload)
    {
        // Set update state to 'Downloading'
        pkgDwlObjPtr->result = pkgDwlPtr->setUpdateState(LWM2MCORE_FW_UPDATE_STATE_DOWNLOADING);
        if (DWL_OK != pkgDwlObjPtr->result)
        {
            LOG("Unable to set update state");
            pkgDwlObjPtr->updateResult = LWM2MCORE_FW_UPDATE_RESULT_COMMUNICATION_ERROR;
            pkgDwlObjPtr->state = PKG_DWL_ERROR;
            return;
        }
        pkgDwlObjPtr->firstDownload = false;

        // Notify the application of the download start
        PkgDwlEvent(PKG_DWL_EVENT_DL_START, pkgDwlPtr, pkgDwlObjPtr);
    }

    // Check if any data to download
    if (0 == pkgDwlObjPtr->lenToDownload)
    {
        // No data to download, next state will be determined by parser
        LOG("No data to download");
        pkgDwlObjPtr->state = PKG_DWL_PARSE;
        return;
    }

    // Callback could download less bytes than requested.
    // Loop until everything is downloaded or a download request failed
    LOG_ARG("Download %u bytes at offset %llu", remainingLen, pkgDwlObjPtr->offset);
    do
    {
        pkgDwlObjPtr->result = pkgDwlPtr->downloadRange(&pkgDwlObjPtr->dwlData[bufOffset],
                                                        remainingLen,
                                                        pkgDwlObjPtr->offset,
                                                        &readLen,
                                                        pkgDwlPtr->ctxPtr);
        LOG_ARG("Downloaded %u bytes, result %d", readLen, pkgDwlObjPtr->result);
        if (DWL_OK == pkgDwlObjPtr->result)
        {
            // Update remaining length and offsets
            remainingLen -= readLen;
            pkgDwlObjPtr->offset += readLen;
            bufOffset += readLen;
            pkgDwlObjPtr->downloadedLen += readLen;
        }
    } while ((remainingLen > 0) && (DWL_OK == pkgDwlObjPtr->result) && (readLen > 0));

    if ((DWL_OK != pkgDwlObjPtr->result) || (remainingLen > 0))
    {
        LOG_ARG("Error during download of %u bytes (%u remaining, result %d)",
                pkgDwlObjPtr->lenToDownload, remainingLen, pkgDwlObjPtr->result);
        pkgDwlObjPtr->updateResult = LWM2MCORE_FW_UPDATE_RESULT_COMMUNICATION_ERROR;
        pkgDwlObjPtr->state = PKG_DWL_ERROR;
        return;
    }

    // Update overall download progress
    pkgDwlObjPtr->downloadProgress = (100.0 * pkgDwlObjPtr->offset) / pkgDwlPtr->data.packageSize;
    // Notify the application of the download progress
    // Note: the downloader has far more information about the progress (e.g. ETA) and a callback
    // could be implemented to retrieve these data instead of computing it here.
    PkgDwlEvent(PKG_DWL_EVENT_DL_PROGRESS, pkgDwlPtr, pkgDwlObjPtr);

    // Parse downloaded data
    pkgDwlObjPtr->state = PKG_DWL_PARSE;
}

//--------------------------------------------------------------------------------------------------
/**
 * Parse a data chunk and determine next state
 */
//--------------------------------------------------------------------------------------------------
static void PkgDwlParse
(
    lwm2mcore_PackageDownloader_t* pkgDwlPtr,   ///< Package downloader
    PackageDownloaderObj_t* pkgDwlObjPtr        ///< Package downloader object
)
{
    // Parse downloaded data and determine next state
    pkgDwlObjPtr->result = DwlParser(pkgDwlObjPtr);
    if (DWL_OK != pkgDwlObjPtr->result)
    {
        LOG("Error while parsing the DWL package");
        pkgDwlObjPtr->state = PKG_DWL_ERROR;
        // No need to change update result, already set by DWL parser
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Store a downloaded data chunk and determine next state
 */
//--------------------------------------------------------------------------------------------------
static void PkgDwlStore
(
    lwm2mcore_PackageDownloader_t* pkgDwlPtr,   ///< Package downloader
    PackageDownloaderObj_t* pkgDwlObjPtr        ///< Package downloader object
)
{
    LOG_ARG("Store %u bytes at offset %llu",
            pkgDwlObjPtr->downloadedLen, pkgDwlObjPtr->storageOffset);

    // Store downloaded data
    pkgDwlObjPtr->result = pkgDwlPtr->storeRange(pkgDwlObjPtr->dwlData,
                                                 pkgDwlObjPtr->downloadedLen,
                                                 pkgDwlObjPtr->storageOffset,
                                                 pkgDwlPtr->ctxPtr);
    if (DWL_OK != pkgDwlObjPtr->result)
    {
        LOG("Error during data storage");
        pkgDwlObjPtr->updateResult = LWM2MCORE_FW_UPDATE_RESULT_OUT_OF_MEMORY;
        pkgDwlObjPtr->state = PKG_DWL_ERROR;
        return;
    }

    // Update storage offset
    pkgDwlObjPtr->storageOffset += pkgDwlObjPtr->downloadedLen;

    // Download next package chunk
    pkgDwlObjPtr->state = PKG_DWL_DOWNLOAD;
}

//--------------------------------------------------------------------------------------------------
/**
 * Process package downloader error and determine next state
 */
//--------------------------------------------------------------------------------------------------
static void PkgDwlError
(
    lwm2mcore_PackageDownloader_t* pkgDwlPtr,   ///< Package downloader
    PackageDownloaderObj_t* pkgDwlObjPtr        ///< Package downloader object
)
{
    // Error during package downloading
    char error[ERROR_STR_MAX_LEN];
    memset(error, 0, sizeof(error));

    // Only display debug traces for the package downloader error, the error will be notified
    // by the 'download end' event.
    // One exception for the signature check error, which should also be notified by a dedicated
    // event.
    switch (pkgDwlObjPtr->updateResult)
    {
        case LWM2MCORE_FW_UPDATE_RESULT_NO_STORAGE_SPACE:
            snprintf(error, ERROR_STR_MAX_LEN, "not enough space");
            break;

        case LWM2MCORE_FW_UPDATE_RESULT_OUT_OF_MEMORY:
            snprintf(error, ERROR_STR_MAX_LEN, "out of memory");
            break;

        case LWM2MCORE_FW_UPDATE_RESULT_COMMUNICATION_ERROR:
            snprintf(error, ERROR_STR_MAX_LEN, "communication error");
            break;

        case LWM2MCORE_FW_UPDATE_RESULT_VERIFY_ERROR:
            snprintf(error, ERROR_STR_MAX_LEN, "package check error");

            // Notify the application of the signature check error
            PkgDwlEvent(PKG_DWL_EVENT_SIGN_KO, pkgDwlPtr, pkgDwlObjPtr);
            break;

        case LWM2MCORE_FW_UPDATE_RESULT_UNSUPPORTED_PKG_TYPE:
            snprintf(error, ERROR_STR_MAX_LEN, "unsupported package");
            break;

        case LWM2MCORE_FW_UPDATE_RESULT_INVALID_URI:
            snprintf(error, ERROR_STR_MAX_LEN, "invalid URI");
            break;

        case LWM2MCORE_FW_UPDATE_RESULT_UNSUPPORTED_PROTOCOL:
            snprintf(error, ERROR_STR_MAX_LEN, "unsupported protocol");
            break;

        default:
            snprintf(error, ERROR_STR_MAX_LEN, "unknown error");
            break;
    }

    LOG_ARG("Error during package downloading: %s (update result = %d)",
            error, pkgDwlObjPtr->updateResult);

    // End of download
    pkgDwlObjPtr->state = PKG_DWL_END;
}

//--------------------------------------------------------------------------------------------------
/**
 * End the download process
 */
//--------------------------------------------------------------------------------------------------
static void PkgDwlEnd
(
    lwm2mcore_PackageDownloader_t* pkgDwlPtr,   ///< Package downloader
    PackageDownloaderObj_t* pkgDwlObjPtr        ///< Package downloader object
)
{
    // Check if an error was detected during the package download or parsing
    if (LWM2MCORE_FW_UPDATE_RESULT_DEFAULT_NORMAL != pkgDwlObjPtr->updateResult)
    {
        // Error during download or parsing, set update result accordingly.
        // No need to change the update state, it should remain set to 'Downloading'.
        pkgDwlObjPtr->result = pkgDwlPtr->setUpdateResult(pkgDwlObjPtr->updateResult);
        if (DWL_OK != pkgDwlObjPtr->result)
        {
            LOG("Unable to set update result");
        }
    }
    else
    {
        // Notify the application of the signature validation
        PkgDwlEvent(PKG_DWL_EVENT_SIGN_OK, pkgDwlPtr, pkgDwlObjPtr);

        // Successful download: set update state to 'Downloaded'.
        // No need to change the update result, it was already set to 'Normal' at the beginning.
        pkgDwlObjPtr->result = pkgDwlPtr->setUpdateState(LWM2MCORE_FW_UPDATE_STATE_DOWNLOADED);
        if (DWL_OK != pkgDwlObjPtr->result)
        {
            LOG("Unable to set update state");
        }
    }

    // Notify the application of the download end
    PkgDwlEvent(PKG_DWL_EVENT_DL_END, pkgDwlPtr, pkgDwlObjPtr);

    // End of download
    pkgDwlObjPtr->result = pkgDwlPtr->endDownload(pkgDwlPtr->ctxPtr);
    if (DWL_OK != pkgDwlObjPtr->result)
    {
        LOG("Error while ending the download");
    }

    // End of processing
    pkgDwlObjPtr->endOfProcessing = true;
}

//--------------------------------------------------------------------------------------------------
// Public functions
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Run the package downloader
 *
 * @return
 *  - DWL_OK    The function succeeded
 *  - DWL_FAULT The function failed
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_DwlResult_t lwm2mcore_PackageDownloaderRun
(
    lwm2mcore_PackageDownloader_t* pkgDwlPtr    ///< Package downloader
)
{
    PackageDownloaderObj_t pkgDwlObj;

    // Check input parameters
    if (!pkgDwlPtr->data.packageUri)
    {
        LOG("No package URI");
        return DWL_FAULT;
    }

    if (!pkgDwlPtr->initDownload)
    {
        LOG("Missing initialization callback");
        return DWL_FAULT;
    }

    if (!pkgDwlPtr->getInfo)
    {
        LOG("Missing get info callback");
        return DWL_FAULT;
    }

    if (!pkgDwlPtr->setUpdateState)
    {
        LOG("Missing firmware update state callback");
        return DWL_FAULT;
    }

    if (!pkgDwlPtr->setUpdateResult)
    {
        LOG("Missing firmware update result callback");
        return DWL_FAULT;
    }

    if (!pkgDwlPtr->downloadRange)
    {
        LOG("Missing download callback");
        return DWL_FAULT;
    }

    if (!pkgDwlPtr->storeRange)
    {
        LOG("Missing storing callback");
        return DWL_FAULT;
    }

    if (!pkgDwlPtr->endDownload)
    {
        LOG("Missing ending callback");
        return DWL_FAULT;
    }

    // Package downloader initialization
    memset(&pkgDwlObj, 0, sizeof(PackageDownloaderObj_t));
    pkgDwlObj.state = PKG_DWL_INIT;
    pkgDwlObj.updateResult = LWM2MCORE_FW_UPDATE_RESULT_DEFAULT_NORMAL;
    pkgDwlObj.endOfProcessing = false;
    pkgDwlObj.firstDownload = true;
    pkgDwlObj.packageType = LWM2MCORE_PKG_NONE;

    // Run the package downloader until end of processing is reached (end of file, error...)
    while (!pkgDwlObj.endOfProcessing)
    {
        // Run the package downloader action based on the current state
        switch (pkgDwlObj.state)
        {
            case PKG_DWL_INIT:
                PkgDwlInit(pkgDwlPtr, &pkgDwlObj);
                break;

            case PKG_DWL_INFO:
                PkgDwlGetInfo(pkgDwlPtr, &pkgDwlObj);
                break;

            case PKG_DWL_DOWNLOAD:
                PkgDwlDownload(pkgDwlPtr, &pkgDwlObj);
                break;

            case PKG_DWL_PARSE:
                PkgDwlParse(pkgDwlPtr, &pkgDwlObj);
                break;

            case PKG_DWL_STORE:
                PkgDwlStore(pkgDwlPtr, &pkgDwlObj);
                break;

            case PKG_DWL_ERROR:
                PkgDwlError(pkgDwlPtr, &pkgDwlObj);
                break;

            case PKG_DWL_END:
                PkgDwlEnd(pkgDwlPtr, &pkgDwlObj);
                break;

            default:
                LOG_ARG("Unknown package downloader state %d", pkgDwlObj.state);
                pkgDwlObj.result = DWL_FAULT;
                pkgDwlObj.endOfProcessing = true;
                break;
        }
    }

    return pkgDwlObj.result;
}
