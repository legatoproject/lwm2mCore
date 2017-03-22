/**
 * @file sessionManager.h
 *
 * Session manager header file
 *
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#ifndef __SESSION_H__
#define __SESSION_H__

#include <lwm2mcore/lwm2mcore.h>
#include "objects.h"
#include "dtlsConnection.h"


//--------------------------------------------------------------------------------------------------
/**
 * Structure for LWM2M core context
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    struct _lwm2mcore_objectsList objects_list;     ///< list of supported objects
}lwm2mcore_context_t;


//--------------------------------------------------------------------------------------------------
/**
 * Structure to be used by the client
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    lwm2m_object_t* securityObjPtr;         ///< Security object list
    lwm2m_object_t* serverObjectPtr;        ///< Server object list
    int sock;                               ///< Socket Id
    dtls_connection_t* connListPtr;         ///< DTLS connection list
    lwm2m_context_t* lwm2mHPtr;             ///< Wakaama LWM2M context
    int addressFamily;                      ///< Socket family address
    lwm2mcore_context_t* lwm2mcoreCtxPtr;   ///< LWM2M Core context
}ClientData_t;

//--------------------------------------------------------------------------------------------------
/**
 * Event status
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    EVENT_STATUS_STARTED,       ///< Event started
    EVENT_STATUS_DONE_SUCCESS,  ///< Event stopped successfully
    EVENT_STATUS_DONE_FAIL,     ///< Event stopped with failure
    EVENT_STATUS_MAX = 0xFF,    ///< Internal usage
}SessionEventStatus_t;

//--------------------------------------------------------------------------------------------------
/**
 * Event types
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    EVENT_TYPE_BOOTSTRAP,       ///< Bootstrap event: started, succeeded or failed
    EVENT_TYPE_REGISTRATION,    ///< Registration event: started, succeeded or failed
    EVENT_TYPE_REG_UPDATE,      ///< Registration update event: started, succeeded or failed
    EVENT_TYPE_DEREG,           ///< Deregistration event: started, succeeded or failed
    EVENT_TYPE_AUTHENTICATION,  ///< Authentication event: started, succeeded or failed
    EVENT_TYPE_RESUMING,        ///< DTLS resuming/re-authentication event: started, succeeded or
                                ///< failed
    EVENT_SESSION,              ///< Session event: started or done with success or failure
    EVENT_TYPE_MAX = 0xFF,      ///< Internal usage
}SessionEventType_t;

//--------------------------------------------------------------------------------------------------
/**
 * Function to send status event to the application, using the callback stored in the LWM2M core
 * session manager
 */
//--------------------------------------------------------------------------------------------------
void SendStatusEvent
(
    lwm2mcore_Status_t status       ///< [IN] LWM2M status event
);

//--------------------------------------------------------------------------------------------------
/**
 * Function for session events
 */
//--------------------------------------------------------------------------------------------------
void SendSessionEvent
(
    SessionEventType_t eventId,     ///< [IN] Event Id
    SessionEventStatus_t status     ///< [IN] Event status
);

#endif /* __SESSION_H__ */

