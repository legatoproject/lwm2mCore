/*******************************************************************************
 *
 * Copyright (c) 2015 Intel Corporation and others.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * The Eclipse Distribution License is available at
 *    http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Simon Bernard - initial API and implementation
 *    Christian Renz - Please refer to git log
 *
 *******************************************************************************/

 /**
 * @file dtlsConnection.h
 *
 * Header file for DTLS connection
 *
 */

#ifndef DTLS_CONNECTION_H_
#define DTLS_CONNECTION_H_

#include <unistd.h>
#include <sys/stat.h>
#include <stdint.h>
#include <platform/types.h>
#include <platform/inet.h>
#include "tinydtls.h"
#include "dtls.h"
#include "liblwm2m.h"

/**
  * @addtogroup lwm2mcore_dtlsconnection_int
  * @{
  */
//--------------------------------------------------------------------------------------------------
/**
 * @brief Define value for the DTLS rehandshake: after 40 seconds of inactivity, a rehandshake is
 * needed in order to send any data to the server
 */
//--------------------------------------------------------------------------------------------------
#define DTLS_NAT_TIMEOUT 40

//--------------------------------------------------------------------------------------------------
/**
 * @brief Structure for DTLS connection
 */
//--------------------------------------------------------------------------------------------------
typedef struct _dtls_Connection_t
{
    struct _dtls_Connection_t*  nextPtr;        ///< Next entry in the list
    int                         sock;           ///< Socket Id used for the DTLS connection
    struct sockaddr_in6         addr;           ///< Socket addess structure
    size_t                      addrLen;        ///< Socket addess structure length
    session_t*                  dtlsSessionPtr; ///< DTLS session
    lwm2m_object_t*             securityObjPtr; ///< LWM2M Security object
    int                         securityInstId; ///< LWM2M Security object instance Id
    lwm2m_context_t*            lwm2mHPtr;      ///< Session handler
    dtls_context_t*             dtlsContextPtr; ///< DTLS context
    time_t                      lastSend;       ///< Last time a data was sent to the server (used for NAT timeouts)
    void (*postRequestHandler)(struct _dtls_Connection_t*); ///< post-Request session handler to invoke
                                                ///< if present
}dtls_Connection_t;

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to search if a DTLS connection is available
 *
 * @return
 *  - @c dtls_Connection_t pointer if the DTLS connection is available
 *  - @c NULL if the DTLS connection is not available in the indicated socket
 */
//--------------------------------------------------------------------------------------------------
dtls_Connection_t* dtls_FindConnection
(
    dtls_Connection_t* connListPtr,         ///< [IN] DTLS connection list
    const struct sockaddr_storage* addrPtr, ///< [IN] Socket address structure
    size_t addrLen                          ///< [IN] Socket address structure length
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to create a new DTLS connection
 *
 * @return
 *  - DTLS connection structure (@c dtls_Connection_t);
 *  - @c NULL in case of failure
 */
//--------------------------------------------------------------------------------------------------
dtls_Connection_t* dtls_HandleNewIncoming
(
    dtls_Connection_t* connListPtr,     ///< [IN] DTLS connection list
    int sock,                           ///< [IN] Socket Id on which the DTLS needs to be created
    const struct sockaddr* addrPtr,     ///< [IN] Socket address structure
    size_t addrLen                      ///< [IN] Socket address structure length
);

//--------------------------------------------------------------------------------------------------
/**
 * Function to create a new connection to the server
 *
 * @return
 *  - DTLS connection pointer (@c dtls_Connection_t)
 *  - @c NULL in case of failure
 */
//--------------------------------------------------------------------------------------------------
dtls_Connection_t* dtls_CreateConnection
(
    dtls_Connection_t* connListPtr,     ///< [IN] DTLS connection structure
    int sock,                           ///< [IN] Socket Id
    lwm2m_object_t* securityObjPtr,     ///< [IN] Security object pointer
    int instanceId,                     ///< [IN] Security object instance Id
    lwm2m_context_t* lwm2mHPtr,         ///< [IN] Session handle
    int addressFamily                   ///< [IN] Address familly
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to free the DTLS connection list
 */
//--------------------------------------------------------------------------------------------------
void dtls_FreeConnection
(
    dtls_Connection_t* connListPtr      ///< [IN] DTLS connection structure
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to update DTLS connection list
 */
//--------------------------------------------------------------------------------------------------
void dtls_UpdateDtlsList
(
    dtls_Connection_t* connListPtr      ///< [IN] DTLS connection structure
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to send data in a specific connection.
 * This function checks if DTLS is activated on the connection.
 *
 * @return
 *  - @c 0 in case of success
 *  - @c -1 in case of failure
 */
//--------------------------------------------------------------------------------------------------
int dtls_Send
(
    dtls_Connection_t* connPtr,         ///< [IN] DTLS connection structure
    uint8_t* bufferPtr,                 ///< [IN] Buffer to be sent
    size_t length                       ///< [IN] Buffer length
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to handle incoming data in a specific connection
 * This function checks if DTLS is activated on the connection.
 *
 * @return
 *  - @c 0 in case of success
 *  - negative value in case of failure (see @c dtls_alert_t)
 */
//--------------------------------------------------------------------------------------------------
int dtls_HandlePacket
(
    dtls_Connection_t* connPtr,         ///< [IN] DTLS connection structure
    uint8_t* bufferPtr,                 ///< [IN] Received buffer
    size_t numBytes                     ///< [IN] Buffer length
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to initiate a new DTLS handshake
 * Usefull when NAT timeout happens and client have a new IP/PORT
 *
 * @return
 *  - @c 0 in case of success (or DTLS is not activated on the connection)
 *  - @c -1 in case of failure
 */
//--------------------------------------------------------------------------------------------------
int dtls_Rehandshake
(
    dtls_Connection_t* connPtr,         ///< [IN] DTLS connection structure
    bool sendCloseNotify                ///< [IN] Flag to send a DTLS_STATE_CLOSED
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to resume a DTLS session
 *
 * @return
 *  - @c 0 in case of success (or DTLS is not activated on the connection)
 *  - @c -1 in case of failure
 */
//--------------------------------------------------------------------------------------------------
int dtls_Resume
(
    dtls_Connection_t* connPtr          ///< [IN] DTLS connection structure
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to manage DTLS handshake retransmission
 */
//--------------------------------------------------------------------------------------------------
void dtls_HandshakeRetransmission
(
    dtls_Connection_t*  connListPtr,        ///< [IN] DTLS connection list
    dtls_tick_t*        timerValue,         ///< [INOUT] Timer value for retransmission
    bool*               isMaxReached        ///< [INOUT] Is maximum retransmission reached ?
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to close and free peer
 */
//--------------------------------------------------------------------------------------------------
void dtls_CloseAndFreePeer
(
    dtls_Connection_t* targetPtr            ///< [IN] DTLS connection
);

/**
  * @}
  */


#endif
