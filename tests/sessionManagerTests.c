//-------------------------------------------------------------------------------------------------
/**
 * @file sessionManagerTests.c
 *
 * Unitary test for sessionManager/lwm2mcoreSession.c file
 *
 * Copyright (C) Sierra Wireless Inc.
 */
//-------------------------------------------------------------------------------------------------

#include "tests.h"
#include "CUnit/Basic.h"
#include "internals.h"
#include "liblwm2m.h"
#include <lwm2mcore/lwm2mcore.h>
#include <objectManager/objects.h>

//--------------------------------------------------------------------------------------------------
/**
 *  Event handler for LWM2MCore events
 */
//--------------------------------------------------------------------------------------------------
static int EventHandler
(
    lwm2mcore_Status_t status              ///< [IN] event status
)
{
    int result = 0;

    switch (status.event)
    {
        case LWM2MCORE_EVENT_SESSION_STARTED:
            break;

        case LWM2MCORE_EVENT_SESSION_FAILED:
            break;

        case LWM2MCORE_EVENT_SESSION_FINISHED:
            break;

        case LWM2MCORE_EVENT_LWM2M_SESSION_TYPE_START:
            break;

        case LWM2MCORE_EVENT_PACKAGE_DOWNLOAD_DETAILS:
        case LWM2MCORE_EVENT_DOWNLOAD_PROGRESS:
        case LWM2MCORE_EVENT_PACKAGE_DOWNLOAD_FINISHED:
        case LWM2MCORE_EVENT_PACKAGE_DOWNLOAD_FAILED:
        case LWM2MCORE_EVENT_UPDATE_STARTED:
        case LWM2MCORE_EVENT_UPDATE_FINISHED:
        case LWM2MCORE_EVENT_UPDATE_FAILED:
            break;

        default:
            break;
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 *  Test function for lwm2mcore_Init API
 */
//--------------------------------------------------------------------------------------------------
static void test_lwm2mcore_Init
(
    void
)
{
    lwm2mcore_Ref_t lwm2mcoreRef = NULL;

    CU_ASSERT_EQUAL(lwm2mcore_Init(NULL), NULL);

    lwm2mcoreRef = lwm2mcore_Init(EventHandler);
    CU_ASSERT_TRUE(lwm2mcoreRef != NULL);

    lwm2mcore_Free(lwm2mcoreRef);
    lwm2mcoreRef = NULL;
}

//-------------------------------------------------------------------------------------------------
/**
 *  Test function for lwm2mcore_Connect API
 */
//--------------------------------------------------------------------------------------------------
static void test_lwm2mcore_Connect
(
    void
)
{
    CU_ASSERT_EQUAL(lwm2mcore_Connect(NULL), false);
}

//-------------------------------------------------------------------------------------------------
/**
 *  Test function for lwm2mcore_Disconnect API
 */
static void test_lwm2mcore_Disconnect
(
    void
)
{
    CU_ASSERT_EQUAL(lwm2mcore_Disconnect(NULL), false);
}

//-------------------------------------------------------------------------------------------------
/**
 *  Test function for lwm2mcore_Update API
 */
//--------------------------------------------------------------------------------------------------
static void test_lwm2mcore_Update
(
    void
)
{
    CU_ASSERT_EQUAL(lwm2mcore_Update(NULL), false);
}

//-------------------------------------------------------------------------------------------------
/**
 *  Test function for lwm2mcore_ConnectionGetType API
 */
//--------------------------------------------------------------------------------------------------
static void test_lwm2mcore_ConnectionGetType
(
    void
)
{
    bool result = false;
    bool isDeviceManagement = false;

    result = lwm2mcore_ConnectionGetType(NULL, &isDeviceManagement);
    CU_ASSERT_EQUAL(result, false);
}

//-------------------------------------------------------------------------------------------------
/**
 *  Test function for lwm2mcore_Free API
 */
//--------------------------------------------------------------------------------------------------
static void test_lwm2mcore_Free
(
    void
)
{
    lwm2mcore_Free(NULL);

}

//-------------------------------------------------------------------------------------------------
/**
 *  Test function for lwm2mcore_Push API
 */
//--------------------------------------------------------------------------------------------------
static void test_lwm2mcore_Push
(
    void
)
{
    CU_ASSERT_EQUAL(lwm2mcore_Push(NULL, NULL, 0, 0, NULL), LWM2MCORE_PUSH_FAILED);
}

/*
 * Suite for lwm2mcoreSession.c file
 */
//-------------------------------------------------------------------------------------------------
/**
 *  Suite for lwm2mcoreSession.c file
 */
//--------------------------------------------------------------------------------------------------
CU_ErrorCode create_SessionManager_suite
(
    void
)
{
    CU_pSuite suitePtr = NULL;

    /*
     * Suite test table
     */
    struct TestTable table[] =
    {
        // Public APIs
        { "test of lwm2mcore_Init()",               test_lwm2mcore_Init },
        { "test of lwm2mcore_Connect()",            test_lwm2mcore_Connect },
        { "test of lwm2mcore_Disconnect()",         test_lwm2mcore_Disconnect },
        { "test of lwm2mcore_Update()",             test_lwm2mcore_Update },
        { "test of lwm2mcore_Free()",               test_lwm2mcore_Free },
        { "test of lwm2mcore_ConnectionGetType()",  test_lwm2mcore_ConnectionGetType },
        { "test of lwm2mcore_Push()",               test_lwm2mcore_Push },
        { NULL,                                     NULL }
    };

    /* Creates a new test suite */
    suitePtr = CU_add_suite("Suite_SessionManager", NULL, NULL);

    if (NULL == suitePtr)
    {
        return CU_get_error();
    }
    return addTest(suitePtr, table);
}
