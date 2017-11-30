/**
 * @file paramStorage.c
 *
 * Porting layer for parameter storage in platform memory
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <platform/types.h>
#include <lwm2mcore/lwm2mcore.h>
#include <lwm2mcore/paramStorage.h>

//--------------------------------------------------------------------------------------------------
/**
 * Configuration filename
 */
//--------------------------------------------------------------------------------------------------
#define CONFIG_FILENAME             "config"

//--------------------------------------------------------------------------------------------------
/**
 * Configuration filename maximum length
 */
//--------------------------------------------------------------------------------------------------
#define CONFIG_FILENAME_MAX_LENGTH  100

//--------------------------------------------------------------------------------------------------
/**
 * Configuration file maximum length
 */
//--------------------------------------------------------------------------------------------------
#define CONFIG_FILE_MAX_LENGTH      200

//--------------------------------------------------------------------------------------------------
/**
 * Write parameter in platform memory
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_SetParam
(
    lwm2mcore_Param_t paramId,      ///< [IN] Parameter Id
    uint8_t* bufferPtr,             ///< [IN] Data buffer
    size_t len                      ///< [IN] Length of input buffer
)
{
    FILE* fPtr = NULL;
    char fname0[CONFIG_FILENAME_MAX_LENGTH];
    char fname1[CONFIG_FILENAME_MAX_LENGTH];

    if ((LWM2MCORE_MAX_PARAM <= paramId) || (NULL == bufferPtr))
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    sprintf(fname0, "%s%d.txt", CONFIG_FILENAME, paramId);
    sprintf(fname1, "%s%d.bak", CONFIG_FILENAME, paramId);

    if (NULL != (fPtr = fopen(fname0, "w")))
    {
        if (fwrite(bufferPtr, 1, len, fPtr) != len)
        {
            fclose(fPtr);
            printf("%s Failed to write file: %s\n", __func__, fname0);
            return LWM2MCORE_ERR_GENERAL_ERROR;
        }
        fclose(fPtr);
    }

    if (NULL != (fPtr = fopen(fname1, "w")))
    {
        if (fwrite(bufferPtr, 1, len, fPtr) != len)
        {
            fclose(fPtr);
            printf("%s Failed to write file: %s\n", __func__, fname1);
            return LWM2MCORE_ERR_GENERAL_ERROR;
        }
        fclose(fPtr);
    }

    return LWM2MCORE_ERR_COMPLETED_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Read parameter from platform memory
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetParam
(
    lwm2mcore_Param_t paramId,      ///< [IN] Parameter Id
    uint8_t* bufferPtr,             ///< [INOUT] Data buffer
    size_t* lenPtr                  ///< [INOUT] Length of input buffer
)
{
    FILE* fPtr = NULL;
    int bsize = *lenPtr;
    int rsize = 0;
    char fname0[CONFIG_FILENAME_MAX_LENGTH];
    char fname1[CONFIG_FILENAME_MAX_LENGTH];

    if ((LWM2MCORE_MAX_PARAM <= paramId) || (NULL == bufferPtr) || (NULL == lenPtr))
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    sprintf(fname0, "%s%d.txt", CONFIG_FILENAME, paramId);
    sprintf(fname1, "%s%d.bak", CONFIG_FILENAME, paramId);

    if (NULL != (fPtr = fopen(fname0, "r")))
    {
        if ((rsize = fread(bufferPtr, 1, bsize, fPtr)) == 0)
        {
            printf("Failed to read file %s\n", fname0);
            fclose(fPtr);
            return LWM2MCORE_ERR_GENERAL_ERROR;
        }
        fclose(fPtr);
    }

    if (rsize == 0)
    {
        if (NULL != (fPtr = fopen(fname1, "r")))
        {
            if (0 == (rsize = fread(bufferPtr, 1, bsize, fPtr)))
            {
                printf("Failed to read file %s\n", fname1);
                bsize = -1; /*Incidate reading error*/
                fclose(fPtr);
                return LWM2MCORE_ERR_GENERAL_ERROR;
            }
            fclose(fPtr);
        }
    }
    *lenPtr = (size_t)rsize;

    if(!rsize)
    {
        return LWM2MCORE_ERR_GENERAL_ERROR;
    }

    return LWM2MCORE_ERR_COMPLETED_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Delete parameter from platform memory
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *      - LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_DeleteParam
(
    lwm2mcore_Param_t paramId       ///< [IN] Parameter Id
)
{
    char fname0[CONFIG_FILENAME_MAX_LENGTH];
    char fname1[CONFIG_FILENAME_MAX_LENGTH];

    if (LWM2MCORE_MAX_PARAM <= paramId)
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    sprintf(fname0, "%s%d.txt", CONFIG_FILENAME, paramId);
    sprintf(fname1, "%s%d.bak", CONFIG_FILENAME, paramId);

    if ( (remove(fname0) != 0)
      || (remove(fname1) != 0) )
    {
        return LWM2MCORE_ERR_GENERAL_ERROR;
    }

    return LWM2MCORE_ERR_COMPLETED_OK;
}

