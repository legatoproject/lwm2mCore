/**
 * @file lwm2mcoreSession.c
 *
 * LWM2M core file for session management
 *
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

/* include files */
#include <string.h>
#include <lwm2mcore/lwm2mcore.h>
#include <lwm2mcore/security.h>
#include <lwm2mcore/coapHandlers.h>
#include <lwm2mcore/timer.h>
#include <lwm2mcore/udp.h>
#include <lwm2mcore/update.h>
#include "liblwm2m.h"
#include "internals.h"
#include "objects.h"
#include "dtlsConnection.h"
#include "sessionManager.h"
#include "handlers.h"
#include "aclConfiguration.h"
#include "bootstrapConfiguration.h"

//--------------------------------------------------------------------------------------------------
/**
 *  Context
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_context_t* Lwm2mcoreCtxPtr = NULL;

//--------------------------------------------------------------------------------------------------
/**
 * Socket configuration variable
 */
//--------------------------------------------------------------------------------------------------
static  lwm2mcore_SocketConfig_t SocketConfig;

//--------------------------------------------------------------------------------------------------
/**
 *  Context
 */
//--------------------------------------------------------------------------------------------------
static smanager_ClientData_t* DataCtxPtr;

//--------------------------------------------------------------------------------------------------
/**
 *  Callback for events
 */
//--------------------------------------------------------------------------------------------------
static lwm2mcore_StatusCb_t StatusCb = NULL;

//--------------------------------------------------------------------------------------------------
/**
 *  Callback for data push events
 */
//--------------------------------------------------------------------------------------------------
static lwm2mcore_PushAckCallback_t PushCb = NULL;

//--------------------------------------------------------------------------------------------------
/**
 *  Static boolean for bootstrap notification
 */
//--------------------------------------------------------------------------------------------------
static bool BootstrapSession = false;

//--------------------------------------------------------------------------------------------------
/**
 *  Inactivity timeout after which a notification will be sent.
 */
//--------------------------------------------------------------------------------------------------
#define INACTIVE_TIMEOUT_SECONDS    20

//--------------------------------------------------------------------------------------------------
/**
 *  Client state bootstrapping / registered etc.,
 */
//--------------------------------------------------------------------------------------------------
#ifdef LWM2M_BOOTSTRAP
static lwm2m_client_state_t PreviousState;
#endif

//--------------------------------------------------------------------------------------------------
/**
 *                      PRIVATE FUNCTIONS
 */
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Initialize the LwM2M context object
 *
 * @return
 *  - pointer to LwM2M context object
 */
//--------------------------------------------------------------------------------------------------
static lwm2mcore_context_t* InitContext
(
    smanager_ClientData_t* dataPtr          ///< [IN] Context
)
{
    dataPtr->lwm2mcoreCtxPtr = (lwm2mcore_context_t*)lwm2m_malloc(sizeof(lwm2mcore_context_t));
    LWM2MCORE_ASSERT(dataPtr->lwm2mcoreCtxPtr);
    memset(dataPtr->lwm2mcoreCtxPtr, 0, sizeof(lwm2mcore_context_t));
    return dataPtr->lwm2mcoreCtxPtr;
}

//--------------------------------------------------------------------------------------------------
/**
 * Callback function called when CoAP data push is acknowledged or timed out
 */
