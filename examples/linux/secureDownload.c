/**
 * @file secureDownload.c
 *
 * Porting layer for package download
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <platform/types.h>
#include <ctype.h>
#include <lwm2mcore/lwm2mcore.h>
#include <lwm2mcore/security.h>
#include <lwm2mcore/update.h>
#include <lwm2mcore/lwm2mcorePackageDownloader.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#ifdef OPENSSL
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/ssl.h>
#include "sslUtilities.h"
#elif defined MBEDTLS
#include "mbedtls/net_sockets.h"
#include "mbedtls/ssl.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/entropy.h"
#include "defaultDerKey.h"
#include "mbedtls/debug.h"
#endif

#include "errno.h"


//--------------------------------------------------------------------------------------------------
/**
 * Socket fd
 */
//--------------------------------------------------------------------------------------------------
int SocketFd = -1;

#ifdef OPENSSL
//--------------------------------------------------------------------------------------------------
/**
 *                      OPEN SSL
 */
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Static BIO pointer
 */
//--------------------------------------------------------------------------------------------------
static BIO* BioPtr;

//--------------------------------------------------------------------------------------------------
/**
 * Static for SSL_CTX object
 */
//--------------------------------------------------------------------------------------------------
static SSL_CTX* CtxPtr = NULL;

//--------------------------------------------------------------------------------------------------
/**
 * Static for SSL_CTX object
 */
//--------------------------------------------------------------------------------------------------
static SSL* SslPtr = NULL;

#elif MBEDTLS
//--------------------------------------------------------------------------------------------------
/**
 *                      MbedTLS
 */
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * MbedTLS wrapper for socket
 */
//--------------------------------------------------------------------------------------------------
static mbedtls_net_context ServerFd;

//--------------------------------------------------------------------------------------------------
/**
 * CTR_DRBG context structure
 */
//--------------------------------------------------------------------------------------------------
static mbedtls_ctr_drbg_context CtrDrbg;

//--------------------------------------------------------------------------------------------------
/**
 * SSL/TLS context
 */
//--------------------------------------------------------------------------------------------------
static mbedtls_ssl_context SslCtx;

//--------------------------------------------------------------------------------------------------
/**
 * SSL/TLS configuration
 */
//--------------------------------------------------------------------------------------------------
static mbedtls_ssl_config SslConf;

//--------------------------------------------------------------------------------------------------
/**
 * Entropy context structure
 */
//--------------------------------------------------------------------------------------------------
static mbedtls_entropy_context Entropy;

//--------------------------------------------------------------------------------------------------
/**
 * Static structure for X.509 certificate
 */
//--------------------------------------------------------------------------------------------------
static mbedtls_x509_crt CaCert;

//--------------------------------------------------------------------------------------------------
/**
 * Debug API for mbedTLS
 */
//--------------------------------------------------------------------------------------------------
static void my_debug(
    void *ctxPtr,           ///< [IN] Context
    int level,              ///< [IN] Log level
    const char *filePtr,    ///< [IN] File in which the log is called
    int line,               ///< [IN] File line in which the log is called
    const char *strPtr      ///< [IN] Log data
)
{
    ((void) level);
    fprintf( (FILE *) ctxPtr, "%s:%04d: %s", filePtr, line, strPtr );
    fflush(  (FILE *) ctxPtr  );
}

#endif

//--------------------------------------------------------------------------------------------------
/**
 * Static filde descriptor to store the downloaded data
 */
//--------------------------------------------------------------------------------------------------
static int FdOutput = -1;

#ifdef OPENSSL
//--------------------------------------------------------------------------------------------------
/**
 * Print SSL error details
 */
//--------------------------------------------------------------------------------------------------
static void PrintSslError
(
    const char* messagePtr,     ///< [IN] Error message
    FILE* outPtr                ///< [IN] File descriptor for output
)
{
    fprintf(outPtr, "%s\n", messagePtr);
    fprintf(outPtr, "Error: %s\n", ERR_reason_error_string(ERR_get_error()));
    fprintf(outPtr, "%s\n", ERR_error_string(ERR_get_error(), NULL));
    ERR_print_errors_fp(outPtr);
}

