/**
 * @file lwm2mcoreCoapHandlers.c
 *
 * CoAP request handlers for user specified objects.
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <stddef.h>
#include <platform/types.h>
#include <lwm2mcore/lwm2mcore.h>
#include <lwm2mcore/coapHandlers.h>
#include "objects.h"
#include "handlers.h"
#include "internals.h"
#include "er-coap-13.h"
#include "internalCoapHandler.h"


//--------------------------------------------------------------------------------------------------
/*
 * Only one event handler is allowed to be registered at a time.
 */
//--------------------------------------------------------------------------------------------------
static coap_request_handler_t RequestHandlerRef = NULL;



//--------------------------------------------------------------------------------------------------
/**
 *                      PRIVATE FUNCTIONS
 */
//--------------------------------------------------------------------------------------------------



//--------------------------------------------------------------------------------------------------
/**
 * Function to translate a resource handler status to a CoAP error
 */
//--------------------------------------------------------------------------------------------------
static uint8_t GetCoapErrorCode
(
    int sid,                            ///< [IN] resource handler status
    coap_method_t method                ///< [IN] action
)
{
    uint8_t result = COAP_503_SERVICE_UNAVAILABLE;
    switch (sid)
    {
        case LWM2MCORE_ERR_COMPLETED_OK:
        {
            if (method == COAP_GET)
            {
                result = COAP_205_CONTENT;
            }
            else if (method == COAP_PUT)
            {
                result = COAP_204_CHANGED;
            }
            else if (method == COAP_POST)
            {
                result = COAP_204_CHANGED;
            }
            else
            {
                result = COAP_400_BAD_REQUEST;
            }
        }
        break;

        case LWM2MCORE_ERR_INVALID_STATE:
        {
            result = COAP_503_SERVICE_UNAVAILABLE;
        }
        break;

        case LWM2MCORE_ERR_INVALID_ARG:
        {
            result = COAP_400_BAD_REQUEST;
        }
        break;

        case LWM2MCORE_ERR_OP_NOT_SUPPORTED:
        {
            result = COAP_404_NOT_FOUND;
        }
        break;

        case LWM2MCORE_ERR_NOT_YET_IMPLEMENTED:
        {
            result = COAP_501_NOT_IMPLEMENTED;
        }
        break;

        case LWM2MCORE_ERR_INCORRECT_RANGE:
        {
            result = COAP_500_INTERNAL_SERVER_ERROR;
        }
        break;

        case LWM2MCORE_ERR_GENERAL_ERROR:
        {
            result = COAP_500_INTERNAL_SERVER_ERROR;
        }
        break;

        case LWM2MCORE_ERR_OVERFLOW:
        {
            result = COAP_500_INTERNAL_SERVER_ERROR;
        }
        break;

        case LWM2MCORE_ERR_ASYNC_OPERATION:
        {
            result = MANUAL_RESPONSE;
        }
        break;

        default:
        {
            result = COAP_500_INTERNAL_SERVER_ERROR;
        }
        break;
    }
    LOG_ARG ("sID %d -> CoAP result %d", sid, result);
    return result;
}

#ifdef DELIMITER
//--------------------------------------------------------------------------------------------------
/**
 * Replace default CoAP URI delimiter character "/" in the string src by the character delim
 *
 * @return
 *      - NULL if original string is NULL
 *      - original string if the delimiter is outside of the range 0x20 - 0x7e
 *      - modified string if it succeeds
 */
//--------------------------------------------------------------------------------------------------
static char* Replace
(
    char*   srcPtr,     ///< [IN] original string
    int     delim       ///< [IN] new delimiter
)
{
    int count = 0;

    if (!srcPtr)
    {
        LOG("Bad address");
        return NULL;
    }

    if (!isprint(delim))
    {
        LOG("Operation not permitted");
        return srcPtr;
    }

    if ('/' == *srcPtr)
    {
        while (*srcPtr)
        {
            *srcPtr = *(srcPtr+1);
            srcPtr++;
            count++;
        }
        srcPtr -= count;
        count = 0;
    }

    while(*srcPtr)
    {
        if ('/' == *srcPtr)
        {
            *srcPtr = delim;
        }
        srcPtr++;
        count++;
    }
    srcPtr -= count;

    return srcPtr;
}
#endif

//--------------------------------------------------------------------------------------------------
/**
 * Set coap event handler
 */
