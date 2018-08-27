//-------------------------------------------------------------------------------------------------
/**
 * @file tests.c
 *
 * Unitary test for Lwm2mCore.
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
#include <sessionManager/sessionManager.h>
#include <lwm2mcore/coapHandlers.h>
#include "sampleConfig.h"

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
 * Maximum size of payload.
 */
//--------------------------------------------------------------------------------------------------
#define MAX_LEN_PAYLOAD  100


//--------------------------------------------------------------------------------------------------
/**
 * Static value for LWM2MCore context storage.
 */
//--------------------------------------------------------------------------------------------------
static lwm2mcore_Ref_t Lwm2mcoreRef = NULL;

//--------------------------------------------------------------------------------------------------
/**
 * LWM2MCore client endpoint
 */
//--------------------------------------------------------------------------------------------------
static char Endpoint[LWM2MCORE_ENDPOINT_LEN] = { 0 };

//--------------------------------------------------------------------------------------------------
/**
 * Lwm2mcore response from CoAP
 */
//--------------------------------------------------------------------------------------------------
static lwm2mcore_CoapResponse_t Lwm2mServerResponse;

//--------------------------------------------------------------------------------------------------
/**
 * Lwm2mcore request to CoAP
 */
//--------------------------------------------------------------------------------------------------
static lwm2mcore_CoapRequest_t* RequestPtr;


//--------------------------------------------------------------------------------------------------
/**
 * Event handler for LwM2MCore events
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
            printf("The OTA update client succeeded in authenticating with the server and has "\
                   "started the session\n");
            break;

        case LWM2MCORE_EVENT_SESSION_FAILED:
            printf("The session with the server failed\n");
            break;

        case LWM2MCORE_EVENT_SESSION_FINISHED:
            printf("The session with the server finished successfully\n");
            break;

        case LWM2MCORE_EVENT_LWM2M_SESSION_TYPE_START:
            if (LWM2MCORE_SESSION_BOOTSTRAP == status.u.session.type)
            {
                printf("Connected to the Bootstrap server \n");
            }
            else
            {
                printf("Connected to the Device Management server \n");
            }
            break;

        case LWM2MCORE_EVENT_PACKAGE_DOWNLOAD_DETAILS:
            printf("A descriptor was downloaded with the package size\n");
            break;

        case LWM2MCORE_EVENT_DOWNLOAD_PROGRESS:
            printf("Download progress %d\%%\n", status.u.pkgStatus.progress);
            break;

        case LWM2MCORE_EVENT_PACKAGE_DOWNLOAD_FINISHED:
            printf("The OTA update package downloaded successfully\n");
            break;

        case LWM2MCORE_EVENT_PACKAGE_DOWNLOAD_FAILED:
            printf("The OTA update package downloaded successfully, but could not be stored "\
                   "in flash\n");
            break;

        case LWM2MCORE_EVENT_UPDATE_STARTED:
            printf("An update package is being applied\n");
            break;

        case LWM2MCORE_EVENT_UPDATE_FINISHED:
            printf("The update succeeded\n");
            break;

        case LWM2MCORE_EVENT_UPDATE_FAILED:
            printf("The update failed\n");
            break;

        default:
            break;
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Test function for lwm2mcore_Init API
 */
//--------------------------------------------------------------------------------------------------
static void test_lwm2mcore_Init
(
    void
)
{
    TEST_ASSERT(lwm2mcore_Init(NULL) == NULL);

    Lwm2mcoreRef = lwm2mcore_Init(EventHandler);
    TEST_ASSERT(Lwm2mcoreRef != NULL);

    strncpy(Endpoint, "SIERRAWIRELESS", sizeof(Endpoint));
    TEST_ASSERT(lwm2mcore_ObjectRegister(Lwm2mcoreRef, Endpoint, NULL, NULL) != 0);
}

//-------------------------------------------------------------------------------------------------
/**
 * Test function for lwm2mcore_Connect API
 */
//--------------------------------------------------------------------------------------------------
static void test_lwm2mcore_Connect
(
    void
)
{
    TEST_ASSERT(lwm2mcore_Connect(NULL) == false);
    printf("Lwm2mcoreRef is %p\n", Lwm2mcoreRef);
    TEST_ASSERT(lwm2mcore_Connect(Lwm2mcoreRef) == true);
}

//-------------------------------------------------------------------------------------------------
/**
 * Test function for lwm2mcore_Disconnect API
 */
