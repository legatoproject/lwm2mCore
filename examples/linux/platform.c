/**
 * @file platform.c
 *
 * Adaptation layer for platform memory allocation and string related functions
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <liblwm2m.h>

#ifndef LWM2M_MEMORY_TRACE
//--------------------------------------------------------------------------------------------------
/**
 * Memory allocation with trace
 *
 * @return
 *  - memory address
 */
//--------------------------------------------------------------------------------------------------
void* lwm2m_malloc
(
    size_t size     ///< [IN] Memory size to be allocated
)
{
    void *mem;

    mem = malloc(size);
    if (!mem)
    {
#ifdef LWM2M_WITH_LOGS
        lwm2m_printf("out of memory\n");
#endif
        return NULL;
    }
    return mem;
}

//--------------------------------------------------------------------------------------------------
/**
 * Memory free
 */
//--------------------------------------------------------------------------------------------------
void lwm2m_free
(
    void* ptr   ///< [IN] Memory address to release
)
{
    free(ptr);
}

//--------------------------------------------------------------------------------------------------
/**
 * Duplicate a string
 *
 * @return
 *  - Duplicated string address
 */
//--------------------------------------------------------------------------------------------------
char* lwm2m_strdup
(
    const char* strPtr  ///< [IN] String to be duplicated
)
{
    char* dstrPtr;

    dstrPtr = strdup(strPtr);
    if (!dstrPtr)
    {
#ifdef LWM2M_WITH_LOGS
        lwm2m_printf("failed to duplicate %s: %m\n", strPtr);
#endif
        return NULL;
    }
    return dstrPtr;
}

#endif

//--------------------------------------------------------------------------------------------------
/**
 * Compare strings
 *
 * @return
 *  - integer less than, equal to, or greater than zero if s1Ptr (or the first n bytes thereof) is
 *    found, respectively, to be less than, to  match,  or  begreater than s2Ptr.
 */
//--------------------------------------------------------------------------------------------------
int lwm2m_strncmp
(
    const char* s1Ptr,  ///< [IN] First string to be compared
    const char* s2Ptr,  ///< [IN] Second string to be compared
    size_t n            ///< [IN] Comparison length
)
{
    return strncmp(s1Ptr, s2Ptr, n);
}

//--------------------------------------------------------------------------------------------------
/**
 * Memory reallocation
 *
 * @return
 *  - memory address
 */
//--------------------------------------------------------------------------------------------------
void* lwm2mcore_realloc
(
    void*  ptr,         ///< [IN] Data address
    size_t newSize      ///< [IN] New size allocation
)
{
    return realloc(ptr, newSize);
}
