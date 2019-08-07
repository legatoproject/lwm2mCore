/**
 * @file objects.c
 *
 * Adaptation layer from the object table managed by the client and the Wakaama object management
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

/* include files */
#include <lwm2mcore/lwm2mcore.h>
#include <lwm2mcore/update.h>
#include <lwm2mcore/security.h>
#include <lwm2mcore/paramStorage.h>
#include "liblwm2m.h"
#include "objects.h"
#include "sessionManager.h"
#include "internals.h"
#include <stdlib.h>
#include "utils.h"
#include "handlers.h"
#include "aclConfiguration.h"
#include "bootstrapConfiguration.h"
#ifdef LWM2M_OBJECT_33406
#include <lwm2mcore/fileTransfer.h>
#endif

//--------------------------------------------------------------------------------------------------
/**
 * Maximum number of objects which can be registered in Wakaama
 */
//--------------------------------------------------------------------------------------------------
#ifdef LWM2M_OBJECT_33406
#define OBJ_COUNT 17
#else
#define OBJ_COUNT 15
#endif

//--------------------------------------------------------------------------------------------------
/**
 * Define for supported object instance list
 */
//--------------------------------------------------------------------------------------------------
#define ONE_PATH_MAX_LEN 90

//--------------------------------------------------------------------------------------------------
/**
 * Padding character in Base 64
 */
//--------------------------------------------------------------------------------------------------

#define B64_PADDING '='

//--------------------------------------------------------------------------------------------------
/**
 * Objects number which are registered in Wakaama
 */
//--------------------------------------------------------------------------------------------------
uint16_t RegisteredObjNb = 0;

//--------------------------------------------------------------------------------------------------
/**
 * Object array to be registered in Wakamaa including the generic handlers to access to these
 * objects
 */
//--------------------------------------------------------------------------------------------------
static lwm2m_object_t* ObjectArray[OBJ_COUNT];

//--------------------------------------------------------------------------------------------------
/**
 * LWM2M core context
 */
//--------------------------------------------------------------------------------------------------
extern lwm2mcore_context_t* Lwm2mcoreCtxPtr;

//--------------------------------------------------------------------------------------------------
/**
 * Static string for software object instance list
 */
//--------------------------------------------------------------------------------------------------
char SwObjectInstanceListPtr[LWM2MCORE_SW_OBJECT_INSTANCE_LIST_MAX_LEN + 1];

//--------------------------------------------------------------------------------------------------
/**
 * Static string for file transfer object instance list
 */
//--------------------------------------------------------------------------------------------------
#ifdef LWM2M_OBJECT_33406
static char FileTransferObjectInstanceListPtr[LWM2MCORE_FILE_TRANSFER_OBJECT_INSTANCE_LIST_MAX_LEN
                                              + 1];
#endif

//--------------------------------------------------------------------------------------------------
/**
 * Structure for supported application list (object 9)
 */
//--------------------------------------------------------------------------------------------------
typedef struct _ObjectInstanceList_ ObjectInstanceList_t;

//--------------------------------------------------------------------------------------------------
/**
 * Structure for supported application list (object 9)
 */
//--------------------------------------------------------------------------------------------------
struct _ObjectInstanceList_
{
    ObjectInstanceList_t* nextPtr;  ///< matches lwm2m_list_t::next
    uint16_t              oiid;     ///< object instance Id, matches lwm2m_list_t::id
    bool                  check;    ///< boolean for list update
};

//--------------------------------------------------------------------------------------------------
/**
 * Object 9 instance list
 */
//--------------------------------------------------------------------------------------------------
static ObjectInstanceList_t* SwApplicationListPtr;

//--------------------------------------------------------------------------------------------------
/**
 * Object 33407 instance list
 */
//--------------------------------------------------------------------------------------------------
#ifdef LWM2M_OBJECT_33406
static ObjectInstanceList_t* FileTransferListPtr;
#endif

//--------------------------------------------------------------------------------------------------
/**
 *                      PRIVATE FUNCTIONS
 */
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Function to translate a resource handler status to a CoAP error
 *
 * @return
 *      - CoAP error value
 */
