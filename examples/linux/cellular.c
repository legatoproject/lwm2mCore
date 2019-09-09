/**
 * @file cellular.c
 *
 * Porting layer for cellular network connectivity parameters
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <platform/types.h>
#include <lwm2mcore/lwm2mcore.h>
#include <lwm2mcore/cellular.h>

static uint8_t EDrxValueIu = 0;
static uint8_t EDrxValueWbS1 = 0;
static uint8_t EDrxValueNbS1 = 0;
static uint8_t EDrxValueAGb = 0;

//--------------------------------------------------------------------------------------------------
/**
 * @brief Static value for Paging Time Window
 */
//--------------------------------------------------------------------------------------------------
#define PAGING_TIME_WINDOW  0x60

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
)
{
    if (!valuePtr)
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    switch (rat)
    {
        case LWM2MCORE_CELL_EDRX_IU_MODE:
            *valuePtr = EDrxValueIu;
            break;

        case LWM2MCORE_CELL_EDRX_WB_S1_MODE:
            *valuePtr = EDrxValueWbS1;
            break;

        case LWM2MCORE_CELL_EDRX_NB_S1_MODE:
            *valuePtr = EDrxValueNbS1;
            break;

        case LWM2MCORE_CELL_EDRX_A_GB_MODE:
            *valuePtr = EDrxValueAGb;
            break;

        default:
            return LWM2MCORE_ERR_INVALID_ARG;
    }
    // Add a static value for Paging Time Window
    *valuePtr |= PAGING_TIME_WINDOW;
    return LWM2MCORE_ERR_COMPLETED_OK;
}

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
)
{
    printf("lwm2mcore_SeteDrxParameters rat %d, value 0x%x\n", rat, value);
    switch (rat)
    {
        case LWM2MCORE_CELL_EDRX_IU_MODE:
            EDrxValueIu = value;
            break;

        case LWM2MCORE_CELL_EDRX_WB_S1_MODE:
            EDrxValueWbS1 = value;
            break;

        case LWM2MCORE_CELL_EDRX_NB_S1_MODE:
            EDrxValueNbS1 = value;
            break;

        case LWM2MCORE_CELL_EDRX_A_GB_MODE:
            EDrxValueAGb = value;
            break;

        default:
            return LWM2MCORE_ERR_INVALID_ARG;
    }
    return LWM2MCORE_ERR_COMPLETED_OK;
}
