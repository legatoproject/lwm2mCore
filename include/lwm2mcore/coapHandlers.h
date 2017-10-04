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

/**
  * @addtogroup lwm2mcore_coap_handler_IFS
  * @{
  */


//--------------------------------------------------------------------------------------------------
/**
 * @brief Maximum length of the COAP path (URI).
 */
//--------------------------------------------------------------------------------------------------
#define COAP_PATH_MAX_LENGTH (256)


//--------------------------------------------------------------------------------------------------
/**
 * @brief CoAP response code.
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    COAP_RESOURCE_CHANGED = 0,      ///< Resource value changed
    COAP_CONTENT_AVAILABLE,         ///< Content available for read response
    COAP_BAD_REQUEST,               ///< Bad request
    COAP_METHOD_UNAUTHORIZED,       ///< Operation not allowed on this resource
    COAP_NOT_FOUND,                 ///< Not found
    COAP_METHOD_NOT_ALLOWED,        ///< Method not allowed
    COAP_PRECONDITION_FAILED,       ///< Precondition Failed
    COAP_REQUEST_ENTITY_TOO_LARGE,  ///< Request Entity Too Large
    COAP_UNSUPPORTED_MEDIA_TYPE,    ///< Unsupported Content-Format
    COAP_INTERNAL_ERROR             ///< Internal error
}
lwm2mcore_CoapResponseCode_t;


//--------------------------------------------------------------------------------------------------
/**
 * @brief CoAP Response
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    lwm2mcore_CoapResponseCode_t code;  ///< [IN] response code
    uint8_t token[8];                   ///< [IN] token
    uint8_t tokenLength;                ///< [IN] token length
    unsigned int contentType;           ///< [IN] payload content type
    uint8_t* payload;                   ///< [IN] payload
    size_t payloadLength;               ///< [IN] payload length
}
lwm2mcore_CoapResponse_t;


//--------------------------------------------------------------------------------------------------
/**
 * @brief CoAP request reference.
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
 * @brief Reference to CoAP Request
 */
//--------------------------------------------------------------------------------------------------
typedef struct lwm2mcore_CoapRequest_t* lwm2mcore_CoapRequestRef_t;


//--------------------------------------------------------------------------------------------------
/**
 * @brief Function pointer of coap resource READ/WRITE/EXECUTE requests.
 */
//--------------------------------------------------------------------------------------------------
typedef void (*coap_request_handler_t)
(
    lwm2mcore_CoapRequest_t* requestRef
);


//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to register a handler for CoAP requests
 */
//--------------------------------------------------------------------------------------------------
void lwm2mcore_SetCoapEventHandler
(
    coap_request_handler_t handlerRef        ///< [IN] Coap action handler
);


//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to get URI from request
 *
 * @return
 *      - URI from request
 */
//--------------------------------------------------------------------------------------------------
const char* lwm2mcore_GetRequestUri
(
    lwm2mcore_CoapRequest_t* requestRef    ///< [IN] Coap request reference
);


//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to get method from request
 *
 * @return
 *      - COAP_GET
 *      - COAP_POST
 *      - COAP_PUT
*       - COAP_DELETE
 */
//--------------------------------------------------------------------------------------------------
coap_method_t lwm2mcore_GetRequestMethod
(
    lwm2mcore_CoapRequest_t* requestRef        ///< [IN] Coap request reference
);


//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to get payload from request
 *
 * @return
 *      - payload from request
 */
//--------------------------------------------------------------------------------------------------
const uint8_t* lwm2mcore_GetRequestPayload
(
    lwm2mcore_CoapRequest_t* requestRef    ///< [IN] Coap request reference
);


//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to get payload length from request
 *
 * @return
 *      - payload length from request
 */
//--------------------------------------------------------------------------------------------------
size_t lwm2mcore_GetRequestPayloadLength
(
    lwm2mcore_CoapRequest_t* requestRef    ///< [IN] Coap request reference
);


//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to get token from request
 *
 * @return
 *      - token from request
 */
//--------------------------------------------------------------------------------------------------
const uint8_t* lwm2mcore_GetToken
(
    lwm2mcore_CoapRequest_t* requestRef    ///< [IN] Coap request reference
);


//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to get token length from request
 *
 * @return
 *      - token length from request
 */
//--------------------------------------------------------------------------------------------------
size_t lwm2mcore_GetTokenLength
(
    lwm2mcore_CoapRequest_t* requestRef    ///< [IN] Coap request reference
);


//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to get content type from request
 *
 * @return
 *      - @c true if an async response is initiated
 *      - else @c false
 */
//--------------------------------------------------------------------------------------------------
const unsigned int lwm2mcore_GetContentType
(
    lwm2mcore_CoapRequest_t* requestRef    ///< [IN] Coap request reference
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to send an asynchronous response to server.
 *
 * @return
 *      - @c true if an asynchronous response is initiated
 *      - else @c false
 */
//--------------------------------------------------------------------------------------------------
bool lwm2mcore_SendAsyncResponse
(
    lwm2mcore_Ref_t instanceRef,            ///< [IN] instance reference
    lwm2mcore_CoapRequest_t* requestPtr,    ///< [IN] coap request reference
    lwm2mcore_CoapResponse_t* responsePtr   ///< [IN] coap response
);

/**
  * @}
  */

#endif /* LWM2MCORE_COAP_HANDLERS_H */
