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
 * Only one external event handler is allowed to be registered at a time.
 */
//--------------------------------------------------------------------------------------------------
static coap_external_handler_t ExternalHandlerRef = NULL;


//--------------------------------------------------------------------------------------------------
/*
 * Only one external acknowledge handler is allowed to be registered at a time.
 */
//--------------------------------------------------------------------------------------------------
static coap_ack_handler_t AckHandlerRef = NULL;

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
            switch (method)
            {
                case COAP_GET:
                    result = COAP_205_CONTENT;
                    break;

                case COAP_PUT:
                case COAP_POST:
                    result = COAP_204_CHANGED;
                    break;

                default:
                    result = COAP_400_BAD_REQUEST;
                    break;
            }
            break;

        case LWM2MCORE_ERR_INVALID_STATE:
            result = COAP_503_SERVICE_UNAVAILABLE;
            break;

        case LWM2MCORE_ERR_INVALID_ARG:
            result = COAP_400_BAD_REQUEST;
            break;

        case LWM2MCORE_ERR_OP_NOT_SUPPORTED:
            result = COAP_404_NOT_FOUND;
            break;

        case LWM2MCORE_ERR_NOT_YET_IMPLEMENTED:
            result = COAP_501_NOT_IMPLEMENTED;
            break;

        case LWM2MCORE_ERR_ASYNC_OPERATION:
            result = MANUAL_RESPONSE;
            break;

        case LWM2MCORE_ERR_INCORRECT_RANGE:
        case LWM2MCORE_ERR_GENERAL_ERROR:
        case LWM2MCORE_ERR_OVERFLOW:
        default:
            result = COAP_500_INTERNAL_SERVER_ERROR;
            break;
    }

    LOG_ARG ("sID %d -> CoAP result %d", sid, result);

    return result;
}

#ifdef DELIMITER
//--------------------------------------------------------------------------------------------------
/**
 * Replace default CoAP URI delimiter character "/" in the string source by the character delimiter
 *
 * @return
 *      - NULL if original string is NULL
 *      - original string if the delimiter is outside of the range 0x20 - 0x7E
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
 * Set CoAP event handler
 */
