/**
 * @file location.h
 *
 * Porting layer for location parameters
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#ifndef __LWM2MCORE_LOCATION_H__
#define __LWM2MCORE_LOCATION_H__

#include <lwm2mcore/lwm2mcore.h>

/**
  * @addtogroup lwm2mcore_location_monitoring_IFS
  * @{
  */

//--------------------------------------------------------------------------------------------------
/**
 * @brief Retrieve the WSG84 latitude
 * This API treatment needs to have a procedural treatment
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *  - @ref LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *  - @ref LWM2MCORE_ERR_OVERFLOW in case of buffer overflow
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetLatitude
(
    char*   bufferPtr,  ///< [IN]    data buffer pointer
    size_t* lenPtr      ///< [INOUT] length of input buffer and length of the returned data
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Retrieve the WSG84 longitude
 * This API treatment needs to have a procedural treatment
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *  - @ref LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *  - @ref LWM2MCORE_ERR_OVERFLOW in case of buffer overflow
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetLongitude
(
    char*   bufferPtr,  ///< [IN]    data buffer pointer
    size_t* lenPtr      ///< [INOUT] length of input buffer and length of the returned data
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Retrieve the altitude (meters above sea level)
 * This API treatment needs to have a procedural treatment
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *  - @ref LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *  - @ref LWM2MCORE_ERR_OVERFLOW in case of buffer overflow
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetAltitude
(
    char*   bufferPtr,  ///< [IN]    data buffer pointer
    size_t* lenPtr      ///< [INOUT] length of input buffer and length of the returned data
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Retrieve the direction of movement (range 0-359 degrees, where 0 is True North)
 * This API treatment needs to have a procedural treatment
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *  - @ref LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *  - @ref LWM2MCORE_ERR_OVERFLOW in case of buffer overflow
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetDirection
(
    uint32_t* valuePtr  ///< [INOUT] data buffer
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Retrieve the horizontal speed in m/s
 * This API treatment needs to have a procedural treatment
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *  - @ref LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *  - @ref LWM2MCORE_ERR_OVERFLOW in case of buffer overflow
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetHorizontalSpeed
(
    uint32_t* valuePtr  ///< [INOUT] data buffer
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Retrieve the vertical speed in m/s, positive up
 * This API treatment needs to have a procedural treatment
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *  - @ref LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *  - @ref LWM2MCORE_ERR_OVERFLOW in case of buffer overflow
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetVerticalSpeed
(
    int32_t* valuePtr   ///< [INOUT] data buffer
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Retrieve the timestamp of when the location measurement was performed
 * This API treatment needs to have a procedural treatment
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *  - @ref LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetLocationTimestamp
(
    uint64_t* valuePtr  ///< [INOUT] data buffer
);

/**
  * @}
  */

#endif /* __LWM2MCORE_LOCATION_H__ */
