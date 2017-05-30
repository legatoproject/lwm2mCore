/**
 * @file security.c
 *
 * Porting layer for credential management and package security (CRC, signature)
 *
 * @note The CRC is computed using the crc32 function from zlib.
 * @note The signature verification uses the OpenSSL library.
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#include <stdint.h>
#include <stddef.h>
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
#include "internals.h"

/* Set the bootstrap credentials with correct values
 * BS server address can be retrieved from LWM2MCore team
 * BS PSK identity and PSK secret can be retrieved from the LWM2M server team (please ask to
 * LWM2MCore team for any support)
 */

#define BS_SERVER_ADDR ""
#define BS_PSK_ID ""
#define BS_PSK ""

/* These values does not need to be filled.
 * When the client connects to the bootstrap server, the bootstrap server sends Device
 * Management credentials to the client.
 * These credentials are filled in these parameters.
 * By default, these parameters are saved in RAM and the credentials storage is not managed
 * by this source code.
 * This implies that a each connection to the LWM2M server, a connection to the bootstrap server
 * will be firstly initiated, followed by a connection to the Device Management server.
 */
static uint8_t DmPskId[LWM2MCORE_PSKID_LEN+1] = {'\0'};
static uint8_t DmPskSecret[LWM2MCORE_PSK_LEN + 1] = {'\0'};
static uint8_t DmServerAddr[LWM2MCORE_SERVERADDR_LEN] = {'\0'};


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
 *      - LWM2MCORE_ERR_OVERFLOW in case of buffer overflow
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetCredential
(
    lwm2mcore_Credentials_t credId,         ///< [IN] credential Id of credential to be retrieved
    char* bufferPtr,                        ///< [INOUT] data buffer
    size_t* lenPtr                          ///< [INOUT] length of input buffer and length of the
                                            ///< returned data
)
{
    lwm2mcore_Sid_t result = LWM2MCORE_ERR_OP_NOT_SUPPORTED;

    if ((bufferPtr == NULL) || (lenPtr == NULL) || (credId >= LWM2MCORE_CREDENTIAL_MAX))
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    memset(bufferPtr, 0, *lenPtr);
    switch (credId)
    {
        case LWM2MCORE_CREDENTIAL_BS_PUBLIC_KEY:
            if (*lenPtr < strlen ((char*)BS_PSK_ID))
            {
                return LWM2MCORE_ERR_OVERFLOW;
            }

            memcpy(bufferPtr, BS_PSK_ID, strlen ((char*)BS_PSK_ID));
            *lenPtr = strlen((char*)BS_PSK_ID);
            result = LWM2MCORE_ERR_COMPLETED_OK;
            break;

        case LWM2MCORE_CREDENTIAL_BS_SERVER_PUBLIC_KEY:
            result = LWM2MCORE_ERR_COMPLETED_OK;
            break;

        case LWM2MCORE_CREDENTIAL_BS_SECRET_KEY:
        {
            uint16_t pskLen = strlen((char*)BS_PSK) / 2;

            // Hex string to binary
            char *h = BS_PSK;
            char *b = bufferPtr;
            char xlate[] = "0123456789ABCDEF";

            for ( ; *h; h += 2, ++b)
            {
                char *l = strchr(xlate, toupper(*h));
                char *r = strchr(xlate, toupper(*(h+1)));

                if (!r || !l)
                {
                    LOG("Failed to parse Pre-Shared-Key HEXSTRING");
                    return -1;
                }

                *b = ((l - xlate) << 4) + (r - xlate);
            }
            *lenPtr = pskLen;
            result = LWM2MCORE_ERR_COMPLETED_OK;
        }
        break;

        case LWM2MCORE_CREDENTIAL_BS_ADDRESS:
            if (*lenPtr > strlen ((char*)BS_SERVER_ADDR))
            {
                return LWM2MCORE_ERR_OVERFLOW;
            }
            memcpy(bufferPtr, BS_SERVER_ADDR, strlen((char*)BS_SERVER_ADDR));
            *lenPtr = strlen((char*)BS_SERVER_ADDR);
            result = LWM2MCORE_ERR_COMPLETED_OK;
            break;

        case LWM2MCORE_CREDENTIAL_DM_PUBLIC_KEY:
            if (*lenPtr > strlen ((char*)DmPskId))
            {
                return LWM2MCORE_ERR_OVERFLOW;
            }
            memcpy(bufferPtr, DmPskId, strlen((char*)DmPskId));
            *lenPtr = strlen((char*)DmPskId);
            result = LWM2MCORE_ERR_COMPLETED_OK;
            break;

        case LWM2MCORE_CREDENTIAL_DM_SERVER_PUBLIC_KEY:
            result = LWM2MCORE_ERR_COMPLETED_OK;
            break;

        case LWM2MCORE_CREDENTIAL_DM_SECRET_KEY:
            memcpy(bufferPtr, DmPskSecret, LWM2MCORE_PSK_LEN);
            *lenPtr = LWM2MCORE_PSK_LEN;
            result = LWM2MCORE_ERR_COMPLETED_OK;
            break;

        case LWM2MCORE_CREDENTIAL_DM_ADDRESS:
            if (*lenPtr > strlen ((char*)DmServerAddr))
            {
                return LWM2MCORE_ERR_OVERFLOW;
            }
            memcpy(bufferPtr, DmServerAddr, strlen((char*)DmServerAddr));
            *lenPtr = strlen((char*)DmServerAddr);
            result = LWM2MCORE_ERR_COMPLETED_OK;
            break;

        case LWM2MCORE_CREDENTIAL_FW_KEY:
        {
            // Public key for firmware package (X.509 SubjectPublicKeyInfo format)
            uint8_t publicKeyFw[] =
            {
                0x30, 0x82, 0x01, 0x20, 0x30, 0x0D, 0x06, 0x09, 0x2A, 0x86,
                0x48, 0x86, 0xF7, 0x0D, 0x01, 0x01, 0x01, 0x05, 0x00, 0x03,
                0x82, 0x01, 0x0D, 0x00, 0x30, 0x82, 0x01, 0x08, 0x02, 0x82,
                0x01, 0x01, 0x00, 0xB2, 0x25, 0xCC, 0xFB, 0x87, 0xA4, 0x9A,
                0x4D, 0xDF, 0xF4, 0xD8, 0xF8, 0x6B, 0x06, 0xFB, 0xAC, 0xA6,
                0x70, 0x74, 0x93, 0xF7, 0x7E, 0x0F, 0x32, 0xA9, 0x8D, 0xB2,
                0x23, 0xF3, 0x57, 0x40, 0x30, 0x83, 0x73, 0x8F, 0x8B, 0x74,
                0xF5, 0x77, 0xA0, 0x39, 0x4F, 0x70, 0x56, 0x96, 0x2D, 0x32,
                0x3C, 0x13, 0xC3, 0x9F, 0x6C, 0x1B, 0x20, 0x73, 0xF9, 0xB4,
                0xCD, 0xA7, 0xEC, 0xF4, 0xAA, 0xB6, 0xCE, 0xF0, 0x70, 0x9C,
                0xEA, 0x7F, 0x22, 0x02, 0x32, 0x0B, 0x2F, 0xF2, 0xDE, 0x35,
                0x55, 0x3F, 0x17, 0xD2, 0x86, 0xDE, 0x95, 0xC8, 0xC6, 0xDC,
                0x33, 0xA2, 0x70, 0x72, 0x58, 0x3A, 0x41, 0x39, 0xAE, 0x6B,
                0x78, 0xDD, 0x4A, 0x1C, 0x6A, 0xC4, 0xDE, 0xAD, 0xB7, 0xF8,
                0xDC, 0xAE, 0xCC, 0x20, 0x3D, 0x20, 0x21, 0x04, 0x04, 0x51,
                0x25, 0xBF, 0xF5, 0x19, 0xE3, 0x98, 0x07, 0x03, 0xB9, 0x00,
                0x2B, 0x54, 0xFB, 0xEC, 0x91, 0x5D, 0xB3, 0x6D, 0x17, 0x79,
                0x12, 0xE0, 0xF2, 0x50, 0x55, 0x21, 0x3F, 0x04, 0xE4, 0xAF,
                0xB2, 0x75, 0x5A, 0xFD, 0x3C, 0x2C, 0xB0, 0x9F, 0xBC, 0x46,
                0x0C, 0x57, 0xC9, 0xE0, 0x25, 0xD9, 0x6C, 0xD3, 0xF6, 0x3B,
                0x31, 0x2C, 0x39, 0x65, 0xA0, 0x14, 0x44, 0x2C, 0x6E, 0x38,
                0xA9, 0x37, 0xED, 0x84, 0xCC, 0x9E, 0xF8, 0xD0, 0xD3, 0x97,
                0x15, 0xB2, 0xB3, 0xE2, 0xC2, 0xFA, 0xF2, 0xEB, 0xB8, 0x9A,
                0x15, 0xBA, 0x69, 0x93, 0xC1, 0x1C, 0xEE, 0x9B, 0x81, 0xA5,
                0x6B, 0x17, 0xAE, 0x8E, 0x2D, 0x36, 0x42, 0xC6, 0x79, 0x19,
                0xBB, 0x05, 0xDD, 0x2B, 0x92, 0x40, 0x95, 0x3C, 0xE5, 0xF2,
                0x41, 0xAD, 0x45, 0x4B, 0x1A, 0xE5, 0x02, 0x10, 0x55, 0xD8,
                0x4B, 0xB7, 0xAA, 0xB6, 0x0B, 0xEA, 0x7D, 0xEA, 0x58, 0xFE,
                0xF9, 0x9E, 0x8D, 0xEC, 0xAA, 0xA8, 0x71, 0x47, 0x49, 0x02,
                0x01, 0x03
            };

            memcpy(bufferPtr, publicKeyFw, sizeof(publicKeyFw));
            *lenPtr = sizeof(publicKeyFw);
            result = LWM2MCORE_ERR_COMPLETED_OK;
        }
        break;

        case LWM2MCORE_CREDENTIAL_SW_KEY:
        {
            // Public key for software package (PEM DER ASN.1 PKCS#1 RSA Public key format)
            uint8_t publicKeySw[] =
            {
                0x30, 0x82, 0x01, 0x08, 0x02, 0x82, 0x01, 0x01, 0x00, 0x9F,
                0x5C, 0xB2, 0xAD, 0x37, 0x64, 0xBB, 0xDB, 0xAB, 0xC3, 0x1C,
                0xDD, 0x60, 0x58, 0x15, 0xE4, 0xC0, 0x95, 0xEC, 0xB4, 0xAA,
                0x5B, 0x6C, 0x7E, 0x11, 0x41, 0x9E, 0x6D, 0x57, 0xB0, 0xF3,
                0xF9, 0x5A, 0x89, 0x7E, 0x27, 0x60, 0xCA, 0x51, 0x5E, 0xEC,
                0xD7, 0x45, 0xA1, 0x15, 0xBA, 0x5F, 0x14, 0xAA, 0x97, 0x19,
                0x0A, 0xD6, 0xB9, 0xC1, 0x16, 0xAB, 0xA0, 0xDE, 0xA6, 0xBE,
                0x6A, 0x9F, 0x9C, 0x06, 0xFB, 0x8C, 0x8E, 0xD6, 0xF9, 0x4A,
                0xD4, 0xDF, 0xC2, 0x1B, 0x1B, 0x87, 0x3B, 0xB8, 0x76, 0xB4,
                0xD4, 0x83, 0x9E, 0xBE, 0x29, 0x0D, 0x65, 0xB4, 0xF4, 0x22,
                0x4E, 0xBD, 0x89, 0x39, 0xFA, 0xC2, 0xCE, 0xCA, 0x1B, 0x37,
                0xC6, 0x67, 0xF0, 0x4A, 0xA5, 0x3C, 0x7D, 0xA3, 0x28, 0x68,
                0xB7, 0xAC, 0x76, 0x19, 0x23, 0x84, 0x55, 0xC4, 0xE3, 0xBE,
                0x5F, 0x9A, 0x48, 0xBC, 0x9D, 0xB8, 0x5C, 0xB0, 0x57, 0x94,
                0x1C, 0x10, 0x20, 0x39, 0x44, 0x77, 0x19, 0x49, 0x9C, 0x32,
                0xFF, 0x09, 0x0C, 0xEC, 0x62, 0xA3, 0x95, 0xD1, 0x41, 0x24,
                0x56, 0x65, 0x1C, 0xF5, 0x1B, 0xE8, 0x8F, 0x02, 0xAD, 0x43,
                0x2A, 0x83, 0x53, 0x8F, 0x80, 0x33, 0xFA, 0x4D, 0xBE, 0xA8,
                0x01, 0x3D, 0xC3, 0xB0, 0x80, 0xCB, 0xF5, 0x7A, 0x5A, 0x2D,
                0x53, 0xA4, 0x49, 0x06, 0x2C, 0x7B, 0xD5, 0x26, 0x66, 0x7C,
                0x36, 0x4E, 0xAD, 0x5D, 0x48, 0x25, 0x6A, 0x8E, 0x72, 0x1C,
                0x00, 0x48, 0x01, 0xC3, 0xF5, 0xA0, 0xD5, 0x48, 0xB0, 0x45,
                0x93, 0x9E, 0xFD, 0x7D, 0x81, 0x6A, 0xA6, 0xE8, 0xA8, 0x58,
                0x74, 0x2D, 0x8A, 0x3B, 0xA2, 0x92, 0x81, 0x4D, 0x03, 0xFF,
                0x87, 0xB1, 0x40, 0x28, 0x7E, 0x73, 0xA7, 0x96, 0x12, 0x6E,
                0xD5, 0xE9, 0x0F, 0xE5, 0x48, 0xC1, 0x03, 0xBA, 0x6E, 0x47,
                0x80, 0xA6, 0x87, 0x52, 0x33, 0x02, 0x01, 0x03
            };

            memcpy(bufferPtr, publicKeySw, sizeof(publicKeySw));
            *lenPtr = sizeof(publicKeySw);
            result = LWM2MCORE_ERR_COMPLETED_OK;
        }
        break;

        default:
            break;
    }
    return result;
}

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
 *      - LWM2MCORE_ERR_OVERFLOW in case of buffer overflow
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_SetCredential
(
    lwm2mcore_Credentials_t credId,         ///< [IN] credential Id of credential to be set
    char* bufferPtr,                        ///< [INOUT] data buffer
    size_t len                              ///< [IN] length of input buffer
)
{
    lwm2mcore_Sid_t result = LWM2MCORE_ERR_OP_NOT_SUPPORTED;

    if ((NULL == bufferPtr) || (!len) || (LWM2MCORE_CREDENTIAL_MAX <= credId))
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    switch (credId)
    {
        case LWM2MCORE_CREDENTIAL_BS_PUBLIC_KEY:
            /* Do not copy just to not overwrite the credential: for test only
             * Need to keep in memory the new credential
             */
            result = LWM2MCORE_ERR_COMPLETED_OK;
            break;

        case LWM2MCORE_CREDENTIAL_BS_SERVER_PUBLIC_KEY:
            result = LWM2MCORE_ERR_COMPLETED_OK;
            break;

        case LWM2MCORE_CREDENTIAL_BS_SECRET_KEY:
            /* Do not copy just to not overwrite the credential: for test only
             * Need to keep in memory the new credential
             */
            result = LWM2MCORE_ERR_COMPLETED_OK;
            break;

        case LWM2MCORE_CREDENTIAL_BS_ADDRESS:
            /* Do not copy just to not overwrite the credential: for test only
             * Need to keep in memory the new credential
             */
            result = LWM2MCORE_ERR_COMPLETED_OK;
            break;

        case LWM2MCORE_CREDENTIAL_DM_PUBLIC_KEY:
            if (len > strlen ((char*)DmPskId))
            {
                return LWM2MCORE_ERR_OVERFLOW;
            }
            memcpy(DmPskId, bufferPtr, len);
            result = LWM2MCORE_ERR_COMPLETED_OK;
            break;

        case LWM2MCORE_CREDENTIAL_DM_SERVER_PUBLIC_KEY:
            result = LWM2MCORE_ERR_COMPLETED_OK;
            break;

        case LWM2MCORE_CREDENTIAL_DM_SECRET_KEY:
            if (len > strlen ((char*)DmPskSecret))
            {
                return LWM2MCORE_ERR_OVERFLOW;
            }
            memcpy(DmPskSecret, bufferPtr, len);
            result = LWM2MCORE_ERR_COMPLETED_OK;
            break;

        case LWM2MCORE_CREDENTIAL_DM_ADDRESS:
            if (len > strlen ((char*)DmServerAddr))
            {
                return LWM2MCORE_ERR_OVERFLOW;
            }
            memcpy(DmServerAddr, bufferPtr, len);
            result = LWM2MCORE_ERR_COMPLETED_OK;
            break;

        default:
            break;
    }
    return result;
}

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
    lwm2mcore_Credentials_t credId      ///< [IN] Credential identifier
)
{
    bool result = false;

    switch (credId)
    {
        case LWM2MCORE_CREDENTIAL_DM_PUBLIC_KEY:
            if (strlen((char*)DmPskId))
            {
                result = true;
            }
            break;

        case LWM2MCORE_CREDENTIAL_DM_SECRET_KEY:
            if (strlen((char*)DmPskSecret))
            {
                result = true;
            }
            break;

        case LWM2MCORE_CREDENTIAL_DM_ADDRESS:
            if (strlen((char*)DmServerAddr))
            {
                result = true;
            }
            break;

        default:
            break;
    }

    LOG_ARG("Credential presence: credId %d result %d", credId, result);
    return result;
}

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
    lwm2mcore_Credentials_t credId      ///< [IN] Credential identifier
)
{
    bool result = true;

    switch (credId)
    {
        case LWM2MCORE_CREDENTIAL_DM_PUBLIC_KEY:
            memset(DmPskId, 0, LWM2MCORE_PSK_LEN + 1);
            break;

        case LWM2MCORE_CREDENTIAL_DM_SECRET_KEY:
            memset(DmPskSecret, 0, LWM2MCORE_PSK_LEN + 1);
            break;

        case LWM2MCORE_CREDENTIAL_DM_ADDRESS:
            memset(DmServerAddr, 0, LWM2MCORE_SERVERADDR_LEN);
            break;

        default:
            result = false;
            break;
    }

    LOG_ARG("Credential presence: credId %d result %d", credId, result);
    return result;
}

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
        LOG_ARG("%s", errorString);

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
        LOG("No SHA1 context pointer");
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    // Load the error strings
    ERR_load_crypto_strings();

    // Initialize the SHA1 context
    // SHA1_Init function returns 1 for success, 0 otherwise
    if (1 != SHA1_Init(&shaCtx))
    {
        LOG("SHA1_Init failed");
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
        LOG("NULL pointer provided");
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    // Update SHA1 digest
    // SHA1_Update function returns 1 for success, 0 otherwise
    if (1 != SHA1_Update((SHA_CTX*)sha1CtxPtr, bufPtr, len))
    {
        LOG("SHA1_Update failed");
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
    lwm2mcore_PkgDwlType_t packageType, ///< [IN] Package type (FW or SW)
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
        LOG("NULL pointer provided");
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    // Finalize SHA1 digest
    // SHA1_Final function returns 1 for success, 0 otherwise
    if (1 != SHA1_Final(sha1Digest, (SHA_CTX*)sha1CtxPtr))
    {
        LOG("SHA1_Final failed");
        PrintOpenSSLErrors();
        return LWM2MCORE_ERR_GENERAL_ERROR;
    }

    // The package type indicates the public key to use
    switch (packageType)
    {
        case LWM2MCORE_PKG_FW:
            credId = LWM2MCORE_CREDENTIAL_FW_KEY;
            break;

        case LWM2MCORE_PKG_SW:
            credId = LWM2MCORE_CREDENTIAL_SW_KEY;
            break;

        default:
            LOG_ARG("Unknown or unsupported package type %d", packageType);
            return LWM2MCORE_ERR_GENERAL_ERROR;
            break;
    }

    // Retrieve the public key corresponding to the package type
    if (LWM2MCORE_ERR_COMPLETED_OK != lwm2mcore_GetCredential(credId,
                                                              publicKey,
                                                              &publicKeyLen))
    {
        LOG_ARG("Error while retrieving credentials %d", credId);
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
        LOG("Unable to create a memory BIO");
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
            LOG("Unable to create a memory BIO");
            PrintOpenSSLErrors();
            return LWM2MCORE_ERR_GENERAL_ERROR;
        }

        // Then convert it to a RSA key using X.509 SubjectPublicKeyInfo format
        rsaKeyPtr = d2i_RSA_PUBKEY_bio(bufioPtr, NULL);
    }
    BIO_free(bufioPtr);
    if (!rsaKeyPtr)
    {
        LOG("Unable to retrieve public key");
        PrintOpenSSLErrors();
        return LWM2MCORE_ERR_GENERAL_ERROR;
    }
    evpPkeyPtr = EVP_PKEY_new();
    if (!evpPkeyPtr)
    {
        LOG("Unable to create EVP_PKEY structure");
        PrintOpenSSLErrors();
        return LWM2MCORE_ERR_GENERAL_ERROR;
    }
    // EVP_PKEY_assign_RSA returns 1 for success and 0 for failure
    if (1 != EVP_PKEY_assign_RSA(evpPkeyPtr, rsaKeyPtr))
    {
        LOG("Unable to assign public key");
        PrintOpenSSLErrors();
        return LWM2MCORE_ERR_GENERAL_ERROR;
    }

    // Create EVP public key context, necessary to verify the signature
    evpPkeyCtxPtr = EVP_PKEY_CTX_new(evpPkeyPtr, NULL);
    if (   (!evpPkeyCtxPtr)
        || (1 != EVP_PKEY_verify_init(evpPkeyCtxPtr))
       )
    {
        LOG("Unable to create and initialize EVP PKEY context");
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
        LOG("Error during EVP PKEY context initialization");
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
        LOG("Signature verification failed");
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
        LOG("Null pointer provided");
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    // Check buffer length
    if (bufSize < sizeof(SHA_CTX))
    {
        LOG_ARG("Buffer is too short (%zu < %d)", bufSize, sizeof(SHA_CTX));
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
        LOG("Null pointer provided");
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    // Check buffer length
    if (bufSize < sizeof(SHA_CTX))
    {
        LOG_ARG("Buffer is too short (%zu < %d)", bufSize, sizeof(SHA_CTX));
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    // Initialize SHA1 context
    if (LWM2MCORE_ERR_COMPLETED_OK != lwm2mcore_StartSha1(sha1CtxPtr))
    {
        LOG("Unable to initialize SHA1 context");
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
        LOG("No SHA1 context pointer");
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    // Reset SHA1 context
    *sha1CtxPtr = NULL;

    return LWM2MCORE_ERR_COMPLETED_OK;
}

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
lwm2mcore_Sid_t lwm2mcore_ConvertDERToPEM
(
    unsigned char*  derKeyPtr,      ///< [IN]       DER key
    int             derKeyLen,      ///< [IN]       DER key length
    unsigned char*  pemKeyPtr,      ///< [OUT]      PEM key
    int*            pemKeyLenPtr    ///< [IN/OUT]   PEM key length
)
{
    X509 *certPtr;
    BIO *memPtr;
    int count;

    if ( (!derKeyPtr) || (!pemKeyPtr) || (!pemKeyLenPtr) )
    {
        LOG_ARG("invalid input arguments: derKeyPtr (%p), pemKeyPtr(%p), "
            "pemKeyLenPtr(%p)", derKeyPtr, pemKeyPtr);
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    if (!derKeyLen)
    {
        LOG("derKeyLen cannot be 0");
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    certPtr = d2i_X509(NULL, &derKeyPtr, derKeyLen);
    if (!certPtr)
    {
        LOG("unable to parse certificate");
        PrintOpenSSLErrors();
        return LWM2MCORE_ERR_GENERAL_ERROR;
    }

    memPtr = BIO_new(BIO_s_mem());
    if (!memPtr)
    {
        LOG("failed to set BIO type");
        PrintOpenSSLErrors();
        goto x509_err;
    }

    if (!PEM_write_bio_X509(memPtr, certPtr))
    {
        LOG("failed to write certificate");
        PrintOpenSSLErrors();
        goto bio_err;
    }

    if (memPtr->num_write > *pemKeyLenPtr)
    {
        LOG("not enough space to hold the key");
        goto bio_err;
    }

    memset(pemKeyPtr, 0, memPtr->num_write + 1);

    *pemKeyLenPtr = BIO_read(memPtr, pemKeyPtr, memPtr->num_write);
    if (*pemKeyLenPtr < memPtr->num_write)
    {
        LOG_ARG("failed to read certificate: count (%d)", *pemKeyLenPtr);
        PrintOpenSSLErrors();
        goto pem_err;
    }

    BIO_free(memPtr);
    X509_free(certPtr);

    return LWM2MCORE_ERR_COMPLETED_OK;

pem_err:
    memset(pemKeyPtr, 0, memPtr->num_write + 1);
bio_err:
    BIO_free(memPtr);
x509_err:
    X509_free(certPtr);
    return LWM2MCORE_ERR_GENERAL_ERROR;
}
