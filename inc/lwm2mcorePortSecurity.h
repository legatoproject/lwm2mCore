/**
 * @file lwm2mcorePortSecurity.h
 *
 * Porting layer for credential management
 *
 * Copyright (C) Sierra Wireless Inc. Use of this work is subject to license.
 *
 */

#ifndef LWM2MCORE_PORTSECURITY_H_
#define LWM2MCORE_PORTSECURITY_H_

#include "lwm2mcore.h"
#include "lwm2mcoreObjectHandler.h"

//--------------------------------------------------------------------------------------------------
/**
 *                  OBJECT 0: SECURITY
 */
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Define value for PSK ID maximum length
 */
//--------------------------------------------------------------------------------------------------
#define LWM2MCORE_PSKID_LEN 32

//--------------------------------------------------------------------------------------------------
/**
 * Define value for PSK secret maximum length
 */
//--------------------------------------------------------------------------------------------------
#define LWM2MCORE_PSK_LEN 16


//--------------------------------------------------------------------------------------------------
/**
 * Retrieve a credential
 * This API treatment needs to have a procedural treatment
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treament succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_sid_t os_portSecurityGetCredential
(
    lwm2mcore_credentials_t credId,         ///< [IN] credential Id of credential to be retreived
    char *bufferPtr,                        ///< [INOUT] data buffer
    size_t *lenPtr                          ///< [INOUT] length of input buffer and length of the
                                            ///< returned data
);

//--------------------------------------------------------------------------------------------------
/**
 * Set a credential
 * This API treatment needs to have a procedural treatment
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treament succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_sid_t os_portSecuritySetCredential
(
    lwm2mcore_credentials_t credId,         ///< [IN] credential Id of credential to be set
    char *bufferPtr,                        ///< [INOUT] data buffer
    size_t len                              ///< [IN] length of input buffer and length of the
                                            ///< returned data
);

//--------------------------------------------------------------------------------------------------
/**
 * Function to check if all Device Management credentials were provisionned
 *
 * @note This API is called by LWM2MCore
 *
 * @return
 *      - true if  a Device Management server was provided
 *      - false else
 */
//--------------------------------------------------------------------------------------------------
bool os_portSecurityCheckDmCredentialsPresence
(
    void
);

#endif /* LWM2MCORE_PORTSECURITY_H_ */

