/**
 * @file download_test.c
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
 *  Returned result of lwm2mcore_ConnectForDownload function
 */
//--------------------------------------------------------------------------------------------------
static lwm2mcore_Sid_t Result_lwm2mcore_ConnectForDownload = LWM2MCORE_ERR_COMPLETED_OK;

//--------------------------------------------------------------------------------------------------
/**
 * Value to indicate how many times lwm2mcore_ConnectForDownload is called
 */
//--------------------------------------------------------------------------------------------------
uint32_t CallNumberForConnectForDownload = 0;

//--------------------------------------------------------------------------------------------------
/**
 *  Returned result of lwm2mcore_ReadForDownload function
 */
//--------------------------------------------------------------------------------------------------
static lwm2mcore_Sid_t Result_lwm2mcore_ReadForDownload = LWM2MCORE_ERR_COMPLETED_OK;

//--------------------------------------------------------------------------------------------------
/**
 *  Function to set the returned result of lwm2mcore_ConnectForDownload function
 */
//--------------------------------------------------------------------------------------------------
void test_setConnectForDownloadResult
(
    lwm2mcore_Sid_t result
)
{
    Result_lwm2mcore_ConnectForDownload = result;
}

//--------------------------------------------------------------------------------------------------
/**
 *  Function to get the returned result of lwm2mcore_ConnectForDownload function
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t test_getConnectForDownloadResult
(
    void
)
{
    CallNumberForConnectForDownload++;
    return Result_lwm2mcore_ConnectForDownload;
}

//--------------------------------------------------------------------------------------------------
/**
 * Reset the counter for lwm2mcore_ConnectForDownload calls
 */
//--------------------------------------------------------------------------------------------------
void test_resetCallNumberConnectForDownload
(
    void
)
{
    CallNumberForConnectForDownload = 0;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the counter of lwm2mcore_ConnectForDownload calls
 */
//--------------------------------------------------------------------------------------------------
uint32_t test_getCallNumberConnectForDownload
(
    void
)
{
    return CallNumberForConnectForDownload;
}

//--------------------------------------------------------------------------------------------------
/**
 *  Function to set the returned result of lwm2mcore_ReadForDownload function
 */
//--------------------------------------------------------------------------------------------------
void test_setReadForDownloadResult
(
    lwm2mcore_Sid_t result
)
{
    Result_lwm2mcore_ReadForDownload = result;
}

//--------------------------------------------------------------------------------------------------
/**
 *  Function to get the returned result of lwm2mcore_ReadForDownload function
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t test_getReadForDownloadResult
(
    void
)
{
    return Result_lwm2mcore_ReadForDownload;
}
