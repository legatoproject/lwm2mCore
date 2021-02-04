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

/**
  * @addtogroup lwm2mcore_platform_adaptor_udp_IFS
  * @{
  */

//--------------------------------------------------------------------------------------------------
/**
 * @brief Maximum size of packet that can be received on UDP socket.
 *
 * @details The maximum size is calculated based
 * on 1024 bytes of CoAP payload, DTLS header, CoAP Header and CoAP options. Note that the CoAP
 * overhead can vary based on the CoAP options in the received message. For instance the CoAP URI
 * can be a maximum of 256 bytes. Hence it is better to use the MTU size of ethernet.
 */
//--------------------------------------------------------------------------------------------------
#define LWM2MCORE_UDP_MAX_PACKET_SIZE   1500

//--------------------------------------------------------------------------------------------------
/**
 *  UDP error codes
 */
//--------------------------------------------------------------------------------------------------
#define LWM2MCORE_UDP_NO_ERR            0x00    ///< UDP error codes: No error
#define LWM2MCORE_UDP_OPEN_ERR          0x01    ///< UDP error codes: Failed to open UDP connection
#define LWM2MCORE_UDP_CLOSE_ERR         0x02    ///< UDP error codes: Failed to close UDP connection
#define LWM2MCORE_UDP_SEND_ERR          0x03    ///< UDP error codes: Error occured during UDP send
#define LWM2MCORE_UDP_RECV_ERR          0x04    ///< UDP error codes: Error occured during UDP receive
#define LWM2MCORE_UDP_CONNECT_ERR       0x05    ///< UDP error codes: UDP connection failure

//--------------------------------------------------------------------------------------------------
/**
 * @brief Callback for data receipt
 */
//--------------------------------------------------------------------------------------------------
typedef void (*lwm2mcore_UdpCb_t)
(
    uint8_t* bufferPtr,                     ///< [IN] Received data
    uint32_t len,                           ///< [IN] Received data length
    struct sockaddr_storage* addrPtr,       ///< [INOUT] source address
    socklen_t addrlen,                      ///< [IN] addrPtr parameter length
    lwm2mcore_SocketConfig_t config         ///< [IN] Socket config
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Callback for data receipt
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 */
//--------------------------------------------------------------------------------------------------
void lwm2mcore_UdpReceiveCb
(
    uint8_t* bufferPtr,                     ///< [IN] Received data
    uint32_t len,                           ///< [IN] Received data length
    struct sockaddr_storage* addrPtr,       ///< [INOUT] source address
    socklen_t addrlen,                      ///< [IN] addrPtr parameter length
    lwm2mcore_SocketConfig_t config         ///< [IN] Socket config
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Open a socket to the server
 *
 * @details The aim of this function is to create a socket and fill the @c configPtr structure
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @c true on success
 *  - @c false on error
 *
 */
//--------------------------------------------------------------------------------------------------
bool lwm2mcore_UdpOpen
(
    lwm2mcore_Ref_t instanceRef,        ///< [IN] LWM2M instance reference
    lwm2mcore_UdpCb_t callback,         ///< [IN] callback for data receipt
    lwm2mcore_SocketConfig_t* config    ///< [INOUT] socket configuration
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Close the socket
 *
 * @details The aim of this function is to close a socket
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @c true on success
 *  - @c false on error
 *
 */
//--------------------------------------------------------------------------------------------------
bool lwm2mcore_UdpClose
(
    lwm2mcore_SocketConfig_t config        ///< [INOUT] socket configuration
);

//--------------------------------------------------------------------------------------------------
/**
 * Connect to the server
 *
 * @details The aim of this function is to connect
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @c true on success
 *  - @c false on error
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
    int* sockPtr                        ///< [IN] Socket file descriptor
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Send data on a socket
 *
 * @details The aim of this function is to send data on a socket
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - Sent data length
 *  - @c -1 on error
 *
 */
//--------------------------------------------------------------------------------------------------
ssize_t lwm2mcore_UdpSend
(
    int sockfd,                            ///< [IN] Socket Id
    const void *bufferPtr,                 ///< [IN] Buffer to be sent
    size_t length,                         ///< [IN] Buffer length to be sent
    int flags,                             ///< [IN] Flags
    const struct sockaddr *destAddrPtr,    ///< [IN] Destination address
    socklen_t addrLen                      ///< [IN] destAddrPtr parameter length
);

/**
  * @}
  */

#endif /* __LWM2MCORE_UDP_H__ */
