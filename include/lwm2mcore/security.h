/**
 * @file security.h
 *
 * Porting layer for credential management and package security (CRC, signature)
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#ifndef __LWM2MCORE_SECURITY_H__
#define __LWM2MCORE_SECURITY_H__

#include <lwm2mcore/lwm2mcore.h>

/**
  * @addtogroup lwm2mcore_security_IFS
  * @{
  */
//--------------------------------------------------------------------------------------------------
/**
 * @brief Define for server address maximum length (including the terminating null byte (@c '\0'))
 */
//--------------------------------------------------------------------------------------------------
#define LWM2MCORE_SERVERADDR_LEN            256

//--------------------------------------------------------------------------------------------------
/**
 * @brief Define value for PSK ID maximum length
 */
//--------------------------------------------------------------------------------------------------
#define LWM2MCORE_PSKID_LEN                 32

//--------------------------------------------------------------------------------------------------
/**
 * @brief Define value for PSK secret maximum length
 */
//--------------------------------------------------------------------------------------------------
#define LWM2MCORE_PSK_LEN                   16

//--------------------------------------------------------------------------------------------------
/**
 * @brief Define value for public key maximum length
 */
//--------------------------------------------------------------------------------------------------
#define LWM2MCORE_PUBLICKEY_LEN             1024

//--------------------------------------------------------------------------------------------------
/**
 * @brief Maximal length of the security error string
 */
//--------------------------------------------------------------------------------------------------
#define LWM2MCORE_ERROR_STR_MAX_LEN         128

//--------------------------------------------------------------------------------------------------
/**
 * @brief This define value is used in @ref lwm2mcore_GetCredential, @ref lwm2mcore_SetCredential,
 * @ref lwm2mcore_CheckCredential APIs to indicate that the credential is linked to the bootstrap
 * server.
 */
//--------------------------------------------------------------------------------------------------
#define LWM2MCORE_BS_SERVER_ID              0

//--------------------------------------------------------------------------------------------------
/**
 * @brief This define value is used in @ref lwm2mcore_GetCredential, @ref lwm2mcore_SetCredential,
 * @ref lwm2mcore_CheckCredential APIs to indicate that the credential is not linked to server.
 */
//--------------------------------------------------------------------------------------------------
#define LWM2MCORE_NO_SERVER_ID              0

//--------------------------------------------------------------------------------------------------
/**
 *                  OBJECT 0: SECURITY
 */
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * @brief Retrieve a credential.
 * This API treatment needs to have a procedural treatment.
 *
 * @details Since the LwM2MCore is able to connect to several Device Management servers, the
 * Device Management server Id is indicated in parameters (this field is useless if the credential
 * Id concerns a bootstrap server credential).
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters is incorrect
 *  - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *  - @ref LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *  - @ref LWM2MCORE_ERR_OVERFLOW in case of buffer overflow
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetCredential
(
    lwm2mcore_Credentials_t credId,     ///< [IN] credential Id of credential to be retrieved
    uint16_t                serverId,   ///< [IN] server Id
    char*                   bufferPtr,  ///< [INOUT] data buffer
    size_t*                 lenPtr      ///< [INOUT] length of input buffer and length of the
                                        ///< returned data
);

//--------------------------------------------------------------------------------------------------
/**
 * Set a credential
 * This API treatment needs to have a procedural treatment
 *
 * @details Since the LwM2MCore is able to connect to several Device Management servers, the
 * Device Management server Id is indicated in parameters (this field is useless if the credential
 * Id concerns a bootstrap server credential).
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters is incorrect
 *  - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *  - @ref LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_SetCredential
(
    lwm2mcore_Credentials_t credId,     ///< [IN] credential Id of credential to be set
    uint16_t                serverId,   ///< [IN] server Id
    char*                   bufferPtr,  ///< [INOUT] data buffer
    size_t                  len         ///< [IN] length of input buffer
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Get the status of credentials provisioned on platform.
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_NO_CREDENTIAL_PROVISIONED if neither Bootstrap nor Device Management
 *                                              credentials are provisioned.
 *  - @ref LWM2MCORE_BS_CREDENTIAL_PROVISIONED if Bootstrap credentials are provisioned but Device
 *                                              Management credentials are not provisioned.
 *  - @ref LWM2MCORE_DM_CREDENTIAL_PROVISIONED if Device Management credentials are provisioned.
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_CredentialStatus_t lwm2mcore_GetCredentialStatus
(
    void
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to check if one credential is present in platform non-volatile storage.
 *
 * @details Since the LwM2MCore is able to connect to several Device Management servers, the
 * Device Management server Id is indicated in parameters (this field is useless if the credential
 * Id concerns a bootstrap server credential).
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @c true if the credential is present
 *  - @c false else
 *
 */
