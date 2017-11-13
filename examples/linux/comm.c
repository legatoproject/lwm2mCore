/**
 * @file comm.c
 *
 * Adaptation layer for UDP and CoAP errors reporting
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#include <stdio.h>
#include <lwm2mcore/lwm2mcore.h>
#include <lwm2mcore/udp.h>
#include <liblwm2m.h>

//--------------------------------------------------------------------------------------------------
/**
 * Get CoAP response class
 */
//--------------------------------------------------------------------------------------------------
#define CLASS(code)             (code >> 5)

//--------------------------------------------------------------------------------------------------
/**
 * Get CoAP response details
 */
//--------------------------------------------------------------------------------------------------
#define DETAILS(code)           (code & 0x1f)

//--------------------------------------------------------------------------------------------------
/**
 * Convert an argument into a constant string
 */
//--------------------------------------------------------------------------------------------------
#define STRINGIFY(x)            #x

//--------------------------------------------------------------------------------------------------
/**
 * String maximum length
 */
//--------------------------------------------------------------------------------------------------
#define COMM_INFO_STR_MAX_LEN   255

//--------------------------------------------------------------------------------------------------
/**
 * Communication info struct definition
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    uint8_t     code;                            ///< Error code identifier
    char        str[COMM_INFO_STR_MAX_LEN+1];    ///< Error code message
}
CommInfo_t;

//--------------------------------------------------------------------------------------------------
/**
 * Convert a UDP error to string
 */
//--------------------------------------------------------------------------------------------------
static const char* UdpErrorToStr
(
    int code
)
{
    const char* str;

    switch (code)
    {
        case LWM2MCORE_UDP_NO_ERR:
            str = STRINGIFY(LWM2MCORE_UDP_NO_ERR);
            break;
        case LWM2MCORE_UDP_OPEN_ERR:
            str = STRINGIFY(LWM2MCORE_UDP_OPEN_ERR);
            break;
        case LWM2MCORE_UDP_CLOSE_ERR:
            str = STRINGIFY(LWM2MCORE_UDP_CLOSE_ERR);
            break;
        case LWM2MCORE_UDP_SEND_ERR:
            str = STRINGIFY(LWM2MCORE_UDP_SEND_ERR);
            break;
        case LWM2MCORE_UDP_RECV_ERR:
            str = STRINGIFY(LWM2MCORE_UDP_RECV_ERR);
            break;
        case LWM2MCORE_UDP_CONNECT_ERR:
            str = STRINGIFY(LWM2MCORE_UDP_CONNECT_ERR);
            break;
        default:
            str = "";
    }

    return str;
};

//--------------------------------------------------------------------------------------------------
/**
 * Function to report UDP error code
 */
//--------------------------------------------------------------------------------------------------
void lwm2mcore_ReportUdpErrorCode
(
    int code  ///< [IN] UDP custom error code defined in udp.h
)
{
    CommInfo_t info;

    info.code = (uint8_t)code;
    snprintf(info.str, COMM_INFO_STR_MAX_LEN, "%s", UdpErrorToStr(code));

    lwm2m_printf("UDP err is %d: %s\n", code, UdpErrorToStr(code));
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to report CoAP response code
 */
//--------------------------------------------------------------------------------------------------
void lwm2mcore_ReportCoapResponseCode
(
    int code  ///< [IN] CoAP error code as defined in RFC 7252 section 12.1.2
)
{
    CommInfo_t info;

    info.code = (uint8_t)code;
    snprintf(info.str, COMM_INFO_STR_MAX_LEN, "CoAP %d.%.2d", CLASS(code), DETAILS(code));

    lwm2m_printf("Received response code %d: %s\n", info.code, info.str);
}