//--------------------------------------------------------------------------------------------------
static void PushCallbackHandler
(
    lwm2m_ack_result_t result,    ///< [IN] Acknowledge result
    uint16_t mid                  ///< [IN] Message Identifier
)
{
    lwm2mcore_AckResult_t ack;
    switch (result)
    {
        case LWM2M_ACK_RECEIVED:
            ack = LWM2MCORE_ACK_RECEIVED;
            break;
        case LWM2M_ACK_TIMEOUT:
            ack = LWM2MCORE_ACK_TIMEOUT;
            break;
        default:
            LOG("Unrecognized acknowledge type");
            return;
    }

    if (PushCb)
    {
        PushCb(ack, mid);
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Function called by the LWM2M core to initiate a connection to a server
 *
 * @return
 *  - dtls_Connection_t structure adress on success
 *  - NULL on failure
 */
//--------------------------------------------------------------------------------------------------
void* lwm2m_connect_server
(
    uint16_t secObjInstID,      ///< [IN] Security object instance ID
    void * userDataPtr          ///< [IN] User data
)
{
    smanager_ClientData_t* dataPtr;
    lwm2m_list_t* instancePtr;
    dtls_Connection_t* newConnPtr = NULL;
    dataPtr = (smanager_ClientData_t*)userDataPtr;

    if (dataPtr)
    {
        lwm2m_object_t* securityObjPtr = dataPtr->securityObjPtr;

        instancePtr = LWM2M_LIST_FIND(dataPtr->securityObjPtr->instanceList, secObjInstID);
        if (!instancePtr)
        {
            return NULL;
        }

        newConnPtr = dtls_CreateConnection(dataPtr->connListPtr,
                                           dataPtr->sock,
                                           securityObjPtr,
                                           instancePtr->id,
                                           dataPtr->lwm2mHPtr,
                                           dataPtr->addressFamily);
        if ((!newConnPtr) || !(newConnPtr->dtlsContextPtr))
        {
            LOG("Connection creation failed");
            if (newConnPtr)
            {
                lwm2m_free(newConnPtr);
            }
            return NULL;
        }

        dataPtr->connListPtr = newConnPtr;
    }

    return (void *)newConnPtr;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function called by the LWM2M core to close a connection from a server
 */
//--------------------------------------------------------------------------------------------------
void lwm2m_close_connection
(
    void* sessionHPtr,     ///< [IN] Connection context
    void* userDataPtr      ///< [IN] Context
)
{
    smanager_ClientData_t* appDataPtr;
    dtls_Connection_t* targetPtr;

    appDataPtr = (smanager_ClientData_t*)userDataPtr;
    targetPtr = (dtls_Connection_t*)sessionHPtr;

    if ((NULL != appDataPtr) && (NULL != targetPtr))
    {
        if (targetPtr == appDataPtr->connListPtr)
        {
            appDataPtr->connListPtr = targetPtr->nextPtr;
            lwm2m_free(targetPtr);
        }
        else
        {
            dtls_Connection_t* parentPtr;
            parentPtr = appDataPtr->connListPtr;
            while ((parentPtr != NULL) && (parentPtr->nextPtr != targetPtr))
            {
                parentPtr = parentPtr->nextPtr;
            }
            if (NULL != parentPtr)
            {
                parentPtr->nextPtr = targetPtr->nextPtr;
                lwm2m_free(targetPtr);
            }
        }
        dtls_UpdateDtlsList(appDataPtr->connListPtr);
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Report CoAP status
 *
 * @return
 *  - CoAP error code
 */
//--------------------------------------------------------------------------------------------------
uint8_t lwm2m_report_coap_status
(
    const char* file,  ///< [IN] File path from where this function is called
    const char* func,  ///< [IN] Name of the caller function
    int code           ///< [IN] CoAP error code as defined in RFC 7252 section 12.1.2
)
{
    // Extract file name from path
    char* fileName = strrchr(file, '/');
    if (NULL == fileName)
    {
        fileName = (char*)file;
    }
    LOG_ARG("[%s:%s] %d.%.2d", fileName, func, (code >> 5), (code & 0x1f));

    lwm2mcore_ReportCoapResponseCode(code);

    return (uint8_t)code;
}

//--------------------------------------------------------------------------------------------------
/**
 * Update bootstrap state and backup security object if bootstrap succeeded.
 */
//--------------------------------------------------------------------------------------------------
#ifdef LWM2M_BOOTSTRAP
static void UpdateBootstrapInfo
(
    lwm2m_client_state_t* previousBsStatePtr,   ///< [IN] Bootstrap state
    lwm2m_context_t* contextPtr                 ///< [IN] Context
)
{
    static bool bootstrapDone = false;

    if (*previousBsStatePtr != contextPtr->state)
    {
        *previousBsStatePtr = contextPtr->state;
        switch(contextPtr->state)
        {
            case STATE_BOOTSTRAPPING:
            {
                bootstrapDone = true;
            }
            break;

            // if we go through bootstrap and registration succeeds, backup security object.
            case STATE_READY:
            {
                if (bootstrapDone)
                {
                    LOG("Backup security object.");
                    //TODO objSecurity_Backup(&context->objectList[0]);
                }
            }
            break;

            default:
            {
            }
            break;
        }
    }
}
#endif

//--------------------------------------------------------------------------------------------------
/**
 *  LwM2M client inactivity timeout.
 */
//--------------------------------------------------------------------------------------------------
static void Lwm2mClientInactivityHandler
(
    void
)
{
    LOG_ARG("client inactive for %d seconds", INACTIVE_TIMEOUT_SECONDS);

    /* Restart the timer for monitoring next period */
    if (false == lwm2mcore_TimerSet(LWM2MCORE_TIMER_INACTIVITY,
                                    INACTIVE_TIMEOUT_SECONDS,
                                    Lwm2mClientInactivityHandler))
    {
        LOG("Error re-launching inactivity timer");
    }

    /* Notify that the session is inactive */
    smanager_SendSessionEvent(EVENT_TYPE_REGISTRATION, EVENT_STATUS_INACTIVE);
}

//--------------------------------------------------------------------------------------------------
/**
 *  LwM2M client step that handles data transmit.
 */
//--------------------------------------------------------------------------------------------------
static void Lwm2mClientStepHandler
(
    void
)
{
    int result = 0;

    static struct timeval tv;
    tv.tv_sec = 60;
    tv.tv_usec = 0;

    LOG("Entering");

    /* This function does two things:
     * - first it does the work needed by liblwm2m (eg. (re)sending some packets).
     * - Secondly it adjusts the timeout value (default 60s) depending on the state of the
     *   transaction
     *   (eg. retransmission) and the time between the next operation
     */

    result = lwm2m_step(DataCtxPtr->lwm2mHPtr, &(tv.tv_sec));
    if (result != 0)
    {
       LOG_ARG("lwm2m_step() failed: 0x%X.", result);
#ifdef LWM2M_BOOTSTRAP
       if (STATE_BOOTSTRAPPING == PreviousState)
       {
#ifdef WITH_LOGS
           LOG("[BOOTSTRAP] restore security and server objects.");
#endif
           //prv_restore_objects(ClientCtxt.lwm2mH);
           DataCtxPtr->lwm2mHPtr->state = STATE_INITIAL;
       }
#endif
    }

    /* Launch timer step */
    if (false == lwm2mcore_TimerSet(LWM2MCORE_TIMER_STEP, tv.tv_sec, Lwm2mClientStepHandler))
    {
        LOG("ERROR to launch the step timer");
    }

#ifdef LWM2M_BOOTSTRAP
    UpdateBootstrapInfo(&PreviousState, DataCtxPtr->lwm2mHPtr);
#endif

    LOG("LwM2M step completed.");
}

//--------------------------------------------------------------------------------------------------
/**
 *  Convert CoAP response code to LwM2M standard error codes
 */
//--------------------------------------------------------------------------------------------------
static uint32_t ConvertToCoapCode
(
    lwm2mcore_CoapResponseCode_t response
)
{
    uint32_t coapCode;

    switch(response)
    {
        case COAP_RESOURCE_CHANGED:
        {
           coapCode = CHANGED_2_04;
           break;
        }
        case COAP_CONTENT_AVAILABLE:
        {
            coapCode =  CONTENT_2_05;
            break;
        }
        case COAP_BAD_REQUEST:
        {
            coapCode = BAD_REQUEST_4_00;
            break;
        }
        case COAP_METHOD_UNAUTHORIZED:
        {
            coapCode = UNAUTHORIZED_4_01;
            break;
        }
        case COAP_NOT_FOUND:
        {
            coapCode = NOT_FOUND_4_04;
            break;
        }
        case COAP_METHOD_NOT_ALLOWED:
        {
            coapCode = METHOD_NOT_ALLOWED_4_05;
            break;
        }
        case COAP_PRECONDITION_FAILED:
        {
            coapCode = PRECONDITION_FAILED_4_12;
            break;
        }
        case COAP_REQUEST_ENTITY_TOO_LARGE:
        {
            coapCode = REQUEST_ENTITY_TOO_LARGE_4_13;
            break;
        }
        case COAP_UNSUPPORTED_MEDIA_TYPE:
        {
            coapCode = UNSUPPORTED_MEDIA_TYPE_4_15;
            break;
        }
        case COAP_INTERNAL_ERROR:
        {
            coapCode = INTERNAL_SERVER_ERROR_5_00;
            break;
        }
        default:
            coapCode = INTERNAL_SERVER_ERROR_5_00;
    }

    return coapCode;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to send status event to the application, using the callback stored in the LwM2MCore
 * session manager
 */
//--------------------------------------------------------------------------------------------------
void smanager_SendStatusEvent
(
    lwm2mcore_Status_t status
)
{
    // Check if a status callback is available
    if (!StatusCb)
    {
        LOG("No StatusCb to send status events");
        return;
    }

    // Send the status event notification
    StatusCb(status);
}

//--------------------------------------------------------------------------------------------------
/**
 * Function for session events
 */
//--------------------------------------------------------------------------------------------------
void smanager_SendSessionEvent
(
    smanager_EventType_t eventId,         ///< [IN] Event Id
    smanager_EventStatus_t eventstatus    ///< [IN] Event status
)
{
    lwm2mcore_Status_t status;

    switch (eventId)
    {
        case EVENT_TYPE_BOOTSTRAP:
        {
            switch (eventstatus)
            {
                case EVENT_STATUS_STARTED:
                {
                    LOG("BOOTSTRAP START");
                    BootstrapSession = true;
                }
                break;

                case EVENT_STATUS_DONE_SUCCESS:
                {
                    LOG("BOOTSTRAP DONE");
                    BootstrapSession = false;
                    omanager_StoreCredentials();
                    omanager_StoreAclConfiguration();
                }
                break;

                case EVENT_STATUS_DONE_FAIL:
                {
                    LOG("BOOTSTRAP FAILURE");
                    status.event = LWM2MCORE_EVENT_SESSION_FAILED;
                    smanager_SendStatusEvent(status);
                }
                break;

                default:
                break;
            }
        }
        break;

        case EVENT_TYPE_REGISTRATION:
        {
            switch (eventstatus)
            {
                case EVENT_STATUS_STARTED:
                {
                    LOG("REGISTER START");
                }
                break;

                case EVENT_STATUS_DONE_SUCCESS:
                {
                    LOG("REGISTER DONE");

                    status.event = LWM2MCORE_EVENT_SESSION_STARTED;
                    smanager_SendStatusEvent(status);

                    status.event = LWM2MCORE_EVENT_LWM2M_SESSION_TYPE_START;
                    status.u.session.type = LWM2MCORE_SESSION_DEVICE_MANAGEMENT;
                    smanager_SendStatusEvent(status);

                    // Check if a download should be resumed
                    if (LWM2MCORE_ERR_COMPLETED_OK != lwm2mcore_ResumePackageDownload())
                    {
                        LOG("Error while checking download resume");
                    }

                    /* Launch inactivity timer to monitor inactivity during registered state */
                    if (false == lwm2mcore_TimerSet(LWM2MCORE_TIMER_INACTIVITY,
                                                    INACTIVE_TIMEOUT_SECONDS,
                                                    Lwm2mClientInactivityHandler))
                    {
                        LOG("Error launching client inactivity timer");
                    }
                }
                break;

                case EVENT_STATUS_INACTIVE:
                {
                    LOG("Session inactive");

                    status.event = LWM2MCORE_EVENT_LWM2M_SESSION_INACTIVE;
                    smanager_SendStatusEvent(status);
                }
                break;

                case EVENT_STATUS_DONE_FAIL:
                {
                    LOG("REGISTER FAILURE");
                    status.event = LWM2MCORE_EVENT_SESSION_FAILED;
                    smanager_SendStatusEvent(status);

                    /* Delete DM credentials in order to force a connection to the BS server */
                    LOG("DELETE DM CREDENTIALS");
                    omanager_DeleteDmCredentials();
                }
                break;

                default:
                break;
            }
        }
        break;

        case EVENT_TYPE_REG_UPDATE:
        {
            switch (eventstatus)
            {
                case EVENT_STATUS_STARTED:
                {
                    LOG("REG UPDATE START");
                }
                break;

                case EVENT_STATUS_DONE_SUCCESS:
                {
                    LOG("REG UPDATE DONE");
                }
                break;

                case EVENT_STATUS_DONE_FAIL:
                {
                    LOG("REG UPDATE FAILURE");
                }
                break;

                default:
                break;
            }
        }
        break;

        case EVENT_TYPE_DEREG:
        {
            switch (eventstatus)
            {
                case EVENT_STATUS_STARTED:
                {
                    LOG("DEREGISTER START");
                }
                break;

                case EVENT_STATUS_DONE_SUCCESS:
                {
                    LOG("DEREGISTER DONE");
                }
                break;

                case EVENT_STATUS_DONE_FAIL:
                {
                    LOG("DEREGISTER FAILURE");
                }
                break;

                default:
                break;
            }
        }
        break;

        case EVENT_TYPE_AUTHENTICATION:
        {
            switch (eventstatus)
            {
                case EVENT_STATUS_STARTED:
                {
                    LOG ("AUTHENTICATION START");
                    status.event = LWM2MCORE_EVENT_AUTHENTICATION_STARTED;
                    if (BootstrapSession)
                    {
                        status.u.session.type = LWM2MCORE_SESSION_BOOTSTRAP;
                    }
                    else
                    {
                        status.u.session.type = LWM2MCORE_SESSION_DEVICE_MANAGEMENT;
                    }
                    smanager_SendStatusEvent(status);
                }
                break;

                case EVENT_STATUS_DONE_SUCCESS:
                {
                    LOG("AUTHENTICATION DONE");

                    if (BootstrapSession)
                    {
                        status.event = LWM2MCORE_EVENT_SESSION_STARTED;
                        smanager_SendStatusEvent(status);

                        status.event = LWM2MCORE_EVENT_LWM2M_SESSION_TYPE_START;
                        status.u.session.type = LWM2MCORE_SESSION_BOOTSTRAP;
                        smanager_SendStatusEvent(status);
                    }
                }
                break;

                case EVENT_STATUS_DONE_FAIL:
                {
                    LOG("AUTHENTICATION FAILURE");
                    status.event = LWM2MCORE_EVENT_AUTHENTICATION_FAILED;
                    if (BootstrapSession)
                    {
                        status.u.session.type = LWM2MCORE_SESSION_BOOTSTRAP;
                    }
                    else
                    {
                        status.u.session.type = LWM2MCORE_SESSION_DEVICE_MANAGEMENT;
                    }
                    smanager_SendStatusEvent(status);
                }
                break;

                default:
                break;
            }
        }
        break;

        case EVENT_TYPE_RESUMING:
        {
            switch (eventstatus)
            {
                case EVENT_STATUS_STARTED:
                {
                    LOG("DTLS RESUME START");
                }
                break;

                case EVENT_STATUS_DONE_SUCCESS:
                {
                    LOG("DTLS RESUME DONE");
                }
                break;

                case EVENT_STATUS_DONE_FAIL:
                {
                    LOG("DTLS RESUME FAILURE");
                }
                break;

                default:
                break;
            }
        }
        break;

        case EVENT_SESSION:
        {
            switch (eventstatus)
            {
                case EVENT_STATUS_STARTED:
                {
                    LOG("SESSION START");
                }
                break;

                case EVENT_STATUS_DONE_SUCCESS:
                {
                    LOG("SESSION DONE");
                    BootstrapSession = false;
                    status.event = LWM2MCORE_EVENT_SESSION_FINISHED;
                    smanager_SendStatusEvent(status);
                }
                break;

                case EVENT_STATUS_DONE_FAIL:
                {
                    LOG("SESSION FAILURE");
                    BootstrapSession = false;
                    status.event = LWM2MCORE_EVENT_SESSION_FAILED;
                    smanager_SendStatusEvent(status);
                }
                break;

                default:
                break;
            }
        }
        break;

        default:
        {
            LOG_ARG("Bad event %d", eventId);
        }
        break;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Callback called when the socked is opened
 */
//--------------------------------------------------------------------------------------------------
void lwm2mcore_UdpReceiveCb
(
    uint8_t* bufferPtr,                 ///< [IN] Received data
    uint32_t len,                       ///< [IN] Received data length
    struct sockaddr_storage *addrPtr,   ///< [INOUT] source address
    socklen_t addrLen,                  ///< [IN] addrPtr parameter length
    lwm2mcore_SocketConfig_t config     ///< [IN] Socket config
)
{
    smanager_ClientData_t* dataPtr;
    dtls_Connection_t* connPtr;
    int rc;

    LOG("avc UDP receive data callback");

    dataPtr = (smanager_ClientData_t*)config.instanceRef;

    connPtr = dtls_FindConnection(dataPtr->connListPtr, addrPtr, addrLen);
    if (!connPtr)
    {
        LOG("Failed to find an available DTLS connection");
        lwm2mcore_ReportUdpErrorCode(LWM2MCORE_UDP_RECV_ERR);
        return;
    }

    // Let liblwm2m respond to the query depending on the context
    LOG("Handling packet");
    rc = dtls_HandlePacket(connPtr, bufferPtr, (size_t)len);
    if (rc)
    {
        LOG_ARG("Failed to handle DTLS packet %d.", rc);
        lwm2mcore_ReportUdpErrorCode(LWM2MCORE_UDP_RECV_ERR);
        return;
    }

    /* Re-launch inactivity timer */
    if (lwm2mcore_TimerIsRunning(LWM2MCORE_TIMER_INACTIVITY))
    {
        if (false == lwm2mcore_TimerSet(LWM2MCORE_TIMER_INACTIVITY,
                                        INACTIVE_TIMEOUT_SECONDS,
                                        Lwm2mClientInactivityHandler))
        {
            LOG("Error launching client inactivity timer");
        }
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Private function to send an update message to the Device Management server
 *
 * @return
 *      - true if the treatment is launched
 *      - else false
 */
//--------------------------------------------------------------------------------------------------
bool omanager_UpdateRequest
(
    lwm2mcore_Ref_t instanceRef,    ///< [IN] instance reference
    bool withObjects                ///< [IN] indicates if supported object instance list needs to
                                    ///< be sent
)
{
    bool result = false;
    bool registered = false;
    smanager_ClientData_t* dataPtr = (smanager_ClientData_t*) instanceRef;

    if (NULL == dataPtr)
    {
        return false;
    }

    /* Check that the device is registered to DM server */
    if ((true == lwm2mcore_ConnectionGetType(instanceRef, &registered) && registered))
    {
        bool schedule = false;
        lwm2m_server_t* targetPtr = dataPtr->lwm2mHPtr->serverList;
        if (NULL == targetPtr)
        {
            LOG("serverList is NULL");
            return false;
        }

        while (targetPtr)
        {
            LOG_ARG("shortServerId %d", targetPtr->shortID);
            if (COAP_NO_ERROR != lwm2m_update_registration(dataPtr->lwm2mHPtr,
                                                           targetPtr->shortID,
                                                           withObjects))
            {
                LOG_ARG("Error while sending update registration on server %d", targetPtr->shortID);
            }
            else
            {
                schedule = true;
            }
            targetPtr = targetPtr->next;
        }

        if (schedule)
        {
            /* Stop the timer and launch it */
            if (false == lwm2mcore_TimerStop(LWM2MCORE_TIMER_STEP))
            {
                LOG("Error to stop the step timer");
            }

            /* Launch the LWM2MCORE_TIMER_STEP timer with 1 second to treat the update request */
            if (false == lwm2mcore_TimerSet(LWM2MCORE_TIMER_STEP, 1, Lwm2mClientStepHandler))
            {
                LOG("ERROR to launch the step timer for registration update");
            }
            else
            {
                result = true;
            }
        }
    }
    else
    {
        LOG("REG update is requested but the device is not registered");
    }
    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 *                      PUBLIC FUNCTIONS
 */
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Initialize the LWM2M core
 *
 * @return
 *  - instance reference
 *  - NULL in case of error
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Ref_t lwm2mcore_Init
(
    lwm2mcore_StatusCb_t eventCb    ///< [IN] event callback
)
{
    smanager_ClientData_t* dataPtr = NULL;

    if (NULL == eventCb)
    {
        return NULL;
    }

    StatusCb = eventCb;
    dataPtr = (smanager_ClientData_t*)lwm2m_malloc(sizeof(smanager_ClientData_t));
    LWM2MCORE_ASSERT(dataPtr);
    memset(dataPtr, 0, sizeof(smanager_ClientData_t));

     /* Initialize LWM2M agent */
    dataPtr->lwm2mHPtr = lwm2m_init(dataPtr);
    LWM2MCORE_ASSERT(dataPtr->lwm2mHPtr);

    dataPtr->lwm2mcoreCtxPtr = InitContext(dataPtr);
    Lwm2mcoreCtxPtr = dataPtr->lwm2mcoreCtxPtr;
    LWM2MCORE_ASSERT(dataPtr->lwm2mcoreCtxPtr);

    DataCtxPtr = dataPtr;

    LOG_ARG("Init done -> context %p", dataPtr);
    return (lwm2mcore_Ref_t)dataPtr;
}

//--------------------------------------------------------------------------------------------------
/**
 * Free the LWM2M core
 *
 */
//--------------------------------------------------------------------------------------------------
void lwm2mcore_Free
(
    lwm2mcore_Ref_t instanceRef     ///< [IN] instance reference
)
{
    smanager_ClientData_t* dataPtr = (smanager_ClientData_t*)instanceRef;

    if (NULL != dataPtr)
    {
        /* Free objects */
        omanager_ObjectsFree();
        omanager_FreeBootstrapInformation();
        omanager_FreeAclConfiguration();

        if (NULL != dataPtr->lwm2mcoreCtxPtr)
        {
            lwm2m_free(dataPtr->lwm2mcoreCtxPtr);
        }

        if (NULL != dataPtr)
        {
            lwm2m_free(dataPtr);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * LWM2M client entry point to initiate a connection
 *
 * @return
 *      - true if the treatment is launched
 *      - else false
 */
//--------------------------------------------------------------------------------------------------
bool lwm2mcore_Connect
(
    lwm2mcore_Ref_t instanceRef     ///< [IN] instance reference
)
{
    smanager_ClientData_t* dataPtr;
    if (!instanceRef)
    {
        LOG("Null instance reference");
        return false;
    }

    /* Create the socket */
    memset(&SocketConfig, 0, sizeof (lwm2mcore_SocketConfig_t));
    if (!lwm2mcore_UdpOpen(instanceRef, lwm2mcore_UdpReceiveCb, &SocketConfig))
    {
        LOG("Failed to open UDP connection");
        lwm2mcore_ReportUdpErrorCode(LWM2MCORE_UDP_OPEN_ERR);
        return false;
    }

    LOG_ARG("lwm2mcore_connect -> socket %d opened ", SocketConfig.sock);

    dataPtr = (smanager_ClientData_t*)instanceRef;
    dataPtr->sock = SocketConfig.sock;
    dataPtr->addressFamily = SocketConfig.af;

    /* Initialize the lwm2m client step timer */
    DataCtxPtr = dataPtr;
    if (!lwm2mcore_TimerSet(LWM2MCORE_TIMER_STEP, 1, Lwm2mClientStepHandler))
    {
        LOG("Failed to launch the 1st step timer");
    }

    LOG("LWM2M Client started");

    return true;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to send an update message to the Device Management server.
 *
 * This API can be used when the application wants to send a notification or during a firmware/app
 * update in order to be able to fully treat the scheduled update job
 *
 * @return
 *      - true if the treatment is launched
 *      - else false
 */
//--------------------------------------------------------------------------------------------------
bool lwm2mcore_Update
(
    lwm2mcore_Ref_t instanceRef     ///< [IN] instance reference
)
{
    if (!instanceRef)
    {
        LOG("Null instance reference");
        return false;
    }

    return omanager_UpdateRequest(instanceRef, false);
}

//--------------------------------------------------------------------------------------------------
 /**
 * @brief Function to notify change on an observed resource.
 *
 * @return
 *      - true if the notify was processed
 *      - else false
 */
//--------------------------------------------------------------------------------------------------
bool lwm2mcore_NotifyResourceChange
(
    lwm2mcore_Ref_t instanceRef,       ///< [IN] instance reference
    uint16_t objectId,                 ///< [IN] object identifier
    uint16_t objectInstanceId,         ///< [IN] object instance identifier
    uint16_t resourceId                ///< [IN] resource identifier
)
{
    if (!instanceRef)
    {
        LOG("Null instance reference");
        return false;
    }

    smanager_ClientData_t* dataPtr = (smanager_ClientData_t*)instanceRef;

    /* Check that the device is registered to DM server */
    bool registered = false;
    if ((true == lwm2mcore_ConnectionGetType(instanceRef, &registered) && registered))
    {
        lwm2m_server_t* targetPtr = dataPtr->lwm2mHPtr->serverList;
        if (NULL == targetPtr)
        {
            LOG("serverList is NULL");
            return false;
        }

        lwm2m_uri_t uriP;
        uriP.flag = LWM2M_URI_FLAG_OBJECT_ID | LWM2M_URI_FLAG_INSTANCE_ID |
                    LWM2M_URI_FLAG_RESOURCE_ID;
        uriP.objectId = objectId;
        uriP.instanceId = objectInstanceId;
        uriP.resourceId = resourceId;

        /* Notify observers */
        while (targetPtr)
        {
            LOG_ARG("shortServerId %d", targetPtr->shortID);
            lwm2m_resource_value_changed(dataPtr->lwm2mHPtr, &uriP);
            targetPtr = targetPtr->next;
        }
    }

    /* Do step */
    if (false == lwm2mcore_TimerSet(LWM2MCORE_TIMER_STEP, 0, Lwm2mClientStepHandler))
    {
        LOG("ERROR to launch the step timer");
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to close a connection
 *
 * @return
 *      - true if the treatment is launched
 *      - else false
 */
//--------------------------------------------------------------------------------------------------
bool lwm2mcore_Disconnect
(
    lwm2mcore_Ref_t instanceRef     ///< [IN] instance reference
)
{
    smanager_ClientData_t* dataPtr;

    if (!instanceRef)
    {
        LOG("Null instance reference");
        return false;
    }

    /* Stop package download if one is on-going */
    LOG("Suspend Download");
    lwm2mcore_SuspendPackageDownload();

    /* Stop the current timers */
    if (!lwm2mcore_TimerStop(LWM2MCORE_TIMER_STEP))
    {
        LOG("Failed to stop the step timer");
    }

    if (!lwm2mcore_TimerStop(LWM2MCORE_TIMER_INACTIVITY))
    {
        LOG("Failed to stop the inactivity timer");
    }

    dataPtr = (smanager_ClientData_t*) instanceRef;

    /* Stop the agent */
    lwm2m_close(dataPtr->lwm2mHPtr);
    dtls_FreeConnection(dataPtr->connListPtr);
    dataPtr->lwm2mHPtr = NULL;
    dataPtr->connListPtr = NULL;

    /* Close the socket */
    if (!lwm2mcore_UdpClose(SocketConfig))
    {
        LOG("Failed to close UDP connection");
        lwm2mcore_ReportUdpErrorCode(LWM2MCORE_UDP_CLOSE_ERR);
    }

    /* Zero-init the socket structure */
    memset(&SocketConfig, 0, sizeof(lwm2mcore_SocketConfig_t));

    /* Notify that the connection is stopped */
    smanager_SendSessionEvent(EVENT_SESSION, EVENT_STATUS_DONE_SUCCESS);

    return true;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to retrieve the status and the type of the current connection
 *
 * @return
 *      - true if the device is connected to any DM server
 *      - else false
 */
//--------------------------------------------------------------------------------------------------
bool lwm2mcore_ConnectionGetType
(
    lwm2mcore_Ref_t instanceRef,    ///< [IN] instance reference
    bool* isDeviceManagement        ///< [INOUT] Session type (false: bootstrap,
                                    ///< true: device management)
)
{
    bool result = false;
    smanager_ClientData_t* dataPtr = (smanager_ClientData_t*) instanceRef;

    if ((NULL == instanceRef) || (NULL == isDeviceManagement))
    {
        return result;
    }


    if ((dataPtr->lwm2mHPtr->state) >= STATE_REGISTER_REQUIRED )
    {
        *isDeviceManagement = true;
    }
    else
    {
        *isDeviceManagement = false;
    }
    result = true;
    LOG_ARG("state %d --> isDeviceManagement %d", dataPtr->lwm2mHPtr->state, *isDeviceManagement);

    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to set the push callback handler
 */
//--------------------------------------------------------------------------------------------------
void lwm2mcore_SetPushCallback
(
    lwm2mcore_PushAckCallback_t callbackP  ///< [IN] push callback pointer
)
{
    PushCb = callbackP;
    lwm2m_set_push_callback(PushCallbackHandler);
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to push data to lwm2mCore
 *
 * @return
 *      - LWM2MCORE_PUSH_INITIATED if data push transaction is initiated
 *      - LWM2MCORE_PUSH_FAILED if data push transaction failed
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_PushResult_t lwm2mcore_Push
(
    lwm2mcore_Ref_t instanceRef,            ///< [IN] instance reference
    uint8_t* payloadPtr,                    ///< [IN] payload
    size_t payloadLength,                   ///< [IN] payload length
    lwm2mcore_PushContent_t content,        ///< [IN] content type
    uint16_t* midPtr                        ///< [OUT] message id
)
{
    int rc;
    lwm2mcore_PushResult_t result = LWM2MCORE_PUSH_FAILED;
    lwm2m_media_type_t contentType;
    bool registered = false;
    smanager_ClientData_t* dataPtr = (smanager_ClientData_t*) instanceRef;

    if (NULL == instanceRef)
    {
        return result;
    }

    switch (content)
    {
        case LWM2MCORE_PUSH_CONTENT_CBOR:
            contentType = LWM2M_CONTENT_CBOR;
            break;
        case LWM2MCORE_PUSH_CONTENT_ZCBOR:
            contentType = LWM2M_CONTENT_ZCBOR;
            break;
        default:
            LOG_ARG("Invalid content type %d", content);
            return LWM2MCORE_PUSH_FAILED;
    }

    /* Check that the device is registered to DM server */
    if ((true == lwm2mcore_ConnectionGetType(instanceRef, &registered) && registered))
    {
        /* Retrieve the serverID from list */
        lwm2m_server_t * targetPtr = dataPtr->lwm2mHPtr->serverList;
        if (NULL == targetPtr)
        {
            LOG("serverList is NULL");
        }
        else
        {
            LOG_ARG("shortServerId %d", targetPtr->shortID);
            rc = lwm2m_data_push(dataPtr->lwm2mHPtr,
                                 targetPtr->shortID,
                                 payloadPtr,
                                 payloadLength,
                                 contentType,
                                 midPtr);

            if (rc == COAP_NO_ERROR)
            {
                result = LWM2MCORE_PUSH_INITIATED;
            }
            else if (rc == COAP_412_PRECONDITION_FAILED)
            {
                result = LWM2MCORE_PUSH_BUSY;
            }
            else
            {
                result = LWM2MCORE_PUSH_FAILED;
            }
        }
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to send an asynchronous response to server.
 *
 * @return
 *      - true if an asynchronous response is initiated
 *      - else false
 */
//--------------------------------------------------------------------------------------------------
bool lwm2mcore_SendAsyncResponse
(
    lwm2mcore_Ref_t instanceRef,                ///< [IN] instance reference
    lwm2mcore_CoapRequest_t* requestPtr,        ///< [IN] CoAP request refernce
    lwm2mcore_CoapResponse_t* responsePtr       ///< [IN] CoAP response
)
{
    bool registered = false;
    smanager_ClientData_t* dataPtr = (smanager_ClientData_t*) instanceRef;

    if (NULL == instanceRef)
    {
        return false;
    }

    /* Check that the device is registered to DM server */
    if ((true == lwm2mcore_ConnectionGetType(instanceRef, &registered) && registered))
    {
        /* Retrieve the serverID from list */
        lwm2m_server_t* targetPtr = dataPtr->lwm2mHPtr->serverList;
        if (NULL == targetPtr)
        {
            LOG("serverList is NULL");
            return false;
        }
        else
        {
            return lwm2m_async_response(dataPtr->lwm2mHPtr,
                                        targetPtr->shortID,
                                        requestPtr->messageId,
                                        ConvertToCoapCode(responsePtr->code),
                                        responsePtr->token,
                                        responsePtr->tokenLength,
                                        responsePtr->contentType,
                                        responsePtr->payload,
                                        responsePtr->payloadLength);
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to check if the client is connected to a bootstrap server
 *
 * @return
 *      - true if the client is connected to a bootstrap server
 *      - else false
 */
//--------------------------------------------------------------------------------------------------
bool smanager_IsBootstrapConnection
(
    void
)
{
    if (BootstrapSession)
    {
        return true;
    }

    return false;
}
