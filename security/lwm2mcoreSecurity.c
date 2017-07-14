/**
 * @file lwm2mcoreSecurity.c
 *
 * LWM2M core file for device security / credentials.
 *
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

/* include files */
#include <lwm2mcore/lwm2mcore.h>
#include <lwm2mcore/security.h>

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
)
{
    char buffer[256];
    size_t bufferLen = sizeof(buffer);
    lwm2mcore_Sid_t rcPsk, rcPskId, rcAddr;

    // Check if we have all information necessary to connect to DM server
    // i.e URL, public key and secret key
    bufferLen = sizeof(buffer);
    rcPskId = lwm2mcore_GetCredential((uint8_t)LWM2MCORE_CREDENTIAL_DM_PUBLIC_KEY,
                                      buffer,
                                      &bufferLen);

    bufferLen = sizeof(buffer);
    rcPsk = lwm2mcore_GetCredential((uint8_t)LWM2MCORE_CREDENTIAL_DM_SECRET_KEY,
                                    buffer,
                                    &bufferLen);

    bufferLen = sizeof(buffer);
    rcAddr = lwm2mcore_GetCredential((uint8_t)LWM2MCORE_CREDENTIAL_DM_ADDRESS,
                                     buffer,
                                     &bufferLen);

    if ((LWM2MCORE_ERR_COMPLETED_OK == rcPskId)
       &&(LWM2MCORE_ERR_COMPLETED_OK == rcPsk)
       &&(LWM2MCORE_ERR_COMPLETED_OK == rcAddr))
    {
        return LWM2MCORE_DM_CREDENTIAL_PROVISIONED;
    }

    // Check if we have all information necessary to connect to BS server
    // i.e URL, public key and secret key
    bufferLen = sizeof(buffer);
    rcPskId = lwm2mcore_GetCredential((uint8_t)LWM2MCORE_CREDENTIAL_BS_PUBLIC_KEY,
                                      buffer,
                                      &bufferLen);

    bufferLen = sizeof(buffer);
    rcPsk = lwm2mcore_GetCredential((uint8_t)LWM2MCORE_CREDENTIAL_BS_SECRET_KEY,
                                    buffer,
                                    &bufferLen);

    bufferLen = sizeof(buffer);
    rcAddr = lwm2mcore_GetCredential((uint8_t)LWM2MCORE_CREDENTIAL_BS_ADDRESS,
                                     buffer,
                                     &bufferLen);

    if ((LWM2MCORE_ERR_COMPLETED_OK == rcPskId)
       &&(LWM2MCORE_ERR_COMPLETED_OK == rcPsk)
       &&(LWM2MCORE_ERR_COMPLETED_OK == rcAddr))
    {
        return LWM2MCORE_BS_CREDENTIAL_PROVISIONED;
    }

    // No credentials provisioned.
    return LWM2MCORE_NO_CREDENTIAL_PROVISIONED;
}