//--------------------------------------------------------------------------------------------------
bool lwm2mcore_CheckCredential
(
    lwm2mcore_Credentials_t credId,     ///< [IN] Credential identifier
    uint16_t                serverId    ///< [IN] server Id
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to check if one credential is present in platform non-volatile storage and
 * matches with our credentials.
 *
 * @details Since the LwM2MCore is able to connect to several Device Management servers, the
 * Device Management server Id is indicated in parameters (this field is useless if the credential
 * Id concerns a bootstrap server credential).
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *      - true if the credential and matches with our credentials
 *      - false else
 *
 */
//--------------------------------------------------------------------------------------------------
bool lwm2mcore_CredentialMatch
(
    lwm2mcore_Credentials_t credId,     ///< [IN] Credential identifier
    uint16_t                serverId,   ///< [IN] server Id
    const char*             credential, ///< [IN] Credential
    size_t                  credLen     ///< [IN] Credential length
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief This function erases one credential from platform non-volatile storage.
 *
 * @details Since the LwM2MCore is able to connect to several Device Management servers, the
 * Device Management server Id is indicated in parameters (this field is useless if the credential
 * Id concerns a bootstrap server credential).
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @c true if the credential is deleted
 *  - @c false else
 */
//--------------------------------------------------------------------------------------------------
bool lwm2mcore_DeleteCredential
(
    lwm2mcore_Credentials_t credId,     ///< [IN] Credential identifier
    uint16_t                serverId    ///< [IN] server Id
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Backup a credential.
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_BackupCredential
(
    lwm2mcore_Credentials_t credId,     ///< [IN] credential Id of credential to be retrieved
    uint16_t                serverId    ///< [IN] server Id
);

//--------------------------------------------------------------------------------------------------
/**
 * Package verification
 */
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * @brief Compute and update CRC32 with data buffer passed as an argument.
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return Updated CRC32
 */
//--------------------------------------------------------------------------------------------------
uint32_t lwm2mcore_Crc32
(
    uint32_t crc,       ///< [IN] Current CRC32 value
    uint8_t* bufPtr,    ///< [IN] Data buffer to hash
    size_t   len        ///< [IN] Data buffer length
);

//--------------------------------------------------------------------------------------------------
/**
 * Perform base64 data encoding.
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid
 *      - LWM2MCORE_ERR_OVERFLOW if buffer overflow occurs
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_Base64Encode
(
    const uint8_t*  src,    ///< [IN] Data to be encoded
    size_t          srcLen, ///< [IN] Data length
    char*           dst,    ///< [OUT] Base64-encoded string buffer
    size_t*         dstLen  ///< [INOUT] Length of the base64-encoded string buffer
);

//--------------------------------------------------------------------------------------------------
/**
 * Decode base64-encoded data.
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid
 *      - LWM2MCORE_ERR_OVERFLOW if buffer overflow occurs
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if incorrect data range
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_Base64Decode
(
    char*       src,    ///< [IN] Base64-encoded data string
    uint8_t*    dst,    ///< [OUT] Decoded data buffer
    size_t*     dstLen  ///< [INOUT] Decoded data buffer length
);

//--------------------------------------------------------------------------------------------------
/**
 * Compute HMAC SHA256 digest using the given data and credential.
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_ComputeHmacSHA256
(
    uint8_t*                data,       ///< [IN] Data buffer
    size_t                  dataLen,    ///< [IN] Data length
    lwm2mcore_Credentials_t credId,     ///< [IN] Key type
    uint8_t*                result,     ///< [OUT] Digest buffer
    size_t*                 resultLen   ///< [INOUT] Digest buffer length
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Initialize the SHA1 computation.
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_StartSha1
(
    void** sha1CtxPtr   ///< [INOUT] SHA1 context pointer
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Compute and update SHA1 digest with the data buffer passed as an argument.
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_ProcessSha1
(
    void*    sha1CtxPtr,    ///< [IN] SHA1 context pointer
    uint8_t* bufPtr,        ///< [IN] Data buffer to hash
    size_t   len            ///< [IN] Data buffer length
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Finalize SHA1 digest and verify the package signature.
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_EndSha1
(
    void* sha1CtxPtr,                   ///< [IN] SHA1 context pointer
    lwm2mcore_UpdateType_t packageType, ///< [IN] Package type (FW or SW)
    uint8_t* signaturePtr,              ///< [IN] Package signature used for verification
    size_t signatureLen                 ///< [IN] Package signature length
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Copy the SHA1 context in a buffer.
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_CopySha1
(
    void*  sha1CtxPtr,  ///< [IN] SHA1 context pointer
    void*  bufPtr,      ///< [INOUT] Buffer
    size_t bufSize      ///< [INOUT] Buffer length
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Restore the SHA1 context from a buffer.
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_RestoreSha1
(
    void*  bufPtr,      ///< [IN] Buffer
    size_t bufSize,     ///< [IN] Buffer length
    void** sha1CtxPtr   ///< [INOUT] SHA1 context pointer
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Cancel and reset the SHA1 computation.
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_CancelSha1
(
    void** sha1CtxPtr   ///< [INOUT] SHA1 context pointer
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Update SSL Certificate.
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @note To delete the saved certificate, set the length to 0
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the update succeeds
 *  - @ref LWM2MCORE_ERR_INCORRECT_RANGE if the size of the certificate is > 4000 bytes
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the update fails
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_UpdateSslCertificate
(
    char*  certPtr,    ///< [IN] Certificate
    size_t len         ///< [IN] Certificate len
);

/**
  * @}
  */
#endif /* __LWM2MCORE_SECURITY_H__ */