//--------------------------------------------------------------------------------------------------
/**
 * Print SSL error details with inserted content
 */
//--------------------------------------------------------------------------------------------------
static void PrintSslError2
(
    const char* messagePtr,     ///< [IN] Error message
    const char* contentPtr,     ///< [IN] Error content
    FILE* outPtr                ///< [IN] File descriptor for output
)
{
    fprintf(outPtr, messagePtr, contentPtr);
    fprintf(outPtr, "Error2: %s\n", ERR_reason_error_string(ERR_get_error()));
    fprintf(outPtr, "%s\n", ERR_error_string(ERR_get_error(), NULL));
    ERR_print_errors_fp(outPtr);
}
#endif /* OPENSSL */

//--------------------------------------------------------------------------------------------------
/**
 * Write to a stream and handle restarts if nessecary
 *
 * @return
 *  - Data length written
 *  - 0 or -1 on failure
 *
 */
//--------------------------------------------------------------------------------------------------
static int WriteToStream
(
    void*   sslCtxPtr,      ///< [IN] SSL context
    char*   bufferPtr,      ///< [IN] Data to be sent
    int     length          ///< [IN] Data length
)
{
#ifdef OPENSSL
    int r = -1;
    BIO* bioPtr;

    if ((!bufferPtr) || (!sslCtxPtr))
    {
        return -1;
    }

    bioPtr = (BIO*)sslCtxPtr;
    while (r < 0)
    {
        r = BIO_write(bioPtr, bufferPtr, length);
        if (r <= 0)
        {
            if (!BIO_should_retry(bioPtr))
            {
                PrintSslError("BIO_read should retry test failed", stdout);
                continue;
            }
            /* It would be prudent to check the reason for the retry and handle
             * it appropriately here
             */
        }
    }
    return r;
#elif MBEDTLS
    int r;

    if ((!bufferPtr) || (!sslCtxPtr))
    {
        return -1;
    }

    r = mbedtls_ssl_write(sslCtxPtr, (const unsigned char*)bufferPtr, (size_t)length);
    if (0 >= r)
    {
        printf("Error on write %d\n", r);
        return r;
    }

    if (0 < r)
    {
        size_t newLength = length;
        while (r > 0)
        {
            newLength -= r;
            bufferPtr = bufferPtr + r;
            r = mbedtls_ssl_write(sslCtxPtr, (const unsigned char*)bufferPtr, newLength);
        }
    }

    if (0 > r)
    {
        printf("Error on write %d\n", r);
    }
    return r;
#endif
}

//--------------------------------------------------------------------------------------------------
/**
 * Read a from a stream and handle restarts if necessary
 *
 * * @return
 *  - Data length read
 *  - 0 or -1 on failure
 */
//--------------------------------------------------------------------------------------------------
static int ReadFromStream
(
    void*   sslCtxPtr,      ///< [IN] SSL context
    char*   bufferPtr,      ///< [IN] Buffer
    int     length          ///< [IN] Buffer length
)
{
#ifdef OPENSSL
    int r = -1;
    BIO* bioPtr;

    if ((!bufferPtr) || (!sslCtxPtr))
    {
        return -1;
    }

    bioPtr = (BIO*)sslCtxPtr;

    while (r < 0)
    {
        r = BIO_read(bioPtr, bufferPtr, length);
        if (r == 0)
        {
            PrintSslError("Reached the end of the data stream.\n", stdout);
            continue;

        }
        else if (r < 0)
        {
            if (!BIO_should_retry(bioPtr))
            {
                PrintSslError("BIO_read should retry test failed", stdout);
                continue;
            }
            /* It would be prudent to check the reason for the retry and handle
             * it appropriately here */
        }
    };

    return r;
#elif MBEDTLS
    int r;

    if ((!bufferPtr) || (!sslCtxPtr))
    {
        return -1;
    }

    r = mbedtls_ssl_read(sslCtxPtr, (unsigned char*)bufferPtr, (size_t)length);
    if (r == 0)
    {
        printf("Reached the end of the data stream.\n");
    }

    if (0 > r)
    {
        printf("Error on read %d\n", r);
    }

    return r;
#endif
}

