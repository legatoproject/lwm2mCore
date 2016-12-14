/**
 * @file lwm2mcoreSessionParam.h
 *
 * Session manager header file
 *
 *
 * Copyright (C) Sierra Wireless Inc. Use of this work is subject to license.
 *
 */

#ifndef LWM2MCORE_SESSIONPARAM_H
#define LWM2MCORE_SESSIONPARAM_H

#include "lwm2mcore.h"
#include "../inc/lwm2mcoreObjectHandler.h"
#include "../objectManager/lwm2mcoreObjects.h"
#include "../sessionManager/dtlsconnection.h"


//--------------------------------------------------------------------------------------------------
/**
 * Structure for LWM2M core context
 */
//--------------------------------------------------------------------------------------------------
typedef struct _lwm2mcore_context
{
    struct _lwm2mcore_objects_list objects_list;    ///< list of supported objects
} lwm2mcore_context_t;


//--------------------------------------------------------------------------------------------------
/**
 * Structure to be used by the client
 */
//--------------------------------------------------------------------------------------------------
typedef struct _client_data
{
    lwm2m_object_t * securityObjP;      ///< Security object list
    lwm2m_object_t * serverObject;      ///< Server object list
    int sock;                           ///< Socket Id
    dtls_connection_t * connList;       ///< DTLS connection list
    lwm2m_context_t * lwm2mH;           ///< Wakaama LWM2M context
    int addressFamily;                  ///< Socket family address
    lwm2mcore_context_t * lwm2mcoreCtx; ///< LWM2M Core context
} client_data_t;

//--------------------------------------------------------------------------------------------------
/**
 * Event status
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    LWM2MCORE_EVENT_STATUS_STARTED,         ///< Event started
    LWM2MCORE_EVENT_STATUS_DONE_SUCCESS,    ///< Event stopped successfully
    LWM2MCORE_EVENT_STATUS_DONE_FAIL,       ///< Event stopped with failure
    LWM2MCORE_EVENT_STATUS_MAX = 0xFF,      ///< Internal usage
}lwm2mcore_sessionEventStatus_t;

//--------------------------------------------------------------------------------------------------
/**
 * Event types
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    LWM2MCORE_EVENT_TYPE_BOOTSTRAP,         ///< Bootstrap event: started, succeeded or failed
    LWM2MCORE_EVENT_TYPE_REGISTRATION,      ///< Registration event: started, succeeded or failed
    LWM2MCORE_EVENT_TYPE_REG_UPDATE,        ///< Registration update event: started, succeeded or
                                            ///< failed
    LWM2MCORE_EVENT_TYPE_DEREG,             ///< Deregistration event: started, succeeded or failed
    LWM2MCORE_EVENT_TYPE_AUTHENTICATION,    ///< Authentication event: started, succeeded or failed
    LWM2MCORE_EVENT_TYPE_RESUMING,          ///< DTLS resuming/re-authentication event: started,
                                            ///< succeedd or failed
    LWM2MCORE_EVENT_SESSION,                ///< Session event: started or done with success or
                                            ///< failure
    LWM2MCORE_EVENT_TYPE_MAX = 0xFF,        ///< Internal usage
}lwm2mcore_sessionEventType_t;

//--------------------------------------------------------------------------------------------------
/**
 * Function for session events
 */
//--------------------------------------------------------------------------------------------------
void lwm2mcore_sessionEvent
(
    lwm2mcore_sessionEventType_t eventId,   ///< [IN] Event Id
    lwm2mcore_sessionEventStatus_t status   ///< [IN] Event status
);

#endif /* LWM2MCORE_SESSIONPARAM_H */

