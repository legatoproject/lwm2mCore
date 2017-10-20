/**
 * @file clientConfig.h
 *
 * Header file for client-dependent configuration
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#ifndef _CLIENTCONFIG_H_
#define _CLIENTCONFIG_H_

#include <lwm2mcore/security.h>

//--------------------------------------------------------------------------------------------------
/**
 * Define the maximal number of PDP contexts
 */
//--------------------------------------------------------------------------------------------------
#define MAX_PDP_CONTEXTS                            4

//--------------------------------------------------------------------------------------------------
/**
 * Define the maximal number of available network bearers (resource 1 of object 4)
 */
//--------------------------------------------------------------------------------------------------
#define CONN_MONITOR_AVAIL_NETWORK_BEARER_MAX_NB    20

//--------------------------------------------------------------------------------------------------
/**
 * Define the maximal number of IP addresses associated to the device:
 * one IPv4 and one IPv6 for each PDP context (resource 4 of object 4)
 */
//--------------------------------------------------------------------------------------------------
#define CONN_MONITOR_IP_ADDRESSES_MAX_NB            (2 * MAX_PDP_CONTEXTS)

//--------------------------------------------------------------------------------------------------
/**
 * Define the maximal number of router IP addresses associated to the device:
 * one IPv4 and one IPv6 for each PDP context (resource 5 of object 4)
 */
//--------------------------------------------------------------------------------------------------
#define CONN_MONITOR_ROUTER_IP_ADDRESSES_MAX_NB     (2 * MAX_PDP_CONTEXTS)

//--------------------------------------------------------------------------------------------------
/**
 * Define the maximal bytes number in an IP address (IPv4 and IPv6), including the null-terminator
 */
//--------------------------------------------------------------------------------------------------
#define CONN_MONITOR_IP_ADDR_MAX_BYTES              46

//--------------------------------------------------------------------------------------------------
/**
 * Define the maximal number of APN, one per PDP context
 * (resource 7 of object 4)
 */
//--------------------------------------------------------------------------------------------------
#define CONN_MONITOR_APN_MAX_NB                     MAX_PDP_CONTEXTS

//--------------------------------------------------------------------------------------------------
/**
 * Define the maximal bytes number in an APN, including the null-terminator
 */
//--------------------------------------------------------------------------------------------------
#define CONN_MONITOR_APN_MAX_BYTES                  101

//--------------------------------------------------------------------------------------------------
/**
 * Client configuration file: General section name
 */
//--------------------------------------------------------------------------------------------------
#define CLIENT_CONFIG_GENERAL_SECTION_NAME          "GENERAL"

//--------------------------------------------------------------------------------------------------
/**
 * Client configuration file: Bootstrap server section name
 */
//--------------------------------------------------------------------------------------------------
#define CLIENT_CONFIG_BS_SERVER_SECTION_NAME        "BOOTSTRAP SECURITY"

//--------------------------------------------------------------------------------------------------
/**
 * Client configuration file: Device Management server section name
 */
//--------------------------------------------------------------------------------------------------
#define CLIENT_CONFIG_DM_SERVER_SECTION_NAME        "LWM2M SECURITY"

//--------------------------------------------------------------------------------------------------
/**
 * Client configuration file: Endpoint parameter name
 */
//--------------------------------------------------------------------------------------------------
#define CLIENT_CONFIG_ENDPOINT                      "ENDPOINT"

//--------------------------------------------------------------------------------------------------
/**
 * Client configuration file: Serial number parameter name
 */
//--------------------------------------------------------------------------------------------------
#define CLIENT_CONFIG_SERIAL_NUMBER                 "SN"

//--------------------------------------------------------------------------------------------------
/**
 * Client configuration file: Server URL parameter name
 */
//--------------------------------------------------------------------------------------------------
#define CLIENT_CONFIG_SERVER_URL                    "SERVER URI"

//--------------------------------------------------------------------------------------------------
/**
 * Client configuration file: PSK identity parameter name
 */
//--------------------------------------------------------------------------------------------------
#define CLIENT_CONFIG_SERVER_PSKID                  "DEVICE PKID"

//--------------------------------------------------------------------------------------------------
/**
 * Client configuration file: PSK secret parameter name
 */
//--------------------------------------------------------------------------------------------------
#define CLIENT_CONFIG_SERVER_PSK                    "SECRET KEY"

//--------------------------------------------------------------------------------------------------
/**
 * Structure for client general configuration
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    char IMEI[LWM2MCORE_ENDPOINT_LEN];  ///< Client Endpoint
    char SN[LWM2MCORE_NAME_LEN];        ///< Client serial number
}
clientGeneralConfig_t;

//--------------------------------------------------------------------------------------------------
/**
 * Structure for server configuration
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    char        serverURI[LWM2MCORE_SERVERADDR_LEN];    ///< Server URI
    char        devicePKID[LWM2MCORE_PSKID_LEN+1];      ///< PSK identity
    int         pkidLen;                                ///< PSK identity length
    uint8_t     secretKey[2*LWM2MCORE_PSK_LEN+1];       ///< PSK secret
    int         secretKeyLen;                           ///< PSK secret length
}
clientSecurityConfig_t;

//--------------------------------------------------------------------------------------------------
/**
 * Structure for LwM2MCore configuration
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    clientGeneralConfig_t general;                                      ///< General configuration
    clientSecurityConfig_t security[LWM2MCORE_BOOTSRAP_SERVER_MAX_COUNT +\
                                    LWM2MCORE_DM_SERVER_MAX_COUNT];     ///< Server configuration
}
clientConfig_t;

//--------------------------------------------------------------------------------------------------
/**
 * Read and parse configuration file into client configuration data structure.
 *
 * @return
 *  - 0 on succes and dereference of config points to configuration pointer
 *  - -1 on failure and dereference of config points to NULL
 */
//--------------------------------------------------------------------------------------------------
int clientConfigRead
(
    clientConfig_t** configPtr      ///< [IN] Client configuration structure
);

//--------------------------------------------------------------------------------------------------
/**
 * Write one line in the client configuration file
 *
 * @return
 *  - 0 on succes
 *  - -1 on failure
 */
//--------------------------------------------------------------------------------------------------
int clientConfigWriteOneLine
(
    const char* sectionPtr,     ///< [IN] Section name
    const char* namePtr,        ///< [IN] Parameter name
    char* valuePtr,             ///< [IN] New value
    clientConfig_t* configPtr   ///< [IN] Pointer on client configuration
);

//--------------------------------------------------------------------------------------------------
/**
 * Return client configuration pointer
 *
 * @return
 *  - Client configuration pointer
 *  - NULL on failure
 */
//--------------------------------------------------------------------------------------------------
clientConfig_t* clientConfigGet
(
    void
);

#endif /* _CLIENTCONFIG_H_ */

