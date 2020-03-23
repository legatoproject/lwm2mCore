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
#include <sys/stat.h>
#include "internals.h"
#include "liblwm2m.h"
#include <lwm2mcore/lwm2mcore.h>
#include <lwm2mcore/lwm2mcorePackageDownloader.h>
#include <objectManager/objects.h>
#include <sessionManager/sessionManager.h>
#include <packageDownloader/downloader.h>
#include <packageDownloader/workspace.h>
#include <objectManager/objects.h>
#include <packageDownloader/updateAgent.h>
#include <lwm2mcore/coapHandlers.h>
#include "sampleConfig.h"

#include "download_stub.h"
#include "download_test.h"

//--------------------------------------------------------------------------------------------------
/**
 * Maximum size of payload.
 */
//--------------------------------------------------------------------------------------------------
#define MAX_LEN_PAYLOAD  100

//--------------------------------------------------------------------------------------------------
/**
 * Maximum size of path.
 */
//--------------------------------------------------------------------------------------------------
#define MAX_LEN_PATH 256

//--------------------------------------------------------------------------------------------------
/**
 * Incorrect package URL (invalid protocol)
 */
//--------------------------------------------------------------------------------------------------
#define INVALID_PROTOCOL_URL        "coap://somewhere.com/file"

//--------------------------------------------------------------------------------------------------
/**
 * Incorrect package URL (too long)
 */
//--------------------------------------------------------------------------------------------------
#define TOO_LONG_URL "http://link12345678901234567890123456789012345678901234567890123456789012345"\
                     "7890123456789012345678901234567890123456789012345678901234567890123456789012"\
                     "3456789012345678901234567890123456789012345678901234567890123456789012345678"\
                     "9012345678901234567890123456"

//--------------------------------------------------------------------------------------------------
/**
 * URL prefix for all package URL in this test
 */
//--------------------------------------------------------------------------------------------------
#define URL_HTTP_PREFIX             "http://somewhere.com/"
#define URL_HTTPS_PREFIX            "https://somewhere.com/"

//--------------------------------------------------------------------------------------------------
/**
 * Prefix for data receipt simulation
 */
//--------------------------------------------------------------------------------------------------
#define FILE_PREFIX                 "../data/"

//--------------------------------------------------------------------------------------------------
/**
 * Prefix for use case: invalid package
 */
//--------------------------------------------------------------------------------------------------
#define IMAGE_FILE_PREFIX           "image.png"

//--------------------------------------------------------------------------------------------------
/**
 * Prefix for use case: bad signature
 */
//--------------------------------------------------------------------------------------------------
#define BAD_SIGNATURE_FILE_PREFIX   "bad_signature"

//--------------------------------------------------------------------------------------------------
/**
 * Prefix for use case: bad CRC
 */
//--------------------------------------------------------------------------------------------------
#define BAD_CRC_FILE_PREFIX         "bad_CRC"

//--------------------------------------------------------------------------------------------------
/**
 * Prefix for use case: valid package
 */
//--------------------------------------------------------------------------------------------------
#define VALID_FILE_PREFIX           "valid_package.bin"

//--------------------------------------------------------------------------------------------------
/**
 * Prefix for use case: HTTP 404 on package URL
 */
//--------------------------------------------------------------------------------------------------
#define FILE_HTTP_404               "http_404"

//--------------------------------------------------------------------------------------------------
/**
 * Prefix for use case: HTTP 500 on package URL
 */
//--------------------------------------------------------------------------------------------------
#define FILE_HTTP_500               "http_500"

//--------------------------------------------------------------------------------------------------
/**
 * Prefix for use case: HTTP 301 on package URL
 */
//--------------------------------------------------------------------------------------------------
#define FILE_HTTP_301               "http_301"

//--------------------------------------------------------------------------------------------------
/**
 * Static value for LwM2MCore context storage.
 */
//--------------------------------------------------------------------------------------------------
static lwm2mcore_Ref_t Lwm2mcoreRef = NULL;

//--------------------------------------------------------------------------------------------------
/**
 * Static values for LwM2MCore events test
 */
//--------------------------------------------------------------------------------------------------
static lwm2mcore_StatusType_t Lwm2mcoreEvent = LWM2MCORE_EVENT_LAST;
static lwm2mcore_SessionType_t Lwm2mcoreSessionType = LWM2MCORE_SESSION_BOOTSTRAP;

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

    printf("EventHandler %d\n", status.event);
    switch (status.event)
    {
        case LWM2MCORE_EVENT_AUTHENTICATION_STARTED:
            printf("Authentication started\n");
            Lwm2mcoreSessionType = status.u.session.type;
            break;

        case LWM2MCORE_EVENT_AUTHENTICATION_FAILED:
            printf("Authentication failed\n");
            Lwm2mcoreSessionType = status.u.session.type;
            break;

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
            Lwm2mcoreSessionType = status.u.session.type;
            break;

        case LWM2MCORE_EVENT_LWM2M_SESSION_INACTIVE:
            printf("Inactive session\n");
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
            return 0;
    }

    Lwm2mcoreEvent = status.event;

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
 * Test function for lwm2mcore_DisconnectWithDeregister API
 */
//--------------------------------------------------------------------------------------------------
static void test_lwm2mcore_DisconnectWithDeregister
(
    void
)
{
    TEST_ASSERT(lwm2mcore_DisconnectWithDeregister(NULL) == false);
    printf("Lwm2mcoreRef is %p\n", Lwm2mcoreRef);
    TEST_ASSERT(lwm2mcore_DisconnectWithDeregister(Lwm2mcoreRef) == true);
}

