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
#define COAP_PATH_MAX_LENGTH 256

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
    lwm2mcore_CoapResponseCode_t code;              ///< [IN] Response code
    uint16_t messageId;                             ///< [IN] Message id
    uint8_t token[8];                               ///< [IN] Token
    uint8_t tokenLength;                            ///< [IN] Token length
    unsigned int contentType;                       ///< [IN] Payload content type
    uint8_t* payloadPtr;                            ///< [IN] Payload pointer
    size_t payloadLength;                           ///< [IN] Payload length
    lwm2mcore_StreamStatus_t  streamStatus;         ///< [IN] Status of the transmit stream
    uint16_t blockSize;                             ///< [IN] Block size
}
lwm2mcore_CoapResponse_t;

//--------------------------------------------------------------------------------------------------
/**
 * @brief CoAP unsolicited message from device (push)
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    uint8_t* uriPtr;                                ///< [IN] URI pointer
    uint8_t* tokenPtr;                              ///< [IN] Token pointer
    uint8_t tokenLength;                            ///< [IN] Token length
    unsigned int contentType;                       ///< [IN] Payload content type
    uint8_t* payloadPtr;                            ///< [IN] Payload pointer
    size_t payloadLength;                           ///< [IN] Payload length
    lwm2mcore_StreamStatus_t  streamStatus;         ///< [IN] Status of the transmit stream
    void* callbackRef;                              ///< [IN] Callback for ack received / timeout
    void* callbackContextPtr;                       ///< [IN] Context ptr for ack callback
}
lwm2mcore_CoapNotification_t;

//--------------------------------------------------------------------------------------------------
/**
 * @brief CoAP block transfer status
 */
//--------------------------------------------------------------------------------------------------
typedef enum
{
    STREAM_IDLE = 0,
    STREAM_START,
    STREAM_IN_PROGRESS,
    STREAM_COMPLETED,
    STREAM_ERROR
}
lwm2mcore_stream_status_t;

//--------------------------------------------------------------------------------------------------
/**
 * @brief CoAP request reference.
 */