//--------------------------------------------------------------------------------------------------
static void test_lwm2mcore_Disconnect
(
    void
)
{
    TEST_ASSERT(lwm2mcore_Disconnect(NULL) == false);
    printf("Lwm2mcoreRef is %p\n", Lwm2mcoreRef);
    TEST_ASSERT(lwm2mcore_Disconnect(Lwm2mcoreRef) == true);
}

//-------------------------------------------------------------------------------------------------
/**
 * Test function for lwm2mcore_Free API
 */
//--------------------------------------------------------------------------------------------------
static void test_lwm2mcore_Free
(
    void
)
{
    lwm2mcore_Free(Lwm2mcoreRef);
    Lwm2mcoreRef = NULL;
}

//-------------------------------------------------------------------------------------------------
/**
 * Test function for lwm2mcore_Update API
 */
//--------------------------------------------------------------------------------------------------
static void test_lwm2mcore_Update
(
    void
)
{
    TEST_ASSERT(lwm2mcore_Update(NULL) == false);
    smanager_ClientData_t* dataPtr = (smanager_ClientData_t*)Lwm2mcoreRef;
    dataPtr->lwm2mHPtr->state = STATE_REGISTER_REQUIRED;
    TEST_ASSERT(lwm2mcore_Update(Lwm2mcoreRef) == false);

    lwm2m_server_t* targetP;
    targetP = (lwm2m_server_t*)lwm2m_malloc(sizeof(lwm2m_server_t));
    if (NULL == targetP)
    {
        printf("targetP is NULL!\n");
        TEST_ASSERT(false);
    }

    memset(targetP, 0, sizeof(lwm2m_server_t));
    targetP->secObjInstID = 123;
    targetP->shortID = 1;
    dataPtr->lwm2mHPtr->serverList = (lwm2m_server_t*)LWM2M_LIST_ADD(dataPtr->lwm2mHPtr->serverList,
                                                                     targetP);

    TEST_ASSERT(lwm2mcore_Update(Lwm2mcoreRef) == true);
}

//-------------------------------------------------------------------------------------------------
/**
 * Test function for lwm2mcore_Push API
 */
//--------------------------------------------------------------------------------------------------
static void test_lwm2mcore_Push
(
    void
)
{
    uint8_t payload[MAX_LEN_PAYLOAD] = "1234567890";
    uint16_t midPtr = 0;
    TEST_ASSERT(lwm2mcore_Push(Lwm2mcoreRef, payload, strlen((const char*)payload),
                               LWM2MCORE_PUSH_CONTENT_CBOR, &midPtr) == LWM2MCORE_PUSH_INITIATED);
}

//-------------------------------------------------------------------------------------------------
/**
 * Test function for lwm2m_connect_server API
 */
//--------------------------------------------------------------------------------------------------
void test_lwm2m_connect_server
(
    void
)
{
    smanager_ClientData_t* dataPtr = (smanager_ClientData_t*)Lwm2mcoreRef;
    lwm2m_list_t* instancePtr;
    instancePtr = (lwm2m_list_t *)lwm2m_malloc(sizeof(lwm2m_list_t));
    if (NULL == instancePtr)
    {
        printf("instancePtr is NULL!\n");
        TEST_ASSERT(false);
    }

    instancePtr->id = 1;
    dataPtr->securityObjPtr->instanceList = LWM2M_LIST_ADD(dataPtr->securityObjPtr->instanceList,
                                                           instancePtr);

    // Connection configuration is not available: connection is not possible
    TEST_ASSERT(lwm2m_connect_server(1, dataPtr->lwm2mHPtr->userData) == NULL);
}

//-------------------------------------------------------------------------------------------------
/**
 * Test function for smanager_SendSessionEvent API
 */
