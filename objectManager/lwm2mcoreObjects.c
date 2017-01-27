/**
 * @file lwm2mcoreObjects.c
 *
 * Adaptation layer from the object table managed by the client and the Wakaama object management
 *
 * Copyright (C) Sierra Wireless Inc. Use of this work is subject to license.
 *
 */

/* include files */
#include "liblwm2m.h"
#include "lwm2mcore.h"
#include "../objectManager/lwm2mcoreObjects.h"
#include "../inc/lwm2mcoreObjectHandler.h"
#include "../os/osDebug.h"
#include "../sessionManager/lwm2mcoreSessionParam.h"
#include "lwm2mcorePortSecurity.h"
#include "internals.h"

//--------------------------------------------------------------------------------------------------
/**
 * Maximum number of objects which can be registered in Wakaama
 */
//--------------------------------------------------------------------------------------------------
#define OBJ_COUNT 10

//--------------------------------------------------------------------------------------------------
/**
 * Objects number which are registered in Wakaama
 */
//--------------------------------------------------------------------------------------------------
uint16_t RegisteredObjNb = 0;

//--------------------------------------------------------------------------------------------------
/**
 * Object array to be registered in Wakamaa including the genereic handlers to access to these
 * objects
 */
//--------------------------------------------------------------------------------------------------
static lwm2m_object_t * ObjectArray[OBJ_COUNT];

//--------------------------------------------------------------------------------------------------
/**
 * Client defined handlers
 */
//--------------------------------------------------------------------------------------------------
extern lwm2mcore_handler_t Lwm2mcoreHandlers;

//--------------------------------------------------------------------------------------------------
/**
 * LWM2M core context
 */
//--------------------------------------------------------------------------------------------------
extern lwm2mcore_context_t* Lwm2mcoreCtxPtr;

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
 *      - object pointer  if the object is found
 *      - NULL  if the object is not found
 */
