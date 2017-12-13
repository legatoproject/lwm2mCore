
/**
 * @file udp.c
 *
 * Adaptation layer for os socket
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <platform/types.h>
#include <lwm2mcore/lwm2mcore.h>
#include <lwm2mcore/udp.h>
#include <platform/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>

//--------------------------------------------------------------------------------------------------
/**
 * Local port for socket
 */
//--------------------------------------------------------------------------------------------------
const char* localPortPtr = "56830";

//--------------------------------------------------------------------------------------------------
/**
 * Socket configuration
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_SocketConfig_t LinuxSocketConfig;

//--------------------------------------------------------------------------------------------------
/**
 * Create a socket
 *
 * @return
 *      - socket id on success
 *      - -1 on error
 *
 */
//--------------------------------------------------------------------------------------------------
static int CreateSocket
(
    const char*                 portStr,    ///< [IN] Port
    lwm2mcore_SocketConfig_t    config      ///< [IN] Socket configuration
)
{
    int s = -1;
    struct addrinfo hints;
    struct addrinfo *res;
    struct addrinfo *p;

    memset(&hints, 0, sizeof hints);

    hints.ai_family = config.af;
    hints.ai_socktype = config.proto;
    hints.ai_flags = AI_PASSIVE;
    if (0 != getaddrinfo(NULL, portStr, &hints, &res))
    {
        return -1;
    }

    for (p = res ; (NULL != p) && (s == -1) ; p = p->ai_next)
    {
        s = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (s == -1)
            continue;

        if (bind(s, p->ai_addr, p->ai_addrlen) == 0)
            //Success
            break;

        close(s);
    }

    if (p == NULL)
    {
        printf("Could not bind\n");
    }

    freeaddrinfo(res);
    return s;
}

//--------------------------------------------------------------------------------------------------
/**
 * Open a socket to the server
 * This function is called by the LwM2MCore and must be adapted to the platform
 * The aim of this function is to create a socket and fill the configPtr structure
 *
 * @return
 *      - true on success
 *      - false on error
 *
 */
//--------------------------------------------------------------------------------------------------
bool lwm2mcore_UdpOpen
(
    lwm2mcore_Ref_t instanceRef,            ///< [IN] LWM2M instance reference
    lwm2mcore_UdpCb_t callback,             ///< [IN] callback for data receipt
    lwm2mcore_SocketConfig_t* configPtr     ///< [INOUT] socket configuration
)
{
    bool result = false;
    (void)callback;

    // IP v4
    configPtr->instanceRef = instanceRef;
    configPtr->af = AF_INET;
    configPtr->type = LWM2MCORE_SOCK_DGRAM;

    LinuxSocketConfig.type = LWM2MCORE_SOCK_DGRAM;
    LinuxSocketConfig.proto = LWM2MCORE_SOCK_UDP;
    configPtr->sock = CreateSocket (localPortPtr, LinuxSocketConfig);
    LinuxSocketConfig.sock = configPtr->sock;
    LinuxSocketConfig.instanceRef = instanceRef;

    if (LinuxSocketConfig.sock < 0)
    {
        printf("Failed to open socket: %d %s\n", errno, strerror(errno));
    }
    else
    {
        result = true;
    }

    printf("os_udpOpen %d\n", result);
    return result;
}


//--------------------------------------------------------------------------------------------------
/**
 * Close the socket
 * This function is called by the LwM2MCore and must be adapted to the platform
 * The aim of this function is to close a socket
 *
 * @return
 *      - true on success
 *      - false on error
 *
 */
//--------------------------------------------------------------------------------------------------
bool lwm2mcore_UdpClose
(
    lwm2mcore_SocketConfig_t config        ///< [INOUT] socket configuration
)
{
    close(config.sock);
    return true;
}


//--------------------------------------------------------------------------------------------------
/**
 * Send data on a socket
 * This function is called by the LwM2MCore and must be adapted to the platform
 * The aim of this function is to send data on a socket
 *
 * @return
 *      -
 *      - false on error
 *
 */
//--------------------------------------------------------------------------------------------------
ssize_t lwm2mcore_UdpSend
(
    int sockfd,
    const void *bufferPtr,
    size_t length,
    int flags,
    const struct sockaddr *dest_addrPtr,
    socklen_t addrlen
)
{
    ssize_t sentSize = 0;
    size_t offset = 0;

    printf ("enter sockfd %d, length %zu\n", sockfd, length);
    lwm2mcore_DataDump( "send data", (void*)bufferPtr, length);
    while (offset != length)
    {
        sentSize = sendto(sockfd,
                          (const void *)((uint8_t*)bufferPtr + offset),
                          length - offset,
                          flags,
                          dest_addrPtr,
                          addrlen);

        if (sentSize == -1)
        {
            perror("socket");
            printf("Error sending: %i\n",errno);
            printf("%s\n", strerror(errno));

            return -1;
        }
        offset += sentSize;
    }
    printf("lwm2mcore_UdpSend sentSize %zu\n", sentSize);
    return sentSize;
}

//--------------------------------------------------------------------------------------------------
/**
 * Connect a socket
 * This function is called by the LwM2MCore and must be adapted to the platform
 * The aim of this function is to send data on a socket
 *
 * @return
 *      - true  on success
 *      - false on error
 *
 */
//--------------------------------------------------------------------------------------------------
bool lwm2mcore_UdpConnect
(
    char* serverAddressPtr,             ///< [IN] Server address URL
    char* hostPtr,                      ///< [IN] Host
    char* portPtr,                      ///< [IN] Port
    int addressFamily,                  ///< [IN] Address familly
    struct sockaddr* saPtr,             ///< [IN] Socket address pointer
    socklen_t* slPtr,                   ///< [IN] Socket address length
    int* sockPtr                        ///< [IN] socket file descriptor
)
{
    struct addrinfo hints;
    struct addrinfo* servinfoPtr = NULL;
    struct addrinfo* p;
    int sockfd;
    (void)serverAddressPtr;

    // Connect
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = addressFamily;
    hints.ai_socktype = SOCK_DGRAM;

    if ((0 != getaddrinfo(hostPtr, portPtr, &hints, &servinfoPtr)) || (servinfoPtr == NULL))
    {
        return false;
    }

    sockfd = -1;
    for (p = servinfoPtr; (p != NULL) && (sockfd == -1); p = p->ai_next)
    {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);

        if (sockfd >= 0)
        {
            *slPtr = p->ai_addrlen;
            memcpy(saPtr, p->ai_addr, p->ai_addrlen);

            if (-1 == connect(sockfd, p->ai_addr, p->ai_addrlen))
            {
                close(sockfd);
                sockfd = -1;
            }
        }
    }
    *sockPtr = sockfd;
    if (NULL != servinfoPtr)
    {
        freeaddrinfo(servinfoPtr);
    }
    return true;
}
