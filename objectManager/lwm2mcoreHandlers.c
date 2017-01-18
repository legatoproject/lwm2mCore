/**
 * @file lwm2mcoreHandlers.c
 *
 * client of the LWM2M stack
 *
 * Copyright (C) Sierra Wireless Inc. Use of this work is subject to license.
 *
 */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "lwm2mcore.h"
#include "lwm2mcoreObjectHandler.h"
#include "../objectManager/lwm2mcoreHandlers.h"
#include "../objectManager/lwm2mcoreObjects.h"
#include "../inc/lwm2mcorePortSecurity.h"
#include "internals.h"
#include "crypto.h"

//--------------------------------------------------------------------------------------------------
/**
 * Credential temporary RAM storage for BS and DM credentials: storage at the end of the bootstrap
 */
//--------------------------------------------------------------------------------------------------
uint8_t BsPskId[DTLS_PSK_MAX_CLIENT_IDENTITY_LEN];
uint16_t BsPskIdLen = 0;
uint8_t BsPsk[DTLS_PSK_MAX_KEY_LEN];
uint16_t BsPskLen = 0;
uint8_t BsAddr[LWM2MCORE_SERVER_URI_MAX_LEN];
uint8_t DmPskId[DTLS_PSK_MAX_CLIENT_IDENTITY_LEN];
uint16_t DmPskIdLen = 0;
uint8_t DmPsk[DTLS_PSK_MAX_KEY_LEN];
uint16_t DmPskLen = 0;
uint8_t DmAddr[LWM2MCORE_SERVER_URI_MAX_LEN];

//--------------------------------------------------------------------------------------------------
/**
 * Lifetime value (temporary value)
 */
//--------------------------------------------------------------------------------------------------
static uint32_t Lifetime = LWM2MCORE_LIFETIME_VALUE_DISABLED;

//--------------------------------------------------------------------------------------------------
/**
 * Convert to unsigned 16 bits integer to network bytes stream
 *
 * @return
 *  - converted size
 */
//--------------------------------------------------------------------------------------------------
static inline size_t FormatUint16ToBytes
(
    uint8_t *bytesPtr,      ///< [INOUT] the buffer contains data converted
    const uint16_t u        ///< [IN] the value to be converted
)
{
    bytesPtr[0] = (u >> 8) & 0xff;
    bytesPtr[1] = u & 0xff;
    return (sizeof(uint16_t));
}

//--------------------------------------------------------------------------------------------------
/**
 * Convert to unsigned 24 bits integer to network bytes stream
 *
 * @return
 *  - converted size
 */
//--------------------------------------------------------------------------------------------------
static inline size_t FormatUint24ToBytes
(
    uint8_t *bytesPtr,      ///< [INOUT] the buffer contains data converted
    const uint32_t u        ///< [IN] the value to be converted
)
{
    bytesPtr[0] = (u >> 16) & 0xff;
    bytesPtr[1] = (u >> 8) & 0xff;
    bytesPtr[2] = u & 0xff;
    return (3); /*uint24 is 3 bytes*/
}

//--------------------------------------------------------------------------------------------------
/**
 * Convert to unsigned 32 bits integer to network bytes stream
 *
 * @return
 *  - converted size
 */
//--------------------------------------------------------------------------------------------------
static inline size_t FormatUint32ToBytes
(
    uint8_t *bytesPtr,      ///< [INOUT] the buffer contains data converted
    const uint32_t u        ///< [IN] the value to be converted
)
{
    bytesPtr[0] = (u >> 24) & 0xff;
    bytesPtr[1] = (u >> 16) & 0xff;
    bytesPtr[2] = (u >> 8) & 0xff;
    bytesPtr[3] = u & 0xff;
    return (sizeof(uint32_t));
}

//--------------------------------------------------------------------------------------------------
/**
 * Convert to unsigned 48 bits integer to network bytes stream
 *
 * @return
 *  - converted size
 */
//--------------------------------------------------------------------------------------------------
static inline size_t FormatUint48ToBytes
(
    uint8_t *bytesPtr,      ///< [INOUT] the buffer contains data converted
    const uint64_t u        ///< [IN] the value to be converted
)
{
    FormatUint16ToBytes(bytesPtr, (uint16_t)((u >> 32) & 0xffff));
    FormatUint32ToBytes(bytesPtr + 2, u & 0xffffffff);
    return (6); /*uint48 is 6 bytes*/
}

//--------------------------------------------------------------------------------------------------
/**
 * Convert to unsigned 64 bits integer to network bytes stream
 *
 * @return
 *  - converted size
 */
//--------------------------------------------------------------------------------------------------
static inline size_t FormatUint64ToBytes
(
    uint8_t *bytesPtr,      ///< [INOUT] the buffer contains data converted
    const uint64_t u        ///< [IN] the value to be converted
)
{
    FormatUint32ToBytes(bytesPtr, (u >> 32) & 0xffffffff);
    FormatUint32ToBytes(bytesPtr + 4, u & 0xffffffff);
    return (sizeof(uint64_t));
}

//--------------------------------------------------------------------------------------------------
/**
 * Function used by object resource API to write value in buffer
 *
 * @return
 *  - converted size
 */
//--------------------------------------------------------------------------------------------------
// TODO: check if the Wakaama utils_encodeInt function does not make the same treatment
static size_t FormatValueToBytes
(
    uint8_t *bytes,         ///< [INOUT] bytes buffer in which u value will be written
    void* u,                ///< [INOUT] Data to be written
    uint32_t size,          ///< [IN] length to be written
    bool bSignedValue       ///< [IN] Indicates if u shall be considered like a signed value
)
{
    size_t lReturn = 0;
    size_t updatedSize = size;

    if ((NULL != bytes) && (NULL != u))
    {
        if (bSignedValue == false)
        {
            uint8_t* u8Value = 0;
            uint16_t* u16Value = 0;
            uint32_t* u32Value = 0;
            uint64_t* u64Value = 0;
            if (size == sizeof(uint8_t))
            {
                u8Value = (uint8_t*)u;
                if (*u8Value > 0x7F)
                {
                    /* Value shall be coded in 2 bytes */
                    u16Value = (uint16_t*)u;
                    updatedSize = sizeof (uint16_t);
                }
            }
            else if (size == sizeof(uint16_t))
            {
                u16Value = (uint16_t*)u;
                if (*u16Value > 0x7FFF)
                {
                    /* Value shall be coded in 4 bytes */
                    u32Value = (uint32_t*)u;
                    updatedSize = sizeof (uint32_t);
                }
                else if (*u16Value <= 0x7F)
                {
                    /* the value could be coded in 1 byte */
                    u8Value = (uint8_t*)u;
                    updatedSize = sizeof (uint8_t);
                }
            }
            else if (size == sizeof (uint32_t))
            {
                u32Value = (uint32_t*)u;
                if (*u32Value > 0x7FFFFFFF)
                {
                    /* Value shall be coded in 8 bytes */
                    u64Value = (uint64_t*)u;
                    updatedSize = sizeof (uint64_t);
                }
                else if (*u32Value <= 0x7F)
                {
                    /* the value could be coded in 1 byte */
                    u8Value = (uint8_t*)u;
                    updatedSize = sizeof (uint8_t);
                }
                else if (*u32Value <= 0x7FFF)
                {
                    /* the value could be coded in 2 bytes */
                    u16Value = (uint16_t*)u;
                    updatedSize = sizeof (uint16_t);
                }
            }
            else if (size == sizeof (uint64_t))
            {
                u64Value = (uint64_t*)u;
                if (*u64Value >> 63)
                {
                    updatedSize = 0;
                }
                else if (*u64Value <= 0x7F)
                {
                    /* the value could be coded in 1 byte */
                    u8Value = (uint8_t*)u;
                    updatedSize = sizeof (uint8_t);
                }
                else if (*u64Value <= 0x7FFF)
                {
                    /* the value could be coded in 2 bytes */
                    u16Value = (uint16_t*)u;
                    updatedSize = sizeof (uint16_t);
                }
                else if (*u64Value <= 0x7FFFFFFF)
                {
                    /* the value could be coded in 4 bytes */
                    u32Value = (uint32_t*)u;
                    updatedSize = sizeof (uint32_t);
                }
            }
            else
            {
                updatedSize = 0;
            }
        }

        switch (updatedSize)
        {
            case 1:
            {
                uint8_t* u8Value = (uint8_t*)u;
                lReturn = sizeof (uint8_t);
                bytes[ 0 ] = *u8Value;
            }
            break;

            case 2:
            {
                uint16_t* u16Value = (uint16_t*)u;
                lReturn = FormatUint16ToBytes (bytes, *u16Value);
            }
            break;

            case 4:
            {
                uint32_t* u32Value = (uint32_t*)u;
                lReturn = FormatUint32ToBytes (bytes, *u32Value);
            }
            break;

            case 8:
            {
                uint64_t* u64Value = (uint64_t*)u;
                lReturn = FormatUint64ToBytes (bytes, *u64Value);
            }
            break;

            default:
            {
                lReturn = -1;
            }
            break;
        }
    }

    return lReturn;
}


