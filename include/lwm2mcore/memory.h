/**
 * @file memory.h
 *
 * Porting layer for memory
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#ifndef __LWM2MCORE_MEMORY_H__
#define __LWM2MCORE_MEMORY_H__

/**
  * @addtogroup lwm2mcore_platform_adaptor_memory_IFS
  * @{
  */

//--------------------------------------------------------------------------------------------------
/**
 * Memory reallocation
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - memory address
 */
//--------------------------------------------------------------------------------------------------
void* lwm2mcore_realloc
(
    void*  ptr,         ///< [IN] Data address
    size_t newSize      ///< [IN] New size allocation
);

/**
  * @}
  */

#endif /* __LWM2MCORE_MEMORY_H__ */