//--------------------------------------------------------------------------------------------------
void test_smanager_SendSessionEvent
(
    void
)
{
    smanager_SendSessionEvent(EVENT_TYPE_BOOTSTRAP, EVENT_STATUS_STARTED);
    smanager_SendSessionEvent(EVENT_TYPE_BOOTSTRAP, EVENT_STATUS_DONE_SUCCESS);
    smanager_SendSessionEvent(EVENT_TYPE_BOOTSTRAP, EVENT_STATUS_DONE_FAIL);
    smanager_SendSessionEvent(EVENT_TYPE_REGISTRATION, EVENT_STATUS_STARTED);
    smanager_SendSessionEvent(EVENT_TYPE_REGISTRATION, EVENT_STATUS_DONE_SUCCESS);
    smanager_SendSessionEvent(EVENT_TYPE_REGISTRATION, EVENT_STATUS_DONE_FAIL);
    smanager_SendSessionEvent(EVENT_TYPE_REG_UPDATE, EVENT_STATUS_STARTED);
    smanager_SendSessionEvent(EVENT_TYPE_REG_UPDATE, EVENT_STATUS_DONE_SUCCESS);
    smanager_SendSessionEvent(EVENT_TYPE_REG_UPDATE, EVENT_STATUS_DONE_FAIL);
    smanager_SendSessionEvent(EVENT_TYPE_DEREG, EVENT_STATUS_STARTED);
    smanager_SendSessionEvent(EVENT_TYPE_DEREG, EVENT_STATUS_DONE_SUCCESS);
    smanager_SendSessionEvent(EVENT_TYPE_DEREG, EVENT_STATUS_DONE_FAIL);
    smanager_SendSessionEvent(EVENT_TYPE_AUTHENTICATION, EVENT_STATUS_STARTED);
    smanager_SendSessionEvent(EVENT_TYPE_AUTHENTICATION, EVENT_STATUS_DONE_SUCCESS);
    smanager_SendSessionEvent(EVENT_TYPE_AUTHENTICATION, EVENT_STATUS_DONE_FAIL);
    smanager_SendSessionEvent(EVENT_TYPE_RESUMING, EVENT_STATUS_STARTED);
    smanager_SendSessionEvent(EVENT_TYPE_RESUMING, EVENT_STATUS_DONE_SUCCESS);
    smanager_SendSessionEvent(EVENT_TYPE_RESUMING, EVENT_STATUS_DONE_FAIL);
}

//-------------------------------------------------------------------------------------------------
/**
 * Test function for lwm2mcore_SendAsyncResponse API
 */
//--------------------------------------------------------------------------------------------------
void test_lwm2mcore_SendAsyncResponse
(
    void
)
{
    RequestPtr = (lwm2mcore_CoapRequest_t*)lwm2m_malloc(sizeof(lwm2mcore_CoapRequest_t));
    if (NULL == RequestPtr)
    {
        printf("RequestPtr is NULL!\n");
        TEST_ASSERT(false);
    }

    RequestPtr->uriLength = strlen("www.sierrawireless.com");
    RequestPtr->uri = (char*)lwm2m_malloc(RequestPtr->uriLength + 1);
    strncpy(RequestPtr->uri, "www.sierrawireless.com", RequestPtr->uriLength + 1);

    RequestPtr->method = 1;

    RequestPtr->bufferLength = strlen("123456789");
    RequestPtr->buffer = (uint8_t*)lwm2m_malloc(RequestPtr->bufferLength + 1);
    strncpy((char*)RequestPtr->buffer, "123456789", RequestPtr->bufferLength + 1);

    RequestPtr->messageId = 100;
    RequestPtr->tokenLength = 3;
    memcpy(RequestPtr->token, "hi", 3);
    RequestPtr->contentType = 1;

    Lwm2mServerResponse.code = COAP_RESOURCE_CHANGED;

    Lwm2mServerResponse.payloadLength = strlen("123456789");
    Lwm2mServerResponse.payload = (uint8_t*)lwm2m_malloc(Lwm2mServerResponse.payloadLength + 1);
    strncpy((char*)Lwm2mServerResponse.payload, "123456789", Lwm2mServerResponse.payloadLength + 1);

    TEST_ASSERT(lwm2mcore_SendAsyncResponse(Lwm2mcoreRef, RequestPtr, &Lwm2mServerResponse)
                != false);
}

//-------------------------------------------------------------------------------------------------
/**
 * Test function for lwm2mcore_SetLifetime API
 */
//--------------------------------------------------------------------------------------------------
static void test_lwm2mcore_SetLifetime
(
    void
)
{
    /* Invalid value */
    TEST_ASSERT(lwm2mcore_SetLifetime(0) == LWM2MCORE_ERR_INCORRECT_RANGE);

    /* No DM servers */
    TEST_ASSERT(lwm2mcore_SetLifetime(8600) == LWM2MCORE_ERR_COMPLETED_OK);
}

//-------------------------------------------------------------------------------------------------
/**
 * Test function for lwm2mcore_UpdateSwList API
 */