//--------------------------------------------------------------------------------------------------
void lwm2mcore_SetCoapEventHandler
(
    coap_request_handler_t handlerRef    ///< [IN] Coap action handler
)
{
    if (handlerRef == NULL)
    {
        RequestHandlerRef = NULL;
    }
    else
    {
        // New handler is being added
        RequestHandlerRef = handlerRef;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Retrieves the registered coap request handler and returns the coap request details
 *
 *  * @return
 *      - CoAP error code from user application
 *      - COAP_501_NOT_IMPLEMENTED if there is no registered handler found.
 */
//--------------------------------------------------------------------------------------------------
coap_status_t lwm2mcore_CallCoapEventHandler
(
    coap_packet_t* message        ///< [IN] CoAP request
)
{
    uint8_t coapErrorCode;
    lwm2mcore_Sid_t result = LWM2MCORE_ERR_NOT_YET_IMPLEMENTED;
    lwm2mcore_CoapRequest_t* requestPtr;

    requestPtr = (lwm2mcore_CoapRequest_t*)lwm2m_malloc(sizeof(lwm2mcore_CoapRequest_t));
    if (!requestPtr)
    {
        LOG("requestPtr is NULL");
        return COAP_500_INTERNAL_SERVER_ERROR;
    }

    requestPtr->uri = coap_get_multi_option_as_string(message->uri_path);
#ifdef DELIMITER
    requestPtr->uri = Replace(requestPtr->uri, DELIMITER);
#endif
    if (requestPtr->uri)
    {
        requestPtr->uriLength = strlen(requestPtr->uri);
    }
    else
    {
        requestPtr->uriLength = 0;
    }
    requestPtr->method = message->code;
    requestPtr->buffer = message->payload;
    requestPtr->bufferLength = message->payload_len;
    requestPtr->messageId = message->mid;
    requestPtr->tokenLength = message->token_len;
    memcpy(requestPtr->token, message->token, message->token_len);
    requestPtr->contentType = message->content_type;

    if (RequestHandlerRef != NULL)
    {
        RequestHandlerRef(requestPtr);
        result = LWM2MCORE_ERR_ASYNC_OPERATION;
    }

    coapErrorCode = GetCoapErrorCode(result, requestPtr->method);
    if ((NULL == RequestHandlerRef) && (requestPtr))
    {
       lwm2m_free(requestPtr);
    }

    return coapErrorCode;
}


//--------------------------------------------------------------------------------------------------
/**
 * Function to get URI from request
 */
//--------------------------------------------------------------------------------------------------
const char* lwm2mcore_GetRequestUri
(
    lwm2mcore_CoapRequest_t* requestRef    ///< [IN] Coap request reference
)
{
    return requestRef->uri;
}


//--------------------------------------------------------------------------------------------------
/**
 * Function to get method from request
 */
//--------------------------------------------------------------------------------------------------
coap_method_t lwm2mcore_GetRequestMethod
(
    lwm2mcore_CoapRequest_t* requestRef        ///< [IN] Coap request reference
)
{
    return requestRef->method;
}


//--------------------------------------------------------------------------------------------------
/**
 * Function to get payload from request
 */
//--------------------------------------------------------------------------------------------------
const uint8_t* lwm2mcore_GetRequestPayload
(
    lwm2mcore_CoapRequest_t* requestRef    ///< [IN] Coap request reference
)
{
    return requestRef->buffer;
}


//--------------------------------------------------------------------------------------------------
/**
 * Function to get payload length from request
 */
//--------------------------------------------------------------------------------------------------
size_t lwm2mcore_GetRequestPayloadLength
(
    lwm2mcore_CoapRequest_t* requestRef    ///< [IN] Coap request reference
)
{
    return requestRef->bufferLength;
}



//--------------------------------------------------------------------------------------------------
/**
 * Function to get token from request
 */
//--------------------------------------------------------------------------------------------------
const uint8_t* lwm2mcore_GetToken
(
    lwm2mcore_CoapRequest_t* requestRef    ///< [IN] Coap request reference
)
{
    return requestRef->token;
}


//--------------------------------------------------------------------------------------------------
/**
 * Function to get token length from request
 */
//--------------------------------------------------------------------------------------------------
size_t lwm2mcore_GetTokenLength
(
    lwm2mcore_CoapRequest_t* requestRef    ///< [IN] Coap request reference
)
{
    return requestRef->tokenLength;
}


//--------------------------------------------------------------------------------------------------
/**
 * Function to get content type from request
 */
//--------------------------------------------------------------------------------------------------
const unsigned int lwm2mcore_GetContentType
(
    lwm2mcore_CoapRequest_t* requestRef    ///< [IN] Coap request reference
)
{
    return requestRef->contentType;
}
