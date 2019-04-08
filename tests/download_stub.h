/**
 * @file download_stub.h
 *
 * stub for package download
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#ifndef __TESTS_DOWNLOAD_STUB_H__
#define __TESTS_DOWNLOAD_STUB_H__
#include <stdint.h>
#include <stdio.h>

//--------------------------------------------------------------------------------------------------
/**
 * Macro definition for assert.
 */
//--------------------------------------------------------------------------------------------------
#define TEST_FATAL(formatString, ...) \
        { printf(formatString, ##__VA_ARGS__); exit(EXIT_FAILURE); }

#define TEST_ASSERT(condition) \
        if (!(condition)) { TEST_FATAL("Assert Failed: '%s'\n", #condition) }


//--------------------------------------------------------------------------------------------------
/**
 * Test function called by tests.c in order to select a file name
 *
 * @return
 *  - true  on success
 *  - false on failure
 */
//--------------------------------------------------------------------------------------------------
bool test_setFileNameForPackageDownload
(
    const char*     fileNamePrefixPtr   ///<[IN] File name prefix
);

#endif /* __TESTS_DOWNLOAD_STUB_H__ */
