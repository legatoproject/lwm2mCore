/**
 * @file paramStorage.c
 *
 * Porting layer for parameter storage in platform memory
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#include <stdint.h>
#include <stddef.h>
#include <platform/types.h>
#include <lwm2mcore/lwm2mcore.h>
#include <lwm2mcore/paramStorage.h>

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
)
{
    if ((LWM2MCORE_MAX_PARAM <= paramId) || (NULL == bufferPtr))
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }
    return LWM2MCORE_ERR_NOT_YET_IMPLEMENTED;
}

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
)
{
    if ((LWM2MCORE_MAX_PARAM <= paramId) || (NULL == bufferPtr) || (NULL == lenPtr))
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }
    return LWM2MCORE_ERR_NOT_YET_IMPLEMENTED;
}

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
)
{
    if (LWM2MCORE_MAX_PARAM <= paramId)
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }
    return LWM2MCORE_ERR_NOT_YET_IMPLEMENTED;
}

