/**
 * @file osPortSecurity.h
 *
 * Porting layer for credential management and package security (CRC, signature)
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#ifndef __OS_PORTSECURITY_H__
#define __OS_PORTSECURITY_H__

#include "lwm2mcore.h"

//--------------------------------------------------------------------------------------------------
/**
 * Define for server address maximum length
 */
//--------------------------------------------------------------------------------------------------
#define OS_PORT_SERVERADDR_LEN 256

//--------------------------------------------------------------------------------------------------
/**
 * Define value for PSK ID maximum length
 */
//--------------------------------------------------------------------------------------------------
#define OS_PORT_PSKID_LEN 32

//--------------------------------------------------------------------------------------------------
/**
 * Define value for PSK secret maximum length
 */
//--------------------------------------------------------------------------------------------------
#define OS_PORT_PSK_LEN 16

//--------------------------------------------------------------------------------------------------
/**
 * Define value for public key maximum length
 */
//--------------------------------------------------------------------------------------------------
#define OS_PORT_PUBLICKEY_LEN   1024

//--------------------------------------------------------------------------------------------------
/**
 * Maximal length of the security error string
 */
//--------------------------------------------------------------------------------------------------
#define OS_PORT_ERROR_STR_MAX_LEN   128

//--------------------------------------------------------------------------------------------------
/**
 *                  OBJECT 0: SECURITY
 */
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Retrieve a credential
 * This API treatment needs to have a procedural treatment
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
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
    lwm2mcore_credentials_t credId,         ///< [IN] credential Id of credential to be retrieved
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
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
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
 * Package verification
 */
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Compute and update CRC32 with data buffer passed as an argument
 *
 * @return Updated CRC32
 */
//--------------------------------------------------------------------------------------------------
uint32_t os_portSecurityCrc32
(
    uint32_t crc,       ///< [IN] Current CRC32 value
    uint8_t* bufPtr,    ///< [IN] Data buffer to hash
    size_t   len        ///< [IN] Data buffer length
);

//--------------------------------------------------------------------------------------------------
/**
 * Initialize the SHA1 computation
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_sid_t os_portSecuritySha1Start
(
    void** sha1CtxPtr   ///< [INOUT] SHA1 context pointer
);

//--------------------------------------------------------------------------------------------------
/**
 * Compute and update SHA1 digest with the data buffer passed as an argument
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_sid_t os_portSecuritySha1Process
(
    void*    sha1CtxPtr,    ///< [IN] SHA1 context pointer
    uint8_t* bufPtr,        ///< [IN] Data buffer to hash
    size_t   len            ///< [IN] Data buffer length
);

//--------------------------------------------------------------------------------------------------
/**
 * Finalize SHA1 digest and verify the package signature
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_sid_t os_portSecuritySha1End
(
    void* sha1CtxPtr,                   ///< [IN] SHA1 context pointer
    lwm2mcore_PkgDwlType_t packageType, ///< [IN] Package type (FW or SW)
    uint8_t* signaturePtr,              ///< [IN] Package signature used for verification
    size_t signatureLen                 ///< [IN] Package signature length
);

//--------------------------------------------------------------------------------------------------
/**
 * Cancel and reset the SHA1 computation
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_sid_t os_portSecuritySha1Cancel
(
    void** sha1CtxPtr   ///< [INOUT] SHA1 context pointer
);

//--------------------------------------------------------------------------------------------------
/**
 * Convert a DER key to PEM key
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the conversion succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the conversion fails
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_sid_t os_portSecurityConvertDERToPEM
(
    unsigned char*  derKeyPtr,      ///< [IN]       DER key
    int             derKeyLen,      ///< [IN]       DER key length
    unsigned char*  pemKeyPtr,      ///< [OUT]      PEM key
    int*            pemKeyLenPtr    ///< [IN/OUT]   PEM key length
);

#endif /* __OS_PORTSECURITY_H__ */
