/**
 * @file lwm2mcorePackageDownloader.c
 *
 * @section lwm2mcorePackageDownloader LWM2M Package Downloader
 *
 * The LWM2M package downloader is launched with lwm2mcore_PackageDownloaderRun().
 * When the package download starts, downloaded data should be sequentially transmitted to the
 * package downloader using lwm2mcore_PackageDownloaderReceiveData().
 *
 * @section lwm2mcoreDwlParser DWL parser
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
 * @section lwm2mcorePackageVerification Package verification
 *
 * The package CRC is retrieved in the first DWL prolog. A CRC is then computed with all binary data
 * from the package, starting from the first byte after the package CRC until the end of the BINA
 * section. The SIGN section is therefore ignored for the CRC computation.
 *
 * The package signature is computed by hashing all the data from the beginning of the file until
 * the end of the BINA section, using the SHA1 algorithm. The SIGN section is therefore ignored for
 * the SHA1 digest computation.
 *
 * <HR>
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#include <stdio.h>
#include <liblwm2m.h>
#include <string.h>
#include <lwm2mcore.h>
#include <internals.h>
#include "lwm2mcorePackageDownloader.h"
#include "sessionManager.h"
#include "osPortSecurity.h"

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
 * Maximal length of a temporary DWL chunk.
 *
 * This chunk is used to store the downloaded data if the received length is too small
 * compared to the awaited DWL subsection length. Each subsection has a indeed defined length,
 * except for the comments:
 * - DWL prolog:  32 bytes
 * - Header:     128 bytes
 * - Padding:      7 bytes (max)
 * - Signature: 1024 bytes (max)
 * - Comments: variable, given by the DWL prolog
 *
 * Considering this, the limit is arbitrarily set to 16k to handle all subsections
 * and hopefully all comments lengths.
 */
//--------------------------------------------------------------------------------------------------
#define TMP_DATA_MAX_LEN    16384

//--------------------------------------------------------------------------------------------------
/**
 * Magic number identifying a DWL prolog
 */
//--------------------------------------------------------------------------------------------------
#define DWL_MAGIC_NUMBER    0x464c5744  ///< DWLF

//--------------------------------------------------------------------------------------------------
/**
 * Possible types of DWL sections
 */
//--------------------------------------------------------------------------------------------------
#define DWL_TYPE_UPCK       0x4b435055  ///< UpdatePackage
#define DWL_TYPE_SIGN       0x4e474953  ///< Signature
#define DWL_TYPE_BINA       0x414e4942  ///< Binary
#define DWL_TYPE_COMP       0x504d4f43  ///< CompBinary
#define DWL_TYPE_XDWL       0x4c574458  ///< Downloader
#define DWL_TYPE_E2PR       0x52503245  ///< EEPROM
#define DWL_TYPE_DIFF       0x46464944  ///< Patch
#define DWL_TYPE_DOTA       0x41544f44  ///< DotaCell
#define DWL_TYPE_RAM_       0x5f4d4152  ///< Ram
#define DWL_TYPE_BOOT       0x544f4f42  ///< Bootstrap

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
    PKG_DWL_DOWNLOAD,   ///< Download file
    PKG_DWL_PARSE,      ///< Parse downloaded data
    PKG_DWL_STORE,      ///< Store downloaded data
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
    PackageDownloaderState_t   state;               ///< State of package downloader state machine
    bool                       endOfProcessing;     ///< End of package processing
    lwm2mcore_DwlResult_t      result;              ///< Current result of package downloader
    lwm2mcore_fwUpdateResult_t updateResult;        ///< Current package update result
    lwm2mcore_PkgDwlType_t     packageType;         ///< Package type (FW or SW)
    uint64_t                   offset;              ///< Current offset in the package
    uint64_t                   storageOffset;       ///< Current offset in data storage
    uint8_t                    tmpData[TMP_DATA_MAX_LEN];   ///< Temporary data chunk
    uint32_t                   tmpDataLen;          ///< Temporary data chunk length
    uint8_t*                   dwlDataPtr;          ///< Downloaded data pointer
    size_t                     downloadedLen;       ///< Length of downloaded data
    size_t                     processedLen;        ///< Length of data processed by last parsing
    uint32_t                   downloadProgress;    ///< Overall download progress
}
PackageDownloaderObj_t;

//--------------------------------------------------------------------------------------------------
/**
 * DWL parser object structure
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    uint8_t* dataToParsePtr;        ///< Data to parse
    size_t   lenToParse;            ///< Length of next subsection to parse
    uint32_t section;               ///< Current DWL section
    uint8_t  subsection;            ///< Current DWL subsection
    uint32_t packageCRC;            ///< Package CRC read in first DWL prolog
    uint32_t computedCRC;           ///< CRC computed with downloaded data
    uint64_t commentSize;           ///< Comments size read in DWL prolog
    uint64_t binarySize;            ///< Binary package size read in DWL prolog
    uint64_t paddingSize;           ///< Binary padding size read in DWL prolog
    uint64_t remainingBinaryData;   ///< Remaining length of binary data to download
    uint64_t signatureSize;         ///< Signature size read in DWL prolog
    void*    sha1CtxPtr;            ///< SHA1 context pointer
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
// Static variables
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Package Downloader object instance
 */
//--------------------------------------------------------------------------------------------------
static PackageDownloaderObj_t PkgDwlObj;

//--------------------------------------------------------------------------------------------------
/**
 * Package Downloader structure pointer
 */
//--------------------------------------------------------------------------------------------------
static lwm2mcore_PackageDownloader_t* PkgDwlPtr = NULL;

//--------------------------------------------------------------------------------------------------
/**
 * DWL parser object instance
 */
