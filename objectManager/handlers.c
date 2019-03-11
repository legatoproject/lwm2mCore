/**
 * @file handlers.c
 *
 * client of the LwM2M stack
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#include <stdint.h>
#include <stddef.h>
#include <errno.h>
#include <platform/types.h>
#include <lwm2mcore/lwm2mcore.h>
#include <lwm2mcore/connectivity.h>
#include <lwm2mcore/device.h>
#include <lwm2mcore/security.h>
#include <lwm2mcore/server.h>
#include <lwm2mcore/paramStorage.h>
#include <lwm2mcore/timer.h>
#include <lwm2mcore/update.h>
#include <lwm2mcore/location.h>
#include <lwm2mcore/lwm2mcorePackageDownloader.h>
#include "downloader.h"
#include <lwm2mcore/timer.h>
#include "handlers.h"
#include "sessionManager.h"
#include "objects.h"
#include "internals.h"
#include "utils.h"
#include "aclConfiguration.h"
#include "bootstrapConfiguration.h"
#include "liblwm2m.h"
#include "workspace.h"
#include "updateAgent.h"

//--------------------------------------------------------------------------------------------------
/**
 * Maximum number of bytes in the Universal Geographical Area Description of velocity
 * GAD is defined in the 3GPP 23.032 standard, section 8
 */
//--------------------------------------------------------------------------------------------------
#define LWM2MCORE_GAD_VELOCITY_MAX_BYTES    7

//--------------------------------------------------------------------------------------------------
/**
 * Velocity type according to the Universal Geographical Area Description
 * Velocity type is defined in the 3GPP 23.032 standard, section 8.6
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    LWM2MCORE_VELOCITY_H                       = 0, ///< Horizontal Velocity
    LWM2MCORE_VELOCITY_H_AND_V                 = 1, ///< Horizontal with Vertical Velocity
    LWM2MCORE_VELOCITY_H_AND_UNCERTAINTY       = 2, ///< Horizontal Velocity with Uncertainty
    LWM2MCORE_VELOCITY_H_AND_V_AND_UNCERTAINTY = 3, ///< Horizontal with Vertical Velocity and
                                                    ///< Uncertainty
}
VelocityType_t;

//--------------------------------------------------------------------------------------------------
/**
 * Build the velocity, formated according to 3GPP 23.032 (Universal Geographical Area
 * Description)
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_OVERFLOW in case of buffer overflow
 */
//--------------------------------------------------------------------------------------------------
static lwm2mcore_Sid_t BuildVelocity
(
    char*   bufferPtr,  ///< [INOUT] data buffer
    size_t* lenPtr      ///< [INOUT] length of input buffer and length of the returned data
)
{
    lwm2mcore_Sid_t sID, lsID, hsID, vsID;
    uint32_t direction = 0;
    uint32_t hSpeed = 0;
    int32_t vSpeed = 0;
    uint8_t gadVelocity[LWM2MCORE_GAD_VELOCITY_MAX_BYTES];
    uint8_t gadVelocityLen = 0;

    /* Get the direction of movement */
    lsID = lwm2mcore_GetDirection(&direction);
    switch (lsID)
    {
        case LWM2MCORE_ERR_INVALID_STATE:
            /* No direction available, assume 0 */
            direction = 0;
            break;

        case LWM2MCORE_ERR_COMPLETED_OK:
            /* Direction successfully retrieved */
            break;

        default:
            /* Direction is necessary to build the velocity */
            return LWM2MCORE_ERR_GENERAL_ERROR;
    }

    /* Get the horizontal speed */
    hsID = lwm2mcore_GetHorizontalSpeed(&hSpeed);
    if (LWM2MCORE_ERR_COMPLETED_OK != hsID)
    {
        if (LWM2MCORE_ERR_INVALID_STATE == hsID)
        {
            return hsID;
        }
        else
        {
            /* We need at least the horizontal speed to build the velocity */
            return LWM2MCORE_ERR_GENERAL_ERROR;
        }
    }

    /* Velocity initialization */
    memset(gadVelocity, 0, sizeof(gadVelocity));

    /* Get the vertical speed */
    vsID = lwm2mcore_GetVerticalSpeed(&vSpeed);
    if (LWM2MCORE_ERR_COMPLETED_OK == vsID)
    {
        uint8_t vSpeedDir;

        /* Bits 5 to 8 of byte 1: Velocity type */
        gadVelocity[0] = (uint8_t)(((uint8_t)(LWM2MCORE_VELOCITY_H_AND_V)) << 4);
        gadVelocityLen++;
        /* Bit 2 of byte 1: Direction of vertical speed */
        /* 0 = upward, 1 = downward */
        vSpeedDir = (vSpeed < 0) ? 1 : 0;
        gadVelocity[0] |= (uint8_t)(vSpeedDir << 1);
        /* Last bit of byte 1 and byte 2: Bearing in degrees */
        gadVelocity[0] |= (uint8_t)(((uint16_t)(direction & 0x0100)) >> 8);
        gadVelocity[1] |= (uint8_t)(direction & 0xFF);
        gadVelocityLen++;
        /* Bytes 3 and 4: Horizontal speed in km/h */
        hSpeed = hSpeed * 3.6;
        gadVelocity[2] = (uint8_t)(((uint16_t)(hSpeed & 0xFF00)) >> 8);
        gadVelocityLen++;
        gadVelocity[3] = (uint8_t)(hSpeed & 0xFF);
        gadVelocityLen++;
        /* Byte 5: Vertical speed in km/h */
        vSpeed = abs(vSpeed) * 3.6;
        gadVelocity[4] = (uint8_t)vSpeed;
        gadVelocityLen++;

        sID = LWM2MCORE_ERR_COMPLETED_OK;
    }
    else
    {
        /* Bits 5 to 8 of byte 1: Velocity type */
        gadVelocity[0] = (uint8_t)(((uint8_t)(LWM2MCORE_VELOCITY_H)) << 4);
        gadVelocityLen++;
        /* Last bit of byte 1 and byte 2: Bearing in degrees */
        gadVelocity[0] |= (uint8_t)(((uint16_t)(direction & 0x0100)) >> 8);
        gadVelocity[1] |= (uint8_t)(direction & 0xFF);
        gadVelocityLen++;
        /* Bytes 3 and 4: Horizontal speed in km/h */
        hSpeed = hSpeed * 3.6;
        gadVelocity[2] = (uint8_t)(((uint16_t)(hSpeed & 0xFF00)) >> 8);
        gadVelocityLen++;
        gadVelocity[3] = (uint8_t)(hSpeed & 0xFF);
        gadVelocityLen++;

        sID = LWM2MCORE_ERR_COMPLETED_OK;
    }
    lwm2mcore_DataDump("Velocity buffer", gadVelocity, gadVelocityLen);

    /* Copy the velocity to the output buffer */
    if (*lenPtr < gadVelocityLen)
    {
        sID = LWM2MCORE_ERR_OVERFLOW;
    }
    else
    {
        memcpy(bufferPtr, gadVelocity, gadVelocityLen);
        *lenPtr = gadVelocityLen;
    }

    return sID;
}

//--------------------------------------------------------------------------------------------------
/**
 * Update lifetime in the server object and store it to file system
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid
 */
