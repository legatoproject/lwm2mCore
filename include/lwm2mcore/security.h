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

//--------------------------------------------------------------------------------------------------
/**
 * Define for server address maximum length
 */
//--------------------------------------------------------------------------------------------------
#define LWM2MCORE_SERVERADDR_LEN            256

//--------------------------------------------------------------------------------------------------
/**
 * Define value for PSK ID maximum length
 */
//--------------------------------------------------------------------------------------------------
#define LWM2MCORE_PSKID_LEN                 32

//--------------------------------------------------------------------------------------------------
/**
 * Define value for PSK secret maximum length
 */
//--------------------------------------------------------------------------------------------------
#define LWM2MCORE_PSK_LEN                   16

//--------------------------------------------------------------------------------------------------
/**
 * Define value for public key maximum length
 */
//--------------------------------------------------------------------------------------------------
#define LWM2MCORE_PUBLICKEY_LEN             1024

//--------------------------------------------------------------------------------------------------
/**
 * Maximal length of the security error string
 */
//--------------------------------------------------------------------------------------------------
#define LWM2MCORE_ERROR_STR_MAX_LEN         128

//--------------------------------------------------------------------------------------------------
/**
 * This define value is used in lwm2mcore_GetCredential, lwm2mcore_SetCredential,
 * lwm2mcore_CheckCredential APIs to indicate that the credential is linked to the bootstrap server.
 */
//--------------------------------------------------------------------------------------------------
#define LWM2MCORE_BS_SERVER_ID              0

//--------------------------------------------------------------------------------------------------
/**
 * This define value is used in lwm2mcore_GetCredential, lwm2mcore_SetCredential,
 * lwm2mcore_CheckCredential APIs to indicate that the credential is not linked to server.
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
 * Retrieve a credential.
 * This API treatment needs to have a procedural treatment.
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
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
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
 * Get the status of credentials provisioned on the device
 *
 * @return
 *     LWM2MCORE_NO_CREDENTIAL_PROVISIONED
 *          - If neither Bootstrap nor Device Management credential is provisioned.
 *     LWM2MCORE_BS_CREDENTIAL_PROVISIONED
 *          - If Bootstrap credential is provisioned but Device Management credential is
              not provisioned.
 *     LWM2MCORE_DM_CREDENTIAL_PROVISIONED
 *          - If Device Management credential is provisioned.
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_CredentialStatus_t lwm2mcore_GetCredentialStatus
(
    void
);

//--------------------------------------------------------------------------------------------------
/**
 * Function to check if one credential is present in platform storage
 *
 * @return
 *      - true if the credential is present
 *      - false else
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
 * This function erases one credential from platform storage
 *
 * @return
 *      - true if the credential is deleted
 *      - false else
 */
//--------------------------------------------------------------------------------------------------
bool lwm2mcore_DeleteCredential
(
    lwm2mcore_Credentials_t credId,     ///< [IN] Credential identifier
    uint16_t                serverId    ///< [IN] server Id
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
uint32_t lwm2mcore_Crc32
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
lwm2mcore_Sid_t lwm2mcore_StartSha1
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
lwm2mcore_Sid_t lwm2mcore_ProcessSha1
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
lwm2mcore_Sid_t lwm2mcore_EndSha1
(
    void* sha1CtxPtr,                   ///< [IN] SHA1 context pointer
    lwm2mcore_PkgDwlType_t packageType, ///< [IN] Package type (FW or SW)
    uint8_t* signaturePtr,              ///< [IN] Package signature used for verification
    size_t signatureLen                 ///< [IN] Package signature length
);

//--------------------------------------------------------------------------------------------------
/**
 * Copy the SHA1 context in a buffer
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid
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
 * Restore the SHA1 context from a buffer
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid
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
 * Cancel and reset the SHA1 computation
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_CancelSha1
(
    void** sha1CtxPtr   ///< [INOUT] SHA1 context pointer
);

//--------------------------------------------------------------------------------------------------
/**
 * Update SSL Certificate
 *
 * @note To delete the saved certificate, set the length to 0
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the update succeeds
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the size of the certificate is > 4000 bytes
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the update fails
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_UpdateSslCertificate
(
    char*  certPtr,    ///< [IN] Certificate
    int    len         ///< [IN] Certificate len
);
#endif /* __LWM2MCORE_SECURITY_H__ */