//--------------------------------------------------------------------------------------------------
typedef struct
{
    char *uri;                              ///< [IN] URI represents the path of the coap response
    coap_method_t method;                   ///< [IN] is the operation GET/PUT/POST
    uint16_t messageId;                     ///< [IN] coap message Id
    uint8_t token[8];                       ///< [IN] token
    uint8_t tokenLength;                    ///< [IN] token length
    unsigned int contentType;               ///< [IN] payload content type
    uint8_t *buffer;                        ///< [IN] payload of coap request
    size_t bufferLength;                    ///< [IN] length of input buffer
    lwm2mcore_StreamStatus_t streamStatus;  ///< [IN] stream status
    uint16_t blockSize;                     ///< [IN] Block size
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
 * @brief Function pointer of CoAP external handler.
 */
//--------------------------------------------------------------------------------------------------
typedef void (*coap_external_handler_t)
(
    lwm2mcore_CoapRequest_t* requestRef
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function pointer of CoAP ack handler.
 */
//--------------------------------------------------------------------------------------------------
typedef void (*coap_ack_handler_t)
(
    lwm2mcore_AckResult_t ackResult
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to register a handler for CoAP requests
 *
 * @remark Public function which can be called by the client.
 */
//--------------------------------------------------------------------------------------------------
void lwm2mcore_SetCoapEventHandler
(
    coap_request_handler_t handlerRef        ///< [IN] Coap action handler
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to register a handler on all CoAP messages other than lwm2m oriented messages
 *
 * @remark Public function which can be called by the client.
 */
//--------------------------------------------------------------------------------------------------
void lwm2mcore_SetCoapExternalHandler
(
    coap_external_handler_t handlerRef        ///< [IN] Handler for receiving incoming CoAP messages
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to register a handler on push ack response
 *
 * @remark Public function which can be called by the client.
 */
//--------------------------------------------------------------------------------------------------
void lwm2mcore_SetCoapAckHandler
(
    coap_ack_handler_t handlerRef        ///< [IN] Handler for receiving CoAP Ack messages
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to get CoAP message id
 *
 * @remark Public function which can be called by the client.
 *
 * @return
 *  - CoAP message ID
 */
//--------------------------------------------------------------------------------------------------
uint16_t lwm2mcore_GetMessageId
(
    lwm2mcore_CoapRequest_t* requestRef    ///< [IN] Coap request reference
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to get CoAP stream status.
 *
 * @remark Public function which can be called by the client.
 *
 * @return
 *  - CoAP stream status
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_StreamStatus_t lwm2mcore_GetStreamStatus
(
    lwm2mcore_CoapRequest_t* requestRef    ///< [IN] Coap request reference
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to get URI from request
 *
 * @remark Public function which can be called by the client.
 *
 * @return
 *  - URI from request
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
 * @remark Public function which can be called by the client.
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
    lwm2mcore_CoapRequest_t* requestRef        ///< [IN] Coap request reference
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to get payload from request
 *
 * @remark Public function which can be called by the client.
 *
 * @return
 *  - payload from request
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
 * @remark Public function which can be called by the client.
 *
 * @return
 *  - payload length from request
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
 * @remark Public function which can be called by the client.
 *
 * @return
 *  - token from request
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
 * @remark Public function which can be called by the client.
 *
 * @return
 *  - token length from request
 */
//--------------------------------------------------------------------------------------------------
uint8_t lwm2mcore_GetTokenLength
(
    lwm2mcore_CoapRequest_t* requestRef    ///< [IN] Coap request reference
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to get content type from request
 *
 * @remark Public function which can be called by the client.
 *
 * @return
 *  - @c true if an async response is initiated
 *  - else @c false
 */
//--------------------------------------------------------------------------------------------------
unsigned int lwm2mcore_GetContentType
(
    lwm2mcore_CoapRequest_t* requestRef    ///< [IN] Coap request reference
);

//--------------------------------------------------------------------------------------------------
/**
 * Function to get block1 size
 *
 * @return
 *  - block size
 */
//--------------------------------------------------------------------------------------------------
uint16_t lwm2mcore_GetRequestBlock1Size
(
    lwm2mcore_CoapRequest_t* requestRef    ///< [IN] CoAP request reference
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to send an asynchronous response to server.
 *
 * @remark Public function which can be called by the client.
 *
 * @return
 *  - @c true if an asynchronous response is initiated
 *  - else @c false
 */
//--------------------------------------------------------------------------------------------------
bool lwm2mcore_SendAsyncResponse
(
    lwm2mcore_Ref_t instanceRef,            ///< [IN] instance reference
    lwm2mcore_CoapRequest_t* requestPtr,    ///< [IN] coap request pointer
    lwm2mcore_CoapResponse_t* responsePtr   ///< [IN] coap response pointer
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to send a response to server.
 *
 * @remark Public function which can be called by the client.
 *
 * @return
 *  - @c true if response is initiated
 *  - else @c false
 */
//--------------------------------------------------------------------------------------------------
bool lwm2mcore_SendResponse
(
    lwm2mcore_Ref_t instanceRef,                ///< [IN] instance reference
    lwm2mcore_CoapResponse_t* responsePtr       ///< [IN] CoAP response pointer
);

//--------------------------------------------------------------------------------------------------
/**
 * Function to send an empty response to server.
 *
 * @return
 *      - true if response is initiated
 *      - else false
 */
//--------------------------------------------------------------------------------------------------
bool lwm2mcore_SendEmptyResponse
(
    uint16_t mid                                 ///< [IN] CoAP message ID
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Function to send an unsolicited message to server.
 *
 * @remark Public function which can be called by the client.
 *
 * @return
 *  - @c true if response is initiated
 *  - else @c false
 */
//--------------------------------------------------------------------------------------------------
bool lwm2mcore_SendNotification
(
    lwm2mcore_CoapNotification_t* notificationPtr        ///< [IN] unsolictied message from device
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Calls the external CoAP push handler function to indicate status of the push operation.
 * If push is streamed the callback is returned only when the stream ends.
  */
//--------------------------------------------------------------------------------------------------
void lwm2mcore_AckCallback
(
    lwm2mcore_AckResult_t result                        ///< [IN] CoAP ack result
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Returns the registered CoAP external event handler
 *
 * @return
 *     - CoAP handler if registered by external app
 *     - NULL if no handler is registered
 */
//--------------------------------------------------------------------------------------------------
coap_external_handler_t lwm2mcore_GetCoapExternalHandler
(
    void
);
/**
  * @}
  */

#endif /* LWM2MCORE_COAP_HANDLERS_H */