//--------------------------------------------------------------------------------------------------
static lwm2mcore_Sid_t UpdateLifetime
(
    ConfigBootstrapFile_t* bsConfigPtr,     ///< [IN] Config pointer
    uint32_t lifetime,                      ///< [IN] Lifetime
    bool storage                            ///< [IN] Should be stored to filesystem?
)
{
    ConfigServerObject_t* serverInformationPtr;
    if (!bsConfigPtr)
    {
        LOG("Invalid bootstrap configuration");
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    serverInformationPtr = bsConfigPtr->serverPtr;
    if (!serverInformationPtr)
    {
        /* Create new serverInformationPtr (Dummy to configure lifetime in factory) */
        serverInformationPtr = (ConfigServerObject_t*)lwm2m_malloc(sizeof(ConfigServerObject_t));
        LWM2MCORE_ASSERT(serverInformationPtr);
        memset(serverInformationPtr, 0, sizeof(ConfigServerObject_t));

        bsConfigPtr->serverObjectNumber++;
        omanager_AddBootstrapConfigurationServer(bsConfigPtr, serverInformationPtr);
    }
    else
    {
        /* Set lifetime for all servers */
        while (serverInformationPtr)
        {
            serverInformationPtr->data.lifetime = lifetime;
            serverInformationPtr = serverInformationPtr->nextPtr;
        }
    }

    if (false == storage)
    {
        return LWM2MCORE_ERR_COMPLETED_OK;
    }

    /* Save bootstrap configuration */
    if (omanager_StoreBootstrapConfiguration(bsConfigPtr))
    {
        LOG("Lifetime update successful");
        return LWM2MCORE_ERR_COMPLETED_OK;
    }

    return LWM2MCORE_ERR_GENERAL_ERROR;
}


//--------------------------------------------------------------------------------------------------
/**
 *                                  OBJECT 0: SECURITY
 */
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Function to write a resource of object 0
 * Object: 0 - Security
 * Resource: all
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int omanager_WriteSecurityObj
(
    lwm2mcore_Uri_t* uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource
    char* bufferPtr,                    ///< [INOUT] data buffer for information
    size_t len                          ///< [IN] length of input buffer
)
{
    int sID = LWM2MCORE_ERR_GENERAL_ERROR;
    ConfigBootstrapFile_t* bsConfigPtr;
    ConfigSecurityObject_t* securityInformationPtr;

    if ((NULL == uriPtr) || (NULL == bufferPtr))
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    if ((uriPtr->op & LWM2MCORE_OP_WRITE) == 0)
    {
        return LWM2MCORE_ERR_OP_NOT_SUPPORTED;
    }

    bsConfigPtr = omanager_GetBootstrapConfiguration();
    if (!bsConfigPtr)
    {
        return LWM2MCORE_ERR_GENERAL_ERROR;
    }

    securityInformationPtr = omanager_GetBootstrapConfigurationSecurityInstance(bsConfigPtr,
                                                                                uriPtr->oiid);
    if (!securityInformationPtr)
    {
        /* Create new securityInformationPtr */
        securityInformationPtr =
                            (ConfigSecurityObject_t*)lwm2m_malloc(sizeof(ConfigSecurityObject_t));
        LWM2MCORE_ASSERT(securityInformationPtr);
        memset(securityInformationPtr, 0, sizeof(ConfigSecurityObject_t));
        bsConfigPtr->securityObjectNumber++;

        /* Set the security object instance Id */
        securityInformationPtr->data.securityObjectInstanceId = uriPtr->oiid;
        omanager_AddBootstrapConfigurationSecurity(bsConfigPtr, securityInformationPtr);
    }

    switch (uriPtr->rid)
    {
        /* Resource 0: LwM2M server URI */
        case LWM2MCORE_SECURITY_SERVER_URI_RID:
            if ((LWM2MCORE_SERVER_URI_MAX_LEN < len) || (LWM2MCORE_BUFFER_MAX_LEN < len))
            {
                sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            }
            else
            {
                /* Copy the server address */
                snprintf((char*)securityInformationPtr->serverURI,
                         LWM2MCORE_SERVER_URI_MAX_LEN,
                         "%s",
                         bufferPtr);
                sID = LWM2MCORE_ERR_COMPLETED_OK;
            }
            break;

        /* Resource 1: Bootstrap server (true or false) */
        case LWM2MCORE_SECURITY_BOOTSTRAP_SERVER_RID:
#ifdef CREDENTIALS_DEBUG
            lwm2mcore_DataDump("server addr write", bufferPtr, len);
#endif
            securityInformationPtr->data.isBootstrapServer =
                                            (bool)omanager_BytesToInt((const char*)bufferPtr, len);
            sID = LWM2MCORE_ERR_COMPLETED_OK;
            break;

        /* Resource 2: Security mode */
        case LWM2MCORE_SECURITY_MODE_RID:
            securityInformationPtr->data.securityMode =
                                (SecurityMode_t)omanager_BytesToInt((const char*)bufferPtr, len);
            sID = LWM2MCORE_ERR_COMPLETED_OK;
            break;

        /* Resource 3: Public key or identity */
        case LWM2MCORE_SECURITY_PKID_RID:
            if (DTLS_PSK_MAX_CLIENT_IDENTITY_LEN < len)
            {
                sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            }
            else
            {
#ifdef CREDENTIALS_DEBUG
                lwm2mcore_DataDump("PSK ID write", bufferPtr, len);
#endif
                memset(securityInformationPtr->devicePKID, 0, DTLS_PSK_MAX_CLIENT_IDENTITY_LEN);
                memcpy(securityInformationPtr->devicePKID, bufferPtr, len);
                securityInformationPtr->pskIdLen = (uint16_t)len;
                sID = LWM2MCORE_ERR_COMPLETED_OK;
            }
            break;

        /* Resource 4: Server public key */
        case LWM2MCORE_SECURITY_SERVER_KEY_RID:
            sID = LWM2MCORE_ERR_COMPLETED_OK;
            break;

        /* Resource 5: Secret key */
        case LWM2MCORE_SECURITY_SECRET_KEY_RID:
            if ((DTLS_PSK_MAX_CLIENT_IDENTITY_LEN < len) || (DTLS_PSK_MAX_KEY_LEN < len))
            {
                sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            }
            else
            {
#ifdef CREDENTIALS_DEBUG
                lwm2mcore_DataDump("PSK secret write", bufferPtr, len);
#endif
                memcpy(securityInformationPtr->secretKey, bufferPtr, len);
                securityInformationPtr->pskLen = (uint16_t)len;
                sID = LWM2MCORE_ERR_COMPLETED_OK;
            }
            break;

        /* Resource 6: SMS security mode */
        case LWM2MCORE_SECURITY_SMS_SECURITY_MODE_RID:
            sID = LWM2MCORE_ERR_NOT_YET_IMPLEMENTED;
            break;

        /* Resource 7: SMS binding key parameters */
        case LWM2MCORE_SECURITY_SMS_BINDING_KEY_PAR_RID:
            sID = LWM2MCORE_ERR_NOT_YET_IMPLEMENTED;
            break;

        /* Resource 8: SMS binding secret key(s) */
        case LWM2MCORE_SECURITY_SMS_BINDING_SEC_KEY_RID:
            sID = LWM2MCORE_ERR_NOT_YET_IMPLEMENTED;
            break;

        /* Resource 9: LwM2M server SMS number */
        case LWM2MCORE_SECURITY_SERVER_SMS_NUMBER_RID:
            sID = LWM2MCORE_ERR_NOT_YET_IMPLEMENTED;
            break;

        /* Resource 10: Short server ID */
        case LWM2MCORE_SECURITY_SERVER_ID_RID:
                securityInformationPtr->data.serverId =
                                        (uint16_t)omanager_BytesToInt((const char*)bufferPtr, len);
                sID = LWM2MCORE_ERR_COMPLETED_OK;
            break;

        /* Resource 11: Client hold of time */
        case LWM2MCORE_SECURITY_CLIENT_HOLD_OFF_TIME_RID:
            securityInformationPtr->data.clientHoldOffTime =
                                        (uint16_t)omanager_BytesToInt((const char*)bufferPtr, len);
            sID = LWM2MCORE_ERR_COMPLETED_OK;
            break;

        /* Resource 12: Bootstrap-Server Account Timeout */
        case LWM2MCORE_SECURITY_BS_ACCOUNT_TIMEOUT_RID:
            sID = LWM2MCORE_ERR_COMPLETED_OK;
            break;

        default:
            sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            break;
    }
    return sID;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to read a resource of object 0
 * Object: 0 - Security
 * Resource: All
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int omanager_ReadSecurityObj
(
    lwm2mcore_Uri_t* uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource
    char* bufferPtr,                    ///< [INOUT] data buffer for information
    size_t* lenPtr,                     ///< [INOUT] length of input buffer and length of the
                                        ///< returned data
    valueChangedCallback_t changedCb    ///< [IN] callback for notification
)
{
    int sID = LWM2MCORE_ERR_GENERAL_ERROR;
    ConfigBootstrapFile_t* bsConfigPtr;
    ConfigSecurityObject_t* securityInformationPtr;

    (void)changedCb;

    if ((NULL == uriPtr) || (NULL == bufferPtr) || (NULL == lenPtr))
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    if ((uriPtr->op & LWM2MCORE_OP_READ) == 0)
    {
        return LWM2MCORE_ERR_OP_NOT_SUPPORTED;
    }

    bsConfigPtr = omanager_GetBootstrapConfiguration();
    if (!bsConfigPtr)
    {
        return LWM2MCORE_ERR_GENERAL_ERROR;
    }

    securityInformationPtr = omanager_GetBootstrapConfigurationSecurityInstance(bsConfigPtr,
                                                                                uriPtr->oiid);
    if (!securityInformationPtr)
    {
        return LWM2MCORE_ERR_INCORRECT_RANGE;
    }

    switch (uriPtr->rid)
    {
        /* Resource 0: LwM2M server URI */
        case LWM2MCORE_SECURITY_SERVER_URI_RID:
            if (securityInformationPtr->data.isBootstrapServer)
            {
                /* Bootstrap server */
                sID = lwm2mcore_GetCredential(LWM2MCORE_CREDENTIAL_BS_ADDRESS,
                                              securityInformationPtr->data.serverId,
                                              bufferPtr,
                                              lenPtr);
            }
            else
            {
                /* Device Management server */
                sID = lwm2mcore_GetCredential(LWM2MCORE_CREDENTIAL_DM_ADDRESS,
                                              securityInformationPtr->data.serverId,
                                              bufferPtr,
                                              lenPtr);
            }
            break;

        /* Resource 1: Bootstrap server (true or false) */
        case LWM2MCORE_SECURITY_BOOTSTRAP_SERVER_RID:
            *lenPtr = omanager_FormatValueToBytes((uint8_t*) bufferPtr,
                                  &(securityInformationPtr->data.isBootstrapServer),
                                  sizeof(securityInformationPtr->data.isBootstrapServer),
                                  false);
            sID = LWM2MCORE_ERR_COMPLETED_OK;
            break;

        /* Resource 2: Security mode */
        case LWM2MCORE_SECURITY_MODE_RID:
            *lenPtr = omanager_FormatValueToBytes((uint8_t*) bufferPtr,
                                        &(securityInformationPtr->data.securityMode),
                                        sizeof(securityInformationPtr->data.securityMode),
                                        false);
            sID = LWM2MCORE_ERR_COMPLETED_OK;
            break;

        /* Resource 3: Public key or identity */
        case LWM2MCORE_SECURITY_PKID_RID:
            if (securityInformationPtr->data.isBootstrapServer)
            {
                /* Bootstrap server */
                sID = lwm2mcore_GetCredential(LWM2MCORE_CREDENTIAL_BS_PUBLIC_KEY,
                                              securityInformationPtr->data.serverId,
                                              bufferPtr,
                                              lenPtr);
            }
            else
            {
                /* Device Management server */
                sID = lwm2mcore_GetCredential(LWM2MCORE_CREDENTIAL_DM_PUBLIC_KEY,
                                              securityInformationPtr->data.serverId,
                                              bufferPtr,
                                              lenPtr);
            }
#ifdef CREDENTIALS_DEBUG
            lwm2mcore_DataDump("PSK ID read", bufferPtr, *lenPtr);
#endif
            break;

        /* Resource 4: Server public key */
        case LWM2MCORE_SECURITY_SERVER_KEY_RID:
            sID = LWM2MCORE_ERR_COMPLETED_OK;
            break;

        /* Resource 5: Secret key */
        case LWM2MCORE_SECURITY_SECRET_KEY_RID:
            if (securityInformationPtr->data.isBootstrapServer)
            {
                /* Bootstrap server */
                sID = lwm2mcore_GetCredential(LWM2MCORE_CREDENTIAL_BS_SECRET_KEY,
                                              securityInformationPtr->data.serverId,
                                              bufferPtr,
                                              lenPtr);
            }
            else
            {
                /* Device Management server */
                sID = lwm2mcore_GetCredential(LWM2MCORE_CREDENTIAL_DM_SECRET_KEY,
                                              securityInformationPtr->data.serverId,
                                              bufferPtr,
                                              lenPtr);
            }
#ifdef CREDENTIALS_DEBUG
            lwm2mcore_DataDump("PSK secret read", bufferPtr, *lenPtr);
#endif
            break;

        /* Resource 6: SMS security mode */
        case LWM2MCORE_SECURITY_SMS_SECURITY_MODE_RID:
            sID = LWM2MCORE_ERR_NOT_YET_IMPLEMENTED;
            break;

        /* Resource 7: SMS binding key parameters */
        case LWM2MCORE_SECURITY_SMS_BINDING_KEY_PAR_RID:
            sID = LWM2MCORE_ERR_NOT_YET_IMPLEMENTED;
            break;

        /* Resource 8: SMS binding secret key(s) */
        case LWM2MCORE_SECURITY_SMS_BINDING_SEC_KEY_RID:
            sID = LWM2MCORE_ERR_NOT_YET_IMPLEMENTED;
            break;

        /* Resource 9: LwM2M server SMS number */
        case LWM2MCORE_SECURITY_SERVER_SMS_NUMBER_RID:
            sID = LWM2MCORE_ERR_NOT_YET_IMPLEMENTED;
            break;

        /* Resource 10: Short server ID */
        case LWM2MCORE_SECURITY_SERVER_ID_RID:
            *lenPtr = omanager_FormatValueToBytes((uint8_t*) bufferPtr,
                                                  &(securityInformationPtr->data.serverId),
                                                  sizeof(securityInformationPtr->data.serverId),
                                                  false);
            sID = LWM2MCORE_ERR_COMPLETED_OK;
            break;

        /* Resource 11: Client hold of time */
        case LWM2MCORE_SECURITY_CLIENT_HOLD_OFF_TIME_RID:
            *lenPtr = omanager_FormatValueToBytes((uint8_t*) bufferPtr,
                                            &(securityInformationPtr->data.clientHoldOffTime),
                                            sizeof(securityInformationPtr->data.clientHoldOffTime),
                                            false);
            sID = LWM2MCORE_ERR_COMPLETED_OK;
            break;

        /* Resource 12: Bootstrap-Server Account Timeout */
        case LWM2MCORE_SECURITY_BS_ACCOUNT_TIMEOUT_RID:
            sID = LWM2MCORE_ERR_NOT_YET_IMPLEMENTED;
            break;

        default:
            sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            break;
    }
    return sID;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to store credentials in non volatile memory
 *
 * @return
 *      - true in case of success
 *      - false in case of failure
 */
//--------------------------------------------------------------------------------------------------
bool omanager_StoreCredentials
(
    void
)
{
    bool result = false;
    int storageResult = LWM2MCORE_ERR_COMPLETED_OK;
    ConfigBootstrapFile_t* bsConfigPtr;
    ConfigSecurityObject_t* securityInformationPtr;

    bsConfigPtr = omanager_GetBootstrapConfiguration();
    if (!bsConfigPtr)
    {
        return false;
    }

    securityInformationPtr = bsConfigPtr->securityPtr;

    while (securityInformationPtr)
    {
        if (securityInformationPtr->data.isBootstrapServer)
        {
            LOG_ARG("Bootstrap: PskIdLen %d PskLen %d Addr len %d",
                    securityInformationPtr->pskIdLen,
                    securityInformationPtr->pskLen,
                    strlen((const char *)securityInformationPtr->serverURI));

            if (   (securityInformationPtr->pskIdLen)
                && (securityInformationPtr->pskLen)
                && (strlen((const char *)securityInformationPtr->serverURI))
                )
            {
                // Only backup and set credentials if they are different from the currently stored ones
                if (!lwm2mcore_CredentialMatch(LWM2MCORE_CREDENTIAL_BS_PUBLIC_KEY,
                                               LWM2MCORE_BS_SERVER_ID,
                                               (char*)securityInformationPtr->devicePKID))
                {
                    lwm2mcore_BackupCredential(LWM2MCORE_CREDENTIAL_BS_PUBLIC_KEY, LWM2MCORE_BS_SERVER_ID);

                    storageResult = lwm2mcore_SetCredential(LWM2MCORE_CREDENTIAL_BS_PUBLIC_KEY,
                                                            LWM2MCORE_BS_SERVER_ID,
                                                            (char*)securityInformationPtr->devicePKID,
                                                            securityInformationPtr->pskIdLen);
                    LOG_ARG("Store Bootstrap PskId result %d", storageResult);
                }

                if (!lwm2mcore_CredentialMatch(LWM2MCORE_CREDENTIAL_BS_SECRET_KEY,
                                               LWM2MCORE_BS_SERVER_ID,
                                               (char*)securityInformationPtr->secretKey))
                {
                    lwm2mcore_BackupCredential(LWM2MCORE_CREDENTIAL_BS_SECRET_KEY, LWM2MCORE_BS_SERVER_ID);

                    storageResult = lwm2mcore_SetCredential(LWM2MCORE_CREDENTIAL_BS_SECRET_KEY,
                                                            LWM2MCORE_BS_SERVER_ID,
                                                            (char*)securityInformationPtr->secretKey,
                                                            securityInformationPtr->pskLen);
                    LOG_ARG("Store Bootstrap Psk result %d", storageResult);
                }

                if (!lwm2mcore_CredentialMatch(LWM2MCORE_CREDENTIAL_BS_ADDRESS,
                                               LWM2MCORE_BS_SERVER_ID,
                                               (char*)securityInformationPtr->serverURI))
                {
                    lwm2mcore_BackupCredential(LWM2MCORE_CREDENTIAL_BS_ADDRESS, LWM2MCORE_BS_SERVER_ID);

                    storageResult = lwm2mcore_SetCredential(LWM2MCORE_CREDENTIAL_BS_ADDRESS,
                                                            LWM2MCORE_BS_SERVER_ID,
                                                            (char*)securityInformationPtr->serverURI,
                                                            strlen((const char *)securityInformationPtr->serverURI));
                    LOG_ARG("Store Bootstrap Addr result %d", storageResult);
                }
            }
        }
        else
        {
            LOG_ARG("Device management: PskIdLen %d PskLen %d Addr len %d",
                    securityInformationPtr->pskIdLen,
                    securityInformationPtr->pskLen,
                    strlen((const char *)securityInformationPtr->serverURI));

            /* In case of non-secure connection, pskIdLen and pskLen can be 0 */
            if ((securityInformationPtr->pskIdLen) && (LWM2MCORE_ERR_COMPLETED_OK == storageResult))
            {
                storageResult = lwm2mcore_SetCredential(LWM2MCORE_CREDENTIAL_DM_PUBLIC_KEY,
                                                        securityInformationPtr->data.serverId,
                                                        (char*)securityInformationPtr->devicePKID,
                                                        securityInformationPtr->pskIdLen);
                LOG_ARG("Store Device management (%d) PskId result %d",
                        securityInformationPtr->data.serverId, storageResult);
            }

            if ((securityInformationPtr->pskLen) && (LWM2MCORE_ERR_COMPLETED_OK == storageResult))
            {
                storageResult = lwm2mcore_SetCredential(LWM2MCORE_CREDENTIAL_DM_SECRET_KEY,
                                                        securityInformationPtr->data.serverId,
                                                        (char*)securityInformationPtr->secretKey,
                                                        securityInformationPtr->pskLen);
                LOG_ARG("Store Device management (%d) Psk result %d",
                        securityInformationPtr->data.serverId, storageResult);
            }

            if ((strlen((const char *)securityInformationPtr->serverURI))
             && (LWM2MCORE_ERR_COMPLETED_OK == storageResult))
            {
                storageResult = lwm2mcore_SetCredential(LWM2MCORE_CREDENTIAL_DM_ADDRESS,
                                                        securityInformationPtr->data.serverId,
                                                        (char*)securityInformationPtr->serverURI,
                                                        strlen((const char *)
                                                               securityInformationPtr->serverURI));
                LOG_ARG("Store Device management (%d) Addr result %d",
                        securityInformationPtr->data.serverId, storageResult);
            }
        }

        securityInformationPtr = securityInformationPtr->nextPtr;
    }

    if (LWM2MCORE_ERR_COMPLETED_OK == storageResult)
    {
        result = true;
    }
    LOG_ARG("credentials storage: %d", result);

    /* Set the bootstrap configuration */
    omanager_StoreBootstrapConfiguration(bsConfigPtr);
    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to restore bootstrap credentials from backup
 *
 * @return
 *  - @c true in case of success
 *  - @c false in case of failure
 */
//--------------------------------------------------------------------------------------------------
bool omanager_RestoreBsCredentials
(
    void
)
{
    lwm2mcore_Sid_t restoreResult = LWM2MCORE_ERR_COMPLETED_OK;

    restoreResult = lwm2mcore_RestoreCredential(LWM2MCORE_CREDENTIAL_BS_PUBLIC_KEY, LWM2MCORE_BS_SERVER_ID);

    if (restoreResult != LWM2MCORE_ERR_COMPLETED_OK)
    {
        return false;
    }

    restoreResult = lwm2mcore_RestoreCredential(LWM2MCORE_CREDENTIAL_BS_SECRET_KEY, LWM2MCORE_BS_SERVER_ID);

    if (restoreResult != LWM2MCORE_ERR_COMPLETED_OK)
    {
        return false;
    }

    restoreResult = lwm2mcore_RestoreCredential(LWM2MCORE_CREDENTIAL_BS_ADDRESS, LWM2MCORE_BS_SERVER_ID);

    if (restoreResult != LWM2MCORE_ERR_COMPLETED_OK)
    {
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function for the server SMS parameters
 * Object: 0 - Security
 * Resources: 6, 7, 8, 9
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int omanager_SmsDummy
(
    lwm2mcore_Uri_t* uriPtr,                ///< [IN] uriPtr represents the requested operation and
                                            ///< object/resource
    char* bufferPtr,                        ///< [INOUT] data buffer for information
    size_t len                             ///< [IN] length of input buffer
)
{
    (void)uriPtr;
    (void)bufferPtr;
    (void)len;

    if ((NULL == uriPtr) || (NULL == bufferPtr))
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    if ((uriPtr->op & (LWM2MCORE_OP_READ | LWM2MCORE_OP_WRITE)) == 0)
    {
        return LWM2MCORE_ERR_OP_NOT_SUPPORTED;
    }

    return LWM2MCORE_ERR_COMPLETED_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 *                                  OBJECT 1: SERVER
 */
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Function to write a resource of object 1
 * Object: 1 - Server
 * Resource: all
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int omanager_WriteServerObj
(
    lwm2mcore_Uri_t* uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource
    char* bufferPtr,                    ///< [INOUT] data buffer for information
    size_t len                          ///< [IN] length of input buffer
)
{
    int sID = LWM2MCORE_ERR_GENERAL_ERROR;
    uint32_t lifetime;
    ConfigBootstrapFile_t* bsConfigPtr;
    ConfigServerObject_t* serverInformationPtr;

    if ((NULL == uriPtr) || (NULL == bufferPtr))
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    if ((uriPtr->op & LWM2MCORE_OP_WRITE) == 0)
    {
        return LWM2MCORE_ERR_OP_NOT_SUPPORTED;
    }

    bsConfigPtr = omanager_GetBootstrapConfiguration();
    if (!bsConfigPtr)
    {
        return LWM2MCORE_ERR_GENERAL_ERROR;
    }

    serverInformationPtr = omanager_GetBootstrapConfigurationServerInstance(bsConfigPtr,
                                                                            uriPtr->oiid);
    if (!serverInformationPtr)
    {
        /* Create new serverInformationPtr */
        serverInformationPtr = (ConfigServerObject_t*)lwm2m_malloc(sizeof(ConfigServerObject_t));
        LWM2MCORE_ASSERT(serverInformationPtr);
        memset(serverInformationPtr, 0, sizeof(ConfigServerObject_t));
        bsConfigPtr->serverObjectNumber++;

        serverInformationPtr->data.serverObjectInstanceId = uriPtr->oiid;
        omanager_AddBootstrapConfigurationServer(bsConfigPtr, serverInformationPtr);
    }

    switch (uriPtr->rid)
    {
        /* Resource 0: Server short ID */
        case LWM2MCORE_SERVER_SHORT_ID_RID:
            serverInformationPtr->data.serverId =
                                (uint16_t)omanager_BytesToInt((const char*)bufferPtr, len);
            sID = LWM2MCORE_ERR_COMPLETED_OK;
            break;

        /* Resource 1: Server lifetime */
        case LWM2MCORE_SERVER_LIFETIME_RID:
            lifetime = (uint32_t)omanager_BytesToInt((const char*)bufferPtr, len);
            LOG_ARG("set lifetime %d", lifetime);
            if (!smanager_IsBootstrapConnection())
            {
                sID = lwm2mcore_SetPollingTimer(lifetime);
                if (LWM2MCORE_ERR_COMPLETED_OK == sID)
                {
                    serverInformationPtr->data.lifetime = lifetime;
                }
            }
            else
            {
                /* In case of bootstrap, the configuration will be stored at the bootstrap end */
                serverInformationPtr->data.lifetime = lifetime;
                sID = LWM2MCORE_ERR_COMPLETED_OK;
            }
            break;

        /* Resource 2: Server default minimum period */
        case LWM2MCORE_SERVER_DEFAULT_MIN_PERIOD_RID:
            serverInformationPtr->data.defaultPmin =
                                (uint16_t)omanager_BytesToInt((const char*)bufferPtr, len);
            sID = LWM2MCORE_ERR_COMPLETED_OK;
            break;

        /* Resource 3: Server default minimum period */
        case LWM2MCORE_SERVER_DEFAULT_MAX_PERIOD_RID:
            serverInformationPtr->data.defaultPmax =
                                    (uint16_t)omanager_BytesToInt((const char*)bufferPtr, len);
            sID = LWM2MCORE_ERR_COMPLETED_OK;
            break;

        /* Resource 5: Disable timeout */
        case LWM2MCORE_SERVER_DISABLE_TIMEOUT_RID:
            serverInformationPtr->data.disableTimeout =
                                    (uint32_t)omanager_BytesToInt((const char*)bufferPtr, len);
            sID = LWM2MCORE_ERR_COMPLETED_OK;
            break;

        /* Resource 6: Notification storing when disabled or offline */
        case LWM2MCORE_SERVER_STORE_NOTIF_WHEN_OFFLINE_RID:
            serverInformationPtr->data.isNotifStored =
                                    (bool)omanager_BytesToInt((const char*)bufferPtr, len);
            sID = LWM2MCORE_ERR_COMPLETED_OK;
            break;

        /* Resource 7: Binding */
        case LWM2MCORE_SERVER_BINDING_MODE_RID:
            memset(serverInformationPtr->data.bindingMode, 0, LWM2MCORE_BINDING_STR_MAX_LEN);
            memcpy(serverInformationPtr->data.bindingMode, (uint8_t*)bufferPtr, len);
            sID = LWM2MCORE_ERR_COMPLETED_OK;
            break;

        default:
            sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            break;
    }

    /* Write server object in platform storage only in case of device management
     * For bootstrap, the configuration is stored at the end of bootstap
     */
    if ((LWM2MCORE_ERR_COMPLETED_OK == sID) && (false == smanager_IsBootstrapConnection()))
    {
        omanager_StoreBootstrapConfiguration(bsConfigPtr);
    }

    return sID;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to read a resource of object 1
 * Object: 1 - Server
 * Resource: All
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int omanager_ReadServerObj
(
    lwm2mcore_Uri_t* uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource
    char* bufferPtr,                    ///< [INOUT] data buffer for information
    size_t* lenPtr,                     ///< [INOUT] length of input buffer and length of the
                                        ///< returned data
    valueChangedCallback_t changedCb    ///< [IN] callback for notification
)
{
    int sID = LWM2MCORE_ERR_GENERAL_ERROR;
    ConfigBootstrapFile_t* bsConfigPtr;
    ConfigServerObject_t* serverInformationPtr;

    (void)changedCb;

    if ((NULL == uriPtr) || (NULL == bufferPtr) || (NULL == lenPtr))
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    if ((uriPtr->op & LWM2MCORE_OP_READ) == 0)
    {
        return LWM2MCORE_ERR_OP_NOT_SUPPORTED;
    }

    bsConfigPtr = omanager_GetBootstrapConfiguration();
    if (!bsConfigPtr)
    {
        return LWM2MCORE_ERR_GENERAL_ERROR;
    }

    serverInformationPtr = omanager_GetBootstrapConfigurationServerInstance(bsConfigPtr,
                                                                            uriPtr->oiid);
    if (!serverInformationPtr)
    {
        LOG("serverInformationPtr NULL");
        return LWM2MCORE_ERR_GENERAL_ERROR;
    }

    switch (uriPtr->rid)
    {
        /* Resource 0: Server short ID */
        case LWM2MCORE_SERVER_SHORT_ID_RID:
            *lenPtr = omanager_FormatValueToBytes((uint8_t*) bufferPtr,
                                         &(serverInformationPtr->data.serverId),
                                         sizeof(serverInformationPtr->data.serverId),
                                         false);
            sID = LWM2MCORE_ERR_COMPLETED_OK;
            break;

        /* Resource 1: Server lifetime */
        case LWM2MCORE_SERVER_LIFETIME_RID:
        {
            serverInformationPtr->data.lifetime = (serverInformationPtr->data.lifetime == 0)
                                                  ? LWM2MCORE_LIFETIME_VALUE_DISABLED
                                                  : serverInformationPtr->data.lifetime;

           *lenPtr = omanager_FormatValueToBytes((uint8_t*) bufferPtr,
                                         &(serverInformationPtr->data.lifetime),
                                         sizeof(serverInformationPtr->data.lifetime),
                                         false);
            LOG_ARG("lifetime read %d", serverInformationPtr->data.lifetime);
            sID = LWM2MCORE_ERR_COMPLETED_OK;
            break;
        }

        /* Resource 2: Server default minimum period */
        case LWM2MCORE_SERVER_DEFAULT_MIN_PERIOD_RID:
            *lenPtr = omanager_FormatValueToBytes((uint8_t*) bufferPtr,
                                         &(serverInformationPtr->data.defaultPmin),
                                         sizeof(serverInformationPtr->data.defaultPmin),
                                         false);
            sID = LWM2MCORE_ERR_COMPLETED_OK;
            break;

        /* Resource 3: Server default minimum period */
        case LWM2MCORE_SERVER_DEFAULT_MAX_PERIOD_RID:
            *lenPtr = omanager_FormatValueToBytes((uint8_t*) bufferPtr,
                                         &(serverInformationPtr->data.defaultPmax),
                                         sizeof(serverInformationPtr->data.defaultPmax),
                                         false);
            sID = LWM2MCORE_ERR_COMPLETED_OK;
            break;

        /* Resource 5: Disable timeout */
        case LWM2MCORE_SERVER_DISABLE_TIMEOUT_RID:
            *lenPtr = omanager_FormatValueToBytes((uint8_t*) bufferPtr,
                                         &(serverInformationPtr->data.disableTimeout),
                                         sizeof(serverInformationPtr->data.disableTimeout),
                                         false);
            sID = LWM2MCORE_ERR_COMPLETED_OK;
            break;

        /* Resource 6: Notification storing when disabled or offline */
        case LWM2MCORE_SERVER_STORE_NOTIF_WHEN_OFFLINE_RID:
            *lenPtr = omanager_FormatValueToBytes((uint8_t*) bufferPtr,
                                         &(serverInformationPtr->data.isNotifStored),
                                         sizeof(serverInformationPtr->data.isNotifStored),
                                         false);
            sID = LWM2MCORE_ERR_COMPLETED_OK;
            break;

        /* Resource 7: Binding */
        case LWM2MCORE_SERVER_BINDING_MODE_RID:
            *lenPtr = snprintf(bufferPtr, *lenPtr, "%s", serverInformationPtr->data.bindingMode);
            sID = LWM2MCORE_ERR_COMPLETED_OK;
            break;

        default:
            sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            break;
    }
    return sID;
}

//--------------------------------------------------------------------------------------------------
/**
 * Sets the lifetime in the server configuration and saves it to platform memory
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the lifetime is not correct
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t omanager_SetLifetime
(
    uint32_t    lifetime,   ///< [IN] lifetime in seconds
    bool        storage     ///< [IN] Indicates if the configuration needs to be stored
)
{
    ConfigBootstrapFile_t* bsConfigPtr;
    lwm2mcore_Sid_t result;

    LOG_ARG("Set Lifetime to %d sec", lifetime);
    if (lwm2mcore_CheckLifetimeLimit(lifetime) != true)
    {
        LOG("Lifetime not in good range");
        return LWM2MCORE_ERR_INCORRECT_RANGE;
    }

    /* Change the configuration entry in file system */
    if (storage)
    {
        /* Allocate memory to read config file */
        bsConfigPtr = (ConfigBootstrapFile_t*)lwm2m_malloc(sizeof(ConfigBootstrapFile_t));
        LWM2MCORE_ASSERT(bsConfigPtr);
        memset(bsConfigPtr, 0, sizeof(ConfigBootstrapFile_t));

        /* Load configuration from file system */
        if (false == omanager_LoadBootstrapConfiguration(bsConfigPtr, false))
        {
            return LWM2MCORE_ERR_GENERAL_ERROR;
        }

        /* Update lifetime in file system */
        result = UpdateLifetime(bsConfigPtr, lifetime, storage);

        /* Free memory allocated */
        lwm2m_free(bsConfigPtr);

        if (LWM2MCORE_ERR_COMPLETED_OK != result)
        {
            LOG("Failed to update lifetime in storage");
            return result;
        }
    }

    /* Change the lifetime entry of server object in memory */
    bsConfigPtr = omanager_GetBootstrapConfiguration();

    result = UpdateLifetime(bsConfigPtr, lifetime, false);

    if (LWM2MCORE_ERR_COMPLETED_OK != result)
    {
        LOG("Failed to update lifetime in memory");
        return result;
    }

    smanager_SendUpdateAllServers(LWM2M_REG_UPDATE_LIFETIME);

    return LWM2MCORE_ERR_COMPLETED_OK;
}


//--------------------------------------------------------------------------------------------------
/**
 * Retrieves the lifetime from the server configuration
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t omanager_GetLifetime
(
    uint32_t* lifetimePtr                 ///< [OUT] lifetime in seconds
)
{
    ConfigBootstrapFile_t* bsConfigPtr;

    /* Allocate memory to read config file */
    bsConfigPtr = (ConfigBootstrapFile_t*)lwm2m_malloc(sizeof(ConfigBootstrapFile_t));
    LWM2MCORE_ASSERT(bsConfigPtr);
    memset(bsConfigPtr, 0, sizeof(ConfigBootstrapFile_t));

    /* Read the configuration from file system */
    if (false == omanager_LoadBootstrapConfiguration(bsConfigPtr, false))
    {
        lwm2m_free(bsConfigPtr);
        return LWM2MCORE_ERR_GENERAL_ERROR;
    }

    if (!(bsConfigPtr->serverPtr))
    {
        /* No DM server configuration */
        LOG("No DM server configuration");

        /* Default state is disabled */
        *lifetimePtr = LWM2MCORE_LIFETIME_VALUE_DISABLED;
    }
    else
    {
        *lifetimePtr = bsConfigPtr->serverPtr->data.lifetime;
    }
    lwm2m_free(bsConfigPtr->securityPtr);
    lwm2m_free(bsConfigPtr->serverPtr);
    lwm2m_free(bsConfigPtr);

    LOG_ARG("Lifetime is %d seconds", *lifetimePtr);
    return LWM2MCORE_ERR_COMPLETED_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 *                                  OBJECT 2: ACCESS CONTROL LISTS
 */
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Function to write a resource of object 2
 * Object: 2 - ACL
 * Resource: all
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int omanager_WriteAclObj
(
    lwm2mcore_Uri_t* uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource
    char* bufferPtr,                    ///< [INOUT] data buffer for information
    size_t len                          ///< [IN] length of input buffer
)
{
    int sID;
    ConfigAclFile_t* AclConfigPtr;
    AclObjectInstance_t* aclObjectInstancePtr;

    if ((NULL == uriPtr) || (NULL == bufferPtr))
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    if (0 == (uriPtr->op & LWM2MCORE_OP_WRITE))
    {
        return LWM2MCORE_ERR_OP_NOT_SUPPORTED;
    }

    AclConfigPtr = omanager_GetAclConfiguration();
    if (!AclConfigPtr)
    {
        return LWM2MCORE_ERR_GENERAL_ERROR;
    }

    aclObjectInstancePtr = omanager_GetAclObjectInstance(AclConfigPtr, uriPtr->oiid);
    if (!aclObjectInstancePtr)
    {
        /* Create new aclObjectInstancePtr */
        aclObjectInstancePtr = (AclObjectInstance_t*)lwm2m_malloc(sizeof(AclObjectInstance_t));
        LWM2MCORE_ASSERT(aclObjectInstancePtr);
        memset(aclObjectInstancePtr, 0, sizeof(AclObjectInstance_t));
        AclConfigPtr->instanceNumber++;

        /* Set the object instance Id */
        aclObjectInstancePtr->aclObjectData.objInstId = uriPtr->oiid;
        omanager_AddAclObjectInstance(AclConfigPtr, aclObjectInstancePtr);
    }

    switch (uriPtr->rid)
    {
        /* Resource 0: Object ID */
        case LWM2M_ACL_OBJECTID_ID:
            aclObjectInstancePtr->aclObjectData.objectId =
                                        (uint16_t)omanager_BytesToInt((const char*)bufferPtr, len);
            sID = LWM2MCORE_ERR_COMPLETED_OK;
            break;

        /* Resource 1: Object instance ID */
        case LWM2M_ACL_OBJECT_INSTANCE_ID:
            aclObjectInstancePtr->aclObjectData.objectInstId =
                                        (uint16_t)omanager_BytesToInt((const char*)bufferPtr, len);
            sID = LWM2MCORE_ERR_COMPLETED_OK;
            break;

        /* Resource 2: ACL */
        case LWM2M_ACL_ACCESS_ID:
        {
            /* Search if the required resource instance Id already exists: update ACL */
            Acl_t* aclRiidPtr = omanager_GetAclFromAclOiidAndRiid(aclObjectInstancePtr,
                                                                  uriPtr->riid);
            if (aclRiidPtr)
            {
                /* Update the ACL */
                aclRiidPtr->acl.resInstId = uriPtr->riid;
                aclRiidPtr->acl.accCtrlValue =
                                        (uint16_t)omanager_BytesToInt((const char*)bufferPtr, len);
            }
            else
            {
                /* Add ACL to existing object instance Id */
                /* Allocate Acl resource instances list */
                aclRiidPtr = (Acl_t*)lwm2m_malloc(sizeof(Acl_t));

                LWM2MCORE_ASSERT(aclRiidPtr);
                memset(aclRiidPtr, 0, sizeof(Acl_t));
                aclRiidPtr->nextPtr = NULL;

                /* Copy the server Id (resource instance Id and access rights */
                aclRiidPtr->acl.resInstId = uriPtr->riid;
                aclRiidPtr->acl.accCtrlValue =
                                        (uint16_t)omanager_BytesToInt((const char*)bufferPtr, len);
                omanager_AddAclAccessRights(aclObjectInstancePtr, aclRiidPtr);
                aclObjectInstancePtr->aclObjectData.aclInstanceNumber++;
            }

            sID = LWM2MCORE_ERR_COMPLETED_OK;
        }
        break;

        /* Resource 3: ACL object instance owner */
        case LWM2M_ACL_OWNER_ID:
            aclObjectInstancePtr->aclObjectData.aclOwner =
                                        (uint16_t)omanager_BytesToInt((const char*)bufferPtr, len);
            sID = LWM2MCORE_ERR_COMPLETED_OK;
            break;

        default:
            sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            break;
    }

    /* Write ACL in platform storage only in case of device management
     * For bootstrap, the configuration is stored at the end of bootstap
     */
    if ((LWM2MCORE_ERR_COMPLETED_OK == sID) && (false == smanager_IsBootstrapConnection()))
    {
        omanager_StoreAclConfiguration();
    }
    return sID;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to read a resource of object 2
 * Object: 2 - ACL
 * Resource: All
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int omanager_ReadAclObj
(
    lwm2mcore_Uri_t* uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource
    char* bufferPtr,                    ///< [INOUT] data buffer for information
    size_t* lenPtr,                     ///< [INOUT] length of input buffer and length of the
                                        ///< returned data
    valueChangedCallback_t changedCb    ///< [IN] callback for notification
)
{
    int sID;
    ConfigAclFile_t* AclConfigPtr;
    AclObjectInstance_t* aclObjectInstancePtr;

    (void)changedCb;

    if ((NULL == uriPtr) || (NULL == bufferPtr) || (NULL == lenPtr))
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    if (0 == (uriPtr->op & LWM2MCORE_OP_READ))
    {
        return LWM2MCORE_ERR_OP_NOT_SUPPORTED;
    }

    AclConfigPtr = omanager_GetAclConfiguration();
    if (!AclConfigPtr)
    {
        return LWM2MCORE_ERR_GENERAL_ERROR;
    }

    aclObjectInstancePtr = omanager_GetAclObjectInstance(AclConfigPtr, uriPtr->oiid);
    if (!aclObjectInstancePtr)
    {
        LOG("aclObjectInstancePtr NULL");
        return LWM2MCORE_ERR_INCORRECT_RANGE;
    }

    switch (uriPtr->rid)
    {
        /* Resource 0: Object ID */
        case LWM2M_ACL_OBJECTID_ID:
            *lenPtr = omanager_FormatValueToBytes(
                                            (uint8_t*) bufferPtr,
                                            &(aclObjectInstancePtr->aclObjectData.objectId),
                                            sizeof(aclObjectInstancePtr->aclObjectData.objectId),
                                            false);
            sID = LWM2MCORE_ERR_COMPLETED_OK;
            break;

        /* Resource 1: Object instance ID */
        case LWM2M_ACL_OBJECT_INSTANCE_ID:
            *lenPtr = omanager_FormatValueToBytes(
                                        (uint8_t*) bufferPtr,
                                        &(aclObjectInstancePtr->aclObjectData.objectInstId),
                                        sizeof(aclObjectInstancePtr->aclObjectData.objectInstId),
                                        false);
            sID = LWM2MCORE_ERR_COMPLETED_OK;
            break;

        /* Resource 2: ACL */
        case LWM2M_ACL_ACCESS_ID:
        {
            uint16_t acl = 0;
            sID = omanager_GetAclValueFromResourceInstance(aclObjectInstancePtr,
                                                           &(uriPtr->riid),
                                                           &acl);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = omanager_FormatValueToBytes((uint8_t*) bufferPtr,
                                                      &acl,
                                                      sizeof(acl),
                                                      false);
            }
        }
        break;

        /* Resource 3: ACL object instance owner */
        case LWM2M_ACL_OWNER_ID:
            *lenPtr = omanager_FormatValueToBytes(
                                            (uint8_t*) bufferPtr,
                                            &(aclObjectInstancePtr->aclObjectData.aclOwner),
                                            sizeof(aclObjectInstancePtr->aclObjectData.aclOwner),
                                            false);
            sID = LWM2MCORE_ERR_COMPLETED_OK;
            break;

        default:
            sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            break;
    }
    return sID;
}

//--------------------------------------------------------------------------------------------------
/**
 *                                  OBJECT 3: DEVICE
 */
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Function to write a resource of object 3
 * Object: 3 - Device
 * Resource: All with write operation
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
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int omanager_WriteDeviceObj
(
    lwm2mcore_Uri_t* uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource
    char* bufferPtr,                    ///< [INOUT] data buffer for information
    size_t len                          ///< [IN] length of input buffer
)
{
    int sID = LWM2MCORE_ERR_GENERAL_ERROR;

    (void)len;

    if ((!uriPtr) || (!bufferPtr))
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    /* Check that the object instance Id is in the correct range (only one object instance) */
    if (0 < uriPtr->oiid)
    {
        return LWM2MCORE_ERR_INCORRECT_RANGE;
    }

    /* Check that the operation is coherent */
    if (0 ==(uriPtr->op & LWM2MCORE_OP_WRITE))
    {
        return LWM2MCORE_ERR_OP_NOT_SUPPORTED;
    }

    switch (uriPtr->rid)
    {
        uint64_t source;

        /* Resource 13: Current time */
        case LWM2MCORE_DEVICE_CURRENT_TIME_RID:
            source = (uint64_t)omanager_BytesToInt((const char*)bufferPtr, len);
            LOG_ARG("Input length %d; input time %lld", len, source);
            sID = lwm2mcore_SetDeviceCurrentTime(source);
            break;

        /* Resource 16: Supported binding mode */
        case LWM2MCORE_DEVICE_SUPPORTED_BINDING_MODE_RID:
            sID = LWM2MCORE_ERR_NOT_YET_IMPLEMENTED;
            break;

        default:
            sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            break;
    }

    return sID;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to read a resource of object 3
 * Object: 3 - Device
 * Resource: All with read operation
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
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int omanager_ReadDeviceObj
(
    lwm2mcore_Uri_t* uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource
    char* bufferPtr,                    ///< [INOUT] data buffer for information
    size_t* lenPtr,                     ///< [INOUT] length of input buffer and length of the
                                        ///< returned data
    valueChangedCallback_t changedCb    ///< [IN] callback for notification
)
{
    int sID = LWM2MCORE_ERR_GENERAL_ERROR;

    (void)changedCb;

    if ((!uriPtr) || (!bufferPtr) || (!lenPtr))
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    /* Check that the object instance Id is in the correct range (only one object instance) */
    if (0 < uriPtr->oiid)
    {
        return LWM2MCORE_ERR_INCORRECT_RANGE;
    }

    /* Check that the operation is coherent */
    if (0 == (uriPtr->op & LWM2MCORE_OP_READ))
    {
        return LWM2MCORE_ERR_OP_NOT_SUPPORTED;
    }

    switch (uriPtr->rid)
    {
        /* Resource 0: Manufacturer */
        case LWM2MCORE_DEVICE_MANUFACTURER_RID:
            sID = lwm2mcore_GetDeviceManufacturer(bufferPtr, lenPtr);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = strlen(bufferPtr);
            }
            break;

        /* Resource 1: Device number */
        case LWM2MCORE_DEVICE_MODEL_NUMBER_RID:
            sID = lwm2mcore_GetDeviceModelNumber(bufferPtr, lenPtr);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = strlen(bufferPtr);
            }
            break;

        /* Resource 2: Serial number */
        case LWM2MCORE_DEVICE_SERIAL_NUMBER_RID:
            sID = lwm2mcore_GetDeviceSerialNumber(bufferPtr, lenPtr);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = strlen(bufferPtr);
            }
            break;

        /* Resource 3: Firmware */
        case LWM2MCORE_DEVICE_FIRMWARE_VERSION_RID:
            sID = lwm2mcore_GetDeviceFirmwareVersion(bufferPtr, lenPtr);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = strlen(bufferPtr);
            }
            break;

        /* Resource 9: Battery level */
        case LWM2MCORE_DEVICE_BATTERY_LEVEL_RID:
        {
            uint8_t batteryLevel = 0;
            sID = lwm2mcore_GetBatteryLevel(&batteryLevel);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = omanager_FormatValueToBytes((uint8_t*)bufferPtr,
                                             &batteryLevel,
                                             sizeof(batteryLevel),
                                             false);
            }
        }
        break;

        /* Resource 13: Current time */
        case LWM2MCORE_DEVICE_CURRENT_TIME_RID:
        {
            uint64_t currentTime = 0;
            sID = lwm2mcore_GetDeviceCurrentTime(&currentTime);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = omanager_FormatValueToBytes((uint8_t*)bufferPtr,
                                             &currentTime,
                                             sizeof(currentTime),
                                             false);
                LOG_ARG("Time retrieved %lld", currentTime);
            }
        }
        break;

        /* Resource 16: Supported binding mode */
        case LWM2MCORE_DEVICE_SUPPORTED_BINDING_MODE_RID:
            *lenPtr = snprintf(bufferPtr, *lenPtr, LWM2MCORE_BINDING_UDP_QUEUE);
            sID = LWM2MCORE_ERR_COMPLETED_OK;
            break;

        default:
            sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            break;
    }

    return sID;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to execute a resource of object 3
 * Object: 3 - Device
 * Resource: All with execute operation
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int omanager_ExecDeviceObj
(
    lwm2mcore_Uri_t* uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource
    char* bufferPtr,                    ///< [INOUT] data buffer for information
    size_t len                          ///< [IN] length of input buffer
)
{
    int sID = LWM2MCORE_ERR_GENERAL_ERROR;

    if ((NULL == uriPtr) || (len && (NULL == bufferPtr)))
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    /* Check that the object instance Id is in the correct range (only one object instance) */
    if (0 != uriPtr->oiid)
    {
        return LWM2MCORE_ERR_INCORRECT_RANGE;
    }

    /* Check that the operation is coherent */
    if (0 == (uriPtr->op & LWM2MCORE_OP_EXECUTE))
    {
        return LWM2MCORE_ERR_OP_NOT_SUPPORTED;
    }

    switch (uriPtr->rid)
    {
        /* Resource 4: Reboot */
        case LWM2MCORE_DEVICE_REBOOT_RID:
            /* Acknowledge the reboot request and launch the actual reboot later */
            sID = lwm2mcore_RebootDevice();
            break;

        default:
            sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            break;
    }

    return sID;
}

//--------------------------------------------------------------------------------------------------
/**
 *                              OBJECT 4: CONNECTIVITY MONITORING
 */
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Function to read a resource of object 4
 * Object: 4 - Connectivity monitoring
 * Resource: All
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
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int omanager_ReadConnectivityMonitoringObj
(
    lwm2mcore_Uri_t* uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource
    char* bufferPtr,                    ///< [INOUT] data buffer for information
    size_t* lenPtr,                     ///< [INOUT] length of input buffer and length of the
                                        ///< returned data
    valueChangedCallback_t changedCb    ///< [IN] callback for notification
)
{
    int sID = LWM2MCORE_ERR_GENERAL_ERROR;

    (void)changedCb;

    if ((!uriPtr) || (!bufferPtr) || (!lenPtr))
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    /* Check that the object instance Id is in the correct range (only one object instance) */
    if (0 < uriPtr->oiid)
    {
        return LWM2MCORE_ERR_INCORRECT_RANGE;
    }

    /* Check that the operation is coherent */
    if (0 == (uriPtr->op & LWM2MCORE_OP_READ))
    {
        return LWM2MCORE_ERR_OP_NOT_SUPPORTED;
    }

    switch (uriPtr->rid)
    {
        /* Resource 0: Network bearer */
        case LWM2MCORE_CONN_MONITOR_NETWORK_BEARER_RID:
        {
            lwm2mcore_networkBearer_enum_t networkBearer;
            sID = lwm2mcore_GetNetworkBearer(&networkBearer);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = omanager_FormatValueToBytes((uint8_t*)bufferPtr,
                                                      &networkBearer,
                                                      sizeof(networkBearer),
                                                      false);
            }
        }
        break;

        /* Resource 1: Available network bearer */
        case LWM2MCORE_CONN_MONITOR_AVAIL_NETWORK_BEARER_RID:
            /* Check that the resource instance Id is in the correct range */
            if (uriPtr->riid < CONN_MONITOR_AVAIL_NETWORK_BEARER_MAX_NB)
            {
                static lwm2mcore_networkBearer_enum_t
                       bearersList[CONN_MONITOR_AVAIL_NETWORK_BEARER_MAX_NB];
                static uint16_t bearersNb = 0;

                if (0 == uriPtr->riid)
                {
                    /* Reset the available network bearers list */
                    memset(bearersList, 0, sizeof(bearersList));
                    bearersNb = 0;

                    /* Retrieve the list */
                    sID = lwm2mcore_GetAvailableNetworkBearers(bearersList, &bearersNb);
                }
                else
                {
                    sID = LWM2MCORE_ERR_COMPLETED_OK;
                }

                if (LWM2MCORE_ERR_COMPLETED_OK == sID)
                {
                    if (uriPtr->riid < bearersNb)
                    {
                        *lenPtr = omanager_FormatValueToBytes((uint8_t*)bufferPtr,
                                                              &bearersList[uriPtr->riid],
                                                              sizeof(bearersList[uriPtr->riid]),
                                                              false);
                    }
                    else
                    {
                        *lenPtr = 0;
                    }
                }
            }
            else
            {
                sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            }
            break;

        /* Resource 2: Radio signal strength */
        case LWM2MCORE_CONN_MONITOR_RADIO_SIGNAL_STRENGTH_RID:
        {
            int32_t signalStrength;
            sID = lwm2mcore_GetSignalStrength(&signalStrength);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = omanager_FormatValueToBytes((uint8_t*)bufferPtr,
                                                      &signalStrength,
                                                      sizeof(signalStrength),
                                                      true);
            }
        }
        break;

        /* Resource 3: Link quality */
        case LWM2MCORE_CONN_MONITOR_LINK_QUALITY_RID:
        {
            int linkQuality;
            sID = lwm2mcore_GetLinkQuality(&linkQuality);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = omanager_FormatValueToBytes((uint8_t*)bufferPtr,
                                                      &linkQuality,
                                                      sizeof(linkQuality),
                                                      true);
            }
        }
        break;

        /* Resource 4: IP addresses */
        case LWM2MCORE_CONN_MONITOR_IP_ADDRESSES_RID:
            /* Check that the resource instance Id is in the correct range */
            if (uriPtr->riid < CONN_MONITOR_IP_ADDRESSES_MAX_NB)
            {
                static char ipAddrList[CONN_MONITOR_IP_ADDRESSES_MAX_NB]
                                      [CONN_MONITOR_IP_ADDR_MAX_BYTES];
                static uint16_t ipAddrNb = 0;

                if (0 == uriPtr->riid)
                {
                    /* Reset the IP addresses list */
                    memset(ipAddrList, 0, sizeof(ipAddrList));
                    ipAddrNb = 0;

                    /* Retrieve the list */
                    sID = lwm2mcore_GetIpAddresses(ipAddrList, &ipAddrNb);
                }
                else
                {
                    sID = LWM2MCORE_ERR_COMPLETED_OK;
                }

                if (LWM2MCORE_ERR_COMPLETED_OK == sID)
                {
                    if (uriPtr->riid < ipAddrNb)
                    {
                        *lenPtr = snprintf(bufferPtr,
                                           CONN_MONITOR_IP_ADDR_MAX_BYTES,
                                           "%s",
                                           ipAddrList[uriPtr->riid]);
                    }
                    else
                    {
                        *lenPtr = 0;
                    }
                }
            }
            else
            {
                sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            }
            break;

        /* Resource 5: Router IP addresses */
        case LWM2MCORE_CONN_MONITOR_ROUTER_IP_ADDRESSES_RID:
            /* Check that the resource instance Id is in the correct range */
            if (uriPtr->riid < CONN_MONITOR_ROUTER_IP_ADDRESSES_MAX_NB)
            {
                static char ipAddrList[CONN_MONITOR_ROUTER_IP_ADDRESSES_MAX_NB]
                                      [CONN_MONITOR_IP_ADDR_MAX_BYTES];
                static uint16_t ipAddrNb = 0;

                if (0 == uriPtr->riid)
                {
                    /* Reset the router IP addresses list */
                    memset(ipAddrList, 0, sizeof(ipAddrList));
                    ipAddrNb = 0;

                    /* Retrieve the list */
                    sID = lwm2mcore_GetRouterIpAddresses(ipAddrList, &ipAddrNb);
                }
                else
                {
                    sID = LWM2MCORE_ERR_COMPLETED_OK;
                }

                if (LWM2MCORE_ERR_COMPLETED_OK == sID)
                {
                    if (uriPtr->riid < ipAddrNb)
                    {
                        *lenPtr = snprintf(bufferPtr,
                                           CONN_MONITOR_IP_ADDR_MAX_BYTES,
                                           "%s",
                                           ipAddrList[uriPtr->riid]);
                    }
                    else
                    {
                        *lenPtr = 0;
                    }
                }
            }
            else
            {
                sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            }
            break;

        /* Resource 6: Link utilization */
        case LWM2MCORE_CONN_MONITOR_LINK_UTILIZATION_RID:
        {
            uint8_t linkUtilization;
            sID = lwm2mcore_GetLinkUtilization(&linkUtilization);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = omanager_FormatValueToBytes((uint8_t*)bufferPtr,
                                                      &linkUtilization,
                                                      sizeof(linkUtilization),
                                                      false);
            }
        }
        break;

        /* Resource 7: Access Point Name */
        case LWM2MCORE_CONN_MONITOR_APN_RID:
            /* Check that the resource instance Id is in the correct range */
            if (uriPtr->riid < CONN_MONITOR_APN_MAX_NB)
            {
                static char apnList[CONN_MONITOR_APN_MAX_NB][CONN_MONITOR_APN_MAX_BYTES];
                static uint16_t apnNb = 0;

                if (0 == uriPtr->riid)
                {
                    /* Reset the APN list */
                    memset(apnList, 0, sizeof(apnList));
                    apnNb = 0;

                    /* Retrieve the list */
                    sID = lwm2mcore_GetAccessPointNames(apnList, &apnNb);
                }
                else
                {
                    sID = LWM2MCORE_ERR_COMPLETED_OK;
                }

                if (LWM2MCORE_ERR_COMPLETED_OK == sID)
                {
                    if (uriPtr->riid < apnNb)
                    {
                        *lenPtr = snprintf(bufferPtr,
                                           CONN_MONITOR_APN_MAX_BYTES,
                                           "%s",
                                           apnList[uriPtr->riid]);
                    }
                    else
                    {
                        *lenPtr = 0;
                    }
                }
            }
            else
            {
                sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            }
            break;

        /* Resource 8: Cell ID */
        case LWM2MCORE_CONN_MONITOR_CELL_ID_RID:
        {
            uint32_t cellId;
            sID = lwm2mcore_GetCellId(&cellId);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = omanager_FormatValueToBytes((uint8_t*)bufferPtr,
                                                      &cellId,
                                                      sizeof(cellId),
                                                      false);
            }
        }
        break;

        /* Resource 9: Serving Mobile Network Code */
        case LWM2MCORE_CONN_MONITOR_SMNC_RID:
        {
            uint16_t mnc;
            sID = lwm2mcore_GetMncMcc(&mnc, NULL);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = omanager_FormatValueToBytes((uint8_t*)bufferPtr,
                                                      &mnc,
                                                      sizeof(mnc),
                                                      false);
            }
        }
        break;

        /* Resource 10: Serving Mobile Country Code */
        case LWM2MCORE_CONN_MONITOR_SMCC_RID:
        {
            uint16_t mcc;
            sID = lwm2mcore_GetMncMcc(NULL, &mcc);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = omanager_FormatValueToBytes((uint8_t*)bufferPtr,
                                                      &mcc,
                                                      sizeof(mcc),
                                                      false);
            }
        }
        break;

        default:
            sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            break;
    }

    return sID;
}