#ifdef OPENSSL
//--------------------------------------------------------------------------------------------------
/**
 * Connect to a host using an encrypted stream
 *
 * @return
 *  - BIO pointer on success
 *  - NULL on failure
 */
//--------------------------------------------------------------------------------------------------
static BIO* ConnectEncrypted
(
    char*       hostPtr,            ///< [IN] Host to connect on
    uint16_t    port,               ///< [IN] Port to connect on
    const char* storePathPtr,       ///< [IN] Certificate path
    SSL_CTX**   ctxPtr,             ///< [IN] SSL_CTX pointer
    SSL**       sslPtr              ///< [IN] SSL pointer
)
{
    BIO* bioPtr = NULL;
    int r = 0;
    char hostAndPortPtr[LWM2MCORE_PACKAGE_URI_MAX_BYTES];

    snprintf(hostAndPortPtr, LWM2MCORE_PACKAGE_URI_MAX_LEN, "%s:%d", hostPtr, port);
    printf("ConnectEncrypted: %s\n", hostAndPortPtr);

    /* Set up the SSL pointers */
#if OPENSSL_VERSION_NUMBER < 0x10100000L
    *ctxPtr = SSL_CTX_new(TLSv1_client_method());
#else
    *ctxPtr = SSL_CTX_new(TLS_client_method());
#endif
    *sslPtr = NULL;
    r = SSL_CTX_load_verify_locations(*ctxPtr, storePathPtr, NULL);

    if (0 == r)
    {
        PrintSslError2("Unable to load the trust store from %s", storePathPtr, stdout);
        return NULL;
    }

    /* Setting up the BIO SSL object */
    bioPtr = BIO_new_ssl_connect(*ctxPtr);
    BIO_get_ssl(bioPtr, sslPtr);
    if (!(*sslPtr))
    {

        PrintSslError("Unable to allocate SSL pointer", stdout);
        return NULL;
    }
    /* Set the SSL_MODE_AUTO_RETRY flag: it will cause read/write operations to only return after
     * the handshake and successful completion
     */
    SSL_set_mode(*sslPtr, SSL_MODE_AUTO_RETRY);

    /* Attempt to connect, this function always returns 1 */
    BIO_set_conn_hostname(bioPtr, hostAndPortPtr);

    /* Verify the connection opened and perform the handshake.
     * This function returns 1 if the connection was successfully established and 0 or -1 if the
     * connection failed.
     */
    if (1 > BIO_do_connect(bioPtr))
    {

        PrintSslError2("Unable to connect BIO %s", hostAndPortPtr, stdout);
        return NULL;
    }

    /*if (X509_V_OK != SSL_get_verify_result(*sslPtr))
    {
        PrintSslError("Unable to verify connection result", stdout);
    }*/

    return bioPtr;
}
#endif /* OPENSSL */

//--------------------------------------------------------------------------------------------------
/**
 * Function to initialize a package download
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @note The returned pointer needs to be deallocated on client side
 *
 * @return
 *  - Package download context
 *  - @c NULL on failure
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_PackageDownloadContext_t* lwm2mcore_InitForDownload
(
    bool    isHttps     ///< [IN] true if HTTPS is requested, else HTTP is requested
)
{
    lwm2mcore_PackageDownloadContext_t* contextPtr =
                                                malloc(sizeof(lwm2mcore_PackageDownloadContext_t));

    if (!contextPtr)
    {
        return NULL;
    }
    contextPtr->isInitMade = false;

    if (isHttps)
    {
        contextPtr->isSecure = true;
#ifdef OPENSSL
        if (-1 == ssl_CheckCertificate())
        {
            lwm2mcore_FreeForDownload(contextPtr);
            return NULL;
        }

        /* This function always returns 1 (no need to check the returned value) */
        SSL_library_init();