//--------------------------------------------------------------------------------------------------
static void test_lwm2mcore_UpdateSwList
(
    void
)
{
    int i = 0;
    char emptyList[] = "";
    char objectsList[256] = {0};
    int offset = 0;

    for (i = 0; i < 10; i++)
    {
        // Add an object instance each loop
        offset += snprintf(objectsList + offset, sizeof(objectsList) - offset, "</lwm2m/9/%d>", i);
        TEST_ASSERT(offset>0);

        // At this point, Lwm2mcoreRef is NULL
        TEST_ASSERT(lwm2mcore_UpdateSwList(Lwm2mcoreRef, objectsList, strlen(objectsList)) == true);

        test_lwm2mcore_Init();

        TEST_ASSERT(lwm2mcore_UpdateSwList(Lwm2mcoreRef, emptyList, strlen(emptyList)) == true);

        test_lwm2mcore_Free();
    }
}

//-------------------------------------------------------------------------------------------------
/**
 * Test function for test_lwm2mcore_SetRegistrationID, lwm2mcore_GetRegistrationID and
 * lwm2mcore_DeleteRegistrationID API
 */
//--------------------------------------------------------------------------------------------------
static void test_lwm2mcore_SetRegistrationID
(
    void
)
{
    int i = 0;
    char registrationId[LWM2MCORE_REGISTRATION_ID_MAX_LEN] = {0};
    char tmpId[LWM2MCORE_REGISTRATION_ID_MAX_LEN] = {0};

    for (i = 1; i < 10; i++)
    {
        // Generate a registration ID using the index
        memset(registrationId, 0x00, LWM2MCORE_REGISTRATION_ID_MAX_LEN);
        snprintf(registrationId, LWM2MCORE_REGISTRATION_ID_MAX_LEN, "/rd/%d", i);

        // Add a registration ID for the server ID
        lwm2mcore_SetRegistrationID(i, registrationId);

        // Check that the written registration ID is correct
        TEST_ASSERT(true == lwm2mcore_GetRegistrationID(i, tmpId, sizeof(tmpId) - 1));
        TEST_ASSERT(0 == strcmp(tmpId, registrationId));
    }

    // Check that delete function only deletes the regisgtration ID associated with the server ID
    TEST_ASSERT(true == lwm2mcore_GetRegistrationID(1, tmpId,  sizeof(tmpId) - 1));
    lwm2mcore_DeleteRegistrationID(1);
    TEST_ASSERT(false == lwm2mcore_GetRegistrationID(1, tmpId,  sizeof(tmpId) - 1));
    for (i = 2; i < 10; i++)
    {
        TEST_ASSERT(true == lwm2mcore_GetRegistrationID(i, tmpId, sizeof(tmpId) - 1));
    }

    // Delete all entries
    lwm2mcore_DeleteRegistrationID(-1);
    TEST_ASSERT(false == lwm2mcore_GetRegistrationID(2, tmpId,  sizeof(tmpId) - 1));
    for (i = 1; i < 10; i++)
    {
        TEST_ASSERT(false == lwm2mcore_GetRegistrationID(i, tmpId, sizeof(tmpId) - 1));
    }
}

//--------------------------------------------------------------------------------------------------
/**
 *  Unitary test entry point.
 */
//--------------------------------------------------------------------------------------------------
int main
(
    void
)
{
    printf("======== Start UnitTest of lwm2mcore ========\n");

    CreateBsConfigurationFiles();

    printf("======== test of lwm2mcore_Init() ========\n");
    test_lwm2mcore_Init();

    printf("======== test of lwm2mcore_SetRegistrationID() ========\n");
    test_lwm2mcore_SetRegistrationID();

    printf("======== test of lwm2mcore_Connect() ========\n");
    test_lwm2mcore_Connect();

    printf("======== test of lwm2mcore_Update() ========\n");
    test_lwm2mcore_Update();

    printf("======== test of lwm2mcore_Push() ========\n");
    test_lwm2mcore_Push();

    printf("======== test of lwm2m_connect_server() ========\n");
    test_lwm2m_connect_server();

    printf("======== test of lwm2mcore_SendAsyncResponse() ========\n");
    test_lwm2mcore_SendAsyncResponse();

    printf("======== test of lwm2mcore_Disconnect() ========\n");
    test_lwm2mcore_Disconnect();

    printf("======== test of smanager_SendSessionEvent() ========\n");
    test_smanager_SendSessionEvent();

    printf("======== test of lwm2mcore_Free() ========\n");
    test_lwm2mcore_Free();

    printf("======== test of lwm2mcore_SetLifetime() ========\n");
    test_lwm2mcore_SetLifetime();

    printf("======== test of lwm2mcore_UpdateSwList() ========\n");
    test_lwm2mcore_UpdateSwList();

    printf("======== UnitTest of lwm2mcore ends with SUCCESS ========\n");

    exit(EXIT_SUCCESS);
}