//--------------------------------------------------------------------------------------------------
/**
 *                                  OBJECT 5: FIRMWARE UPDATE
 */
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * The server sends a package URI to the LwM2M client
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 */
//--------------------------------------------------------------------------------------------------
static lwm2mcore_Sid_t SetUpdatePackageUri
(
    lwm2mcore_UpdateType_t type,    ///< [IN] Update type
    uint16_t instanceId,            ///< [IN] Instance Id (0 for FW, any value for SW)
    char* bufferPtr,                ///< [INOUT] Data buffer
    size_t len                      ///< [IN] Length of input buffer
)
{
    PackageDownloaderWorkspace_t workspace;
    int sID = LWM2MCORE_ERR_GENERAL_ERROR;

    // Update the package type.
    // This is the first step as error handling is dependent on update type.
    if (DWL_OK != ReadPkgDwlWorkspace(&workspace))
    {
        return LWM2MCORE_ERR_GENERAL_ERROR;
    }

    // Copy the updateType
    workspace.updateType = type;

    // Store the workspace
    if (DWL_OK != WritePkgDwlWorkspace(&workspace))
    {
       LOG("Error on saving workspace");
       return LWM2MCORE_ERR_GENERAL_ERROR;
    }

    if (!len)
    {
        /* If length is 0, the server requests to abort the previous download */
        return lwm2mcore_AbortDownload();
    }

    bool state = false;
    if (DWL_OK != GetTpfWorkspace(&state))
    {
        LOG("Unable to get the TPF state");
        state = false;
    }

    /* Check if a package is under download except in TPF mode */
    if (DWL_OK == ReadPkgDwlWorkspace(&workspace))
    {
        if ((LWM2MCORE_FW_UPDATE_TYPE == workspace.updateType)
         && (!strncmp(workspace.url,
                     bufferPtr,
                     (len > strlen(workspace.url)) ? len : strlen(workspace.url)))
         && (!state))
        {
            LOG("Same package URI is already stored");
            return LWM2MCORE_ERR_COMPLETED_OK;
        }

        /* For interoperability:
         * If a new package URI is received while a package URI is already stored (which means that
         * the package is under download (the URI is erased at package download end)), the download
         * should be aborted.
         */

        if (strlen(workspace.url))
        {
            LOG("Need to abort download");
            lwm2mcore_AbortDownload();
        }
    }

    lwm2mcore_DeletePackageDownloaderResumeInfo();
    lwm2mcore_CleanStaleData(workspace.updateType);

    if (LWM2MCORE_ERR_COMPLETED_OK == downloader_InitializeDownload(type,
                                                                    instanceId,
                                                                    bufferPtr,
                                                                    len))
    {
        /* Check if the package download timer is running */
        if (lwm2mcore_TimerIsRunning(LWM2MCORE_TIMER_DOWNLOAD))
        {
            lwm2mcore_TimerStop(LWM2MCORE_TIMER_DOWNLOAD);
        }

        if (false == lwm2mcore_TimerSet(LWM2MCORE_TIMER_DOWNLOAD,
                                        DOWNLOADER_PACKAGE_TIMER_VALUE,
                                        downloader_PackageDownloadHandler))
        {
            LOG("Error launching download package timer");
            switch (type)
            {
                case LWM2MCORE_FW_UPDATE_TYPE:
                    downloader_SetFwUpdateResult(LWM2MCORE_FW_UPDATE_RESULT_COMMUNICATION_ERROR);
                    break;

                case LWM2MCORE_SW_UPDATE_TYPE:
                    lwm2mcore_SetSwUpdateResult(LWM2MCORE_SW_UPDATE_RESULT_DEVICE_ERROR);
                    break;

                default:
                    LOG("Unhandled update type");
                    break;
            }
        }
        else
        {
            sID = LWM2MCORE_ERR_COMPLETED_OK;
        }
    }

    return (lwm2mcore_Sid_t)sID;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to write a resource of object 5
 * Object: 5 - Firmware update
 * Resource: all with write operation
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int omanager_WriteFwUpdateObj
(
    lwm2mcore_Uri_t* uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource
    char* bufferPtr,                    ///< [INOUT] data buffer for information
    size_t len                          ///< [IN] length of input buffer
)
{
    int sID = LWM2MCORE_ERR_GENERAL_ERROR;
    if ((NULL == uriPtr) || (NULL == bufferPtr))
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    /* Only one object instance */
    if (0 < uriPtr->oiid)
    {
        return LWM2MCORE_ERR_INCORRECT_RANGE;
    }

    if (0 == (uriPtr->op & LWM2MCORE_OP_WRITE))
    {
        return LWM2MCORE_ERR_OP_NOT_SUPPORTED;
    }

    switch (uriPtr->rid)
    {
        /* Resource 1: Package URI */
        case LWM2MCORE_FW_UPDATE_PACKAGE_URI_RID:
            if (LWM2MCORE_PACKAGE_URI_MAX_LEN < len)
            {
                sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            }
            else
            {
#ifdef LWM2M_EXTERNAL_DOWNLOADER
                /* For an external downloader, the DOWNLOAD_DETAILS notification needs to be sent to
                 * the application including the package URL without its size
                 */
#else
            sID = SetUpdatePackageUri(LWM2MCORE_FW_UPDATE_TYPE,
                                      uriPtr->oid,
                                      bufferPtr,
                                      len);
#endif /* !LWM2M_EXTERNAL_DOWNLOADER */
            }
            break;

        default:
            sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            break;
    }
    return sID;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to read a resource of object 5
 * Object: 5 - Firmware update
 * Resource: all with read operation
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int omanager_ReadFwUpdateObj
(
    lwm2mcore_Uri_t* uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource
    char* bufferPtr,                    ///< [INOUT] data buffer for information
    size_t* lenPtr,                     ///< [INOUT] length of input buffer and length of the
                                        ///< returned data
    valueChangedCallback_t changedCb    ///< [IN] not used for READ operation but for WRITE one
)
{
    int sID = LWM2MCORE_ERR_GENERAL_ERROR;

    (void)changedCb;

    if ((NULL == uriPtr) || (NULL == bufferPtr) || (NULL == lenPtr))
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    /* Only one object instance */
    if (0 < uriPtr->oiid)
    {
        return LWM2MCORE_ERR_INCORRECT_RANGE;
    }

    if (0 == (uriPtr->op & LWM2MCORE_OP_READ))
    {
        return LWM2MCORE_ERR_OP_NOT_SUPPORTED;
    }

    switch (uriPtr->rid)
    {
        /* Resource 1: Package URI */
        case LWM2MCORE_FW_UPDATE_PACKAGE_URI_RID:
        {
            PackageDownloaderWorkspace_t workspace;
            memset(&workspace, 0, sizeof(PackageDownloaderWorkspace_t));

            if (DWL_OK != ReadPkgDwlWorkspace(&workspace))
            {
                LOG("Error on reading workspace");
            }

            memset(bufferPtr, 0, *lenPtr);
            memcpy(bufferPtr, workspace.url, strlen(workspace.url));
            *lenPtr = strlen(workspace.url);
            sID = LWM2MCORE_ERR_COMPLETED_OK;
        }
        break;

        /* Resource 3: Update state */
        case LWM2MCORE_FW_UPDATE_UPDATE_STATE_RID:
        {
            lwm2mcore_FwUpdateState_t updateState;
            sID = downloader_GetFwUpdateState(&updateState);
            if (sID == LWM2MCORE_ERR_COMPLETED_OK)
            {
                *lenPtr = omanager_FormatValueToBytes((uint8_t*) bufferPtr,
                                             &updateState,
                                             sizeof(updateState),
                                             false);
            }
        }
        break;

        /* Resource 5: Update result */
        case LWM2MCORE_FW_UPDATE_UPDATE_RESULT_RID:
        {
            lwm2mcore_FwUpdateResult_t updateResult;
            sID = downloader_GetFwUpdateResult(&updateResult);
            if (sID == LWM2MCORE_ERR_COMPLETED_OK)
            {
                *lenPtr = omanager_FormatValueToBytes((uint8_t*) bufferPtr,
                                            &updateResult,
                                            sizeof(updateResult),
                                            false);
            }
            lwm2mcore_UpdateResultWasNotified(LWM2MCORE_FW_UPDATE_TYPE);
        }
        break;

        /* Resource 6: Package name */
        case LWM2MCORE_FW_UPDATE_PACKAGE_NAME_RID:
            sID = LWM2MCORE_ERR_OP_NOT_SUPPORTED;
            break;

        /* Resource 7: Package version */
        case LWM2MCORE_FW_UPDATE_PACKAGE_VERSION_RID:
            sID = LWM2MCORE_ERR_OP_NOT_SUPPORTED;
            break;

        /* Resource 8: Firmware update protocol support */
        case LWM2MCORE_FW_UPDATE_PROTO_SUPPORT_RID:
        {
            /* 2 protocols are supported: HTTP and HTTPS */
            lwm2mcore_FwUpdateProtocolSupport_t protocol[2] =
                        { LWM2MCORE_FW_UPDATE_HTTPS_1_1_PROTOCOL ,
                          LWM2MCORE_FW_UPDATE_HTTP_1_1_PROTOCOL};
            if ( 2 > (uriPtr->oiid))
            {
                /* Only support pull method using HTTP(S) */
                *lenPtr = omanager_FormatValueToBytes((uint8_t*)bufferPtr,
                                                      &(protocol[uriPtr->riid]),
                                                      sizeof(protocol[uriPtr->riid]),
                                                      false);
                sID = LWM2MCORE_ERR_COMPLETED_OK;
            }
            else
            {
                sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            }
        }
        break;

        /* Resource 9: Firmware update delivery method */
        case LWM2MCORE_FW_UPDATE_DELIVERY_METHOD_RID:
        {
            /* Only support pull method via HTTP(S) */
            lwm2mcore_FwUpdateDeliveryMethod_t method = LWM2MCORE_FW_UPDATE_PULL_METHOD;
            *lenPtr = omanager_FormatValueToBytes((uint8_t*)bufferPtr,
                                                  &method,
                                                  sizeof(method),
                                                  false);
            sID = LWM2MCORE_ERR_COMPLETED_OK;
        }
        break;

        default:
            sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            break;
    }
    return sID;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to execute a resource of object 5
 * Object: 5 - Firmware update
 * Resource: all with execute operation
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int omanager_ExecFwUpdate
(
    lwm2mcore_Uri_t* uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource
    char* bufferPtr,                    ///< [INOUT] data buffer for information
    size_t len                          ///< [IN] length of input buffer
)
{
    int sID = LWM2MCORE_ERR_GENERAL_ERROR;

    (void) bufferPtr;
    (void) len;

    /* BufferPtr can be null as per spec (OMA-TS-LightweightM2M-V1_0-20151214-C.pdf, E.6) */
    if (NULL == uriPtr)
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    if (uriPtr->oiid != 0)
    {
        /* Only one object instance */
        return LWM2MCORE_ERR_INCORRECT_RANGE;
    }

    /* Check if the related command is EXECUTE */
    if (0 == (uriPtr->op & LWM2MCORE_OP_EXECUTE))
    {
        return LWM2MCORE_ERR_OP_NOT_SUPPORTED;
    }

    switch (uriPtr->rid)
    {
        /* Resource 2: Update */
        case LWM2MCORE_FW_UPDATE_UPDATE_RID:
            sID = lwm2mcore_LaunchUpdate(LWM2MCORE_FW_UPDATE_TYPE, 0, NULL, 0);
            break;

        default:
            sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            break;
    }

    return sID;
}

//--------------------------------------------------------------------------------------------------
/**
 *                                  OBJECT 6: LOCATION
 */
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Function to read a resource of object 6
 * Object: 6 - Location
 * Resource: All
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
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int omanager_ReadLocationObj
(
    lwm2mcore_Uri_t* uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource
    char* bufferPtr,                    ///< [INOUT] data buffer for information
    size_t* lenPtr,                     ///< [INOUT] length of input buffer and length of the
                                        ///< returned data
    valueChangedCallback_t changedCb    ///< [IN] callback for notification
)
{
    int sID = LWM2MCORE_ERR_GENERAL_ERROR;

    (void)changedCb;

    if ((!uriPtr) || (!bufferPtr) || (!lenPtr))
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    /* Check that the object instance Id is in the correct range (only one object instance) */
    if (0 < uriPtr->oiid)
    {
        return LWM2MCORE_ERR_INCORRECT_RANGE;
    }

    /* Check that the operation is coherent */
    if (0 == (uriPtr->op & LWM2MCORE_OP_READ))
    {
        return LWM2MCORE_ERR_OP_NOT_SUPPORTED;
    }

    switch (uriPtr->rid)
    {
        /* Resource 0: Latitude */
        case LWM2MCORE_LOCATION_LATITUDE_RID:
            sID = lwm2mcore_GetLatitude(bufferPtr, lenPtr);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = strlen(bufferPtr);
            }
            break;

        /* Resource 1: Longitude */
        case LWM2MCORE_LOCATION_LONGITUDE_RID:
            sID = lwm2mcore_GetLongitude(bufferPtr, lenPtr);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = strlen(bufferPtr);
            }
            break;

        /* Resource 2: Altitude */
        case LWM2MCORE_LOCATION_ALTITUDE_RID:
            sID = lwm2mcore_GetAltitude(bufferPtr, lenPtr);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = strlen(bufferPtr);
            }
            break;

        /* Resource 4: Velocity */
        case LWM2MCORE_LOCATION_VELOCITY_RID:
            /* Build the velocity with direction, horizontal and vertical speeds */
            sID = BuildVelocity(bufferPtr, lenPtr);
            break;

        /* Resource 5: Timestamp */
        case LWM2MCORE_LOCATION_TIMESTAMP_RID:
        {
            uint64_t timestamp = 0;
            sID = lwm2mcore_GetLocationTimestamp(&timestamp);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = omanager_FormatValueToBytes((uint8_t*)bufferPtr,
                                             &timestamp,
                                             sizeof(timestamp),
                                             false);
            }
        }
        break;

        default:
            sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            break;
    }

    return sID;
}

