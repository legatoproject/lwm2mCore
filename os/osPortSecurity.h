/**
 * @file osPortSecurity.h
 *
 * Header file for adaptation layer for credentials management *
 *
 * Copyright (C) Sierra Wireless Inc. Use of this work is subject to license.
 *
 */

#ifndef OS_PORTSECURITY_INCLUDE_GUARD
#define OS_PORTSECURITY_INCLUDE_GUARD

#include "lwm2mcore.h"

//--------------------------------------------------------------------------------------------------
/**
 * Define for server address maximum length
 */
//--------------------------------------------------------------------------------------------------
#define OS_PORT_SERVERADDRLEN 256

//--------------------------------------------------------------------------------------------------
/**
 * Define for PSK length TODO: could be 64
 */
//--------------------------------------------------------------------------------------------------
#define OS_PORT_PSKLEN 32

//--------------------------------------------------------------------------------------------------
/**
 * Get the security mode: PSK
 *
 * @return
 *      - true on success
 *      - false on error
 *
 */
//--------------------------------------------------------------------------------------------------
bool os_portSecurityMode
(
    uint16_t instanceId,    ///< [IN] Security object instance Id
    uint8_t* securityMode   ///< [INOUT] Security mode
);

//--------------------------------------------------------------------------------------------------
/**
 * Retrieve a credential length
 *
 * @return
 *      - credential length
 *
 */
//--------------------------------------------------------------------------------------------------
uint32_t os_portCredentialReadLen
(
    lwm2mcore_credentials_t credential     ///< [IN] credential to be read
);

//--------------------------------------------------------------------------------------------------
/**
 * Retrieve a credential
 *
 * @return
 *      - credential length
 *
 */
//--------------------------------------------------------------------------------------------------
uint32_t os_portCredentialRead
(
    lwm2mcore_credentials_t credential,     ///< [IN] credential to be read
    uint8_t* bufferPtr,                     ///< [INOUT] Buffer in which data is written
    uint32_t length                         ///< [IN] Buffer length
);

#endif /* OS_PORTSECURITY_INCLUDE_GUARD */