//--------------------------------------------------------------------------------------------------
/**
 *                  OBJECT 0: SECURITY
 */
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Function to read/write the server URI
 * Object: 0 - Security
 * Resource: 0
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treament succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int OnLWM2MSecurityServerURI
(
    lwm2mcore_uri_t *uriPtr,                ///< [IN] uriPtr represents the requested operation and
                                            ///< object/resource
    char *bufferPtr,                        ///< [INOUT] data buffer for information
    size_t *lenPtr,                         ///< [INOUT] length of input buffer and length of the
                                            ///< returned data
    value_changed_callback_t changed_cb     ///< [IN] not used for READ operation but for WRITE one
)
{
    int sID;

    if ((uriPtr == NULL) || (bufferPtr == NULL) || (lenPtr == NULL))
    {
        sID = LWM2MCORE_ERR_INVALID_ARG;
    }
    else
    {
        lwm2mcore_op_type_t supported_op_mask = LWM2MCORE_OP_READ | LWM2MCORE_OP_WRITE;

        /* Check that the server which tries to read/write is the bootstrap one
         * The Device Management server can not access to this resource
         */
        //TODO

        if (0 == (uriPtr->op & supported_op_mask))
        {
            sID = LWM2MCORE_ERR_OP_NOT_SUPPORTED;
        }
        else
        {
            /* Check that the object instance Id is in the correct range */
            if (uriPtr->oiid >= (LWM2MCORE_DM_SERVER_MAX_COUNT \
                               + LWM2MCORE_BOOTSRAP_SERVER_MAX_COUNT))
            {
                sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            }
            else
            {
                if (uriPtr->op & LWM2MCORE_OP_READ)
                {
                    /* Read operation */
                    if (LWM2MCORE_BS_SERVER_OIID == uriPtr->oiid)
                    {
                        /* Bootstrap server */
                        sID = os_portSecurityGetCredential (
                                                        (uint8_t)LWM2MCORE_CREDENTIAL_BS_ADDRESS,
                                                        bufferPtr,
                                                        lenPtr
                                                            );
                    }
                    else
                    {
                        /* Device Management server */
                        sID = os_portSecurityGetCredential (
                                                        (uint8_t)LWM2MCORE_CREDENTIAL_DM_ADDRESS,
                                                        bufferPtr,
                                                        lenPtr
                                                            );
                    }
                }
                else
                {
                    /* Write operation */
                    if (LWM2MCORE_SERVER_URI_MAX_LEN < *lenPtr )
                    {
                        sID = LWM2MCORE_ERR_INCORRECT_RANGE;
                    }
                    else
                    {
#ifdef CREDENTIALS_DEBUG
                        os_debug_data_dump ("Server URI write", bufferPtr, *lenPtr);
#endif
                        if (LWM2MCORE_BS_SERVER_OIID == uriPtr->oiid)
                        {
                            /* Bootstrap server */
                            memcpy(BsAddr, bufferPtr, *lenPtr);
                            sID = LWM2MCORE_ERR_COMPLETED_OK;
                        }
                        else
                        {
                            /* Device Management server */
                            memcpy(DmAddr, bufferPtr, *lenPtr);
                            sID = LWM2MCORE_ERR_COMPLETED_OK;
                        }
                    }
                }
            }
        }
    }
    return sID;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to read/write the server type
 * Object: 0 - Security
 * Resource: 1
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treament succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int OnLWM2MSecurityServerType
(
    lwm2mcore_uri_t *uriPtr,                ///< [IN] uriPtr represents the requested operation and
                                            ///< object/resource
    char *bufferPtr,                        ///< [INOUT] data buffer for information
    size_t *lenPtr,                         ///< [INOUT] length of input buffer and length of the
                                            ///< returned data
    value_changed_callback_t changed_cb     ///< [IN] not used for READ operation but for WRITE one
)
{
    int sID;

    if ((uriPtr == NULL) || (bufferPtr == NULL) || (lenPtr == NULL))
    {
        sID = LWM2MCORE_ERR_INVALID_ARG;
    }
    else
    {
         lwm2mcore_op_type_t supported_op_mask = LWM2MCORE_OP_READ | LWM2MCORE_OP_WRITE;

        /* Check that the server which tries to read/write is the bootstrap one
         * The Device Management server can not access to this resource
         */
        //TODO

        if (0 == (uriPtr->op & supported_op_mask))
        {
            sID = LWM2MCORE_ERR_OP_NOT_SUPPORTED;
        }
        else
        {
            /* Check that the object instance Id is in the correct range */
            if (uriPtr->oiid >= (LWM2MCORE_DM_SERVER_MAX_COUNT \
                               + LWM2MCORE_BOOTSRAP_SERVER_MAX_COUNT))
            {
                sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            }
            else
            {
                if (uriPtr->op & LWM2MCORE_OP_READ)
                {
                    /* Read operation */
                    if (LWM2MCORE_BS_SERVER_OIID == uriPtr->oiid)
                    {
                        /* Bootstrap server */
                        bufferPtr[0] = 1;
                    }
                    else
                    {
                        /* Device Management server */
                        bufferPtr[0] = 0;
                    }
                    *lenPtr = 1;
                    sID = LWM2MCORE_ERR_COMPLETED_OK;

                }
                else
                {
                    /* Write operation */
                    if (LWM2MCORE_BS_SERVER_OIID == uriPtr->oiid)
                    {
                        /* Bootstrap server */
                        sID = LWM2MCORE_ERR_COMPLETED_OK;
                    }
                    else
                    {
                        /* Device Management server */
                        sID = LWM2MCORE_ERR_COMPLETED_OK;
                    }
                }
            }
        }
    }
    return sID;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to read/write the server security mode
 * Object: 0 - Security
 * Resource: 2
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treament succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int OnLWM2MSecurityMode
(
    lwm2mcore_uri_t *uriPtr,                ///< [IN] uriPtr represents the requested operation and
                                            ///< object/resource
    char *bufferPtr,                        ///< [INOUT] data buffer for information
    size_t *lenPtr,                         ///< [INOUT] length of input buffer and length of the
                                            ///< returned data
    value_changed_callback_t changed_cb     ///< [IN] not used for READ operation but for WRITE one
)
{
    int sID;

    if ((uriPtr == NULL) || (bufferPtr == NULL) || (lenPtr == NULL))
    {
        sID = LWM2MCORE_ERR_INVALID_ARG;
    }
    else
    {
        lwm2mcore_op_type_t supported_op_mask = LWM2MCORE_OP_READ | LWM2MCORE_OP_WRITE;

        /* Check that the server which tries to read/write is the bootstrap one
         * The Device Management server can not access to this resource
         */
        //TODO

        if (0 == (uriPtr->op & supported_op_mask))
        {
            sID = LWM2MCORE_ERR_OP_NOT_SUPPORTED;
        }
        else
        {
            /* Check that the object instance Id is in the correct range */
            if (uriPtr->oiid >= (LWM2MCORE_DM_SERVER_MAX_COUNT \
                               + LWM2MCORE_BOOTSRAP_SERVER_MAX_COUNT))
            {
                sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            }
            else
            {
                if (uriPtr->op & LWM2MCORE_OP_READ)
                {
                    /* Read operation */
                    bufferPtr[0] = LWM2MCORE_SEC_PSK;
                    *lenPtr = 1;
                    sID = LWM2MCORE_ERR_COMPLETED_OK;

                }
                else
                {
                    /* Write operation */
                    sID = LWM2MCORE_ERR_COMPLETED_OK;
                }
            }
        }
    }
    return sID;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to read/write the device PSK Identity
 * Object: 0 - Security
 * Resource: 3
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treament succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int OnLWM2MSecurityDevicePKID
(
    lwm2mcore_uri_t *uriPtr,                ///< [IN] uriPtr represents the requested operation and
                                            ///< object/resource
    char *bufferPtr,                        ///< [INOUT] data buffer for information
    size_t *lenPtr,                         ///< [INOUT] length of input buffer and length of the
                                            ///< returned data
    value_changed_callback_t changed_cb     ///< [IN] not used for READ operation but for WRITE one
)
{
    int sID;

    if ((uriPtr == NULL) || (bufferPtr == NULL) || (lenPtr == NULL))
    {
        sID = LWM2MCORE_ERR_INVALID_ARG;
    }
    else
    {
        lwm2mcore_op_type_t supported_op_mask = LWM2MCORE_OP_READ | LWM2MCORE_OP_WRITE;

        /* Check that the server which tries to read/write is the bootstrap one
         * The Device Management server can not access to this resource
         */
        //TODO

        if (0 == (uriPtr->op & supported_op_mask))
        {
            sID = LWM2MCORE_ERR_OP_NOT_SUPPORTED;
        }
        else
        {
            /* Check that the object instance Id is in the correct range */
            if (uriPtr->oiid >= (LWM2MCORE_DM_SERVER_MAX_COUNT \
                               + LWM2MCORE_BOOTSRAP_SERVER_MAX_COUNT))
            {
                sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            }
            else
            {
                if (uriPtr->op & LWM2MCORE_OP_READ)
                {
                    /* Read operation */
                    if (LWM2MCORE_BS_SERVER_OIID == uriPtr->oiid)
                    {
                        /* Bootstrap server */
                        sID = os_portSecurityGetCredential (
                                                        (uint8_t)LWM2MCORE_CREDENTIAL_BS_PUBLIC_KEY,
                                                        bufferPtr,
                                                        lenPtr
                                                            );
                    }
                    else
                    {
                        /* Device Management server */
                        sID = os_portSecurityGetCredential (
                                                        (uint8_t)LWM2MCORE_CREDENTIAL_DM_PUBLIC_KEY,
                                                        bufferPtr,
                                                        lenPtr
                                                            );
                    }
#ifdef CREDENTIALS_DEBUG
                    os_debug_data_dump ("PSK ID read", bufferPtr, *lenPtr);
#endif
                }
                else
                {
                    /* Write operation */
#ifdef CREDENTIALS_DEBUG
                    os_debug_data_dump ("PSK ID write", bufferPtr, *lenPtr);
#endif
                    if (*lenPtr > DTLS_PSK_MAX_CLIENT_IDENTITY_LEN)
                    {
                        sID = LWM2MCORE_ERR_INCORRECT_RANGE;
                    }
                    else
                    {
                        if (LWM2MCORE_BS_SERVER_OIID == uriPtr->oiid)
                        {
                            /* Bootstrap server */
                            memcpy(BsPskId, bufferPtr, *lenPtr);
                            BsPskIdLen = *lenPtr;
                            sID = LWM2MCORE_ERR_COMPLETED_OK;
                        }
                        else
                        {
                            /* Device Management server */
                            memcpy(DmPskId, bufferPtr, *lenPtr);
                            DmPskIdLen = *lenPtr;
                            sID = LWM2MCORE_ERR_COMPLETED_OK;
                        }
                    }
                }
            }
        }
    }
    return sID;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to read/write the server public key
 * Object: 0 - Security
 * Resource: 4
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treament succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int OnLWM2MSecurityServerKey
(
    lwm2mcore_uri_t *uriPtr,                ///< [IN] uriPtr represents the requested operation and
                                            ///< object/resource
    char *bufferPtr,                        ///< [INOUT] data buffer for information
    size_t *lenPtr,                         ///< [INOUT] length of input buffer and length of the
                                            ///< returned data
    value_changed_callback_t changed_cb     ///< [IN] not used for READ operation but for WRITE one
)
{
    int sID;

    if ((uriPtr == NULL) || (bufferPtr == NULL) || (lenPtr == NULL))
    {
        sID = LWM2MCORE_ERR_INVALID_ARG;
    }
    else
    {
        lwm2mcore_op_type_t supported_op_mask = LWM2MCORE_OP_READ | LWM2MCORE_OP_WRITE;

        /* Check that the server which tries to read/write is the bootstrap one
         * The Device Management server can not access to this resource
         */
        //TODO

        if (0 == (uriPtr->op & supported_op_mask))
        {
            sID = LWM2MCORE_ERR_OP_NOT_SUPPORTED;
        }
        else
        {
            /* Check that the object instance Id is in the correct range */
            if (uriPtr->oiid >= (LWM2MCORE_DM_SERVER_MAX_COUNT \
                               + LWM2MCORE_BOOTSRAP_SERVER_MAX_COUNT))
            {
                sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            }
            else
            {
                if (uriPtr->op & LWM2MCORE_OP_READ)
                {
                    /* Read operation */
                    if (LWM2MCORE_BS_SERVER_OIID == uriPtr->oiid)
                    {
                        /* Bootstrap server */
                        sID = os_portSecurityGetCredential (
                                                (uint8_t)LWM2MCORE_CREDENTIAL_BS_SERVER_PUBLIC_KEY,
                                                bufferPtr,
                                                lenPtr
                                                            );
                    }
                    else
                    {
                        /* Device Management server */
                        sID = os_portSecurityGetCredential (
                                                (uint8_t)LWM2MCORE_CREDENTIAL_DM_SERVER_PUBLIC_KEY,
                                                bufferPtr,
                                                lenPtr
                                                            );
                    }
#ifdef CREDENTIALS_DEBUG
                    os_debug_data_dump ("Server key read", bufferPtr, *lenPtr);
#endif
                }
                else
                {
                    /* Write operation */
#ifdef CREDENTIALS_DEBUG
                    os_debug_data_dump ("Server key write", bufferPtr, *lenPtr);
#endif
                    if (*lenPtr > DTLS_PSK_MAX_CLIENT_IDENTITY_LEN)
                    {
                        sID = LWM2MCORE_ERR_INCORRECT_RANGE;
                    }
                    else
                    {
                        if (LWM2MCORE_BS_SERVER_OIID == uriPtr->oiid)
                        {
                            /* Bootstrap server
                             * Not used in PSK: respond OK
                             */
                            sID = LWM2MCORE_ERR_COMPLETED_OK;
                        }
                        else
                        {
                            /* Device Management server
                             * Not used in PSK: respond OK
                             */
                            sID = LWM2MCORE_ERR_COMPLETED_OK;
                        }
                    }
                }
            }
        }
    }
    return sID;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to read/write the device secret key
 * Object: 0 - Security
 * Resource: 5
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treament succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int OnLWM2MSecuritySecretKey
(
    lwm2mcore_uri_t *uriPtr,                ///< [IN] uriPtr represents the requested operation and
                                            ///< object/resource
    char *bufferPtr,                        ///< [INOUT] data buffer for information
    size_t *lenPtr,                         ///< [INOUT] length of input buffer and length of the
                                            ///< returned data
    value_changed_callback_t changed_cb     ///< [IN] not used for READ operation but for WRITE one
)
{
    int sID;

    if ((uriPtr == NULL) || (bufferPtr == NULL) || (lenPtr == NULL))
    {
        sID = LWM2MCORE_ERR_INVALID_ARG;
    }
    else
    {
        lwm2mcore_op_type_t supported_op_mask = LWM2MCORE_OP_READ | LWM2MCORE_OP_WRITE;

        /* Check that the server which tries to read/write is the bootstrap one
         * The Device Management server can not access to this resource
         */
        //TODO

        if (0 == (uriPtr->op & supported_op_mask))
        {
            sID = LWM2MCORE_ERR_OP_NOT_SUPPORTED;
        }
        else
        {
            /* Check that the object instance Id is in the correct range */
            if (uriPtr->oiid >= (LWM2MCORE_DM_SERVER_MAX_COUNT \
                               + LWM2MCORE_BOOTSRAP_SERVER_MAX_COUNT))
            {
                sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            }
            else
            {
                if (uriPtr->op & LWM2MCORE_OP_READ)
                {
                    /* Read operation */
                    if (LWM2MCORE_BS_SERVER_OIID == uriPtr->oiid)
                    {
                        /* Bootstrap server */
                        sID = os_portSecurityGetCredential (
                                                        (uint8_t)LWM2MCORE_CREDENTIAL_BS_SECRET_KEY,
                                                        bufferPtr,
                                                        lenPtr
                                                            );
                    }
                    else
                    {
                        /* Device Management server */
                        sID = os_portSecurityGetCredential (
                                                        (uint8_t)LWM2MCORE_CREDENTIAL_DM_SECRET_KEY,
                                                        bufferPtr,
                                                        lenPtr
                                                            );
                    }
#ifdef CREDENTIALS_DEBUG
                    os_debug_data_dump ("PSK secret read", bufferPtr, *lenPtr);
#endif
                }
                else
                {
                    /* Write operation */
#ifdef CREDENTIALS_DEBUG
                    os_debug_data_dump ("PSK secret write", bufferPtr, *lenPtr);
#endif
                    if (*lenPtr > DTLS_PSK_MAX_CLIENT_IDENTITY_LEN)
                    {
                        sID = LWM2MCORE_ERR_INCORRECT_RANGE;
                    }
                    else
                    {
                        if (LWM2MCORE_BS_SERVER_OIID == uriPtr->oiid)
                        {
                            /* Bootstrap server */
                            memcpy(BsPsk, bufferPtr, *lenPtr);
                            BsPskLen = *lenPtr;
                            sID = LWM2MCORE_ERR_COMPLETED_OK;
                        }
                        else
                        {
                            /* Device Management server */
                            memcpy(DmPsk, bufferPtr, *lenPtr);
                            DmPskLen = *lenPtr;
                            sID = LWM2MCORE_ERR_COMPLETED_OK;
                        }
                    }
                }
            }
        }
    }
    return sID;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function for the server SMS parameters
 * Object: 0 - Security
 * Resources: 6, 7, 8, 9
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treament succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int OnLWM2MSecuritySMSDummy
(
    lwm2mcore_uri_t *uriPtr,                ///< [IN] uriPtr represents the requested operation and
                                            ///< object/resource
    char *bufferPtr,                        ///< [INOUT] data buffer for information
    size_t *lenPtr,                         ///< [INOUT] length of input buffer and length of the
                                            ///< returned data
    value_changed_callback_t changed_cb     ///< [IN] not used for READ operation but for WRITE one
)
{
    int sID;

    if ((uriPtr == NULL) || (bufferPtr == NULL) || (lenPtr == NULL))
    {
        sID = LWM2MCORE_ERR_INVALID_ARG;
    }
    else
    {
        lwm2mcore_op_type_t supported_op_mask = LWM2MCORE_OP_READ | LWM2MCORE_OP_WRITE;

        /* Check that the server which tries to read/write is the bootstrap one
         * The Device Management server can not access to this resource
         */
        //TODO

        if (0 == (uriPtr->op & supported_op_mask))
        {
            sID = LWM2MCORE_ERR_OP_NOT_SUPPORTED;
        }
        else
        {
            /* Check that the object instance Id is in the correct range */
            if (uriPtr->oiid >= (LWM2MCORE_DM_SERVER_MAX_COUNT \
                               + LWM2MCORE_BOOTSRAP_SERVER_MAX_COUNT))
            {
                sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            }
            else
            {
                sID = LWM2MCORE_ERR_COMPLETED_OK;
            }
        }
    }
    return sID;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function for the short server ID
 * Object: 0 - Security
 * Resource: 10
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treament succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int OnLWM2MSecurityServerID
(
    lwm2mcore_uri_t *uriPtr,                ///< [IN] uriPtr represents the requested operation and
                                            ///< object/resource
    char *bufferPtr,                        ///< [INOUT] data buffer for information
    size_t *lenPtr,                         ///< [INOUT] length of input buffer and length of the
                                            ///< returned data
    value_changed_callback_t changed_cb     ///< [IN] not used for READ operation but for WRITE one
)
{
    int sID;

    if ((uriPtr == NULL) || (bufferPtr == NULL) || (lenPtr == NULL))
    {
        sID = LWM2MCORE_ERR_INVALID_ARG;
    }
    else
    {
        lwm2mcore_op_type_t supported_op_mask = LWM2MCORE_OP_READ | LWM2MCORE_OP_WRITE;

        /* Check that the server which tries to read/write is the bootstrap one
         * The Device Management server can not access to this resource
         */
        //TODO

        if (0 == (uriPtr->op & supported_op_mask))
        {
            sID = LWM2MCORE_ERR_OP_NOT_SUPPORTED;
        }
        else
        {
            /* Check that the object instance Id is in the correct range */
            if (uriPtr->oiid >= (LWM2MCORE_DM_SERVER_MAX_COUNT \
                               + LWM2MCORE_BOOTSRAP_SERVER_MAX_COUNT))
            {
                sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            }
            else
            {
                if (uriPtr->op & LWM2MCORE_OP_READ)
                {
                    /* Read operation */
                    if (LWM2MCORE_BS_SERVER_OIID == uriPtr->oiid)
                    {
                        /* Bootstrap server */
                        bufferPtr[0] = 0;
                        *lenPtr = 1;
                        sID = LWM2MCORE_ERR_COMPLETED_OK;
                    }
                    else
                    {
                        /* Device Management server */
                        bufferPtr[0] = 1;
                        *lenPtr = 1;
                        sID = LWM2MCORE_ERR_COMPLETED_OK;
                    }
                }
                else
                {
                    /* Write operation */
                    if (LWM2MCORE_BS_SERVER_OIID == uriPtr->oiid)
                    {
                        /* Bootstrap server */
                        sID = LWM2MCORE_ERR_COMPLETED_OK;
                    }
                    else
                    {
                        /* Device Management server */
                        sID = LWM2MCORE_ERR_COMPLETED_OK;
                    }
                }
            }
        }
    }
    return sID;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function for the Client Hold Off Time
 * Object: 0 - Security
 * Resource: 11
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treament succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int OnLWM2MSecurityClientHoldOffTime
(
    lwm2mcore_uri_t *uriPtr,                ///< [IN] uriPtr represents the requested operation and
                                            ///< object/resource
    char *bufferPtr,                        ///< [INOUT] data buffer for information
    size_t *lenPtr,                         ///< [INOUT] length of input buffer and length of the
                                            ///< returned data
    value_changed_callback_t changed_cb     ///< [IN] not used for READ operation but for WRITE one
)
{
    int sID;

    if ((uriPtr == NULL) || (bufferPtr == NULL) || (lenPtr == NULL))
    {
        sID = LWM2MCORE_ERR_INVALID_ARG;
    }
    else
    {
        lwm2mcore_op_type_t supported_op_mask = LWM2MCORE_OP_READ | LWM2MCORE_OP_WRITE;

        /* Check that the server which tries to read/write is the bootstrap one
         * The Device Management server can not access to this resource
         */
        //TODO

        if (0 == (uriPtr->op & supported_op_mask))
        {
            sID = LWM2MCORE_ERR_OP_NOT_SUPPORTED;
        }
        else
        {
            /* Check that the object instance Id is in the correct range */
            if (uriPtr->oiid >= (LWM2MCORE_DM_SERVER_MAX_COUNT \
                               + LWM2MCORE_BOOTSRAP_SERVER_MAX_COUNT))
            {
                sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            }
            else
            {
                if (uriPtr->op & LWM2MCORE_OP_READ)
                {
                    /* Read operation */
                    if (LWM2MCORE_BS_SERVER_OIID == uriPtr->oiid)
                    {
                        /* Bootstrap server */
                        bufferPtr[0] = 0;
                        *lenPtr = 1;
                        sID = LWM2MCORE_ERR_COMPLETED_OK;
                    }
                    else
                    {
                        /* Device Management server */
                        bufferPtr[0] = 0;
                        *lenPtr = 1;
                        sID = LWM2MCORE_ERR_COMPLETED_OK;
                    }
                }
                else
                {
                    /* Write operation */
                    if (LWM2MCORE_BS_SERVER_OIID == uriPtr->oiid)
                    {
                        /* Bootstrap server */
                        sID = LWM2MCORE_ERR_COMPLETED_OK;
                    }
                    else
                    {
                        /* Device Management server */
                        sID = LWM2MCORE_ERR_COMPLETED_OK;
                    }
                }
            }
        }
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
bool lwm2mcore_StoreCredentials
(
    void
)
{
    bool result = false;
    int storageResult = LWM2MCORE_ERR_COMPLETED_OK;

    LOG_ARG( "BsPskIdLen %d BsPskLen %d strlen (BsAddr) %d", BsPskIdLen, BsPskLen, strlen (BsAddr));
    LOG_ARG( "DmPskIdLen %d DmPskLen %d strlen (DmAddr) %d", DmPskIdLen, DmPskLen, strlen (DmAddr));
    if (BsPskIdLen && BsPskLen && strlen (BsAddr))
    {
        storageResult = os_portSecuritySetCredential ((uint8_t)LWM2MCORE_CREDENTIAL_BS_PUBLIC_KEY,
                                                        (char*)BsPskId,
                                                        BsPskIdLen);
        LOG_ARG( "Store BsPskId result %d", storageResult);

        storageResult = os_portSecuritySetCredential ((uint8_t)LWM2MCORE_CREDENTIAL_BS_SECRET_KEY,
                                                        (char*)BsPsk,
                                                        BsPskLen);
        LOG_ARG( "Store BsPsk result %d", storageResult);

        storageResult = os_portSecuritySetCredential ((uint8_t)LWM2MCORE_CREDENTIAL_BS_ADDRESS,
                                                        (char*)BsAddr,
                                                        strlen(BsAddr));
        LOG_ARG( "Store BsAddr result %d", storageResult);
    }

    if (BsPskIdLen && BsPskLen && strlen (BsAddr) && (LWM2MCORE_ERR_COMPLETED_OK == storageResult))
    {
        storageResult = os_portSecuritySetCredential ((uint8_t)LWM2MCORE_CREDENTIAL_DM_PUBLIC_KEY,
                                                        (char*)DmPskId,
                                                        DmPskIdLen);
        LOG_ARG( "Store DmPskId result %d", storageResult);

        storageResult = os_portSecuritySetCredential ((uint8_t)LWM2MCORE_CREDENTIAL_DM_SECRET_KEY,
                                                        (char*)DmPsk,
                                                        DmPskLen);
        LOG_ARG( "Store DmPsk result %d", storageResult);

        storageResult = os_portSecuritySetCredential ((uint8_t)LWM2MCORE_CREDENTIAL_DM_ADDRESS,
                                                        (char*)DmAddr,
                                                        strlen(DmAddr));
        LOG_ARG( "Store DmAddr result %d", storageResult);
    }

    if (LWM2MCORE_ERR_COMPLETED_OK == storageResult)
    {
        result = true;

        /* Reset local variables */
        BsPskIdLen = 0;
        BsPskLen = 0;
        DmPskIdLen = 0;
        DmPskLen = 0;

        memset (BsPskId, 0, DTLS_PSK_MAX_CLIENT_IDENTITY_LEN);
        memset (BsPsk, 0, DTLS_PSK_MAX_KEY_LEN);
        memset (BsAddr, 0, LWM2MCORE_SERVER_URI_MAX_LEN);
        memset (DmPskId, 0, DTLS_PSK_MAX_CLIENT_IDENTITY_LEN);
        memset (DmPsk, 0, DTLS_PSK_MAX_KEY_LEN);
        memset (DmAddr, 0, LWM2MCORE_SERVER_URI_MAX_LEN);
    }
    LOG_ARG ("credentials storage: %d", result);
    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 *                  OBJECT 1: SERVER
 */
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Function to read/write the short server ID
 * Object: 1 - Server
 * Resource: 0
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treament succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int OnLWM2MServerID
(
    lwm2mcore_uri_t *uriPtr,                ///< [IN] uriPtr represents the requested operation and
                                            ///< object/resource
    char *bufferPtr,                        ///< [INOUT] data buffer for information
    size_t *lenPtr,                         ///< [INOUT] length of input buffer and length of the
                                            ///< returned data
    value_changed_callback_t changed_cb     ///< [IN] not used for READ operation but for WRITE one
)
{
    int sID;

    if ((uriPtr == NULL) || (bufferPtr == NULL) || (lenPtr == NULL))
    {
        sID = LWM2MCORE_ERR_INVALID_ARG;
    }
    else
    {
        lwm2mcore_op_type_t supported_op_mask = LWM2MCORE_OP_READ | LWM2MCORE_OP_WRITE;

        if (0 == (uriPtr->op & supported_op_mask))
        {
            sID = LWM2MCORE_ERR_OP_NOT_SUPPORTED;
        }
        else
        {
            if (uriPtr->oiid >= (LWM2MCORE_DM_SERVER_MAX_COUNT))
            {
                sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            }
            else
            {
                if (uriPtr->op & LWM2MCORE_OP_READ)
                {
                    /* Read operation */
                    bufferPtr[0] = 1;
                    *lenPtr = 1;
                    sID = LWM2MCORE_ERR_COMPLETED_OK;
                }
                else
                {
                    /* Write operation */
                    sID = LWM2MCORE_ERR_COMPLETED_OK;
                }
            }
        }
    }
    return sID;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to read/write the server lifetime
 * Object: 1 - Server
 * Resource: 1
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treament succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int OnLWM2MServerLifeTime
(
    lwm2mcore_uri_t *uriPtr,                ///< [IN] uriPtr represents the requested operation and
                                            ///< object/resource
    char *bufferPtr,                        ///< [INOUT] data buffer for information
    size_t *lenPtr,                         ///< [INOUT] length of input buffer and length of the
                                            ///< returned data
    value_changed_callback_t changed_cb     ///< [IN] not used for READ operation but for WRITE one
)
{
    int sID;

    if ((uriPtr == NULL) || (bufferPtr == NULL) || (lenPtr == NULL))
    {
        sID = LWM2MCORE_ERR_INVALID_ARG;
    }
    else
    {
        lwm2mcore_op_type_t supported_op_mask = LWM2MCORE_OP_READ | LWM2MCORE_OP_WRITE;

        if (0 == (uriPtr->op & supported_op_mask))
        {
            sID = LWM2MCORE_ERR_OP_NOT_SUPPORTED;
        }
        else
        {
            if (uriPtr->oiid >= (LWM2MCORE_DM_SERVER_MAX_COUNT))
            {
                sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            }
            else
            {
                if (uriPtr->op & LWM2MCORE_OP_READ)
                {
                    /* Read operation */
                    *lenPtr = FormatValueToBytes ((uint8_t*) bufferPtr,
                                                      &Lifetime,
                                                      sizeof (Lifetime),
                                                      false);
                    LOG_ARG ("lifetime read len %d", *lenPtr);
                    sID = LWM2MCORE_ERR_COMPLETED_OK;
                }
                else
                {
                    /* Write operation */
                    Lifetime = (uint64_t)bytesToInt((uint8_t*)bufferPtr, *lenPtr);
                    sID = LWM2MCORE_ERR_COMPLETED_OK;
                }
            }
        }
    }
    return sID;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to read/write the server Pmin
 * Object: 1 - Server
 * Resource: 2
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treament succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int OnLWM2MServerMinPeriod
(
    lwm2mcore_uri_t *uriPtr,                ///< [IN] uriPtr represents the requested operation and
                                            ///< object/resource
    char *bufferPtr,                        ///< [INOUT] data buffer for information
    size_t *lenPtr,                         ///< [INOUT] length of input buffer and length of the
                                            ///< returned data
    value_changed_callback_t changed_cb     ///< [IN] not used for READ operation but for WRITE one
)
{
    int sID;

    if ((uriPtr == NULL) || (bufferPtr == NULL) || (lenPtr == NULL))
    {
        sID = LWM2MCORE_ERR_INVALID_ARG;
    }
    else
    {
        lwm2mcore_op_type_t supported_op_mask = LWM2MCORE_OP_READ | LWM2MCORE_OP_WRITE;

        if (0 == (uriPtr->op & supported_op_mask))
        {
            sID = LWM2MCORE_ERR_OP_NOT_SUPPORTED;
        }
        else
        {
            if (uriPtr->oiid >= (LWM2MCORE_DM_SERVER_MAX_COUNT))
            {
                sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            }
            else
            {
                if (uriPtr->op & LWM2MCORE_OP_READ)
                {
                    /* Read operation */
                    bufferPtr[0] = 30;
                    *lenPtr = 1;
                    sID = LWM2MCORE_ERR_COMPLETED_OK;
                }
                else
                {
                    /* Write operation */
                    sID = LWM2MCORE_ERR_COMPLETED_OK;
                }
            }
        }
    }
    return sID;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to read/write the server Pmax
 * Object: 1 - Server
 * Resource: 3
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treament succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int OnLWM2MServerMaxPeriod
(
    lwm2mcore_uri_t *uriPtr,                ///< [IN] uriPtr represents the requested operation and
                                            ///< object/resource
    char *bufferPtr,                        ///< [INOUT] data buffer for information
    size_t *lenPtr,                         ///< [INOUT] length of input buffer and length of the
                                            ///< returned data
    value_changed_callback_t changed_cb     ///< [IN] not used for READ operation but for WRITE one
)
{
    int sID;
    if ((uriPtr == NULL) || (bufferPtr == NULL) || (lenPtr == NULL))
    {
        sID = LWM2MCORE_ERR_INVALID_ARG;
    }
    else
    {
        lwm2mcore_op_type_t supported_op_mask = LWM2MCORE_OP_READ | LWM2MCORE_OP_WRITE;

        if (0 == (uriPtr->op & supported_op_mask))
        {
            sID = LWM2MCORE_ERR_OP_NOT_SUPPORTED;
        }
        else
        {
            if (uriPtr->oiid >= (LWM2MCORE_DM_SERVER_MAX_COUNT))
            {
                sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            }
            else
            {
                if (uriPtr->op & LWM2MCORE_OP_READ)
                {
                    /* Read operation */
                    bufferPtr[0] = 60;
                    *lenPtr = 1;
                    sID = LWM2MCORE_ERR_COMPLETED_OK;
                }
                else
                {
                    /* Write operation */
                    sID = LWM2MCORE_ERR_COMPLETED_OK;
                }
            }
        }
    }
    return sID;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to read/write the server Notification Storing When Disabled or Offline
 * Object: 1 - Server
 * Resource: 6
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treament succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int OnLWM2MServerQueueUpNotification
(
    lwm2mcore_uri_t *uriPtr,                ///< [IN] uriPtr represents the requested operation and
                                            ///< object/resource
    char *bufferPtr,                        ///< [INOUT] data buffer for information
    size_t *lenPtr,                         ///< [INOUT] length of input buffer and length of the
                                            ///< returned data
    value_changed_callback_t changed_cb     ///< [IN] not used for READ operation but for WRITE one
)
{
    int sID;

    if ((uriPtr == NULL) || (bufferPtr == NULL) || (lenPtr == NULL))
    {
        sID = LWM2MCORE_ERR_INVALID_ARG;
    }
    else
    {
        lwm2mcore_op_type_t supported_op_mask = LWM2MCORE_OP_READ | LWM2MCORE_OP_WRITE;

        if (0 == (uriPtr->op & supported_op_mask))
        {
            sID = LWM2MCORE_ERR_OP_NOT_SUPPORTED;
        }
        else
        {
            if (uriPtr->oiid >= (LWM2MCORE_DM_SERVER_MAX_COUNT))
            {
                sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            }
            else
            {
                if (uriPtr->op & LWM2MCORE_OP_READ)
                {
                    /* Read operation */
                    bufferPtr[0] = 0;
                    *lenPtr = 1;
                    sID = LWM2MCORE_ERR_COMPLETED_OK;
                }
                else
                {
                    /* Write operation */
                    sID = LWM2MCORE_ERR_COMPLETED_OK;
                }
            }
        }
    }
    return sID;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to read/write the server binding mode
 * Object: 1 - Server
 * Resource: 7
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treament succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int OnLWM2MServerBindingMode
(
    lwm2mcore_uri_t *uriPtr,                ///< [IN] uriPtr represents the requested operation and
                                            ///< object/resource
    char *bufferPtr,                        ///< [INOUT] data buffer for information
    size_t *lenPtr,                         ///< [INOUT] length of input buffer and length of the
                                            ///< returned data
    value_changed_callback_t changed_cb     ///< [IN] not used for READ operation but for WRITE one
)
{
    int sID;

    if ((uriPtr == NULL) || (bufferPtr == NULL) || (lenPtr == NULL))
    {
        sID = LWM2MCORE_ERR_INVALID_ARG;
    }
    else
    {
        lwm2mcore_op_type_t supported_op_mask = LWM2MCORE_OP_READ | LWM2MCORE_OP_WRITE;

        if (0 == (uriPtr->op & supported_op_mask))
        {
            sID = LWM2MCORE_ERR_OP_NOT_SUPPORTED;
        }
        else
        {
            if (uriPtr->oiid >= (LWM2MCORE_DM_SERVER_MAX_COUNT))
            {
                sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            }
            else
            {
                if (uriPtr->op & LWM2MCORE_OP_READ)
                {
                    /* Read operation */
                    *lenPtr = snprintf (bufferPtr, *lenPtr, "UQ");
                    sID = LWM2MCORE_ERR_COMPLETED_OK;
                }
                else
                {
                    /* Write operation */
                    sID = LWM2MCORE_ERR_COMPLETED_OK;
                }
            }
        }
    }
    return sID;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to send a REGISTRATION UPDATE to the LWM2M server (DM server)
 * Object: 1 - Server
 * Resource: 8
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treament succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int OnExecLWM2MServerRegUpdate
(
    lwm2mcore_uri_t *uriPtr,               ///< [IN] uriPtr represents the requested operation and
                                            ///< object/resource
    char *buffer,                       ///< [INOUT] contain arguments
    size_t *len                         ///< [INOUT] length of buffer
)
{
    return LWM2MCORE_ERR_NOT_YET_IMPLEMENTED;
}


//--------------------------------------------------------------------------------------------------
/**
 *                  OBJECT 3: DEVICE
 */
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Function to read the device manufacturer
 * Object: 3 - Device
 * Resource: 0
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treament succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int OnManufacturer
(
    lwm2mcore_uri_t *uriPtr,                ///< [IN] uriPtr represents the requested operation and
                                            ///< object/resource
    char *bufferPtr,                        ///< [INOUT] data buffer for information
    size_t *lenPtr,                         ///< [INOUT] length of input buffer and length of the
                                            ///< returned data
    value_changed_callback_t changed_cb     ///< [IN] not used for READ operation but for WRITE one
)
{
    int sID;

    if ((uriPtr == NULL) || (bufferPtr == NULL) || (lenPtr == NULL))
    {
        sID = LWM2MCORE_ERR_INVALID_ARG;
    }
    else
    {
        lwm2mcore_op_type_t supported_op_mask = LWM2MCORE_OP_READ;

        if (0 == (uriPtr->op & supported_op_mask))
        {
            sID = LWM2MCORE_ERR_OP_NOT_SUPPORTED;
        }
        else
        {
            if (uriPtr->oiid >= LWM2MCORE_DM_SERVER_MAX_COUNT)
            {
                sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            }
            else
            {
                sID = os_portDeviceManufacturer ((char*)bufferPtr, (uint32_t*) lenPtr);
                if (sID == LWM2MCORE_ERR_COMPLETED_OK)
                {
                    *lenPtr = ( size_t )strlen ((char*)bufferPtr);
                }
            }
        }
    }
    return sID;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to read the model number
 * Object: 3 - Device
 * Resource: 1
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treament succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int OnModelNumber
(
    lwm2mcore_uri_t *uriPtr,                ///< [IN] uriPtr represents the requested operation and
                                            ///< object/resource
    char *bufferPtr,                        ///< [INOUT] data buffer for information
    size_t *lenPtr,                         ///< [INOUT] length of input buffer and length of the
                                            ///< returned data
    value_changed_callback_t changed_cb     ///< [IN] not used for READ operation but for WRITE one
)
{
    int sID;

    if ((uriPtr == NULL) || (bufferPtr == NULL) || (lenPtr == NULL))
    {
        sID = LWM2MCORE_ERR_INVALID_ARG;
    }
    else
    {
        lwm2mcore_op_type_t supported_op_mask = LWM2MCORE_OP_READ;

        if (0 == (uriPtr->op & supported_op_mask))
        {
            sID = LWM2MCORE_ERR_OP_NOT_SUPPORTED;
        }
        else
        {
            if (uriPtr->oiid >= LWM2MCORE_DM_SERVER_MAX_COUNT)
            {
                sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            }
            else
            {
                sID = os_portDeviceModelNumber ((char*)bufferPtr, (uint32_t*) lenPtr);
                if (sID == LWM2MCORE_ERR_COMPLETED_OK)
                {
                    *lenPtr = ( size_t )strlen ((char*)bufferPtr);
                }
            }
        }
    }
    return sID;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to read the device serial number
 * Object: 3 - Device
 * Resource: 2
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treament succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int OnSerialNumber
(
    lwm2mcore_uri_t *uriPtr,                ///< [IN] uriPtr represents the requested operation and
                                            ///< object/resource
    char *bufferPtr,                        ///< [INOUT] data buffer for information
    size_t *lenPtr,                         ///< [INOUT] length of input buffer and length of the
                                            ///< returned data
    value_changed_callback_t changed_cb     ///< [IN] not used for READ operation but for WRITE one
)
{
    int sID;

    if ((uriPtr == NULL) || (bufferPtr == NULL) || (lenPtr == NULL))
    {
        sID = LWM2MCORE_ERR_INVALID_ARG;
    }
    else
    {
        lwm2mcore_op_type_t supported_op_mask = LWM2MCORE_OP_READ;

        if (0 == (uriPtr->op & supported_op_mask))
        {
            sID = LWM2MCORE_ERR_OP_NOT_SUPPORTED;
        }
        else
        {
            if (uriPtr->oiid >= LWM2MCORE_DM_SERVER_MAX_COUNT)
            {
                sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            }
            else
            {
                sID = os_portDeviceSerialNumber ((char*)bufferPtr, (uint32_t*) lenPtr);
                if (sID == LWM2MCORE_ERR_COMPLETED_OK)
                {
                    *lenPtr = ( size_t )strlen ((char*)bufferPtr);
                }
            }
        }
    }
    return sID;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to read the firmware version
 * Object: 3 - Device
 * Resource: 3
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treament succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int OnFirmwareVersion
(
    lwm2mcore_uri_t *uriPtr,                ///< [IN] uriPtr represents the requested operation and
                                            ///< object/resource
    char *bufferPtr,                        ///< [INOUT] data buffer for information
    size_t *lenPtr,                         ///< [INOUT] length of input buffer and length of the
                                            ///< returned data
    value_changed_callback_t changed_cb     ///< [IN] not used for READ operation but for WRITE one
)
{
    int sID;

    if ((uriPtr == NULL) || (bufferPtr == NULL) || (lenPtr == NULL))
    {
        sID = LWM2MCORE_ERR_INVALID_ARG;
    }
    else
    {
        lwm2mcore_op_type_t supported_op_mask = LWM2MCORE_OP_READ;

        if (0 == (uriPtr->op & supported_op_mask))
        {
            sID = LWM2MCORE_ERR_OP_NOT_SUPPORTED;
        }
        else
        {
            if (uriPtr->oiid >= LWM2MCORE_DM_SERVER_MAX_COUNT)
            {
                sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            }
            else
            {
                sID = os_portDeviceFirmwareVersion ((char*)bufferPtr, (uint32_t*) lenPtr);
                if (sID == LWM2MCORE_ERR_COMPLETED_OK)
                {
                    *lenPtr = ( size_t )strlen ((char*)bufferPtr);
                }
            }
        }
    }
    return sID;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to read/write the device time
 * Object: 3 - Device
 * Resource: 13
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treament succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int OnCurrentTime
(
    lwm2mcore_uri_t *uriPtr,                ///< [IN] uriPtr represents the requested operation and
                                            ///< object/resource
    char *bufferPtr,                        ///< [INOUT] data buffer for information
    size_t *lenPtr,                         ///< [INOUT] length of input buffer and length of the
                                            ///< returned data
    value_changed_callback_t changed_cb     ///< [IN] not used for READ operation but for WRITE one
)
{
    int sID;

    if ((uriPtr == NULL) || (bufferPtr == NULL) || (lenPtr == NULL))
    {
        sID = LWM2MCORE_ERR_INVALID_ARG;
    }
    else
    {
        lwm2mcore_op_type_t supported_op_mask = LWM2MCORE_OP_READ;

        if (0 == (uriPtr->op & supported_op_mask))
        {
            sID = LWM2MCORE_ERR_OP_NOT_SUPPORTED;
        }
        else
        {
            if (uriPtr->oiid >= LWM2MCORE_DM_SERVER_MAX_COUNT)
            {
                sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            }
            else
            {
                uint64_t time;
                sID = os_portDeviceCurrentTime (&time);
                if (sID == LWM2MCORE_ERR_COMPLETED_OK)
                {
                    *lenPtr = FormatValueToBytes ((uint8_t*) bufferPtr,
                                                      &time,
                                                      sizeof (time),
                                                      false);
                    sID = LWM2MCORE_ERR_COMPLETED_OK;
                }
            }
        }
    }
    return sID;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to read/write the device Supported Binding and Modes
 * Object: 3 - Device
 * Resource: 15
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treament succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int OnClientSupportedBindingMode
(
    lwm2mcore_uri_t *uriPtr,                ///< [IN] uriPtr represents the requested operation and
                                            ///< object/resource
    char *bufferPtr,                        ///< [INOUT] data buffer for information
    size_t *lenPtr,                         ///< [INOUT] length of input buffer and length of the
                                            ///< returned data
    value_changed_callback_t changed_cb     ///< [IN] not used for READ operation but for WRITE one
)
{
    int sID;
    if ((uriPtr == NULL) || (bufferPtr == NULL) || (lenPtr == NULL))
    {
        sID = LWM2MCORE_ERR_INVALID_ARG;
    }
    else
    {
        lwm2mcore_op_type_t supported_op_mask = LWM2MCORE_OP_READ;

        if (0 == (uriPtr->op & supported_op_mask))
        {
            sID = LWM2MCORE_ERR_OP_NOT_SUPPORTED;
        }
        else
        {
            if (uriPtr->oiid >= LWM2MCORE_DM_SERVER_MAX_COUNT)
            {
                sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            }
            else
            {
                *lenPtr = snprintf(bufferPtr, *lenPtr, "UQ");
                sID = LWM2MCORE_ERR_COMPLETED_OK;
            }
        }
    }
    return sID;
}


//--------------------------------------------------------------------------------------------------
/**
 * Function to read/write the SSL certificates
 * Object: 10243 - SSL certificates
 * Resource: 0
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treament succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int OnSslCertif
(
    lwm2mcore_uri_t *uriPtr,                ///< [IN] uriPtr represents the requested operation and
                                            ///< object/resource
    char *bufferPtr,                        ///< [INOUT] data buffer for information
    size_t *lenPtr,                         ///< [INOUT] length of input buffer and length of the
                                            ///< returned data
    value_changed_callback_t changed_cb     ///< [IN] not used for READ operation but for WRITE one
)
{
    int sID;
    if ((uriPtr == NULL) || (bufferPtr == NULL) || (lenPtr == NULL))
    {
        sID = LWM2MCORE_ERR_INVALID_ARG;
    }
    else
    {
        lwm2mcore_op_type_t supported_op_mask = LWM2MCORE_OP_READ | LWM2MCORE_OP_WRITE;

        if (0 == (uriPtr->op & supported_op_mask))
        {
            sID = LWM2MCORE_ERR_OP_NOT_SUPPORTED;
        }
        else
        {
            if (uriPtr->oiid > 0)
            {
                sID = LWM2MCORE_ERR_INCORRECT_RANGE;
            }
            else
            {
                /* This resource needs the BLOCK1 option support */
                if (uriPtr->op & LWM2MCORE_OP_READ)
                {
                    /* Read operation */
                    if (*lenPtr == 0)
                    {
                        /* Delete the certificates */
                        sID = LWM2MCORE_ERR_NOT_YET_IMPLEMENTED;
                    }
                    else
                    {
                        /* Read the stored certificates */
                        sID = LWM2MCORE_ERR_NOT_YET_IMPLEMENTED;
                    }
                }
                else
                {
                    /* Write a certificate */
                    sID = LWM2MCORE_ERR_NOT_YET_IMPLEMENTED;
                }
            }
        }
    }
    return sID;
}



//--------------------------------------------------------------------------------------------------
/**
 * Function for not registered objects
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treament succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *      - positive value for asynchronous response
 */
//--------------------------------------------------------------------------------------------------
int OnUnlistedObject
(
    lwm2mcore_uri_t *uriPtr,                ///< [IN] uriPtr represents the requested operation and
                                            ///< object/resource
    char *bufferPtr,                        ///< [INOUT] data buffer for information
    size_t *lenPtr,                         ///< [INOUT] length of input buffer and length of the
                                            ///< returned data
    value_changed_callback_t changed_cb     ///< [IN] callback function pointer for OBSERVE
                                            ///< operation
)
{
    return LWM2MCORE_ERR_NOT_YET_IMPLEMENTED;
}

