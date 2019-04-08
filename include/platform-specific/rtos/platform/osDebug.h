/**
 * @file debug.h
 *
 * Header file for adaptation layer for log management
 *
 * <hr>
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#ifndef __LWM2MCORE_DEBUG_H__
#define __LWM2MCORE_DEBUG_H__

#include <stdint.h>
#include "platform/types.h"
#if (__SWI_PLAT__ & (SWI_HL85xx))
#include "sal_common.h"
#include "sal_debug.h"
#endif

//--------------------------------------------------------------------------------------------------
/**
 * Constants
 */
//--------------------------------------------------------------------------------------------------
#define LWM2MC_TRACE_BUFFER_LEN 256


//--------------------------------------------------------------------------------------------------
/**
 *
 */
//--------------------------------------------------------------------------------------------------

void lwm2m_printf(const char * format, ...);

#define _LWM2MTR(...) lwm2m_printf(__VA_ARGS__)


void lwm2mcore_DataDump
(
    char *descPtr,                  ///< [IN] data description
    void *addrPtr,                  ///< [IN] Data address to be dumped
    int len                         ///< [IN] Data length
);
#endif //__LWM2MCORE_DEBUG_H__