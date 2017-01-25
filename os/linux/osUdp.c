/**
 * @file osUdp.c
 *
 * Adaptation layer for os socket
 *
 * <hr>
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#include <stdbool.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "osUdp.h"
#include "osDebug.h"
#include "lwm2mcore.h"

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
    os_socketConfig_t* configPtr    ///< [INOUT] socket configuration
)
{
    bool result = false;

    return result;
}


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
)
{
    bool result = false;
    return result;
}


//--------------------------------------------------------------------------------------------------
/**
 * Send data on a socket
 * This function is called by the LWM2MCore and must be adapted to the platform
 * The aim of this function is to send data on a socket
 *
 * @return
 *      -
 *      - false on error
 *
 */
//--------------------------------------------------------------------------------------------------
ssize_t os_udpSend
(
    int sockfd,
    const void *bufferPtr,
    size_t length,
    int flags,
    const struct sockaddr *dest_addrPtr,
    socklen_t addrlen
)
{
    return 0;
}

