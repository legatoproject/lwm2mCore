/**
 * @file secureDownload.c
 *
 * Porting layer for package security (CRC, signature)
 *
 * @note The CRC is computed using the crc32 function from zlib.
 * @note The signature verification uses the OpenSSL library.
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <platform/types.h>
#include <ctype.h>
#include <zlib.h>
#include <openssl/sha.h>
#include <openssl/bio.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/x509.h>
#include <lwm2mcore/lwm2mcore.h>
#include <lwm2mcore/security.h>
#include <sys/stat.h>
#include "clientConfig.h"
#include "handlers.h"
#include "crypto.h"

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
)
{
    return crc32(crc, bufPtr, len);
}

//--------------------------------------------------------------------------------------------------
/**
 * Print OpenSSL errors
 */
//--------------------------------------------------------------------------------------------------
static void PrintOpenSSLErrors
(
    void
)
{
    char errorString[LWM2MCORE_ERROR_STR_MAX_LEN];
    unsigned long error;

    // Retrieve the first error and remove it from the queue
    error = ERR_get_error();
    while (0 != error)
    {
        // Convert the error code to a human-readable string and print it
        ERR_error_string_n(error, errorString, sizeof(errorString));
        printf("%s\n", errorString);

        // Retrieve the next error and remove it from the queue
        error = ERR_get_error();
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Initialize the SHA1 computation
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_StartSha1
(
    void** sha1CtxPtr   ///< [INOUT] SHA1 context pointer
)
{
    static SHA_CTX shaCtx;

    // Check if SHA1 context pointer is set
    if (!sha1CtxPtr)
    {
        fprintf(stderr, "No SHA1 context pointer\n");
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    // Load the error strings
    ERR_load_crypto_strings();

    // Initialize the SHA1 context
    // SHA1_Init function returns 1 for success, 0 otherwise
    if (1 != SHA1_Init(&shaCtx))
    {
        fprintf(stderr, "SHA1_Init failed\n");
        PrintOpenSSLErrors();
        return LWM2MCORE_ERR_GENERAL_ERROR;
    }
    else
    {
        *sha1CtxPtr = (void*)&shaCtx;
        return LWM2MCORE_ERR_COMPLETED_OK;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Compute and update SHA1 digest with the data buffer passed as an argument
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_ProcessSha1
(
    void*    sha1CtxPtr,    ///< [IN] SHA1 context pointer
    uint8_t* bufPtr,        ///< [IN] Data buffer to hash
    size_t   len            ///< [IN] Data buffer length
)
{
    // Check if pointers are set
    if ((!sha1CtxPtr) || (!bufPtr))
    {
        fprintf(stderr, "NULL pointer provided\n");
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    // Update SHA1 digest
    // SHA1_Update function returns 1 for success, 0 otherwise
    if (1 != SHA1_Update((SHA_CTX*)sha1CtxPtr, bufPtr, len))
    {
        fprintf(stderr, "SHA1_Update failed\n");
        PrintOpenSSLErrors();
        return LWM2MCORE_ERR_GENERAL_ERROR;
    }

    return LWM2MCORE_ERR_COMPLETED_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Finalize SHA1 digest and verify the package signature
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_EndSha1
(
    void* sha1CtxPtr,                   ///< [IN] SHA1 context pointer
    lwm2mcore_UpdateType_t packageType, ///< [IN] Package type (FW or SW)
    uint8_t* signaturePtr,              ///< [IN] Package signature used for verification
    size_t signatureLen                 ///< [IN] Package signature length
)
{
    unsigned char sha1Digest[SHA_DIGEST_LENGTH];
    lwm2mcore_Credentials_t credId;
    char publicKey[LWM2MCORE_PUBLICKEY_LEN];
    size_t publicKeyLen = LWM2MCORE_PUBLICKEY_LEN;
    BIO* bufioPtr = NULL;
    RSA* rsaKeyPtr = NULL;
    EVP_PKEY* evpPkeyPtr = NULL;
    EVP_PKEY_CTX* evpPkeyCtxPtr = NULL;

    // Check if pointers are set
    if ((!sha1CtxPtr) || (!signaturePtr))
    {
        fprintf(stderr, "NULL pointer provided\n");
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    // Finalize SHA1 digest
    // SHA1_Final function returns 1 for success, 0 otherwise
    if (1 != SHA1_Final(sha1Digest, (SHA_CTX*)sha1CtxPtr))
    {
        fprintf(stderr, "SHA1_Final failed\n");
        PrintOpenSSLErrors();
        return LWM2MCORE_ERR_GENERAL_ERROR;
    }

    // The package type indicates the public key to use
    switch (packageType)
    {
        case LWM2MCORE_FW_UPDATE_TYPE:
            credId = LWM2MCORE_CREDENTIAL_FW_KEY;
            break;

        case LWM2MCORE_SW_UPDATE_TYPE:
            credId = LWM2MCORE_CREDENTIAL_SW_KEY;
            break;

#ifdef LWM2M_OBJECT_33406
        // ToDo: Do we need a new credential provisioned for File Stream?
        case LWM2MCORE_FILE_TRANSFER_TYPE:
            credId = LWM2MCORE_CREDENTIAL_SW_KEY;
            break;
#endif

        default:
            fprintf(stderr, "Unknown or unsupported package type %d\n", packageType);
            return LWM2MCORE_ERR_GENERAL_ERROR;
            break;
    }

    // Retrieve the public key corresponding to the package type
    if (LWM2MCORE_ERR_COMPLETED_OK != lwm2mcore_GetCredential(credId,
                                                              LWM2MCORE_BS_SERVER_ID,
                                                              publicKey,
                                                              &publicKeyLen))
    {
        fprintf(stderr, "Error while retrieving credentials %d\n", credId);
        return LWM2MCORE_ERR_GENERAL_ERROR;
    }

    // The public key is stored in PKCS #1 DER format, convert it to a RSA key.
    // Note that two formats are possible, try both of them if necessary:
    // - PEM DER ASN.1 PKCS#1 RSA Public key: ASN.1 type RSAPublicKey
    // - X.509 SubjectPublicKeyInfo: Object Identifier rsaEncryption added for AlgorithmIdentifier

    // First create the memory BIO containing the DER key
    bufioPtr = BIO_new_mem_buf((void*)publicKey, publicKeyLen);
    if (!bufioPtr)
    {
        fprintf(stderr, "Unable to create a memory BIO\n");
        PrintOpenSSLErrors();
        return LWM2MCORE_ERR_GENERAL_ERROR;
    }
    // Then convert it to a RSA key using PEM DER ASN.1 PKCS#1 RSA Public key format
    rsaKeyPtr = d2i_RSAPublicKey_bio(bufioPtr, NULL);
    if (!rsaKeyPtr)
    {
        // Memory BIO is modified by last function call, retrieve the DER key again
        BIO_free(bufioPtr);
        bufioPtr = BIO_new_mem_buf((void*)publicKey, publicKeyLen);
        if (!bufioPtr)
        {
            fprintf(stderr, "Unable to create a memory BIO\n");
            PrintOpenSSLErrors();
            return LWM2MCORE_ERR_GENERAL_ERROR;
        }

        // Then convert it to a RSA key using X.509 SubjectPublicKeyInfo format
        rsaKeyPtr = d2i_RSA_PUBKEY_bio(bufioPtr, NULL);
    }
    BIO_free(bufioPtr);
    if (!rsaKeyPtr)
    {
        fprintf(stderr, "Unable to retrieve public key\n");
        PrintOpenSSLErrors();
        return LWM2MCORE_ERR_GENERAL_ERROR;
    }
    evpPkeyPtr = EVP_PKEY_new();
    if (!evpPkeyPtr)
    {
        fprintf(stderr, "Unable to create EVP_PKEY structure\n");
        PrintOpenSSLErrors();
        return LWM2MCORE_ERR_GENERAL_ERROR;
    }
    // EVP_PKEY_assign_RSA returns 1 for success and 0 for failure
    if (1 != EVP_PKEY_assign_RSA(evpPkeyPtr, rsaKeyPtr))
    {
        fprintf(stderr, "Unable to assign public key\n");
        PrintOpenSSLErrors();
        return LWM2MCORE_ERR_GENERAL_ERROR;
    }

    // Create EVP public key context, necessary to verify the signature
    evpPkeyCtxPtr = EVP_PKEY_CTX_new(evpPkeyPtr, NULL);
    if (   (!evpPkeyCtxPtr)
        || (1 != EVP_PKEY_verify_init(evpPkeyCtxPtr))
       )
    {
        fprintf(stderr, "Unable to create and initialize EVP PKEY context\n");
        PrintOpenSSLErrors();
        return LWM2MCORE_ERR_GENERAL_ERROR;
    }

    // Set the signature verification options:
    // - RSA padding mode is PSS
    // - message digest type is SHA1
    // EVP_PKEY_CTX_ctrl functions return a positive value for success
    // and 0 or a negative value for failure
    if (   (EVP_PKEY_CTX_set_rsa_padding(evpPkeyCtxPtr, RSA_PKCS1_PSS_PADDING) <= 0)
        || (EVP_PKEY_CTX_set_signature_md(evpPkeyCtxPtr, EVP_sha1()) <= 0)
       )
    {
        fprintf(stderr, "Error during EVP PKEY context initialization\n");
        PrintOpenSSLErrors();
        return LWM2MCORE_ERR_GENERAL_ERROR;
    }

    // Verify signature
    // VP_PKEY_verify returns 1 if the verification was successful and 0 if it failed
    if (1 != EVP_PKEY_verify(evpPkeyCtxPtr,
                             signaturePtr,
                             signatureLen,
                             sha1Digest,
                             sizeof(sha1Digest)))
    {
        fprintf(stderr, "Signature verification failed\n");
        PrintOpenSSLErrors();
        return LWM2MCORE_ERR_GENERAL_ERROR;
    }

    return LWM2MCORE_ERR_COMPLETED_OK;
}

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
)
{
    // Check if pointers are set
    if ((!sha1CtxPtr) || (!bufPtr))
    {
        fprintf(stderr, "Null pointer provided\n");
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    // Check buffer length
    if (bufSize < sizeof(SHA_CTX))
    {
        fprintf(stderr, "Buffer is too short (%zu < %zu)\n", bufSize, sizeof(SHA_CTX));
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    // Copy the SHA1 context
    memset(bufPtr, 0, bufSize);
    memcpy(bufPtr, sha1CtxPtr, sizeof(SHA_CTX));
    return LWM2MCORE_ERR_COMPLETED_OK;
}

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
)
{
    // Check if pointers are set
    if ((!sha1CtxPtr) || (!bufPtr))
    {
        fprintf(stderr, "Null pointer provided\n");
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    // Check buffer length
    if (bufSize < sizeof(SHA_CTX))
    {
        fprintf(stderr, "Buffer is too short (%zu < %zu)\n", bufSize, sizeof(SHA_CTX));
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    // Initialize SHA1 context
    if (LWM2MCORE_ERR_COMPLETED_OK != lwm2mcore_StartSha1(sha1CtxPtr))
    {
        fprintf(stderr, "Unable to initialize SHA1 context\n");
        return LWM2MCORE_ERR_GENERAL_ERROR;
    }

    // Restore the SHA1 context
    memcpy(*sha1CtxPtr, bufPtr, sizeof(SHA_CTX));
    return LWM2MCORE_ERR_COMPLETED_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Cancel and reset the SHA1 computation
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_CancelSha1
(
    void** sha1CtxPtr   ///< [INOUT] SHA1 context pointer
)
{
    // Check if SHA1 context pointer is set
    if (!sha1CtxPtr)
    {
        fprintf(stderr, "No SHA1 context pointer\n");
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    // Reset SHA1 context
    *sha1CtxPtr = NULL;

    return LWM2MCORE_ERR_COMPLETED_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Update SSL Certificate
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the update succeeds
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_UpdateSslCertificate
(
    char*  certPtr,    ///< [IN] Certificate
    size_t len         ///< [IN] Certificate len
)
{
    (void)(len);
    if (!certPtr)
    {
        fprintf(stderr, "NULL certificate\n");
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    return LWM2MCORE_ERR_COMPLETED_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Initialize the SHA256 computation
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_StartSha256
(
    void** sha256CtxPtr   ///< [INOUT] SHA256 context pointer
)
{
    static SHA256_CTX shaCtx;

    // Check if SHA256 context pointer is set
    if (!sha256CtxPtr)
    {
        printf("No SHA256 context pointer\n");
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    // Load the error strings
    ERR_load_crypto_strings();

    // Initialize the SHA256 context
    // SHA256_Init function returns 1 for success, 0 otherwise
    if (1 != SHA256_Init(&shaCtx))
    {
        printf("SHA256_Init failed\n");
        PrintOpenSSLErrors();
        return LWM2MCORE_ERR_GENERAL_ERROR;
    }
    else
    {
        *sha256CtxPtr = (void*)&shaCtx;
        return LWM2MCORE_ERR_COMPLETED_OK;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Compute and update SHA256 digest with the data buffer passed as an argument
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_ProcessSha256
(
    void*    sha256CtxPtr,  ///< [IN] SHA256 context pointer
    uint8_t* bufPtr,        ///< [IN] Data buffer to hash
    size_t   len            ///< [IN] Data buffer length
)
{
    // Check if pointers are set
    if ((!sha256CtxPtr) || (!bufPtr))
    {
        printf("NULL pointer provided\n");
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    // Update SHA256 digest
    // SHA256_Update function returns 1 for success, 0 otherwise
    if (1 != SHA256_Update((SHA256_CTX*)sha256CtxPtr, bufPtr, len))
    {
        printf("SHA256_Update failed\n");
        PrintOpenSSLErrors();
        return LWM2MCORE_ERR_GENERAL_ERROR;
    }
    else
    {
        return LWM2MCORE_ERR_COMPLETED_OK;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * @brief Finalize SHA256 digest and verify the checksum.
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_EndAndCheckSha256
(
    void* sha256txPtr,                  ///< [IN] SHA256 context pointer
    char* sha256DigestToCompare         ///< [IN] SHA256 digest to compare
)
{
    unsigned char sha256Digest[SHA256_DIGEST_LENGTH];
    char outputBuffer[(2*SHA256_DIGEST_LENGTH)+1];
    int i = 0;

    // Check if pointers are set
    if ((!sha256txPtr) || (!sha256DigestToCompare))
    {
        printf("NULL pointer provided\n");
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    // Finalize SHA256 digest
    // SHA56_Final function returns 1 for success, 0 otherwise
    if (1 != SHA256_Final(sha256Digest, (SHA256_CTX*)sha256txPtr))
    {
        printf("SHA256_Final failed\n");
        PrintOpenSSLErrors();
        return LWM2MCORE_ERR_GENERAL_ERROR;
    }

    for(i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        sprintf(outputBuffer + (i * 2), "%02x", sha256Digest[i]);
    }
    outputBuffer[2*SHA256_DIGEST_LENGTH] = 0;
    if (strncmp(outputBuffer, sha256DigestToCompare, 2*SHA256_DIGEST_LENGTH))
    {
        printf("SHA256 check error\n");
        return LWM2MCORE_ERR_SHA_DIGEST_MISMATCH;
    }

    return LWM2MCORE_ERR_COMPLETED_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Copy the SHA256 context in a buffer
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_CopySha256
(
    void*  sha256CtxPtr,    ///< [IN] SHA256 context pointer
    void*  bufPtr,          ///< [INOUT] Buffer
    size_t bufSize          ///< [INOUT] Buffer length
)
{
    // Check if pointers are set
    if ((!sha256CtxPtr) || (!bufPtr))
    {
        printf("Null pointer provided\n");
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    // Check buffer length
    if (bufSize < sizeof(SHA256_CTX))
    {
        printf("Buffer is too short (%zu < %zu)\n", bufSize, sizeof(SHA256_CTX));
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    // Copy the SHA256 context
    memset(bufPtr, 0, bufSize);
    memcpy(bufPtr, sha256CtxPtr, sizeof(SHA256_CTX));
    return LWM2MCORE_ERR_COMPLETED_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Restore the SHA256 context from a buffer
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_RestoreSha256
(
    void*  bufPtr,      ///< [IN] Buffer
    size_t bufSize,     ///< [IN] Buffer length
    void** sha256CtxPtr ///< [INOUT] SHA256 context pointer
)
{
    // Check if pointers are set
    if ((!sha256CtxPtr) || (!bufPtr))
    {
        printf("Null pointer provided\n");
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    // Check buffer length
    if (bufSize < sizeof(SHA256_CTX))
    {
        printf("Buffer is too short (%zu < %zd)\n", bufSize, sizeof(SHA256_CTX));
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    // Initialize SHA256 context
    if (LWM2MCORE_ERR_COMPLETED_OK != lwm2mcore_StartSha256(sha256CtxPtr))
    {
        printf("Unable to initialize SHA256 context\n");
        return LWM2MCORE_ERR_GENERAL_ERROR;
    }

    // Restore the SHA256 context
    memcpy(*sha256CtxPtr, bufPtr, sizeof(SHA256_CTX));
    return LWM2MCORE_ERR_COMPLETED_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Cancel and reset the SHA256 computation
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_CancelSha256
(
    void** sha256CtxPtr   ///< [INOUT] SHA256 context pointer
)
{
    // Check if SHA256 context pointer is set
    if (!sha256CtxPtr)
    {
        printf("No SHA256 context pointer\n");
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    // Reset SHA256 context
    *sha256CtxPtr = NULL;

    return LWM2MCORE_ERR_COMPLETED_OK;
}
