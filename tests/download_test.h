/**
 * @file download_test.h
 *
 * Header file for test function for download tests
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#ifndef __TESTS_DOWNLOAD_TEST_H__
#define __TESTS_DOWNLOAD_TEST_H__

#include <stdint.h>
#include <stdio.h>

//--------------------------------------------------------------------------------------------------
/**
 *  Function to set the returned result of lwm2mcore_ConnectForDownload function
 */
//--------------------------------------------------------------------------------------------------
void test_setConnectForDownloadResult
(
    lwm2mcore_Sid_t result
);

//--------------------------------------------------------------------------------------------------
/**
 *  Function to get the returned result of lwm2mcore_ConnectForDownload function
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t test_getConnectForDownloadResult
(
    void
);

//--------------------------------------------------------------------------------------------------
/**
 * Reset the counter of lwm2mcore_ConnectForDownload calls
 */
//--------------------------------------------------------------------------------------------------
void test_resetCallNumberConnectForDownload
(
    void
);

//--------------------------------------------------------------------------------------------------
/**
 * Get the counter of lwm2mcore_ConnectForDownload calls
 */
//--------------------------------------------------------------------------------------------------
uint32_t test_getCallNumberConnectForDownload
(
    void
);

//--------------------------------------------------------------------------------------------------
/**
 *  Function to set the returned result of lwm2mcore_ReadForDownload function
 */
//--------------------------------------------------------------------------------------------------
void test_setReadForDownloadResult
(
    lwm2mcore_Sid_t result
);

//--------------------------------------------------------------------------------------------------
/**
 *  Function to get the returned result of lwm2mcore_ReadForDownload function
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t test_getReadForDownloadResult
(
    void
);

#endif /* __TESTS_DOWNLOAD_TEST_H__ */
