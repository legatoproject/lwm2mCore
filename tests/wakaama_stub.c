//-------------------------------------------------------------------------------------------------
/**
 * @file wakaama_stub.c
 *
 * Stub code for wakaama functions.
 *
 * Copyright (C) Sierra Wireless Inc.
 */
//-------------------------------------------------------------------------------------------------

#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <lwm2mcore/lwm2mcore.h>
#include "liblwm2m.h"

char* coap_get_multi_option_as_string
(
    char* path
)
{
    return NULL;
}

void lwm2m_data_encode_int
(
    int64_t value,
    lwm2m_data_t* dataP
)
{
    return;
}

void lwm2m_data_encode_bool
(
    bool value,
    lwm2m_data_t* dataP
)
{
    return;
}

void lwm2m_data_encode_nstring
(
    const char* string,
    size_t length,
    lwm2m_data_t* dataP
)
{
    return;
}

void lwm2m_data_encode_opaque
(
    uint8_t* buffer,
    size_t length,
    lwm2m_data_t* dataP
)
{
    return;
}

void lwm2m_data_encode_float
(
    double value,
    lwm2m_data_t* dataP
)
{
    return;
}

lwm2m_data_t* lwm2m_data_new
(
    int size
)
{
    return NULL;
}

void lwm2m_data_encode_instances
(
    lwm2m_data_t* subDataP,
    size_t count,
    lwm2m_data_t* dataP
)
{
    return;
}

lwm2m_list_t* lwm2m_list_find
(
    lwm2m_list_t* head,
    uint16_t id
)
{
    return NULL;
}

int utils_textToInt
(
    uint8_t* buffer,
    int length,
    int64_t* dataP
)
{
    return -1;
}

lwm2m_list_t* lwm2m_list_add
(
    lwm2m_list_t* head,
    lwm2m_list_t* node
)
{
    return NULL;
}

lwm2m_list_t* lwm2m_list_remove
(
    lwm2m_list_t* head,
    uint16_t id,
    lwm2m_list_t** nodeP
)
{
    return NULL;
}

int lwm2m_configure
(
    lwm2m_context_t* contextP,
    const char* endpointName,
    const char* msisdn,
    const char* altPath,
    uint16_t numObject,
    lwm2m_object_t* objectList[]
)
{
    return -1;
}

int lwm2m_data_decode_int
(
    const lwm2m_data_t* dataP,
    int64_t* valueP
)
{
    return -1;
}

void lwm2m_handle_packet
(
    lwm2m_context_t* contextP,
    uint8_t* buffer,
    int length,
    void* fromSessionH
)
{
    return;
}

int lwm2m_step
(
    lwm2m_context_t* contextP,
    time_t* timeoutP
)
{
    return -1;
}

int lwm2m_update_registration
(
    lwm2m_context_t* contextP,
    uint16_t shortServerID,
    bool withObjects
)
{
    return -1;
}

void lwm2m_close
(
    lwm2m_context_t* contextP
)
{
    return;
}

void lwm2m_set_push_callback
(
    lwm2mcore_PushAckCallback_t callbackP
)
{
    return;
}

int lwm2m_data_push
(
    lwm2m_context_t* contextP,
    uint16_t shortServerID,
    uint8_t* payloadP,
    size_t payload_len,
    lwm2m_media_type_t contentType,
    uint16_t* midP
)
{
    return -1;
}

bool lwm2m_async_response
(
    lwm2m_context_t* contextP,
    uint16_t shortServerId,
    uint16_t mid,
    uint32_t code,
    uint8_t* token,
    uint8_t token_len,
    uint16_t content_type,
    uint8_t* payload,
    size_t payload_len
)
{
    return false;
}

void lwm2m_data_free
(
    int size,
    lwm2m_data_t* dataP
)
{
    return;
}

lwm2m_context_t* lwm2m_init
(
    void* userData
)
{
    return NULL;
}