#elif MBEDTLS
        int ret;
        const char *persPtr = "mini_client";

        /*
         * 0. Initialize the RNG and the session data
         */
        mbedtls_net_init(&ServerFd);
        mbedtls_ssl_init(&SslCtx);
        mbedtls_ssl_config_init(&SslConf);
        mbedtls_ctr_drbg_init(&CtrDrbg);

        printf("\n  . Seeding the random number generator...");
        fflush(stdout);
        mbedtls_entropy_init(&Entropy);
        ret = mbedtls_ctr_drbg_seed(&CtrDrbg,
                                    mbedtls_entropy_func,
                                    &Entropy,
                                    (const unsigned char*) persPtr,
                                    strlen( persPtr ));
        if (ret)
        {
            printf(" failed\n  ! mbedtls_ctr_drbg_seed returned %d\n", ret);
            lwm2mcore_FreeForDownload(contextPtr);
            return NULL;
        }
        printf(" ok\n");

        /*
         * 1. Initialize certificates
         */
        printf("  . Loading the CA root certificate ...");
        fflush(stdout);

        ret = mbedtls_x509_crt_parse(&CaCert,
                                     (const unsigned char*)DefaultDerKey,
                                     DEFAULT_DER_KEY_LEN );
        if (ret < 0)
        {
            printf( " failed\n  !  mbedtls_x509_crt_parse returned -0x%x\n\n", -ret );
            lwm2mcore_FreeForDownload(contextPtr);
            return NULL;
        }

        printf(" ok (%d skipped)\n", ret);



        if (mbedtls_ssl_config_defaults(&SslConf,
                                        MBEDTLS_SSL_IS_CLIENT,
                                        MBEDTLS_SSL_TRANSPORT_STREAM,
                                        MBEDTLS_SSL_PRESET_DEFAULT ))
        {
            printf("ssl_config_defaults_failed\n");
            lwm2mcore_FreeForDownload(contextPtr);
            return NULL;
        }

        mbedtls_ssl_conf_rng(&SslConf, mbedtls_ctr_drbg_random, &CtrDrbg);