//--------------------------------------------------------------------------------------------------
static uint8_t SetCoapError
(
    int sid,                            ///< [IN] resource handler status
    lwm2mcore_OpType_t operation        ///< [IN] operation
)
{
    uint8_t result = COAP_503_SERVICE_UNAVAILABLE;

    switch (sid)
    {
        case LWM2MCORE_ERR_COMPLETED_OK:
        case LWM2MCORE_ERR_ALREADY_PROCESSED:
            switch (operation)
            {
                case LWM2MCORE_OP_READ:
                    result = COAP_205_CONTENT;
                    break;

                case LWM2MCORE_OP_WRITE:
                case LWM2MCORE_OP_EXECUTE:
                    result = COAP_204_CHANGED;
                    break;

                default:
                    result = COAP_400_BAD_REQUEST;
                    break;
            }
        break;

        // LWM2MCORE_ERR_INVALID_STATE needs to be mapped to COAP_404_NOT_FOUND and not
        // COAP_503_SERVICE_UNAVAILABLE in order to have the required behavior:
        // - Data is ignored on a READ command on an object
        // - CoAP 4.04 is returned on command on an atomic resource
        case LWM2MCORE_ERR_INVALID_STATE:
            result = COAP_404_NOT_FOUND;
            break;

        case LWM2MCORE_ERR_INVALID_ARG:
            result = COAP_400_BAD_REQUEST;
            break;

        case LWM2MCORE_ERR_OP_NOT_SUPPORTED:
        case LWM2MCORE_ERR_NOT_YET_IMPLEMENTED:
            result = COAP_404_NOT_FOUND;
            break;

        case LWM2MCORE_ERR_INCORRECT_RANGE:
        case LWM2MCORE_ERR_GENERAL_ERROR:
        case LWM2MCORE_ERR_OVERFLOW:
        default:
            result = COAP_500_INTERNAL_SERVER_ERROR;
            break;
    }

    REPORT_COAP(result);
    LOG_ARG("sID %d operation %d -> CoAP result %d", sid, operation, result);

    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function which returns a registered object
 *
 * @return
 *      - object pointer  if the object is found
 *      - NULL  if the object is not found
 */
//--------------------------------------------------------------------------------------------------
static lwm2mcore_internalObject_t* FindObject
(
    lwm2mcore_context_t* ctxPtr,            ///< [IN] LWM2M core context
    uint16_t oid                            ///< [IN] object ID to find
)
{
    lwm2mcore_internalObject_t* objPtr = NULL;

    if (NULL == ctxPtr)
    {
        return NULL;
    }

    for (objPtr = DLIST_FIRST(&(ctxPtr->objects_list)); objPtr; objPtr = DLIST_NEXT(objPtr, list))
    {
        if (objPtr->id == oid)
        {
            break;
        }
    }

    return objPtr;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function which returns a registered resource for a specific object
 *
 * @return
 *      - resource pointer  if the resource is found
 *      - NULL  if the object is not found
 */
//--------------------------------------------------------------------------------------------------
static lwm2mcore_internalResource_t* FindResource
(
    lwm2mcore_internalObject_t* objPtr,     ///< [IN] Object pointer
    uint16_t rid                            ///< [IN] resource ID
)
{
    lwm2mcore_internalResource_t* resourcePtr = NULL;

    LWM2MCORE_ASSERT(objPtr);

    for (resourcePtr = DLIST_FIRST(&(objPtr->resource_list));
         resourcePtr;
         resourcePtr = DLIST_NEXT(resourcePtr, list))
    {
        if (resourcePtr->id == rid)
        {
            break;
        }
    }

    return resourcePtr;
}

//--------------------------------------------------------------------------------------------------
/**
 * Encode read data as a LWM2M data
 *
 * @return
 *      - COAP_205_CONTENT if the data is correctly encoded
 *      - COAP_500_INTERNAL_SERVER_ERROR in case of error
 */
//--------------------------------------------------------------------------------------------------
static uint8_t EncodeData
(
    lwm2mcore_ResourceType_t type,  ///< [IN] LWM2M resource type
    const char* bufPtr,             ///< [IN] Data to encode
    size_t bufSize,                 ///< [IN] Length of data to encode
    lwm2m_data_t* dataPtr           ///< [INOUT] Encoded LWM2M data
)
{
    uint8_t result = COAP_205_CONTENT;

    switch (type)
    {
        case LWM2MCORE_RESOURCE_TYPE_INT:
        case LWM2MCORE_RESOURCE_TYPE_TIME:
        {
            int64_t value = 0;
            value = omanager_BytesToInt(bufPtr, bufSize);
            lwm2m_data_encode_int(value, dataPtr);
        }
        break;

        case LWM2MCORE_RESOURCE_TYPE_BOOL:
            lwm2m_data_encode_bool(bufPtr[0], dataPtr);
            break;

        case LWM2MCORE_RESOURCE_TYPE_STRING:
            lwm2m_data_encode_nstring(bufPtr, bufSize, dataPtr);
            break;

        case LWM2MCORE_RESOURCE_TYPE_OPAQUE:
        case LWM2MCORE_RESOURCE_TYPE_UNKNOWN:
            lwm2m_data_encode_opaque((uint8_t*)bufPtr, bufSize, dataPtr);
            break;

        case LWM2MCORE_RESOURCE_TYPE_FLOAT:
            lwm2m_data_encode_float(atof(bufPtr), dataPtr);
            break;

        default:
            result = COAP_500_INTERNAL_SERVER_ERROR;
            break;
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Read resources with multiple instances in an object
 *
 * @return
 *      - COAP_205_CONTENT if the request is well treated
 *      - COAP_404_NOT_FOUND if no instance is present
 *      - COAP_500_INTERNAL_SERVER_ERROR in case of error
 *      - COAP_501_NOT_IMPLEMENTED if the read callback is not implemented
 */
//--------------------------------------------------------------------------------------------------
static uint8_t ReadResourceInstances
(
    lwm2mcore_Uri_t* uriPtr,                    ///< [IN] Requested operation and object/resource
    lwm2mcore_internalResource_t* resourcePtr,  ///< [IN] LWM2M resource
    lwm2m_data_t* dataPtr,                      ///< [INOUT] Encoded LWM2M data
    char* bufPtr,                               ///< [IN] Read buffer
    size_t bufSize                              ///< [IN] Read Buffer size
)
{
    int sid = 0;
    uint16_t i = 0;
    uint16_t instanceNumber;
    uint8_t result = COAP_404_NOT_FOUND;
    lwm2m_data_t* instancesPtr;

    /* Check for object 2 (ACL
     * For this object, the resource instance Id are not incremented but correspond
     * to a server Id.
     * This means that, for example, 2 resource instances can exist with
     * 1st resource instance Id = 1 (server Id = 1)
     * 2nd resource instance Id = 123 (server Id = 123)
     * So the number of resource instances can not be linked to riid in this case
     */
    if (LWM2MCORE_ACL_OID == uriPtr->oid)
    {
        instanceNumber = omanager_GetAclInstanceNumber(uriPtr->oiid);
    }
    else
    {
        instanceNumber = resourcePtr->maxInstCount;
    }

    instancesPtr = lwm2m_data_new(instanceNumber);
    if (!instancesPtr)
    {
        return COAP_500_INTERNAL_SERVER_ERROR;
    }

    do
    {
        size_t readSize = bufSize;

        memset(bufPtr, 0, bufSize);
        uriPtr->riid = i;

        /* Read the instance of the resource */
        sid  = resourcePtr->read(uriPtr, bufPtr, &readSize, NULL);
        LOG_ARG("Result of reading instance %d: %d", uriPtr->oiid, sid);

        /* Define the CoAP result */
        result = SetCoapError(sid, LWM2MCORE_OP_READ);

        if (COAP_205_CONTENT == result)
        {
            /* Check if some data was returned */
            if (readSize)
            {
                /* Set resource id and encode as LWM2M data */
                (instancesPtr + i)->id = uriPtr->riid;
                result = EncodeData(resourcePtr->type,
                                    bufPtr,
                                    readSize,
                                    instancesPtr + i);
            }
            else
            {
                if (0 == uriPtr->riid)
                {
                    /* No instance, return an error */
                    result = COAP_404_NOT_FOUND;
                }
                else
                {
                    /* No more instance, stop processing without throwing an error */
                    i = resourcePtr->maxInstCount;
                }
            }
        }
        i++;
    }
    while ((i < instanceNumber) && (COAP_205_CONTENT == result));

    if (COAP_205_CONTENT == result)
    {
        /* No error, encode the resources in a single LWM2M data */
        if (LWM2MCORE_ACL_OID == uriPtr->oid)
        {
            lwm2m_data_encode_instances(instancesPtr, instanceNumber, dataPtr);
        }
        else
        {
            lwm2m_data_encode_instances(instancesPtr, uriPtr->riid, dataPtr);
        }
        result = COAP_205_CONTENT;
    }
    else
    {
        /* Error, free allocated memory */
        lwm2m_free(instancesPtr);
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Validate the operational state to see if it's allowed to proceed with the given object instance.
 * So far all objects are allowed to proceed, except object 33405 when it's in the process of and
 * not done with a system time update. Then this function will return false to advise the caller
 * to not proceed.
 *
 * @return
 *      - true if it's good to proceed; false otherwise.
 */
//--------------------------------------------------------------------------------------------------
static bool ValidStateForOperation
(
    lwm2mcore_Uri_t* uri
)
{
    if (!uri)
    {
        return false;
    }

    if (LWM2MCORE_CLOCK_TIME_CONFIG_OID != uri->oid)
    {
        return true;
    }

    // Check if the current state of Clock Service allows this operation
    if (lwm2mcore_UpdateSystemClockInProgress())
    {
        LOG("Operation disallowed when system clock time update is in progress");
        return false;
    }
    return true;
}

//--------------------------------------------------------------------------------------------------
/**
 * Generic function when a READ command is treated for a specific object (Wakaama)
 *
 * @return
 *      - COAP_404_NOT_FOUND if the object instance or read callback is not registered
 *      - COAP_500_INTERNAL_SERVER_ERROR in case of error
 *      - COAP_205_CONTENT if the request is well treated
 */
//--------------------------------------------------------------------------------------------------
static uint8_t ReadCb
(
    uint16_t instanceId,            ///< [IN] Object ID
    int* numDataPtr,                ///< [IN] Number of resources to be read
    lwm2m_data_t** dataArrayPtr,    ///< [IN] Array of requested resources to be read
    lwm2m_object_t* objectPtr       ///< [IN] Pointer on object
)
{
    uint8_t result;
    int i = 0;
    int sid = 0;
    lwm2mcore_Uri_t uri;
    lwm2mcore_internalObject_t* objPtr;
    lwm2mcore_internalResource_t* resourcePtr = NULL;
    char asyncBuf[LWM2MCORE_BUFFER_MAX_LEN];
    size_t asyncBufLen = LWM2MCORE_BUFFER_MAX_LEN;

    if ((NULL == objectPtr) || (NULL == dataArrayPtr))
    {
        return COAP_500_INTERNAL_SERVER_ERROR;
    }

    LOG_ARG("ReadCb oid %d oiid %d", objectPtr->objID, instanceId);

    /* Search if the object was registered */
    if (!LWM2M_LIST_FIND(objectPtr->instanceList, instanceId))
    {
        LOG_ARG("Object %d not found", objectPtr->objID);
        return COAP_404_NOT_FOUND;
    }
    LOG("object instance Id was registered");

    objPtr = FindObject(Lwm2mcoreCtxPtr, objectPtr->objID);
    if (NULL == objPtr)
    {
        LOG_ARG("Object %d is NOT registered", objectPtr->objID);
        return COAP_404_NOT_FOUND;
    }

    memset(&uri, 0, sizeof(uri));
    uri.op = LWM2MCORE_OP_READ;
    uri.oid = objectPtr->objID;
    uri.oiid = instanceId;

    // Validate the operational state early here, as the code to follow may block
    if (!ValidStateForOperation(&uri))
    {
        LOG("Operation disallowed due to the present state");
        return COAP_500_INTERNAL_SERVER_ERROR;
    }

    LOG_ARG("numDataP %d", *numDataPtr);

    /* *numDataPtr set to 0 means that the server is asking for the full object.
     * Otherwise *numDataPtr is set to 1 to read only one resource,
     * dataArrayPtr is already allocated by Wakaama
     * and its id is set to the resource to read. */
    if (0 == *numDataPtr)
    {
        uint16_t resList[50];
        int nbRes = 0;

        /* Search the supported resources for the required object */
        i = 0;
        for (resourcePtr = DLIST_FIRST(&(objPtr->resource_list));
             resourcePtr;
             resourcePtr = DLIST_NEXT(resourcePtr, list))
        {
            if (NULL != resourcePtr->read)
            {
                resList[ i ] = resourcePtr->id;
                LOG_ARG("resList[ %d ] %d", i, resList[ i ]);
                i++;
            }
        }

        nbRes = i;
        LOG_ARG("nbRes %d", nbRes);

        *dataArrayPtr = lwm2m_data_new(nbRes);
        if (NULL == *dataArrayPtr)
        {
            return COAP_500_INTERNAL_SERVER_ERROR;
        }
        *numDataPtr = nbRes;
        for (i = 0 ; i < nbRes ; i++)
        {
            (*dataArrayPtr)[i].id = resList[i];
        }
    }

    i = 0;
    do
    {
        uri.rid = (*dataArrayPtr)[i].id;

        /* Search the resource handler */
        resourcePtr = FindResource(objPtr, uri.rid);
        if (NULL != resourcePtr)
        {
            if (NULL != resourcePtr->read)
            {
                LOG_ARG("READ /%d/%d/%d", uri.oid, uri.oiid, uri.rid);
                asyncBufLen = LWM2MCORE_BUFFER_MAX_LEN;
                memset(asyncBuf, 0, asyncBufLen);

                if (1 < resourcePtr->maxInstCount)
                {
                    result = ReadResourceInstances(&uri,
                                                   resourcePtr,
                                                   (*dataArrayPtr) + i,
                                                   asyncBuf,
                                                   asyncBufLen);
                    LOG_ARG("Result of reading object: %d", objectPtr->objID, result);
                }
                else
                {
                    sid = resourcePtr->read(&uri, asyncBuf, &asyncBufLen, NULL);
                    LOG_ARG("Result of reading instance %d: %d", instanceId, sid);

                    /* Define the CoAP result */
                    result = SetCoapError(sid, LWM2MCORE_OP_READ);

                    if (COAP_205_CONTENT == result)
                    {
                        result = EncodeData(resourcePtr->type,
                                            asyncBuf,
                                            asyncBufLen,
                                            (*dataArrayPtr) + i);
                    }
                }

                if (COAP_404_NOT_FOUND == result)
                {
                    /* Read resource is not implemented, check the number of resources */
                    if (1 == *numDataPtr)
                    {
                        /* This was the only resource to read, return an error.
                         * No need to free the memory as it will be done by
                         * the calling function. */
                        result = COAP_404_NOT_FOUND;
                        i++;
                    }
                    else
                    {
                        /* Remove the corresponding data */
                        lwm2m_data_t* newDataArrayPtr;

                        /* Allocate a new buffer */
                        newDataArrayPtr = lwm2m_data_new((*numDataPtr)-1);
                        if (NULL == newDataArrayPtr)
                        {
                            return COAP_500_INTERNAL_SERVER_ERROR;
                        }
                        /* Copy the previous data, except for the "not implemented" one */
                        memcpy(newDataArrayPtr, *dataArrayPtr, i * sizeof(lwm2m_data_t));
                        memcpy(newDataArrayPtr + i,
                               (*dataArrayPtr) + (i+1),
                               ((*numDataPtr)-(i+1)) * sizeof(lwm2m_data_t));
                        /* Free the previous buffer */
                        lwm2m_free(*dataArrayPtr);
                        /* Update the output data */
                        *dataArrayPtr = newDataArrayPtr;
                        (*numDataPtr)--;
                        result = COAP_205_CONTENT;
                    }
                }
                else
                {
                    i++;
                }
            }
            else
            {
                LOG("READ callback NULL");
                result = COAP_405_METHOD_NOT_ALLOWED;
                i++;
            }
        }
        else
        {
            LOG("resource NULL");
            result = COAP_404_NOT_FOUND;
            i++;
        }
    } while (   (i < *numDataPtr)
             && (   (COAP_205_CONTENT == result)
                 || (COAP_NO_ERROR == result)
                )
            );

    LOG_ARG("ReadCb result %d", result);
    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Revert a character from base 64
 *
 * @return
 *      - character value
 *      - 0 on error
 */
//--------------------------------------------------------------------------------------------------
static uint8_t prv_b64Revert
(
    uint8_t value       ///< [IN] Character to decode
)
{
    if (value >= 'A' && value <= 'Z')
    {
        return (value - 'A');
    }

    if (value >= 'a' && value <= 'z')
    {
        return (26 + value - 'a');
    }

    if (value >= '0' && value <= '9')
    {
        return (52 + value - '0');
    }

    switch (value)
    {
        case '+':
            return 62;
        case '/':
            return 63;
        default:
            return 0;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Decode a 4-bit base 64 encoded block
 */
//--------------------------------------------------------------------------------------------------
static void prv_decodeBlock
(
    uint8_t input[4],       ///< [IN] Base 64 encoded bloc
    uint8_t output[3]       ///< [OUT] Decoded block
)
{
    uint8_t tmp[4];
    int i;

    memset(output, 0, 3);

    for (i = 0; i < 4; i++)
    {
        tmp[i] = prv_b64Revert(input[i]);
    }

    output[0] = (tmp[0] << 2) | (tmp[1] >> 4);
    output[1] = (tmp[1] << 4) | (tmp[2] >> 2);
    output[2] = (tmp[2] << 6) | tmp[3];
}

//--------------------------------------------------------------------------------------------------
/**
 * Decode a base 64 string
 *
 * @return
 *      - decoded string length
 *      - 0 on error
 */
//--------------------------------------------------------------------------------------------------
size_t base64_decode
(
    uint8_t * dataP,    ///< [IN] Base 64 string
    size_t    dataLen,  ///< [IN] Base 64 string length
    uint8_t * bufferP   ///< [OUT] Buffer for decoded string
)
{
    size_t data_index;
    size_t result_index;
    size_t result_len;

    // Check if the number of bytes is a multiple of 4
    if (dataLen % 4)
    {
        return 0;
    }

    result_len = (dataLen >> 2) * 3;

    // remove padding
    while (dataP[dataLen - 1] == B64_PADDING)
    {
        dataLen--;
    }

    data_index = 0;
    result_index = 0;
    while (data_index < dataLen)
    {
        prv_decodeBlock(dataP + data_index, bufferP + result_index);
        data_index += 4;
        result_index += 3;
    }

    switch (data_index - dataLen)
    {
        case 0:
            break;

        case 2:
        {
            uint8_t tmp[2];

            tmp[0] = prv_b64Revert(dataP[dataLen - 2]);
            tmp[1] = prv_b64Revert(dataP[dataLen - 1]);

            bufferP[result_index - 3] = (tmp[0] << 2) | (tmp[1] >> 4);
            bufferP[result_index - 2] = (tmp[1] << 4);
            result_len -= 2;
        }
        break;

        case 3:
        {
            uint8_t tmp[3];

            tmp[0] = prv_b64Revert(dataP[dataLen - 3]);
            tmp[1] = prv_b64Revert(dataP[dataLen - 2]);
            tmp[2] = prv_b64Revert(dataP[dataLen - 1]);

            bufferP[result_index - 3] = (tmp[0] << 2) | (tmp[1] >> 4);
            bufferP[result_index - 2] = (tmp[1] << 4) | (tmp[2] >> 2);
            bufferP[result_index - 1] = (tmp[2] << 6);
            result_len -= 1;
        }
        break;

        default:
            bufferP = NULL;
            result_len = 0;
            break;
    }

    return result_len;
}


//--------------------------------------------------------------------------------------------------
/**
 * Generic function when a WRITE/EXECUTE command is treated to format the received data
 *
 * @return
 *      - true on success
 *      - false else
 */
//--------------------------------------------------------------------------------------------------
static bool FormatDataWriteExecute
(
    lwm2mcore_ResourceType_t    resourceFormatType,     ///< [IN] Resource format type
    lwm2m_data_t                dataArray,              ///< [IN] Array of requested resources to be
                                                        ///< written
    char*                       bufferPtr,              ///< [IN] Output buffer
    size_t*                     bufferLenPtr            ///< [IN] Output buffer length
)
{
    bool result = false;

    if ( (NULL == bufferPtr) || (NULL == bufferLenPtr))
    {
        return false;
    }

    // This indicates in which format the server sent a data
    // According to Wakaama source code (see lwm2m_data_parse API)):
    // - when a WRITE/EXECUTE command is received on a specific resoure in TEXT format, the type is
    //   set to LWM2M_TYPE_STRING
    // - when a WRITE/EXECUTE command is received on a specific resoure in TLV format, the type is
    //   set to LWM2M_TYPE_OPAQUE
    // - when a WRITE command is received on a specific object in TLV format, the type is set to
    //   LWM2M_TYPE_OPAQUE
    switch (dataArray.type)
    {
        case LWM2M_TYPE_STRING:
        {
            if (dataArray.value.asBuffer.length <= (*bufferLenPtr))
            {
                // Check the resource format
                switch (resourceFormatType)
                {
                    case LWM2MCORE_RESOURCE_TYPE_INT:
                    case LWM2MCORE_RESOURCE_TYPE_BOOL:
                    case LWM2MCORE_RESOURCE_TYPE_TIME:
                    {
                        // The received data is the integer/boolean value in text
                        // Time is also a type represented in integer rather than string form
                        // Example: value 123 is sent like 0x31 32 33
                        // Change the string in value
                        int64_t value = 0;

                        // If the length is 0, immediately return the right length value
                        // else Wakaama utils_textToInt will return an error
                        if(!(dataArray.value.asBuffer.length))
                        {
                            *bufferLenPtr = 0;
                            return true;
                        }

                        if (utils_textToInt(dataArray.value.asBuffer.buffer,
                                            dataArray.value.asBuffer.length,
                                            &value))
                        {
                            // Treat it in buffer for generic handlers treament
                            *bufferLenPtr = omanager_FormatValueToBytes((uint8_t*) bufferPtr,
                                                                        &value,
                                                                        sizeof(value),
                                                                        false);
                            result = true;
                        }
                    }
                    break;

                    case LWM2MCORE_RESOURCE_TYPE_OPAQUE:
                    {
                        size_t len = 0;
                        // decode b64
                        len = base64_decode(dataArray.value.asBuffer.buffer,
                                            dataArray.value.asBuffer.length,
                                            (uint8_t*)bufferPtr);
                        *bufferLenPtr = len;
                        result = true;
                    }
                    break;

                    case LWM2MCORE_RESOURCE_TYPE_FLOAT:
                    case LWM2MCORE_RESOURCE_TYPE_UNKNOWN:
                    default:
                        memcpy(bufferPtr,
                               dataArray.value.asBuffer.buffer,
                               dataArray.value.asBuffer.length);
                        *bufferLenPtr = dataArray.value.asBuffer.length;
                        result = true;
                    break;
                }
            }
        }
        break;

        case LWM2M_TYPE_OPAQUE:
        {
            if (dataArray.value.asBuffer.length <= (*bufferLenPtr))
            {
                memcpy(bufferPtr, dataArray.value.asBuffer.buffer, dataArray.value.asBuffer.length);
                *bufferLenPtr = dataArray.value.asBuffer.length;
                result = true;
            }
        }
        break;

        case LWM2M_TYPE_INTEGER:
            *bufferLenPtr = omanager_FormatValueToBytes((uint8_t*) bufferPtr,
                                                        &(dataArray.value.asInteger),
                                                        sizeof(dataArray.value.asInteger),
                                                        false);
            result = true;
        break;

        case LWM2M_TYPE_FLOAT:
        case LWM2M_TYPE_BOOLEAN:
        default:
            LOG_ARG("Unmanaged type format for WRITE/EXECUTE %d", dataArray.type);
            *bufferLenPtr = 0;
        break;
    }

    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Generic function when a WRITE command is treated for a specific object (Wakaama)
 *
 * @return
 *      - COAP_404_NOT_FOUND if the object instance or write callback is not registered
 *      - COAP_500_INTERNAL_SERVER_ERROR in case of error
 *      - COAP_204_CHANGED if the request is well treated
 */
//--------------------------------------------------------------------------------------------------
static uint8_t WriteCb
(
    uint16_t instanceId,            ///< [IN] Object ID
    int numData,                    ///< [IN] Number of resources to be written
    lwm2m_data_t* dataArrayPtr,     ///< [IN] Array of requested resources to be written
    lwm2m_object_t* objectPtr       ///< [IN] Pointer on object
)
{
    uint8_t result = COAP_400_BAD_REQUEST;
    int i;
    bool isMalloc = false;
    int sid = 0;
    lwm2mcore_Uri_t uri;
    lwm2mcore_internalResource_t* resourcePtr = NULL;
    lwm2mcore_internalObject_t* objPtr;
    char asyncBuf[LWM2MCORE_BUFFER_MAX_LEN];
    size_t asyncBufLen = LWM2MCORE_BUFFER_MAX_LEN;

    if ((NULL == objectPtr) || (NULL == dataArrayPtr))
    {
        return COAP_500_INTERNAL_SERVER_ERROR;
    }

    LOG_ARG("WriteCb oid %d oiid %d", objectPtr->objID, instanceId);

    /* Search if the object was registered */
    if (!LWM2M_LIST_FIND(objectPtr->instanceList, instanceId))
    {
        LOG_ARG("Object %d not found", objectPtr->objID);
        return COAP_404_NOT_FOUND;
    }

    LOG("object instance Id was registered");

    objPtr = FindObject(Lwm2mcoreCtxPtr, objectPtr->objID);
    if (!objPtr)
    {
        LOG_ARG("Object %d is NOT registered", objectPtr->objID);
        return COAP_404_NOT_FOUND;
    }

    memset(&uri, 0, sizeof (lwm2mcore_Uri_t));
    uri.op = LWM2MCORE_OP_WRITE;
    uri.oid = objectPtr->objID;
    uri.oiid = instanceId;

    // Validate the operational state early here, as the code to follow may block
    if (!ValidStateForOperation(&uri))
    {
        LOG("Operation disallowed due to the present state");
        return COAP_500_INTERNAL_SERVER_ERROR;
    }

    LOG_ARG("numData %d", numData);
    // is the server asking for the full object ?
    if (0 == numData)
    {
        uint16_t resList[50];
        int nbRes = 0;
        isMalloc = true;
        /* Search the supported resources for the required object */
        i = 0;
        for (resourcePtr = DLIST_FIRST(&(objPtr->resource_list));
             resourcePtr;
             resourcePtr = DLIST_NEXT(resourcePtr, list))
        {
            resList[ i++ ] = resourcePtr->id;
        }

        nbRes = i;

        dataArrayPtr = lwm2m_data_new(nbRes);
        if (NULL == dataArrayPtr)
        {
            return COAP_500_INTERNAL_SERVER_ERROR;
        }
        numData = nbRes;
        for (i = 0 ; i < nbRes ; i++)
        {
            dataArrayPtr[i].id = resList[i];
        }
    }

    i = 0;
    do
    {
        uri.rid = dataArrayPtr[i].id;
        memset(asyncBuf, 0, LWM2MCORE_BUFFER_MAX_LEN);
        asyncBufLen = LWM2MCORE_BUFFER_MAX_LEN;

        /* Search the resource handler */
        resourcePtr = FindResource(objPtr, uri.rid);
        if (!resourcePtr)
        {
            LOG("resource NULL");
            if (isMalloc)
            {
                lwm2m_free(dataArrayPtr);
            }
            return COAP_404_NOT_FOUND;
        }

        if (!(resourcePtr->write))
        {
            LOG("WRITE callback NULL");
             if (isMalloc)
            {
                lwm2m_free(dataArrayPtr);
            }
            return COAP_405_METHOD_NOT_ALLOWED;
        }

        LOG_ARG("data type %d resourcePtr->ptr %d", dataArrayPtr[i].type, resourcePtr->type);
        if (LWM2M_TYPE_MULTIPLE_RESOURCE != dataArrayPtr[i].type)
        {
            if (FormatDataWriteExecute(resourcePtr->type,
                                       dataArrayPtr[i],
                                       asyncBuf,
                                       &asyncBufLen))
            {
                LOG_ARG("WRITE / %d / %d / %d", uri.oid, uri.oiid, uri.rid);
                sid = resourcePtr->write(&uri, asyncBuf, asyncBufLen);
                LOG_ARG("WRITE sID %d", sid);
                /* Define the CoAP result */
                result = SetCoapError(sid, LWM2MCORE_OP_WRITE);
            }
            else
            {
                result = COAP_400_BAD_REQUEST;
            }
        }
        else
        {
            uint32_t count = (uint32_t)dataArrayPtr[i].value.asChildren.count;
            uint32_t loop;

            for (loop = 0; loop < count; loop++)
            {
                if (FormatDataWriteExecute(resourcePtr->type,
                                           dataArrayPtr[i].value.asChildren.array[loop],
                                           asyncBuf,
                                           &asyncBufLen))
                {
                    uri.riid = dataArrayPtr[i].value.asChildren.array[loop].id;
                    LOG_ARG("WRITE / %d / %d / %d / %d", uri.oid, uri.oiid, uri.rid, uri.riid);
                    sid = resourcePtr->write(&uri, asyncBuf, asyncBufLen);
                    LOG_ARG("WRITE sID %d", sid);
                    /* Define the CoAP result */
                    result = SetCoapError(sid, LWM2MCORE_OP_WRITE);
                }
                else
                {
                    result = COAP_400_BAD_REQUEST;
                }
            }
        }
        i++;
    } while ((i < numData) && ((COAP_204_CHANGED == result) || (COAP_NO_ERROR == result)));

    LOG_ARG("WriteCb result %d", result);

    if (isMalloc)
    {
        lwm2m_free(dataArrayPtr);
    }
    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Delete an object instance in the Wakaama format
 *
 * @return
 *      - COAP_404_NOT_FOUND if the object instance does not exist
 *      - COAP_500_INTERNAL_SERVER_ERROR in case of error
 *      - COAP_202_DELETED if the request is well treated
 */
//--------------------------------------------------------------------------------------------------
static uint8_t DeleteObjInstance
(
    uint16_t id,                    ///< [IN] Object instance ID
    lwm2m_object_t* objectPtr       ///< [IN] Pointer on object
)
{
    lwm2m_list_t* instancePtr;

    if (NULL == objectPtr)
    {
        return COAP_500_INTERNAL_SERVER_ERROR;
    }

    objectPtr->instanceList = lwm2m_list_remove(objectPtr->instanceList,
                                                id,
                                                (lwm2m_list_t **)&instancePtr);
    if (NULL == instancePtr)
    {
        return COAP_404_NOT_FOUND;
    }

    lwm2m_free(instancePtr);

    return COAP_202_DELETED;
}

//--------------------------------------------------------------------------------------------------
/**
 * Generic function when a CREATE command is treated for a specific object (Wakaama)
 *
 * @return
 *      - COAP_400_BAD_REQUEST if the object instance already exists
 *      - COAP_500_INTERNAL_SERVER_ERROR in case of error
 *      - COAP_201_CREATED if the request is well treated
 */
//--------------------------------------------------------------------------------------------------
static uint8_t CreateCb
(
    uint16_t instanceId,            ///< [IN] Object ID
    int numData,                    ///< [IN] Number of resources to be written
    lwm2m_data_t* dataArrayPtr,     ///< [IN] Array of requested resources to be written
    lwm2m_object_t* objectPtr       ///< [IN] Pointer on object
)
{
    uint8_t result;

    if ((NULL == objectPtr) || (NULL == dataArrayPtr))
    {
        return COAP_500_INTERNAL_SERVER_ERROR;
    }

    LOG_ARG("CreateCb oid %d oiid %d", objectPtr->objID, instanceId);

    if (LWM2MCORE_SOFTWARE_UPDATE_OID == objectPtr->objID)
    {
        if (LWM2MCORE_ERR_COMPLETED_OK != lwm2mcore_SoftwareUpdateInstance(true, instanceId))
        {
            LOG("Error from client to create object instance");
            return COAP_500_INTERNAL_SERVER_ERROR;
        }
    }

    if (NULL == objectPtr->instanceList)
    {
        LOG("objectPtr->instanceList == NULL");
        objectPtr->instanceList = (lwm2m_list_t *)lwm2m_malloc(sizeof(lwm2m_list_t));
        if (NULL != objectPtr->instanceList)
        {
            memset(objectPtr->instanceList, 0, sizeof(lwm2m_list_t));
        }
    }
    /* Search if the object was registered */
    else if (LWM2M_LIST_FIND(objectPtr->instanceList, instanceId) == NULL)
    {
        lwm2m_list_t* instancePtr;
        /* Add the object instance in the Wakaama format */
        instancePtr = (lwm2m_list_t *)lwm2m_malloc(sizeof(lwm2m_list_t));
        if (!instancePtr)
        {
           LOG("instancePtr is NULL");
           return COAP_500_INTERNAL_SERVER_ERROR;
        }
        instancePtr->id = instanceId;
        objectPtr->instanceList = LWM2M_LIST_ADD(objectPtr->instanceList, instancePtr);
        /* instancePtr is released by omanager_ObjectsFree API */
    }
    else
    {
        LOG("Object instance already exists");
        return COAP_400_BAD_REQUEST;
    }

    if (COAP_204_CHANGED != WriteCb(instanceId, numData, dataArrayPtr, objectPtr))
    {
        LOG_ARG("CreateCb --> delete oiid %d", instanceId);
        DeleteObjInstance(instanceId, objectPtr);
        result = COAP_500_INTERNAL_SERVER_ERROR;
    }
    else
    {
        result = COAP_201_CREATED;
    }

    LOG_ARG("CreateCb result %d", result);

    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Generic function when a DELETE command is treated for a specific object (Wakaama)
 *
 * @return
 *      - COAP_400_BAD_REQUEST if the object instance does not exist
 *      - COAP_500_INTERNAL_SERVER_ERROR in case of error
 *      - COAP_202_DELETED if the request is well treated
 */
//--------------------------------------------------------------------------------------------------
static uint8_t DeleteCb
(
    uint16_t instanceId,            ///< [IN] Object instance ID
    lwm2m_object_t* objectPtr       ///< [IN] Pointer on object
)
{
    uint8_t result;
    bool isDeviceManagement = false;
    lwm2m_list_t* instancePtr;

    if ((NULL == objectPtr) || (NULL == objectPtr->userData))
    {
        return COAP_500_INTERNAL_SERVER_ERROR;
    }

    LOG_ARG("DeleteCb oid %d oiid %d", objectPtr->objID, instanceId);

    /* Check the session
     * If the device is connected to the bootstrap server, only accept DELETE command on
     * Security object (object 0)
     */
    if (true == lwm2mcore_ConnectionGetType(objectPtr->userData, &isDeviceManagement))
    {
        if ((false == isDeviceManagement)
        && (LWM2MCORE_SECURITY_OID != objectPtr->objID))
        {
            LOG("DeleteCb return COAP_405_METHOD_NOT_ALLOWED");
            return COAP_405_METHOD_NOT_ALLOWED;
        }
    }
    else
    {
        LOG("error on Get type");
        return COAP_500_INTERNAL_SERVER_ERROR;
    }

    if (LWM2MCORE_SOFTWARE_UPDATE_OID == objectPtr->objID)
    {
        if (LWM2MCORE_ERR_COMPLETED_OK != lwm2mcore_SoftwareUpdateInstance(false, instanceId))
        {
            LOG("Error from client to delete object instance");
            return COAP_500_INTERNAL_SERVER_ERROR;
        }
    }

    /* Search if the object instance was registered */
    instancePtr = LWM2M_LIST_FIND(objectPtr->instanceList, instanceId);

    if (NULL != instancePtr)
    {
        if (LWM2MCORE_ACL_OID == (objectPtr->objID))
        {
            omanager_RemoveAclObjectInstance(instanceId);
            lwm2m_acl_deleteObjectInstance(objectPtr, instanceId);
            omanager_StoreAclConfiguration();
            result = COAP_202_DELETED;
        }
        else
        {
            lwm2m_client_t* clientP;
            /* Delete the object instance in the Wakaama format */
            objectPtr->instanceList = LWM2M_LIST_RM(objectPtr->instanceList, instanceId, &clientP);
            lwm2m_free(instancePtr);

            if (LWM2MCORE_SOFTWARE_UPDATE_OID == objectPtr->objID)
            {
                LOG_ARG("Remove oiid %d from SwApplicationListPtr", instanceId);
                instancePtr = (lwm2m_list_t*)LWM2M_LIST_FIND(SwApplicationListPtr, instanceId);

                if (NULL != instancePtr)
                {
                    ObjectInstanceList_t* appPtr;
                    SwApplicationListPtr = (ObjectInstanceList_t*)LWM2M_LIST_RM(SwApplicationListPtr,
                                                                               instanceId,
                                                                               &appPtr);
                    lwm2m_free(instancePtr);
                }
                result = COAP_202_DELETED;
            }
#ifdef LWM2M_OBJECT_33406
            else if (LWM2MCORE_FILE_LIST_OID == objectPtr->objID)
            {
                lwm2mcore_Sid_t sID;
                sID = lwm2mcore_DeleteFileByInstance(instanceId);
                switch (sID)
                {
                    case LWM2MCORE_ERR_COMPLETED_OK:
                        result = COAP_202_DELETED;
                        break;

                    case LWM2MCORE_ERR_INVALID_ARG:
                        result = COAP_404_NOT_FOUND;
                        break;

                    default:
                        result = COAP_400_BAD_REQUEST;
                        break;
                }

            }
#endif
            else
            {
                result = COAP_202_DELETED;
            }
        }
    }
    else
    {
        LOG("Object instance does not exist");
        result = COAP_400_BAD_REQUEST;
    }
    LOG_ARG("DeleteCb result %d", result);
    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Generic function when a DISCOVER command is treated for a specific object (Wakaama)
 *
 * @return
 *      - 0
 */
//--------------------------------------------------------------------------------------------------
static uint8_t DiscoverCb
(
    uint16_t instanceId,            ///< [IN] Object ID
    int* numDataPtr,                ///< [INOUT] Number of resources which were read
    lwm2m_data_t ** dataArrayPtr,   ///< [IN] Array of requested resources to be discovered
    lwm2m_object_t* objectPtr       ///< [IN] Pointer on object
)
{
    (void)instanceId;
    (void)numDataPtr;

    if ((NULL == objectPtr) || (NULL == dataArrayPtr))
    {
        return COAP_500_INTERNAL_SERVER_ERROR;
    }
    return 0;
}

//--------------------------------------------------------------------------------------------------
/**
 * Generic function when a EXECUTE command is treated for a specific object (Wakaama)
 *
 * @return
 *      - COAP_404_NOT_FOUND if the object / object instance / resource instance does not exist
 *      - COAP_500_INTERNAL_SERVER_ERROR in case of error
 *      - COAP_204_CHANGED if the request is well treated
 */
//--------------------------------------------------------------------------------------------------
static uint8_t ExecuteCb
(
    uint16_t instanceId,            ///< [IN] Object ID
    uint16_t resourceId,            ///< [IN] Resource ID
    uint8_t* bufferPtr,             ///< [IN] Data provided in the EXECUTE command
    int length,                     ///< [IN] Data length
    lwm2m_object_t* objectPtr       ///< [IN] Pointer on object
)
{
    uint8_t result;

    if ((NULL == objectPtr) || ((NULL == bufferPtr) && length))
    {
        return COAP_500_INTERNAL_SERVER_ERROR;
    }

    LOG_ARG("ExecuteCb oid %d oiid %d rid %d", objectPtr->objID, instanceId, resourceId);

    /* Search if the object was registered */
    if (LWM2M_LIST_FIND(objectPtr->instanceList, instanceId))
    {
        lwm2mcore_internalObject_t* objPtr;
        LOG("object instance Id was registered");

        objPtr = FindObject(Lwm2mcoreCtxPtr, objectPtr->objID);
        if (NULL == objPtr)
        {
            LOG_ARG("Object %d is NOT registered", objectPtr->objID);
            result = COAP_404_NOT_FOUND;
        }
        else
        {
            int sid = 0;
            lwm2mcore_internalResource_t* resourcePtr = NULL;
            char asyncBuf[LWM2MCORE_BUFFER_MAX_LEN];
            size_t asyncBufLen = LWM2MCORE_BUFFER_MAX_LEN;
            lwm2mcore_Uri_t uri;
            memset(&uri, 0, sizeof (lwm2mcore_Uri_t));
            uri.op = LWM2MCORE_OP_EXECUTE;
            uri.oid = objectPtr->objID;
            uri.oiid = instanceId;
            uri.rid = resourceId;

            // Validate the operational state early here, as the code to follow may block
            if (!ValidStateForOperation(&uri))
            {
                LOG("Operation disallowed due to the present state");
                return COAP_500_INTERNAL_SERVER_ERROR;
            }

            /* Search the resource handler */
            resourcePtr = FindResource(objPtr, uri.rid);
            if (NULL != resourcePtr)
            {
                if (NULL != resourcePtr->exec)
                {
                    lwm2m_data_t dataArray;
                    dataArray.type = LWM2M_TYPE_STRING;
                    dataArray.value.asBuffer.length = (size_t)length;
                    dataArray.value.asBuffer.buffer = bufferPtr;
                    memset(asyncBuf, 0, asyncBufLen);

                    LOG_ARG("data type %d resourcePtr->type %d", dataArray.type, resourcePtr->type);
                    if (FormatDataWriteExecute(resourcePtr->type,
                                               dataArray,
                                               asyncBuf,
                                               &asyncBufLen))
                    {
                        LOG_ARG("EXECUTE / %d / %d / %d", uri.oid, uri.oiid, uri.rid);
                        sid  = resourcePtr->exec(&uri, asyncBuf, asyncBufLen);
                        LOG_ARG("EXECUTE sID %d", sid);
                        /* Define the CoAP result */
                        result = SetCoapError(sid, LWM2MCORE_OP_EXECUTE);
                    }
                    else
                    {
                        result = COAP_400_BAD_REQUEST;
                    }
                }
                else
                {
                    LOG("EXECUTE callback NULL");
                    result = COAP_405_METHOD_NOT_ALLOWED;
                }
            }
            else
            {
                LOG("resource NULL");
                result = COAP_404_NOT_FOUND;
            }
        }
    }
    else
    {
        LOG_ARG("Object %d not found", objectPtr->objID);
        result = COAP_404_NOT_FOUND;
    }
    LOG_ARG("ExecuteCb result %d", result);
    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Get the supported object list for LWM2M Core
 *
 * @return
 *      - NULL in case of error
 *      - object list else
 */
//--------------------------------------------------------------------------------------------------
static struct _lwm2mcore_objectsList* GetObjectsList
(
    void
)
{
    if (NULL != Lwm2mcoreCtxPtr)
    {
        return &(Lwm2mcoreCtxPtr->objects_list);
    }
    else
    {
        return NULL;
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Initialize LwM2M object
 *
 * @return
 *      - object pointer
 */
//--------------------------------------------------------------------------------------------------
static lwm2mcore_internalObject_t* InitObject
(
    lwm2mcore_Object_t* client_objPtr,  ///< [IN] pointer to object passed from client
    uint16_t iid,                       ///< [IN] object instance ID
    bool multiple                       ///< [IN] if this is single or multiple instance object
)
{
    int j;
    lwm2mcore_internalObject_t* objPtr = NULL;
    lwm2mcore_internalResource_t* resourcePtr = NULL;
    lwm2mcore_Resource_t *client_resourcePtr = NULL;

    if (NULL == client_objPtr)
    {
        return NULL;
    }

    LOG_ARG("InitObject /%d/%d, multiple %d", client_objPtr->id, iid, multiple);

    objPtr = (lwm2mcore_internalObject_t*)lwm2m_malloc(sizeof (lwm2mcore_internalObject_t));

    LWM2MCORE_ASSERT(objPtr);

    memset(objPtr, 0, sizeof (lwm2mcore_internalObject_t));

    objPtr->multiple = multiple;
    objPtr->id = client_objPtr->id;
    objPtr->iid = iid;
    memset(&(objPtr->attr), 0, sizeof (lwm2m_attribute_t));

    /* Object's create and delete handlers should be invoked by the LWM2M client
     * itself. Once the operation is completed, the client shall call avcm_create_lwm2m_object
     * or avcm_delete_lwm2m_object accordingly */
    client_resourcePtr = client_objPtr->resources;

    LOG_ARG("InitObject client_obj->resCnt %d", client_objPtr->resCnt);

    DLIST_INIT(&(objPtr->resource_list));

    if ((LWM2MCORE_SOFTWARE_UPDATE_OID == client_objPtr->id)
     && (LWM2MCORE_ID_NONE == iid))
    {
        /* Object 9 without any object instance
         * Get information from host
         */
        LOG("Object 9 without any object instance");

    }

    for (j = 0; j < client_objPtr->resCnt; j++)
    {
        resourcePtr =
                (lwm2mcore_internalResource_t*)lwm2m_malloc(sizeof(lwm2mcore_internalResource_t));

        LWM2MCORE_ASSERT(resourcePtr);
        memset(resourcePtr, 0, sizeof(lwm2mcore_internalResource_t));

        resourcePtr->id = (client_resourcePtr + j)->id;
        resourcePtr->iid = 0;
        resourcePtr->type = (client_resourcePtr + j)->type;
        resourcePtr->maxInstCount = (client_resourcePtr + j)->maxResInstCnt;
        memset(&resourcePtr->attr, 0, sizeof(lwm2m_attribute_t));
        resourcePtr->read = (client_resourcePtr + j)->read;
        resourcePtr->write = (client_resourcePtr + j)->write;
        resourcePtr->exec = (client_resourcePtr + j)->exec;
        DLIST_INSERT_TAIL(&(objPtr->resource_list), resourcePtr, list);
    }

    return objPtr;
}

//--------------------------------------------------------------------------------------------------
/**
 * Initialize supported objects/resources based on handler data.
 */
//--------------------------------------------------------------------------------------------------
static void InitObjectsList
(
    struct _lwm2mcore_objectsList* objects_list ,   ///< [IN] Object list
    lwm2mcore_Handler_t* clientHandlerPtr           ///< [IN] Object and resource table which are
                                                    ///<      supported by the client
)
{
    int i, j;
    lwm2mcore_internalObject_t* objPtr = NULL;

    if ((NULL == objects_list) || (NULL == clientHandlerPtr))
    {
        return;
    }

    LOG_ARG("objCnt %d", clientHandlerPtr->objCnt);

    for (i = 0; i < clientHandlerPtr->objCnt; i++)
    {
        if (LWM2MCORE_ID_NONE == (clientHandlerPtr->objects + i)->maxObjInstCnt)
        {
            /* Unknown object instance count is always assumed to be multiple */
            objPtr = InitObject(clientHandlerPtr->objects + i, LWM2MCORE_ID_NONE, true);
            if (!objPtr)
            {
               LOG("objPtr is NULL");
               return;
            }
            DLIST_INSERT_TAIL(objects_list, objPtr, list);
        }
        else if ((clientHandlerPtr->objects + i)->maxObjInstCnt > 1)
        {
            for (j = 0; j < (clientHandlerPtr->objects + i)->maxObjInstCnt; j++)
            {
                objPtr = InitObject(clientHandlerPtr->objects + i, j, true);
                if (!objPtr)
                {
                   LOG("objPtr is NULL");
                   return;
                }
                DLIST_INSERT_TAIL(objects_list, objPtr, list);
            }
        }
        else if (LWM2M_SERVER_OBJECT_ID == (clientHandlerPtr->objects + i)->id)
        {
            /* the maxObjInstCnt is 1 for this object, but this is actually multiple instance */
            objPtr = InitObject(clientHandlerPtr->objects + i, 0, true);
            if (!objPtr)
            {
               LOG("objPtr is NULL");
               return;
            }
            DLIST_INSERT_TAIL(objects_list, objPtr, list);
        }
        else
        {
            objPtr = InitObject(clientHandlerPtr->objects + i, 0, false);
            if (!objPtr)
            {
               LOG("objPtr is NULL");
               return;
            }
            DLIST_INSERT_TAIL(objects_list, objPtr, list);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Free the registered objects and resources (LwM2MCore and Wakaama)
 */
//--------------------------------------------------------------------------------------------------
void omanager_ObjectsFree
(
    void
)
{
    struct _lwm2mcore_objectsList* objectsListPtr = GetObjectsList();
    lwm2mcore_internalObject_t* objPtr = NULL;
    lwm2mcore_internalResource_t* resPtr = NULL;
    uint32_t i = 0;
    if (NULL == objectsListPtr)
    {
       LOG("objectsListPtr is NULL");
       return;
    }

    /* Free memory for objects and resources for LwM2MCore */
    while ((objPtr = DLIST_FIRST(objectsListPtr)) != NULL)
    {
        while ((resPtr = DLIST_FIRST(&(objPtr->resource_list))) != NULL)
        {
            DLIST_REMOVE_HEAD(&(objPtr->resource_list), list);
            lwm2m_free(resPtr);
        }
        DLIST_REMOVE_HEAD(objectsListPtr, list);
        lwm2m_free(objPtr);
    }

    /* Free memory for objects and resources for Wakaama */
    LOG_ARG("Wakaama RegisteredObjNb %d", RegisteredObjNb);
    for (i = 0; i < RegisteredObjNb; i++)
    {
        if (ObjectArray[i])
        {
            while (ObjectArray[i]->instanceList != NULL)
            {
                lwm2m_list_t *listPtr = ObjectArray[i]->instanceList;
                ObjectArray[i]->instanceList = ObjectArray[i]->instanceList->next;
                lwm2m_free(listPtr);
            }

            lwm2m_free(ObjectArray[i]);
            ObjectArray[i] = NULL;
        }
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Free the registered objects and resources (LwM2MCore and Wakaama) for a specific object Id
 */
//--------------------------------------------------------------------------------------------------
void omanager_FreeObjectById
(
    uint16_t    objectId        ///< [IN] Object Id to remove
)
{
    uint32_t i = 0;

    /* Free memory for objects and resources for Wakaama */
    for (i = 0; i < RegisteredObjNb; i++)
    {
        if (ObjectArray[i] && (ObjectArray[i]->objID == objectId))
        {
            while (ObjectArray[i]->instanceList != NULL)
            {
                lwm2m_list_t *listPtr = ObjectArray[i]->instanceList;
                ObjectArray[i]->instanceList = ObjectArray[i]->instanceList->next;
                lwm2m_free(listPtr);
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/**
 * Free the registered objects and resources (LwM2MCore and Wakaama) for a specific object Id and
 * object instance Id
 */
//--------------------------------------------------------------------------------------------------
void omanager_FreeObjectByInstanceId
(
    uint16_t    objectId,           ///< [IN] Object Id to remove
    uint16_t    objectInstanceId    ///< [IN] Object instance Id to remove
)
{
    uint32_t i = 0;

    /* Free memory for objects and resources for Wakaama */
    for (i = 0; i < RegisteredObjNb; i++)
    {
        if (ObjectArray[i] && (ObjectArray[i]->objID == objectId))
        {
            lwm2m_list_t* instancePtr;
            ObjectArray[i]->instanceList = lwm2m_list_remove(ObjectArray[i]->instanceList,
                                                             objectInstanceId,
                                                             (lwm2m_list_t **)&instancePtr);
            lwm2m_free(instancePtr);
        }
    }
}


//--------------------------------------------------------------------------------------------------
/**
 * @brief Get the number of registered object instance ID for a specific object
 *
 * @note If the object is not registered, 0 is returned
 *
 * @return
 *  - Object instance count
 */
//--------------------------------------------------------------------------------------------------
uint16_t omanager_ObjectInstanceCount
(
    uint16_t oid                            ///< [IN] object ID to find
)
{
    uint32_t i = 0;
    uint16_t count = 0;

    for (i = 0; i < RegisteredObjNb; i++)
    {
        if (ObjectArray[i] && (ObjectArray[i]->objID == oid))
        {
            lwm2m_list_t *listPtr = ObjectArray[i]->instanceList;
            while (listPtr != NULL)
            {
                listPtr = listPtr->next;
                count++;
            }
        }
    }
    return count;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to register an object table
 */
//--------------------------------------------------------------------------------------------------
static bool RegisterObjTable
(
    lwm2mcore_Ref_t instanceRef,            ///< [IN] instance reference
    lwm2mcore_Handler_t* const handlerPtr,  ///< [IN] List of supported object/resource by client
    uint16_t* registeredObjNbPtr,           ///< [INOUT] Registered bject number
    bool clientTable                        ///< [IN] Indicate if the table is provided by the
                                            ///< client
)
{
    uint32_t i = 0;
    uint32_t j = 0;
    uint16_t ObjNb = 0;
    uint16_t objInstanceNb = 0;
    bool dmServerPresence = false;
    uint16_t securityObjectNumber;
    uint16_t serverObjectNumber;
    struct _lwm2mcore_objectsList *objectsListPtr = NULL;

    if ((NULL == handlerPtr) || (NULL == registeredObjNbPtr))
    {
        return false;
    }

    ObjNb = *registeredObjNbPtr;

    /* Check if a DM server was provided: only for static LwM2MCore case */
    omanager_GetBootstrapConfigObjectsNumber(&securityObjectNumber, &serverObjectNumber);
    LOG_ARG("securityObjectNumber %d, serverObjectNumber %d",
            securityObjectNumber, serverObjectNumber);

    if ((false == clientTable) && serverObjectNumber)
    {
        dmServerPresence = true;
    }

    LOG_ARG("dmServerPresence %d", dmServerPresence);

    /* Check if ObjectArray is large enough for all the objects */
    if (OBJ_COUNT < handlerPtr->objCnt + ObjNb)
    {
        return false;
    }

    /* Initialize all objects for Wakaama: handlerPtr */
    for (i = 0; i < (handlerPtr->objCnt); i++)
    {
        /* Memory allocation for one object */
        ObjectArray[ObjNb]  = (lwm2m_object_t *)lwm2m_malloc(sizeof(lwm2m_object_t));
        if (NULL != ObjectArray[ObjNb])
        {
            memset(ObjectArray[ObjNb], 0, sizeof(lwm2m_object_t));

            /* Assign the object ID */
            ObjectArray[ObjNb]->objID = (handlerPtr->objects + i)->id;
            objInstanceNb = (handlerPtr->objects + i)->maxObjInstCnt;

            /* Object 0: security */
            if (LWM2M_SECURITY_OBJECT_ID == ObjectArray[ObjNb]->objID)
            {
                objInstanceNb = securityObjectNumber;
            }

            /* Object 1: server */
            if (LWM2M_SERVER_OBJECT_ID == ObjectArray[ObjNb]->objID)
            {
                if (false == dmServerPresence)
                {
                    /* Do not create object instance for server object (no provisioned DM server)
                     * This means that a bootstrap connection will be initiated
                     */
                    objInstanceNb = LWM2MCORE_ID_NONE;
                }
                else
                {
                    objInstanceNb = serverObjectNumber;
                }
            }

            /* Object 2 case: check stored ACL configuration */
            if (LWM2M_ACL_OBJECT_ID == ObjectArray[ObjNb]->objID)
            {
                uint16_t object2InstanceNumber = omanager_GetObject2InstanceNumber();
                if (object2InstanceNumber >= 1)
                {
                    objInstanceNb = object2InstanceNumber;
                }
                else
                {
                    /* Consider that ACLs are not configured: single server */
                    objInstanceNb = LWM2MCORE_ID_NONE;
                }
            }

            LOG_ARG("Object Id %d, objInstanceNb %d", ObjectArray[ObjNb]->objID, objInstanceNb);

            if (LWM2MCORE_ID_NONE == objInstanceNb)
            {
                /* Unknown object instance count is always assumed to be multiple */
                LOG_ARG("Object with multiple instances oid %d", ObjectArray[ObjNb]->objID);
            }
            else if (1 < objInstanceNb)
            {
                lwm2m_list_t* instancePtr;
                ObjectArray[ObjNb]->instanceList =
                        (lwm2m_list_t *)lwm2m_malloc(sizeof(lwm2m_list_t));
                memset(ObjectArray[ObjNb]->instanceList, 0, sizeof(lwm2m_list_t));
                // Since ObjectArray[0] is already malloced, the following loop starts with 1
                for (j = 1; j < objInstanceNb; j++)
                {
                    /* Add the object instance in the Wakaama format */
                    instancePtr = (lwm2m_list_t *)lwm2m_malloc(sizeof(lwm2m_list_t));
                    if (!instancePtr)
                    {
                       LOG("instancePtr is NULL");
                       return false;
                    }
                    instancePtr->id = j;
                    ObjectArray[ObjNb]->instanceList =
                        LWM2M_LIST_ADD (ObjectArray[ObjNb]->instanceList,
                                        instancePtr);
                    /* instancePtr is released by omanager_ObjectsFree API */
                }

                for (j = 0; j < objInstanceNb; j++)
                {
                    if (NULL == lwm2m_list_find(ObjectArray[ObjNb]->instanceList, j))
                    {
                        LOG_ARG("Oid %d / oiid %d NOT present", ObjectArray[ObjNb]->objID, j);
                    }
                    else
                    {
                        LOG_ARG("Oid %d / oiid %d present", ObjectArray[ObjNb]->objID, j);
                    }
                }
            }
            else if (1 == objInstanceNb)
            {
                /* Allocate the unique object instance */
                ObjectArray[ObjNb]->instanceList =
                                            (lwm2m_list_t *)lwm2m_malloc(sizeof(lwm2m_list_t));
                if (ObjectArray[ObjNb]->instanceList != NULL)
                {
                    memset(ObjectArray[ObjNb]->instanceList, 0, sizeof(lwm2m_list_t));
                }
                else
                {
                    lwm2m_free(ObjectArray[ObjNb]);
                    return false;
                }

                if (NULL == lwm2m_list_find(ObjectArray[ObjNb]->instanceList, 0))
                {
                    LOG_ARG("oid %d / oiid %d NOT present", ObjectArray[ObjNb]->objID, 0);
                }
                else
                {
                    LOG_ARG("oid %d / oiid %d present", ObjectArray[ObjNb]->objID, 0);
                }
            }
            else
            {
                LOG_ARG("No instance to create in Wakaama for object %d",
                        ObjectArray[ObjNb]->objID);
            }

            if (objInstanceNb)
            {
                /*
                 * And the private function that will access the object.
                 * Those function will be called when a read/write/execute query is made by the
                 * server. In fact the library doesn't need to know the resources of the object,
                 * only the server does.
                 */
                ObjectArray[ObjNb]->readFunc     = ReadCb;
                ObjectArray[ObjNb]->discoverFunc = DiscoverCb;
                ObjectArray[ObjNb]->writeFunc    = WriteCb;
                ObjectArray[ObjNb]->executeFunc  = ExecuteCb;
                ObjectArray[ObjNb]->createFunc   = CreateCb;
                ObjectArray[ObjNb]->deleteFunc   = DeleteCb;

                /* Store the context */
                ObjectArray[ObjNb]->userData = instanceRef;
                ObjNb++;
            }
        }
    }

    /* Allocate object and resource linked to object/resource table provided by the client
     * This is used to make a link between the lwm2mcore_Handler_t provided by the client
     * and the lwm2m_object_t for Wakaama
     */
    objectsListPtr = GetObjectsList();
    InitObjectsList(objectsListPtr, handlerPtr);
    *registeredObjNbPtr = ObjNb;
    return true;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to notify Wakaama of supported object instance list for software and asset data
 *
 * @return
 *      - true if the list was successfully treated
 *      - else false
 */
//--------------------------------------------------------------------------------------------------
static bool UpdateObjectInstanceListWakaama
(
    lwm2mcore_Ref_t instanceRef,    ///< [IN] instance reference
    uint16_t        objectId        ///< [IN] Object Id
)
{
    char tempPath[LWM2MCORE_SW_OBJECT_INSTANCE_LIST_MAX_LEN + 1];
    bool updatedList = false;
    uint16_t oid;
    uint16_t oiid;
    uint16_t listLen = 0;
    ObjectInstanceList_t* objectInstanceListPtr;
    char* listBufferPtr;
    int numChars;
    char aOnePath[ ONE_PATH_MAX_LEN ];
    char prefix[ LWM2MCORE_NAME_LEN + 1];
    char* aData = NULL;
    char* cSavePtr;
    char* cSaveOnePathPtr = NULL;
    ObjectInstanceList_t* instancePtr;
    lwm2m_list_t* wakaamaInstancePtr;
    lwm2m_object_t* targetPtr;
    smanager_ClientData_t* dataPtr = (smanager_ClientData_t*) instanceRef;

    LOG_ARG("list len %d, objectId %d", strlen(SwObjectInstanceListPtr), objectId);

    switch (objectId)
    {
        case LWM2MCORE_SOFTWARE_UPDATE_OID:
            LOG_ARG("SwObjectInstanceListPtr %s", SwObjectInstanceListPtr);
            listLen = LWM2MCORE_SW_OBJECT_INSTANCE_LIST_MAX_LEN;
            listBufferPtr = SwObjectInstanceListPtr;
            objectInstanceListPtr = SwApplicationListPtr;
            break;

#ifdef LWM2M_OBJECT_33406
        case LWM2MCORE_FILE_LIST_OID:
            LOG_ARG("FileTransferObjectInstanceListPtr %s", FileTransferObjectInstanceListPtr);
            listLen = LWM2MCORE_FILE_TRANSFER_OBJECT_INSTANCE_LIST_MAX_LEN;
            listBufferPtr = FileTransferObjectInstanceListPtr;
            objectInstanceListPtr = FileTransferListPtr;
            break;
#endif

        default:
            return false;
    }

    if (NULL == dataPtr)
    {
        return false;
    }

    /* Treat the list:
     * All object instances of object Id needs to be registered in Wakaama
     */
    tempPath[0] = 0;
    numChars = snprintf(tempPath, listLen + 1, "%s", listBufferPtr);
    /* Check that the string is not truncated or any error */
    if ((numChars < 0) || (listLen < numChars))
    {
        LOG_ARG("Error on list: numChars %d", numChars);
        return false;
    }

    // Set all list entries to uncheck
    instancePtr = objectInstanceListPtr;
    while (NULL != instancePtr)
    {
        instancePtr->check = false;
        LOG("Set check false");
        instancePtr = instancePtr->nextPtr;
    }

    aData = strtok_r(tempPath, REG_PATH_END, &cSavePtr);
    while (NULL != aData)
    {
        memset(aOnePath, 0, sizeof(aOnePath));
        if (strlen(aData) < sizeof(aOnePath))
        {
            omanager_StrCopy(aOnePath, aData, sizeof(aOnePath));
        }
        else
        {
            LOG("String length of aData is greater than aOnePath!");
            return false;
        }

        /* Get the object instance string
         * The path format shall be
         *  </path(prefix)/ObjectId/InstanceId>,
         */
        aData = strtok_r(aOnePath, REG_PATH_SEPARATOR, &cSaveOnePathPtr);
        if (NULL != aData)
        {
            aData = strtok_r(NULL, REG_PATH_SEPARATOR, &cSaveOnePathPtr);
            if (NULL != aData)
            {
                memset(prefix, 0, sizeof(prefix));
                if (strlen(aData) < sizeof(prefix))
                {
                    omanager_StrCopy(prefix, aData, sizeof(prefix));
                }
                else
                {
                    LOG("String length of aData is greater than prefix!");
                    return false;
                }

                aData = strtok_r(NULL, REG_PATH_SEPARATOR, &cSaveOnePathPtr);
                if (NULL != aData)
                {
                    oid = atoi(aData);
                    aData = strtok_r(NULL, REG_PATH_SEPARATOR, &cSaveOnePathPtr);
                    /* check if aData is digit
                     * if yes, oiid is present
                     * else no oiid
                     */
                    if (NULL != aData)
                    {
                        oiid = atoi(aData);
                    }
                    else
                    {
                        oiid = LWM2MCORE_ID_NONE;
                    }

                    LOG_ARG("Object instance to add: /%s/%d/%d", prefix, oid, oiid);

                    /* Check if the object instance Id exist in Wakaama */
                    targetPtr = (lwm2m_object_t*)LWM2M_LIST_FIND(dataPtr->lwm2mHPtr->objectList,
                                                                 oid);
                    if (NULL != targetPtr)
                    {
                        LOG_ARG("Obj %d is registered, search instance %d", objectId, oiid);

                        instancePtr =
                            (ObjectInstanceList_t*)LWM2M_LIST_FIND(objectInstanceListPtr, oiid);

                        if (NULL == instancePtr)
                        {
                            // Object instance is not registered
                            instancePtr =
                                (ObjectInstanceList_t*)lwm2m_malloc(sizeof(ObjectInstanceList_t));
                            LOG_ARG("Obj instance %d is NOT registered", oiid);
                            if (!instancePtr)
                            {
                               LOG("instancePtr is NULL");
                               return false;
                            }
                            memset(instancePtr, 0, sizeof(ObjectInstanceList_t));
                            instancePtr->nextPtr = NULL;
                            instancePtr->oiid = oiid;
                            instancePtr->check = true;
                            objectInstanceListPtr =
                                (ObjectInstanceList_t*)LWM2M_LIST_ADD(objectInstanceListPtr,
                                                                     instancePtr);
                            updatedList = true;
                        }
                        else
                        {
                            instancePtr->check = true;
                        }
                    }
                    else
                    {
                        LOG("Obj 9 is not registered: AOTA is not possible");
                        return false;
                    }
                }
            }
        }
        aData = strtok_r(NULL, REG_PATH_END, &cSavePtr);
    }
    LOG_ARG("listBufferPtr %s", listBufferPtr);

    targetPtr = (lwm2m_object_t*)LWM2M_LIST_FIND(dataPtr->lwm2mHPtr->objectList, objectId);
    if (NULL != targetPtr)
    {
        LOG_ARG("Obj %d is registered", objectId);

        // Search if one or several object instance of objectId in objectInstanceListPtr need to be
        // added or removed in Wakaama.

        // Search in Wakaama list if object instance is in objectInstanceListPtr
        instancePtr = objectInstanceListPtr;
        while (NULL != instancePtr)
        {
            LOG_ARG("objectInstanceListPtr /%d/%d", objectId, instancePtr->oiid);
            if (NULL == LWM2M_LIST_FIND(targetPtr->instanceList, instancePtr->oiid))
            {
                LOG_ARG("Oiid %d not registered in Wakaama, check %d",
                        instancePtr->oiid, instancePtr->check);
                // Only add the object instance in Wakaama if check is true
                if (instancePtr->check)
                {
                    wakaamaInstancePtr = (lwm2m_list_t*)lwm2m_malloc(sizeof(lwm2m_list_t));
                    if (!wakaamaInstancePtr)
                    {
                       LOG("instancePtr is NULL");
                       return false;
                    }
                    memset(wakaamaInstancePtr, 0, sizeof(lwm2m_list_t));
                    wakaamaInstancePtr->id = instancePtr->oiid;
                    targetPtr->instanceList =
                            (lwm2m_list_t*)LWM2M_LIST_ADD(targetPtr->instanceList,
                                                          wakaamaInstancePtr);
                    updatedList = true;
                }
                instancePtr = instancePtr->nextPtr;
            }
            else
            {
                LOG_ARG("Oiid %d already registered in Wakaama --> check value %d",
                        instancePtr->oiid, instancePtr->check);
                if (false == instancePtr->check)
                {
                    ObjectInstanceList_t* appPtr;
                    LOG_ARG("Remove oiid %d from objectInstanceListPtr", instancePtr->oiid);
                    objectInstanceListPtr = (ObjectInstanceList_t*)LWM2M_LIST_RM(objectInstanceListPtr,
                                                                               instancePtr->oiid,
                                                                               &appPtr);
                    lwm2m_free(instancePtr);
                    instancePtr = objectInstanceListPtr;
                }
                else
                {
                    instancePtr = instancePtr->nextPtr;
                }
            }
        }

        // Search in objectInstanceListPtr list if object instance is not in Wakaama list
        wakaamaInstancePtr = targetPtr->instanceList;
        lwm2m_list_t* nextPtr;
        while (NULL != wakaamaInstancePtr)
        {
            LOG_ARG("wakaamaInstancePtr /%d/%d", objectId, wakaamaInstancePtr->id);

            nextPtr = wakaamaInstancePtr->next;

            if (NULL == LWM2M_LIST_FIND(objectInstanceListPtr, wakaamaInstancePtr->id))
            {
                LOG_ARG("Oiid %d not registered in objectInstanceListPtr --> remove in Wakaama",
                        wakaamaInstancePtr->id);
                targetPtr->instanceList =
                            LWM2M_LIST_RM(targetPtr->instanceList, wakaamaInstancePtr->id, NULL);
                lwm2m_free(wakaamaInstancePtr);
                updatedList = true;
            }
            else
            {
                LOG_ARG("Oiid %d already registered in objectInstanceListPtr --> keep it in Wakaama",
                        wakaamaInstancePtr->id);
            }

            wakaamaInstancePtr = nextPtr;
        }
    }

    // Send a registration update if the device is registered to the DM server
    if (updatedList)
    {
        omanager_UpdateRequest(instanceRef, LWM2M_REG_UPDATE_OBJECT_LIST);

        switch (objectId)
        {
            case LWM2MCORE_SOFTWARE_UPDATE_OID:
                SwApplicationListPtr = objectInstanceListPtr;
                break;

#ifdef LWM2M_OBJECT_33406
            case LWM2MCORE_FILE_LIST_OID:
                FileTransferListPtr = objectInstanceListPtr;
                break;
#endif
            default:
            break;
        }

    }
    return true;
}

//--------------------------------------------------------------------------------------------------
/**
 * search for a specific resource of object from the object table
 *
 * @return
 *      - lwm2mcore_Resource_t*  pointer for the found resource
 *      - NULL if the resource is not found
 */
//--------------------------------------------------------------------------------------------------
static lwm2mcore_Resource_t* SearchResource
(
    uint16_t objectId,                 ///< [IN] object identifier
    uint16_t resourceId               ///< [IN] resource identifier
)
{
    int i, j;
    lwm2mcore_Handler_t* lwm2mHandlersPtr = NULL;
    lwm2mcore_Resource_t* resourcePtr = NULL ;
    lwm2mHandlersPtr = omanager_GetHandlers();

    if (!lwm2mHandlersPtr)
    {
        return false;
    }

    // Search for the resource in the object table
    for (i = 0; i < (lwm2mHandlersPtr->objCnt); i++)
    {
        if (objectId == (lwm2mHandlersPtr->objects[i].id))
        {
            for (j = 0; j < (lwm2mHandlersPtr->objects[i].resCnt); j++)
            {
                if (resourceId == (lwm2mHandlersPtr->objects[i].resources[j].id))
                {
                    resourcePtr = &lwm2mHandlersPtr->objects[i].resources[j];
                    break;
                }
            }
        }
    }

    return resourcePtr;
}

//--------------------------------------------------------------------------------------------------
/**
 *                      PUBLIC FUNCTIONS
 */
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 * Read a resource from the object table
 *
 * @return
 *      - true if resource is found and read succeeded
 *      - else false
 */
//--------------------------------------------------------------------------------------------------
bool lwm2mcore_ResourceRead
(
    uint16_t objectId,                 ///< [IN] object identifier
    uint16_t objectInstanceId,         ///< [IN] object instance identifier
    uint16_t resourceId,               ///< [IN] resource identifier
    uint16_t resourceInstanceId,       ///< [IN] resource instance identifier
    char*    dataPtr,                  ///< [OUT] Array of requested resources to be read
    size_t*  dataSizePtr               ///< [IN/OUT] Size of the array
)
{
    lwm2mcore_Uri_t uri;
    lwm2mcore_Resource_t* resourcePtr = NULL;
    char asyncBuf[LWM2MCORE_BUFFER_MAX_LEN] = {0};
    size_t dataBufferSize;

    if ((!dataPtr) || (!dataSizePtr))
    {
        return false;
    }

    // Get the data buffer size
    dataBufferSize = *dataSizePtr;

    memset(&uri, 0, sizeof(uri));
    uri.oid = objectId;
    uri.oiid = objectInstanceId;
    uri.rid = resourceId;
    uri.riid = resourceInstanceId;
    uri.op = LWM2MCORE_OP_READ;

    resourcePtr = SearchResource(objectId, resourceId);

    if (!resourcePtr)
    {
        LOG("Requested ressource not found");
        return false;
    }

    if (!resourcePtr->read)
    {
        LOG("Requested resource cannot be read");
        return false;
    }

    // Execute the read function
    if (LWM2MCORE_ERR_COMPLETED_OK != resourcePtr->read(&uri, asyncBuf, dataSizePtr, NULL))
    {
        return false;
    }

    // Format result and store it in dataPtr
    switch (resourcePtr->type)
    {
        case LWM2MCORE_RESOURCE_TYPE_INT:
        case LWM2MCORE_RESOURCE_TYPE_TIME:
        {
            int64_t value;
            value = omanager_BytesToInt(asyncBuf, *dataSizePtr);
            *dataSizePtr = snprintf(dataPtr, dataBufferSize - 1, "%lld", (long long)value);
        }
        break;

        case LWM2MCORE_RESOURCE_TYPE_BOOL:
            *dataSizePtr = snprintf(dataPtr, dataBufferSize - 1, "%d", asyncBuf[0]);
            break;

        case LWM2MCORE_RESOURCE_TYPE_FLOAT:
        {
            double value;
            memcpy(&value, asyncBuf, sizeof(double));
            *dataSizePtr = snprintf(dataPtr, dataBufferSize - 1, "%lf", value);
        }
        break;

        default:
            memcpy(dataPtr, asyncBuf, *dataSizePtr);
            break;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
/**
 * Write a resource from the object table
 *
 * @return
 *      - true if resource is found and read succeeded
 *      - else false
 */
//--------------------------------------------------------------------------------------------------
bool lwm2mcore_ResourceWrite
(
    uint16_t objectId,                 ///< [IN] object identifier
    uint16_t objectInstanceId,         ///< [IN] object instance identifier
    uint16_t resourceId,               ///< [IN] resource identifier
    uint16_t resourceInstanceId,       ///< [IN] resource instance identifier
    char*    dataPtr,                  ///< [IN] Array of requested resources to be write
    size_t*  dataSizePtr               ///< [IN/OUT] Size of the array
)
{
    lwm2mcore_Uri_t uri;
    lwm2mcore_Resource_t* resourcePtr = NULL;

    // add condition to restrict the use of this function only for object 5 (Fwupdate)
    if ((!dataPtr) || (!dataSizePtr) || (objectId != LWM2MCORE_FIRMWARE_UPDATE_OID))
    {
        return false;
    }

    memset(&uri, 0, sizeof(uri));
    uri.oid = objectId;
    uri.oiid = objectInstanceId;
    uri.rid = resourceId;
    uri.riid = resourceInstanceId;
    uri.op = LWM2MCORE_OP_WRITE;

    resourcePtr = SearchResource(objectId, resourceId);

    if (!resourcePtr)
    {
        LOG("Requested ressource not found");
        return false;
    }

    if (!resourcePtr->write)
    {
        LOG("Requested resource cannot be write");
        return false;
    }

    // Execute the write function
    LOG_ARG("Execute the write function in resourceid : %u ", resourcePtr->id);
    resourcePtr->type = LWM2MCORE_RESOURCE_TYPE_STRING;
    resourcePtr->maxResInstCnt = 1;
    if (LWM2MCORE_ERR_COMPLETED_OK != resourcePtr->write(&uri, dataPtr, *dataSizePtr))
    {
        return false;
    }

    return true;
}
//--------------------------------------------------------------------------------------------------
/**
 * execute a resource from the object table
 *
 * @return
 *      - true if resource is found and read succeeded
 *      - else false
 */
//--------------------------------------------------------------------------------------------------
bool lwm2mcore_ResourceExec
(
    uint16_t objectId,                 ///< [IN] object identifier
    uint16_t objectInstanceId,         ///< [IN] object instance identifier
    uint16_t resourceId,               ///< [IN] resource identifier
    uint16_t resourceInstanceId,       ///< [IN] resource instance identifier
    char*    dataPtr,                  ///< [IN] Array of requested resources to be write
    size_t*  dataSizePtr               ///< [IN/OUT] Size of the array
)
{
    // add condition to restrict the use of this function only for object 5 (Fwupdate)
    if (objectId != LWM2MCORE_FIRMWARE_UPDATE_OID)
    {
        return false;
    }

    lwm2mcore_Uri_t uri;
    lwm2mcore_Resource_t* resourcePtr = NULL;

    memset(&uri, 0, sizeof(uri));
    uri.oid = objectId;
    uri.oiid = objectInstanceId;
    uri.rid = resourceId;
    uri.riid = resourceInstanceId;
    uri.op = LWM2MCORE_OP_EXECUTE;

    resourcePtr = SearchResource(objectId, resourceId);

    if (!resourcePtr)
    {
        LOG("Requested ressource not found");
        return false;
    }

    if (!resourcePtr->exec)
    {
        LOG("Requested resource cannot be executed");
        return false;
    }

    // call to esecute function
    LOG_ARG("Execute function in resourceid : %u ", resourcePtr->id);
    resourcePtr->maxResInstCnt = 1;
    if (LWM2MCORE_ERR_COMPLETED_OK != resourcePtr->exec(&uri, dataPtr, *dataSizePtr))
    {
        return false;
    }

    return true;
}
//--------------------------------------------------------------------------------------------------
/**
 * Register the object table and service API
 *
 * @note If handlerPtr parameter is NULL, LwM2MCore registers it's own "standard" object list
 *
 * @return
 *      - number of registered objects
 */
//--------------------------------------------------------------------------------------------------
uint16_t lwm2mcore_ObjectRegister
(
    lwm2mcore_Ref_t instanceRef,             ///< [IN] instance reference
    char* endpointPtr,                       ///< [IN] Device endpoint
    lwm2mcore_Handler_t* const handlerPtr,   ///< [IN] List of supported object/resource by client
    void * const servicePtr                  ///< [IN] Client service API table
)
{
    bool result;
    lwm2mcore_Handler_t* lwm2mcoreHandlersPtr;

    RegisteredObjNb = 0;

    if (!instanceRef)
    {
        LOG("Null instance reference");
        return RegisteredObjNb;
    }

    /* For the moment, servicePtr can be NULL */
    (void)servicePtr;
    if (!endpointPtr)
    {
        LOG("param error");
        return RegisteredObjNb;
    }

    smanager_ClientData_t* dataPtr = (smanager_ClientData_t*)instanceRef;
    LOG_ARG("lwm2mcore_ObjectRegister RegisteredObjNb %d", RegisteredObjNb);

    /* Read the bootstrap configuration file */
    if (false == omanager_LoadBootstrapConfigurationFile())
    {
        /* If the file is not present:
         * Delete DM credentials to force a connection to the bootstrap server
         * Then the configuration file will be created at the end of the bootstrap procedure
         */
        omanager_DeleteDmCredentials();
    }

    /* Read the ACL configuration file */
    if (false == omanager_LoadAclConfiguration())
    {
        LOG("ERROR on reading ACL configuration -> set default");
    }

    lwm2mcoreHandlersPtr = omanager_GetHandlers();

    /* Register static object tables managed by LwM2MCore */
    result = RegisterObjTable(instanceRef, lwm2mcoreHandlersPtr, &RegisteredObjNb, false);
    if (false == result)
    {
        RegisteredObjNb = 0;
        LOG("ERROR on registering LwM2MCore object table");
        return RegisteredObjNb;
    }

    if (NULL != handlerPtr)
    {
        LOG("Register client object list");
        /* Register object tables filled by the client */
        result = RegisterObjTable(instanceRef, handlerPtr, &RegisteredObjNb, true);
        if (result == false)
        {
            RegisteredObjNb = 0;
            LOG("ERROR on registering client object table");
            return RegisteredObjNb;
        }
    }
    else
    {
        LOG("Only register LwM2MCore object list");
    }

    if (true == result)
    {
        int test = 0;
        /* Save the security object list in the context (used for connection) */
        dataPtr->securityObjPtr = ObjectArray[LWM2M_SECURITY_OBJECT_ID];

        /* Wakaama configuration and the object registration */
        LOG_ARG("RegisteredObjNb %d", RegisteredObjNb);
        test = lwm2m_configure(dataPtr->lwm2mHPtr,
                               endpointPtr,
                               NULL,
                               NULL,
                               RegisteredObjNb,
                               ObjectArray);
        if (test != COAP_NO_ERROR)
        {
            LOG_ARG("Failed to configure LwM2M client: test %d", test);
            RegisteredObjNb = 0;
        }
        else
        {
            LOG("configure LwM2M client OK");
        }

        // Check if some software object instance exist
        UpdateObjectInstanceListWakaama(instanceRef, LWM2MCORE_SOFTWARE_UPDATE_OID);
#ifdef LWM2M_OBJECT_33406
        UpdateObjectInstanceListWakaama(instanceRef, LWM2MCORE_FILE_LIST_OID);
#endif /* LWM2M_OBJECT_33406 */
    }
    LOG_ARG("Number of registered objects: %u", RegisteredObjNb);

    return RegisteredObjNb;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to notify LwM2MCore of supported object instance list for software and asset data
 *
 * @return
 *      - true if the list was successfully treated
 *      - else false
 */
//--------------------------------------------------------------------------------------------------
bool lwm2mcore_UpdateSwList
(
    lwm2mcore_Ref_t instanceRef,    ///< [IN] Instance reference (Set to NULL if this API is used if
                                    ///< lwm2mcore_init API was no called)
    const char* listPtr,            ///< [IN] Formatted list
    size_t listLen                  ///< [IN] Size of the update list
)
{
    int numChars;

    if ((LWM2MCORE_SW_OBJECT_INSTANCE_LIST_MAX_LEN < listLen)
     || (NULL == listPtr))
    {
        return false;
    }
    // store the string
    numChars = snprintf(SwObjectInstanceListPtr,
                        LWM2MCORE_SW_OBJECT_INSTANCE_LIST_MAX_LEN + 1,
                        "%s",
                        listPtr);
    /* Check that the string is not truncated or any error */
    if ((numChars < 0) || (LWM2MCORE_SW_OBJECT_INSTANCE_LIST_MAX_LEN < numChars))
    {
        LOG_ARG("Error on list: numChars %d", numChars);
        return false;
    }

    if (NULL == instanceRef)
    {
        return true;
    }
    return UpdateObjectInstanceListWakaama(instanceRef, LWM2M_SOFTWARE_UPDATE_OBJECT_ID);
}

#ifdef LWM2M_OBJECT_33406
//--------------------------------------------------------------------------------------------------
/**
 * Function to notify LwM2MCore of supported object instance list for file transfer
 *
 * @return
 *      - true if the list was successfully treated
 *      - else false
 */
//--------------------------------------------------------------------------------------------------
bool lwm2mcore_UpdateFileTransferList
(
    lwm2mcore_Ref_t instanceRef,    ///< [IN] Instance reference (Set to NULL if this API is used if
                                    ///< lwm2mcore_init API was no called)
    const char* listPtr,            ///< [IN] Formatted list
    size_t listLen                  ///< [IN] Size of the update list
)
{
    int numChars;
    LOG("lwm2mcore_UpdateFileTransferList");

    if ((LWM2MCORE_FILE_TRANSFER_OBJECT_INSTANCE_LIST_MAX_LEN < listLen)
     || (NULL == listPtr))
    {
        return false;
    }
    // store the string
    numChars = snprintf(FileTransferObjectInstanceListPtr,
                        LWM2MCORE_FILE_TRANSFER_OBJECT_INSTANCE_LIST_MAX_LEN + 1,
                        "%s",
                        listPtr);
    /* Check that the string is not truncated or any error */
    if ((numChars < 0) || (LWM2MCORE_FILE_TRANSFER_OBJECT_INSTANCE_LIST_MAX_LEN < numChars))
    {
        LOG_ARG("Error on list: numChars %d", numChars);
        return false;
    }

    if (NULL == instanceRef)
    {
        return true;
    }
    return UpdateObjectInstanceListWakaama(instanceRef, LWM2MCORE_FILE_LIST_OID);
}
#endif /* LWM2M_OBJECT_33406 */

//--------------------------------------------------------------------------------------------------
/**
 * Function to read the lifetime from the server object.
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_INVALID_STATE if no device management server are configured
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_GetLifetime
(
    uint32_t* lifetimePtr           ///< [OUT] Lifetime in seconds
)
{
    return omanager_GetLifetime(lifetimePtr);
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to set the lifetime in the server object and save to platform storage.
 *
 * @return
 *      - LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *      - LWM2MCORE_ERR_INCORRECT_RANGE if the lifetime is not correct
 *      - LWM2MCORE_ERR_INVALID_STATE if no device management server are configured
 *      - LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 */
//--------------------------------------------------------------------------------------------------
lwm2mcore_Sid_t lwm2mcore_SetLifetime
(
    uint32_t lifetime               ///< [IN] Lifetime in seconds
)
{
    LOG_ARG("lwm2mcore_SetLifetime %d sec", lifetime);
    return omanager_SetLifetime(lifetime, true);
}