//--------------------------------------------------------------------------------------------------
void lwm2mcore_SetCoapEventHandler
(
    coap_request_handler_t handlerRef   ///< [IN] CoAP action handler
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
 * Set CoAP external handler
 */
//--------------------------------------------------------------------------------------------------
void lwm2mcore_SetCoapExternalHandler
(
    coap_request_handler_t handlerRef   ///< [IN] CoAP external event handler
)
{
    // New handler is being added
    ExternalHandlerRef = handlerRef;
}

//--------------------------------------------------------------------------------------------------
/**
 * Set CoAP acknowledge handler
 */
//--------------------------------------------------------------------------------------------------
void lwm2mcore_SetCoapAckHandler
(
    coap_ack_handler_t handlerRef    ///< [IN] CoAP external acknowledge handler
)
{
    // New handler is being added
    AckHandlerRef = handlerRef;
}

//--------------------------------------------------------------------------------------------------
/**
 * Calls the external CoAP push handler function to indicate status of the push operation.
 * If push is streamed the callback is returned only when the stream ends.
  */
//--------------------------------------------------------------------------------------------------
void lwm2mcore_AckCallback
(
    lwm2mcore_AckResult_t result     ///< [IN] CoAP ack result
)
{
    if (AckHandlerRef != NULL)
    {
        AckHandlerRef(result);
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Calls the external CoAP event handler to handle incoming CoAP messages
 *
 * @return
 *      - CoAP error code from user application
 *      - COAP_501_NOT_IMPLEMENTED if there is no registered handler found.
 */
//--------------------------------------------------------------------------------------------------
coap_status_t lwm2mcore_CallCoapExternalHandler
(
    coap_packet_t* messagePtr,                        ///< [IN] CoAP request pointer
    lwm2mcore_StreamStatus_t streamStatus             ///< [IN] Stream status
)
{
    lwm2mcore_CoapRequest_t* requestPtr;

    requestPtr = (lwm2mcore_CoapRequest_t*)lwm2m_malloc(sizeof(lwm2mcore_CoapRequest_t));
    if (!requestPtr)
    {
        LOG("requestPtr is NULL");
        return (coap_status_t)COAP_500_INTERNAL_SERVER_ERROR;
    }

    requestPtr->uri = coap_get_multi_option_as_string(messagePtr->uri_path);
#ifdef DELIMITER
    requestPtr->uri = Replace(requestPtr->uri, DELIMITER);
#endif

    requestPtr->method = (coap_method_t)messagePtr->code;
    requestPtr->buffer = messagePtr->payload;
    requestPtr->bufferLength = messagePtr->payload_len;
    requestPtr->messageId = messagePtr->mid;
    requestPtr->tokenLength = messagePtr->token_len;
    memcpy(requestPtr->token, messagePtr->token, messagePtr->token_len);
    requestPtr->contentType = messagePtr->content_type;
    requestPtr->streamStatus = streamStatus;

    if (ExternalHandlerRef != NULL)
    {
        /* Call external CoAP Handler */
        ExternalHandlerRef(requestPtr);
    }

    if ((NULL == ExternalHandlerRef) && (requestPtr))
    {
       lwm2m_free(requestPtr);
    }

    // TODO: Initiate a timer to delay this ack by 2 seconds (application processing time)
    // If the application responds within 2 seconds, we can send a piggy backed response.

    // Actual response will be sent by external application
    return (coap_status_t)COAP_IGNORE;
}

//--------------------------------------------------------------------------------------------------
/**
 * Returns the registered CoAP external event handler
 *
 * @return
 *     - CoAP handler if registered by external application
 *     - NULL if no handler is registered
 */
//--------------------------------------------------------------------------------------------------
coap_external_handler_t lwm2mcore_GetCoapExternalHandler
(
    void
)
{
    return ExternalHandlerRef;
}

//--------------------------------------------------------------------------------------------------
/**
 * Retrieves the registered CoAP request handler and returns the CoAP request details
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
        return (coap_status_t)COAP_500_INTERNAL_SERVER_ERROR;
    }

    requestPtr->uri = coap_get_multi_option_as_string(message->uri_path);
#ifdef DELIMITER
    requestPtr->uri = Replace(requestPtr->uri, DELIMITER);
#endif

    requestPtr->method = (coap_method_t)message->code;
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

    return (coap_status_t)coapErrorCode;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to get CoAP message identifier
 *
 * @return
 *     - CoAP message identifier
 */
//--------------------------------------------------------------------------------------------------
uint16_t lwm2mcore_GetMessageId
(
    lwm2mcore_CoapRequest_t* requestRef    ///< [IN] CoAP request reference
)
{
    return requestRef->messageId;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to get CoAP stream status
 *
 * @return
 *     - CoAP stream status
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_StreamStatus_t lwm2mcore_GetStreamStatus
(
    lwm2mcore_CoapRequest_t* requestRef    ///< [IN] CoAP request reference
)
{
    return requestRef->streamStatus;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to get URI from request
 *
 * @return
 *  - URI from request
 */
//--------------------------------------------------------------------------------------------------
const char* lwm2mcore_GetRequestUri
(
    lwm2mcore_CoapRequest_t* requestRef    ///< [IN] CoAP request reference
)
{
    return requestRef->uri;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to get method from request
 *
 * @return
 *  - COAP_GET
 *  - COAP_POST
 *  - COAP_PUT
 *  - COAP_DELETE
 */
//--------------------------------------------------------------------------------------------------
coap_method_t lwm2mcore_GetRequestMethod
(
    lwm2mcore_CoapRequest_t* requestRef        ///< [IN] CoAP request reference
)
{
    return requestRef->method;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to get payload from request
 *
 * @return
 *     - CoAP payload pointer from request
 */
//--------------------------------------------------------------------------------------------------
const uint8_t* lwm2mcore_GetRequestPayload
(
    lwm2mcore_CoapRequest_t* requestRef    ///< [IN] CoAP request reference
)
{
    return requestRef->buffer;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to get payload length from request
 *
 * @return
 *     - CoAP payload length from request
 */
//--------------------------------------------------------------------------------------------------
size_t lwm2mcore_GetRequestPayloadLength
(
    lwm2mcore_CoapRequest_t* requestRef    ///< [IN] CoAP request reference
)
{
    return requestRef->bufferLength;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to get token from request
 *
 * @return
 *     - CoAP token pointer from request
 */
//--------------------------------------------------------------------------------------------------
const uint8_t* lwm2mcore_GetToken
(
    lwm2mcore_CoapRequest_t* requestRef    ///< [IN] CoAP request reference
)
{
    return requestRef->token;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to get token length from request
 *
 * @return
 *     - CoAP token length from request
 */
//--------------------------------------------------------------------------------------------------
uint8_t lwm2mcore_GetTokenLength
(
    lwm2mcore_CoapRequest_t* requestRef    ///< [IN] CoAP request reference
)
{
    return requestRef->tokenLength;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to get content type from request
 *
 * @return
 *  - @c true if an asynchronous response is initiated
 *  - else @c false
 */
//--------------------------------------------------------------------------------------------------
unsigned int lwm2mcore_GetContentType
(
    lwm2mcore_CoapRequest_t* requestRef    ///< [IN] CoAP request reference
)
{
    return requestRef->contentType;
}
