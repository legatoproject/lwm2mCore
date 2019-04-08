/**
 * @file sslUtilities.h
 *
 * <HR>
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#ifndef _SSLUTILITIES_H
#define _SSLUTILITIES_H

#include <stddef.h>

#define SSLCERT_PATH "cert"

#define PEMCERT_PATH  "mycert.pem"

//--------------------------------------------------------------------------------------------------
/**
 * Certificate max length
 */
//--------------------------------------------------------------------------------------------------
#define MAX_CERT_LEN    8192

//--------------------------------------------------------------------------------------------------
/**
 * Check if SSL certificate exists and load it
 */
//--------------------------------------------------------------------------------------------------
int ssl_CheckCertificate
(
    void
);

#endif /* _SSLUTILITIES_H */