//--------------------------------------------------------------------------------------------------
static uint8_t setCoapError
(
    int sid,                            ///< [IN] resource handler status
    lwm2mcore_op_type_t operation       ///< [IN] operation
)
{
    uint8_t result = COAP_503_SERVICE_UNAVAILABLE;
    switch (sid)
    {
        case LWM2MCORE_ERR_COMPLETED_OK:
        {
            if (LWM2MCORE_OP_READ == operation)
            {
                result = COAP_205_CONTENT;
            }
            else if ((LWM2MCORE_OP_WRITE == operation) || (LWM2MCORE_OP_EXECUTE == operation))
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

        default:
        {
            result = COAP_500_INTERNAL_SERVER_ERROR;
        }
        break;
    }
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
static lwm2mcore_internal_object_t* findObject
(
    lwm2mcore_context_t* ctxPtr,            ///< [IN] LWM2M core context
    uint16_t oid                            ///< [IN] object ID to find
)
{
    lwm2mcore_internal_object_t *obj = NULL;

    for (obj = DLIST_FIRST(&(ctxPtr->objects_list)); obj; obj = DLIST_NEXT(obj, list))
    {
        if (obj->id == oid)
            break;
    }

    return obj;
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
static lwm2mcore_internal_resource_t* findResource
(
    lwm2mcore_internal_object_t* objPtr,    ///< [IN] Object pointer
    uint16_t rid                            ///< [IN] resource ID
)
{
    lwm2mcore_internal_resource_t* resourcePtr = NULL;

    OS_ASSERT(objPtr);

    for (resourcePtr = DLIST_FIRST(&(objPtr->resource_list));
         resourcePtr;
         resourcePtr = DLIST_NEXT(resourcePtr, list))
    {
        if (resourcePtr->id == rid)
           break;
    }

    return resourcePtr;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to convert bytes(in network byte order) to unsigned 16 bits integer
 *
 * @return
 *      - converted data
 */
//--------------------------------------------------------------------------------------------------
static uint16_t bytesToUint16
(
    const uint8_t* bytesPtr     ///< [IN] bytes the buffer contains data to be converted
)
{
    return ((bytesPtr[0] << 8) | bytesPtr[1]);
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to convert bytes(in network byte order) to unsigned 32 bits integer
 *
 * @return
 *      - converted data
 */
//--------------------------------------------------------------------------------------------------
static uint32_t bytesToUint32
(
    const uint8_t* bytesPtr
)
{
    return ((bytesPtr[0] << 24) | (bytesPtr[1] << 16) | (bytesPtr[2] << 8) | bytesPtr[3]);
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to convert bytes(in network byte order) to unsigned 64 bits integer
 *
 * @return
 *      - converted data
 */
//--------------------------------------------------------------------------------------------------
static uint64_t bytesToUint64
(
    const uint8_t* bytesPtr
)
{
    return (((uint64_t)bytesToUint32(bytesPtr) << 32)
            | bytesToUint32(bytesPtr + 4));
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to convert bytes(in network byte order) to integer
 *
 * @return
 *      - converted data
 */
//--------------------------------------------------------------------------------------------------
inline int64_t bytesToInt
(
    const uint8_t* bytesPtr,
    size_t len
)
{
    int64_t value;

    switch(len)
    {
        case 1:
        {
            value = *bytesPtr;
        }
        break;

        case 2:
        {
            value = (int16_t)bytesToUint16(bytesPtr);
        }
        break;

        case 4:
        {
            value = (int32_t)bytesToUint32(bytesPtr);
        }
        break;

        case 8:
        {
            value = (int64_t)bytesToUint64(bytesPtr);
        }
        break;

        default:
        {
            value = -1;
        }
        break;
    }

    return value;
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
static uint8_t readCb
(
    uint16_t instanceId,            ///< [IN] Object ID
    int* numDataPtr,                ///< [IN] Number of resources to be read
    lwm2m_data_t ** dataArrayPtr,   ///< [IN] Array of requested resources to be read
    lwm2m_object_t * objectPtr      ///< [IN] Pointer on object
)
{
    uint8_t result;
    int i;

    LOG_ARG("readCb oid %d oiid %d", objectPtr->objID, instanceId);

    /* Search if the object was registered */
    if (LWM2M_LIST_FIND(objectPtr->instanceList, instanceId))
    {
        lwm2mcore_uri_t uri;
        lwm2mcore_internal_object_t *objPtr;
        LOG("object instance Id was registered");

        memset(&uri, 0, sizeof (lwm2mcore_uri_t));

        uri.op = LWM2MCORE_OP_READ;
        uri.oid = objectPtr->objID;
        uri.oiid = instanceId;

        objPtr = findObject(Lwm2mcoreCtxPtr, objectPtr->objID);
        if (NULL == objPtr)
        {
            LOG_ARG("Object %d is NOT registered", objectPtr->objID);
            result = COAP_404_NOT_FOUND;
        }
        else
        {
            int sid = 0;
            lwm2mcore_internal_resource_t*resourcePtr = NULL;
            char async_buf[LWM2MCORE_BUFFER_MAX_LEN];
            size_t async_buf_len = LWM2MCORE_BUFFER_MAX_LEN;

            LOG_ARG("numDataP %d", *numDataPtr);
            // is the server asking for the full object ?
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
                    resList[ i ] = resourcePtr->id;
                    LOG_ARG("resList[ %d ] %d", i, resList[ i ]);
                    i++;
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
                resourcePtr = findResource(objPtr, uri.rid);
                if (NULL != resourcePtr)
                {
                    if (NULL != resourcePtr->read)
                    {
                        async_buf_len = LWM2MCORE_BUFFER_MAX_LEN;
                        memset( async_buf, 0, async_buf_len);
                        LOG_ARG("READ / %d / %d / %d", uri.oid, uri.oiid, uri.rid);
                        sid  = resourcePtr->read(&uri,
                                                async_buf,
                                                &async_buf_len,
                                                NULL);

                        /* Define the CoAP result */
                        result = setCoapError(sid, LWM2MCORE_OP_READ);

                        if (COAP_205_CONTENT == result)
                        {
                            switch (resourcePtr->type)
                            {
                                case LWM2MCORE_RESOURCE_TYPE_INT:
                                {
                                    int64_t value = 0;
                                    value = bytesToInt((uint8_t*)async_buf, async_buf_len);
                                    lwm2m_data_encode_int(value, (*dataArrayPtr) + i);
                                    if (LWM2MCORE_SECURITY_OID != uri.oid)
                                    {
                                        LOG_ARG("readCb sID %d value %d", sid, value);
                                    }
                                    else
                                    {
                                        LOG_ARG("readCb sID %d", sid);
                                    }
                                }
                                break;

                                case LWM2MCORE_RESOURCE_TYPE_BOOL:
                                {
                                    lwm2m_data_encode_bool(async_buf[0], (*dataArrayPtr) + i);
                                    if (LWM2MCORE_SECURITY_OID != uri.oid)
                                    {
                                        LOG_ARG("readCb sID %d value %d", sid, async_buf[0]);
                                    }
                                    else
                                    {
                                        LOG_ARG("readCb sID %d", sid);
                                    }
                                }
                                break;

                                case LWM2MCORE_RESOURCE_TYPE_STRING:
                                {
                                    lwm2m_data_encode_nstring(async_buf,
                                                              async_buf_len,
                                                              (*dataArrayPtr) + i);
                                    if (LWM2MCORE_SECURITY_OID != uri.oid)
                                    {
                                        LOG_ARG("readCb sID %d async_buf %s", sid, async_buf);
                                    }
                                    else
                                    {
                                        LOG_ARG("readCb sID %d", sid);
                                    }
                                }
                                break;

                                case LWM2MCORE_RESOURCE_TYPE_OPAQUE:
                                {
                                    lwm2m_data_encode_opaque(async_buf,
                                                             async_buf_len,
                                                             (*dataArrayPtr) + i);
                                    if (LWM2MCORE_SECURITY_OID != uri.oid)
                                    {
                                        LOG_ARG("readCb sID %d async_buf %s", sid, async_buf);
                                    }
                                    else
                                    {
                                        LOG_ARG("readCb sID %d", sid);
                                    }
                                }
                                break;

                                case LWM2MCORE_RESOURCE_TYPE_FLOAT:
                                {
                                    result = COAP_500_INTERNAL_SERVER_ERROR;
                                }
                                break;

                                case LWM2MCORE_RESOURCE_TYPE_TIME:
                                {
                                    int64_t value = 0;
                                    value = bytesToInt((uint8_t*)async_buf, async_buf_len);
                                    lwm2m_data_encode_int(value, (*dataArrayPtr) + i);
                                    if (LWM2MCORE_SECURITY_OID != uri.oid)
                                    {
                                        LOG_ARG("readCb sID %d value %d", sid, value);
                                    }
                                    else
                                    {
                                        LOG_ARG("readCb sID %d", sid);
                                    }
                                }
                                break;

                                case LWM2MCORE_RESOURCE_TYPE_UNKNOWN:
                                {
                                    lwm2m_data_encode_opaque(async_buf,
                                                             async_buf_len,
                                                             (*dataArrayPtr) + i);
                                    if (LWM2MCORE_SECURITY_OID != uri.oid)
                                    {
                                        LOG_ARG("readCb sID %d async_buf %s", sid, async_buf);
                                    }
                                    else
                                    {
                                        LOG_ARG("readCb sID %d", sid);
                                    }
                                }
                                break;

                                default:
                                {
                                    result = COAP_500_INTERNAL_SERVER_ERROR;
                                }
                                break;
                            }
                        }
                    }
                    else
                    {
                        LOG("READ callback NULL");
                        result = COAP_404_NOT_FOUND;
                    }
                }
                else
                {
                    LOG("resource NULL");
                    result = COAP_404_NOT_FOUND;
                }
                i++;
            } while ((i < *numDataPtr) && ((COAP_205_CONTENT == result)
                  || (COAP_NO_ERROR == result)));
        }
    }
    else
    {
        LOG_ARG("Object %d not found", objectPtr->objID);
        result = COAP_404_NOT_FOUND;
    }
    LOG_ARG("readCb result %d", result);
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
static uint8_t writeCb
(
    uint16_t instanceId,            ///< [IN] Object ID
    int numData,                    ///< [IN] Number of resources to be written
    lwm2m_data_t* dataArrayPtr,     ///< [IN] Array of requested resources to be written
    lwm2m_object_t* objectPtr       ///< [IN] Pointer on object
)
{
    uint8_t result;
    int i;

    LOG_ARG("writeCb oid %d oiid %d", objectPtr->objID, instanceId);

    /* Search if the object was registered */
    if (LWM2M_LIST_FIND(objectPtr->instanceList, instanceId))
    {
        lwm2mcore_uri_t uri;
        lwm2mcore_internal_object_t *objPtr;
        LOG("object instance Id was registered");

        memset( &uri, 0, sizeof (lwm2mcore_uri_t));

        uri.op = LWM2MCORE_OP_WRITE;
        uri.oid = objectPtr->objID;
        uri.oiid = instanceId;

        objPtr = findObject(Lwm2mcoreCtxPtr, objectPtr->objID);
        if (NULL == objPtr)
        {
            LOG_ARG("Object %d is NOT registered", objectPtr->objID);
            result = COAP_404_NOT_FOUND;
        }
        else
        {
            int sid = 0;
            lwm2mcore_internal_resource_t* resourcePtr = NULL;
            char async_buf[LWM2MCORE_BUFFER_MAX_LEN];
            size_t async_buf_len = LWM2MCORE_BUFFER_MAX_LEN;

            LOG_ARG("numData %d", numData);
            // is the server asking for the full object ?
            if (0 == numData)
            {
                uint16_t resList[50];
                int nbRes = 0;

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
                memset(async_buf, 0, LWM2MCORE_BUFFER_MAX_LEN);
                async_buf_len = LWM2MCORE_BUFFER_MAX_LEN;

                /* Search the resource handler */
                resourcePtr = findResource(objPtr, uri.rid);
                if (NULL != resourcePtr)
                {
                    if (NULL != resourcePtr->write)
                    {
                        LOG_ARG("data type %d", dataArrayPtr[i].type);

                        switch (dataArrayPtr[i].type)
                        {
                            case LWM2M_TYPE_STRING:
                            {
                                LOG("writeCb string");
                                if (dataArrayPtr[i].value.asBuffer.length <= \
                                                LWM2MCORE_BUFFER_MAX_LEN)
                                {
                                    memcpy(async_buf,
                                           dataArrayPtr[i].value.asBuffer.buffer,
                                           dataArrayPtr[i].value.asBuffer.length);
                                    async_buf_len = dataArrayPtr[i].value.asBuffer.length;
                                }
                            }
                            break;

                            case LWM2M_TYPE_OPAQUE:
                            {
                                LOG("writeCb opaque");
                                if (dataArrayPtr[i].value.asBuffer.length <= \
                                                    LWM2MCORE_BUFFER_MAX_LEN)
                                {
                                    memcpy(async_buf,
                                           dataArrayPtr[i].value.asBuffer.buffer,
                                           dataArrayPtr[i].value.asBuffer.length);
                                    async_buf_len = dataArrayPtr[i].value.asBuffer.length;
                                }
                            }
                            break;

                            case LWM2M_TYPE_INTEGER:
                            {
                                int64_t value = 0;
                                LOG("writeCb integer");
                                if (0 == lwm2m_data_decode_int(dataArrayPtr+i, &value))
                                {
                                    LOG("integer decode ERROR");
                                }
                                else
                                {
                                    LOG_ARG("writeCb integer %d", value);
                                    snprintf(async_buf, LWM2MCORE_BUFFER_MAX_LEN, "%d", value);
                                }
                            }
                            break;

                            case LWM2M_TYPE_FLOAT:
                            {
                                LOG("writeCb float");
                            }
                            break;

                            case LWM2M_TYPE_BOOLEAN:
                            {
                                bool value = false;
                                LOG("writeCb bool");
                                if (0 == lwm2m_data_decode_bool(dataArrayPtr+i, &value))
                                {
                                    LOG("bool decode ERROR");
                                }
                                else
                                {
                                    LOG_ARG("writeCb bool %d", value);
                                }
                            }
                            break;

                            default:
                            break;
                        }
                        LOG_ARG("WRITE / %d / %d / %d", uri.oid, uri.oiid, uri.rid);
                        sid  = resourcePtr->write(&uri,
                                                  async_buf,
                                                  async_buf_len);
                        LOG_ARG("WRITE sID %d", sid);
                        /* Define the CoAP result */
                        result = setCoapError(sid, LWM2MCORE_OP_WRITE);
                    }
                    else
                    {
                        LOG("READ callback NULL");
                        result = COAP_404_NOT_FOUND;
                    }
                }
                else
                {
                    LOG("resource NULL");
                    result = COAP_404_NOT_FOUND;
                }
                i++;
            } while ((i < numData) && ((COAP_204_CHANGED == result) || (COAP_NO_ERROR == result)));
        }
    }
    else
    {
        LOG_ARG("Object %d not found", objectPtr->objID);
        result = COAP_404_NOT_FOUND;
    }
    LOG_ARG("writeCb result %d", result);
    return result;
}

//--------------------------------------------------------------------------------------------------
/**
 * Delete an oject instance in the Wakaama format
 *
 * @return
 *      - COAP_404_NOT_FOUND if the object instance does not exist
 *      - COAP_202_DELETED if the request is well treated
 */
//--------------------------------------------------------------------------------------------------
static uint8_t deleteObjInstance
(
    uint16_t id,                    ///< [IN] Object instance ID
    lwm2m_object_t* objectPtr       ///< [IN] Pointer on object
)
{
    lwm2m_list_t* instancePtr;
    LOG("Enter");
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
static uint8_t createCb
(
    uint16_t instanceId,            ///< [IN] Object ID
    int numData,                    ///< [IN] Number of resources to be written
    lwm2m_data_t* dataArrayPtr,     ///< [IN] Array of requested resources to be written
    lwm2m_object_t* objectPtr       ///< [IN] Pointer on object
)
{
    uint8_t result;
    bool instanceCreated = false;
    LOG_ARG("createCb oid %d oiid %d", objectPtr->objID, instanceId);

    if (NULL == objectPtr->instanceList)
    {
        LOG("objectPtr->instanceList == NULL");
        objectPtr->instanceList = (lwm2m_list_t *)lwm2m_malloc(sizeof(lwm2m_list_t));
        if (NULL != objectPtr->instanceList)
        {
            memset(objectPtr->instanceList, 0, sizeof(lwm2m_list_t));
            instanceCreated = true;
        }
    }
    else
        LOG("objectPtr->instanceList != NULL");

    /* Search if the object was registered */
    if ((LWM2M_LIST_FIND(objectPtr->instanceList, instanceId) == NULL)
     || instanceCreated)
    {
        lwm2m_list_t* instancePtr;
        /* Add the object instance in the Wakaama format */
        instancePtr = (lwm2m_list_t *)lwm2m_malloc(sizeof(lwm2m_list_t));
        instancePtr->id = instanceId;
        objectPtr->instanceList = LWM2M_LIST_ADD(objectPtr->instanceList, instancePtr);

        result = writeCb(instanceId, numData, dataArrayPtr, objectPtr);
        if (COAP_204_CHANGED != result)
        {
            LOG_ARG("createCb --> delete oiid %d", instanceId);
            (void)deleteObjInstance(instanceId, objectPtr);
            result = COAP_500_INTERNAL_SERVER_ERROR;
        }
        else
        {
            result = COAP_201_CREATED;
        }
    }
    else
    {
        LOG("Object instance already exists");
        result = COAP_400_BAD_REQUEST;
    }
    LOG_ARG("createCb result %d", result);
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
static uint8_t discoverCb
(
    uint16_t instanceId,            ///< [IN] Object ID
    int* numDataPtr,                ///< [INOUT] Number of resources which were read
    lwm2m_data_t ** dataArrayPtr,   ///< [IN] Array of requested resources to be discovered
    lwm2m_object_t* objectPtr       ///< [IN] Pointer on object
)
{
    return 0;
}

//--------------------------------------------------------------------------------------------------
/**
 * Generic function when a EXECUTE command is treated for a specific object (Wakaama)
 *
 * @return
 *      - 0
 */
//--------------------------------------------------------------------------------------------------
static uint8_t executeCb
(
    uint16_t instanceId,            ///< [IN] Object ID
    uint16_t resourceId,            ///< [IN] Resource ID
    uint8_t* bufferPtr,             ///< [IN] Data provided in the EXECUTE command
    int length,                     ///< [IN] Data length
    lwm2m_object_t* objectPtr       ///< [IN] Pointer on object
)
{
    uint8_t result;

    LOG_ARG("ExecuteCb oid %d oiid %d rid %d", objectPtr->objID, instanceId, resourceId);

    /* Search if the object was registered */
    if (LWM2M_LIST_FIND(objectPtr->instanceList, instanceId))
    {
        lwm2mcore_uri_t uri;
        lwm2mcore_internal_object_t *objPtr;
        LOG("object instance Id was registered");

        memset(&uri, 0, sizeof (lwm2mcore_uri_t));

        uri.op = LWM2MCORE_OP_EXECUTE;
        uri.oid = objectPtr->objID;
        uri.oiid = instanceId;
        uri.rid = resourceId;

        objPtr = findObject(Lwm2mcoreCtxPtr, objectPtr->objID);
        if (NULL == objPtr)
        {
            LOG_ARG("Object %d is NOT registered", objectPtr->objID);
            result = COAP_404_NOT_FOUND;
        }
        else
        {
            int sid = 0;
            lwm2mcore_internal_resource_t* resourcePtr = NULL;
            size_t len = (size_t) length;

            /* Search the resource handler */
            resourcePtr = findResource(objPtr, uri.rid);
            if (NULL != resourcePtr)
            {
                if (NULL != resourcePtr->exec)
                {
                    LOG_ARG("EXECUTE / %d / %d / %d", uri.oid, uri.oiid, uri.rid);
                    sid  = resourcePtr->exec(&uri,
                                            bufferPtr,
                                            len);
                    LOG_ARG("EXECUTE sID %d", sid);
                    /* Define the CoAP result */
                    result = setCoapError(sid, LWM2MCORE_OP_EXECUTE);
                }
                else
                {
                    LOG("EXECUTE callback NULL");
                    result = COAP_404_NOT_FOUND;
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
struct _lwm2mcore_objects_list* getObjectsList
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
 * Initialize lwm2m object
 *
 * @return
 *      - object pointer
 */
//--------------------------------------------------------------------------------------------------
static lwm2mcore_internal_object_t* initObject
(
    lwm2mcore_object_t* client_objPtr,  ///< [IN] pointer to object passed from client
    uint16_t iid,                       ///< [IN] object instance ID
    bool multiple                       ///< [IN] if this is single or multiple instance object
)
{
    int j;
    lwm2mcore_internal_object_t* objPtr = NULL;
    lwm2mcore_internal_resource_t* resourcePtr = NULL;
    lwm2mcore_resource_t *client_resourcePtr = NULL;

    LOG_ARG("initObject /%d/%d, multiple %d", client_objPtr->id, iid, multiple);

    objPtr = (lwm2mcore_internal_object_t *)malloc (sizeof (lwm2mcore_internal_object_t));

    OS_ASSERT(objPtr);

    memset(objPtr, 0, sizeof (lwm2mcore_internal_object_t));

    objPtr->multiple = multiple;
    objPtr->id = client_objPtr->id;
    objPtr->iid = iid;
    memset(&(objPtr->attr), 0, sizeof (lwm2m_attribute_t));

    /* Object's create and delete handlers should be invoked by the LWM2M client
     * itself. Once the operation is completed, the client shall call avcm_create_lwm2m_object
     * or avcm_delete_lwm2m_object accordingly */
    client_resourcePtr = client_objPtr->resources;

    LOG_ARG("initObject client_obj->res_cnt %d", client_objPtr->res_cnt);

    DLIST_INIT(&(objPtr->resource_list));

    for (j = 0; j < client_objPtr->res_cnt; j++) {
        resourcePtr = (lwm2mcore_internal_resource_t *)malloc (sizeof (lwm2mcore_internal_resource_t));

        OS_ASSERT(resourcePtr);
        memset(resourcePtr, 0, sizeof(lwm2mcore_internal_resource_t));

        resourcePtr->id = (client_resourcePtr + j)->id;
        resourcePtr->iid = 0;
        resourcePtr->type = (client_resourcePtr + j)->type;
        resourcePtr->multiple = (client_resourcePtr + j)->max_res_inst_cnt > 1;
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
 * Initialize supported objects/resources based on AVCA handler data.
 *
 */
//--------------------------------------------------------------------------------------------------
static void initObjectsList
(
    struct _lwm2mcore_objects_list* objects_list,   ///< [IN] Object list
    lwm2mcore_handler_t* clientHandlerPtr           ///< [IN] Object and resource table which are
                                                    ///<      supported by the client
)
{
    int i, j;
    lwm2mcore_internal_object_t* objPtr = NULL;

    LOG_ARG("obj_cnt %d", clientHandlerPtr->obj_cnt);

    for (i = 0; i < clientHandlerPtr->obj_cnt; i++)
    {
        if (LWM2MCORE_ID_NONE == (clientHandlerPtr->objects + i)->max_obj_inst_cnt)
        {
            /*Unknown object instance count is always assumed to be multiple*/
            objPtr = initObject(clientHandlerPtr->objects + i, LWM2MCORE_ID_NONE, true);
            DLIST_INSERT_TAIL(objects_list, objPtr, list);
        }
        else if ((clientHandlerPtr->objects + i)->max_obj_inst_cnt > 1)
        {
            for (j = 0; j < (clientHandlerPtr->objects + i)->max_obj_inst_cnt; j++)
            {
                objPtr = initObject(clientHandlerPtr->objects + i, j, true);
                DLIST_INSERT_TAIL(objects_list, objPtr, list);
            }
        }
        else if (LWM2M_SERVER_OBJECT_ID == (clientHandlerPtr->objects + i)->id)
        {
            /* the max_obj_inst_cnt is 1 for this object, but this is actually multiple instance */
            objPtr = initObject(clientHandlerPtr->objects + i, 0, true);
            DLIST_INSERT_TAIL(objects_list, objPtr, list);
        }
        else
        {
            objPtr = initObject(clientHandlerPtr->objects + i, 0, false);
            DLIST_INSERT_TAIL(objects_list, objPtr, list);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/**
 *  Free the registered objects and resources (LWM2MCore and Wakaama)
 *
 */
//--------------------------------------------------------------------------------------------------
void lwm2mcore_objectFree
(
    void
)
{
    struct _lwm2mcore_objects_list* objectsListPtr = getObjectsList ();
    lwm2mcore_internal_object_t* objPtr = NULL;
    lwm2mcore_internal_resource_t* resPtr = NULL;
    uint32_t i = 0;

    /* Free memory for objects and resources for LWM2MCore */
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
        while (ObjectArray[i]->instanceList != NULL )
        {
            lwm2m_list_t *listPtr = ObjectArray[i]->instanceList;
            ObjectArray[i]->instanceList = ObjectArray[i]->instanceList->next;
            lwm2m_free(listPtr);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/**
 *  Function to register an object table
 *
 */
//--------------------------------------------------------------------------------------------------
bool RegisterObjTable
(
    lwm2mcore_handler_t* const handlerPtr,  ///< [IN] List of supported object/resource by client
    uint16_t* registeredObjNbPtr,           ///< [INOUT] Registered bject number
    bool clientTable                        ///< [IN] Indicate if the table is provided by the
                                            ///< client
)
{
    uint32_t i = 0;
    uint32_t j = 0;
    uint16_t ObjNb = *registeredObjNbPtr;
    uint16_t objInstanceNb = 0;
    bool dmServerPresence = false;
    struct _lwm2mcore_objects_list *objectsListPtr = NULL;

    /* Check if a DM server was provided: only for static LWM2MCore case */
    if ((clientTable == false) && os_portSecurityCheckDmCredentialsPresence())
    {
        dmServerPresence = true;
    }
    LOG_ARG("dmServerPresence %d", dmServerPresence);

    /* Initialize all objects for Wakaama: handlerPtr */
    for (i = 0; i < handlerPtr->obj_cnt; i++)
    {
        /* Memory allocation for one object */
        ObjectArray[ObjNb]  = \
                        (lwm2m_object_t *)lwm2m_malloc (sizeof (lwm2m_object_t));
        if (NULL != ObjectArray[ObjNb])
        {
            memset(ObjectArray[ObjNb], 0, sizeof(lwm2m_object_t));

            /* Assign the object ID */
            ObjectArray[ObjNb]->objID = (handlerPtr->objects + i)->id;
            objInstanceNb = (handlerPtr->objects + i)->max_obj_inst_cnt;

            if ((LWM2M_SECURITY_OBJECT_ID == ObjectArray[ObjNb]->objID)
                && (false == dmServerPresence))
            {
                /* Only consider one object instance for security */
                objInstanceNb = 1;
            }

            if ((LWM2M_SERVER_OBJECT_ID == ObjectArray[ObjNb]->objID)
                && (false == dmServerPresence))
            {
                /* Do not create object instance for server object (no provisioned DM server)
                 * This means that a bootstrap connection will be initiated
                 */
                objInstanceNb = 0;
            }

            if (LWM2MCORE_ID_NONE == objInstanceNb)
            {
                /* Unknown object instance count is always assumed to be multiple */
                /* TODO */
            }
            else if (1 < objInstanceNb)
            {
                lwm2m_list_t* instancePtr;
                ObjectArray[ObjNb]->instanceList = \
                        (lwm2m_list_t *)lwm2m_malloc(sizeof(lwm2m_list_t));
                memset(ObjectArray[ObjNb]->instanceList, 0, sizeof(lwm2m_list_t));
                for (j = 0; j < objInstanceNb; j++)
                {
                    /* Add the object instance in the Wakaama format */
                    instancePtr = (lwm2m_list_t *)lwm2m_malloc(sizeof(lwm2m_list_t));
                    instancePtr->id = j;
                    ObjectArray[ObjNb]->instanceList = \
                        LWM2M_LIST_ADD (ObjectArray[ObjNb]->instanceList,
                                        instancePtr);
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
            else if (objInstanceNb == 1)
            {
                /* Allocate the unique object instance */
                ObjectArray[ObjNb]->instanceList = \
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

             /*
              * And the private function that will access the object.
              * Those function will be called when a read/write/execute query is made by the
              * server. In fact the library doesn't need to know the resources of the object,
              * only the server does.
              */
            ObjectArray[ObjNb]->readFunc     = readCb;
            ObjectArray[ObjNb]->discoverFunc = discoverCb;
            ObjectArray[ObjNb]->writeFunc    = writeCb;
            ObjectArray[ObjNb]->executeFunc  = executeCb;
            ObjectArray[ObjNb]->createFunc   = createCb;

            ObjectArray[ObjNb]->userData = NULL;
            ObjNb++;
        }
    }

    /* Allocate object and resource linked to object/resource table provided by the client
     * This is used to make a link between the lwm2mcore_handler_t provided by the client
     * and the lwm2m_object_t for Wakaama
     */
    objectsListPtr = getObjectsList();
    initObjectsList(objectsListPtr, handlerPtr);
    *registeredObjNbPtr = ObjNb;
    return true;
}

//--------------------------------------------------------------------------------------------------
/**
 *                      PUBLIC FUNCTIONS
 */
//--------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------
/**
 *  Register the object table and service API
 *
 * @note If handlerPtr parameter is NULL, LWM2MCore registers it's own "standard" object list
 *
 * @return
 *      - number of registered objects
 */
//--------------------------------------------------------------------------------------------------
uint16_t lwm2mcore_objectRegister
(
    int context,                            ///< [IN] Context
    char* endpointPtr,                      ///< [IN] Device endpoint
    lwm2mcore_handler_t* const handlerPtr,  ///< [IN] List of supported object/resource by client
    void * const servicePtr                 ///< [IN] Client service API table
)
{
    RegisteredObjNb = 0;

    /* For the moment, servicePtr can be NULL */
    if (NULL == endpointPtr)
    {
        LOG("param error");
    }
    else
    {
        bool result = false;

        client_data_t* dataPtr = (client_data_t*)context;
        LOG_ARG("lwm2mcore_objectRegister context %d RegisteredObjNb %d", context, RegisteredObjNb);

        /* Register static object tables managed by LWM2MCore */
        result = RegisterObjTable(&Lwm2mcoreHandlers, &RegisteredObjNb, false);
        if (result == false)
        {
            RegisteredObjNb = 0;
            LOG("ERROR on registering LWM2MCore object table");
        }
        else
        {
            if (NULL != handlerPtr)
            {
                LOG("Register client object list");
                /* Register object tables filled by the client */
                result = RegisterObjTable(handlerPtr, &RegisteredObjNb, true);
                if (result == false)
                {
                    RegisteredObjNb = 0;
                    LOG("ERROR on registering client object table");
                }
            }
            else
            {
                LOG("Only register LWM2MCore object list");
            }

            if (true == result)
            {
                int test = 0;
                /* Save the security object list in the context (used for connection) */
                dataPtr->securityObjP = ObjectArray[LWM2M_SECURITY_OBJECT_ID];

                /* Wakaama configuration and the object registration */
                LOG_ARG("RegisteredObjNb %d", RegisteredObjNb);
                test = lwm2m_configure(dataPtr->lwm2mH,
                                       endpointPtr,
                                       NULL,
                                       NULL,
                                       RegisteredObjNb,
                                       ObjectArray);
                if (test != COAP_NO_ERROR)
                {
                    LOG_ARG("Failed to configure lwm2m client: test %d", test);
                    RegisteredObjNb = 0;
                }
                else
                {
                    LOG("configure lwm2m client OK");
                }
            }
        }
    }
    return RegisteredObjNb;
}

