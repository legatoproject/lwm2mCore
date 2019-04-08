/**
 * @file updateAgent.h
 *
 * LwM2MCore update agent definitions
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#ifndef UPDATEAGENT_H
#define UPDATEAGENT_H

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
 * Timer value for package download timer in seconds
 */
//--------------------------------------------------------------------------------------------------
#define DOWNLOADER_PACKAGE_TIMER_VALUE  2

//--------------------------------------------------------------------------------------------------
// Function definitions
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Function to initialize a package update
 *
 * @note
 * This function is not available if @c LWM2M_EXTERNAL_DOWNLOADER compilation flag is embedded
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK on success
 *  - @ref LWM2MCORE_ERR_INVALID_ARG when the package URL is not valid
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR on failure
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t downloader_InitializeDownload
(
    lwm2mcore_UpdateType_t type,    ///< [IN] Update type
    uint16_t instanceId,            ///< [IN] Instance Id (0 for FW, any value for SW)
    char* bufferPtr,                ///< [INOUT] data buffer
    size_t len                      ///< [IN] length of input buffer
);

//--------------------------------------------------------------------------------------------------
/**
 *  Handler to launch the package size request
 *
 * @note
 * This function is not available if @c LWM2M_EXTERNAL_DOWNLOADER compilation flag is embedded
 */
//--------------------------------------------------------------------------------------------------
void downloader_PackageDownloadHandler
(
    void
);

#endif /* !LWM2M_EXTERNAL_DOWNLOADER */

//--------------------------------------------------------------------------------------------------
/**
 * Function to get an update result
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK on success
 *  - @ref LWM2MCORE_ERR_INVALID_ARG when parameter is invalid
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR on failure
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t downloader_GetFwUpdateResult
(
    lwm2mcore_FwUpdateResult_t* resultPtr   ///< [IN] FW update result
);

//--------------------------------------------------------------------------------------------------
/**
 * Function to get an update state
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK on success
 *  - @ref LWM2MCORE_ERR_INVALID_ARG when parameter is invalid
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR on failure
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t downloader_GetFwUpdateState
(
    lwm2mcore_FwUpdateState_t*  statePtr    ///< [IN] FW update state
);

//--------------------------------------------------------------------------------------------------
/**
 * Function to set an update state
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK on success
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR on failure
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t downloader_SetFwUpdateState
(
    lwm2mcore_FwUpdateState_t   state   ///< [IN] FW update state
);

//--------------------------------------------------------------------------------------------------
/**
 * Function to set an update result
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK on success
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR on failure
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t downloader_SetFwUpdateResult
(
    lwm2mcore_FwUpdateResult_t   result ///< [IN] FW update result
);

#ifndef LWM2M_EXTERNAL_DOWNLOADER
//--------------------------------------------------------------------------------------------------
/**
 * Resume a package download
 *
 * This function is called in a dedicated thread/task.
 *
 * @return
 *  - LWM2MCORE_ERR_COMPLETED_OK on success
 *  - LWM2MCORE_ERR_INVALID_STATE if no package download is suspended
 *  - LWM2MCORE_ERR_GENERAL_ERROR on failure
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t downloadManager_ResumePackageDownloader
(
    void
);
#endif /* !LWM2M_EXTERNAL_DOWNLOADER */

#endif /* UPDATEAGENT_H */
