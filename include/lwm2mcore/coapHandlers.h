/**
 * @file coapHandlers.h
 *
 * Coap request handlers
 *
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#ifndef LWM2MCORE_COAP_HANDLERS_H
#define LWM2MCORE_COAP_HANDLERS_H

#include "liblwm2m.h"
#include "er-coap-13.h"


//--------------------------------------------------------------------------------------------------
/**
 * Maximum length of the COAP path (URI).
 */
//--------------------------------------------------------------------------------------------------
#define COAP_PATH_MAX_LENGTH (256)


//--------------------------------------------------------------------------------------------------
/**
 * Coap response code.
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    COAP_RESOURCE_CHANGED = 0,      ///< Resource value changed
    COAP_CONTENT_AVAILABLE,         ///< Content available for read response
    COAP_BAD_REQUEST,               ///< Bad request
    COAP_METHOD_UNAUTHORIZED,       ///< Operation not allowed on this resource
    COAP_RESOURCE_NOT_FOUND,        ///< Resource not found
    COAP_METHOD_NOT_ALLOWED,        ///< Method not allowed
    COAP_INTERNAL_ERROR             ///< Internal error
}
CoapResponseCode_t;


//--------------------------------------------------------------------------------------------------
/**
 * Coap Response
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    CoapResponseCode_t code;            ///< [IN] response code
    uint8_t token[8];                   ///< [IN] token
    uint8_t tokenLength;                ///< [IN] token length
    unsigned int contentType;           ///< [IN] payload content type
    uint8_t* payload;                   ///< [IN] payload
    size_t payloadLength;               ///< [IN] payload length
}
lwm2mcore_CoapResponse_t;


//--------------------------------------------------------------------------------------------------
/**
 * Coap request reference.
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    char *uri;                      ///< [IN] uri represents the path of the coap response
    size_t uriLength;               ///< [IN] length of uri
    coap_method_t method;           ///< [IN] is the operation GET/PUT/POST
    uint16_t messageId;             ///< [IN] coap message Id
    uint8_t token[8];               ///< [IN] token
    uint8_t tokenLength;            ///< [IN] token length
    unsigned int contentType;       ///< [IN] payload content type
    uint8_t *buffer;                ///< [IN] payload of coap request
    size_t bufferLength;            ///< [IN] length of input buffer
}
lwm2mcore_CoapRequest_t;


//--------------------------------------------------------------------------------------------------
/**
 * Reference to CoAP Request
 */
//--------------------------------------------------------------------------------------------------
typedef struct lwm2mcore_CoapRequest_t* lwm2mcore_CoapRequestRef_t;


//--------------------------------------------------------------------------------------------------
/**
 * Function pointer of coap resource READ/WRITE/EXECUTE requests.
 */
//--------------------------------------------------------------------------------------------------
typedef void (*coap_request_handler_t)
(
    lwm2mcore_CoapRequest_t* requestRef
);


//--------------------------------------------------------------------------------------------------
/**
 * Function to register a handler for CoAP requests
 */
//--------------------------------------------------------------------------------------------------
void lwm2mcore_SetCoapEventHandler
(
    coap_request_handler_t handlerRef        ///< [IN] Coap action handler
);


//--------------------------------------------------------------------------------------------------
/**
 * Function to get URI from request
 */
//--------------------------------------------------------------------------------------------------
const char* lwm2mcore_GetRequestUri
(
    lwm2mcore_CoapRequest_t* requestRef    ///< [IN] Coap request reference
);


//--------------------------------------------------------------------------------------------------
/**
 * Function to get method from request
 */
//--------------------------------------------------------------------------------------------------
coap_method_t lwm2mcore_GetRequestMethod
(
    lwm2mcore_CoapRequest_t* requestRef        ///< [IN] Coap request reference
);


//--------------------------------------------------------------------------------------------------
/**
 * Function to get payload from request
 */
//--------------------------------------------------------------------------------------------------
const uint8_t* lwm2mcore_GetRequestPayload
(
    lwm2mcore_CoapRequest_t* requestRef    ///< [IN] Coap request reference
);


//--------------------------------------------------------------------------------------------------
/**
 * Function to get payload length from request
 */
//--------------------------------------------------------------------------------------------------
size_t lwm2mcore_GetRequestPayloadLength
(
    lwm2mcore_CoapRequest_t* requestRef    ///< [IN] Coap request reference
);


//--------------------------------------------------------------------------------------------------
/**
 * Function to get token from request
 */
//--------------------------------------------------------------------------------------------------
const uint8_t* lwm2mcore_getToken
(
    lwm2mcore_CoapRequest_t* requestRef    ///< [IN] Coap request reference
);


//--------------------------------------------------------------------------------------------------
/**
 * Function to get token length from request
 */
//--------------------------------------------------------------------------------------------------
size_t lwm2mcore_getTokenLength
(
    lwm2mcore_CoapRequest_t* requestRef    ///< [IN] Coap request reference
);


//--------------------------------------------------------------------------------------------------
/**
 * Function to get content type from request
 */
//--------------------------------------------------------------------------------------------------
const unsigned int lwm2mcore_getContentType
(
    lwm2mcore_CoapRequest_t* requestRef    ///< [IN] Coap request reference
);

//--------------------------------------------------------------------------------------------------
/**
 * Function to send an async response to server.
 *
 * @return
 *      - true if an async response is initiated
 *      - else false
 */
//--------------------------------------------------------------------------------------------------
bool lwm2mcore_sendAsyncResponse
(
    int context,                            ///< [IN] context
    lwm2mcore_CoapRequest_t* requestPtr,    ///< [IN] coap request reference
    lwm2mcore_CoapResponse_t* responsePtr   ///< [IN] coap response
);


#endif /* LWM2MCORE_COAP_HANDLERS_H */