//--------------------------------------------------------------------------------------------------
static DwlParserObj_t DwlParserObj;

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
    lwm2mcore_PackageDownloader_t* pkgDwlPtr    ///< Package downloader
)
{
    lwm2mcore_status_t status;

    // Create the status event
    switch (eventId)
    {
        case PKG_DWL_EVENT_DETAILS:
            LOG_ARG("Package download size: %llu bytes", pkgDwlPtr->data.packageSize);
            status.event = LWM2MCORE_EVENT_PACKAGE_DOWNLOAD_DETAILS;
            status.u.pkgStatus.pkgType = PkgDwlObj.packageType;
            status.u.pkgStatus.numBytes = (uint32_t)pkgDwlPtr->data.packageSize;
            status.u.pkgStatus.progress = 0;
            status.u.pkgStatus.errorCode = 0;

            // TODO Check that the device has sufficient space to store the package
            // Need a new porting layer function
            break;

        case PKG_DWL_EVENT_DL_START:
            LOG("Package download start");
            status.event = LWM2MCORE_EVENT_DOWNLOAD_PROGRESS;
            status.u.pkgStatus.pkgType = PkgDwlObj.packageType;
            status.u.pkgStatus.numBytes = 0;
            status.u.pkgStatus.progress = 0;
            status.u.pkgStatus.errorCode = 0;
            break;

        case PKG_DWL_EVENT_DL_PROGRESS:
            LOG_ARG("Package download progress: %llu bytes, %u%%",
                    PkgDwlObj.offset, PkgDwlObj.downloadProgress);

            if (   (100 < PkgDwlObj.downloadProgress)
                || (pkgDwlPtr->data.packageSize < PkgDwlObj.offset)
               )
            {
                // Incoherent download progress
                return;
            }

            status.event = LWM2MCORE_EVENT_DOWNLOAD_PROGRESS;
            status.u.pkgStatus.pkgType = PkgDwlObj.packageType;
            status.u.pkgStatus.numBytes = (uint32_t)PkgDwlObj.offset;
            status.u.pkgStatus.progress = PkgDwlObj.downloadProgress;
            status.u.pkgStatus.errorCode = 0;
            break;

        case PKG_DWL_EVENT_DL_END:
            // Determine download status with update result
            switch (PkgDwlObj.updateResult)
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
                    LOG_ARG("Unknown update result %d", PkgDwlObj.updateResult);
                    status.event = LWM2MCORE_EVENT_PACKAGE_DOWNLOAD_FAILED;
                    status.u.pkgStatus.errorCode = LWM2MCORE_FUMO_ALTERNATE_DL_ERROR;
                    break;
            }
            status.u.pkgStatus.pkgType = PkgDwlObj.packageType;
            status.u.pkgStatus.numBytes = (uint32_t)PkgDwlObj.offset;
            status.u.pkgStatus.progress = PkgDwlObj.downloadProgress;

            LOG_ARG("Package download end: event %d, errorCode %d",
                    status.event, status.u.pkgStatus.errorCode);
            break;

        case PKG_DWL_EVENT_SIGN_OK:
            LOG("Signature check successful");
            status.event = LWM2MCORE_EVENT_PACKAGE_CERTIFICATION_OK;
            status.u.pkgStatus.pkgType = PkgDwlObj.packageType;
            break;

        case PKG_DWL_EVENT_SIGN_KO:
            LOG("Signature check failed");
            status.event = LWM2MCORE_EVENT_PACKAGE_CERTIFICATION_NOT_OK;
            status.u.pkgStatus.pkgType = PkgDwlObj.packageType;
            break;

        case PKG_DWL_EVENT_UPDATE_START:
            LOG("Package update is launched");
            status.event = LWM2MCORE_EVENT_UPDATE_STARTED;
            status.u.pkgStatus.pkgType = PkgDwlObj.packageType;
            break;

        case PKG_DWL_EVENT_UPDATE_SUCCESS:
            LOG("Package update successful");
            status.event = LWM2MCORE_EVENT_UPDATE_FINISHED;
            status.u.pkgStatus.pkgType = PkgDwlObj.packageType;
            break;

        case PKG_DWL_EVENT_UPDATE_FAILURE:
            LOG("Package update failed");
            status.event = LWM2MCORE_EVENT_UPDATE_FAILED;
            status.u.pkgStatus.pkgType = PkgDwlObj.packageType;
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
 * Hash data if necessary, based on the current DWL section/subsection:
 * - compute CRC32
 * - compute SHA1 digest
 *
 * @return
 *  - DWL_OK      The function succeeded
 *  - DWL_FAULT   The function failed
 */
//--------------------------------------------------------------------------------------------------
static lwm2mcore_DwlResult_t HashData
(
    void
)
{
    // Initialize SHA1 context and CRC if not already done
    if (!DwlParserObj.sha1CtxPtr)
    {
        if (LWM2MCORE_ERR_COMPLETED_OK != os_portSecuritySha1Start(&DwlParserObj.sha1CtxPtr))
        {
            LOG("Unable to initialize SHA1 context");
            PkgDwlObj.updateResult = LWM2MCORE_FW_UPDATE_RESULT_VERIFY_ERROR;
            return DWL_FAULT;
        }

        // Initialize computed CRC
        DwlParserObj.computedCRC = os_portSecurityCrc32(0L, NULL, 0);
    }

    // Some parts of the DWL data are excluded from the CRC computation and/or the SHA1 digest
    switch (DwlParserObj.section)
    {
        case DWL_TYPE_UPCK:
            if (DWL_SUB_PROLOG == DwlParserObj.subsection)
            {
                // Compute CRC starting from fileSize in UPCK DWL prolog,
                // ignore DWLF magic, file size, CRC
                DwlProlog_t* dwlPrologPtr = (DwlProlog_t*)DwlParserObj.dataToParsePtr;
                size_t prologSizeForCrc = sizeof(DwlProlog_t) - (3 * sizeof(uint32_t));
                DwlParserObj.computedCRC = os_portSecurityCrc32(DwlParserObj.computedCRC,
                                                                (uint8_t*)&dwlPrologPtr->fileSize,
                                                                prologSizeForCrc);
            }
            else
            {
                // All other UPCK subsections are used for CRC computation
                DwlParserObj.computedCRC = os_portSecurityCrc32(DwlParserObj.computedCRC,
                                                                DwlParserObj.dataToParsePtr,
                                                                PkgDwlObj.processedLen);
            }

            // SHA1 digest is updated with all UPCK data
            if (LWM2MCORE_ERR_COMPLETED_OK!=os_portSecuritySha1Process(DwlParserObj.sha1CtxPtr,
                                                                      DwlParserObj.dataToParsePtr,
                                                                      PkgDwlObj.processedLen))
            {
                LOG("Unable to update SHA1 digest");
                PkgDwlObj.updateResult = LWM2MCORE_FW_UPDATE_RESULT_VERIFY_ERROR;
                return DWL_FAULT;
            }

            break;

        case DWL_TYPE_BINA:
            // All BINA subsections are used for CRC computation
            DwlParserObj.computedCRC = os_portSecurityCrc32(DwlParserObj.computedCRC,
                                                            DwlParserObj.dataToParsePtr,
                                                            PkgDwlObj.processedLen);

            // SHA1 digest is updated with all BINA data
            if (LWM2MCORE_ERR_COMPLETED_OK!=os_portSecuritySha1Process(DwlParserObj.sha1CtxPtr,
                                                                      DwlParserObj.dataToParsePtr,
                                                                      PkgDwlObj.processedLen))
            {
                LOG("Unable to update SHA1 digest");
                PkgDwlObj.updateResult = LWM2MCORE_FW_UPDATE_RESULT_VERIFY_ERROR;
                return DWL_FAULT;
            }
            break;

        case DWL_TYPE_SIGN:
            // Whole SIGN section is ignored for CRC computation and SHA1 digest
            break;

        default:
            LOG_ARG("Unknown DWL section 0x%08x", DwlParserObj.section);
            PkgDwlObj.updateResult = LWM2MCORE_FW_UPDATE_RESULT_UNSUPPORTED_PKG_TYPE;
            return DWL_FAULT;
            break;
    }

    return DWL_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Check the package integrity:
 * - compare the computed CRC with the package CRC
 * - check if the computed SHA1 digest matches the package signature
 *
 * @return
 *  - DWL_OK      The function succeeded
 *  - DWL_FAULT   The function failed
 */
//--------------------------------------------------------------------------------------------------
static lwm2mcore_DwlResult_t CheckCrcAndSignature
(
    void
)
{
    // Compare package CRC retrieved from first DWL prolog and computed CRC.
    if (DwlParserObj.packageCRC != DwlParserObj.computedCRC)
    {
        LOG_ARG("Incorrect CRC: expected 0x%08x, computed 0x%08x",
                DwlParserObj.packageCRC, DwlParserObj.computedCRC);
        PkgDwlObj.updateResult = LWM2MCORE_FW_UPDATE_RESULT_VERIFY_ERROR;
        return DWL_FAULT;
    }

    // Verify package signature
    if (LWM2MCORE_ERR_COMPLETED_OK != os_portSecuritySha1End(DwlParserObj.sha1CtxPtr,
                                                             PkgDwlObj.packageType,
                                                             DwlParserObj.dataToParsePtr,
                                                             PkgDwlObj.processedLen))
    {
        LOG("Incorrect package signature");
        PkgDwlObj.updateResult = LWM2MCORE_FW_UPDATE_RESULT_VERIFY_ERROR;
        return DWL_FAULT;
    }

    return DWL_OK;
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
    void
)
{
    lwm2mcore_DwlResult_t result;
    DwlProlog_t* dwlPrologPtr = (DwlProlog_t*)DwlParserObj.dataToParsePtr;

    // Check DWL magic number
    if (DWL_MAGIC_NUMBER != dwlPrologPtr->magicNumber)
    {
        LOG_ARG("Unknown package format, magic number 0x%08x", dwlPrologPtr->magicNumber);
        PkgDwlObj.updateResult = LWM2MCORE_FW_UPDATE_RESULT_UNSUPPORTED_PKG_TYPE;
        return DWL_FAULT;
    }

    // Store current DWL section
    DwlParserObj.section = dwlPrologPtr->dataType;
    LOG_ARG("Parse new DWL section '%C%C%C%C'",
            DwlParserObj.section & 0xFF,
            (DwlParserObj.section >> 8) & 0xFF,
            (DwlParserObj.section >> 16) & 0xFF,
            (DwlParserObj.section >> 24) & 0xFF);

    // The whole DWL prolog is processed
    PkgDwlObj.processedLen = DwlParserObj.lenToParse;

    // Hash the prolog data
    result = HashData();
    if (DWL_OK != result)
    {
        // updateResult is already set by HashData
        return result;
    }

    // Store necessary data and determine next awaited subsection
    switch (DwlParserObj.section)
    {
        case DWL_TYPE_UPCK:
            // Store prolog data
            DwlParserObj.commentSize = (dwlPrologPtr->commentSize << 3);
            DwlParserObj.packageCRC = dwlPrologPtr->crc32;
            LOG_ARG("Package CRC: 0x%08x", DwlParserObj.packageCRC);

            // Parse DWL comments
            PkgDwlObj.state = PKG_DWL_PARSE;
            DwlParserObj.subsection = DWL_SUB_COMMENTS;
            DwlParserObj.lenToParse = DwlParserObj.commentSize;
            break;

        case DWL_TYPE_BINA:
            // Store prolog data
            DwlParserObj.commentSize = (dwlPrologPtr->commentSize << 3);
            DwlParserObj.binarySize = dwlPrologPtr->fileSize
                                      - DwlParserObj.commentSize
                                      - LWM2MCORE_BINA_HEADER_SIZE
                                      - sizeof(DwlProlog_t);
            DwlParserObj.paddingSize = ((dwlPrologPtr->fileSize + 7) & 0xFFFFFFF8)
                                       - dwlPrologPtr->fileSize;

            // Parse DWL comments
            PkgDwlObj.state = PKG_DWL_PARSE;
            DwlParserObj.subsection = DWL_SUB_COMMENTS;
            DwlParserObj.lenToParse = DwlParserObj.commentSize;
            break;

        case DWL_TYPE_SIGN:
            // Store prolog data
            DwlParserObj.commentSize = (dwlPrologPtr->commentSize << 3);
            DwlParserObj.signatureSize = dwlPrologPtr->fileSize
                                         - DwlParserObj.commentSize
                                         - sizeof(DwlProlog_t);

            // Parse DWL comments
            PkgDwlObj.state = PKG_DWL_PARSE;
            DwlParserObj.subsection = DWL_SUB_COMMENTS;
            DwlParserObj.lenToParse = DwlParserObj.commentSize;
            break;

        default:
            LOG_ARG("Unexpected DWL prolog for section type 0x%08x", DwlParserObj.section);
            PkgDwlObj.updateResult = LWM2MCORE_FW_UPDATE_RESULT_UNSUPPORTED_PKG_TYPE;
            result = DWL_FAULT;
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
    void
)
{
    lwm2mcore_DwlResult_t result = DWL_OK;

    LOG_ARG("Parse DWL comments, length %u", DwlParserObj.lenToParse);

    // The comment section is processed
    PkgDwlObj.processedLen = DwlParserObj.lenToParse;

    // Check if the comment section is not empty
    if (0 != DwlParserObj.lenToParse)
    {
        LOG_ARG("DWL comments: %s", PkgDwlObj.dwlDataPtr);

        // Hash the comments data
        result = HashData();
        if (DWL_OK != result)
        {
            // updateResult is already set by HashData
            return result;
        }
    }

    // Determine next awaited subsection
    switch (DwlParserObj.section)
    {
        case DWL_TYPE_UPCK:
            // Parse UPCK header
            PkgDwlObj.state = PKG_DWL_PARSE;
            DwlParserObj.subsection = DWL_SUB_HEADER;
            DwlParserObj.lenToParse = LWM2MCORE_UPCK_HEADER_SIZE;
            break;

        case DWL_TYPE_BINA:
            // Parse BINA header
            PkgDwlObj.state = PKG_DWL_PARSE;
            DwlParserObj.subsection = DWL_SUB_HEADER;
            DwlParserObj.lenToParse = LWM2MCORE_BINA_HEADER_SIZE;
            break;

        case DWL_TYPE_SIGN:
            // Parse signature
            PkgDwlObj.state = PKG_DWL_PARSE;
            DwlParserObj.subsection = DWL_SUB_SIGNATURE;
            DwlParserObj.lenToParse = DwlParserObj.signatureSize;
            break;

        default:
            LOG_ARG("Unexpected DWL comments for section type 0x%08x", DwlParserObj.section);
            PkgDwlObj.updateResult = LWM2MCORE_FW_UPDATE_RESULT_UNSUPPORTED_PKG_TYPE;
            result = DWL_FAULT;
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
    void
)
{
    lwm2mcore_DwlResult_t result;

    LOG_ARG("Parse DWL header, length %u", DwlParserObj.lenToParse);

    // The header section is processed
    PkgDwlObj.processedLen = DwlParserObj.lenToParse;

    // Hash the header data
    result = HashData();
    if (DWL_OK != result)
    {
        // updateResult is already set by HashData
        return result;
    }

    // Parse header and determine next awaited subsection
    switch (DwlParserObj.section)
    {
        case DWL_TYPE_UPCK:
        {
            // Check UPCK type
            uint32_t upckType =
                     ((UpckHeader_t*)DwlParserObj.dataToParsePtr)->structHeader.upckType;
            if (   (LWM2MCORE_UPCK_TYPE_FW != upckType)
                && (LWM2MCORE_UPCK_TYPE_AMSS != upckType)
               )
            {
                LOG_ARG("Incorrect Update Package type %u", upckType);
                PkgDwlObj.updateResult = LWM2MCORE_FW_UPDATE_RESULT_UNSUPPORTED_PKG_TYPE;
                return DWL_FAULT;
            }

            // Parse next DWL prolog
            PkgDwlObj.state = PKG_DWL_PARSE;
            DwlParserObj.subsection = DWL_SUB_PROLOG;
            DwlParserObj.lenToParse = sizeof(DwlProlog_t);
        }
        break;

        case DWL_TYPE_BINA:
            // Parse DWL binary data
            PkgDwlObj.state = PKG_DWL_PARSE;
            DwlParserObj.subsection = DWL_SUB_BINARY;
            DwlParserObj.lenToParse = DwlParserObj.binarySize;
            DwlParserObj.remainingBinaryData = DwlParserObj.binarySize;
            break;

        default:
            LOG_ARG("Unexpected DWL header for section type 0x%08x", DwlParserObj.section);
            PkgDwlObj.updateResult = LWM2MCORE_FW_UPDATE_RESULT_UNSUPPORTED_PKG_TYPE;
            result = DWL_FAULT;
            break;
    }

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
    void
)
{
    lwm2mcore_DwlResult_t result;

    // Check if subsection is expected in current DWL section
    if (DWL_TYPE_BINA != DwlParserObj.section)
    {
        LOG_ARG("Unexpected DWL binary data for section type 0x%08x", DwlParserObj.section);
        PkgDwlObj.updateResult = LWM2MCORE_FW_UPDATE_RESULT_UNSUPPORTED_PKG_TYPE;
        return DWL_FAULT;
    }

    // The binary data are processed
    PkgDwlObj.processedLen = DwlParserObj.lenToParse;
    DwlParserObj.remainingBinaryData -= DwlParserObj.lenToParse;

    // Hash the binary data
    result = HashData();
    if (DWL_OK != result)
    {
        // updateResult is already set by HashData
        return result;
    }

    // Store downloaded binary data
    PkgDwlObj.state = PKG_DWL_STORE;

    // Check if all binary data is received
    if (0 == DwlParserObj.remainingBinaryData)
    {
        // End of binary data, prepare download of DWL padding data
        DwlParserObj.subsection = DWL_SUB_PADDING;
        DwlParserObj.lenToParse = DwlParserObj.paddingSize;
    }

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
    void
)
{
    lwm2mcore_DwlResult_t result;

    LOG_ARG("Parse DWL padding, length %u", PkgDwlObj.processedLen);

    // Check if subsection is expected in current DWL section
    if (DWL_TYPE_BINA != DwlParserObj.section)
    {
        LOG_ARG("Unexpected DWL padding data for section type 0x%08x", DwlParserObj.section);
        PkgDwlObj.updateResult = LWM2MCORE_FW_UPDATE_RESULT_UNSUPPORTED_PKG_TYPE;
        return DWL_FAULT;
    }

    // The padding section is processed
    PkgDwlObj.processedLen = DwlParserObj.lenToParse;

    // Hash the padding data
    result = HashData();
    if (DWL_OK != result)
    {
        // updateResult is already set by HashData
        return result;
    }

    // Parse next DWL prolog
    PkgDwlObj.state = PKG_DWL_PARSE;
    DwlParserObj.subsection = DWL_SUB_PROLOG;
    DwlParserObj.lenToParse = sizeof(DwlProlog_t);

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
    void
)
{
    lwm2mcore_DwlResult_t result;

    LOG_ARG("Parse DWL signature, length %u", DwlParserObj.lenToParse);

    // Check if subsection is expected in current DWL section
    if (DWL_TYPE_SIGN != DwlParserObj.section)
    {
        LOG_ARG("Unexpected DWL signature for section type 0x%08x", DwlParserObj.section);
        PkgDwlObj.updateResult = LWM2MCORE_FW_UPDATE_RESULT_UNSUPPORTED_PKG_TYPE;
        return DWL_FAULT;
    }

    // The padding section is processed
    PkgDwlObj.processedLen = DwlParserObj.lenToParse;

    // The signature subsection is ignored for CRC and SHA1 digest computation,
    // no need to hash the data

    // Check the package CRC and verify the signature
    result = CheckCrcAndSignature();
    if (DWL_OK != result)
    {
        // updateResult is already set by CheckCrcAndSignature
        return result;
    }

    // End of file
    PkgDwlObj.state = PKG_DWL_END;

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
    void
)
{
    lwm2mcore_DwlResult_t result;

    // Check if there are data to parse
    if (!DwlParserObj.dataToParsePtr)
    {
        LOG("NULL data pointer in DWL parser");
        PkgDwlObj.updateResult = LWM2MCORE_FW_UPDATE_RESULT_COMMUNICATION_ERROR;
        return DWL_FAULT;
    }

    // Parse the downloaded data based on the current subsection
    switch (DwlParserObj.subsection)
    {
        case DWL_SUB_PROLOG:
            result = ParseDwlProlog();
            break;

        case DWL_SUB_COMMENTS:
            result = ParseDwlComments();
            break;

        case DWL_SUB_HEADER:
            result = ParseDwlHeader();
            break;

        case DWL_SUB_BINARY:
            result = ParseDwlBinary();
            break;

        case DWL_SUB_PADDING:
            result = ParseDwlPadding();
            break;

        case DWL_SUB_SIGNATURE:
            result = ParseDwlSignature();
            break;

        default:
            LOG_ARG("Unknown DWL subsection %u", DwlParserObj.subsection);
            PkgDwlObj.updateResult = LWM2MCORE_FW_UPDATE_RESULT_UNSUPPORTED_PKG_TYPE;
            result = DWL_FAULT;
            break;
    }

    // Check if the DWL parsing is finished
    if ((DWL_OK != result) || (PKG_DWL_END == PkgDwlObj.state))
    {
        // Cancel the SHA1 computation and reset SHA1 context
        if (LWM2MCORE_ERR_COMPLETED_OK != os_portSecuritySha1Cancel(&DwlParserObj.sha1CtxPtr))
        {
            LOG("Unable to reset SHA1 context");
        }

        // Reset the DWL parser object for next use
        memset(&DwlParserObj, 0, sizeof(DwlParserObj_t));
        DwlParserObj.subsection = DWL_SUB_PROLOG;
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Buffer the downloaded data if necessary in order to parse it.
 *
 * This function checks if the downloaded data should be buffered before being parsed by the DWL
 * parser. The next awaited DWL subsection should indeed be fully downloaded before being processed,
 * except for the binary data subsection.
 *
 * @return
 *  - DWL_OK      The function succeeded
 *  - DWL_FAULT   The function failed
 */
//--------------------------------------------------------------------------------------------------
static lwm2mcore_DwlResult_t BufferAndSetDataToParse
(
    bool* parseData     ///< Indicates if the data should be parsed
)
{
    // Check input argument
    if (!parseData)
    {
        LOG("Null pointer passed to BufferAndSetDataToParse");
        return DWL_FAULT;
    }

    // Data will not be parsed by default
    *parseData = false;

    // The binary data subsection can handle any length
    if (DWL_SUB_BINARY == DwlParserObj.subsection)
    {
        // Check if all the data are binary data
        if (PkgDwlObj.downloadedLen <= DwlParserObj.remainingBinaryData)
        {
            DwlParserObj.lenToParse = PkgDwlObj.downloadedLen;
        }
        else
        {
            DwlParserObj.lenToParse = DwlParserObj.remainingBinaryData;
        }

        // Parse downloaded data with the correct length
        *parseData = true;
        DwlParserObj.dataToParsePtr = PkgDwlObj.dwlDataPtr;
        return DWL_OK;
    }

    // Check if enough data are received for the next DWL subsection
    if ((PkgDwlObj.tmpDataLen + PkgDwlObj.downloadedLen) < DwlParserObj.lenToParse)
    {
        if ((PkgDwlObj.tmpDataLen + PkgDwlObj.downloadedLen) <= TMP_DATA_MAX_LEN)
        {
            // Store the data to use it later
            memcpy(&PkgDwlObj.tmpData[PkgDwlObj.tmpDataLen],
                   PkgDwlObj.dwlDataPtr,
                   PkgDwlObj.downloadedLen);
            PkgDwlObj.tmpDataLen += PkgDwlObj.downloadedLen;
            PkgDwlObj.processedLen = PkgDwlObj.downloadedLen;
            return DWL_OK;
        }
        else
        {
            LOG_ARG("Unable to store %zu bytes in temporary buffer, contains %d, max = %d",
                    PkgDwlObj.downloadedLen, PkgDwlObj.tmpDataLen, TMP_DATA_MAX_LEN);
            PkgDwlObj.updateResult = LWM2MCORE_FW_UPDATE_RESULT_COMMUNICATION_ERROR;
            PkgDwlObj.state = PKG_DWL_ERROR;
            return DWL_FAULT;
        }
    }
    // Enough data for the next DWL subsection, check if the temporary buffer is being used
    if (PkgDwlObj.tmpDataLen)
    {
        if ((PkgDwlObj.tmpDataLen + DwlParserObj.lenToParse) <= TMP_DATA_MAX_LEN)
        {
            // Copy the required data into the temporary buffer
            uint32_t lenToCopy = (DwlParserObj.lenToParse - PkgDwlObj.tmpDataLen);
            memcpy(&PkgDwlObj.tmpData[PkgDwlObj.tmpDataLen],
                   PkgDwlObj.dwlDataPtr,
                   lenToCopy);
            PkgDwlObj.tmpDataLen += lenToCopy;

            // Update the downloaded data pointer
            PkgDwlObj.dwlDataPtr += lenToCopy;
            PkgDwlObj.downloadedLen -= lenToCopy;

            // Parse the temporary buffer
            *parseData = true;
            DwlParserObj.dataToParsePtr = PkgDwlObj.tmpData;
            return DWL_OK;
        }
        else
        {
            LOG_ARG("Unable to store %zu bytes in temporary buffer, contains %d, max=%d",
                    PkgDwlObj.downloadedLen, PkgDwlObj.tmpDataLen, TMP_DATA_MAX_LEN);
            PkgDwlObj.updateResult = LWM2MCORE_FW_UPDATE_RESULT_COMMUNICATION_ERROR;
            PkgDwlObj.state = PKG_DWL_ERROR;
            return DWL_FAULT;
        }
    }

    // No temporary buffer, the data can be parsed 'as is'
    *parseData = true;
    DwlParserObj.dataToParsePtr = PkgDwlObj.dwlDataPtr;
    return DWL_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Initialize the package download and determine next state
 */
//--------------------------------------------------------------------------------------------------
static void PkgDwlInit
(
    lwm2mcore_PackageDownloader_t* pkgDwlPtr    ///< Package downloader
)
{
    // Initialize download
    PkgDwlObj.result = pkgDwlPtr->initDownload(pkgDwlPtr->data.packageUri, pkgDwlPtr->ctxPtr);
    if (DWL_OK != PkgDwlObj.result)
    {
        LOG("Error during download initialization");
        PkgDwlObj.updateResult = LWM2MCORE_FW_UPDATE_RESULT_COMMUNICATION_ERROR;
        PkgDwlObj.state = PKG_DWL_ERROR;
        return;
    }

    // Set the package type and the update result according to the update type
    switch (pkgDwlPtr->data.updateType)
    {
        case LWM2MCORE_FW_UPDATE_TYPE:
            LOG("Receiving FW package");
            PkgDwlObj.packageType = LWM2MCORE_PKG_FW;
            break;

        case LWM2MCORE_SW_UPDATE_TYPE:
            LOG("Receiving SW package");
            PkgDwlObj.packageType = LWM2MCORE_PKG_SW;
            break;

        default:
            LOG_ARG("Unknown package type %d", pkgDwlPtr->data.updateType);
            PkgDwlObj.updateResult = LWM2MCORE_FW_UPDATE_RESULT_UNSUPPORTED_PKG_TYPE;
            PkgDwlObj.state = PKG_DWL_ERROR;
            return;
    }

    // Set update result to 'Normal' when the updating process is initiated
    PkgDwlObj.updateResult = LWM2MCORE_FW_UPDATE_RESULT_DEFAULT_NORMAL;
    PkgDwlObj.result = pkgDwlPtr->setFwUpdateResult(PkgDwlObj.updateResult);
    if (DWL_OK != PkgDwlObj.result)
    {
        LOG("Unable to set update result");
        PkgDwlObj.updateResult = LWM2MCORE_FW_UPDATE_RESULT_COMMUNICATION_ERROR;
        PkgDwlObj.state = PKG_DWL_ERROR;
        return;
    }

    // Retrieve package information
    PkgDwlObj.state = PKG_DWL_INFO;
}

//--------------------------------------------------------------------------------------------------
/**
 * Retrieve information about the package to download and determine next state
 */
//--------------------------------------------------------------------------------------------------
static void PkgDwlGetInfo
(
    lwm2mcore_PackageDownloader_t* pkgDwlPtr    ///< Package downloader
)
{
    // Get information about the package
    PkgDwlObj.result = pkgDwlPtr->getInfo(&pkgDwlPtr->data, pkgDwlPtr->ctxPtr);
    if (DWL_OK != PkgDwlObj.result)
    {
        LOG("Error while getting the package information");
        PkgDwlObj.updateResult = LWM2MCORE_FW_UPDATE_RESULT_COMMUNICATION_ERROR;
        PkgDwlObj.state = PKG_DWL_ERROR;
        return;
    }

    // Notify the application of the package size
    PkgDwlEvent(PKG_DWL_EVENT_DETAILS, pkgDwlPtr);

    // Download the package
    PkgDwlObj.state = PKG_DWL_DOWNLOAD;
    // Require to parse at least the length of DWL prolog, enough to determine the file type
    memset(&DwlParserObj, 0, sizeof(DwlParserObj_t));
    DwlParserObj.subsection = DWL_SUB_PROLOG;
    DwlParserObj.lenToParse = sizeof(DwlProlog_t);
}

//--------------------------------------------------------------------------------------------------
/**
 * Download the package
 */
//--------------------------------------------------------------------------------------------------
static void PkgDwlDownload
(
    lwm2mcore_PackageDownloader_t* pkgDwlPtr    ///< Package downloader
)
{
    // Notify the download beginning
    // Set update state to 'Downloading'
    PkgDwlObj.result = pkgDwlPtr->setFwUpdateState(LWM2MCORE_FW_UPDATE_STATE_DOWNLOADING);
    if (DWL_OK != PkgDwlObj.result)
    {
        LOG("Unable to set update state");
        PkgDwlObj.updateResult = LWM2MCORE_FW_UPDATE_RESULT_COMMUNICATION_ERROR;
        PkgDwlObj.state = PKG_DWL_ERROR;
        return;
    }

    // Notify the application of the download start
    PkgDwlEvent(PKG_DWL_EVENT_DL_START, pkgDwlPtr);

    // Be ready to parse downloaded data
    PkgDwlObj.state = PKG_DWL_PARSE;

    // Start downloading
    LOG_ARG("Download starting at offset %llu", PkgDwlObj.offset);
    PkgDwlObj.result = pkgDwlPtr->download(PkgDwlObj.offset, pkgDwlPtr->ctxPtr);
    if (DWL_OK != PkgDwlObj.result)
    {
        LOG_ARG("Error during download, result %d", PkgDwlObj.result);
        PkgDwlObj.updateResult = LWM2MCORE_FW_UPDATE_RESULT_COMMUNICATION_ERROR;
        PkgDwlObj.state = PKG_DWL_ERROR;
        return;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Parse downloaded data and determine next state
 */
//--------------------------------------------------------------------------------------------------
static void PkgDwlParse
(
    lwm2mcore_PackageDownloader_t* pkgDwlPtr    ///< Package downloader
)
{
    // Parse downloaded data and determine next state
    PkgDwlObj.result = DwlParser();
    if (DWL_OK != PkgDwlObj.result)
    {
        LOG("Error while parsing the DWL package");
        // No need to change update result, already set by DWL parser
        PkgDwlObj.state = PKG_DWL_ERROR;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Store downloaded data and determine next state
 */
//--------------------------------------------------------------------------------------------------
static void PkgDwlStore
(
    lwm2mcore_PackageDownloader_t* pkgDwlPtr    ///< Package downloader
)
{
    // Store downloaded data
    PkgDwlObj.result = pkgDwlPtr->storeRange(DwlParserObj.dataToParsePtr,
                                             PkgDwlObj.processedLen,
                                             PkgDwlObj.storageOffset,
                                             pkgDwlPtr->ctxPtr);
    if (DWL_OK != PkgDwlObj.result)
    {
        LOG("Error during data storage");
        PkgDwlObj.updateResult = LWM2MCORE_FW_UPDATE_RESULT_OUT_OF_MEMORY;
        PkgDwlObj.state = PKG_DWL_ERROR;
        return;
    }

    // Update storage offset
    PkgDwlObj.storageOffset += PkgDwlObj.processedLen;

    // Parse next downloaded data
    PkgDwlObj.state = PKG_DWL_PARSE;
}

//--------------------------------------------------------------------------------------------------
/**
 * Process package downloader error and determine next state
 */
//--------------------------------------------------------------------------------------------------
static void PkgDwlError
(
    lwm2mcore_PackageDownloader_t* pkgDwlPtr    ///< Package downloader
)
{
    // Error during package downloading
    char error[ERROR_STR_MAX_LEN];
    memset(error, 0, sizeof(error));

    // Only display debug traces for the package downloader error, the error will be notified
    // by the 'download end' event.
    // One exception for the signature check error, which should also be notified by a dedicated
    // event.
    switch (PkgDwlObj.updateResult)
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
            PkgDwlEvent(PKG_DWL_EVENT_SIGN_KO, pkgDwlPtr);
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
            error, PkgDwlObj.updateResult);

    // End of download
    PkgDwlObj.state = PKG_DWL_END;
}

//--------------------------------------------------------------------------------------------------
/**
 * End the download process
 */
//--------------------------------------------------------------------------------------------------
static void PkgDwlEnd
(
    lwm2mcore_PackageDownloader_t* pkgDwlPtr    ///< Package downloader
)
{
    // Check if an error was detected during the package download or parsing
    if (LWM2MCORE_FW_UPDATE_RESULT_DEFAULT_NORMAL != PkgDwlObj.updateResult)
    {
        // Error during download or parsing, set update result accordingly.
        // No need to change the update state, it should remain set to 'Downloading'.
        PkgDwlObj.result = pkgDwlPtr->setFwUpdateResult(PkgDwlObj.updateResult);
        if (DWL_OK != PkgDwlObj.result)
        {
            LOG("Unable to set update result");
        }
    }
    else
    {
        // Notify the application of the signature validation
        PkgDwlEvent(PKG_DWL_EVENT_SIGN_OK, pkgDwlPtr);

        // Successful download: set update state to 'Downloaded'.
        // No need to change the update result, it was already set to 'Normal' at the beginning.
        PkgDwlObj.result = pkgDwlPtr->setFwUpdateState(LWM2MCORE_FW_UPDATE_STATE_DOWNLOADED);
        if (DWL_OK != PkgDwlObj.result)
        {
            LOG("Unable to set update state");
        }
    }

    // Notify the application of the download end
    PkgDwlEvent(PKG_DWL_EVENT_DL_END, pkgDwlPtr);

    // End of download
    PkgDwlObj.result = pkgDwlPtr->endDownload(pkgDwlPtr->ctxPtr);
    if (DWL_OK != PkgDwlObj.result)
    {
        LOG("Error while ending the download");
    }

    // End of processing
    PkgDwlObj.endOfProcessing = true;
}

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
    lwm2mcore_PackageDownloader_t* pkgDwlPtr    ///< Package downloader
)
{
    // Check input parameters
    if (!pkgDwlPtr)
    {
        LOG("No package downloader object");
        return DWL_FAULT;
    }

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

    if (!pkgDwlPtr->setFwUpdateState)
    {
        LOG("Missing firmware update state callback");
        return DWL_FAULT;
    }

    if (!pkgDwlPtr->setFwUpdateResult)
    {
        LOG("Missing firmware update result callback");
        return DWL_FAULT;
    }

    if (!pkgDwlPtr->setSwUpdateState)
    {
        LOG("Missing software update state callback");
        return DWL_FAULT;
    }

    if (!pkgDwlPtr->setSwUpdateResult)
    {
        LOG("Missing software update result callback");
        return DWL_FAULT;
    }

    if (!pkgDwlPtr->download)
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

    // Store the package downloader
    PkgDwlPtr = pkgDwlPtr;

    // Package downloader object initialization
    memset(&PkgDwlObj, 0, sizeof(PackageDownloaderObj_t));
    PkgDwlObj.state = PKG_DWL_INIT;
    PkgDwlObj.updateResult = LWM2MCORE_FW_UPDATE_RESULT_DEFAULT_NORMAL;
    PkgDwlObj.endOfProcessing = false;
    PkgDwlObj.packageType = LWM2MCORE_PKG_NONE;

    // Run the package downloader until end of processing is reached (end of file, error...)
    while (!PkgDwlObj.endOfProcessing)
    {
        // Run the package downloader action based on the current state
        switch (PkgDwlObj.state)
        {
            case PKG_DWL_INIT:
                PkgDwlInit(pkgDwlPtr);
                break;

            case PKG_DWL_INFO:
                PkgDwlGetInfo(pkgDwlPtr);
                break;

            case PKG_DWL_DOWNLOAD:
                PkgDwlDownload(pkgDwlPtr);
                break;

            case PKG_DWL_PARSE:
            case PKG_DWL_STORE:
                // Nothing to do, just wait for the parsing and storing end
                break;

            case PKG_DWL_ERROR:
                PkgDwlError(pkgDwlPtr);
                break;

            case PKG_DWL_END:
                PkgDwlEnd(pkgDwlPtr);
                break;

            default:
                LOG_ARG("Unknown package downloader state %d in Run", PkgDwlObj.state);
                PkgDwlObj.result = DWL_FAULT;
                PkgDwlObj.endOfProcessing = true;
                break;
        }
    }

    return PkgDwlObj.result;
}

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
)
{
    // Check if the necessary callback is correctly set
    if ((!PkgDwlPtr) || (!PkgDwlPtr->storeRange))
    {
        LOG("Missing storing callback");
        return DWL_FAULT;
    }

    // Check downloaded buffer
    if (!bufPtr)
    {
        LOG("Null data pointer");
        return DWL_FAULT;
    }
    if (0 == bufSize)
    {
        LOG("No data to process");
        return DWL_OK;
    }

    // Copy the received data
    PkgDwlObj.dwlDataPtr = bufPtr;
    PkgDwlObj.downloadedLen = bufSize;

    // Parse and store all the received data
    while ((PkgDwlObj.downloadedLen > 0) && (DWL_OK == PkgDwlObj.result))
    {
        switch (PkgDwlObj.state)
        {
            case PKG_DWL_PARSE:
            {
                // Buffer and set data to parse
                bool parseData = false;
                lwm2mcore_DwlResult_t result = BufferAndSetDataToParse(&parseData);
                if ((DWL_OK != result) || (!parseData))
                {
                    return result;
                }
                // Reset processed length
                PkgDwlObj.processedLen = 0;
                // Parse data
                PkgDwlParse(PkgDwlPtr);
            }
            break;

            case PKG_DWL_STORE:
                PkgDwlStore(PkgDwlPtr);
                break;

            default:
                LOG_ARG("Unexpected package downloader state %d in ReceiveData", PkgDwlObj.state);
                PkgDwlObj.result = DWL_FAULT;
                PkgDwlObj.endOfProcessing = true;
                break;
        }

        // Update data pointer and length according to processed data
        // If storing is necessary, processing is not complete yet
        if (PKG_DWL_STORE != PkgDwlObj.state)
        {
            double downloadProgress;

            // Update offset
            PkgDwlObj.offset += PkgDwlObj.processedLen;

            // Compute download progress
            downloadProgress = (100 * PkgDwlObj.offset) / PkgDwlPtr->data.packageSize;

            if (downloadProgress != PkgDwlObj.downloadProgress)
            {
                // Update overall download progress
                PkgDwlObj.downloadProgress = (uint32_t)downloadProgress;
                // Notify the application of the download progress if it changed since last time
                // Note: the downloader has far more information about the progress (e.g. ETA) and
                // a callback could be implemented to retrieve these data.
                PkgDwlEvent(PKG_DWL_EVENT_DL_PROGRESS, PkgDwlPtr);
            }

            if (PkgDwlObj.tmpDataLen)
            {
                // Reset temporary buffer now that it is parsed
                memset(PkgDwlObj.tmpData, 0, TMP_DATA_MAX_LEN);
                PkgDwlObj.tmpDataLen = 0;
            }
            else
            {
                // Update downloaded data pointer
                PkgDwlObj.dwlDataPtr += PkgDwlObj.processedLen;
                PkgDwlObj.downloadedLen -= PkgDwlObj.processedLen;
            }
        }
    }

    return PkgDwlObj.result;
}