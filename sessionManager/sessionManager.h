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
#include <lwm2mcore/coapHandlers.h>
#include "objects.h"
#include "dtlsConnection.h"

/**
  * @addtogroup lwm2mcore_sessionManager_int
  * @{
  */

//--------------------------------------------------------------------------------------------------
/**
 * @brief Structure for LWM2M core context
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    struct _lwm2mcore_objectsList objects_list;     ///< list of supported objects
}lwm2mcore_context_t;


//--------------------------------------------------------------------------------------------------
/**
 * @brief Structure to be used by the client
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    lwm2m_object_t* securityObjPtr;         ///< Security object list
    lwm2m_object_t* serverObjectPtr;        ///< Server object list
    int sock;                               ///< Socket Id
    dtls_Connection_t* connListPtr;         ///< DTLS connection list
    lwm2m_context_t* lwm2mHPtr;             ///< Wakaama LWM2M context
    int addressFamily;                      ///< Socket family address
    lwm2mcore_context_t* lwm2mcoreCtxPtr;   ///< LWM2M Core context
    uint16_t serverId;                      ///< Server ID (MAX_UINT16 for all servers)
    bool isEdmEnabled;                      ///< Flag specifying whether Extended Device
                                            ///<   Management (EDM) is enabled
}smanager_ClientData_t;

//--------------------------------------------------------------------------------------------------
/**
 * @brief Event status
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    EVENT_STATUS_STARTED,       ///< Event started
    EVENT_STATUS_DONE_SUCCESS,  ///< Event stopped successfully
    EVENT_STATUS_DONE_FAIL,     ///< Event stopped with failure
    EVENT_STATUS_INACTIVE,      ///< Event inactive
    EVENT_STATUS_FINISHING,     ///< Event finishing
    EVENT_STATUS_MAX = 0xFF,    ///< Internal usage
}smanager_EventStatus_t;

//--------------------------------------------------------------------------------------------------
/**
 * @brief Event types
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
}smanager_EventType_t;

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to send status event to the application, using the callback stored in the
 * Lwm2MCore session manager
 */
//--------------------------------------------------------------------------------------------------
void smanager_SendStatusEvent
(
    lwm2mcore_Status_t status       ///< [IN] LWM2M status event
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function for session events
 */
//--------------------------------------------------------------------------------------------------
void smanager_SendSessionEvent
(
    smanager_EventType_t    eventId,        ///< [IN] Event Id
    smanager_EventStatus_t  status,         ///< [IN] Event status
    void*                   contextPtr      ///< [IN] Context
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to check if the client is connected to a bootstrap server
 *
 * @return
 *  - @c true if the client is connected to a bootstrap server
 *  - @c else false
 */
//--------------------------------------------------------------------------------------------------
bool smanager_IsBootstrapConnection
(
    void
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to schedule a registration update message to all servers
 *
 */
//--------------------------------------------------------------------------------------------------
void smanager_SendUpdateAllServers
(
    uint8_t regUpdateOptions        ///< [IN] bitfield of requested parameters to be added in the
                                    ///< registration update message
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to force a DTLS handshake
 *
 */
//--------------------------------------------------------------------------------------------------
void smanager_ForceDtlsHandshake
(
    void
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to force a bootstrap
 *
 */
//--------------------------------------------------------------------------------------------------
void smanager_ForceBootstrap
(
    bool    removeTransaction       ///< [IN] Indicates if transactions need be removed
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to force a registration
 *
 */
//--------------------------------------------------------------------------------------------------
void smanager_Registration
(
    void
);
/**
  * @}
  */

#endif /* __SESSION_H__ */
