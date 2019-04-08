/**
 * @file paramStorage.h
 *
 * Porting layer for platform parameter storage
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#ifndef __PARAMSTORAGE_H__
#define __PARAMSTORAGE_H__

/**
  * @addtogroup lwm2mcore_platform_adaptor_storage_IFS
  * @{
  */

//--------------------------------------------------------------------------------------------------
/**
 * Parameter identities enumeration
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    LWM2MCORE_BOOTSTRAP_PARAM,              ///< Bootstrap configuration parameters
    LWM2MCORE_DWL_WORKSPACE_PARAM,          ///< Download workspace parameters
    LWM2MCORE_BOOTSTRAP_INFO_SIZE_PARAM,    ///< Bootstrap configuration file size
    LWM2MCORE_ACCESS_RIGHTS_PARAM,          ///< ACL data
    LWM2MCORE_ACCESS_RIGHTS_SIZE_PARAM,     ///< ACL data size
    LWM2MCORE_MAX_PARAM                     ///< Maximum parameter value (internal use)
}lwm2mcore_Param_t;

//--------------------------------------------------------------------------------------------------
/**
 * @brief Write parameter in platform non-volatile memory
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters is incorrect
 *  - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *  - @ref LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_SetParam
(
    lwm2mcore_Param_t paramId,      ///< [IN] Parameter Id
    uint8_t* bufferPtr,             ///< [IN] data buffer
    size_t len                      ///< [IN] length of input buffer
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Read parameter from platform non-volatile memory
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters is incorrect
 *  - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *  - @ref LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 *  - @ref LWM2MCORE_ERR_OVERFLOW in case of buffer overflow
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetParam
(
    lwm2mcore_Param_t paramId,      ///< [IN] Parameter Id
    uint8_t* bufferPtr,             ///< [INOUT] data buffer
    size_t* lenPtr                  ///< [INOUT] length of input buffer
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Delete parameter from platform non-volatile memory
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters is incorrect
 *  - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *  - @ref LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_DeleteParam
(
    lwm2mcore_Param_t paramId       ///< [IN] Parameter Id
);

/**
  * @}
  */

#endif /* __PARAMSTORAGE_H__ */

