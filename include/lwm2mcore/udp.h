/**
 * @file udp.h
 *
 * Header file for adaptation layer for UDP socket management
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#ifndef __LWM2MCORE_UDP_H__
#define __LWM2MCORE_UDP_H__

#include <lwm2mcore/socket.h>
#include <platform/inet.h>

//--------------------------------------------------------------------------------------------------
/**
 *  Maximum size of packet that can be received on UDP socket.
 */
//--------------------------------------------------------------------------------------------------
#define LWM2MCORE_UDP_MAX_PACKET_SIZE 1024

//--------------------------------------------------------------------------------------------------
/**
 * Callback for data receipt
 */
//--------------------------------------------------------------------------------------------------
typedef void* (*lwm2mcore_UdpCb_t)
(
    uint8_t* bufferPtr,                     ///< [IN] Received data
    uint32_t len,                           ///< [IN] Received data length
    struct sockaddr_storage* addrPtr,       ///< [INOUT] source address
    socklen_t addrlen,                      ///< @TODO
    lwm2mcore_SocketConfig_t config         ///< [IN] Socket config
);

//--------------------------------------------------------------------------------------------------
/**
 * Callback for data receipt
 * This function is defined in LWM2MCore
 */
//--------------------------------------------------------------------------------------------------
void lwm2mcore_UdpReceiveCb
(
    uint8_t* bufferPtr,                     ///< [IN] Received data
    uint32_t len,                           ///< [IN] Received data length
    struct sockaddr_storage* addrPtr,       ///< [INOUT] source address
    socklen_t addrlen,                      ///< @TODO
    lwm2mcore_SocketConfig_t config         ///< [IN] Socket config
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
bool lwm2mcore_UdpOpen
(
    int context,                        ///< [IN] LWM2M context
    lwm2mcore_UdpCb_t callback,         ///< [IN] callback for data receipt
    lwm2mcore_SocketConfig_t* config    ///< [INOUT] socket configuration
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
bool lwm2mcore_UdpClose
(
    lwm2mcore_SocketConfig_t config        ///< [INOUT] socket configuration
);

#endif /* __LWM2MCORE_UDP_H__ */
