//-------------------------------------------------------------------------------------------------
/**
 * @file tinydtls_stub.c
 *
 * Stub code for tinydtls functions.
 *
 * Copyright (C) Sierra Wireless Inc.
 */
//-------------------------------------------------------------------------------------------------

#include "dtls.h"

void dtls_init
(
    void
)
{
    return;
}

dtls_context_t* dtls_new_context
(
    void *app_data
)
{
    (void)app_data;
    return NULL;
}

void dtls_free_context
(
    dtls_context_t* ctx
)
{
    (void)ctx;
    return;
}

int dtls_write
(
    struct dtls_context_t* ctx,
    session_t* dst,
    uint8* buf,
    size_t len
)
{
    (void)ctx;
    (void)dst;
    (void)buf;
    (void)len;
    return -1;
}

int dtls_handle_message
(
    dtls_context_t* ctx,
    session_t* session,
    uint8* msg,
    int msglen
)
{
    (void)ctx;
    (void)session;
    (void)msg;
    (void)msglen;
    return -1;
}

dtls_peer_t* dtls_get_peer
(
    const dtls_context_t* ctx,
    const session_t* session
)
{
    (void)ctx;
    (void)session;
    return NULL;
}

void dtls_reset_peer
(
    dtls_context_t* ctx,
    dtls_peer_t* peer
)
{
    (void)ctx;
    (void)peer;
    return;
}

int dtls_connect_peer
(
    dtls_context_t* ctx,
    dtls_peer_t* peer
)
{
    (void)ctx;
    (void)peer;
    return -1;
}

int dtls_connect
(
    dtls_context_t* ctx,
    const session_t *dst
)
{
    (void)ctx;
    (void)dst;
    return -1;
}