//-------------------------------------------------------------------------------------------------
/**
 * Test function for lwm2mcore_HardDisconnect API
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
    Lwm2mcoreEvent = LWM2MCORE_EVENT_LAST;

    // Bootstrap session
    smanager_SendSessionEvent(EVENT_TYPE_BOOTSTRAP, EVENT_STATUS_STARTED, NULL);
    // No generated event
    TEST_ASSERT(Lwm2mcoreEvent == LWM2MCORE_EVENT_LAST);

    smanager_SendSessionEvent(EVENT_TYPE_AUTHENTICATION, EVENT_STATUS_STARTED, NULL);
    TEST_ASSERT(Lwm2mcoreEvent == LWM2MCORE_EVENT_AUTHENTICATION_STARTED);
    TEST_ASSERT(Lwm2mcoreSessionType == LWM2MCORE_SESSION_BOOTSTRAP);
    Lwm2mcoreEvent = LWM2MCORE_EVENT_LAST;

    smanager_SendSessionEvent(EVENT_TYPE_AUTHENTICATION, EVENT_STATUS_DONE_FAIL, NULL);
    TEST_ASSERT(Lwm2mcoreEvent == LWM2MCORE_EVENT_AUTHENTICATION_FAILED);
    TEST_ASSERT(Lwm2mcoreSessionType == LWM2MCORE_SESSION_BOOTSTRAP);
    Lwm2mcoreEvent = LWM2MCORE_EVENT_LAST;

    smanager_SendSessionEvent(EVENT_TYPE_AUTHENTICATION, EVENT_STATUS_DONE_SUCCESS, NULL);
    TEST_ASSERT(Lwm2mcoreEvent == LWM2MCORE_EVENT_LWM2M_SESSION_TYPE_START);
    TEST_ASSERT(Lwm2mcoreSessionType == LWM2MCORE_SESSION_BOOTSTRAP);
    Lwm2mcoreEvent = LWM2MCORE_EVENT_LAST;

    smanager_SendSessionEvent(EVENT_TYPE_BOOTSTRAP, EVENT_STATUS_DONE_FAIL, NULL);
    TEST_ASSERT(Lwm2mcoreEvent == LWM2MCORE_EVENT_SESSION_FAILED);
    Lwm2mcoreEvent = LWM2MCORE_EVENT_LAST;

    smanager_SendSessionEvent(EVENT_TYPE_BOOTSTRAP, EVENT_STATUS_DONE_SUCCESS, NULL);
    // No generated event
    TEST_ASSERT(Lwm2mcoreEvent == LWM2MCORE_EVENT_LAST);


    // Device Management session
    smanager_SendSessionEvent(EVENT_TYPE_AUTHENTICATION, EVENT_STATUS_STARTED, NULL);
    TEST_ASSERT(Lwm2mcoreEvent == LWM2MCORE_EVENT_AUTHENTICATION_STARTED);
    TEST_ASSERT(Lwm2mcoreSessionType == LWM2MCORE_SESSION_DEVICE_MANAGEMENT);
    Lwm2mcoreEvent = LWM2MCORE_EVENT_LAST;

    smanager_SendSessionEvent(EVENT_TYPE_AUTHENTICATION, EVENT_STATUS_DONE_FAIL, NULL);
    TEST_ASSERT(Lwm2mcoreEvent == LWM2MCORE_EVENT_AUTHENTICATION_FAILED);
    TEST_ASSERT(Lwm2mcoreSessionType == LWM2MCORE_SESSION_DEVICE_MANAGEMENT);
    Lwm2mcoreEvent = LWM2MCORE_EVENT_LAST;

    smanager_SendSessionEvent(EVENT_TYPE_AUTHENTICATION, EVENT_STATUS_DONE_SUCCESS, NULL);
    // No generated event
    TEST_ASSERT(Lwm2mcoreEvent == LWM2MCORE_EVENT_LAST);

    smanager_SendSessionEvent(EVENT_TYPE_REGISTRATION, EVENT_STATUS_STARTED, NULL);
    // No generated event
    TEST_ASSERT(Lwm2mcoreEvent == LWM2MCORE_EVENT_LAST);

    smanager_SendSessionEvent(EVENT_TYPE_REGISTRATION, EVENT_STATUS_DONE_FAIL, NULL);
    TEST_ASSERT(Lwm2mcoreEvent == LWM2MCORE_EVENT_SESSION_FAILED);
    Lwm2mcoreEvent = LWM2MCORE_EVENT_LAST;

    smanager_SendSessionEvent(EVENT_TYPE_REGISTRATION, EVENT_STATUS_DONE_SUCCESS, NULL);
    TEST_ASSERT(Lwm2mcoreEvent == LWM2MCORE_EVENT_LWM2M_SESSION_TYPE_START);
    TEST_ASSERT(Lwm2mcoreSessionType == LWM2MCORE_SESSION_DEVICE_MANAGEMENT);
    Lwm2mcoreEvent = LWM2MCORE_EVENT_LAST;

    smanager_SendSessionEvent(EVENT_TYPE_REGISTRATION, EVENT_STATUS_INACTIVE, NULL);
    TEST_ASSERT(Lwm2mcoreEvent == LWM2MCORE_EVENT_LWM2M_SESSION_INACTIVE);
    Lwm2mcoreEvent = LWM2MCORE_EVENT_LAST;


    // Registration update
    smanager_SendSessionEvent(EVENT_TYPE_REG_UPDATE, EVENT_STATUS_STARTED, NULL);
    // No generated event
    TEST_ASSERT(Lwm2mcoreEvent == LWM2MCORE_EVENT_LAST);

    smanager_SendSessionEvent(EVENT_TYPE_REG_UPDATE, EVENT_STATUS_DONE_SUCCESS, NULL);
    // No generated event
    TEST_ASSERT(Lwm2mcoreEvent == LWM2MCORE_EVENT_LAST);

    smanager_SendSessionEvent(EVENT_TYPE_REG_UPDATE, EVENT_STATUS_DONE_FAIL, NULL);
    // No generated event
    TEST_ASSERT(Lwm2mcoreEvent == LWM2MCORE_EVENT_LAST);


    // Deregistration
    smanager_SendSessionEvent(EVENT_TYPE_DEREG, EVENT_STATUS_STARTED, NULL);
    // No generated event
    TEST_ASSERT(Lwm2mcoreEvent == LWM2MCORE_EVENT_LAST);

    smanager_SendSessionEvent(EVENT_TYPE_DEREG, EVENT_STATUS_DONE_SUCCESS, NULL);
    // No generated event
    TEST_ASSERT(Lwm2mcoreEvent == LWM2MCORE_EVENT_LAST);

    smanager_SendSessionEvent(EVENT_TYPE_DEREG, EVENT_STATUS_DONE_FAIL, NULL);
    // No generated event
    TEST_ASSERT(Lwm2mcoreEvent == LWM2MCORE_EVENT_LAST);


    // DTLS resume
    smanager_SendSessionEvent(EVENT_TYPE_RESUMING, EVENT_STATUS_STARTED, NULL);
    // No generated event
    TEST_ASSERT(Lwm2mcoreEvent == LWM2MCORE_EVENT_LAST);

    smanager_SendSessionEvent(EVENT_TYPE_RESUMING, EVENT_STATUS_DONE_SUCCESS, NULL);
    // No generated event
    TEST_ASSERT(Lwm2mcoreEvent == LWM2MCORE_EVENT_LAST);

    smanager_SendSessionEvent(EVENT_TYPE_RESUMING, EVENT_STATUS_DONE_FAIL, NULL);
    // No generated event
    TEST_ASSERT(Lwm2mcoreEvent == LWM2MCORE_EVENT_LAST);


    // Session
    smanager_SendSessionEvent(EVENT_SESSION, EVENT_STATUS_STARTED, NULL);
    // No generated event
    TEST_ASSERT(Lwm2mcoreEvent == LWM2MCORE_EVENT_LAST);

    smanager_SendSessionEvent(EVENT_SESSION, EVENT_STATUS_DONE_SUCCESS, NULL);
    TEST_ASSERT(Lwm2mcoreEvent == LWM2MCORE_EVENT_SESSION_FINISHED);
    Lwm2mcoreEvent = LWM2MCORE_EVENT_LAST;

    smanager_SendSessionEvent(EVENT_SESSION, EVENT_STATUS_DONE_FAIL, NULL);
    TEST_ASSERT(Lwm2mcoreEvent == LWM2MCORE_EVENT_SESSION_FAILED);
    Lwm2mcoreEvent = LWM2MCORE_EVENT_LAST;

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

    size_t uriLength = strlen("www.sierrawireless.com");
    RequestPtr->uri = (char*)lwm2m_malloc(uriLength+ 1);
    strncpy(RequestPtr->uri, "www.sierrawireless.com", uriLength + 1);

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
    Lwm2mServerResponse.payloadPtr = (uint8_t*)lwm2m_malloc(Lwm2mServerResponse.payloadLength + 1);
    strncpy((char*)Lwm2mServerResponse.payloadPtr, "123456789",
            Lwm2mServerResponse.payloadLength + 1);

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
 * Test function for lwm2mcore_ResourceRead API
 */