#endif
    }
    else
    {
        contextPtr->isSecure = false;
    }
    contextPtr->isInitMade = true;
    return contextPtr;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to initiate the connection for package download
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - LWM2MCORE_ERR_COMPLETED_OK on success
 *  - LWM2MCORE_ERR_INVALID_ARG if the parameter is invalid
 *  - LWM2MCORE_ERR_GENERAL_ERROR on failure
 *  - LWM2MCORE_ERR_MEMORY on memory allocation issue
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_ConnectForDownload
(
    lwm2mcore_PackageDownloadContext_t* contextPtr, ///< [IN] Package donwload context
    char*                               hostPtr,    ///< [IN] Host to connect on
    uint16_t                            port        ///< [IN] Port to connect on
)
{
    if ((!contextPtr) || (!hostPtr))
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    if (contextPtr->isSecure)
    {
#ifdef OPENSSL
        if ((BioPtr = ConnectEncrypted(hostPtr, port, PEMCERT_PATH, &CtxPtr, &SslPtr)) == NULL)
        {
            return LWM2MCORE_ERR_GENERAL_ERROR;
        }
#elif MBEDTLS
        int ret;
        char portBuffer[6];
        uint32_t flags;

        if (!hostPtr)
        {
            return LWM2MCORE_ERR_INVALID_ARG;
        }

        /*
         * 1. Start the connection
         */
        snprintf(portBuffer, sizeof(portBuffer), "%d", port);
        printf("  . Connecting to tcp/%s:%d - %s:%s...", hostPtr, port, hostPtr, portBuffer);
        fflush(stdout);

        if ( ( ret = mbedtls_net_connect(&ServerFd,
                                         hostPtr,
                                         portBuffer,
                                         MBEDTLS_NET_PROTO_TCP ) ) != 0 )
        {
            printf(" failed\n  ! mbedtls_net_connect returned %d\n\n", ret);
            return LWM2MCORE_ERR_GENERAL_ERROR;
        }

        printf(" ok\n");

        /*
         * 2. Setup stuff
         */
        printf("  . Setting up the SSL/TLS structure...");
        fflush(stdout);

        if ( ( ret = mbedtls_ssl_config_defaults(&SslConf,
                                                MBEDTLS_SSL_IS_CLIENT,
                                                MBEDTLS_SSL_TRANSPORT_STREAM,
                                                MBEDTLS_SSL_PRESET_DEFAULT ) ) != 0 )
        {
            printf(" failed\n  ! mbedtls_ssl_config_defaults returned %d\n\n", ret);
            return LWM2MCORE_ERR_GENERAL_ERROR;
        }

        printf(" ok\n");

        /* OPTIONAL is not optimal for security,
         * but makes interop easier in this simplified example */
        mbedtls_ssl_conf_authmode(&SslConf, MBEDTLS_SSL_VERIFY_OPTIONAL);
        mbedtls_ssl_conf_ca_chain(&SslConf, &CaCert, NULL);
        mbedtls_ssl_conf_rng(&SslConf, mbedtls_ctr_drbg_random, &CtrDrbg);
        mbedtls_debug_set_threshold(1);
        mbedtls_ssl_conf_dbg(&SslConf, my_debug, stdout);

        if ( ( ret = mbedtls_ssl_setup(&SslCtx, &SslConf) ) != 0 )
        {
            printf(" failed\n  ! mbedtls_ssl_setup returned %d\n\n", ret);
            return LWM2MCORE_ERR_GENERAL_ERROR;
        }

        if ( ( ret = mbedtls_ssl_set_hostname(&SslCtx, hostPtr) ) != 0 )
        {
            printf(" failed\n  ! mbedtls_ssl_set_hostname returned %d\n\n", ret);
            return LWM2MCORE_ERR_GENERAL_ERROR;
        }

        mbedtls_ssl_set_bio(&SslCtx, &ServerFd, mbedtls_net_send, mbedtls_net_recv, NULL);

        /*
         * 4. Handshake
         */
        printf("  . Performing the SSL/TLS handshake...");
        fflush(stdout);

        while (( ret = mbedtls_ssl_handshake(&SslCtx) ) != 0)
        {
            if ((ret != MBEDTLS_ERR_SSL_WANT_READ) && (ret != MBEDTLS_ERR_SSL_WANT_WRITE))
            {
                printf(" failed\n  ! mbedtls_ssl_handshake returned -0x%x\n\n", -ret);
                return LWM2MCORE_ERR_GENERAL_ERROR;
            }
        }

        printf(" ok\n");

        /*
         * 5. Verify the server certificate
         */
        //printf("  . Verifying peer X.509 certificate...");

        /* In real life, we probably want to bail out when ret != 0 */
        /*if( ( flags = mbedtls_ssl_get_verify_result(&SslCtx) ) != 0 )
        {
            char vrfy_buf[512];

            printf(" failed\n");

            mbedtls_x509_crt_verify_info(vrfy_buf, sizeof(vrfy_buf), "  ! ", flags);

            printf("%s\n", vrfy_buf);
        }
        else
        {
            printf(" ok\n");
        }*/
#endif
    }
    else
    {
        struct addrinfo* servInfoPtr = NULL;
        struct addrinfo* aiPtr = NULL;

        if (getaddrinfo(hostPtr, "http", NULL, &servInfoPtr))
        {
            goto error;
        }

        for (aiPtr = servInfoPtr; aiPtr != NULL; aiPtr = aiPtr->ai_next)
        {
            if ((SocketFd = socket(aiPtr->ai_family, aiPtr->ai_socktype,aiPtr->ai_protocol)) == -1)
            {
                fprintf(stderr, "Socket error: %m\n");
                continue;
            }
            if (connect(SocketFd, aiPtr->ai_addr, aiPtr->ai_addrlen) == -1)
            {
                fprintf(stderr, "Connect error: %m\n");
                close(SocketFd);
                continue;
            }
            break;
        }
        freeaddrinfo(servInfoPtr);
        servInfoPtr = NULL;

        return LWM2MCORE_ERR_COMPLETED_OK;

error:
        if (-1 != SocketFd)
        {
            close(SocketFd);
        }
        if (servInfoPtr)
        {
            freeaddrinfo(servInfoPtr);
        }
        return LWM2MCORE_ERR_GENERAL_ERROR;
    }
    return LWM2MCORE_ERR_COMPLETED_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to disconnect the connection for package download
 *
 * @return
 *  - LWM2MCORE_ERR_COMPLETED_OK on success
 *  - LWM2MCORE_ERR_GENERAL_ERROR on failure
 *  - LWM2MCORE_ERR_INVALID_ARG if the parameter is invalid
 *  - LWM2MCORE_ERR_INVALID_STATE if no connection was initiated for package download
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_DisconnectForDownload
(
    lwm2mcore_PackageDownloadContext_t* contextPtr  ///< [IN] Package donwload context
)
{
    if (!contextPtr)
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    if (contextPtr->isSecure)
    {
#ifdef OPENSSL
        if (!BioPtr)
        {
            printf("!BioPtr\n");
            return LWM2MCORE_ERR_INVALID_STATE;
        }
#endif

        if (-1 != FdOutput)
        {
            close(FdOutput);
            FdOutput = -1;
        }

#ifdef OPENSSL
        BIO_ssl_shutdown(BioPtr);
#elif MBEDTLS
        mbedtls_net_free(&ServerFd);
#endif
    }
    else
    {
        close(SocketFd);
        SocketFd = -1;
    }
    return LWM2MCORE_ERR_COMPLETED_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to free the connection for package download
 *
 * @return
 *  - LWM2MCORE_ERR_COMPLETED_OK on success
 *  - LWM2MCORE_ERR_GENERAL_ERROR on failure
 *  - LWM2MCORE_ERR_INVALID_ARG if the parameter is invalid
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_FreeForDownload
(
    lwm2mcore_PackageDownloadContext_t* contextPtr  ///< [IN] Package donwload context
)
{
    if (!contextPtr)
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    if (contextPtr->isSecure)
    {
#ifdef OPENSSL
        BIO_free_all(BioPtr);
        BioPtr = NULL;
        SSL_CTX_free(CtxPtr);
#if OPENSSL_API_COMPAT < 0x10100000L
#if OPENSSL_API_COMPAT > 0x10002000L
        // In versions of OpenSSL prior to 1.1.0 SSL_COMP_free_compression_methods() freed the
        // internal table of compression methods that were built internally, and possibly augmented
        // by adding SSL_COMP_add_compression_method().
        // However this is now unnecessary from version 1.1.0. No explicit initialisation or
        // de-initialisation is necessary.
        SSL_COMP_free_compression_methods();
#endif
#endif
        EVP_cleanup();
        ERR_free_strings();

        //FIPS_mode_set(0);
        //ENGINE_cleanup();
        //CONF_modules_unload(1);
        CRYPTO_cleanup_all_ex_data();
#if OPENSSL_VERSION_NUMBER < 0x10100000L
        ERR_remove_state(0);
#endif
#elif MBEDTLS
        mbedtls_net_free(&ServerFd);
        mbedtls_x509_crt_free(&CaCert);
        mbedtls_ssl_free(&SslCtx);
        mbedtls_ssl_config_free(&SslConf);
        mbedtls_ctr_drbg_free( &CtrDrbg );
        mbedtls_entropy_free(&Entropy);
#endif
    }

    free(contextPtr);
    return LWM2MCORE_ERR_COMPLETED_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to send a HTTP request for package download
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - LWM2MCORE_ERR_COMPLETED_OK on success
 *  - LWM2MCORE_ERR_INVALID_ARG if the request is invalid
 *  - LWM2MCORE_ERR_GENERAL_ERROR on failure
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_SendForDownload
(
    lwm2mcore_PackageDownloadContext_t* contextPtr,         ///< [IN] Package donwload context
    char*                               serverRequestPtr    ///< [IN] HTTP(S) request
)
{
    if ((!contextPtr) || (!serverRequestPtr))
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    printf("Request sent to the server:\n%s\n", serverRequestPtr);

    if (contextPtr->isSecure)
    {
#ifdef OPENSSL
        if (0 >= WriteToStream(BioPtr, serverRequestPtr, (int)strlen(serverRequestPtr)))
#elif MBEDTLS
        if (0 >= WriteToStream(&SslCtx, serverRequestPtr, (int)strlen(serverRequestPtr)))
#endif
        {
            return LWM2MCORE_ERR_GENERAL_ERROR;
        }
    }
    else
    {
        int len = send(SocketFd, serverRequestPtr, strlen(serverRequestPtr), 0);
        if (len != (int)strlen(serverRequestPtr))
        {
            fprintf(stderr, "Send error %m\n");
            lwm2mcore_DisconnectForDownload(contextPtr);
            return LWM2MCORE_ERR_GENERAL_ERROR;
        }
    }
    return LWM2MCORE_ERR_COMPLETED_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to read received data for package download
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - LWM2MCORE_ERR_COMPLETED_OK on success
 *  - LWM2MCORE_ERR_INVALID_ARG if the request is invalid
 *  - LWM2MCORE_ERR_GENERAL_ERROR on failure
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_ReadForDownload
(
    lwm2mcore_PackageDownloadContext_t* contextPtr,     ///< [IN] Package donwload context
    char*                               bufferPtr,      ///< [INOUT] Buffer
    int*                                lenPtr          ///< [IN] Buffer length
)
{
    if ((!contextPtr) || (!bufferPtr) || (!lenPtr))
    {
        return LWM2MCORE_ERR_INVALID_ARG;
    }

    if (contextPtr->isSecure)
    {
#ifdef OPENSSL
        if ((*lenPtr = ReadFromStream(BioPtr, bufferPtr, *lenPtr)) < 0)
#elif MBEDTLS
        if ((*lenPtr = ReadFromStream(&SslCtx, bufferPtr, *lenPtr)) < 0)
#endif
        {
            return LWM2MCORE_ERR_GENERAL_ERROR;
        }
    }
    else
    {
        int ndata = recv(SocketFd, bufferPtr, (size_t)(*lenPtr), 0);
        if (ndata <= 0)
        {
            fprintf(stderr, "Receive error %m\n");
            lwm2mcore_DisconnectForDownload(contextPtr);
            return LWM2MCORE_ERR_GENERAL_ERROR;
        }
        *lenPtr = ndata;
    }

    return LWM2MCORE_ERR_COMPLETED_OK;
}

//--------------------------------------------------------------------------------------------------
/**
 * Write data
 *
 * This function is called in a dedicated thread/task.
 *
 * @return
 *  - LWM2MCORE_ERR_COMPLETED_OK on success
 *  - LWM2MCORE_ERR_INVALID_STATE if no package download is suspended
 *  - LWM2MCORE_ERR_GENERAL_ERROR on failure
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_WritePackageData
(
    uint8_t* bufferPtr,     ///< [IN] Data to be written
    uint32_t length,        ///< [IN] Data length
    void*    opaquePtr      ///< [IN] Opaque pointer
)
{
    struct stat sb;
    int lwrite;

    (void)opaquePtr;

    if (-1 == stat("download.bin", &sb))
    {
        printf("Create the output file to store downloaded data\n");
        FdOutput = open("download.bin", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    }
    else
    {
        if(-1 == FdOutput)
        {
            FdOutput = open("download.bin", O_WRONLY | O_APPEND, S_IRUSR | S_IWUSR);
        }
    }
    lwrite = write(FdOutput, bufferPtr, length);
    if (-1 == lwrite)
    {
        fprintf(stderr, "Write error %m\n");
        return LWM2MCORE_ERR_GENERAL_ERROR;
    }
    return LWM2MCORE_ERR_COMPLETED_OK;
}
