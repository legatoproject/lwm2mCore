/**
 * @file cellular.h
 *
 * Porting layer for cellular network connectivity parameters
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#ifndef __LWM2MCORE_CELLULAR_H__
#define __LWM2MCORE_CELLULAR_H__

//--------------------------------------------------------------------------------------------------
/**
 * @brief Define for eDRX max value
 */
//--------------------------------------------------------------------------------------------------
#define EDRX_MAX_VALUE  15

//--------------------------------------------------------------------------------------------------
/**
 * @brief Enumeration for the eDRX RAT
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    LWM2MCORE_CELL_EDRX_IU_MODE = 1,    ///< eDRX for Iu mode
    LWM2MCORE_CELL_EDRX_WB_S1_MODE,     ///< eDRX for WB-S1 mode
    LWM2MCORE_CELL_EDRX_NB_S1_MODE,     ///< eDRX for NB-S1 mode
    LWM2MCORE_CELL_EDRX_A_GB_MODE       ///< eDRX for A/Gb mode
}
lwm2mcore_CelleDrxRat_t;

//--------------------------------------------------------------------------------------------------
/**
 * @brief Retrieve the eDRX parameters
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GeteDrxParameters
(
    lwm2mcore_CelleDrxRat_t rat,        ///< [IN] RAT
    uint8_t*                valuePtr    ///< [INOUT] value
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Set the eDRX parameters
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_SeteDrxParameters
(
    lwm2mcore_CelleDrxRat_t rat,        ///< [IN] RAT
    uint8_t                 value       ///< [INOUT] value
);

#endif /* __LWM2MCORE_CELLULAR_H__ */