//--------------------------------------------------------------------------------------------------
static void test_lwm2mcore_ResourceRead
(
    void
)
{
    bool result;
    char buffer[256];
    size_t len = sizeof(buffer);

    memset(buffer, 0, 256);

    // Invalid arguments
    result = lwm2mcore_ResourceRead(LWM2MCORE_DEVICE_OID,
                                    0,
                                    LWM2MCORE_DEVICE_MANUFACTURER_RID,
                                    0,
                                    NULL,
                                    &len);
    TEST_ASSERT(false == result);

    result = lwm2mcore_ResourceRead(LWM2MCORE_DEVICE_OID,
                                    0,
                                    LWM2MCORE_DEVICE_MANUFACTURER_RID,
                                    0,
                                    buffer,
                                    NULL);
    TEST_ASSERT(false == result);

    // Read a resource which does not support read handler
    result = lwm2mcore_ResourceRead(LWM2MCORE_DEVICE_OID,
                                    0,
                                    LWM2MCORE_DEVICE_REBOOT_RID,
                                    0,
                                    buffer,
                                    &len);
    TEST_ASSERT(false == result);

    // Read a not supported resource
    result = lwm2mcore_ResourceRead(LWM2MCORE_DEVICE_OID,
                                    0,
                                    255,
                                    0,
                                    buffer,
                                    &len);
    TEST_ASSERT(false == result);

    // Read a valid resource (string)
    result = lwm2mcore_ResourceRead(LWM2MCORE_DEVICE_OID,
                                    0,
                                    LWM2MCORE_DEVICE_MANUFACTURER_RID,
                                    0,
                                    buffer,
                                    &len);
    TEST_ASSERT(true == result);
    TEST_ASSERT(!strcmp(buffer, "Sierra Wireless"));
    TEST_ASSERT(len == strlen("Sierra Wireless"))  ;

    // Read a valid resource (int)
    len = sizeof(buffer);
    result = lwm2mcore_ResourceRead(LWM2MCORE_EXT_CONN_STATS_OID,
                                    0,
                                    LWM2MCORE_EXT_CONN_STATS_TEMPERATURE_RID,
                                    0,
                                    buffer,
                                    &len);
    TEST_ASSERT(true == result);
    TEST_ASSERT(!strcmp(buffer, "26"));
    TEST_ASSERT(len == strlen("26"))  ;
}

//-------------------------------------------------------------------------------------------------
/**
 * Test function to set environment for package download tests
 */
//--------------------------------------------------------------------------------------------------
static bool test_lwm2mcore_PrepareFilesForDownload
(
    const char*     fileNamePrefixPtr,  ///< [IN] File name prefix
    char*           packageUrlPtr       ///< [INOUT] Package URL according to 1st parameter
)
{
    if ((!fileNamePrefixPtr) || (!packageUrlPtr))
    {
        return false;
    }

    memset(packageUrlPtr, 0, LWM2MCORE_PACKAGE_URI_MAX_BYTES);
    snprintf(packageUrlPtr,
             LWM2MCORE_PACKAGE_URI_MAX_BYTES,
             "%s%s",
             URL_HTTP_PREFIX,
             fileNamePrefixPtr);
    test_setFileNameForPackageDownload(fileNamePrefixPtr);
    return true;
}

//--------------------------------------------------------------------------------------------------
/**
 * Find the path containing the currently-running program executable
 */
//--------------------------------------------------------------------------------------------------
static bool GetExecPath(char* buffer, size_t buflen)
{
    int length;
    char* pathEndPtr = NULL;

    length = readlink("/proc/self/exe", buffer, buflen - 1);
    if (length <= 0)
    {
        return -1;
    }
    buffer[length] = '\0';

    // Delete the binary name from the path
    pathEndPtr = strrchr(buffer, '/');
    if (NULL == pathEndPtr)
    {
        return -1;
    }
    *(pathEndPtr+1) = '\0';

    return 0;
}

//-------------------------------------------------------------------------------------------------
/**
 * Test function for downloader APIs
 */