//--------------------------------------------------------------------------------------------------
/**
 *                              OBJECT 7: CONNECTIVITY STATISTICS
 */
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Function to read a resource of object 7
 * Object: 7 - Connectivity statistics
 * Resource: All with read operation
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
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int omanager_ReadConnectivityStatisticsObj
(
    lwm2mcore_Uri_t* uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource
    char* bufferPtr,                    ///< [INOUT] data buffer for information
    size_t* lenPtr,                     ///< [INOUT] length of input buffer and length of the
                                        ///< returned data
    valueChangedCallback_t changedCb    ///< [IN] callback for notification
)
{
    int sID = LWM2MCORE_ERR_GENERAL_ERROR;

    (void)changedCb;

    if ((!uriPtr) || (!bufferPtr) || (!lenPtr))
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    /* Check that the object instance Id is in the correct range (only one object instance) */
    if (0 < uriPtr->oiid)
    {
        return LWM2MCORE_ERR_INCORRECT_RANGE;
    }

    /* Check that the operation is coherent */
    if (0 == (uriPtr->op & LWM2MCORE_OP_READ))
    {
        return LWM2MCORE_ERR_OP_NOT_SUPPORTED;
    }

    switch (uriPtr->rid)
    {
        /* Resource 0: SMS Tx counter */
        case LWM2MCORE_CONN_STATS_TX_SMS_COUNT_RID:
        {
            uint64_t smsTxCount;
            sID = lwm2mcore_GetSmsTxCount(&smsTxCount);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = omanager_FormatValueToBytes((uint8_t*)bufferPtr,
                                             &smsTxCount,
                                             sizeof(smsTxCount),
                                             false);
            }
        }
        break;

        /* Resource 1: SMS Rx counter */
        case LWM2MCORE_CONN_STATS_RX_SMS_COUNT_RID:
        {
            uint64_t smsRxCount;
            sID = lwm2mcore_GetSmsRxCount(&smsRxCount);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = omanager_FormatValueToBytes((uint8_t*)bufferPtr,
                                             &smsRxCount,
                                             sizeof(smsRxCount),
                                             false);
            }
        }
        break;

        /* Resource 2: Tx data */
        case LWM2MCORE_CONN_STATS_TX_DATA_COUNT_RID:
        {
            uint64_t txData;
            sID = lwm2mcore_GetTxData(&txData);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = omanager_FormatValueToBytes((uint8_t*)bufferPtr,
                                             &txData,
                                             sizeof(txData),
                                             false);
            }
        }
        break;

        /* Resource 3: Rx data */
        case LWM2MCORE_CONN_STATS_RX_DATA_COUNT_RID:
        {
            uint64_t rxData;
            sID = lwm2mcore_GetRxData(&rxData);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = omanager_FormatValueToBytes((uint8_t*)bufferPtr,
                                             &rxData,
                                             sizeof(rxData),
                                             false);
            }
        }
        break;

        default:
            sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            break;
    }

    return sID;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to execute a resource of object 7
 * Object: 7 - Connectivity statistics
 * Resource: All with execute operation
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int omanager_ExecConnectivityStatistics
(
    lwm2mcore_Uri_t* uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource
    char* bufferPtr,                    ///< [INOUT] data buffer for information
    size_t len                          ///< [IN] length of input buffer
)
{
    int sID = LWM2MCORE_ERR_GENERAL_ERROR;

    if ((NULL == uriPtr) || (len && (NULL == bufferPtr)))
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    /* Check that the object instance Id is in the correct range (only one object instance) */
    if (0 < uriPtr->oiid)
    {
        return LWM2MCORE_ERR_INCORRECT_RANGE;
    }

    /* Check that the operation is coherent */
    if (0 == (uriPtr->op & LWM2MCORE_OP_EXECUTE))
    {
        return LWM2MCORE_ERR_OP_NOT_SUPPORTED;
    }

    switch (uriPtr->rid)
    {
        /* Resource 6: Start */
        case LWM2MCORE_CONN_STATS_START_RID:
            sID = lwm2mcore_StartConnectivityCounters();
            break;

        /* Resource 7: Stop */
        case LWM2MCORE_CONN_STATS_STOP_RID:
            sID = lwm2mcore_StopConnectivityCounters();
            break;

        default:
            sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            break;
    }

    return sID;
}

