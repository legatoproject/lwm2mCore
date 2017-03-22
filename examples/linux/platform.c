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

void *lwm2m_malloc(size_t size)
{
    void *mem;

    mem = malloc(size);
    if (!mem) {
#ifdef LWM2M_WITH_LOGS
    lwm2m_printf("out of memory\n");
#endif
    return NULL;
}
    return mem;
}

void lwm2m_free(void *p)
{
    free(p);
}

char *lwm2m_strdup(const char *str)
{
    char *dstr;

    dstr = strdup(str);
    if (!dstr) {
#ifdef LWM2M_WITH_LOGS
    lwm2m_printf("failed to duplicate %s: %m\n", str);
#endif
    return NULL;
}
    return dstr;
}

#endif

int lwm2m_strncmp(const char *s1, const char *s2, size_t n)
{
    return strncmp(s1, s2, n);
}
