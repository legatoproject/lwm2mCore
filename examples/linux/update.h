/**
 * @file update.h
 *
 * Porting layer for Firmware Over The Air update
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#ifndef _LINUX_CLIENT_UPDATE_H_

//--------------------------------------------------------------------------------------------------
/**
 * Start the download
 */
//--------------------------------------------------------------------------------------------------
void ClientStartDownload
(
    lwm2mcore_UpdateType_t  type,           ///< [IN] Update type
    uint64_t                packageSize,    ///< [IN] Package size
    bool                    isResume        ///< [IN] Indicates if it's a download resume
);

#endif /* _LINUX_CLIENT_UPDATE_H_ */
