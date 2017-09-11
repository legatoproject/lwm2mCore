//-------------------------------------------------------------------------------------------------
/**
 * @file tests.c
 *
 * Unitary test entry point
 *
 * Copyright (C) Sierra Wireless Inc.
 */
//-------------------------------------------------------------------------------------------------

#include <stdio.h>
#include <stdint.h>
#include "CUnit/Basic.h"
#include "tests.h"

//--------------------------------------------------------------------------------------------------
/**
 *  Function to add a test table
 */
//--------------------------------------------------------------------------------------------------
CU_ErrorCode addTest
(
    CU_pSuite suitePtr,             ///< [IN] Test suite pointer
    struct TestTable* testTablePtr  ///< [IN] Test functions to be registered
)
{
    int loop;

    /* Treat the registered test table */
    for (loop = 0; (NULL != testTablePtr) && (NULL != testTablePtr[loop].namePtr); ++loop)
    {
        /* Creates a new test having the specified name and test function */
        if (NULL == CU_add_test(suitePtr, testTablePtr[loop].namePtr, testTablePtr[loop].function))
        {
            fprintf(stderr, "Failed to add test %s\n", testTablePtr[loop].namePtr);
            return CU_get_error();
        }
    }
    return CUE_SUCCESS;
}

//--------------------------------------------------------------------------------------------------
/**
 *  Function to exit from the test environment
 */
//--------------------------------------------------------------------------------------------------
static CU_ErrorCode CleanTestEnvironment
(
    void
)
{
    /* Test end: clean up and release memory used by the framework */
    CU_cleanup_registry();

    return CU_get_error();
}

//--------------------------------------------------------------------------------------------------
/**
 *  Unitary test entry point
 */
//--------------------------------------------------------------------------------------------------
int main
(
    void
)
{
    /* Initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
    {
        return CU_get_error();
    }

    /* Create the session manager test suite */
    if (CUE_SUCCESS != create_SessionManager_suite())
    {
        /* Test error: clean up and release memory used by the framework */
        return CleanTestEnvironment();
    }

    /* Set the run mode for the basic interface */
    CU_basic_set_mode(CU_BRM_VERBOSE);

    /* Run all registered CUnit tests using the basic interface */
    CU_basic_run_tests();

    /* Test end: clean up and release memory used by the framework */
    return CleanTestEnvironment();
}
