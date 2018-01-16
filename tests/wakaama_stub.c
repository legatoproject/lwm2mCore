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
#include <sessionManager/sessionManager.h>
#include "liblwm2m.h"
#include <stdarg.h>


//-------------------------------------------------------------------------------------------------
/**
 * maximum length of the buffer
 */
//-------------------------------------------------------------------------------------------------
#define MAX_BUFFER_LEN 100


char* coap_get_multi_option_as_string
(
    multi_option_t* option
)
{
    (void)option;
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
    lwm2m_data_t* dataP;
    int length;
    char buffer[MAX_BUFFER_LEN] = "coaps://sierra:2467";

    if (size <= 0)
    {
        return NULL;
    }

    dataP = (lwm2m_data_t*)lwm2m_malloc(size * sizeof(lwm2m_data_t));

    if (dataP == NULL)
    {
        return NULL;
    }

    memset(dataP, 0, size * sizeof(lwm2m_data_t));
    dataP->type = LWM2M_TYPE_STRING;
    length = strlen(buffer);
    dataP->value.asBuffer.buffer = (uint8_t*)lwm2m_malloc(length + 1);
    strncpy((char*)dataP->value.asBuffer.buffer, buffer, length + 1);
    dataP->value.asBuffer.length = length + 1;
    return dataP;
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
    while (NULL != head && head->id < id)
    {
        head = head->next;
    }

    if (NULL != head && head->id == id)
    {
        return head;
    }

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
    lwm2m_list_t * target;

    if (NULL == head)
    {
       return node;
    }

    if (head->id > node->id)
    {
        node->next = head;
        return node;
    }

    target = head;
    while (NULL != target->next && target->next->id < node->id)
    {
        target = target->next;
    }

    node->next = target->next;
    target->next = node;

    return head;
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
    int i;
    (void)endpointName;
    (void)msisdn;
    (void)altPath;

    for (i = 0; i < numObject; i++)
    {
        objectList[i]->next = NULL;
        contextP->objectList = (lwm2m_object_t *)LWM2M_LIST_ADD(contextP->objectList, objectList[i]);
    }
    return COAP_NO_ERROR;
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
    return 0;
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

    return COAP_NO_ERROR;
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

    return COAP_NO_ERROR;
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
    return true;
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

lwm2m_context_t * lwm2m_init(void * userData)
{
    lwm2m_context_t * contextP;

    contextP = (lwm2m_context_t *)lwm2m_malloc(sizeof(lwm2m_context_t));
    if (NULL != contextP)
    {
        memset(contextP, 0, sizeof(lwm2m_context_t));
        contextP->userData = userData;
        srand((int)lwm2m_gettime());
        contextP->nextMID = rand();
    }

    return contextP;
}

/**
 * Delete an object instance of object 2
 *
 * @return
 *  - true on success
 *  - false on failure.
 */
bool lwm2m_acl_deleteObjectInstance(lwm2m_object_t * objectP, uint16_t oiid)
{
    (void)objectP;
    (void) oiid;
    return false;
}
