//-------------------------------------------------------------------------------------------------
/**
 * @file tests.h
 *
 * Unitary test header file
 *
 * Copyright (C) Sierra Wireless Inc.
 */
//-------------------------------------------------------------------------------------------------

#ifndef TESTS_H_
#define TESTS_H_

#include "CUnit/CUError.h"

//--------------------------------------------------------------------------------------------------
/**
 *  Structure to register tests
 */
//--------------------------------------------------------------------------------------------------
struct TestTable
{
    const char* namePtr;        ///< [IN] Test name
    CU_TestFunc function;       ///< [IN] Test function
};

//--------------------------------------------------------------------------------------------------
/**
 *  Function to add a test table
 */
//--------------------------------------------------------------------------------------------------
CU_ErrorCode addTest
(
    CU_pSuite suitePtr,             ///< [IN] Test suite pointer
    struct TestTable* testTablePTr  ///< [IN] Test functions to be registered
);

//--------------------------------------------------------------------------------------------------
/**
 *  Function to create the test suite related to session manager
 */
//--------------------------------------------------------------------------------------------------
CU_ErrorCode create_SessionManager_suite
(
    void
);

#endif /* TESTS_H_ */
