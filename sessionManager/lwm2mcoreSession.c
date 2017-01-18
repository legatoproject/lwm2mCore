/**
 * @file lwm2mcoreSession.c
 *
 * LWM2M core file for session management
 *
 *
 * Copyright (C) Sierra Wireless Inc. Use of this work is subject to license.
 *
 */

/* include files */
#include "liblwm2m.h"
#include "lwm2mcore.h"
#include "internals.h"
#include "../objectManager/lwm2mcoreObjects.h"
#include "../os/osDebug.h"
#include "../os/osTimer.h"
#include "../os/osUdp.h"
#include "../sessionManager/dtlsconnection.h"
#include "../sessionManager/lwm2mcoreSessionParam.h"


//--------------------------------------------------------------------------------------------------
/**
 *  Context
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_context_t *lwm2mcore_ctx = NULL;

//--------------------------------------------------------------------------------------------------
/**
 * Socket configuration variable
 */
//--------------------------------------------------------------------------------------------------
os_socketConfig_t SocketConfig;

//--------------------------------------------------------------------------------------------------
/**
 *  Context
 */
//--------------------------------------------------------------------------------------------------
static client_data_t* DataCtx;
//--------------------------------------------------------------------------------------------------
/**
 *  Callback for events
 */
//--------------------------------------------------------------------------------------------------
static lwm2mcore_statucCb_t lwm2mcore_statucCb = NULL;

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
static lwm2mcore_context_t *initContext
(
    client_data_t* dataPtr,         ///< [IN] Context
    lwm2m_endpoint_type_t ep_type   ///< [IN] Lwm2m endpoint type, e.g. server/client.
)
{
    dataPtr->lwm2mcoreCtx = (lwm2mcore_context_t *)malloc(sizeof(lwm2mcore_context_t));
    OS_ASSERT(dataPtr->lwm2mcoreCtx);
    memset(dataPtr->lwm2mcoreCtx, 0, sizeof(lwm2mcore_context_t));
    return dataPtr->lwm2mcoreCtx;
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
    client_data_t * dataPtr;
    lwm2m_list_t * instance;
    dtls_connection_t * newConnPtr = NULL;
    dataPtr = (client_data_t *)userDataPtr;

    if (dataPtr != NULL)
    {
        lwm2m_object_t  * securityObj = dataPtr->securityObjP;

        instance = LWM2M_LIST_FIND(dataPtr->securityObjP->instanceList, secObjInstID);
        if (instance == NULL) return NULL;

        newConnPtr = connection_create (dataPtr->connList,
                                        dataPtr->sock,
                                        securityObj,
                                        instance->id,
                                        dataPtr->lwm2mH,
                                        dataPtr->addressFamily);
        if (newConnPtr == NULL)
        {
            fprintf(stderr, "Connection creation failed.\n");
            return NULL;
        }
        dataPtr->connList = newConnPtr;
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
    void * sessionHPtr,     ///< [IN] Connection context
    void * userDataPtr      ///< [IN] Context
)
{
    client_data_t * app_data;
    dtls_connection_t * targetP;

    app_data = (client_data_t *)userDataPtr;
    targetP = (dtls_connection_t *)sessionHPtr;

    if ((app_data != NULL) && (targetP != NULL))
    {
        if (targetP == app_data->connList)
        {
            app_data->connList = targetP->next;
            lwm2m_free(targetP);
        }
        else
        {
            dtls_connection_t * parentP;
            parentP = app_data->connList;
            while (parentP != NULL && parentP->next != targetP)
            {
                parentP = parentP->next;
            }
            if (parentP != NULL)
            {
                parentP->next = targetP->next;
                lwm2m_free(targetP);
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
    lwm2m_client_state_t * previousBsStatePtr,  ///< [IN] Bootstrap state
    lwm2m_context_t * contextPtr                ///< [IN] Context
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
                    LOG ("Backup security object.");
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

    LOG ("Entering");

    /* This function does two things:
     * - first it does the work needed by liblwm2m (eg. (re)sending some packets).
     * - Secondly it adjusts the timeout value (default 60s) depending on the state of the
     *   transaction
     *   (eg. retransmission) and the time between the next operation
     */

    result = lwm2m_step (DataCtx->lwm2mH, &(tv.tv_sec));
    if (result != 0)
    {
       LOG_ARG ("lwm2m_step() failed: 0x%X.", result);
#ifdef LWM2M_BOOTSTRAP
       if(PreviousState == STATE_BOOTSTRAPPING)
       {
#ifdef WITH_LOGS
           LOG ("[BOOTSTRAP] restore security and server objects.");
#endif
           //prv_restore_objects(ClientCtxt.lwm2mH);
           DataCtx->lwm2mH->state = STATE_INITIAL;
       }
#endif
    }

    /* Maunch timer step */
    if (os_timerSet (OS_TIMER_STEP, tv.tv_sec, NULL) == false)
    {
        LOG ("ERROR to launch the step timer");
    }

    UpdateBootstrapInfo(&PreviousState, DataCtx->lwm2mH);

    LOG ("lwm2m step completed.");
}

//--------------------------------------------------------------------------------------------------
/**
 * Function for session events
 */
//--------------------------------------------------------------------------------------------------
void lwm2mcore_sessionEvent
(
    lwm2mcore_sessionEventType_t eventId,       ///< [IN] Event Id
    lwm2mcore_sessionEventStatus_t eventstatus  ///< [IN] Event status
)
{
    if (lwm2mcore_statucCb != NULL)
    {
        lwm2mcore_status_t status;

        switch (eventId)
        {
            case LWM2MCORE_EVENT_TYPE_BOOTSTRAP:
            {
                switch (eventstatus)
                {
                    case LWM2MCORE_EVENT_STATUS_STARTED:
                    {
                        LOG ("BOOTSTRAP START");
                        status.event = LWM2MCORE_EVENT_LWM2M_SESSION_TYPE_START;
                        status.u.sessionType = LWM2MCORE_SESSION_BOOTSTRAP;
                        lwm2mcore_statucCb (status);
                    }
                    break;

                    case LWM2MCORE_EVENT_STATUS_DONE_SUCCESS:
                    {
                        LOG ("BOOTSTRAP DONE");
                        lwm2mcore_StoreCredentials();
                    }
                    break;

                    case LWM2MCORE_EVENT_STATUS_DONE_FAIL:
                    {
                        LOG ("BOOTSTRAP FAILURE");
                        status.event = LWM2MCORE_EVENT_SESSION_FAILED;
                        lwm2mcore_statucCb (status);
                    }
                    break;

                    default:
                    break;
                }
            }
            break;

            case LWM2MCORE_EVENT_TYPE_REGISTRATION:
            {
                switch (eventstatus)
                {
                    case LWM2MCORE_EVENT_STATUS_STARTED:
                    {
                        LOG ("REGISTER START");
                    }
                    break;

                    case LWM2MCORE_EVENT_STATUS_DONE_SUCCESS:
                    {
                        LOG ("REGISTER DONE");
                        status.event = LWM2MCORE_EVENT_LWM2M_SESSION_TYPE_START;
                        status.u.sessionType = LWM2MCORE_SESSION_DEVICE_MANAGEMENT;
                        lwm2mcore_statucCb (status);
                    }
                    break;

                    case LWM2MCORE_EVENT_STATUS_DONE_FAIL:
                    {
                        LOG ("REGISTER FAILURE");
                        status.event = LWM2MCORE_EVENT_SESSION_FAILED;
                        lwm2mcore_statucCb (status);
                    }
                    break;

                    default:
                    break;
                }
            }
            break;

            case LWM2MCORE_EVENT_TYPE_REG_UPDATE:
            {
                switch (eventstatus)
                {
                    case LWM2MCORE_EVENT_STATUS_STARTED:
                    {
                        LOG ("REG UPDATE START");
                    }
                    break;

                    case LWM2MCORE_EVENT_STATUS_DONE_SUCCESS:
                    {
                        LOG ("REG UPDATE DONE");
                    }
                    break;

                    case LWM2MCORE_EVENT_STATUS_DONE_FAIL:
                    {
                        LOG ("REG UPDATE FAILURE");
                    }
                    break;

                    default:
                    break;
                }
            }
            break;

            case LWM2MCORE_EVENT_TYPE_DEREG:
            {
                switch (eventstatus)
                {
                    case LWM2MCORE_EVENT_STATUS_STARTED:
                    {
                        LOG ("DEREGISTER START");
                    }
                    break;

                    case LWM2MCORE_EVENT_STATUS_DONE_SUCCESS:
                    {
                        LOG ("DEREGISTER DONE");
                    }
                    break;

                    case LWM2MCORE_EVENT_STATUS_DONE_FAIL:
                    {
                        LOG ("DEREGISTER FAILURE");
                    }
                    break;

                    default:
                    break;
                }
            }
            break;

            case LWM2MCORE_EVENT_TYPE_AUTHENTICATION:
            {
                switch (eventstatus)
                {
                    case LWM2MCORE_EVENT_STATUS_STARTED:
                    {
                        LOG ("AUTHENTICATION START");
                        status.event = LWM2MCORE_EVENT_AUTHENTICATION_STARTED;
                        lwm2mcore_statucCb (status);
                    }
                    break;

                    case LWM2MCORE_EVENT_STATUS_DONE_SUCCESS:
                    {
                        LOG ("AUTHENTICATION DONE");
                        status.event = LWM2MCORE_EVENT_SESSION_STARTED;
                        lwm2mcore_statucCb (status);
                    }
                    break;

                    case LWM2MCORE_EVENT_STATUS_DONE_FAIL:
                    {
                        LOG ("AUTHENTICATION FAILURE");
                        status.event = LWM2MCORE_EVENT_AUTHENTICATION_FAILED;
                        lwm2mcore_statucCb (status);
                    }
                    break;

                    default:
                    break;
                }
            }
            break;

            case LWM2MCORE_EVENT_TYPE_RESUMING:
            {
                switch (eventstatus)
                {
                    case LWM2MCORE_EVENT_STATUS_STARTED:
                    {
                        LOG ("DTLS RESUME START");
                    }
                    break;

                    case LWM2MCORE_EVENT_STATUS_DONE_SUCCESS:
                    {
                        LOG ("DTLS RESUME DONE");
                    }
                    break;

                    case LWM2MCORE_EVENT_STATUS_DONE_FAIL:
                    {
                        LOG ("DTLS RESUME FAILURE");
                    }
                    break;

                    default:
                    break;
                }
            }
            break;

            case LWM2MCORE_EVENT_SESSION:
            {
                switch (eventstatus)
                {
                    case LWM2MCORE_EVENT_STATUS_STARTED:
                    {
                        LOG ("SESSION START");
                    }
                    break;

                    case LWM2MCORE_EVENT_STATUS_DONE_SUCCESS:
                    {
                        LOG ("SESSION DONE");
                    }
                    break;

                    case LWM2MCORE_EVENT_STATUS_DONE_FAIL:
                    {
                        LOG ("SESSION FAILURE");
                    }
                    break;

                    default:
                    break;
                }
            }
            break;

            default:
            {
                LOG_ARG ("Bad event %d", eventId);
            }
            break;
        }
    }
}

void os_udpReceiveCb
(
    uint8_t* bufferPtr,                 ///< [IN] Received data
    uint32_t len,                       ///< [IN] Received data length
    struct sockaddr_storage *addrPtr,   ///< [INOUT] source address
    socklen_t addrLen,                  ///< @TODO
    os_socketConfig_t config            ///< [IN] Socket config
)
{
    client_data_t* dataPtr = (client_data_t*)config.context;
    dtls_connection_t * connPtr;
    LOG ("avc_udpCb");

    dataPtr->sock = config.sock;
    dataPtr->addressFamily = config.af;

    connPtr = connection_find (dataPtr->connList, addrPtr, addrLen);
    if (connPtr != NULL)
    {
        // Let liblwm2m respond to the query depending on the context
        LOG ("Handle packet");
        int result = connection_handle_packet (connPtr, bufferPtr, (size_t)len);
        if (0 != result)
        {
             LOG_ARG ("Error handling message %d.",result);
        }
    }
    else
    {
        LOG ("Received bytes ignored.");
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
    lwm2mcore_statucCb_t eventCb    ///< [IN] event callback
)
{
    int result = -1;
    if (eventCb != NULL)
    {
        client_data_t* dataPtr = NULL;
        lwm2mcore_statucCb = eventCb;

        dataPtr = (client_data_t*)lwm2m_malloc (sizeof (client_data_t));
        OS_ASSERT (dataPtr);
        memset (dataPtr, 0, sizeof (client_data_t));

         /* Initialize LWM2M agent */
        dataPtr->lwm2mH = lwm2m_init (dataPtr);
        OS_ASSERT (dataPtr->lwm2mH);

        dataPtr->lwm2mcoreCtx = initContext (dataPtr, ENDPOINT_CLIENT);
        lwm2mcore_ctx = dataPtr->lwm2mcoreCtx;
        OS_ASSERT (dataPtr->lwm2mcoreCtx);

        result = (int)dataPtr;
        DataCtx = dataPtr;
    }
    LOG_ARG ("lwm2mcore_init -> context %d", result);
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
    client_data_t* dataPtr = (client_data_t*)context;

    if (dataPtr != NULL)
    {
        /* Free objects */
        connection_free (dataPtr->connList);

        lwm2mcore_objectFree();

        if (dataPtr->lwm2mcoreCtx != NULL)
        {
            lwm2m_free (dataPtr->lwm2mcoreCtx);
        }

        if (dataPtr != NULL)
        {
            lwm2m_free (dataPtr);
            LOG ("free dataPtr");
        }
        else
        {
            LOG ("dataPtr NOT free");
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
    client_data_t* dataPtr = (client_data_t*)context;

    if (dataPtr != NULL)
    {
        /* Create the socket */
        memset (&SocketConfig, 0, sizeof (os_socketConfig_t));
        result = os_udpOpen (context, os_udpReceiveCb, &SocketConfig);

        dataPtr->sock = SocketConfig.sock;
        dataPtr->addressFamily = SocketConfig.af;

        if (result == true)
        {
            /* Initialize the lwm2m client step timer */
            DataCtx = dataPtr;
            if (os_timerSet (OS_TIMER_STEP, 1, Lwm2mClientStepHandler) == false)
            {
                LOG ("ERROR to launch the 1st step timer");
            }
            else
            {
                LOG ("LWM2M Client started");
                result = true;
            }
        }
        else
        {
            LOG ("ERROR on socket create");
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
 * @note This function is NOT YET IMPLEMENTED
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
    /* Not yet supported */
    client_data_t* dataPtr = (client_data_t*)context;

    if (dataPtr != NULL)
    {
    }
    return false;
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
    client_data_t* dataPtr = (client_data_t*) context;

    if (dataPtr != NULL)
    {
        /* Stop the current timers */
        if (os_timerStop (OS_TIMER_STEP) == false)
        {
            LOG ("Error to stop the step timer");
        }

        /* Stop the agent */
        lwm2m_close (dataPtr->lwm2mH);

        /* Close the socket */
        result = os_udpClose (SocketConfig);
        if (result == false)
        {
            LOG ("ERROR in socket closure");
        }
        else
        {
            memset (&SocketConfig, 0, sizeof (os_socketConfig_t));
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
    client_data_t* dataPtr = (client_data_t*) context;

    if ((dataPtr != NULL) && (isDeviceManagement != NULL))
    {
        if ((dataPtr->lwm2mH->state) >= STATE_REGISTER_REQUIRED )
        {
            *isDeviceManagement = true;
        }
        else
        {
            *isDeviceManagement = false;
        }

        LOG_ARG ("state %d --> isDeviceManagement %d",
                dataPtr->lwm2mH->state, *isDeviceManagement);
    }
    return result;
}

