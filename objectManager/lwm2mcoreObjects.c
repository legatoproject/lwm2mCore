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
 * Maximum buffer length for data when LWM2MCore object resource handlers are called
 */
//--------------------------------------------------------------------------------------------------
#define LWM2MCORE_ASYNC_BUF_LEN 256

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
extern lwm2mcore_context_t *lwm2mcore_ctx;

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
            if (operation == LWM2MCORE_OP_READ)
            {
                result = COAP_205_CONTENT;
            }
            else if (operation == LWM2MCORE_OP_WRITE)
            {
                result = COAP_204_CHANGED;
            }
            else if (operation == LWM2MCORE_OP_EXECUTE)
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
    LOG_ARG ("sID %d operation %d -> CoAP result %d", sid, operation, result);
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
static lwm2mcore_object_t *findObject
(
    lwm2mcore_context_t *lwm2mcore_ctx, ///< [IN] LWM2M core context
    uint16_t oid                    ///< [IN] object ID to find
)
{
    lwm2mcore_internal_object_t *obj = NULL;

    for (obj = DLIST_FIRST(&(lwm2mcore_ctx->objects_list)); obj; obj = DLIST_NEXT(obj, list))
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
static lwm2mcore_internal_resource_t *findResource
(
    lwm2mcore_internal_object_t *obj,   ///< [IN] Object pointer
    uint16_t rid                        ///< [IN] resource ID
)
{
    lwm2mcore_internal_resource_t *resource = NULL;

    OS_ASSERT(obj);

    for (resource = DLIST_FIRST(&(obj->resource_list));
         resource;
         resource = DLIST_NEXT(resource, list))
    {
        if (resource->id == rid)
           break;
    }

    return resource;
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to convert bytes(in network byte order) to unsigned 16 bits integer
 *
 * @return
 *      - converted data
 */
//--------------------------------------------------------------------------------------------------
static inline uint16_t bytesToUint16
(
    const uint8_t *bytes    ///< [IN] bytes the buffer contains data to be converted
)
{
    return ((bytes[0] << 8) | bytes[1]);
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to convert bytes(in network byte order) to unsigned 24 bits integer
 *
 * @return
 *      - converted data
 */
//--------------------------------------------------------------------------------------------------
static inline uint32_t bytesToUint24
(
    const uint8_t *bytes
)
{
    return ((bytes[0] << 16) | (bytes[1] << 8) | bytes[2]);
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to convert bytes(in network byte order) to unsigned 32 bits integer
 *
 * @return
 *      - converted data
 */
//--------------------------------------------------------------------------------------------------
static inline uint32_t bytesToUint32
(
    const uint8_t *bytes
)
{
    return ((bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | bytes[3]);
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to convert bytes(in network byte order) to unsigned 48 bits integer
 *
 * @return
 *      - converted data
 */
//--------------------------------------------------------------------------------------------------
static inline uint64_t bytesToUint48
(
    const uint8_t *bytes
)
{
    return (((uint64_t)bytesToUint32(bytes) << 16)
            | bytesToUint16(bytes + 4));
}

//--------------------------------------------------------------------------------------------------
/**
 * Function to convert bytes(in network byte order) to unsigned 64 bits integer
 *
 * @return
 *      - converted data
 */
//--------------------------------------------------------------------------------------------------
static inline uint64_t bytesToUint64
(
    const uint8_t *bytes
)
{
    return (((uint64_t)bytesToUint32(bytes) << 32)
            | bytesToUint32(bytes + 4));
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
    const uint8_t *bytes,
    size_t len
)
{
    int64_t value;

    switch(len)
    {
        case 1:
        {
            value = *bytes;
        }
        break;

        case 2:
        {
            value = (int16_t)bytesToUint16(bytes);
        }
        break;

        case 4:
        {
            value = (int32_t)bytesToUint32(bytes);
        }
        break;

        case 8:
        {
            value = (int64_t)bytesToUint64(bytes);
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
    int * numDataP,                 ///< [IN] Number of resources to be read
    lwm2m_data_t ** dataArrayP,     ///< [IN] Array of requested resources to be read
    lwm2m_object_t * objectP        ///< [IN] Pointer on object
)
{
    uint8_t result;
    int i;

    LOG_ARG ("readCb oid %d oiid %d", objectP->objID, instanceId);

    /* Search if the object was registered */
    if (LWM2M_LIST_FIND(objectP->instanceList, instanceId))
    {
        lwm2mcore_uri_t uri;
        lwm2mcore_internal_object_t *obj;
        LOG ("object instance Id was registered");

        memset( &uri, 0, sizeof (lwm2mcore_uri_t));

        uri.op = LWM2MCORE_OP_READ;
        uri.oid = objectP->objID;
        uri.oiid = instanceId;

        obj = findObject (lwm2mcore_ctx, objectP->objID);
        if (obj == NULL)
        {
            LOG_ARG ("Object %d is NOT registered", objectP->objID);
            result = COAP_404_NOT_FOUND;
        }
        else
        {
            int sid = 0;
            lwm2mcore_internal_resource_t *resource = NULL;
            char async_buf[LWM2MCORE_ASYNC_BUF_LEN];
            size_t async_buf_len = LWM2MCORE_ASYNC_BUF_LEN;

            LOG_ARG ("numDataP %d", *numDataP);
            // is the server asking for the full object ?
            if (*numDataP == 0)
            {
                uint16_t resList[50];
                int nbRes = 0;

                /* Search the supported resources for the required object */
                i = 0;
                for (resource = DLIST_FIRST(&(obj->resource_list));
                     resource;
                     resource = DLIST_NEXT(resource, list))
                {
                    resList[ i ] = resource->id;
                    LOG_ARG ("resList[ %d ] %d", i, resList[ i ]);
                    i++;
                }

                nbRes = i;
                LOG_ARG ("nbRes %d", nbRes);

                *dataArrayP = lwm2m_data_new(nbRes);
                if (*dataArrayP == NULL) return COAP_500_INTERNAL_SERVER_ERROR;
                *numDataP = nbRes;
                for (i = 0 ; i < nbRes ; i++)
                {
                    (*dataArrayP)[i].id = resList[i];
                }
            }

            i = 0;
            do
            {
                uri.rid = (*dataArrayP)[i].id;

                /* Search the resource handler */
                resource = findResource (obj, uri.rid);
                if (resource != NULL )
                {
                    if (resource->read != NULL )
                    {
                        async_buf_len = LWM2MCORE_ASYNC_BUF_LEN;
                        memset( async_buf, 0, async_buf_len);
                        LOG_ARG ("READ / %d / %d / %d", uri.oid, uri.oiid, uri.rid);
                        sid  = resource->read (&uri,
                                                async_buf,
                                                &async_buf_len,
                                                NULL);

                        /* Define the CoAP result */
                        result = setCoapError (sid, LWM2MCORE_OP_READ);

                        if (result == COAP_205_CONTENT)
                        {
                            switch (resource->type)
                            {
                                case LWM2MCORE_RESOURCE_TYPE_INT:
                                {
                                    int64_t value = 0;
                                    value = bytesToInt ((uint8_t*)async_buf, async_buf_len);
                                    lwm2m_data_encode_int(value, (*dataArrayP) + i);
                                    if (uri.oid != LWM2MCORE_SECURITY_OID)
                                    {
                                        LOG_ARG ("readCb sID %d value %d", sid, value);
                                    }
                                    else
                                    {
                                        LOG_ARG ("readCb sID %d", sid);
                                    }
                                }
                                break;

                                case LWM2MCORE_RESOURCE_TYPE_BOOL:
                                {
                                    lwm2m_data_encode_bool(async_buf[0], (*dataArrayP) + i);
                                    if (uri.oid != LWM2MCORE_SECURITY_OID)
                                    {
                                        LOG_ARG ("readCb sID %d value %d", sid, async_buf[0]);
                                    }
                                    else
                                    {
                                        LOG_ARG ("readCb sID %d", sid);
                                    }
                                }
                                break;

                                case LWM2MCORE_RESOURCE_TYPE_STRING:
                                {
                                    lwm2m_data_encode_nstring (async_buf,
                                                                async_buf_len,
                                                                (*dataArrayP) + i);
                                    if (uri.oid != LWM2MCORE_SECURITY_OID)
                                    {
                                        LOG_ARG ("readCb sID %d async_buf %s", sid, async_buf);
                                    }
                                    else
                                    {
                                        LOG_ARG ("readCb sID %d", sid);
                                    }
                                }
                                break;

                                case LWM2MCORE_RESOURCE_TYPE_OPAQUE:
                                {
                                    lwm2m_data_encode_opaque (async_buf,
                                                                async_buf_len,
                                                                (*dataArrayP) + i);
                                    if (uri.oid != LWM2MCORE_SECURITY_OID)
                                    {
                                        LOG_ARG ("readCb sID %d async_buf %s", sid, async_buf);
                                    }
                                    else
                                    {
                                        LOG_ARG ("readCb sID %d", sid);
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
                                    value = bytesToInt ((uint8_t*)async_buf, async_buf_len);
                                    lwm2m_data_encode_int(value, (*dataArrayP) + i);
                                    if (uri.oid != LWM2MCORE_SECURITY_OID)
                                    {
                                        LOG_ARG ("readCb sID %d value %d", sid, value);
                                    }
                                    else
                                    {
                                        LOG_ARG ("readCb sID %d", sid);
                                    }
                                }
                                break;

                                case LWM2MCORE_RESOURCE_TYPE_UNKNOWN:
                                {
                                    lwm2m_data_encode_opaque (async_buf,
                                                                async_buf_len,
                                                                (*dataArrayP) + i);
                                    if (uri.oid != LWM2MCORE_SECURITY_OID)
                                    {
                                        LOG_ARG ("readCb sID %d async_buf %s", sid, async_buf);
                                    }
                                    else
                                    {
                                        LOG_ARG ("readCb sID %d", sid);
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
                        LOG ("READ callback NULL");
                        result = COAP_404_NOT_FOUND;
                    }
                }
                else
                {
                    LOG ("resource NULL");
                    result = COAP_404_NOT_FOUND;
                }
                i++;
            } while ((i < *numDataP) && ((result == COAP_205_CONTENT)
                  || (result == COAP_NO_ERROR)));
        }
    }
    else
    {
        LOG_ARG ("Object %d not found", objectP->objID);
        result = COAP_404_NOT_FOUND;
    }
    LOG_ARG ("readCb result %d", result);
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
    lwm2m_data_t * dataArrayP,      ///< [IN] Array of requested resources to be written
    lwm2m_object_t * objectP        ///< [IN] Pointer on object
)
{
    uint8_t result;
    int i;

    LOG_ARG ("writeCb oid %d oiid %d", objectP->objID, instanceId);

    /* Search if the object was registered */
    if (LWM2M_LIST_FIND(objectP->instanceList, instanceId))
    {
        lwm2mcore_uri_t uri;
        lwm2mcore_internal_object_t *obj;
        LOG ("object instance Id was registered");

        memset( &uri, 0, sizeof (lwm2mcore_uri_t));

        uri.op = LWM2MCORE_OP_WRITE;
        uri.oid = objectP->objID;
        uri.oiid = instanceId;

        obj = findObject (lwm2mcore_ctx, objectP->objID);
        if (obj == NULL)
        {
            LOG_ARG ("Object %d is NOT registered", objectP->objID);
            result = COAP_404_NOT_FOUND;
        }
        else
        {
            int sid = 0;
            lwm2mcore_internal_resource_t *resource = NULL;
            char async_buf[LWM2MCORE_ASYNC_BUF_LEN];
            size_t async_buf_len = LWM2MCORE_ASYNC_BUF_LEN;

            LOG_ARG ("numData %d", numData);
            // is the server asking for the full object ?
            if (numData == 0)
            {
                uint16_t resList[50];
                int nbRes = 0;

                /* Search the supported resources for the required object */
                i = 0;
                for (resource = DLIST_FIRST(&(obj->resource_list));
                     resource;
                     resource = DLIST_NEXT(resource, list))
                {
                    resList[ i++ ] = resource->id;
                }

                nbRes = i;

                dataArrayP = lwm2m_data_new(nbRes);
                if (dataArrayP == NULL) return COAP_500_INTERNAL_SERVER_ERROR;
                numData = nbRes;
                for (i = 0 ; i < nbRes ; i++)
                {
                    dataArrayP[i].id = resList[i];
                }
            }

            i = 0;
            do
            {
                uri.rid = dataArrayP[i].id;
                memset (async_buf, 0, LWM2MCORE_ASYNC_BUF_LEN);
                async_buf_len = LWM2MCORE_ASYNC_BUF_LEN;

                /* Search the resource handler */
                resource = findResource (obj, uri.rid);
                if (resource != NULL )
                {
                    if (resource->write != NULL )
                    {
                        LOG_ARG ("data type %d", dataArrayP[i].type);

                        switch (dataArrayP[i].type)
                        {
                            case LWM2M_TYPE_STRING:
                            {
                                LOG ("writeCb string");
                                if (dataArrayP[i].value.asBuffer.length <= LWM2MCORE_ASYNC_BUF_LEN)
                                {
                                    memcpy (async_buf,
                                            dataArrayP[i].value.asBuffer.buffer,
                                            dataArrayP[i].value.asBuffer.length);
                                    async_buf_len = dataArrayP[i].value.asBuffer.length;
                                }
                            }
                            break;

                            case LWM2M_TYPE_OPAQUE:
                            {
                                LOG ("writeCb opaque");
                                if (dataArrayP[i].value.asBuffer.length <= LWM2MCORE_ASYNC_BUF_LEN)
                                {
                                    memcpy (async_buf,
                                            dataArrayP[i].value.asBuffer.buffer,
                                            dataArrayP[i].value.asBuffer.length);
                                    async_buf_len = dataArrayP[i].value.asBuffer.length;
                                }
                            }
                            break;

                            case LWM2M_TYPE_INTEGER:
                            {
                                int64_t value = 0;
                                LOG ("writeCb integer");
                                if (0 == lwm2m_data_decode_int (dataArrayP+i, &value))
                                {
                                    LOG ("integer decode ERROR");
                                }
                                else
                                {
                                    LOG_ARG ("writeCb integer %d", value);
                                }
                            }
                            break;

                            case LWM2M_TYPE_FLOAT:
                            {
                                LOG ("writeCb float");
                            }
                            break;

                            case LWM2M_TYPE_BOOLEAN:
                            {
                                bool value = false;
                                LOG ("writeCb bool");
                                if (0 == lwm2m_data_decode_bool (dataArrayP+i, &value))
                                {
                                    LOG ("bool decode ERROR");
                                }
                                else
                                {
                                    LOG_ARG ("writeCb bool %d", value);
                                }
                            }
                            break;

                            default:
                            break;
                        }
                        LOG_ARG ("WRITE / %d / %d / %d", uri.oid, uri.oiid, uri.rid);
                        sid  = resource->write (&uri,
                                                async_buf,
                                                &async_buf_len,
                                                NULL);
                        LOG_ARG ("WRITE sID %d", sid);
                        /* Define the CoAP result */
                        result = setCoapError (sid, LWM2MCORE_OP_WRITE);
                    }
                    else
                    {
                        LOG ("READ callback NULL");
                        result = COAP_404_NOT_FOUND;
                    }
                }
                else
                {
                    LOG ("resource NULL");
                    result = COAP_404_NOT_FOUND;
                }
                i++;
            } while ((i < numData) && ((result == COAP_204_CHANGED) || (result == COAP_NO_ERROR)));
        }
    }
    else
    {
        LOG_ARG ("Object %d not found", objectP->objID);
        result = COAP_404_NOT_FOUND;
    }
    LOG_ARG ("writeCb result %d", result);
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
    uint16_t id,
    lwm2m_object_t * objectP
)
{
    lwm2m_list_t * instanceP;
    LOG ("Enter");
    objectP->instanceList = lwm2m_list_remove(objectP->instanceList,
                                                id,
                                                (lwm2m_list_t **)&instanceP);
    if (NULL == instanceP) return COAP_404_NOT_FOUND;

    lwm2m_free(instanceP);

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
    lwm2m_data_t * dataArrayP,      ///< [IN] Array of requested resources to be written
    lwm2m_object_t * objectP        ///< [IN] Pointer on object
)
{
    uint8_t result;
    bool instanceCreated = false;
    LOG_ARG ("createCb oid %d oiid %d", objectP->objID, instanceId);

    if (objectP->instanceList == NULL)
    {
        LOG ("objectP->instanceList == NULL");
        objectP->instanceList = (lwm2m_list_t *)lwm2m_malloc(sizeof(lwm2m_list_t));
        if (objectP->instanceList != NULL)
        {
            memset(objectP->instanceList, 0, sizeof(lwm2m_list_t));
            instanceCreated = true;
        }
    }
    else
        LOG ("objectP->instanceList != NULL");

    /* Search if the object was registered */
    if ((LWM2M_LIST_FIND(objectP->instanceList, instanceId) == NULL)
     || instanceCreated)
    {
        lwm2m_list_t * instanceP;
        /* Add the object instance in the Wakaama format */
        instanceP = (lwm2m_list_t *)lwm2m_malloc(sizeof(lwm2m_list_t));
        instanceP->id = instanceId;
        objectP->instanceList = LWM2M_LIST_ADD(objectP->instanceList, instanceP);

        result = writeCb (instanceId, numData, dataArrayP, objectP);
        if (result != COAP_204_CHANGED)
        {
            LOG_ARG ("createCb --> delete oiid %d", instanceId);
            (void)deleteObjInstance(instanceId, objectP);
            result = COAP_500_INTERNAL_SERVER_ERROR;
        }
        else
        {
            result = COAP_201_CREATED;
        }
    }
    else
    {
        LOG ("Object instance already exists");
        result = COAP_400_BAD_REQUEST;
    }
    LOG_ARG ("createCb result %d", result);
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
    int * numDataP,                 ///< [INOUT] Number of resources which were read
    lwm2m_data_t ** dataArrayP,     ///< [IN] Array of requested resources to be discovered
    lwm2m_object_t * objectP        ///< [IN] Pointer on object
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
    uint8_t * buffer,               ///< [IN] Data provided in the EXECUTE command
    int length,                     ///< [IN] Data length
    lwm2m_object_t * objectP        ///< [IN] Pointer on object
)
{
    uint8_t result;
    int i;

    LOG_ARG ("ExecuteCb oid %d oiid %d rid %d", objectP->objID, instanceId, resourceId);

    /* Search if the object was registered */
    if (LWM2M_LIST_FIND(objectP->instanceList, instanceId))
    {
        lwm2mcore_uri_t uri;
        lwm2mcore_internal_object_t *obj;
        LOG ("object instance Id was registered");

        memset (&uri, 0, sizeof (lwm2mcore_uri_t));

        uri.op = LWM2MCORE_OP_EXECUTE;
        uri.oid = objectP->objID;
        uri.oiid = instanceId;
        uri.rid = resourceId;

        obj = findObject (lwm2mcore_ctx, objectP->objID);
        if (obj == NULL)
        {
            LOG_ARG ("Object %d is NOT registered", objectP->objID);
            result = COAP_404_NOT_FOUND;
        }
        else
        {
            int sid = 0;
            lwm2mcore_internal_resource_t *resource = NULL;
            size_t len = (size_t) length;

            /* Search the resource handler */
            resource = findResource (obj, uri.rid);
            if (resource != NULL )
            {
                if (resource->exec != NULL )
                {
                    LOG_ARG ("EXECUTE / %d / %d / %d", uri.oid, uri.oiid, uri.rid);
                    sid  = resource->exec (&uri,
                                            buffer,
                                            &len);
                    LOG_ARG ("EXECUTE sID %d", sid);
                    /* Define the CoAP result */
                    result = setCoapError (sid, LWM2MCORE_OP_EXECUTE);
                }
                else
                {
                    LOG ("EXECUTE callback NULL");
                    result = COAP_404_NOT_FOUND;
                }
            }
            else
            {
                LOG ("resource NULL");
                result = COAP_404_NOT_FOUND;
            }
        }
    }
    else
    {
        LOG_ARG ("Object %d not found", objectP->objID);
        result = COAP_404_NOT_FOUND;
    }
    LOG_ARG ("ExecuteCb result %d", result);
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
struct _lwm2mcore_objects_list *getObjectsList
(
    void
)
{
    if (lwm2mcore_ctx != NULL)
        return &(lwm2mcore_ctx->objects_list);
    else
        return NULL;
}

//--------------------------------------------------------------------------------------------------
/**
 * Initialize lwm2m object
 *
 * @return
 *      - object pointer
 */
//--------------------------------------------------------------------------------------------------
static lwm2mcore_internal_object_t *initObject
(
    lwm2mcore_object_t *client_obj, ///< [IN] pointer to object passed from client
    uint16_t iid,                   ///< [IN] object instance ID
    bool multiple                   ///< [IN] if this is single or multiple instance object
)
{
    int j;
    lwm2mcore_internal_object_t *obj = NULL;
    lwm2mcore_internal_resource_t *resource = NULL;
    lwm2mcore_resource_t *client_resource_p = NULL;

    LOG_ARG ("initObject /%d/%d, multiple %d", client_obj->id, iid, multiple);

    obj = (lwm2mcore_internal_object_t *)malloc (sizeof (lwm2mcore_internal_object_t));

    OS_ASSERT (obj);

    memset (obj, 0, sizeof (lwm2mcore_internal_object_t));

    obj->multiple = multiple;
    obj->id = client_obj->id;
    obj->iid = iid;
    memset (&(obj->attr), 0, sizeof (lwm2m_attribute_t));

    /* Object's create and delete handlers should be invoked by the LWM2M client
     * itself. Once the operation is completed, the client shall call avcm_create_lwm2m_object
     * or avcm_delete_lwm2m_object accordingly */
    client_resource_p = client_obj->resources;

    LOG_ARG ("initObject client_obj->res_cnt %d", client_obj->res_cnt);

    DLIST_INIT(&(obj->resource_list));

    for (j = 0; j < client_obj->res_cnt; j++) {
        resource = (lwm2mcore_internal_resource_t *)malloc (sizeof (lwm2mcore_internal_resource_t));

        OS_ASSERT (resource);
        memset (resource, 0, sizeof(lwm2mcore_internal_resource_t));

        resource->id = (client_resource_p + j)->id;
        resource->iid = 0;
        resource->type = (client_resource_p + j)->type;
        resource->multiple = (client_resource_p + j)->max_res_inst_cnt > 1;
        memset(&resource->attr, 0, sizeof(lwm2m_attribute_t));
        resource->read = (client_resource_p + j)->read;
        resource->write = (client_resource_p + j)->write;
        resource->exec = (client_resource_p + j)->exec;
        DLIST_INSERT_TAIL(&(obj->resource_list), resource, list);
    }

    return obj;
}

//--------------------------------------------------------------------------------------------------
/**
 * Initialize supported objects/resources based on AVCA handler data.
 *
 */
//--------------------------------------------------------------------------------------------------
static void initObjectsList
(
    struct _lwm2mcore_objects_list *objects_list,   ///< [IN] Object list
    lwm2mcore_handler_t *clientHandlerPtr           ///< [IN] Object and resource table which are
                                                    ///<      supported by the client
)
{
    int i, j;
    lwm2mcore_internal_object_t *obj = NULL;

    LOG_ARG ("obj_cnt %d", clientHandlerPtr->obj_cnt);

    for (i = 0; i < clientHandlerPtr->obj_cnt; i++)
    {
        if ((clientHandlerPtr->objects + i)->max_obj_inst_cnt == LWM2MCORE_ID_NONE)
        {
            /*Unknown object instance count is always assumed to be multiple*/
            obj = initObject (clientHandlerPtr->objects + i,
                                                    LWM2MCORE_ID_NONE,
                                                    true);
            DLIST_INSERT_TAIL(objects_list, obj, list);
        }
        else if ((clientHandlerPtr->objects + i)->max_obj_inst_cnt > 1)
        {
            for (j = 0; j < (clientHandlerPtr->objects + i)->max_obj_inst_cnt; j++)
            {
                obj = initObject(clientHandlerPtr->objects + i, j, true);
                DLIST_INSERT_TAIL(objects_list, obj, list);
            }
        }
        else  if ((clientHandlerPtr->objects + i)->id == LWM2M_SERVER_OBJECT_ID)
        {
            /* the max_obj_inst_cnt is 1 for this object, but this is actually multiple instance */
            obj = initObject(clientHandlerPtr->objects + i, 0, true);
            DLIST_INSERT_TAIL(objects_list, obj, list);
        }
        else
        {
            obj = initObject(clientHandlerPtr->objects + i, 0, false);
            DLIST_INSERT_TAIL(objects_list, obj, list);
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
    struct _lwm2mcore_objects_list *objects_list = getObjectsList ();
    lwm2mcore_internal_object_t *obj = NULL;
    lwm2mcore_internal_resource_t *res = NULL;
    uint32_t i = 0;

    /* Free memory for objects and resources for LWM2MCore */
    while ((obj = DLIST_FIRST(objects_list)) != NULL)
    {
        while ((res = DLIST_FIRST(&(obj->resource_list))) != NULL)
        {
            DLIST_REMOVE_HEAD(&(obj->resource_list), list);
            lwm2m_free (res);
        }
        DLIST_REMOVE_HEAD(objects_list, list);
        lwm2m_free (obj);
    }

    /* Free memory for objects and resources for Wakaama */
    LOG_ARG ("Wakaama RegisteredObjNb %d", RegisteredObjNb);
    for (i = 0; i < RegisteredObjNb; i++)
    {
        while (ObjectArray[i]->instanceList != NULL )
        {
            lwm2m_list_t *list = ObjectArray[i]->instanceList;
            ObjectArray[i]->instanceList = ObjectArray[i]->instanceList->next;
            lwm2m_free (list);
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
    lwm2mcore_handler_t *const handlerPtr,  ///< [IN] List of supported object/resource by client
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
    struct _lwm2mcore_objects_list *objects_list = NULL;

    /* Check if a DM server was provided: only for static LWM2MCore case */
    if ((clientTable == false) && os_portSecurityCheckDmCredentialsPresence())
    {
        dmServerPresence = true;
    }
    LOG_ARG ("dmServerPresence %d", dmServerPresence);

    /* Initialize all objects for Wakaama: handlerPtr */
    for (i = 0; i < handlerPtr->obj_cnt; i++)
    {
        /* Memory allocation for one object */
        ObjectArray[ObjNb]  = \
                        (lwm2m_object_t *)lwm2m_malloc (sizeof (lwm2m_object_t));
        if (NULL != ObjectArray[ObjNb])
        {
            memset (ObjectArray[ObjNb], 0, sizeof(lwm2m_object_t));

            /* Assign the object ID */
            ObjectArray[ObjNb]->objID = (handlerPtr->objects + i)->id;
            objInstanceNb = (handlerPtr->objects + i)->max_obj_inst_cnt;

            if ((ObjectArray[ObjNb]->objID == LWM2M_SECURITY_OBJECT_ID)
                && (dmServerPresence == false))
            {
                /* Only consider one object instance for security */
                objInstanceNb = 1;
            }

            if ((ObjectArray[ObjNb]->objID == LWM2M_SERVER_OBJECT_ID)
                && (dmServerPresence == false))
            {
                /* Do not create object instance for server object (no provisioned DM server)
                 * This means that a bootstrap connection will be initiated
                 */
                objInstanceNb = 0;
            }

            if (objInstanceNb == LWM2MCORE_ID_NONE)
            {
                /* Unknown object instance count is always assumed to be multiple */
                /* TODO */
            }
            else if (objInstanceNb > 1)
            {
                lwm2m_list_t * instanceP;
                ObjectArray[ObjNb]->instanceList = \
                        (lwm2m_list_t *)lwm2m_malloc(sizeof(lwm2m_list_t));
                memset (ObjectArray[ObjNb]->instanceList,
                        0,
                        sizeof(lwm2m_list_t));
                for (j = 0; j < objInstanceNb; j++)
                {
                    /* Add the object instance in the Wakaama format */
                    instanceP = (lwm2m_list_t *)lwm2m_malloc(sizeof(lwm2m_list_t));
                    instanceP->id = j;
                    ObjectArray[ObjNb]->instanceList = \
                        LWM2M_LIST_ADD (ObjectArray[ObjNb]->instanceList,
                                        instanceP);
                }

                for (j = 0; j < objInstanceNb; j++)
                {
                    if (NULL == lwm2m_list_find(ObjectArray[ObjNb]->instanceList, j))
                    {
                        LOG_ARG ("Oid %d / oiid %d NOT present",
                                    ObjectArray[ObjNb]->objID, j);
                    }
                    else
                    {
                        LOG_ARG ("Oid %d / oiid %d present",
                                    ObjectArray[ObjNb]->objID, j);
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
                    memset (ObjectArray[ObjNb]->instanceList,
                            0,
                            sizeof(lwm2m_list_t));
                }
                else
                {
                    lwm2m_free(ObjectArray[ObjNb]);
                    return false;
                }

                if (NULL == lwm2m_list_find(ObjectArray[ObjNb]->instanceList,
                                            0))
                {
                    LOG_ARG ("oid %d / oiid %d NOT present",
                                ObjectArray[ObjNb]->objID, 0);
                }
                else
                {
                    LOG_ARG ("oid %d / oiid %d present",
                                ObjectArray[ObjNb]->objID, 0);
                }
            }
            else
            {
                LOG_ARG ("No instance to create in Wakaama for object %d",
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
    objects_list = getObjectsList ();
    initObjectsList (objects_list, handlerPtr);
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
    lwm2mcore_handler_t *const handlerPtr,  ///< [IN] List of supported object/resource by client
    void * const servicePtr                 ///< [IN] Client service API table
)
{
    RegisteredObjNb = 0;

    if (endpointPtr == NULL) //|| (servicePtr == NULL ))*/
    {
        LOG("param error");
    }
    else
    {
        bool result = false;

        client_data_t* dataPtr = (client_data_t*)context;
        LOG_ARG ("lwm2mcore_objectRegister context %d RegisteredObjNb %d", context, RegisteredObjNb);

        /* Register static object tables managed by LWM2MCore */
        result = RegisterObjTable (&Lwm2mcoreHandlers, &RegisteredObjNb, false);
        if (result == false)
        {
            RegisteredObjNb = 0;
            LOG ("ERROR on registering LWM2MCore object table");
        }
        else
        {
            if (handlerPtr != NULL)
            {
                LOG ("Register client object list");
                /* Register object tables filled by the client */
                result = RegisterObjTable (handlerPtr, &RegisteredObjNb, true);
                if (result == false)
                {
                    RegisteredObjNb = 0;
                    LOG ("ERROR on registering client object table");
                }
            }
            else
            {
                LOG ("Only register LWM2MCore object list");
            }

            if (result == true)
            {
                int test = 0;
                /* Save the security object list in the context (used for connection) */
                dataPtr->securityObjP = ObjectArray[LWM2M_SECURITY_OBJECT_ID];

                /* Wakaama configuration and the object registration */
                LOG_ARG ("RegisteredObjNb %d", RegisteredObjNb);
                test = lwm2m_configure (dataPtr->lwm2mH,
                                        endpointPtr,
                                        NULL,
                                        NULL,
                                        RegisteredObjNb,
                                        ObjectArray);
                if (test != COAP_NO_ERROR)
                {
                    LOG_ARG ("Failed to configure lwm2m client: test %d", test);
                    RegisteredObjNb = 0;
                }
                else
                {
                    LOG ("configure lwm2m client OK");
                }
            }
        }
    }
    return RegisteredObjNb;
}

