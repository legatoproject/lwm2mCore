/**
 * @file utils.h
 *
 * Tools header file
 *
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#ifndef __UTILS_H__
#define __UTILS_H__

/**
  * @addtogroup lwm2mcore_utils_int
  * @{
  */

//--------------------------------------------------------------------------------------------------
/**
 * @brief Convert to unsigned 16 bits integer to network bytes stream
 *
 * @return
 *  - converted size
 */
//--------------------------------------------------------------------------------------------------
size_t omanager_FormatUint16ToBytes
(
    uint8_t* bytesPtr,      ///< [INOUT] the buffer contains data converted
    const uint16_t u        ///< [IN] the value to be converted
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Convert to unsigned 32 bits integer to network bytes stream
 *
 * @return
 *  - converted size
 */
//--------------------------------------------------------------------------------------------------
size_t omanager_FormatUint32ToBytes
(
    uint8_t* bytesPtr,      ///< [INOUT] the buffer contains data converted
    const uint32_t u        ///< [IN] the value to be converted
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Convert to unsigned 64 bits integer to network bytes stream
 *
 * @return
 *  - converted size
 */
//--------------------------------------------------------------------------------------------------
size_t omanager_FormatUint64ToBytes
(
    uint8_t* bytesPtr,      ///< [INOUT] the buffer contains data converted
    const uint64_t u        ///< [IN] the value to be converted
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function used by object resource API to write value in buffer
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
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to convert bytes(in network byte order) to unsigned 16 bits integer
 *
 * @return
 *      - converted data
 */
//--------------------------------------------------------------------------------------------------
uint16_t omanager_BytesToUint16
(
    const uint8_t* bytesPtr     ///< [IN] bytes the buffer contains data to be converted
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to convert bytes(in network byte order) to unsigned 32 bits integer
 *
 * @return
 *      - converted data
 */
//--------------------------------------------------------------------------------------------------
uint32_t omanager_BytesToUint32
(
    const uint8_t* bytesPtr     ///< [IN] bytes the buffer contains data to be converted
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to convert bytes(in network byte order) to unsigned 64 bits integer
 *
 * @return
 *      - converted data
 */
//--------------------------------------------------------------------------------------------------
uint64_t omanager_BytesToUint64
(
    const uint8_t* bytesPtr     ///< [IN] bytes the buffer contains data to be converted
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to convert bytes(in network byte order) to integer
 *
 * @return
 *      - converted data
 */
//--------------------------------------------------------------------------------------------------
int64_t omanager_BytesToInt
(
    const char* bytesPtr,    ///< [IN] bytes the buffer contains data to be converted
    size_t len                  ///< [IN] Bytes length
);
/**
  * @}
  */

#endif /* __UTILS_H__ */
