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

//--------------------------------------------------------------------------------------------------
/**
 * Parameter identities enumeration
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    LWM2MCORE_BOOTSTRAP_PARAM,      ///< Bootstrap configuration parameters
    LWM2MCORE_DWL_WORKSPACE_PARAM,  ///< Download workspace parameters
    LWM2MCORE_MAX_PARAM             ///< Maximum parameter value (internal use)
}lwm2mcore_Param_t;

//--------------------------------------------------------------------------------------------------
/**
 * Write parameter in platform memory
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
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
 * Read parameter from platform memory
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
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
 * Delete parameter from platform memory
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the provided parameters (WRITE operation) is incorrect
 *      - LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *      - LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *      - LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_DeleteParam
(
    lwm2mcore_Param_t paramId       ///< [IN] Parameter Id
);

#endif /* __PARAMSTORAGE_H__ */

