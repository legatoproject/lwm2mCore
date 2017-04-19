/**
 * @file socket.h
 *
 * Header file for adaptation layer for socket management
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#ifndef __LWM2MCORE_SOCKET_H__
#define __LWM2MCORE_SOCKET_H__

#include <platform/inet.h>

//--------------------------------------------------------------------------------------------------
/**
 * Structure for address family
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    LWM2MCORE_SOCK_AF_UNSPEC,       ///< Unspecified family
    LWM2MCORE_SOCK_AF_INET,         ///< IP v4
    LWM2MCORE_SOCK_AF_INET6,        ///< IP v6
    LWM2MCORE_SOCK_AF_MAX           ///< Internal value
}lwm2mcore_SocketAf_t;

//--------------------------------------------------------------------------------------------------
/**
 * Structure for socket type
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    LWM2MCORE_SOCK_STREAM,     ///< Stream
    LWM2MCORE_SOCK_DGRAM,      ///< Datagram
    LWM2MCORE_SOCK_TYPE_MAX,   ///< Internal value
}lwm2mcore_SocketType_t;

//--------------------------------------------------------------------------------------------------
/**
 * Structure for socket prototype
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    LWM2MCORE_SOCK_ICMP,       ///< ICMP
    LWM2MCORE_SOCK_TCP,        ///< TCP
    LWM2MCORE_SOCK_UDP,        ///< UDP
    LWM2MCORE_SOCK_ICMPV6,     ///< ICMP v6
    LWM2MCORE_SOCK_PROTO_MAX,  ///< Internal value
}lwm2mcore_SocketPrototype_t;

//--------------------------------------------------------------------------------------------------
/**
 * Structure for socket configuration
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    lwm2mcore_SocketAf_t af;            ///< Address family
    lwm2mcore_SocketType_t type;        ///< Socket type
    lwm2mcore_SocketPrototype_t proto;  ///< Socket prototype
    int sock;                           ///< Socket fd
    lwm2mcore_Ref_t instanceRef;        ///< Instance reference
}lwm2mcore_SocketConfig_t;

#endif /* __LWM2MCORE_SOCKET_H__ */