//--------------------------------------------------------------------------------------------------
static void test_lwm2mcore_Downloader
(
    void
)
{
    downloaderResult_t result;
    char fileName[50];
    char packageUrl[LWM2MCORE_PACKAGE_URI_MAX_BYTES];
    uint64_t packageSize = 0;
    uint64_t offset = 0;
    uint16_t httpErrorCode = 0;
    struct stat sb;
    char path[MAX_LEN_PATH] = {0};

    // Tests on downloader_GetPackageSize
    // FW update state and results do not need to be checked
    //These values are not updated by the downloader itself
    printf("\n====== Tests on downloader_GetPackageSize ======\n");

    // Invalid parameters
    printf("\n====== downloader_GetPackageSize: Invalid parameters ======\n");
    TEST_ASSERT(DOWNLOADER_INVALID_ARG == downloader_GetPackageSize(NULL, NULL));
    TEST_ASSERT(DOWNLOADER_INVALID_ARG == downloader_GetPackageSize(packageUrl, NULL));
    TEST_ASSERT(DOWNLOADER_INVALID_ARG == downloader_GetPackageSize(NULL, &packageSize));
    TEST_ASSERT(DOWNLOADER_INVALID_ARG == downloader_GetPackageSize((char*)TOO_LONG_URL,
                                                                       &packageSize));

    // Invalid protocol
    printf("\n====== downloader_GetPackageSize: Invalid protocol ======\n");
    packageSize = 0;
    memset(packageUrl, 0, LWM2MCORE_PACKAGE_URI_MAX_BYTES);
    strncpy(packageUrl, INVALID_PROTOCOL_URL, LWM2MCORE_PACKAGE_URI_MAX_BYTES);
    result = downloader_GetPackageSize(packageUrl, &packageSize);
    TEST_ASSERT(DOWNLOADER_INVALID_ARG == result);

    // Empty URL
    printf("\n====== downloader_GetPackageSize: empty URL ======\n");
    memset(packageUrl, 0, LWM2MCORE_PACKAGE_URI_MAX_BYTES);
    result = downloader_GetPackageSize(packageUrl, &packageSize);
    TEST_ASSERT(DOWNLOADER_INVALID_ARG == result);


    // Invalid URL: HTTP 404
    printf("\n====== downloader_GetPackageSize: HTTP response 404 ======\n");
    TEST_ASSERT(true == test_lwm2mcore_PrepareFilesForDownload(FILE_HTTP_404, packageUrl));
    result = downloader_GetPackageSize(packageUrl, &packageSize);
    TEST_ASSERT(DOWNLOADER_ERROR == result);
    httpErrorCode = 0;
    TEST_ASSERT(LWM2MCORE_ERR_COMPLETED_OK == lwm2mcore_GetLastHttpErrorCode(&httpErrorCode));
    TEST_ASSERT(404 == httpErrorCode);

    // HTTP 500
    printf("\n====== downloader_GetPackageSize: HTTP response 500 ======\n");
    TEST_ASSERT(true == test_lwm2mcore_PrepareFilesForDownload(FILE_HTTP_500, packageUrl));
    result = downloader_GetPackageSize(packageUrl, &packageSize);
    TEST_ASSERT(DOWNLOADER_ERROR == result);
    httpErrorCode = 0;
    TEST_ASSERT(LWM2MCORE_ERR_COMPLETED_OK == lwm2mcore_GetLastHttpErrorCode(&httpErrorCode));
    TEST_ASSERT(500 == httpErrorCode);

    // HTTP 301
    printf("\n====== downloader_GetPackageSize: HTTP response 301 ======\n");
    TEST_ASSERT(true == test_lwm2mcore_PrepareFilesForDownload(FILE_HTTP_301, packageUrl));
    result = downloader_GetPackageSize(packageUrl, &packageSize);
    TEST_ASSERT(DOWNLOADER_ERROR == result);
    httpErrorCode = 0;
    TEST_ASSERT(LWM2MCORE_ERR_COMPLETED_OK == lwm2mcore_GetLastHttpErrorCode(&httpErrorCode));
    TEST_ASSERT(301 == httpErrorCode);


    // Valid URL
    printf("\n====== downloader_GetPackageSize: valid URL ======\n");
    memset(packageUrl, 0, LWM2MCORE_PACKAGE_URI_MAX_BYTES);
    snprintf(packageUrl,
             LWM2MCORE_PACKAGE_URI_MAX_BYTES,
             "%s%s",
             URL_HTTP_PREFIX,
             IMAGE_FILE_PREFIX);
    test_setFileNameForPackageDownload(IMAGE_FILE_PREFIX);
    result = downloader_GetPackageSize(packageUrl, &packageSize);
    TEST_ASSERT(DOWNLOADER_OK == result);
    memset(fileName, 0, sizeof(fileName));
    snprintf(fileName,
             sizeof(fileName),
             "%s%s",
             FILE_PREFIX,
             IMAGE_FILE_PREFIX);
    TEST_ASSERT(0 == GetExecPath(path, MAX_LEN_PATH));
    TEST_ASSERT(strlen(path) + strlen(fileName) < MAX_LEN_PATH);
    strncat(path, fileName, strlen(fileName));
    TEST_ASSERT(0 == stat(path, &sb))
    TEST_ASSERT(packageSize == (uint64_t)sb.st_size);

    memset(packageUrl, 0, LWM2MCORE_PACKAGE_URI_MAX_BYTES);
    snprintf(packageUrl,
             LWM2MCORE_PACKAGE_URI_MAX_BYTES,
             "%s%s",
             URL_HTTPS_PREFIX,
             IMAGE_FILE_PREFIX);
    test_setFileNameForPackageDownload(IMAGE_FILE_PREFIX);
    result = downloader_GetPackageSize(packageUrl, &packageSize);
    TEST_ASSERT(DOWNLOADER_OK == result);
    memset(fileName, 0, 50);
    snprintf(fileName,
             sizeof(fileName),
             "%s%s",
             FILE_PREFIX,
             IMAGE_FILE_PREFIX);
    // Concat the path with the relative path
    TEST_ASSERT(0 == GetExecPath(path, MAX_LEN_PATH));
    TEST_ASSERT(strlen(path) + strlen(fileName) < MAX_LEN_PATH);
    strncat(path, fileName, strlen(fileName));
    TEST_ASSERT(0 == stat(path, &sb))
    TEST_ASSERT(packageSize == (uint64_t)sb.st_size);


    printf("\n====== Tests on downloader_StartDownload ======\n");

    printf("\n====== downloader_StartDownload: Invalid parameters ======\n");
    TEST_ASSERT(DOWNLOADER_INVALID_ARG == downloader_StartDownload(NULL, offset, NULL));
    TEST_ASSERT(DOWNLOADER_INVALID_ARG == downloader_StartDownload((char*)"", offset, NULL));
    TEST_ASSERT(DOWNLOADER_INVALID_ARG == downloader_StartDownload((char*)TOO_LONG_URL,
                                                                      offset,
                                                                      NULL));

    printf("\n====== Tests on downloader_AbortDownload ======\n");
    downloader_AbortDownload();
    TEST_ASSERT(DWL_ABORTED == downloader_GetDownloadStatus());

    printf("\n====== Tests on downloader_SuspendDownload ======\n");
    downloader_SuspendDownload();
    TEST_ASSERT(DWL_SUSPEND == downloader_GetDownloadStatus());

    printf("\n====== Tests on downloader_CheckDownloadToAbort ======\n");
    downloader_SuspendDownload();
    TEST_ASSERT(false == downloader_CheckDownloadToAbort());
    downloader_AbortDownload();
    TEST_ASSERT(true == downloader_CheckDownloadToAbort());

    printf("\n====== Tests on downloader_CheckDownloadToSuspend ======\n");
    downloader_AbortDownload();
    TEST_ASSERT(false == downloader_CheckDownloadToSuspend());
    downloader_SuspendDownload();
    TEST_ASSERT(true == downloader_CheckDownloadToSuspend());

    printf("\n====== Tests on lwm2mcore_GetLastHttpErrorCode ======\n");
    TEST_ASSERT(LWM2MCORE_ERR_INVALID_ARG == lwm2mcore_GetLastHttpErrorCode(NULL));
    // Tests on returned value are already made


    // Test memory allocation issue
    printf("\n====== Tests memory allocation issue ======\n");
    test_setConnectForDownloadResult(LWM2MCORE_ERR_MEMORY);
    memset(packageUrl, 0, LWM2MCORE_PACKAGE_URI_MAX_BYTES);
    snprintf(packageUrl,
             LWM2MCORE_PACKAGE_URI_MAX_BYTES,
             "%s%s",
             URL_HTTP_PREFIX,
             IMAGE_FILE_PREFIX);
    test_setFileNameForPackageDownload(IMAGE_FILE_PREFIX);
    result = downloader_GetPackageSize(packageUrl, &packageSize);
    TEST_ASSERT(DOWNLOADER_MEMORY_ERROR == result);

    memset(packageUrl, 0, LWM2MCORE_PACKAGE_URI_MAX_BYTES);
    snprintf(packageUrl,
             LWM2MCORE_PACKAGE_URI_MAX_BYTES,
             "%s%s",
             URL_HTTPS_PREFIX,
             IMAGE_FILE_PREFIX);
    test_setFileNameForPackageDownload(IMAGE_FILE_PREFIX);
    result = downloader_GetPackageSize(packageUrl, &packageSize);
    TEST_ASSERT(DOWNLOADER_MEMORY_ERROR == result);

    test_setConnectForDownloadResult(LWM2MCORE_ERR_COMPLETED_OK);



    test_setConnectForDownloadResult(LWM2MCORE_ERR_NET_RECV_FAILED);
    memset(packageUrl, 0, LWM2MCORE_PACKAGE_URI_MAX_BYTES);
    snprintf(packageUrl,
             LWM2MCORE_PACKAGE_URI_MAX_BYTES,
             "%s%s",
             URL_HTTP_PREFIX,
             IMAGE_FILE_PREFIX);
    test_setFileNameForPackageDownload(IMAGE_FILE_PREFIX);
    result = downloader_GetPackageSize(packageUrl, &packageSize);
    TEST_ASSERT(DOWNLOADER_RECV_ERROR == result);

    memset(packageUrl, 0, LWM2MCORE_PACKAGE_URI_MAX_BYTES);
    snprintf(packageUrl,
             LWM2MCORE_PACKAGE_URI_MAX_BYTES,
             "%s%s",
             URL_HTTPS_PREFIX,
             IMAGE_FILE_PREFIX);
    test_setFileNameForPackageDownload(IMAGE_FILE_PREFIX);
    result = downloader_GetPackageSize(packageUrl, &packageSize);
    TEST_ASSERT(DOWNLOADER_RECV_ERROR == result);

    test_setConnectForDownloadResult(LWM2MCORE_ERR_COMPLETED_OK);


    test_setConnectForDownloadResult(LWM2MCORE_ERR_NET_SEND_FAILED);
    memset(packageUrl, 0, LWM2MCORE_PACKAGE_URI_MAX_BYTES);
    snprintf(packageUrl,
             LWM2MCORE_PACKAGE_URI_MAX_BYTES,
             "%s%s",
             URL_HTTP_PREFIX,
             IMAGE_FILE_PREFIX);
    test_setFileNameForPackageDownload(IMAGE_FILE_PREFIX);
    result = downloader_GetPackageSize(packageUrl, &packageSize);
    TEST_ASSERT(DOWNLOADER_SEND_ERROR == result);

    memset(packageUrl, 0, LWM2MCORE_PACKAGE_URI_MAX_BYTES);
    snprintf(packageUrl,
             LWM2MCORE_PACKAGE_URI_MAX_BYTES,
             "%s%s",
             URL_HTTPS_PREFIX,
             IMAGE_FILE_PREFIX);
    test_setFileNameForPackageDownload(IMAGE_FILE_PREFIX);
    result = downloader_GetPackageSize(packageUrl, &packageSize);
    TEST_ASSERT(DOWNLOADER_SEND_ERROR == result);

    test_setConnectForDownloadResult(LWM2MCORE_ERR_COMPLETED_OK);


    test_setConnectForDownloadResult(LWM2MCORE_ERR_GENERAL_ERROR);
    memset(packageUrl, 0, LWM2MCORE_PACKAGE_URI_MAX_BYTES);
    snprintf(packageUrl,
             LWM2MCORE_PACKAGE_URI_MAX_BYTES,
             "%s%s",
             URL_HTTP_PREFIX,
             IMAGE_FILE_PREFIX);
    test_setFileNameForPackageDownload(IMAGE_FILE_PREFIX);
    result = downloader_GetPackageSize(packageUrl, &packageSize);
    TEST_ASSERT(DOWNLOADER_CONNECTION_ERROR == result);

    memset(packageUrl, 0, LWM2MCORE_PACKAGE_URI_MAX_BYTES);
    snprintf(packageUrl,
             LWM2MCORE_PACKAGE_URI_MAX_BYTES,
             "%s%s",
             URL_HTTPS_PREFIX,
             IMAGE_FILE_PREFIX);
    test_setFileNameForPackageDownload(IMAGE_FILE_PREFIX);
    result = downloader_GetPackageSize(packageUrl, &packageSize);
    TEST_ASSERT(DOWNLOADER_CONNECTION_ERROR == result);

    test_setConnectForDownloadResult(LWM2MCORE_ERR_COMPLETED_OK);





    test_setReadForDownloadResult(LWM2MCORE_ERR_TIMEOUT);
    memset(packageUrl, 0, LWM2MCORE_PACKAGE_URI_MAX_BYTES);
    snprintf(packageUrl,
             LWM2MCORE_PACKAGE_URI_MAX_BYTES,
             "%s%s",
             URL_HTTP_PREFIX,
             IMAGE_FILE_PREFIX);
    test_setFileNameForPackageDownload(IMAGE_FILE_PREFIX);
    result = downloader_GetPackageSize(packageUrl, &packageSize);
    TEST_ASSERT(DOWNLOADER_TIMEOUT == result);

    memset(packageUrl, 0, LWM2MCORE_PACKAGE_URI_MAX_BYTES);
    snprintf(packageUrl,
             LWM2MCORE_PACKAGE_URI_MAX_BYTES,
             "%s%s",
             URL_HTTPS_PREFIX,
             IMAGE_FILE_PREFIX);
    test_setFileNameForPackageDownload(IMAGE_FILE_PREFIX);
    result = downloader_GetPackageSize(packageUrl, &packageSize);
    TEST_ASSERT(DOWNLOADER_TIMEOUT == result);

    test_setReadForDownloadResult(LWM2MCORE_ERR_COMPLETED_OK);
}

