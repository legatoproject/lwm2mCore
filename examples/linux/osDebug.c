/**
 * @file osDebug.c
 *
 * Adaptation layer for debug
 *
 * <hr>
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

//--------------------------------------------------------------------------------------------------
/**
 * Function for assert
 */
//--------------------------------------------------------------------------------------------------
void os_assert
(
    bool condition,         /// [IN] Condition to be checked
    char* functionPtr,      /// [IN] Function name which calls the assert function
    uint32_t line           /// [IN] Function line which calls the assert function
)
{
}

//--------------------------------------------------------------------------------------------------
/**
 * Adaptation function for log
 */
//--------------------------------------------------------------------------------------------------
void lwm2m_printf(const char * format, ...)
{
    va_list ap;

    va_start(ap, format);

    vfprintf(stderr, format, ap);

    va_end(ap);
}

//--------------------------------------------------------------------------------------------------
/**
 * Adaptation function for log: dump data
 */
//--------------------------------------------------------------------------------------------------
void os_debug_data_dump
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
        printf ("%s:\n", descPtr);

    if (len == 0) {
        printf("  ZERO LENGTH\n");
        return;
    }
    if (len < 0) {
        printf("  NEGATIVE LENGTH: %i\n",len);
        return;
    }

    // Process every byte in the data.
    for (i = 0; i < len; i++) {
        // Multiple of 16 means new line (with line offset).

        if ((i % 16) == 0) {
            // Just don't print ASCII for the zeroth line.
            if (i != 0)
                printf ("  %s\n", buff);

            // Output the offset.
            printf ("  %04x ", i);
        }

        // Now the hex code for the specific character.
        printf (" %02x", pc[i]);

        // And store a printable ASCII character for later.
        if ((pc[i] < 0x20) || (pc[i] > 0x7e))
            buff[i % 16] = '.';
        else
            buff[i % 16] = pc[i];
        buff[(i % 16) + 1] = '\0';
    }

    // Pad out last line if not exactly 16 characters.
    while ((i % 16) != 0) {
        printf ("   ");
        i++;
    }

    // And print the final ASCII bit.
    printf ("  %s\n", buff);
}

