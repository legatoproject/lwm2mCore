/**
 * @file osSocket.h
 *
 * Header file for adaptation layer for socket management
 *
 * <hr>
 *
 * Copyright (C) Sierra Wireless Inc. Use of this work is subject to license.
 *
 */

#ifndef OS_SOCKET_H
#define OS_SOCKET_H

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
    os_socketPrototype_t proto      ///< Socket prototype
    int sock;                       ///< Socket Id
    int context;                    ///< Context
}os_socketConfig_t;



#define OS_SOCKET_INADDR_ANY ((uint32_t)0)
#define OS_SOCKET_INADDR_NONE ((uint32_t)0xffffffff)
#define OS_SOCKET_INADDR_BROADCAST ((uint32_t)0xffffffff)
#define OS_SOCKET_INVALID_SOCKET (-1)

/* Must undefine s_addr because of pj_in_addr below */
typedef struct _os_socket_in_addr
{
    uint32_t s_addr;
} os_socket_in_addr_t;

typedef union _os_sock_in6_addr
{
    /* This is the main entry */
    uint8_t s6_addr[16];
    /* While these are used for proper alignment */
    uint32_t u6_addr32[4];
} os_sock_in6_addr_t;

#ifndef OS_SOCKET_SOCKADDR_IN_SIN_ZERO_LEN
# define OS_SOCKET_SOCKADDR_IN_SIN_ZERO_LEN 8
#endif

typedef struct _os_socket_sockaddr_in
{
    uint16_t sin_family;
    uint16_t sin_port;
    os_socket_in_addr_t sin_addr;
    char sin_zero[OS_SOCKET_SOCKADDR_IN_SIN_ZERO_LEN];
}os_socket_sockaddr_in_t;


typedef struct _os_socket_sockaddr_in6
{
    uint16_t sin6_family;
    uint16_t sin6_port;
    uint32_t sin6_flowinfo;
    os_sock_in6_addr_t sin6_addr;
    uint32_t sin6_scope_id;
} os_socket_sockaddr_in6_t;

typedef struct _os_socket_addr_hdr
{
    uint16_t sa_family;
}os_socket_addr_hdr_t;


typedef union _os_socket_sockaddr
{
    os_socket_addr_hdr_t addr;
    os_socket_sockaddr_in_t ipv4;
    os_socket_sockaddr_in6_t ipv6;
}os_socket_sockaddr_t;


typedef struct _os_socket_ip_mreq
{
    os_socket_in_addr_t imr_multiaddr;
    os_socket_in_addr_t imr_interface;
}os_socket_ip_mreq_t;



//--------------------------------------------------------------------------------------------------
/**
 * Convert 16-bit value from network byte order to host byte order.
 *
 * @return
 *      - 16-bit host value
 *
 */
//--------------------------------------------------------------------------------------------------
uint16_t os_socket_ntohs(
    uint16_t netshort       ///< [IN] 16-bit network value
);


#endif // OS_SOCKET_H

