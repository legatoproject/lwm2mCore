/**
 * @file debug.c
 *
 * Adaptation layer for debug
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <platform/types.h>
#include <stdint.h>

//--------------------------------------------------------------------------------------------------
/**
 * Adaptation function for log
 */
//--------------------------------------------------------------------------------------------------
#if LWM2M_WITH_LOGS
void lwm2m_printf(const char * format, ...)
{
    va_list ap;
    va_start(ap, format);
    vprintf(format, ap);
    va_end(ap);
}
#endif

//--------------------------------------------------------------------------------------------------
/**
 * Function for assert
 */
//--------------------------------------------------------------------------------------------------
void lwm2mcore_Assert
(
    bool condition,         ///< [IN] Condition to be checked
    char* functionPtr,      ///< [IN] Function name which calls the assert function
    uint32_t line           ///< [IN] Function line which calls the assert function
)
{
    char func[32] = "none";

    if (functionPtr)
    {
        memset(func, 0, 32);
        snprintf(func, 31, "%s", functionPtr);
    }

    if (!condition)
    {
        fprintf(stderr, "%s - %d: Assertion failed\n", func, line);
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Adaptation function for log: dump data
 */
//--------------------------------------------------------------------------------------------------
void lwm2mcore_DataDump
(
    char *descPtr,                  ///< [IN] data description
    void *addrPtr,                  ///< [IN] Data address to be dumped
    int len                         ///< [IN] Data length
)
{
    int i;
    unsigned char buff[17];
    unsigned char *pc = (unsigned char*)addrPtr;

    // Output description if given.
    if (descPtr != NULL)
    {
        printf("%s:\n", descPtr);
    }

    if (len == 0)
    {
        printf("  ZERO LENGTH\n");
        return;
    }
    if (len < 0)
    {
        printf("  NEGATIVE LENGTH: %i\n",len);
        return;
    }

    // Process every byte in the data.
    for (i = 0; i < len; i++)
    {
        // Multiple of 16 means new line (with line offset).

        if ((i % 16) == 0)
        {
            // Just don't print ASCII for the zeroth line.
            if (i != 0)
                printf("  %s\n", buff);

            // Output the offset.
            printf("  %04x ", i);
        }

        // Now the hex code for the specific character.
        printf(" %02x", pc[i]);

        // And store a printable ASCII character for later.
        if ((pc[i] < 0x20) || (pc[i] > 0x7e))
        {
            buff[i % 16] = '.';
        }
        else
        {
            buff[i % 16] = pc[i];
        }
        buff[(i % 16) + 1] = '\0';
    }

    // Pad out last line if not exactly 16 characters.
    while ((i % 16) != 0)
    {
        printf("   ");
        i++;
    }

    // And print the final ASCII bit.
    printf("  %s\n", buff);
}
