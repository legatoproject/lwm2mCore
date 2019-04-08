/**
 * @file sslUtilities.c
 *
 * <HR>
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */


#include <stdio.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#ifdef OPENSSL
#include <openssl/bio.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/x509.h>
#include "defaultDerKey.h"
#elif defined MBEDTLS
#include "mbedtls/net_sockets.h"
#include "mbedtls/ssl.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/entropy.h"
#endif
#include "sslUtilities.h"
#include <fcntl.h>
#include <string.h>
#include <stdint.h>

#ifdef OPENSSL

//--------------------------------------------------------------------------------------------------
/**
 * Base64 line break position
 */
//--------------------------------------------------------------------------------------------------
#define BASE64_NL           64

//--------------------------------------------------------------------------------------------------
/**
 * PEM certificate header
 */
//--------------------------------------------------------------------------------------------------
#define PEM_CERT_HEADER     "-----BEGIN CERTIFICATE-----"

//--------------------------------------------------------------------------------------------------
/**
 * PEM certificate footer
 */
//--------------------------------------------------------------------------------------------------
#define PEM_CERT_FOOTER     "-----END CERTIFICATE-----"

#ifdef MBEDTLS
static mbedtls_x509_crt Ca;
#endif

//--------------------------------------------------------------------------------------------------
/**
 * Convert a DER key to PEM key
 *
 */
//--------------------------------------------------------------------------------------------------
static int ConvertDERToPEM
(
    unsigned char*  derKeyPtr,      ///< [IN] DER key
    int             derKeyLen,      ///< [IN] DER key length
    unsigned char*  pemKeyPtr,      ///< [OUT] PEM key
    uint16_t        pemKeyLen       ///< [IN] PEM key length
)
{
#ifdef OPENSSL
    X509 *certPtr;
    BIO *memPtr;
    int countRead;
    uint32_t countWrite;

    if ( (!derKeyPtr) || (!pemKeyPtr) )
    {
        fprintf(stderr,
                "invalid input arguments: derKeyPtr (%p), pemKeyPtr(%p)\n",
                derKeyPtr, pemKeyPtr);
        return -1;
    }

    if (!derKeyLen)
    {
        fprintf(stderr, "derKeyLen cannot be 0\n");
        return -1;
    }

    certPtr = d2i_X509(NULL, (const unsigned char **)&derKeyPtr, derKeyLen);
    if (!certPtr)
    {
        fprintf(stderr, "unable to parse certificate: %lu\n", ERR_get_error());
        return -1;
    }

    memPtr = BIO_new(BIO_s_mem());
    if (!memPtr)
    {
        fprintf(stderr, "failed to set BIO type: %lu\n", ERR_get_error());
        goto x509_err;
    }

    if (!PEM_write_bio_X509(memPtr, certPtr))
    {
        fprintf(stderr, "failed to write certificate: %lu\n", ERR_get_error());
        goto bio_err;
    }

    if (BIO_number_written(memPtr) > pemKeyLen)
    {
        fprintf(stderr, "not enough space to hold the key\n");
        goto bio_err;
    }

    memset(pemKeyPtr, 0, BIO_number_written(memPtr) + 1);

    countRead = BIO_read(memPtr, pemKeyPtr, BIO_number_written(memPtr));
    if ( 0 >= countRead)
    {
        fprintf(stderr, "error on BIO read %d\n", countRead);
        goto bio_err;
    }
    countWrite = (uint32_t)countRead;

    if (countWrite < BIO_number_written(memPtr))
    {
        fprintf(stderr,
                "failed to read certificate: count (%d): %d\n",
                countWrite,
                (int)ERR_get_error());
        goto pem_err;
    }

    BIO_free(memPtr);
    X509_free(certPtr);

    return countRead;

pem_err:
    memset(pemKeyPtr, 0, BIO_number_written(memPtr) + 1);
bio_err:
    BIO_free(memPtr);
x509_err:
    X509_free(certPtr);
    return -1;

#elif MBEDTLS
    unsigned char buffer[4096];

    mbedtls_x509_crt_init(&Ca);
    if (mbedtls_x509_crt_parse_der(&Ca, derKeyPtr, derKeyLen))
    {
        fprintf(stderr, "x509_crt_parse failed\n");
        return -1;
    }
    printf("X509 parse OK\n");

    if (mbedtls_x509write_crt_pem(&Ca, buffer, sizeof(buffer), NULL, NULL))
    {
        fprintf(stderr, "mbedtls_x509write_crt_pem failed\n");
        return -1;
    }
    printf("mbedtls_x509write_crt_pem OK\n");

#endif
}

//--------------------------------------------------------------------------------------------------
/**
 * Load default certificate
 */
//--------------------------------------------------------------------------------------------------
static int LoadDefaultCertificate
(
    void
)
{
    unsigned char cert[MAX_CERT_LEN] = {0};
    int len;
    int fd;
    ssize_t size;

    len = ConvertDERToPEM(DefaultDerKey, DEFAULT_DER_KEY_LEN, cert, MAX_CERT_LEN);
    if (-1 == len)
    {
        return -1;
    }

    fd = open(SSLCERT_PATH, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

    if (-1 == fd)
    {
        fprintf(stderr, "Error on opening certificate\n");
        return -1;
    }

    size = write(fd, cert, len);

    close(fd);

    if (-1 == size)
    {
        fprintf(stderr, "Error on write %m\n");
        return -1;
    }

    return 0;
}

//--------------------------------------------------------------------------------------------------
/**
 * Write PEM key to default certificate file path
 */
//--------------------------------------------------------------------------------------------------
static int WritePEMCertificate
(
    const char*     certPtr,
    unsigned char*  pemKeyPtr,
    int             pemKeyLen
)
{
    int fd;
    ssize_t count;

    fd = open(certPtr, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fd < 0)
    {
        fprintf(stderr, "failed to open %s: %m\n", certPtr);
        return -1;
    }

    while (pemKeyLen)
    {
        count = write(fd, pemKeyPtr, pemKeyLen);
        if (count == -1)
        {
            fprintf(stderr, "failed to write PEM cert: %m\n");
            close(fd);
            return -1;
        }
        else if (0 < count)
        {
            pemKeyLen -= count;
            pemKeyPtr += count;
        }
        else if (!count)
        {
            fprintf(stderr, "Write returns 0\n");
            close (fd);
            return -1;
        }
    }

    close(fd);
    return 0;
}

//--------------------------------------------------------------------------------------------------
/**
 * Check if SSL certificate exists and load it
 */
//--------------------------------------------------------------------------------------------------
int ssl_CheckCertificate
(
    void
)
{
    unsigned char buf[MAX_CERT_LEN] = {0};
    int result;

    int   fd = open(SSLCERT_PATH, O_RDONLY);

    if (fd < 0)
    {
        fprintf(stdout, "SSL certificate not found, loading default certificate\n");
        result = LoadDefaultCertificate();
        fprintf(stdout, "LoadDefaultCertificate %d\n", result);
        if (0 != result)
        {
            return result;
        }

        fd = open(SSLCERT_PATH, O_RDONLY);
    }
    else
    {
        fprintf(stdout, "Using saved SSL certificate\n");
    }

    result = read(fd, buf, MAX_CERT_LEN);
    if (0 >= result)
    {
        close(fd);
        return result;
    }

    close(fd);
    return WritePEMCertificate(PEMCERT_PATH, buf, result);
}
#endif
