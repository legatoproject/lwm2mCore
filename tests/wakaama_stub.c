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
    (void)path;
    return NULL;
}

void lwm2m_data_encode_int
(
    int64_t value,
    lwm2m_data_t* dataP
)
{
    (void)value;
    (void)dataP;
    return;
}

void lwm2m_data_encode_bool
(
    bool value,
    lwm2m_data_t* dataP
)
{
    (void)value;
    (void)dataP;
    return;
}

void lwm2m_data_encode_nstring
(
    const char* string,
    size_t length,
    lwm2m_data_t* dataP
)
{
    (void)string;
    (void)length;
    (void)dataP;
    return;
}

void lwm2m_data_encode_opaque
(
    uint8_t* buffer,
    size_t length,
    lwm2m_data_t* dataP
)
{
    (void)buffer;
    (void)length;
    (void)dataP;
    return;
}

void lwm2m_data_encode_float
(
    double value,
    lwm2m_data_t* dataP
)
{
    (void)value;
    (void)dataP;
    return;
}

lwm2m_data_t* lwm2m_data_new
(
    int size
)
{
    (void)size;
    return NULL;
}

void lwm2m_data_encode_instances
(
    lwm2m_data_t* subDataP,
    size_t count,
    lwm2m_data_t* dataP
)
{
    (void)subDataP;
    (void)count;
    (void)dataP;
    return;
}

lwm2m_list_t* lwm2m_list_find
(
    lwm2m_list_t* head,
    uint16_t id
)
{
    (void)head;
    (void)id;
    return NULL;
}

int utils_textToInt
(
    uint8_t* buffer,
    int length,
    int64_t* dataP
)
{
    (void)buffer;
    (void)length;
    (void)dataP;
    return -1;
}

lwm2m_list_t* lwm2m_list_add
(
    lwm2m_list_t* head,
    lwm2m_list_t* node
)
{
    (void)head;
    (void)node;
    return NULL;
}

lwm2m_list_t* lwm2m_list_remove
(
    lwm2m_list_t* head,
    uint16_t id,
    lwm2m_list_t** nodeP
)
{
    (void)head;
    (void)id;
    (void)nodeP;
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
    (void)contextP;
    (void)endpointName;
    (void)msisdn;
    (void)altPath;
    (void)numObject;
    (void)objectList;
    return -1;
}

int lwm2m_data_decode_int
(
    const lwm2m_data_t* dataP,
    int64_t* valueP
)
{
    (void)dataP;
    (void)valueP;
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
    (void)contextP;
    (void)buffer;
    (void)length;
    (void)fromSessionH;
    return;
}

int lwm2m_step
(
    lwm2m_context_t* contextP,
    time_t* timeoutP
)
{
    (void)contextP;
    (void)timeoutP;
    return -1;
}

int lwm2m_update_registration
(
    lwm2m_context_t* contextP,
    uint16_t shortServerID,
    bool withObjects
)
{
    (void)contextP;
    (void)shortServerID;
    (void)withObjects;
    return -1;
}

void lwm2m_close
(
    lwm2m_context_t* contextP
)
{
    (void)contextP;
    return;
}

void lwm2m_set_push_callback
(
    lwm2mcore_PushAckCallback_t callbackP
)
{
    (void)callbackP;
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
    (void)contextP;
    (void)shortServerID;
    (void)payloadP;
    (void)payload_len;
    (void)contentType;
    (void)midP;
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
    (void)contextP;
    (void)shortServerId;
    (void)mid;
    (void)code;
    (void)token;
    (void)token_len;
    (void)content_type;
    (void)payload;
    (void)payload_len;
    return false;
}

void lwm2m_data_free
(
    int size,
    lwm2m_data_t* dataP
)
{
    (void)size;
    (void)dataP;
    return;
}

lwm2m_context_t* lwm2m_init
(
    void* userData
)
{
    (void)userData;
    return NULL;
}