//--------------------------------------------------------------------------------------------------
/**
 *                                  OBJECT 9: SOFTWARE UPDATE
 */
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Function to write a resource of object 9
 * Object: 9 - software update
 * Resource: all with write operation
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int omanager_WriteSwUpdateObj
(
    lwm2mcore_Uri_t* uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource
    char* bufferPtr,                    ///< [INOUT] data buffer for information
    size_t len                          ///< [IN] length of input buffer
)
{
    int sID = LWM2MCORE_ERR_GENERAL_ERROR;
    if ((NULL == uriPtr) || (NULL == bufferPtr))
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    /* Check if the related command is WRITE */
    if (0 == (uriPtr->op & LWM2MCORE_OP_WRITE))
    {
        return LWM2MCORE_ERR_OP_NOT_SUPPORTED;
    }

    switch (uriPtr->rid)
    {
        /* Resource 3: Package URI */
        case LWM2MCORE_SW_UPDATE_PACKAGE_URI_RID:
            if (LWM2MCORE_BUFFER_MAX_LEN < len)
            {
                sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            }
            else
            {
#ifdef LWM2M_EXTERNAL_DOWNLOADER
                /* For an external downloader, the DOWNLOAD_DETAILS notification needs to be sent to
                 * the application including the package URL without its size
                 */
#else
            sID = SetUpdatePackageUri(LWM2MCORE_SW_UPDATE_TYPE,
                                      uriPtr->oiid,
                                      bufferPtr,
                                      len);
#endif /* !LWM2M_EXTERNAL_DOWNLOADER */
            }
            break;

        /* Resource 8: Update Supported Objects */
        case LWM2MCORE_SW_UPDATE_UPDATE_SUPPORTED_OBJ_RID:
            if (LWM2MCORE_BUFFER_MAX_LEN < len)
            {
                sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            }
            else
            {
                sID = lwm2mcore_SetSwUpdateSupportedObjects(uriPtr->oiid,
                                (bool)omanager_BytesToInt((const char*)bufferPtr, len));
            }
            break;

        default:
            sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            break;
    }
    return sID;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to read a resource of object 9
 * Object: 9 - Software update
 * Resource: all with read operation
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int omanager_ReadSwUpdateObj
(
    lwm2mcore_Uri_t* uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource
    char* bufferPtr,                    ///< [INOUT] data buffer for information
    size_t* lenPtr,                     ///< [INOUT] length of input buffer and length of the
                                        ///< returned data
    valueChangedCallback_t changedCb    ///< [IN] not used for READ operation but for WRITE one
)
{
    int sID = LWM2MCORE_ERR_GENERAL_ERROR;

    (void)changedCb;

    if ((NULL == uriPtr) || (NULL == bufferPtr) || (NULL == lenPtr))
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    /* Check if the related command is READ */
    if (0 == (uriPtr->op & LWM2MCORE_OP_READ))
    {
        return LWM2MCORE_ERR_OP_NOT_SUPPORTED;
    }

    switch (uriPtr->rid)
    {
        /* Resource 0: package name */
        case LWM2MCORE_SW_UPDATE_PACKAGE_NAME_RID:
            sID = lwm2mcore_GetUpdatePackageName(LWM2MCORE_SW_UPDATE_TYPE,
                                                 uriPtr->oiid,
                                                 bufferPtr,
                                                 (uint32_t)*lenPtr);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = strlen(bufferPtr);
            }
            break;

        /* Resource 1: package version */
        case LWM2MCORE_SW_UPDATE_PACKAGE_VERSION_RID:
            sID = lwm2mcore_GetUpdatePackageVersion(LWM2MCORE_SW_UPDATE_TYPE,
                                                    uriPtr->oiid,
                                                    bufferPtr,
                                                    (uint32_t)*lenPtr);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = strlen(bufferPtr);
            }
            break;

        /* Resource 7: Update State */
        case LWM2MCORE_SW_UPDATE_UPDATE_STATE_RID:
        {
            uint8_t updateResult;
            sID = lwm2mcore_GetSwUpdateState(uriPtr->oiid, &updateResult);
            if (sID == LWM2MCORE_ERR_COMPLETED_OK)
            {
                *lenPtr = omanager_FormatValueToBytes((uint8_t*) bufferPtr,
                                             &updateResult,
                                             sizeof(updateResult),
                                             false);
            }
        }
        break;

        /* Resource 8: Update Supported Objects */
        case LWM2MCORE_SW_UPDATE_UPDATE_SUPPORTED_OBJ_RID:
        {
            bool value;
            sID = lwm2mcore_GetSwUpdateSupportedObjects(uriPtr->oiid, &value);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = omanager_FormatValueToBytes((uint8_t*) bufferPtr,
                                             &value,
                                             sizeof(value),
                                             false);
            }
        }
        break;

        /* Resource 9: Update result */
        case LWM2MCORE_SW_UPDATE_UPDATE_RESULT_RID:
        {
            uint8_t updateResult;
            sID = lwm2mcore_GetSwUpdateResult(uriPtr->oiid, &updateResult);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = omanager_FormatValueToBytes((uint8_t*) bufferPtr,
                                             &updateResult,
                                             sizeof(updateResult),
                                             false);
            }
        }
        break;

        /* Resource 12: Activation state */
        case LWM2MCORE_SW_UPDATE_ACTIVATION_STATE_RID:
        {
            bool value;
            sID = lwm2mcore_GetSwUpdateActivationState(uriPtr->oiid, &value);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = omanager_FormatValueToBytes((uint8_t*) bufferPtr,
                                             &value,
                                             sizeof(value),
                                             false);
            }
        }
        break;

        default:
            sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            break;
    }
    return sID;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to execute a resource of object 9
 * Object: 9 - Software update
 * Resource: all with execute operation
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int omanager_ExecSwUpdate
(
    lwm2mcore_Uri_t* uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource
    char* bufferPtr,                    ///< [INOUT] data buffer for information
    size_t len                          ///< [IN] length of input buffer
)
{
    int sID = LWM2MCORE_ERR_GENERAL_ERROR;

    if ((NULL == uriPtr) || ((NULL == bufferPtr) && len))
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    /* Check if the related command is EXECUTE */
    if (0 == (uriPtr->op & LWM2MCORE_OP_EXECUTE))
    {
        return LWM2MCORE_ERR_OP_NOT_SUPPORTED;
    }

    switch (uriPtr->rid)
    {
        /* Resource 4: Install */
        case LWM2MCORE_SW_UPDATE_INSTALL_RID:
            sID = lwm2mcore_LaunchUpdate(LWM2MCORE_SW_UPDATE_TYPE, uriPtr->oiid, bufferPtr, len);
            break;

        /* Resource 6: Uninstall */
        case LWM2MCORE_SW_UPDATE_UNINSTALL_RID:
            sID = lwm2mcore_LaunchSwUpdateUninstall(uriPtr->oiid, bufferPtr, len);
            break;

        /* Resource 10: Activate */
        case LWM2MCORE_SW_UPDATE_ACTIVATE_RID:
            sID = lwm2mcore_ActivateSoftware(true, uriPtr->oiid, bufferPtr, len);
            break;

        /* Resource 11: Deactivate */
        case LWM2MCORE_SW_UPDATE_DEACTIVATE_RID:
            sID = lwm2mcore_ActivateSoftware(false, uriPtr->oiid, bufferPtr, len);
            break;

        default:
            sID = LWM2MCORE_ERR_INCORRECT_RANGE;
        break;
    }

    return sID;
}

