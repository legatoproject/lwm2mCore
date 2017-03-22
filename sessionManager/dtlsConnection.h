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

#ifndef DTLS_CONNECTION_H_
#define DTLS_CONNECTION_H_

#include <unistd.h>
#include <sys/stat.h>
#include <stdint.h>
#include <stdbool.h>
#include "osInet.h"
#include "tinydtls.h"
#include "dtls.h"
#include "liblwm2m.h"

//--------------------------------------------------------------------------------------------------
/**
 * Define value for the DTLS rehandshake: after 40 seconds of inactivity, a rehandshake is needed
 * in order to send any data to the server
 */
//--------------------------------------------------------------------------------------------------
#define DTLS_NAT_TIMEOUT 40

//--------------------------------------------------------------------------------------------------
/**
 * Structure for DTLS connection
 */
//--------------------------------------------------------------------------------------------------
typedef struct _dtls_connection_t
{
    struct _dtls_connection_t*  nextPtr;        ///< Next entry in the list
    int                         sock;           ///< Socket Id used for the DTLS connection
    struct sockaddr_in6         addr;           ///< Socket addess structure
    size_t                      addrLen;        ///< Socket addess structure length
    session_t*                  dtlsSessionPtr; ///< DTLS session
    lwm2m_object_t*             securityObjPtr; ///< LWM2M Security object
    int                         securityInstId; ///< LWM2M Security object instance Id
    lwm2m_context_t*            lwm2mHPtr;      ///< Session handler
    dtls_context_t*             dtlsContextPtr; ///< DTLS context
    time_t                      lastSend;       ///< Last time a data was sent to the server
                                                ///< (used for NAT timeouts)
}dtls_connection_t;

//--------------------------------------------------------------------------------------------------
/**
 * Function to search if a DTLS connection is available
 *
 * @return
 *  - dtls_connection_t pointer if the DTLS connection is available
 *  - NULL if the DTLS connection is not available in the indicated socket
 */
//--------------------------------------------------------------------------------------------------
dtls_connection_t* connection_find
(
    dtls_connection_t* connListPtr,         ///< [IN] DTLS connection list
    const struct sockaddr_storage* addrPtr, ///< [IN] Socket address structure
    size_t addrLen                          ///< [IN] Socket address structure length
);

//--------------------------------------------------------------------------------------------------
/**
 * Function to create a new DTLS connection
 *
 * @return
 *  - DTLS connection structure (dtls_connection_t);
 *  - NULL in case of failure
 */
//--------------------------------------------------------------------------------------------------
dtls_connection_t* connection_newIncoming
(
    dtls_connection_t* connListPtr,     ///< [IN] DTLS connection list
    int sock,                           ///< [IN] Socket Id on which the DTLS needs to be created
    const struct sockaddr* addrPtr,     ///< [IN] Socket address structure
    size_t addrLen                      ///< [IN] Socket address structure length
);

//--------------------------------------------------------------------------------------------------
/**
 * Function to create a new connection to the server
 *
 * @return
 *  - DTLS connection pointer (dtls_connection_t)
 *  - NULL in case of failure
 */
//--------------------------------------------------------------------------------------------------
dtls_connection_t* connection_create
(
    dtls_connection_t* connListPtr,     ///< [IN] DTLS connection structure
    int sock,                           ///< [IN] Socket Id
    lwm2m_object_t* securityObjPtr,     ///< [IN] Security object pointer
    int instanceId,                     ///< [IN] Security object instance Id
    lwm2m_context_t* lwm2mHPtr,         ///< [IN] Session handle
    int addressFamily                   ///< [IN] Address familly
);

//--------------------------------------------------------------------------------------------------
/**
 * Function to free the DTLS connection list
 */
//--------------------------------------------------------------------------------------------------
void connection_free
(
    dtls_connection_t* connListPtr      ///< [IN] DTLS connection structure
);

//--------------------------------------------------------------------------------------------------
/**
 * Function to send data in a specific connection.
 * This function checks if DTLS is activated on the connection.
 *
 * @return
 *  - 0 in case of success
 *  - -1 in case of failure
 */
//--------------------------------------------------------------------------------------------------
int connection_send
(
    dtls_connection_t* connPtr,         ///< [IN] DTLS connection structure
    uint8_t* bufferPtr,                 ///< [IN] Buffer to be sent
    size_t length                       ///< [IN] Buffer length
);

//--------------------------------------------------------------------------------------------------
/**
 * Function to handle incoming data in a specific connection
 * This function checks if DTLS is activated on the connection.
 *
 * @return
 *  - 0 in case of success
 *  - negative value in case of failure (see dtls_alert_t)
 */
//--------------------------------------------------------------------------------------------------
int connection_handlePacket
(
    dtls_connection_t* connPtr,         ///< [IN] DTLS connection structure
    uint8_t* bufferPtr,                 ///< [IN] Received buffer
    size_t numBytes                     ///< [IN] Buffer length
);

//--------------------------------------------------------------------------------------------------
/**
 * Function to initiate a new DTLS handshake
 * Usefull when NAT timeout happens and client have a new IP/PORT
 *
 * @return
 *  - 0 in case of success (or DTLS is not activated on the connection)
 *  - -1 in case of failure
 */
//--------------------------------------------------------------------------------------------------
int connection_rehandshake
(
    dtls_connection_t* connPtr,         ///< [IN] DTLS connection structure
    bool sendCloseNotify                ///< [IN] Flag to send a DTLS_STATE_CLOSED
);

#endif

