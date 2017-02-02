/**
 * @file osUdp.h
 *
 * Header file for adaptation layer for UDP socket management
 *
 * <hr>
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#ifndef __OSUDP_H__
#define __OSUDP_H__

#include <arpa/inet.h>
#include <unistd.h>
#include <sys/stat.h>

//--------------------------------------------------------------------------------------------------
/**
 *  Maximum size of packet that can be received on UDP socket.
 */
//--------------------------------------------------------------------------------------------------
#define OS_UDP_MAX_PACKET_SIZE 1024

//--------------------------------------------------------------------------------------------------
/**
 * Structure for address family
 */
//--------------------------------------------------------------------------------------------------
typedef enum _os_socketAf
{
    OS_SOCK_AF_UNSPEC,       ///< Unspecified family
    OS_SOCK_AF_INET,         ///< IP v4
    OS_SOCK_AF_INET6,        ///< IP v6
    OS_SOCK_AF_MAX           ///< Internal value
}os_socketAf_t;

//--------------------------------------------------------------------------------------------------
/**
 * Structure for socket type
 */
//--------------------------------------------------------------------------------------------------
typedef enum _os_socketType
{
    OS_SOCK_STREAM,     ///< Stream
    OS_SOCK_DGRAM,      ///< Datagram
    OS_SOCK_TYPE_MAX,   ///< Internal value
}os_socketType_t;

//--------------------------------------------------------------------------------------------------
/**
 * Structure for socket prototype
 */
//--------------------------------------------------------------------------------------------------
typedef enum _os_socketPrototype
{
    OS_SOCK_ICMP,       ///< ICMP
    OS_SOCK_TCP,        ///< TCP
    OS_SOCK_UDP,        ///< UDP
    OS_SOCK_ICMPV6,     ///< ICMP v6
    OS_SOCK_PROTO_MAX,  ///< Internal value
}os_socketPrototype_t;

//--------------------------------------------------------------------------------------------------
/**
 * Structure for socket configuration
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    os_socketAf_t af;               ///< Address family
    os_socketType_t type;           ///< Socket type
    os_socketPrototype_t proto;     ///< Socket prototype
    int sock;                       ///< Socket Id
    int context;                    ///< Context
}os_socketConfig_t;

//--------------------------------------------------------------------------------------------------
/**
 * Callback for data receipt
 */
//--------------------------------------------------------------------------------------------------
typedef void* (*os_udpCb_t)
(
    uint8_t* bufferPtr,                     ///< [IN] Received data
    uint32_t len,                           ///< [IN] Received data length
    struct sockaddr_storage* addrPtr,       ///< [INOUT] source address
    socklen_t addrlen,                      ///< @TODO
    os_socketConfig_t config                ///< [IN] Socket config
);

//--------------------------------------------------------------------------------------------------
/**
 * Callback for data receipt
 * This function is defined in LWM2MCore
 */
//--------------------------------------------------------------------------------------------------
void os_udpReceiveCb
(
    uint8_t* bufferPtr,                     ///< [IN] Received data
    uint32_t len,                           ///< [IN] Received data length
    struct sockaddr_storage* addrPtr,       ///< [INOUT] source address
    socklen_t addrlen,                      ///< @TODO
    os_socketConfig_t config                ///< [IN] Socket config
);

//--------------------------------------------------------------------------------------------------
/**
 * Open a socket to the server
 * This function is called by the LWM2MCore and must be adapted to the platform
 * The aim of this function is to create a socket and fill the configPtr structure
 *
 * @return
 *      - true on success
 *      - false on error
 *
 */
//--------------------------------------------------------------------------------------------------
bool os_udpOpen
(
    int context,                    ///< [IN] LWM2M context
    os_udpCb_t callback,            ///< [IN] callback for data receipt
    os_socketConfig_t* config       ///< [INOUT] socket configuration
);

//--------------------------------------------------------------------------------------------------
/**
 * Close the socket
 * This function is called by the LWM2MCore and must be adapted to the platform
 * The aim of this function is to close a socket
 *
 * @return
 *      - true on success
 *      - false on error
 *
 */
//--------------------------------------------------------------------------------------------------
bool os_udpClose
(
    os_socketConfig_t config        ///< [INOUT] socket configuration
);

#endif //__OSUDP_H__