//--------------------------------------------------------------------------------------------------
/**
 *                              OBJECT 10241: SUBSCRIPTION
 */
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Function to read a resource of object 10241
 * Object: 10241 - Subscription
 * Resource: All with read operation
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
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int omanager_ReadSubscriptionObj
(
    lwm2mcore_Uri_t* uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource
    char* bufferPtr,                    ///< [INOUT] data buffer for information
    size_t* lenPtr,                     ///< [INOUT] length of input buffer and length of the
                                        ///< returned data
    valueChangedCallback_t changedCb    ///< [IN] callback for notification
)
{
    int sID = LWM2MCORE_ERR_GENERAL_ERROR;

    (void)changedCb;

    if ((!uriPtr) || (!bufferPtr) || (!lenPtr))
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    /* Check that the object instance Id is in the correct range (only one object instance) */
    if (0 < uriPtr->oiid)
    {
        return LWM2MCORE_ERR_INCORRECT_RANGE;
    }

    /* Check that the operation is coherent */
    if (0 == (uriPtr->op & LWM2MCORE_OP_READ))
    {
        return LWM2MCORE_ERR_OP_NOT_SUPPORTED;
    }

    switch (uriPtr->rid)
    {
        /* Resource 0: Module identity */
        case LWM2MCORE_SUBSCRIPTION_IMEI_RID:
            sID = lwm2mcore_GetDeviceImei(bufferPtr, lenPtr);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = strlen(bufferPtr);
            }
            break;

        /* Resource 1: SIM card identifier */
        case LWM2MCORE_SUBSCRIPTION_ICCID_RID:
            sID = lwm2mcore_GetIccid(bufferPtr, lenPtr);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = strlen(bufferPtr);
            }
            break;

        /* Resource 2: Subscription identity */
        case LWM2MCORE_SUBSCRIPTION_IDENTITY_RID:
            sID = lwm2mcore_GetSubscriptionIdentity(bufferPtr, lenPtr);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = strlen(bufferPtr);
            }
            break;

        /* Resource 3: Subscription phone number */
        case LWM2MCORE_SUBSCRIPTION_MSISDN_RID:
            sID = lwm2mcore_GetMsisdn(bufferPtr, lenPtr);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = strlen(bufferPtr);
            }
            break;

        /* Resource 5: Currently used SIM */
        case LWM2MCORE_SUBSCRIPTION_CURRENT_SIM_RID:
        {
            uint8_t currentSim = 0;
            sID = lwm2mcore_GetCurrentSimCard(&currentSim);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = omanager_FormatValueToBytes((uint8_t*)bufferPtr,
                                                      &currentSim,
                                                      sizeof(currentSim),
                                                      false);
            }
        }
        break;

        /* Resource 6: SIM current mode */
        case LWM2MCORE_SUBSCRIPTION_CURRENT_SIM_MODE_RID:
        {
            uint8_t mode = 0;
            sID = lwm2mcore_GetCurrentSimMode(&mode);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = omanager_FormatValueToBytes((uint8_t*)bufferPtr,
                                                      &mode,
                                                      sizeof(mode),
                                                      false);
            }
        }
        break;

        /* Resource 7: Last SIM switch status */
        case LWM2MCORE_SUBSCRIPTION_SIM_SWITCH_STATUS_RID:
        {
            uint8_t status = 0;
            sID = lwm2mcore_GetLastSimSwitchStatus(&status);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = omanager_FormatValueToBytes((uint8_t*)bufferPtr,
                                                      &status,
                                                      sizeof(status),
                                                      false);
            }
        }
        break;

        default:
            sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            break;
    }

    return sID;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to exec a resource of object 10241
 * Object: 10241 - Subscription
 * Resource: 4
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
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int omanager_ExecSubscriptionObj
(
    lwm2mcore_Uri_t* uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource
    char* bufferPtr,                    ///< [INOUT] data buffer for information
    size_t len                          ///< [IN] length of input buffer
)
{
    int sID = LWM2MCORE_ERR_GENERAL_ERROR;

    if ((NULL == uriPtr) || (len && (NULL == bufferPtr)))
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    /* Check that the object instance Id is in the correct range (only one object instance) */
    if (0 != uriPtr->oiid)
    {
        return LWM2MCORE_ERR_INCORRECT_RANGE;
    }

    /* Check that the operation is coherent */
    if (0 == (uriPtr->op & LWM2MCORE_OP_EXECUTE))
    {
        return LWM2MCORE_ERR_OP_NOT_SUPPORTED;
    }

    switch (uriPtr->rid)
    {
        /* Resource 4: SIM mode */
        case LWM2MCORE_SUBSCRIPTION_SIM_MODE_RID:
            sID = lwm2mcore_SetSimMode(bufferPtr, &len);
            break;

        default:
            sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            break;
    }

    return sID;
}

