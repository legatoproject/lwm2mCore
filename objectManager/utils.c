/**
 * @file utils.c
 *
 * Tools for LwM2MCore
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

/* include files */
#include <stdint.h>
#include <platform/types.h>
#include <stddef.h>
#include "utils.h"

//--------------------------------------------------------------------------------------------------
/**
 * Convert to unsigned 16 bits integer to network bytes stream
 *
 * @return
 *  - converted size
 */
//--------------------------------------------------------------------------------------------------
size_t omanager_FormatUint16ToBytes
(
    uint8_t* bytesPtr,      ///< [INOUT] the buffer contains data converted
    const uint16_t u        ///< [IN] the value to be converted
)
{
    bytesPtr[0] = (u >> 8) & 0xff;
    bytesPtr[1] = u & 0xff;
    return (sizeof(uint16_t));
}

//--------------------------------------------------------------------------------------------------
/**
 * Convert to unsigned 32 bits integer to network bytes stream
 *
 * @return
 *  - converted size
 */
//--------------------------------------------------------------------------------------------------
size_t omanager_FormatUint32ToBytes
(
    uint8_t* bytesPtr,      ///< [INOUT] the buffer contains data converted
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
 * Convert to unsigned 64 bits integer to network bytes stream
 *
 * @return
 *  - converted size
 */
//--------------------------------------------------------------------------------------------------
size_t omanager_FormatUint64ToBytes
(
    uint8_t* bytesPtr,      ///< [INOUT] the buffer contains data converted
    const uint64_t u        ///< [IN] the value to be converted
)
{
    omanager_FormatUint32ToBytes(bytesPtr, (u >> 32) & 0xffffffff);
    omanager_FormatUint32ToBytes(bytesPtr + 4, u & 0xffffffff);
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
size_t omanager_FormatValueToBytes
(
    uint8_t* bytesPtr,      ///< [INOUT] bytes buffer in which uPtr value will be written
    void* uPtr,             ///< [INOUT] Data to be written
    uint32_t size,          ///< [IN] length to be written
    bool bSignedValue       ///< [IN] Indicates if uPtr shall be considered like a signed value
)
{
    size_t lReturn = 0;
    size_t updatedSize = size;

    uint8_t u8Value;
    uint16_t u16Value;
    uint32_t u32Value;
    uint64_t u64Value;

    uint8_t* u8ValuePtr = 0;
    uint16_t* u16ValuePtr = 0;
    uint32_t* u32ValuePtr = 0;
    uint64_t* u64ValuePtr = 0;

    if ((!bytesPtr) || (!uPtr))
    {
        return 0;
    }

    if (bSignedValue == false)
    {
        if (size == sizeof(uint8_t))
        {
            u8ValuePtr = (uint8_t*)uPtr;
            if (*u8ValuePtr > 0x7F)
            {
                /* Value shall be coded in 2 bytes */
                u16Value = (uint16_t)*u8ValuePtr;
                u16ValuePtr = &u16Value;
                updatedSize = sizeof (uint16_t);
            }
        }
        else if (size == sizeof(uint16_t))
        {
            u16ValuePtr = (uint16_t*)uPtr;
            if (*u16ValuePtr > 0x7FFF)
            {
                /* Value shall be coded in 4 bytes */
                u32Value = (uint32_t)*u16ValuePtr;
                u32ValuePtr = &u32Value;
                updatedSize = sizeof (uint32_t);
            }
            else if (*u16ValuePtr <= 0x7F)
            {
                /* the value could be coded in 1 byte */
                u8Value = (uint8_t)*u16ValuePtr;
                u8ValuePtr = &u8Value;
                updatedSize = sizeof (uint8_t);
            }
        }
        else if (size == sizeof (uint32_t))
        {
            u32ValuePtr = (uint32_t*)uPtr;
            if (*u32ValuePtr > 0x7FFFFFFF)
            {
                /* Value shall be coded in 8 bytes */
                u64Value = (uint64_t)*u32ValuePtr;
                u64ValuePtr = &u64Value;
                updatedSize = sizeof (uint64_t);
            }
            else if (*u32ValuePtr <= 0x7F)
            {
                /* the value could be coded in 1 byte */
                u8Value = (uint8_t)*u32ValuePtr;
                u8ValuePtr = &u8Value;
                updatedSize = sizeof (uint8_t);
            }
            else if (*u32ValuePtr <= 0x7FFF)
            {
                /* the value could be coded in 2 bytes */
                u16Value = (uint16_t)*u32ValuePtr;
                u16ValuePtr = &u16Value;
                updatedSize = sizeof (uint16_t);
            }
        }
        else if (size == sizeof (uint64_t))
        {
            u64ValuePtr = (uint64_t*)uPtr;
            if (*u64ValuePtr >> 63)
            {
                updatedSize = 0;
            }
            else if (*u64ValuePtr <= 0x7F)
            {
                /* the value could be coded in 1 byte */
                u8Value = (uint8_t)*u64ValuePtr;
                u8ValuePtr = &u8Value;
                updatedSize = sizeof (uint8_t);
            }
            else if (*u64ValuePtr <= 0x7FFF)
            {
                /* the value could be coded in 2 bytes */
                u16Value = (uint16_t)*u64ValuePtr;
                u16ValuePtr = &u16Value;
                updatedSize = sizeof (uint16_t);
            }
            else if (*u64ValuePtr <= 0x7FFFFFFF)
            {
                /* the value could be coded in 4 bytes */
                u32Value = (uint32_t)*u64ValuePtr;
                u32ValuePtr = &u32Value;
                updatedSize = sizeof (uint32_t);
            }
        }
        else
        {
            updatedSize = 0;
        }
    }
    else
    {
        if (size == sizeof(uint8_t))
        {
            u8ValuePtr = (uint8_t*)uPtr;
        }
        else if (size == sizeof(uint16_t))
        {
            u16ValuePtr = (uint16_t*)uPtr;
        }
        else if (size == sizeof (uint32_t))
        {
            u32ValuePtr = (uint32_t*)uPtr;
        }
        else if (size == sizeof (uint64_t))
        {
            u64ValuePtr = (uint64_t*)uPtr;
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
            lReturn = sizeof (uint8_t);
            bytesPtr[ 0 ] = *u8ValuePtr;
        }
        break;

        case 2:
        {
            lReturn = omanager_FormatUint16ToBytes(bytesPtr, *u16ValuePtr);
        }
        break;

        case 4:
        {
            lReturn = omanager_FormatUint32ToBytes(bytesPtr, *u32ValuePtr);
        }
        break;

        case 8:
        {
            lReturn = omanager_FormatUint64ToBytes(bytesPtr, *u64ValuePtr);
        }
        break;

        default:
        {
            lReturn = -1;
        }
        break;
    }

    return lReturn;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to convert bytes(in network byte order) to unsigned 16 bits integer
 *
 * @return
 *      - converted data
 */
//--------------------------------------------------------------------------------------------------
uint16_t omanager_BytesToUint16
(
    const uint8_t* bytesPtr     ///< [IN] bytes the buffer contains data to be converted
)
{
    return ((bytesPtr[0] << 8) | bytesPtr[1]);
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to convert bytes(in network byte order) to unsigned 32 bits integer
 *
 * @return
 *      - converted data
 */
//--------------------------------------------------------------------------------------------------
uint32_t omanager_BytesToUint32
(
    const uint8_t* bytesPtr     ///< [IN] bytes the buffer contains data to be converted
)
{
    return ((bytesPtr[0] << 24) | (bytesPtr[1] << 16) | (bytesPtr[2] << 8) | bytesPtr[3]);
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to convert bytes(in network byte order) to unsigned 64 bits integer
 *
 * @return
 *      - converted data
 */
//--------------------------------------------------------------------------------------------------
uint64_t omanager_BytesToUint64
(
    const uint8_t* bytesPtr     ///< [IN] bytes the buffer contains data to be converted
)
{
    return (((uint64_t)omanager_BytesToUint32(bytesPtr) << 32)
            | omanager_BytesToUint32(bytesPtr + 4));
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to convert bytes(in network byte order) to integer
 *
 * @return
 *      - converted data
 */
//--------------------------------------------------------------------------------------------------
int64_t omanager_BytesToInt
(
    const char* bytesPtr,    ///< [IN] bytes the buffer contains data to be converted
    size_t len                  ///< [IN] Bytes length
)
{
    int64_t value;

    switch(len)
    {
        case 1:
        {
            value = *bytesPtr;
        }
        break;

        case 2:
        {
            value = (int16_t)omanager_BytesToUint16((const uint8_t*)bytesPtr);
        }
        break;

        case 4:
        {
            value = (int32_t)omanager_BytesToUint32((const uint8_t*)bytesPtr);
        }
        break;

        case 8:
        {
            value = (int64_t)omanager_BytesToUint64((const uint8_t*)bytesPtr);
        }
        break;

        default:
        {
            value = -1;
        }
        break;
    }

    return value;
}


