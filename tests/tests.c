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
#include "internals.h"
#include "liblwm2m.h"
#include <lwm2mcore/lwm2mcore.h>
#include <objectManager/objects.h>

#define TEST_FATAL(formatString, ...) \
        { printf(formatString, ##__VA_ARGS__); exit(EXIT_FAILURE); }


#define TEST_ASSERT(condition) \
        if (!(condition)) { TEST_FATAL("Assert Failed: '%s'\n", #condition) }

//--------------------------------------------------------------------------------------------------
/**
 *  Event handler for LwM2MCore events
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

    TEST_ASSERT(lwm2mcore_Init(NULL) == NULL);

    lwm2mcoreRef = lwm2mcore_Init(EventHandler);
    TEST_ASSERT(lwm2mcoreRef != NULL);

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
    TEST_ASSERT(lwm2mcore_Connect(NULL) == false);
}

//-------------------------------------------------------------------------------------------------
/**
 *  Test function for lwm2mcore_Disconnect API
 */
//--------------------------------------------------------------------------------------------------
static void test_lwm2mcore_Disconnect
(
    void
)
{
    TEST_ASSERT(lwm2mcore_Disconnect(NULL) == false);
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
    TEST_ASSERT(lwm2mcore_Update(NULL) == false);
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
    TEST_ASSERT(result == false);
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
    TEST_ASSERT(lwm2mcore_Push(NULL, NULL, 0, 0, NULL) == LWM2MCORE_PUSH_FAILED);
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
    printf("======== Start UnitTest of lwm2mcore ========\n");

    printf("======== test of lwm2mcore_Init() ========\n");
    test_lwm2mcore_Init();

    printf("======== test of lwm2mcore_Connect() ========\n");
    test_lwm2mcore_Connect();

    printf("======== test of lwm2mcore_Disconnect() ========\n");
    test_lwm2mcore_Disconnect();

    printf("======== test of lwm2mcore_Update() ========\n");
    test_lwm2mcore_Update();

    printf("======== test of lwm2mcore_ConnectionGetType() ========\n");
    test_lwm2mcore_ConnectionGetType();

    printf("======== test of lwm2mcore_Free() ========\n");
    test_lwm2mcore_Free();

    printf("======== test of lwm2mcore_Push() ========\n");
    test_lwm2mcore_Push();

    printf("======== UnitTest of Lwm2mcore API ends with SUCCESS ========\n");

    exit(EXIT_SUCCESS);
}
