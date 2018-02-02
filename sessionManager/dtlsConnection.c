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
#include <platform/types.h>
#include <lwm2mcore/lwm2mcore.h>
#include <lwm2mcore/socket.h>
#include <lwm2mcore/udp.h>
#include "objects.h"
#include "dtlsConnection.h"
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
    uint8_t result;
    if (!dataPtr)
    {
       LOG("Memory not allocated for dataPtr");
       return NULL;
    }
    dataPtr->id = LWM2M_SECURITY_URI_ID;

    result = objPtr->readFunc(instanceId, &size, &dataPtr, objPtr);

    if ( (COAP_205_CONTENT == result)
      && (NULL != dataPtr)
      && (LWM2M_TYPE_STRING == dataPtr->type)
      && (0 < dataPtr->value.asBuffer.length))

    {
        if((size_t)bufferSize > dataPtr->value.asBuffer.length)
        {
            memset(uriBufferPtr, 0, (dataPtr->value.asBuffer.length + 1 ));
            strncpy(uriBufferPtr,
                    (const char*) dataPtr->value.asBuffer.buffer,
                    dataPtr->value.asBuffer.length);
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
    uint8_t result;
    lwm2m_data_t* dataPtr = lwm2m_data_new(size);
    if (!dataPtr)
    {
       LOG("Memory not allocated for dataPtr");
       return LWM2M_SECURITY_MODE_NONE;
    }
    dataPtr->id = LWM2MCORE_SECURITY_MODE_RID;

    result = objPtr->readFunc(instanceId, &size, &dataPtr, objPtr);
    if ( (COAP_205_CONTENT == result)
      && (0 != lwm2m_data_decode_int(dataPtr,&mode)))
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
    uint8_t result;
    lwm2m_data_t* dataPtr = lwm2m_data_new(size);
    if (!dataPtr)
    {
       LOG("Memory not allocated for dataPtr");
       return NULL;
    }
    dataPtr->id = LWM2MCORE_SECURITY_PKID_RID;

    result = objPtr->readFunc(instanceId, &size, &dataPtr, objPtr);
    if ( (COAP_205_CONTENT == result)
      && (NULL != dataPtr)
      && (LWM2M_TYPE_OPAQUE == dataPtr->type)
      && (dataPtr->value.asBuffer.length))
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

    lwm2m_data_free(size, dataPtr);
    return NULL;
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
    uint8_t result;
    lwm2m_data_t* dataPtr = lwm2m_data_new(size);
    if (!dataPtr)
    {
       LOG("Memory not allocated for dataPtr");
       return NULL;
    }
    dataPtr->id = LWM2MCORE_SECURITY_SECRET_KEY_RID;

    result = objPtr->readFunc(instanceId, &size, &dataPtr, objPtr);
    if ( (COAP_205_CONTENT == result)
      && (NULL != dataPtr)
      && (LWM2M_TYPE_OPAQUE == dataPtr->type)
      && (dataPtr->value.asBuffer.length))
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

    lwm2m_data_free(size, dataPtr);
    return NULL;
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
    dtls_Connection_t* connPtr, ///< [IN] DTLS connection
    uint8_t* bufferPtr,         ///< [IN] Buffer to be sent
    size_t length               ///< [IN] Buffer length
)
{
    int nbSent;
    size_t offset;
    LOG("SendData");

#ifdef WITH_LOGS
    char s[INET6_ADDRSTRLEN];
    in_port_t port = 0;

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
        lwm2mcore_DataDump("Sent bytes in no sec", bufferPtr, length);
    }

    offset = 0;
    while (offset != length)
    {
        nbSent = lwm2mcore_UdpSend(connPtr->sock,
                                   bufferPtr + offset,
                                   length - offset,
                                   0,
                                   (struct sockaddr *)&(connPtr->addr),
                                   connPtr->addrLen);
        if (-1 == nbSent)
        {
            lwm2mcore_ReportUdpErrorCode(LWM2MCORE_UDP_SEND_ERR);
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
    dtls_Connection_t* cnxPtr;

    (void)idPtr;
    (void)idLen;

    LOG_ARG("GetPskInfo type %d", type);
    // find connection
    cnxPtr = dtls_FindConnection((dtls_Connection_t*) ctxPtr->app,
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
            int length = 0;
            char* identityPtr;
            identityPtr = SecurityGetPublicId(cnxPtr->securityObjPtr,
                                              cnxPtr->securityInstId,
                                              &length);
#ifdef CREDENTIALS_DEBUG
            LOG_ARG("DTLS_PSK_IDENTITY resultLength %d length %d", resultLength, length);
#endif
            if (resultLength < (size_t)length)
            {
                if (identityPtr)
                {
                   lwm2m_free(identityPtr);
                }

                LOG("Cannot set psk_identity -- buffer too small");
                return dtls_alert_fatal_create(DTLS_ALERT_INTERNAL_ERROR);
            }

            memcpy(resultPtr, identityPtr, length);
            lwm2m_free(identityPtr);
            return length;
        }
        case DTLS_PSK_KEY:
        {
            int keyLen = 0;
            char* keyPtr;
            keyPtr = SecurityGetSecretKey(cnxPtr->securityObjPtr, cnxPtr->securityInstId, &keyLen);
#ifdef CREDENTIALS_DEBUG
            LOG_ARG("DTLS_PSK_KEY resultLength %d keyLen %d", resultLength, keyLen);
#endif
            if (resultLength < (size_t)keyLen)
            {
                if (keyPtr)
                {
                   lwm2m_free(keyPtr);
                }

                LOG("Cannot set psk -- buffer too small");
                return dtls_alert_fatal_create(DTLS_ALERT_INTERNAL_ERROR);
            }

            memcpy(resultPtr, keyPtr, keyLen);
            lwm2m_free(keyPtr);
            return keyLen;
        }
        default:
            LOG_ARG("Unsupported request type: %d", type);
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
    dtls_Connection_t* cnxPtr;

    if ((!ctxPtr) || (!sessionPtr) || (!dataPtr))
    {
        return -1;
    }

    // find connection
    cnxPtr = dtls_FindConnection((dtls_Connection_t*) ctxPtr->app,
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
    dtls_Connection_t* cnxPtr = dtls_FindConnection((dtls_Connection_t*) ctxPtr->app,
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
    (void)level;
    (void)sessionPtr;
    (void)ctxPtr;

    switch (code)
    {
        case DTLS_EVENT_CONNECT:
        case DTLS_EVENT_RENEGOTIATE:
        {
            /* Notify that the device starts an authentication */
            smanager_SendSessionEvent(EVENT_TYPE_AUTHENTICATION, EVENT_STATUS_STARTED);
        }
        break;

        case DTLS_EVENT_CONNECTED:
        {
            /* Notify that the device authentication succeeds */
            smanager_SendSessionEvent(EVENT_TYPE_AUTHENTICATION, EVENT_STATUS_DONE_SUCCESS);
        }
        break;

        case DTLS_ALERT_INTERNAL_ERROR:
        case DTLS_ALERT_HANDSHAKE_FAILURE:
        {
            /* Notify that the device authentication fails */
            smanager_SendSessionEvent(EVENT_TYPE_AUTHENTICATION, EVENT_STATUS_DONE_FAIL);
        }
        break;

        default:
        {
            LOG_ARG("DtlsEventCb unsupported DTLS event %d", code);
        }
        break;
    }

    return 0;
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
  NULL,                         //.get_ecdsa_key
  NULL                          //.verify_ecdsa_key
};


//--------------------------------------------------------------------------------------------------
/**
 * This function returns a DTLS context from DTLS connection list
 *
 * @return
 *  - dtls_context_t pointer
 *  - NULL in case of failure
 */
//--------------------------------------------------------------------------------------------------
static dtls_context_t* GetDtlsContext
(
    dtls_Connection_t* connListPtr    ///< [IN] DTLS connection
)
{
    if (NULL == DtlsContextPtr)
    {
        dtls_init();
        DtlsContextPtr = dtls_new_context(connListPtr);
        if (NULL == DtlsContextPtr)
        {
            LOG("Failed to create the DTLS context");
            return NULL;
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
    struct sockaddr* x      ///< [IN] Socket information
)
{
    if (!x)
    {
        LOG("Invalid parameter");
        return -1;
    }

    if (x->sa_family == AF_INET)
    {
        return ((struct sockaddr_in *)(void*)x)->sin_port;
    }
    else if (x->sa_family == AF_INET6)
    {
        return ((struct sockaddr_in6 *)(void*)x)->sin6_port;
    }
    else
    {
        LOG("non IPV4 or IPV6 address");
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
    int portX;
    int portY;

    if ((!x) || (!y))
    {
        LOG("Invalid parameter");
        return -1;
    }

    portX = GetPort(x);
    portY = GetPort(y);

    // if the port is invalid of different
    if ((-1 == portX) || (portX != portY))
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
            return ((struct sockaddr_in *)(void*)x)->sin_addr.s_addr == \
                                            ((struct sockaddr_in *)(void*)y)->sin_addr.s_addr;
            // is V6 mapped V4?
        }
        else if (IN6_IS_ADDR_V4MAPPED(&((struct sockaddr_in6 *)(void*)y)->sin6_addr))
        {
            struct in6_addr* addr6 = &((struct sockaddr_in6 *)(void*)y)->sin6_addr;
            uint32_t y6to4 = addr6->s6_addr[15] << 24 | \
                             addr6->s6_addr[14] << 16 | \
                             addr6->s6_addr[13] << 8 | \
                             addr6->s6_addr[12];
            return y6to4 == ((struct sockaddr_in *)(void*)x)->sin_addr.s_addr;
        }
        else
        {
            return 0;
        }
    }
    else if (x->sa_family == AF_INET6 && y->sa_family == AF_INET6)
    {
        // IPV6 with IPV6 compare
        return memcmp(((struct sockaddr_in6 *)(void*)x)->sin6_addr.s6_addr,
                      ((struct sockaddr_in6 *)(void*)y)->sin6_addr.s6_addr,
                      16) == 0;
    }
    else
    {
        // unknown address type
        LOG("Non IPV4 or IPV6 address");
        return 0;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to search if a DTLS connection is available
 *
 * @return
 *  - dtls_Connection_t pointer if the DTLS connection is available
 *  - NULL if the DTLS connection is not available in the indicated socket
 */
//--------------------------------------------------------------------------------------------------
dtls_Connection_t* dtls_FindConnection
(
    dtls_Connection_t* connListPtr,         ///< [IN] DTLS connection list
    const struct sockaddr_storage* addrPtr, ///< [IN] Socket address structure
    size_t addrLen                          ///< [IN] Socket address structure length
)
{
    dtls_Connection_t* connPtr;

    if (!addrPtr)
    {
        return NULL;
    }

    (void)addrLen;

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
 *  - DTLS connection structure (dtls_Connection_t);
 *  - NULL in case of failure
 */
//--------------------------------------------------------------------------------------------------
dtls_Connection_t* dtls_HandleNewIncoming
(
    dtls_Connection_t* connListPtr,     ///< [IN] DTLS connection list
    int sock,                           ///< [IN] Socket Id on which the DTLS needs to be created
    const struct sockaddr* addrPtr,     ///< [IN] Socket address structure
    size_t addrLen                      ///< [IN] Socket address structure length
)
{
    dtls_Connection_t* connPtr = (dtls_Connection_t*)lwm2m_malloc(sizeof(dtls_Connection_t));
    if (NULL != connPtr)
    {
        connPtr->sock = sock;
        memcpy(&(connPtr->addr), addrPtr, addrLen);
        connPtr->addrLen = addrLen;
        connPtr->nextPtr = connListPtr;

        connPtr->dtlsSessionPtr = (session_t*)lwm2m_malloc(sizeof(session_t));
        if (!(connPtr->dtlsSessionPtr))
        {
           LOG("connPtr->dtlsSessionPtr is NULL");
           return NULL;
        }
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
 *  - DTLS connection pointer (dtls_Connection_t)
 *  - NULL in case of failure
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
)
{
    int s;
    struct sockaddr saPtr;
    socklen_t sl = 0;
    dtls_Connection_t* connPtr = NULL;
    char uriBuf[URI_LENGTH];
    char* uriPtr;
    char* hostPtr;
    char* portPtr;
    const char* defaultPortPtr;

    LOG("Entering");

    uriPtr = SecurityGetUri(securityObjPtr, instanceId, uriBuf, URI_LENGTH);
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
        portPtr = (char*)defaultPortPtr;
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
            else
            {
                return NULL;
            }
        }
        // split strings
        *portPtr = 0;
        portPtr++;
    }

    if (false == lwm2mcore_UdpConnect(uriPtr, hostPtr, portPtr, addressFamily, &saPtr, &sl, &s))
    {
        LOG("Connect failure");
        lwm2mcore_ReportUdpErrorCode(LWM2MCORE_UDP_CONNECT_ERR);
        return NULL;
    }

    if (s >= 0)
    {
        lwm2mcore_SocketConfig_t config;
        connPtr = dtls_HandleNewIncoming(connListPtr, sock, &saPtr, sl);
        config.sock = s;
        if (!lwm2mcore_UdpClose(config))
        {
            lwm2mcore_ReportUdpErrorCode(LWM2MCORE_UDP_CLOSE_ERR);
        }

        // Do we need to start tinydtls?
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

    return connPtr;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to free the DTLS connection list
 */
//--------------------------------------------------------------------------------------------------
void dtls_FreeConnection
(
    dtls_Connection_t* connListPtr
)
{
    dtls_free_context(DtlsContextPtr);
    DtlsContextPtr = NULL;

    while (NULL != connListPtr)
    {
        dtls_Connection_t* nextPtr = connListPtr->nextPtr;
        if (connListPtr->dtlsSessionPtr)
        {
            lwm2m_free(connListPtr->dtlsSessionPtr);
        }
        lwm2m_free(connListPtr);
        connListPtr = nextPtr;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to update DTLS connection list
 */
//--------------------------------------------------------------------------------------------------
void dtls_UpdateDtlsList
(
    dtls_Connection_t* connListPtr      ///< [IN] DTLS connection structure
)
{
    if (DtlsContextPtr)
    {
        DtlsContextPtr->app = connListPtr;
    }
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
    dtls_Connection_t* connPtr,         ///< [IN] DTLS connection structure
    uint8_t* bufferPtr,                 ///< [IN] Buffer to be sent
    size_t length,                      ///< [IN] Buffer length
    bool firstBlock                     ///< [IN] First data block
)
{
    if (NULL == connPtr->dtlsSessionPtr)
    {
        LOG("ConnectionSend NO SEC");
        // no security
        if ( 0 != SendData(connPtr, bufferPtr, length))
        {
            LOG("ConnectionSend SendData != 0");
            return -1;
        }
    }
    else
    {
        time_t timeFromLastData = lwm2m_gettime() - connPtr->lastSend;
        LOG_ARG("now - connP->lastSend %d", timeFromLastData);

        if (firstBlock)
        {
            // If difference is negative, a time update could have been made on platform side.
            // In this case, do a rehandshake
            if (timeFromLastData < 0)
            {
                // We need to rehandhake because our source IP/port probably changed for the server
                if (0 != dtls_Rehandshake(connPtr, false))
                {
                    LOG("Unable to perform rehandshake");
                    return -1;
                }
            }
            else if ((0 < DTLS_NAT_TIMEOUT) && (DTLS_NAT_TIMEOUT < timeFromLastData))
            {
                if (0 != dtls_Resume(connPtr))
                {
                    LOG("Unable to resume. Fall-back to a rehandshake");
                    if (0 != dtls_Rehandshake(connPtr, false))
                    {
                        LOG("Unable to perform rehandshake");
                        return -1;
                    }
                }
            }
        }

        LOG_ARG("ConnectionSend SEC length %d", length);
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
int dtls_HandlePacket
(
    dtls_Connection_t* connPtr,         ///< [IN] DTLS connection structure
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
             LOG_ARG("Error DTLS handling message %d",result);
        }
        return result;
    }
    else
    {
        // no security, just give the plaintext buffer to liblwm2m
        lwm2mcore_DataDump("Received bytes in no sec", bufferPtr, numBytes);
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
int dtls_Rehandshake
(
    dtls_Connection_t* connPtr,
    bool sendCloseNotify
)
{
    dtls_peer_t* peer;
    int result;

    LOG("Initiate a DTLS rehandshake");

    // if not a dtls connection we do nothing
    if (NULL == connPtr->dtlsSessionPtr)
    {
        return 0;
    }

    // reset current session
    peer = dtls_get_peer(connPtr->dtlsContextPtr, connPtr->dtlsSessionPtr);
    if (peer != NULL)
    {
        if (!sendCloseNotify)
        {
            peer->state =  DTLS_STATE_CLOSED;
        }
        dtls_reset_peer(connPtr->dtlsContextPtr, peer);
    }

    // start a fresh handshake
    result = dtls_connect(connPtr->dtlsContextPtr, connPtr->dtlsSessionPtr);
    if (0 != result)
    {
         LOG_ARG("Error DTLS reconnection %d",result);
    }
    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to resume a DTLS session
 *
 * @return
 *  - 0 in case of success (or DTLS is not activated on the connection)
 *  - -1 in case of failure
 */
//--------------------------------------------------------------------------------------------------
int dtls_Resume
(
    dtls_Connection_t* connPtr  ///< [IN] DTLS connection structure
)
{
    int result;

    LOG("Initiate a DTLS resume");

    // if not a dtls connection we do nothing
    if (NULL == (connPtr->dtlsSessionPtr))
    {
        return 0;
    }

    // start a resume
    result = dtls_resume(connPtr->dtlsContextPtr, connPtr->dtlsSessionPtr);
    if (result)
    {
         LOG_ARG("Error DTLS resume %d",result);
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
    void* userDataPtr,      ///< [IN] Parameter to lwm2m_init()
    bool firstBlock         ///< [IN] First block
)
{
    dtls_Connection_t* connPtr = (dtls_Connection_t*) sessionHPtr;

    (void)userDataPtr;

    if (NULL == connPtr)
    {
        LOG_ARG("#> Failed sending %lu bytes, missing connection", length);
        return COAP_500_INTERNAL_SERVER_ERROR ;
    }

    if (-1 == ConnectionSend(connPtr, bufferPtr, length, firstBlock))
    {
        LOG_ARG("#> Failed sending %lu bytes", length);
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
    (void)userDataPtr;
    return (session1Ptr == session2Ptr);
}