//--------------------------------------------------------------------------------------------------
/**
 *                          OBJECT 10242: EXTENDED CONNECTIVITY STATISTICS
 */
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Function to read a resource of object 10242
 * Object: 10242 - Extended connectivity statistics
 * Resource: All
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
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int omanager_ReadExtConnectivityStatsObj
(
    lwm2mcore_Uri_t* uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource
    char* bufferPtr,                    ///< [INOUT] data buffer for information
    size_t* lenPtr,                     ///< [INOUT] length of input buffer and length of the
                                        ///< returned data
    valueChangedCallback_t changedCb    ///< [IN] callback for notification
)
{
    int sID = LWM2MCORE_ERR_GENERAL_ERROR;

    (void)changedCb;

    if ((!uriPtr) || (!bufferPtr) || (!lenPtr))
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    /* Check that the object instance Id is in the correct range (only one object instance) */
    if (0 < uriPtr->oiid)
    {
        return LWM2MCORE_ERR_INCORRECT_RANGE;
    }

    /* Check that the operation is coherent */
    if (0 == (uriPtr->op & LWM2MCORE_OP_READ))
    {
        return LWM2MCORE_ERR_OP_NOT_SUPPORTED;
    }

    switch (uriPtr->rid)
    {
        /* Resource 0: Signal bars */
        case LWM2MCORE_EXT_CONN_STATS_SIGNAL_BARS_RID:
        {
            uint8_t signalBars = 0;
            sID = lwm2mcore_GetSignalBars(&signalBars);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = omanager_FormatValueToBytes((uint8_t*)bufferPtr,
                                                      &signalBars,
                                                      sizeof(signalBars),
                                                      false);
            }
        }
        break;

        /* Resource 1: Currently used cellular technology */
        case LWM2MCORE_EXT_CONN_STATS_CELLULAR_TECH_RID:
            sID = lwm2mcore_GetCellularTechUsed((char*)bufferPtr, lenPtr);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = strlen((char*)bufferPtr);
            }
            break;

        /* Resource 2: Roaming indicator */
        case LWM2MCORE_EXT_CONN_STATS_ROAMING_RID:
        {
            uint8_t roamingIndicator = 0;
            sID = lwm2mcore_GetRoamingIndicator(&roamingIndicator);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = omanager_FormatValueToBytes((uint8_t*)bufferPtr,
                                                      &roamingIndicator,
                                                      sizeof(roamingIndicator),
                                                      false);
            }
        }
        break;

        /* Resource 3: Ec/Io */
        case LWM2MCORE_EXT_CONN_STATS_ECIO_RID:
        {
            int32_t ecIo = 0;
            sID = lwm2mcore_GetEcIo(&ecIo);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = omanager_FormatValueToBytes((uint8_t*)bufferPtr,
                                                      &ecIo,
                                                      sizeof(ecIo),
                                                      true);
            }
        }
        break;

        /* Resource 4: RSRP */
        case LWM2MCORE_EXT_CONN_STATS_RSRP_RID:
        {
            int32_t rsrp = 0;
            sID = lwm2mcore_GetRsrp(&rsrp);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = omanager_FormatValueToBytes((uint8_t*)bufferPtr,
                                                      &rsrp,
                                                      sizeof(rsrp),
                                                      true);
            }
        }
        break;

        /* Resource 5: RSRQ */
        case LWM2MCORE_EXT_CONN_STATS_RSRQ_RID:
        {
            int32_t rsrq = 0;
            sID = lwm2mcore_GetRsrq(&rsrq);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = omanager_FormatValueToBytes((uint8_t*)bufferPtr,
                                                      &rsrq,
                                                      sizeof(rsrq),
                                                      true);
            }
        }
        break;

        /* Resource 6: RSCP */
        case LWM2MCORE_EXT_CONN_STATS_RSCP_RID:
        {
            int32_t rscp = 0;
            sID = lwm2mcore_GetRscp(&rscp);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = omanager_FormatValueToBytes((uint8_t*)bufferPtr,
                                                      &rscp,
                                                      sizeof(rscp),
                                                      true);
            }
        }
        break;

        /* Resource 7: Device temperature */
        case LWM2MCORE_EXT_CONN_STATS_TEMPERATURE_RID:
        {
            int32_t deviceTemperature = 0;
            sID = lwm2mcore_GetDeviceTemperature(&deviceTemperature);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = omanager_FormatValueToBytes((uint8_t*)bufferPtr,
                                                      &deviceTemperature,
                                                      sizeof(deviceTemperature),
                                                      true);
            }
        }
        break;

        /* Resource 8: Unexpected reset counter */
        case LWM2MCORE_EXT_CONN_STATS_UNEXPECTED_RESETS_RID:
        {
            uint32_t unexpectedResets = 0;
            sID = lwm2mcore_GetDeviceUnexpectedResets(&unexpectedResets);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = omanager_FormatValueToBytes((uint8_t*)bufferPtr,
                                                      &unexpectedResets,
                                                      sizeof(unexpectedResets),
                                                      false);
            }
        }
        break;

        /* Resource 9: Total reset counter */
        case LWM2MCORE_EXT_CONN_STATS_TOTAL_RESETS_RID:
        {
            uint32_t totalResets = 0;
            sID = lwm2mcore_GetDeviceTotalResets(&totalResets);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = omanager_FormatValueToBytes((uint8_t*)bufferPtr,
                                                      &totalResets,
                                                      sizeof(totalResets),
                                                      false);
            }
        }
        break;

        /* Resource 10: Location Area Code */
        case LWM2MCORE_EXT_CONN_STATS_LAC_RID:
        {
            uint32_t lac = 0;
            sID = lwm2mcore_GetLac(&lac);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = omanager_FormatValueToBytes((uint8_t*)bufferPtr,
                                                      &lac,
                                                      sizeof(lac),
                                                      false);
            }
        }
        break;

        /* Resource 11: Tracking Area Code (LTE) */
        case LWM2MCORE_EXT_CONN_STATS_TAC_RID:
        {
            uint16_t tac = 0;
            sID = lwm2mcore_GetServingCellLteTracAreaCode(&tac);
            if (LWM2MCORE_ERR_COMPLETED_OK == sID)
            {
                *lenPtr = omanager_FormatValueToBytes((uint8_t*)bufferPtr,
                                                      &tac,
                                                      sizeof(tac),
                                                      false);
            }
        }
        break;

        default:
            sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            break;
    }

    return sID;
}

