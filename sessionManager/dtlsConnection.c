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
 *    David Navarro, Intel Corporation - initial API and implementation
 *    Christian Renz - Please refer to git log
 *
 *******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "lwm2mcore.h"
#include "objects.h"
#include "dtlsConnection.h"
#include "osDebug.h"
#include "sessionManager.h"
#include "internals.h"
#include "liblwm2m.h"

#define COAP_PORT "5683"
#define COAPS_PORT "5684"
#define URI_LENGTH LWM2MCORE_SERVER_URI_MAX_LEN + 1

//--------------------------------------------------------------------------------------------------
/**
 * Global for DTLS context
 */
//--------------------------------------------------------------------------------------------------
dtls_context_t* DtlsContextPtr;

//--------------------------------------------------------------------------------------------------
/**
 * Function to search the server URI (resource 0 of object 0)
 *
 * @return
 *  - pointer on server URI
 *  - NULL if server URI was not found
 */
//--------------------------------------------------------------------------------------------------
static char* SecurityGetUri
(
    lwm2m_object_t* objPtr,     ///< [IN] Pointer on the object structure
    int instanceId,             ///< [IN] Object instance Id
    char* uriBufferPtr,         ///< [INOUT] Data buffer
    int bufferSize              ///< [IN] Buffer size
)
{
    int size = 1;
    lwm2m_data_t* dataPtr = lwm2m_data_new(size);
    uint8_t test;
    dataPtr->id = LWM2M_SECURITY_URI_ID;

    test = objPtr->readFunc(instanceId, &size, &dataPtr, objPtr);
    LOG_ARG("security_get_uri readFunc return %d", test);

    if ((NULL != dataPtr) &&
        (LWM2M_TYPE_STRING == dataPtr->type) &&
        (0 < dataPtr->value.asBuffer.length))
    {
        if(bufferSize > dataPtr->value.asBuffer.length)
        {
            memset(uriBufferPtr, 0, (dataPtr->value.asBuffer.length + 1 ));
            strncpy(uriBufferPtr, dataPtr->value.asBuffer.buffer, dataPtr->value.asBuffer.length);
            lwm2m_data_free(size, dataPtr);
            return uriBufferPtr;
        }
        else
        {
            LOG_ARG("uriBuffer size is too short: bufferSize %d < length %d",
                    bufferSize, dataPtr->value.asBuffer.length);
        }
    }
    lwm2m_data_free(size, dataPtr);
    return NULL;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to get the security mode from security object (resource 2 of object 0)
 *
 * @return
 *  - LWM2M_SECURITY_MODE_PRE_SHARED_KEY if PSKs are used for security
 *  - LWM2M_SECURITY_MODE_RAW_PUBLIC_KEY if raw public keys are used for security
 *  - LWM2M_SECURITY_MODE_CERTIFICATE if certificates are used for security
 *  - LWM2M_SECURITY_MODE_NONE in case of no security
 */
//--------------------------------------------------------------------------------------------------
static int64_t SecurityGetMode
(
    lwm2m_object_t* objPtr,     ///< [IN] Pointer on the object structure
    int instanceId              ///< [IN] Objecxt instance Id
)
{
    int64_t mode;
    int size = 1;
    lwm2m_data_t* dataPtr = lwm2m_data_new(size);
    dataPtr->id = LWM2MCORE_SECURITY_MODE_RID;

    objPtr->readFunc(instanceId, &size, &dataPtr, objPtr);
    if (0 != lwm2m_data_decode_int(dataPtr,&mode))
    {
        lwm2m_data_free(size, dataPtr);
        return mode;
    }

    lwm2m_data_free(size, dataPtr);
    LOG("Unable to get security mode : use not secure mode");
    return LWM2M_SECURITY_MODE_NONE;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to get the public key or identity (resource 3 of object 0)
 *
 * @return
 *  - pointer on public key or identity
 *  - NULL in case of failure
 */
//--------------------------------------------------------------------------------------------------
static char* SecurityGetPublicId
(
    lwm2m_object_t* objPtr,     ///< [IN] Pointer on the object structure
    int instanceId,             ///< [IN] Objecxt instance Id
    int* lengthPtr              ///< [INOUT] Key length
)
{
    int size = 1;
    lwm2m_data_t* dataPtr = lwm2m_data_new(size);
    dataPtr->id = LWM2MCORE_SECURITY_PKID_RID;

    objPtr->readFunc(instanceId, &size, &dataPtr, objPtr);
    if ((NULL != dataPtr)
    && (LWM2M_TYPE_OPAQUE == dataPtr->type))
    {
        char* buffPtr;

        buffPtr = (char*)lwm2m_malloc(dataPtr->value.asBuffer.length);
        if (0 != buffPtr)
        {
            memcpy(buffPtr, dataPtr->value.asBuffer.buffer, dataPtr->value.asBuffer.length);
            *lengthPtr = dataPtr->value.asBuffer.length;
        }
        lwm2m_data_free(size, dataPtr);

        return buffPtr;
    }
    else
    {
        return NULL;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to get the secret key (resource 5 of object 0)
 *
 * @return
 *  - pointer on secret key
 *  - NULL in case of failure
 */
//--------------------------------------------------------------------------------------------------
static char* SecurityGetSecretKey
(
    lwm2m_object_t* objPtr,     ///< [IN] Pointer on the object structure
    int instanceId,             ///< [IN] Objecxt instance Id
    int* lengthPtr              ///< [INOUT] Key length
)
{
    int size = 1;
    lwm2m_data_t* dataPtr = lwm2m_data_new(size);
    dataPtr->id = LWM2MCORE_SECURITY_SECRET_KEY_RID;

    objPtr->readFunc(instanceId, &size, &dataPtr, objPtr);
    if ((NULL != dataPtr)
    && (LWM2M_TYPE_OPAQUE == dataPtr->type))
    {
        char * buffPtr;

        buffPtr = (char*)lwm2m_malloc(dataPtr->value.asBuffer.length);
        if (0 != buffPtr)
        {
            memcpy(buffPtr, dataPtr->value.asBuffer.buffer, dataPtr->value.asBuffer.length);
            *lengthPtr = dataPtr->value.asBuffer.length;
        }
        lwm2m_data_free(size, dataPtr);

        return buffPtr;
    }
    else
    {
        return NULL;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to send data on DTLS
 *
 * @return
 *  - data length sent on DTLS
 *  - 0 if no data were sent
 *  - -1 in case of failure
 */
//--------------------------------------------------------------------------------------------------
static int SendData
(
    dtls_connection_t* connPtr, ///< [IN] DTLS connection
    uint8_t* bufferPtr,         ///< [IN] Buffer to be sent
    size_t length               ///< [IN] Buffer length
)
{
    int nbSent;
    size_t offset;
    LOG("SendData");

#ifdef WITH_LOGS
    char s[INET6_ADDRSTRLEN];
    in_port_t port;

    s[0] = 0;

    if (AF_INET == connPtr->addr.sin6_family)
    {
        struct sockaddr_in *saddrPtr = (struct sockaddr_in *)&connPtr->addr;
        inet_ntop(saddrPtr->sin_family, &saddrPtr->sin_addr, s, INET6_ADDRSTRLEN);
        port = saddrPtr->sin_port;
    }
    else if (AF_INET6 == connPtr->addr.sin6_family)
    {
        struct sockaddr_in6 *saddrPtr = (struct sockaddr_in6 *)&connPtr->addr;
        inet_ntop(saddrPtr->sin6_family, &saddrPtr->sin6_addr, s, INET6_ADDRSTRLEN);
        port = saddrPtr->sin6_port;
    }

    LOG_ARG("Sending %d bytes to [%s]:%hu", length, s, ntohs(port));

#endif
    if (NULL == connPtr->dtlsSessionPtr)
    {
        os_debug_data_dump("Sent bytes in no sec", bufferPtr, length);
    }

    offset = 0;
    while (offset != length)
    {
        nbSent = os_udpSend(connPtr->sock,
                            bufferPtr + offset,
                            length - offset,
                            0,
                            (struct sockaddr *)&(connPtr->addr),
                            connPtr->addrLen);
        if (-1 == nbSent)
        {
            return -1;
        }
        offset += nbSent;
    }
    connPtr->lastSend = lwm2m_gettime();
    return 0;
}

//--------------------------------------------------------------------------------------------------
/**
 * TinyDTLS Callbacks
 * This function is the "key store" for tinyDTLS. It is called to retrieve a key for the given
 * identity within this particular session. PSKs is only supported.
 *
 * @return
 *  - key length
 *  - 0 if no data were sent
 *  - negative value in case of failure (see dtls_alert_t)
 */
//--------------------------------------------------------------------------------------------------
static int GetPskInfo
(
    struct dtls_context_t* ctxPtr,      ///< [IN] DTLS context
    const session_t* sessionPtr,        ///< [IN] DTLS session
    dtls_credentials_type_t type,       ///< [IN] Requested redential
    const unsigned char* idPtr,         ///< [INOUT] Resource Id linked to the requested credential
    size_t idLen,                       ///< [INOUT] Credential length
    unsigned char* resultPtr,           ///< [INOUT] Buffer in which the credential is written
    size_t resultLength                 ///< [IN] Maximum credential length
)
{
    LOG_ARG("GetPskInfo type %d", type);
    // find connection
    dtls_connection_t* cnxPtr = connection_find((dtls_connection_t*) ctxPtr->app,
                                                &(sessionPtr->addr.st),
                                                sessionPtr->size);
    if (NULL == cnxPtr)
    {
        LOG("GET PSK session not found");
        return dtls_alert_fatal_create(DTLS_ALERT_INTERNAL_ERROR);
    }
    switch (type)
    {
        case DTLS_PSK_IDENTITY:
        {
            int idLen;
            char* idPtr;
            idPtr = SecurityGetPublicId(cnxPtr->securityObjPtr, cnxPtr->securityInstId, &idLen);
#ifdef CREDENTIALS_DEBUG
            LOG_ARG("DTLS_PSK_IDENTITY resultLength %d idLen %d", resultLength, idLen);
#endif
            if (resultLength < idLen)
            {
                LOG("cannot set psk_identity -- buffer too small");
                return dtls_alert_fatal_create(DTLS_ALERT_INTERNAL_ERROR);
            }

            memcpy(resultPtr, idPtr, idLen);
            lwm2m_free(idPtr);
            return idLen;
        }
        case DTLS_PSK_KEY:
        {
            int keyLen;
            char* keyPtr;
            keyPtr = SecurityGetSecretKey(cnxPtr->securityObjPtr, cnxPtr->securityInstId, &keyLen);
#ifdef CREDENTIALS_DEBUG
            LOG_ARG("DTLS_PSK_KEY resultLength %d keyLen %d", resultLength, keyLen);
#endif
            if (resultLength < keyLen)
            {
                LOG("cannot set psk -- buffer too small");
                return dtls_alert_fatal_create(DTLS_ALERT_INTERNAL_ERROR);
            }

            memcpy(resultPtr, keyPtr, keyLen);
            lwm2m_free(keyPtr);
            return keyLen;
        }
        default:
            LOG_ARG("unsupported request type: %d", type);
        break;
    }

    return dtls_alert_fatal_create(DTLS_ALERT_INTERNAL_ERROR);
}

//--------------------------------------------------------------------------------------------------
/**
 * TinyDTLS Callbacks
 * This function is the "write" for tinyDTLS. It is called to send a buffer to a peer.
 *
 * @return
 *  - 0 if data were well sent
 *  - -1 in case of failure
 */
//--------------------------------------------------------------------------------------------------
static int SendToPeer
(
    struct dtls_context_t* ctxPtr,      ///< [IN] DTLS context
    session_t* sessionPtr,              ///< [IN] DTLS session
    uint8_t* dataPtr,                   ///< [IN] Buffer to be sent
    size_t len                          ///< [IN] Buffer length
)
{
    // find connection
    dtls_connection_t* connPtr = (dtls_connection_t*) ctxPtr->app;
    dtls_connection_t* cnxPtr = connection_find((dtls_connection_t*) ctxPtr->app,
                                                &(sessionPtr->addr.st),
                                                sessionPtr->size);
    if (NULL != cnxPtr)
    {
        // send data to peer

        // TODO: nat expiration?
        int err = SendData(cnxPtr, dataPtr, len);
        if (COAP_NO_ERROR != err)
        {
            return -1;
        }
        return 0;
    }
    return -1;
}

//--------------------------------------------------------------------------------------------------
/**
 * TinyDTLS Callbacks
 * This function is the "read" for tinyDTLS. It is called to read a buffer from a peer.
 *
 * @return
 *  - 0 if data were well handled
 *  - -1 in case of failure
 */
//--------------------------------------------------------------------------------------------------
static int ReadFromPeer
(
    struct dtls_context_t* ctxPtr,      ///< [IN] DTLS context
    session_t* sessionPtr,              ///< [IN] DTLS session
    uint8_t* dataPtr,                   ///< [INOUT] Buffer
    size_t len                          ///< [IN] Buffer length
)
{
    // find connection
    dtls_connection_t* connPtr = (dtls_connection_t*) ctxPtr->app;
    dtls_connection_t* cnxPtr = connection_find((dtls_connection_t*) ctxPtr->app,
                                                &(sessionPtr->addr.st),
                                                sessionPtr->size);
    if (NULL != cnxPtr)
    {
        lwm2m_handle_packet(cnxPtr->lwm2mHPtr, dataPtr, len, (void*)cnxPtr);
        return 0;
    }
    return -1;
}

//--------------------------------------------------------------------------------------------------
/**
 * DTLS event callback
 *
 * @return
 *  - ignored

 */
//--------------------------------------------------------------------------------------------------
static int dtlsEventCb
(
    struct dtls_context_t* ctxPtr,  ///< [IN] Current dtls context
    session_t* sessionPtr,          ///< [IN] Session object that was affected
    dtls_alert_level_t level,       ///< [IN] Alert level or 0 when an event ocurred that  is not an
                                    ///< alert
    unsigned short code             ///< [IN] Values less than 256 indicate alerts, while 256 or
                                    ///< greater indicate internal DTLS session changes.
)
{
    switch (code)
    {
        case DTLS_EVENT_CONNECT:
        case DTLS_EVENT_RENEGOTIATE:
        {
            /* Notify that the device starts an authentication */
            SendSessionEvent(EVENT_TYPE_AUTHENTICATION, EVENT_STATUS_STARTED);
        }
        break;

        case DTLS_EVENT_CONNECTED:
        {
            /* Notify that the device authentication succeeds */
            SendSessionEvent(EVENT_TYPE_AUTHENTICATION, EVENT_STATUS_DONE_SUCCESS);
        }
        break;

        case DTLS_ALERT_INTERNAL_ERROR:
        {
            /* Notify that the device authentication fails */
            SendSessionEvent(EVENT_TYPE_AUTHENTICATION, EVENT_STATUS_DONE_FAIL);
        }
        break;

        default:
        {
            LOG_ARG("dtlsEventCb unsupported DTLS event %d", code);
        }
        break;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * TinyDTLS Callbacks
 */
//--------------------------------------------------------------------------------------------------
static dtls_handler_t cb = {
  SendToPeer,                   //.write
  ReadFromPeer,                 //.read
  dtlsEventCb,                  //.event
  GetPskInfo,                   //.get_psk_info
};


//--------------------------------------------------------------------------------------------------
/**
 * This function returns a DTLS context from DTLS connection list
 *
 * @return
 *  - dtls_context_t pointer
 */
//--------------------------------------------------------------------------------------------------
static dtls_context_t* GetDtlsContext
(
    dtls_connection_t* connListPtr    ///< [IN] DTLS connection
)
{
    if (NULL == DtlsContextPtr)
    {
        dtls_init();
        DtlsContextPtr = dtls_new_context(connListPtr);
        if (NULL == DtlsContextPtr)
        {
            LOG("Failed to create the DTLS context");
        }
        dtls_set_handler(DtlsContextPtr, &cb);
    }
    else
    {
        DtlsContextPtr->app = connListPtr;
    }
    return DtlsContextPtr;
}

//--------------------------------------------------------------------------------------------------
/**
 * This function returns the port associated to sockaddr
 *
 * @return
 *  - port number
 *  - -1 in case of failure
 */
//--------------------------------------------------------------------------------------------------
static int GetPort
(
    struct sockaddr *x      ///< [IN] Socket information
)
{
   if (x->sa_family == AF_INET)
   {
       return ((struct sockaddr_in *)x)->sin_port;
   }
   else if (x->sa_family == AF_INET6)
   {
       return ((struct sockaddr_in6 *)x)->sin6_port;
   }
   else
   {
       printf("non IPV4 or IPV6 address\n");
       return  -1;
   }
}

//--------------------------------------------------------------------------------------------------
/**
 * This function compares 2 socket information
 *
 * @return
 *  - port number
 *  - -1 in case of failure
 */
//--------------------------------------------------------------------------------------------------
static int SockaddrCmp
(
    struct sockaddr *x,     ///< [IN] Socket information 1
    struct sockaddr *y      ///< [IN] Socket information 2
)
{
    int portX = GetPort(x);
    int portY = GetPort(y);

    // if the port is invalid of different
    if (( -1 == portX) || (portX != portY))
    {
        return 0;
    }

    // IPV4?
    if (AF_INET == x->sa_family)
    {
        // is V4?
        if (AF_INET == y->sa_family)
        {
            // compare V4 with V4
            return ((struct sockaddr_in *)x)->sin_addr.s_addr == \
                                            ((struct sockaddr_in *)y)->sin_addr.s_addr;
            // is V6 mapped V4?
        }
        else if (IN6_IS_ADDR_V4MAPPED(&((struct sockaddr_in6 *)y)->sin6_addr))
        {
            struct in6_addr* addr6 = &((struct sockaddr_in6 *)y)->sin6_addr;
            uint32_t y6to4 = addr6->s6_addr[15] << 24 | addr6->s6_addr[14] << 16 | \
                                                    addr6->s6_addr[13] << 8 | addr6->s6_addr[12];
            return y6to4 == ((struct sockaddr_in *)x)->sin_addr.s_addr;
        }
        else
        {
            return 0;
        }
    }
    else if (x->sa_family == AF_INET6 && y->sa_family == AF_INET6)
    {
        // IPV6 with IPV6 compare
        return memcmp(((struct sockaddr_in6 *)x)->sin6_addr.s6_addr,
                      ((struct sockaddr_in6 *)y)->sin6_addr.s6_addr,
                      16) == 0;
    }
    else
    {
        // unknown address type
        printf("non IPV4 or IPV6 address\n");
        return 0;
    }
}

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
)
{
    dtls_connection_t* connPtr;

    connPtr = connListPtr;
    while (NULL != connPtr)
    {

       if (SockaddrCmp((struct sockaddr*) (&connPtr->addr), (struct sockaddr*) addrPtr))
       {
            return connPtr;
       }

       connPtr = connPtr->nextPtr;
    }

    return connPtr;
}

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
)
{
    dtls_connection_t* connPtr;

    connPtr = (dtls_connection_t*)malloc(sizeof(dtls_connection_t));
    if (NULL != connPtr)
    {
        connPtr->sock = sock;
        memcpy(&(connPtr->addr), addrPtr, addrLen);
        connPtr->addrLen = addrLen;
        connPtr->nextPtr = connListPtr;

        connPtr->dtlsSessionPtr = (session_t*)malloc(sizeof(session_t));
        connPtr->dtlsSessionPtr->addr.sin6 = connPtr->addr;
        connPtr->dtlsSessionPtr->size = connPtr->addrLen;
        connPtr->lastSend = lwm2m_gettime();
    }

    return connPtr;
}

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
)
{
    struct addrinfo hints;
    struct addrinfo* servinfoPtr = NULL;
    struct addrinfo* p;
    int s;
    struct sockaddr* saPtr;
    socklen_t sl;
    dtls_connection_t* connPtr = NULL;
    char uriBuf[URI_LENGTH];
    char* uriPtr;
    char* hostPtr;
    char* portPtr;
    char* defaultPortPtr;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = addressFamily;
    hints.ai_socktype = SOCK_DGRAM;

    LOG("Entering");

    uriPtr = SecurityGetUri(securityObjPtr, instanceId, uriBuf, URI_LENGTH);
    LOG_ARG("connection_create uri %s", uriPtr);
    if (NULL == uriPtr)
    {
        return NULL;
    }

    // parse uri in the form "coaps://[host]:[port]"
    if (0 == strncmp(uriPtr, "coaps://", strlen("coaps://")))
    {
        hostPtr = uriPtr + strlen("coaps://");
        defaultPortPtr = COAPS_PORT;
    }
    else if (0 == strncmp(uriPtr, "coap://", strlen("coap://")))
    {
        hostPtr = uriPtr + strlen("coap://");
        defaultPortPtr = COAP_PORT;
    }
    else
    {
        LOG("ERROR in uri");
        return NULL;
    }
    portPtr = strrchr(hostPtr, ':');
    if (NULL == portPtr)
    {
        portPtr = defaultPortPtr;
    }
    else
    {
        // remove brackets
        if (hostPtr[0] == '[')
        {
            hostPtr++;
            if (*(portPtr - 1) == ']')
            {
                *(portPtr - 1) = 0;
            }
            return NULL;
        }
        // split strings
        *portPtr = 0;
        portPtr++;
    }
    LOG_ARG("port %s", portPtr);

    if (0 != getaddrinfo(hostPtr, portPtr, &hints, &servinfoPtr) || servinfoPtr == NULL)
    {
        return NULL;
    }

    // we test the various addresses
    s = -1;
    for(p = servinfoPtr ; p != NULL && s == -1 ; p = p->ai_next)
    {
        s = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (s >= 0)
        {
            saPtr = p->ai_addr;
            sl = p->ai_addrlen;
            if (-1 == connect(s, p->ai_addr, p->ai_addrlen))
            {
                close(s);
                s = -1;
            }
        }
    }
    if (s >= 0)
    {
        connPtr = connection_newIncoming(connListPtr, sock, saPtr, sl);
        close(s);

        // do we need to start tinydtls?
        if (NULL != connPtr)
        {
            connPtr->securityObjPtr = securityObjPtr;
            connPtr->securityInstId = instanceId;
            connPtr->lwm2mHPtr = lwm2mHPtr;

            if (LWM2M_SECURITY_MODE_NONE != SecurityGetMode(connPtr->securityObjPtr,
                                                            connPtr->securityInstId))
            {
                connPtr->dtlsContextPtr = GetDtlsContext(connPtr);
            }
            else
            {
                // no dtls session
                connPtr->dtlsSessionPtr = NULL;
            }
        }
    }

    if (NULL != servinfoPtr)
    {
        free(servinfoPtr);
    }

    return connPtr;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to free the DTLS connection list
 */
//--------------------------------------------------------------------------------------------------
void connection_free
(
    dtls_connection_t* connListPtr
)
{
    while (NULL != connListPtr)
    {
        dtls_connection_t* nextPtr;

        nextPtr = connListPtr->nextPtr;
        free(connListPtr);

        connListPtr = nextPtr;
    }
    dtls_free_context(DtlsContextPtr);
    DtlsContextPtr = NULL;
}

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
static int ConnectionSend
(
    dtls_connection_t* connPtr,         ///< [IN] DTLS connection structure
    uint8_t* bufferPtr,                 ///< [IN] Buffer to be sent
    size_t length                       ///< [IN] Buffer length
)
{
    if (NULL == connPtr->dtlsSessionPtr)
    {
        LOG("ConnectionSend NO SEC");
        // no security
        if ( 0 != SendData(connPtr, bufferPtr, length))
        {
            LOG("ConnectionSend SendData != 0");
            return -1 ;
        }
    }
    else
    {
        LOG_ARG("now - connP->lastSend %d", lwm2m_gettime() - connPtr->lastSend);
        if ((0 < DTLS_NAT_TIMEOUT)
         && (DTLS_NAT_TIMEOUT < (lwm2m_gettime() - connPtr->lastSend)))
        {
            // we need to rehandhake because our source IP/port probably changed for the server
            if (0 != connection_rehandshake(connPtr, false))
            {
                LOG("can't send due to rehandshake error");
                return -1;
            }
        }
        LOG_ARG("ConnectionSend SEC length %d", length);
        os_debug_data_dump("Data to send", bufferPtr, length);
        if (-1 == dtls_write(connPtr->dtlsContextPtr,
                             connPtr->dtlsSessionPtr,
                             bufferPtr,
                             length))
        {
            LOG("ConnectionSend dtls_write -1");
            return -1;
        }
    }

    return 0;
}

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
)
{
    if (NULL != connPtr->dtlsSessionPtr)
    {
        // Let liblwm2m respond to the query depending on the context
        int result = dtls_handle_message(connPtr->dtlsContextPtr,
                                         connPtr->dtlsSessionPtr,
                                         bufferPtr,
                                         numBytes);
        if (0 != result)
        {
             LOG_ARG("error dtls handling message %d",result);
        }
        return result;
    }
    else
    {
        // no security, just give the plaintext buffer to liblwm2m
        os_debug_data_dump("received bytes in no sec", bufferPtr, numBytes);
        lwm2m_handle_packet(connPtr->lwm2mHPtr, bufferPtr, numBytes, (void*)connPtr);
        return 0;
    }
}

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
    dtls_connection_t* connPtr,
    bool sendCloseNotify
)
{
    LOG("Entering");
    // if not a dtls connection we do nothing
    if (NULL == connPtr->dtlsSessionPtr)
    {
        return 0;
    }

    // reset current session
    dtls_peer_t* peer = dtls_get_peer(connPtr->dtlsContextPtr, connPtr->dtlsSessionPtr);
    if (peer != NULL)
    {
        if (!sendCloseNotify)
        {
            peer->state =  DTLS_STATE_CLOSED;
        }
        dtls_reset_peer(connPtr->dtlsContextPtr, peer);
    }

    // start a fresh handshake
    int result = dtls_connect(connPtr->dtlsContextPtr, connPtr->dtlsSessionPtr);
    if (0 != result)
    {
         LOG_ARG("error dtls reconnection %d",result);
    }
    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to send data on a specific peer
 *
 * @return
 *  - COAP_NO_ERROR in case of success
 *  - COAP_500_INTERNAL_SERVER_ERROR in case of failure
 */
//--------------------------------------------------------------------------------------------------
uint8_t lwm2m_buffer_send
(
    void* sessionHPtr,      ///< [IN] Session handle identifying the peer (opaque to the core)
    uint8_t* bufferPtr,     ///< [IN] Data to be sent
    size_t length,          ///< [IN] Data length
    void* userdataPtr       ///< [IN] Parameter to lwm2m_init()
)
{
    dtls_connection_t* connPtr = (dtls_connection_t*) sessionHPtr;

    LOG("lwm2m_buffer_send");
    if (NULL == connPtr)
    {
        LOG_ARG("#> failed sending %lu bytes, missing connection", length);
        return COAP_500_INTERNAL_SERVER_ERROR ;
    }

    if (-1 == ConnectionSend(connPtr, bufferPtr, length))
    {
        LOG_ARG("#> failed sending %lu bytes", length);
        return COAP_500_INTERNAL_SERVER_ERROR ;
    }

    return COAP_NO_ERROR;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to compare two LWM2M session handles
 *
 * @return
 *  - true if the two sessions identify the same peer
 *  - true if the two sessions do not identify the same peer
 */
//--------------------------------------------------------------------------------------------------
bool lwm2m_session_is_equal
(
    void * session1Ptr,
    void * session2Ptr,
    void * userDataPtr
)
{
    return (session1Ptr == session2Ptr);
}

