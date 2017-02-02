/**
 * @file osPortSecurity.c
 *
 * Porting layer for credential management
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "osPortSecurity.h"
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
 * These credentials are filles in these parameters.
 * By default, these parameters are saved in RAM and the credentials storage is not managed
 * by this source code.
 * This implies that a each connection to the LWM2M server, a connection to the bootstrap server
 * will be firstly initiated, followed by a connection to the Device Management server.
 */
uint8_t DmPskId[OS_PORT_PSK_LEN+1] = {'\0'};
uint8_t DmPskSecret[OS_PORT_PSK_LEN + 1] = {'\0'};
uint8_t DmServerAddr[OS_PORT_SERVERADDR_LEN] = {'\0'};


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
    char* bufferPtr,                        ///< [INOUT] data buffer
    size_t* lenPtr                          ///< [INOUT] length of input buffer and length of the
                                            ///< returned data
)
{
    lwm2mcore_sid_t result = LWM2MCORE_ERR_OP_NOT_SUPPORTED;

    if ((bufferPtr == NULL) || (lenPtr == NULL) || (credId >= LWM2MCORE_CREDENTIAL_MAX))
    {
        result = LWM2MCORE_ERR_INVALID_ARG;
    }
    else
    {
        switch (credId)
        {
            memset (bufferPtr, 0, *lenPtr);
            case LWM2MCORE_CREDENTIAL_BS_PUBLIC_KEY:
            {
                memcpy(bufferPtr, BS_PSK_ID, strlen ((char*)BS_PSK_ID));
                *lenPtr = strlen((char*)BS_PSK_ID);
                result = LWM2MCORE_ERR_COMPLETED_OK;
            }
            break;

            case LWM2MCORE_CREDENTIAL_BS_SERVER_PUBLIC_KEY:
            {
                result = LWM2MCORE_ERR_COMPLETED_OK;
            }
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
            {
                memcpy(bufferPtr, BS_SERVER_ADDR, strlen ((char*)BS_SERVER_ADDR));
                *lenPtr = strlen((char*)BS_SERVER_ADDR);
                result = LWM2MCORE_ERR_COMPLETED_OK;
            }
            break;

            case LWM2MCORE_CREDENTIAL_DM_PUBLIC_KEY:
            {
                memcpy(bufferPtr, DmPskId, strlen ((char*)DmPskId));
                *lenPtr = strlen((char*)DmPskId);
                result = LWM2MCORE_ERR_COMPLETED_OK;
            }
            break;

            case LWM2MCORE_CREDENTIAL_DM_SERVER_PUBLIC_KEY:
            {
                result = LWM2MCORE_ERR_COMPLETED_OK;
            }
            break;

            case LWM2MCORE_CREDENTIAL_DM_SECRET_KEY:
            {
                memcpy(bufferPtr, DmPskSecret, OS_PORT_PSK_LEN);
                *lenPtr = OS_PORT_PSK_LEN;
                result = LWM2MCORE_ERR_COMPLETED_OK;
            }
            break;

            case LWM2MCORE_CREDENTIAL_DM_ADDRESS:
            {
                memcpy(bufferPtr, DmServerAddr, strlen((char*)DmServerAddr));
                *lenPtr = strlen((char*)DmServerAddr);
                result = LWM2MCORE_ERR_COMPLETED_OK;
            }
            break;

            default:
            {
            }
            break;
        }
    }
    LOG_ARG("os_portSecurityGetCredential credId %d result %d, *lenPtr %d",
            credId, result, *lenPtr);
    return result;
}

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
    char* bufferPtr,                        ///< [INOUT] data buffer
    size_t len                              ///< [IN] length of input buffer and length of the
                                            ///< returned data
)
{
    lwm2mcore_sid_t result = LWM2MCORE_ERR_OP_NOT_SUPPORTED;

    if ((NULL == bufferPtr) || (!len) || (LWM2MCORE_CREDENTIAL_MAX <= credId))
    {
        result = LWM2MCORE_ERR_INVALID_ARG;
    }
    else
    {
        switch (credId)
        {
            case LWM2MCORE_CREDENTIAL_BS_PUBLIC_KEY:
            {
                /* Do not copy just to not overwrite the credential: for test only
                 * Need to keep in memory the new credential
                 */
                result = LWM2MCORE_ERR_COMPLETED_OK;
            }
            break;

            case LWM2MCORE_CREDENTIAL_BS_SERVER_PUBLIC_KEY:
            {
                result = LWM2MCORE_ERR_COMPLETED_OK;
            }
            break;

            case LWM2MCORE_CREDENTIAL_BS_SECRET_KEY:
            {
                /* Do not copy just to not overwrite the credential: for test only
                 * Need to keep in memory the new credential
                 */
                result = LWM2MCORE_ERR_COMPLETED_OK;
            }
            break;

            case LWM2MCORE_CREDENTIAL_BS_ADDRESS:
            {
                /* Do not copy just to not overwrite the credential: for test only
                 * Need to keep in memory the new credential
                 */
                result = LWM2MCORE_ERR_COMPLETED_OK;
            }
            break;

            case LWM2MCORE_CREDENTIAL_DM_PUBLIC_KEY:
            {
                memcpy(DmPskId, bufferPtr, len);
                result = LWM2MCORE_ERR_COMPLETED_OK;
            }
            break;

            case LWM2MCORE_CREDENTIAL_DM_SERVER_PUBLIC_KEY:
            {
                result = LWM2MCORE_ERR_COMPLETED_OK;
            }
            break;

            case LWM2MCORE_CREDENTIAL_DM_SECRET_KEY:
            {
                memcpy(DmPskSecret, bufferPtr, len);
                result = LWM2MCORE_ERR_COMPLETED_OK;
            }
            break;

            case LWM2MCORE_CREDENTIAL_DM_ADDRESS:
            {
                memcpy(DmServerAddr, bufferPtr, len);
                result = LWM2MCORE_ERR_COMPLETED_OK;
            }
            break;

            default:
            {
            }
            break;
        }
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
static bool CredentialCheckPresence
(
    lwm2mcore_credentials_t credId      ///< [IN] Credential identifier
)
{
    bool result = false;

    switch (credId)
    {
        case LWM2MCORE_CREDENTIAL_DM_PUBLIC_KEY:
        {
            if (strlen((char*)DmPskId))
            {
                result = true;
            }
        }
        break;

        case LWM2MCORE_CREDENTIAL_DM_SECRET_KEY:
        {
            if (strlen((char*)DmPskSecret))
            {
                result = true;
            }
        }
        break;

        case LWM2MCORE_CREDENTIAL_DM_ADDRESS:
        {
            if (strlen((char*)DmServerAddr))
            {
                result = true;
            }
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
 * Function to check if all Device Management credentials were provisioned
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
)
{
    bool result = false;

    /* Check if credentials linked to DM server are present:
     * PSK Id, PSK secret, server URL
     */
    if (CredentialCheckPresence(LWM2MCORE_CREDENTIAL_DM_PUBLIC_KEY)
     && CredentialCheckPresence(LWM2MCORE_CREDENTIAL_DM_SECRET_KEY)
     && CredentialCheckPresence(LWM2MCORE_CREDENTIAL_DM_ADDRESS))
    {
        result = true;
    }
    LOG_ARG("os_portSecurityDmServerPresence result %d", result);
    return result;
}