//--------------------------------------------------------------------------------------------------
/**
 *                              OBJECT 10243: SSL certificates
 */
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Function to update the SSL certificates
 * Object: 10243 - SSL certificates
 * Resource: 0
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
int omanager_WriteSslCertif
(
    lwm2mcore_Uri_t* uriPtr,            ///< [IN] uri represents the requested operation and
                                        ///< object/resource
    char* bufferPtr,                    ///< [INOUT] data buffer for information
    size_t len                          ///< [IN] length of input buffer
)
{
    if (!uriPtr)
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    if ( (!(uriPtr->op & LWM2MCORE_OP_WRITE)) || (uriPtr->oiid) )
    {
        return LWM2MCORE_ERR_OP_NOT_SUPPORTED;
    }

    return lwm2mcore_UpdateSslCertificate(bufferPtr, len);
}

//--------------------------------------------------------------------------------------------------
/**
 * Function for not registered objects
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int omanager_OnUnlistedObject
(
    lwm2mcore_Uri_t* uriPtr,                ///< [IN] uriPtr represents the requested operation and
                                            ///< object/resource
    char* bufferPtr,                        ///< [INOUT] data buffer for information
    size_t* lenPtr,                         ///< [INOUT] length of input buffer and length of the
                                            ///< returned data
    valueChangedCallback_t changedCb        ///< [IN] callback function pointer for OBSERVE
                                            ///< operation
)
{
    (void)uriPtr;
    (void)bufferPtr;
    (void)lenPtr;
    (void)changedCb;

    return LWM2MCORE_ERR_NOT_YET_IMPLEMENTED;
}
