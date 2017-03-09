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
#include "liblwm2m.h"
#include "lwm2mcore.h"
#include "internals.h"
#include "objects.h"
#include "osDebug.h"
#include "osTimer.h"
#include "osUdp.h"
#include "dtlsConnection.h"
#include "sessionManager.h"
#include "osPortSecurity.h"

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
static  os_socketConfig_t SocketConfig;

//--------------------------------------------------------------------------------------------------
/**
 *  Context
 */
//--------------------------------------------------------------------------------------------------
static ClientData_t* DataCtxPtr;

//--------------------------------------------------------------------------------------------------
/**
 *  Callback for events
 */
//--------------------------------------------------------------------------------------------------
static lwm2mcore_statusCb_t StatusCb = NULL;

//--------------------------------------------------------------------------------------------------
/**
 *  Static boolean for bootstrap notification
 */
//--------------------------------------------------------------------------------------------------
static bool BootstrapSession = false;

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
/** initialize the lwm2m context object
 *
 * @return
 *  - pointer to lwm2m context object
 */
//--------------------------------------------------------------------------------------------------
static lwm2mcore_context_t* InitContext
(
    ClientData_t* dataPtr,         ///< [IN] Context
    lwm2m_endpoint_type_t epType   ///< [IN] Lwm2m endpoint type, e.g. server/client.
)
{
    dataPtr->lwm2mcoreCtxPtr = (lwm2mcore_context_t*)malloc(sizeof(lwm2mcore_context_t));
    OS_ASSERT(dataPtr->lwm2mcoreCtxPtr);
    memset(dataPtr->lwm2mcoreCtxPtr, 0, sizeof(lwm2mcore_context_t));
    return dataPtr->lwm2mcoreCtxPtr;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function called by the LWM2M core to initiate a connection to a server
 *
 * @return
 *  - dtls_connection_t structure adress on success
 *  - NULL on failure
 */
//--------------------------------------------------------------------------------------------------
void* lwm2m_connect_server
(
    uint16_t secObjInstID,      ///< [IN] Security object instance ID
    void * userDataPtr          ///< [IN] User data
)
{
    ClientData_t* dataPtr;
    lwm2m_list_t* instancePtr;
    dtls_connection_t* newConnPtr = NULL;
    dataPtr = (ClientData_t*)userDataPtr;

    if (NULL != dataPtr)
    {
        lwm2m_object_t  * securityObj = dataPtr->securityObjPtr;

        instancePtr = LWM2M_LIST_FIND(dataPtr->securityObjPtr->instanceList, secObjInstID);
        if (NULL == instancePtr)
        {
            return NULL;
        }

        newConnPtr = connection_create(dataPtr->connListPtr,
                                       dataPtr->sock,
                                       securityObj,
                                       instancePtr->id,
                                       dataPtr->lwm2mHPtr,
                                       dataPtr->addressFamily);
        if (NULL == newConnPtr)
        {
            LOG("Connection creation failed");
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
    ClientData_t* appDataPtr;
    dtls_connection_t* targetPtr;

    appDataPtr = (ClientData_t*)userDataPtr;
    targetPtr = (dtls_connection_t*)sessionHPtr;

    if ((NULL != appDataPtr) && (NULL != targetPtr))
    {
        if (targetPtr == appDataPtr->connListPtr)
        {
            appDataPtr->connListPtr = targetPtr->nextPtr;
            lwm2m_free(targetPtr);
        }
        else
        {
            dtls_connection_t* parentPtr;
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
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Update bootstrap state and backup security object if bootstrap succeeded.
 */
//--------------------------------------------------------------------------------------------------
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

//--------------------------------------------------------------------------------------------------
/**
 *  lwm2m client step that handles data transmit.
 */
//--------------------------------------------------------------------------------------------------
static void Lwm2mClientStepHandler
(
    void* timerRef    ///< Timer that expired
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

    /* Maunch timer step */
    if (false == os_timerSet(OS_TIMER_STEP, tv.tv_sec, NULL))
    {
        LOG("ERROR to launch the step timer");
    }

    UpdateBootstrapInfo(&PreviousState, DataCtxPtr->lwm2mHPtr);

    LOG("lwm2m step completed.");
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to send status event to the application, using the callback stored in the LWM2M core
 * session manager
 */
//--------------------------------------------------------------------------------------------------
void SendStatusEvent
(
    lwm2mcore_status_t status
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
void SendSessionEvent
(
    SessionEventType_t eventId,         ///< [IN] Event Id
    SessionEventStatus_t eventstatus    ///< [IN] Event status
)
{
    lwm2mcore_status_t status;

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
                    StoreCredentials();
                }
                break;

                case EVENT_STATUS_DONE_FAIL:
                {
                    LOG("BOOTSTRAP FAILURE");
                    status.event = LWM2MCORE_EVENT_SESSION_FAILED;
                    SendStatusEvent(status);
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
                    SendStatusEvent(status);

                    status.event = LWM2MCORE_EVENT_LWM2M_SESSION_TYPE_START;
                    status.u.session.type = LWM2MCORE_SESSION_DEVICE_MANAGEMENT;
                    SendStatusEvent(status);
                }
                break;

                case EVENT_STATUS_DONE_FAIL:
                {
                    LOG("REGISTER FAILURE");
                    status.event = LWM2MCORE_EVENT_SESSION_FAILED;
                    SendStatusEvent(status);

                    /* Delete DM credentials in order to forwe a connection to the BS server */
                    LOG("DELETE DM CREDENTIALS");
                    os_portSecurityDeleteCredential(LWM2MCORE_CREDENTIAL_DM_PUBLIC_KEY);
                    os_portSecurityDeleteCredential(LWM2MCORE_CREDENTIAL_DM_SERVER_PUBLIC_KEY);
                    os_portSecurityDeleteCredential(LWM2MCORE_CREDENTIAL_DM_SECRET_KEY);
                    os_portSecurityDeleteCredential(LWM2MCORE_CREDENTIAL_DM_ADDRESS);
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
                    SendStatusEvent(status);
                }
                break;

                case EVENT_STATUS_DONE_SUCCESS:
                {
                    LOG("AUTHENTICATION DONE");

                    if (BootstrapSession)
                    {
                        status.event = LWM2MCORE_EVENT_SESSION_STARTED;
                        SendStatusEvent(status);

                        status.event = LWM2MCORE_EVENT_LWM2M_SESSION_TYPE_START;
                        status.u.session.type = LWM2MCORE_SESSION_BOOTSTRAP;
                        SendStatusEvent(status);
                        BootstrapSession = false;
                    }
                }
                break;

                case EVENT_STATUS_DONE_FAIL:
                {
                    LOG("AUTHENTICATION FAILURE");
                    status.event = LWM2MCORE_EVENT_AUTHENTICATION_FAILED;
                    SendStatusEvent(status);
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
                    SendStatusEvent(status);
                }
                break;

                case EVENT_STATUS_DONE_FAIL:
                {
                    LOG("SESSION FAILURE");
                    BootstrapSession = false;
                    status.event = LWM2MCORE_EVENT_SESSION_FAILED;
                    SendStatusEvent(status);
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
void os_udpReceiveCb
(
    uint8_t* bufferPtr,                 ///< [IN] Received data
    uint32_t len,                       ///< [IN] Received data length
    struct sockaddr_storage *addrPtr,   ///< [INOUT] source address
    socklen_t addrLen,                  ///< @TODO
    os_socketConfig_t config            ///< [IN] Socket config
)
{
    ClientData_t* dataPtr = (ClientData_t*)config.context;
    dtls_connection_t* connPtr;
    LOG("avc_udpCb");

    dataPtr->sock = config.sock;
    dataPtr->addressFamily = config.af;

    connPtr = connection_find(dataPtr->connListPtr, addrPtr, addrLen);
    if (NULL != connPtr)
    {
        // Let liblwm2m respond to the query depending on the context
        int result;
        LOG("Handle packet");
        result = connection_handlePacket(connPtr, bufferPtr, (size_t)len);
        if (0 != result)
        {
             LOG_ARG("Error handling message %d.",result);
        }
    }
    else
    {
        LOG("Received bytes ignored.");
    }
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
 *  - context address
 *  - -1 in case of error
 */
//--------------------------------------------------------------------------------------------------
int lwm2mcore_init
(
    lwm2mcore_statusCb_t eventCb    ///< [IN] event callback
)
{
    int result = -1;
    if (NULL != eventCb)
    {
        ClientData_t* dataPtr = NULL;
        StatusCb = eventCb;

        dataPtr = (ClientData_t*)lwm2m_malloc(sizeof (ClientData_t));
        OS_ASSERT(dataPtr);
        memset(dataPtr, 0, sizeof (ClientData_t));

         /* Initialize LWM2M agent */
        dataPtr->lwm2mHPtr = lwm2m_init(dataPtr);
        OS_ASSERT(dataPtr->lwm2mHPtr);

        dataPtr->lwm2mcoreCtxPtr = InitContext(dataPtr, ENDPOINT_CLIENT);
        Lwm2mcoreCtxPtr = dataPtr->lwm2mcoreCtxPtr;
        OS_ASSERT(dataPtr->lwm2mcoreCtxPtr);

        result = (int)dataPtr;
        DataCtxPtr = dataPtr;
    }
    LOG_ARG("lwm2mcore_init -> context %d", result);
    return (int)result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Free the LWM2M core
 *
 */
//--------------------------------------------------------------------------------------------------
void lwm2mcore_free
(
    int context     ///< [IN] context
)
{
    ClientData_t* dataPtr = (ClientData_t*)context;

    if (NULL != dataPtr)
    {
        /* Free objects */
        ObjectsFree();

        if (NULL != dataPtr->lwm2mcoreCtxPtr)
        {
            lwm2m_free(dataPtr->lwm2mcoreCtxPtr);
        }

        if (NULL != dataPtr)
        {
            lwm2m_free(dataPtr);
            LOG("free dataPtr");
        }
        else
        {
            LOG("dataPtr NOT free");
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
bool lwm2mcore_connect
(
    int context                             ///< [IN] LWM2MCore context
)
{
    bool result = false;
    ClientData_t* dataPtr = (ClientData_t*)context;

    if (dataPtr != NULL)
    {
        /* Create the socket */
        memset(&SocketConfig, 0, sizeof (os_socketConfig_t));
        result = os_udpOpen(context, os_udpReceiveCb, &SocketConfig);

        dataPtr->sock = SocketConfig.sock;
        dataPtr->addressFamily = SocketConfig.af;

        if (true == result)
        {
            /* Initialize the lwm2m client step timer */
            DataCtxPtr = dataPtr;
            if (false == os_timerSet(OS_TIMER_STEP, 1, Lwm2mClientStepHandler))
            {
                LOG("ERROR to launch the 1st step timer");
            }
            else
            {
                LOG("LWM2M Client started");
                result = true;
            }
        }
        else
        {
            LOG("ERROR on socket create");
        }
    }
    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to send an update message to the Device Management server
 * This API can be used when the application wants to send a notification or during a firmware/app
 * update in order to be able to fully treat the scheduled update job
 *
 * @return
 *      - true if the treatment is launched
 *      - else false
 */
//--------------------------------------------------------------------------------------------------
bool lwm2mcore_update
(
    int context     ///< [IN] context
)
{
    bool result = false;
    ClientData_t* dataPtr = (ClientData_t*) context;

    if (NULL != dataPtr)
    {
        /* Check that the device is registered to DM server */
        bool registered = false;
        if ((true == lwm2mcore_connectionGetType(context, &registered) && registered))
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

                int iresult = lwm2m_update_registration(dataPtr->lwm2mHPtr, targetPtr->shortID, false);
                LOG_ARG("lwm2m_update_registration return %d", iresult);
                if (!iresult)
                {
                    /* Stop the timer and launch it */
                    if (false == os_timerStop(OS_TIMER_STEP) )
                    {
                        LOG("Error to stop the step timer");
                    }

                    /* Launch the OS_TIMER_STEP timer with 1 second to treat the update request */
                    if (false == os_timerSet(OS_TIMER_STEP, 1, Lwm2mClientStepHandler))
                    {
                        LOG("ERROR to launch the step timer for registration update");
                    }
                    else
                    {
                        result = true;
                    }
                }
            }
        }
        else
        {
            LOG("REG update is requested but the device is not registered");
        }
    }
    return result;
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
bool lwm2mcore_disconnect
(
    int context     ///< [IN] context
)
{
    bool result = false;
    ClientData_t* dataPtr = (ClientData_t*) context;

    if (dataPtr != NULL)
    {
        /* Stop the current timers */
        if (false == os_timerStop (OS_TIMER_STEP))
        {
            LOG("Error to stop the step timer");
        }

        /* Stop the agent */
        lwm2m_close(dataPtr->lwm2mHPtr);
        connection_free(dataPtr->connListPtr);
        dataPtr->lwm2mHPtr = NULL;
        dataPtr->connListPtr = NULL;

        /* Close the socket */
        result = os_udpClose(SocketConfig);
        if (false == result)
        {
            LOG("ERROR in socket closure");
        }
        else
        {
            memset(&SocketConfig, 0, sizeof (os_socketConfig_t));
            /* Notify that the connection is stopped */
            SendSessionEvent(EVENT_SESSION, EVENT_STATUS_DONE_SUCCESS);
        }
    }
    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to know what is the current connection
 *
 * @return
 *      - true if the device is connected to any DM server
 *      - else false
 */
//--------------------------------------------------------------------------------------------------
bool lwm2mcore_connectionGetType
(
    int context,                ///< [IN] context
    bool* isDeviceManagement    ///< [INOUT] Session type (false: bootstrap,
                                ///< true: device management)
)
{
    bool result = true;
    ClientData_t* dataPtr = (ClientData_t*) context;

    if ((NULL != dataPtr) && (NULL != isDeviceManagement))
    {
        if ((dataPtr->lwm2mHPtr->state) >= STATE_REGISTER_REQUIRED )
        {
            *isDeviceManagement = true;
        }
        else
        {
            *isDeviceManagement = false;
        }

        LOG_ARG("state %d --> isDeviceManagement %d",
                dataPtr->lwm2mHPtr->state, *isDeviceManagement);
    }
    return result;
}


//--------------------------------------------------------------------------------------------------
/**
 * Function to push data to lwm2mCore
 *
 * @return
 *      - true if a data push transaction is initiated
 *      - else false
 */
//--------------------------------------------------------------------------------------------------
bool lwm2mcore_push
(
    int context,                ///< [IN] context
    uint8_t* payloadPtr,        ///< [IN] payload
    size_t payloadLength,       ///< [IN] payload length
    void* callbackPtr           ///< [IN] callback for payload
)
{
    int rc;
    bool result = false;
    ClientData_t* dataPtr = (ClientData_t*) context;

    if (NULL != dataPtr)
    {
        /* Check that the device is registered to DM server */
        bool registered = false;
        if ((true == lwm2mcore_connectionGetType(context, &registered) && registered))
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
                                    callbackPtr);

                if (rc == COAP_NO_ERROR)
                {
                    result = true;
                }
            }
        }
    }

    return result;
}