//-------------------------------------------------------------------------------------------------
/**
 * Test function for upodate package APIs
 */
//--------------------------------------------------------------------------------------------------
static void test_lwm2mcore_UpdatePackage
(
    void
)
{
    lwm2mcore_Sid_t result;
    char host[50];
    char buffer[100];
    char request[100];
    uint8_t bufferRead[100];
    char packageUrl[LWM2MCORE_PACKAGE_URI_MAX_BYTES];
    int len = 0;
    int loop;
    uint64_t packageSize = 0;
    uint16_t errorCode = 0;
    size_t size = 0;
    lwm2mcore_FwUpdateState_t fwState = LWM2MCORE_FW_UPDATE_STATE_IDLE;
    lwm2mcore_FwUpdateResult_t fwResult = LWM2MCORE_FW_UPDATE_RESULT_DEFAULT_NORMAL;
    //PackageDownloaderError_t dwlError;
    lwm2mcore_UpdateType_t updateType = LWM2MCORE_MAX_UPDATE_TYPE;
    PackageDownloaderWorkspace_t workspace;
    lwm2mcore_PackageDownloadContext_t* downloadContextPtr;

    Lwm2mcoreEvent = LWM2MCORE_EVENT_LAST;

    // Test on NULL download context
    TEST_ASSERT(LWM2MCORE_ERR_INVALID_ARG == lwm2mcore_FreeForDownload(NULL));
    TEST_ASSERT(LWM2MCORE_ERR_INVALID_ARG == lwm2mcore_ConnectForDownload(NULL, host, 80));
    TEST_ASSERT(LWM2MCORE_ERR_INVALID_ARG == lwm2mcore_ReadForDownload(NULL, buffer, &len));
    TEST_ASSERT(LWM2MCORE_ERR_INVALID_ARG == lwm2mcore_SendForDownload(NULL, request));
    TEST_ASSERT(LWM2MCORE_ERR_INVALID_ARG == lwm2mcore_DisconnectForDownload(NULL));


    // Test on invalid argument (execept download context)
    downloadContextPtr = lwm2mcore_InitForDownload(true);
    TEST_ASSERT(NULL != downloadContextPtr);

    TEST_ASSERT(LWM2MCORE_ERR_INVALID_ARG == lwm2mcore_ConnectForDownload(downloadContextPtr,
                                                                          NULL,
                                                                          80));

    TEST_ASSERT(LWM2MCORE_ERR_INVALID_ARG == lwm2mcore_ReadForDownload(downloadContextPtr,
                                                                       NULL,
                                                                       &len));

    TEST_ASSERT(LWM2MCORE_ERR_INVALID_ARG == lwm2mcore_SendForDownload(downloadContextPtr, NULL));

    TEST_ASSERT(LWM2MCORE_ERR_INVALID_ARG == lwm2mcore_SendForDownload(NULL, 0));

    TEST_ASSERT(LWM2MCORE_ERR_COMPLETED_OK == lwm2mcore_FreeForDownload(downloadContextPtr));

    TEST_ASSERT(LWM2MCORE_ERR_INVALID_ARG == downloader_InitializeDownload
                                                                (LWM2MCORE_MAX_UPDATE_TYPE,
                                                                 0,
                                                                 buffer,
                                                                 10));

    TEST_ASSERT(LWM2MCORE_ERR_INVALID_ARG == downloader_InitializeDownload
                                                                (LWM2MCORE_FW_UPDATE_TYPE,
                                                                 0,
                                                                 NULL,
                                                                 10));

    // Workspace internal APIs
    TEST_ASSERT(DWL_FAULT == ReadPkgDwlWorkspace(NULL));
    TEST_ASSERT(DWL_FAULT == WritePkgDwlWorkspace(NULL));

    TEST_ASSERT(DWL_OK == DeletePkgDwlWorkspace());
    // No workspace for the moment: read default
    TEST_ASSERT(DWL_OK == ReadPkgDwlWorkspace(&workspace));

    workspace.version = PKGDWL_WORKSPACE_VERSION;
    workspace.offset = 100;
    workspace.section = 200;
    workspace.subsection = 50;
    workspace.packageCRC = 300;
    workspace.commentSize = 400;
    workspace.binarySize = 600;
    workspace.paddingSize = 700;
    workspace.remainingBinaryData = 800;
    workspace.signatureSize = 900;
    workspace.computedCRC = 1000;
    for(loop = 0; loop < 255; loop++)
    {
        workspace.sha1Ctx[loop] = loop;
    }

    TEST_ASSERT(DWL_OK == WritePkgDwlWorkspace(&workspace));

    memset(&workspace, 0, sizeof(PackageDownloaderWorkspace_t));
    TEST_ASSERT(DWL_OK == ReadPkgDwlWorkspace(&workspace));

    TEST_ASSERT(workspace.version == PKGDWL_WORKSPACE_VERSION);
    TEST_ASSERT(workspace.offset == 100);
    TEST_ASSERT(workspace.section == 200);
    TEST_ASSERT(workspace.subsection == 50);
    TEST_ASSERT(workspace.packageCRC == 300);
    TEST_ASSERT(workspace.commentSize == 400);
    TEST_ASSERT(workspace.binarySize == 600);
    TEST_ASSERT(workspace.paddingSize == 700);
    TEST_ASSERT(workspace.remainingBinaryData == 800);
    TEST_ASSERT(workspace.signatureSize == 900);
    TEST_ASSERT(workspace.computedCRC == 1000);
    for(loop = 0; loop < 255; loop++)
    {
        TEST_ASSERT(workspace.sha1Ctx[loop] == loop);
    }

    TEST_ASSERT(DWL_OK == DeletePkgDwlWorkspace());



    printf("\n====== Tests on downloader_InitializeDownload ======\n");

    printf("\n====== downloader_InitializeDownload: invalid protocol ======\n");
    // Test with invalid package URL protocol (should be in http, https)
    result = downloader_InitializeDownload( LWM2MCORE_FW_UPDATE_TYPE,
                                            0,
                                            (char*)INVALID_PROTOCOL_URL,
                                            strlen(INVALID_PROTOCOL_URL));
    TEST_ASSERT(LWM2MCORE_ERR_COMPLETED_OK == result);

    // Check that the URL was updated
    TEST_ASSERT(DWL_OK == ReadPkgDwlWorkspace(&workspace));
    TEST_ASSERT(!strncmp(INVALID_PROTOCOL_URL, workspace.url, strlen(INVALID_PROTOCOL_URL)));
    downloader_PackageDownloadHandler();
    // Check FW update state and result
    TEST_ASSERT(LWM2MCORE_ERR_COMPLETED_OK == downloader_GetFwUpdateState(&fwState));
    TEST_ASSERT(LWM2MCORE_FW_UPDATE_STATE_IDLE == fwState);
    TEST_ASSERT(LWM2MCORE_ERR_COMPLETED_OK == downloader_GetFwUpdateResult(&fwResult));
    //TEST_ASSERT(LWM2MCORE_FW_UPDATE_RESULT_UNSUPPORTED_PROTOCOL == fwResult);
    TEST_ASSERT(LWM2MCORE_FW_UPDATE_RESULT_INVALID_URI == fwResult);
    // Check that the URL was erased
    TEST_ASSERT(DWL_OK == ReadPkgDwlWorkspace(&workspace));
    TEST_ASSERT(0 == strlen(workspace.url));

    printf("\n====== downloader_InitializeDownload: too long URL ======\n");
    // Test too long URL (max length = 255 char. Test with 256 chars)
    result = downloader_InitializeDownload( LWM2MCORE_FW_UPDATE_TYPE,
                                            0,
                                            (char*)TOO_LONG_URL,
                                            strlen(TOO_LONG_URL));
    TEST_ASSERT(LWM2MCORE_ERR_INVALID_ARG == result);
    // Check FW update state and result
    TEST_ASSERT(LWM2MCORE_ERR_COMPLETED_OK == downloader_GetFwUpdateState(&fwState));
    TEST_ASSERT(LWM2MCORE_FW_UPDATE_STATE_IDLE == fwState);
    TEST_ASSERT(LWM2MCORE_ERR_COMPLETED_OK == downloader_GetFwUpdateResult(&fwResult));
    TEST_ASSERT(LWM2MCORE_FW_UPDATE_RESULT_INVALID_URI == fwResult);
    // Check that the URL was not stored
    TEST_ASSERT(DWL_OK == ReadPkgDwlWorkspace(&workspace));
    TEST_ASSERT(0 == strlen(workspace.url));

    // Test lwm2mcore_StartPackageDownloader public API
    // Invalid protocol
    printf("\n====== Tests on lwm2mcore_StartPackageDownloader ======\n");
    printf("\n====== lwm2mcore_StartPackageDownloader: get size from invalid protocol ======\n");
    TEST_ASSERT(DWL_OK == ReadPkgDwlWorkspace(&workspace));
    strncpy(workspace.url, INVALID_PROTOCOL_URL, sizeof(workspace.url));
    workspace.packageSize = 0;
    workspace.updateType = LWM2MCORE_FW_UPDATE_TYPE;
    TEST_ASSERT(DWL_OK == WritePkgDwlWorkspace(&workspace));
    TEST_ASSERT( LWM2MCORE_ERR_COMPLETED_OK == lwm2mcore_StartPackageDownloader(NULL));
    // Check FW update state and result
    TEST_ASSERT(LWM2MCORE_ERR_COMPLETED_OK == downloader_GetFwUpdateState(&fwState));
    TEST_ASSERT(LWM2MCORE_FW_UPDATE_STATE_IDLE == fwState);
    TEST_ASSERT(LWM2MCORE_ERR_COMPLETED_OK == downloader_GetFwUpdateResult(&fwResult));
    // Result is set to invalid URI. unsupported protocol is managed by
    // downloader_InitializeDownload API
    TEST_ASSERT(LWM2MCORE_FW_UPDATE_RESULT_INVALID_URI == fwResult);

    printf("\n====== lwm2mcore_StartPackageDownloader: download from invalid protocol ======\n");
    // Invalid protocol
    TEST_ASSERT(DWL_OK == ReadPkgDwlWorkspace(&workspace));
    strncpy(workspace.url, INVALID_PROTOCOL_URL, sizeof(workspace.url));
    workspace.packageSize = 100;
    TEST_ASSERT(DWL_OK == WritePkgDwlWorkspace(&workspace));
    TEST_ASSERT( LWM2MCORE_ERR_COMPLETED_OK == lwm2mcore_StartPackageDownloader(NULL));
    // Check FW update state and result
    TEST_ASSERT(LWM2MCORE_ERR_COMPLETED_OK == downloader_GetFwUpdateState(&fwState));
    TEST_ASSERT(LWM2MCORE_FW_UPDATE_STATE_IDLE == fwState);
    TEST_ASSERT(LWM2MCORE_ERR_COMPLETED_OK == downloader_GetFwUpdateResult(&fwResult));
    // Result is set to invalid URI. unsupported protocol is managed by
    // downloader_InitializeDownload API
    TEST_ASSERT(LWM2MCORE_FW_UPDATE_RESULT_INVALID_URI == fwResult);

    printf("\n====== lwm2mcore_StartPackageDownloader: download invalid package header ======\n");
    // Invalid package
    DeletePkgDwlWorkspace();
    TEST_ASSERT(DWL_OK == ReadPkgDwlWorkspace(&workspace));
    // Prepare the file to download
    TEST_ASSERT(true == test_lwm2mcore_PrepareFilesForDownload(IMAGE_FILE_PREFIX, packageUrl));
    strncpy(workspace.url, packageUrl, strlen(packageUrl));
    workspace.updateType = LWM2MCORE_FW_UPDATE_TYPE;
    TEST_ASSERT(DOWNLOADER_OK == downloader_GetPackageSize(packageUrl, &packageSize));
    workspace.packageSize = packageSize;
    TEST_ASSERT(DWL_OK == WritePkgDwlWorkspace(&workspace));
    TEST_ASSERT(LWM2MCORE_ERR_COMPLETED_OK == lwm2mcore_StartPackageDownloader(NULL));
    // Check FW update state and result
    TEST_ASSERT(LWM2MCORE_ERR_COMPLETED_OK == downloader_GetFwUpdateState(&fwState));
    TEST_ASSERT(LWM2MCORE_FW_UPDATE_STATE_IDLE == fwState);
    TEST_ASSERT(LWM2MCORE_ERR_COMPLETED_OK == downloader_GetFwUpdateResult(&fwResult));
    TEST_ASSERT(LWM2MCORE_FW_UPDATE_RESULT_UNSUPPORTED_PKG_TYPE == fwResult);
    TEST_ASSERT(Lwm2mcoreEvent == LWM2MCORE_EVENT_PACKAGE_DOWNLOAD_FAILED);
    Lwm2mcoreEvent = LWM2MCORE_EVENT_LAST;

    printf("\n====== lwm2mcore_StartPackageDownloader: invalid signature ======\n");
    // Invalid signature: SOTA package for FOTA
    // (incorrect signature because it's not the right public key)
    DeletePkgDwlWorkspace();
    TEST_ASSERT(DWL_OK == ReadPkgDwlWorkspace(&workspace));
    // Prepare the file to download
    TEST_ASSERT(true == test_lwm2mcore_PrepareFilesForDownload(BAD_SIGNATURE_FILE_PREFIX,
                                                               packageUrl));
    strncpy(workspace.url, packageUrl, strlen(packageUrl));
    workspace.updateType = LWM2MCORE_FW_UPDATE_TYPE;
    TEST_ASSERT( DOWNLOADER_OK == downloader_GetPackageSize(packageUrl, &packageSize));
    workspace.packageSize = packageSize;
    TEST_ASSERT(DWL_OK == WritePkgDwlWorkspace(&workspace));
    TEST_ASSERT( LWM2MCORE_ERR_COMPLETED_OK == lwm2mcore_StartPackageDownloader(NULL));
    // Check FW update state and result
    TEST_ASSERT(LWM2MCORE_ERR_COMPLETED_OK == downloader_GetFwUpdateState(&fwState));
    TEST_ASSERT(LWM2MCORE_FW_UPDATE_STATE_IDLE == fwState);
    TEST_ASSERT(LWM2MCORE_ERR_COMPLETED_OK == downloader_GetFwUpdateResult(&fwResult));
    TEST_ASSERT(LWM2MCORE_FW_UPDATE_RESULT_VERIFY_ERROR == fwResult);
    TEST_ASSERT(Lwm2mcoreEvent == LWM2MCORE_EVENT_PACKAGE_DOWNLOAD_FAILED);
    Lwm2mcoreEvent = LWM2MCORE_EVENT_LAST;

    printf("\n====== lwm2mcore_StartPackageDownloader: invalid CRC ======\n");
    // Invalid CRC
    // Package is an hello world (so SOTA) with an updated fake CRC)
    DeletePkgDwlWorkspace();
    TEST_ASSERT(DWL_OK == ReadPkgDwlWorkspace(&workspace));
    // Prepare the file to download
    TEST_ASSERT(true == test_lwm2mcore_PrepareFilesForDownload(BAD_CRC_FILE_PREFIX, packageUrl));
    strncpy(workspace.url, packageUrl, strlen(packageUrl));
    workspace.updateType = LWM2MCORE_FW_UPDATE_TYPE;
    TEST_ASSERT( DOWNLOADER_OK == downloader_GetPackageSize(packageUrl, &packageSize));
    workspace.packageSize = packageSize;
    TEST_ASSERT(DWL_OK == WritePkgDwlWorkspace(&workspace));
    TEST_ASSERT( LWM2MCORE_ERR_COMPLETED_OK == lwm2mcore_StartPackageDownloader(NULL));
    // Check FW update state and result
    TEST_ASSERT(LWM2MCORE_ERR_COMPLETED_OK == downloader_GetFwUpdateState(&fwState));
    TEST_ASSERT(LWM2MCORE_FW_UPDATE_STATE_IDLE == fwState);
    TEST_ASSERT(LWM2MCORE_ERR_COMPLETED_OK == downloader_GetFwUpdateResult(&fwResult));
    TEST_ASSERT(LWM2MCORE_FW_UPDATE_RESULT_VERIFY_ERROR == fwResult);
    TEST_ASSERT(Lwm2mcoreEvent == LWM2MCORE_EVENT_PACKAGE_DOWNLOAD_FAILED);
    Lwm2mcoreEvent = LWM2MCORE_EVENT_LAST;

    printf("\n====== lwm2mcore_StartPackageDownloader: valid package ======\n");
    // Valid package
    DeletePkgDwlWorkspace();
    TEST_ASSERT(DWL_OK == ReadPkgDwlWorkspace(&workspace));
    // Prepare the file to download
    TEST_ASSERT(true == test_lwm2mcore_PrepareFilesForDownload(VALID_FILE_PREFIX, packageUrl));
    strncpy(workspace.url, packageUrl, strlen(packageUrl));
    workspace.updateType = LWM2MCORE_FW_UPDATE_TYPE;
    TEST_ASSERT(DWL_OK == WritePkgDwlWorkspace(&workspace));
    TEST_ASSERT( DOWNLOADER_OK == downloader_GetPackageSize(packageUrl, &packageSize));
    TEST_ASSERT( LWM2MCORE_ERR_COMPLETED_OK == lwm2mcore_StartPackageDownloader(NULL));
    // Check FW update state and result
    TEST_ASSERT(LWM2MCORE_ERR_COMPLETED_OK == downloader_GetFwUpdateState(&fwState));
    TEST_ASSERT(LWM2MCORE_FW_UPDATE_STATE_DOWNLOADED == fwState);
    TEST_ASSERT(LWM2MCORE_ERR_COMPLETED_OK == downloader_GetFwUpdateResult(&fwResult));
    TEST_ASSERT(LWM2MCORE_FW_UPDATE_RESULT_DEFAULT_NORMAL == fwResult);

    TEST_ASSERT(LWM2MCORE_ERR_COMPLETED_OK == lwm2mcore_GetLastHttpErrorCode(&errorCode));
    TEST_ASSERT(200 == errorCode);

    printf("\n====== Tests on lwm2mcore_GetDownloadInfo ======\n");
    printf("\n====== lwm2mcore_GetDownloadInfo: Invalid parameters ======\n");
    TEST_ASSERT(LWM2MCORE_ERR_INVALID_ARG == lwm2mcore_GetDownloadInfo(NULL, NULL));
    TEST_ASSERT(LWM2MCORE_ERR_INVALID_ARG == lwm2mcore_GetDownloadInfo(&updateType, NULL));
    TEST_ASSERT(LWM2MCORE_ERR_INVALID_ARG == lwm2mcore_GetDownloadInfo(NULL, &packageSize));

    printf("\n====== lwm2mcore_GetDownloadInfo: no resume ======\n");
    TEST_ASSERT(DWL_OK == ReadPkgDwlWorkspace(&workspace));
    workspace.updateType = LWM2MCORE_MAX_UPDATE_TYPE;
    workspace.packageSize = 0;
    // Prepare the file to download
    TEST_ASSERT(true == test_lwm2mcore_PrepareFilesForDownload(VALID_FILE_PREFIX, packageUrl));
    strncpy(workspace.url, packageUrl, strlen(packageUrl));
    TEST_ASSERT(DWL_OK == WritePkgDwlWorkspace(&workspace));
    TEST_ASSERT(LWM2MCORE_ERR_INVALID_STATE == lwm2mcore_GetDownloadInfo(&updateType,
                                                                         &packageSize));

    TEST_ASSERT(DWL_OK == ReadPkgDwlWorkspace(&workspace));
    workspace.updateType = LWM2MCORE_FW_UPDATE_TYPE;
    workspace.packageSize = 0;
    memset(workspace.url, 0, LWM2MCORE_PACKAGE_URI_MAX_BYTES);
    TEST_ASSERT(DWL_OK == WritePkgDwlWorkspace(&workspace));
    TEST_ASSERT(LWM2MCORE_ERR_INVALID_STATE == lwm2mcore_GetDownloadInfo(&updateType,
                                                                         &packageSize));

    printf("\n====== lwm2mcore_GetDownloadInfo: resume ======\n");
    TEST_ASSERT(DWL_OK == ReadPkgDwlWorkspace(&workspace));
    workspace.updateType = LWM2MCORE_FW_UPDATE_TYPE;
    workspace.packageSize = 1000;
    // Prepare the file to download
    TEST_ASSERT(true == test_lwm2mcore_PrepareFilesForDownload(VALID_FILE_PREFIX, packageUrl));
    strncpy(workspace.url, packageUrl, strlen(packageUrl));
    TEST_ASSERT(DWL_OK == WritePkgDwlWorkspace(&workspace));
    TEST_ASSERT(LWM2MCORE_ERR_COMPLETED_OK == lwm2mcore_GetDownloadInfo(&updateType,
                                                                        &packageSize));
    TEST_ASSERT(LWM2MCORE_FW_UPDATE_TYPE == updateType);
    TEST_ASSERT(1000 == packageSize);

    printf("\n====== Tests on lwm2mcore_PackageDownloaderReceiveData ======\n");
    printf("\n====== lwm2mcore_PackageDownloaderReceiveData: invalid arguments ======\n");
    size = 10;
    TEST_ASSERT(DWL_FAULT == lwm2mcore_PackageDownloaderReceiveData(NULL, size, NULL));
    TEST_ASSERT(DWL_FAULT == lwm2mcore_PackageDownloaderReceiveData(bufferRead, size, NULL));

    printf("\n====== lwm2mcore_PackageDownloaderReceiveData: valid buffer ======\n");
    size = 0;
    TEST_ASSERT(DWL_OK == lwm2mcore_PackageDownloaderReceiveData(bufferRead, size, NULL));

    // Real data case was already made by previous tests

    TEST_ASSERT(DWL_OK == DeletePkgDwlWorkspace());
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

//-------------------------------------------------------------------------------------------------
/**
 * Test function for upodate package APIs
 */
//--------------------------------------------------------------------------------------------------
static void test_downloader
(
    void
)
{
    PackageDownloaderWorkspace_t workspace;
    char packageUrl[LWM2MCORE_PACKAGE_URI_MAX_BYTES];

    printf("\n====== test_downloader: test retry mechanism to get the package size ======\n");
    DeletePkgDwlWorkspace();
    TEST_ASSERT(DWL_OK == ReadPkgDwlWorkspace(&workspace));
    // Prepare the file to download
    memset(packageUrl, 0, LWM2MCORE_PACKAGE_URI_MAX_BYTES);
    TEST_ASSERT(true == test_lwm2mcore_PrepareFilesForDownload(VALID_FILE_PREFIX, packageUrl));
    strncpy(workspace.url, packageUrl, strlen(packageUrl));
    workspace.updateType = LWM2MCORE_FW_UPDATE_TYPE;
    TEST_ASSERT(DWL_OK == WritePkgDwlWorkspace(&workspace));

    // Set the lwm2mcore_ConnectForDownload error code to LWM2MCORE_ERR_GENERAL_ERROR
    test_setConnectForDownloadResult(LWM2MCORE_ERR_GENERAL_ERROR);
    // Reset the counter to lwm2mcore_ConnectForDownload calls
    test_resetCallNumberConnectForDownload();
    // Proceed to the global download mechanism
    downloader_PackageDownloadHandler();
    // The error cause can not be retreived, check if the lwm2mcore_ConnectForDownload call number
    // is equal to the maximum retry
    TEST_ASSERT(DWL_RETRIES == test_getCallNumberConnectForDownload());
    // Reset the counter to lwm2mcore_ConnectForDownload calls
    test_resetCallNumberConnectForDownload();
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

    printf("======== test of test_lwm2mcore_DisconnectWithDeregister() ========\n");
    test_lwm2mcore_DisconnectWithDeregister();

    printf("======== test of lwm2mcore_Free() ========\n");
    test_lwm2mcore_Free();

    printf("======== test of lwm2mcore_Init() ========\n");
    test_lwm2mcore_Init();

    printf("======== test of lwm2mcore_Connect() ========\n");
    test_lwm2mcore_Connect();

    printf("======== test of lwm2mcore_Disconnect() ========\n");
    test_lwm2mcore_Disconnect();

    printf("======== test of smanager_SendSessionEvent() ========\n");
    test_lwm2mcore_Init();
    test_smanager_SendSessionEvent();

    printf("======== test of lwm2mcore_Free() ========\n");
    test_lwm2mcore_Free();

    printf("======== test of lwm2mcore_SetLifetime() ========\n");
    test_lwm2mcore_SetLifetime();

    printf("======== test of lwm2mcore_UpdateSwList() ========\n");
    test_lwm2mcore_UpdateSwList();

    printf("======== test of lwm2mcore_ResourceRead() ========\n");
    test_lwm2mcore_ResourceRead();

    printf("======== test of downloader() ========\n");
    test_lwm2mcore_Downloader();

    if (0)
    {
        printf("======== test of update package() ========\n");
        test_lwm2mcore_UpdatePackage();
    }

    printf("======== test of downloader() ========\n");
    test_downloader();

    printf("======== UnitTest of lwm2mcore ends with SUCCESS ========\n");

    exit(EXIT_SUCCESS);
}
