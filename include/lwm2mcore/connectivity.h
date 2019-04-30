/**
 * @file connectivity.h
 *
 * Porting layer for connectivity parameters
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#ifndef __LWM2MCORE_CONNECTIVITY_H__
#define __LWM2MCORE_CONNECTIVITY_H__

#include <lwm2mcore/lwm2mcore.h>
#include "objects.h"
#include "clientConfig.h"

/**
  * @addtogroup lwm2mcore_connectivity_monitoring_IFS
  * @{
  */

//--------------------------------------------------------------------------------------------------
/**
 * @brief Retrieve the network bearer used for the current LWM2M communication session
 * This API treatment needs to have a procedural treatment
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *  - @ref LWM2MCORE_ERR_OVERFLOW in case of buffer overflow
 */
//--------------------------------------------------------------------------------------------------
LWM2MCORE_SHARED lwm2mcore_Sid_t lwm2mcore_GetNetworkBearer
(
    lwm2mcore_networkBearer_enum_t* valuePtr    ///< [INOUT] data buffer
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Retrieve the list of current available network bearers
 * This API treatment needs to have a procedural treatment
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *  - @ref LWM2MCORE_ERR_OVERFLOW in case of buffer overflow
 */
//--------------------------------------------------------------------------------------------------
LWM2MCORE_SHARED lwm2mcore_Sid_t lwm2mcore_GetAvailableNetworkBearers
(
    lwm2mcore_networkBearer_enum_t* bearersListPtr,     ///< [IN]    bearers list pointer
    uint16_t* bearersNbPtr                              ///< [INOUT] bearers number
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Retrieve the average value of the received signal strength indication used in the current
 * network bearer (in dBm)
 * This API treatment needs to have a procedural treatment
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 */
//--------------------------------------------------------------------------------------------------
LWM2MCORE_SHARED lwm2mcore_Sid_t lwm2mcore_GetSignalStrength
(
    int32_t* valuePtr   ///< [INOUT] data buffer
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Retrieve the received link quality
 * This API treatment needs to have a procedural treatment
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 */
//--------------------------------------------------------------------------------------------------
LWM2MCORE_SHARED lwm2mcore_Sid_t lwm2mcore_GetLinkQuality
(
    int* valuePtr  ///< [INOUT] data buffer
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Retrieve the list of IP addresses assigned to the connectivity interface
 * This API treatment needs to have a procedural treatment
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 */
//--------------------------------------------------------------------------------------------------
LWM2MCORE_SHARED lwm2mcore_Sid_t lwm2mcore_GetIpAddresses
(
    char ipAddrList[CONN_MONITOR_IP_ADDRESSES_MAX_NB][CONN_MONITOR_IP_ADDR_MAX_BYTES],
                            ///< [INOUT] IP addresses list
    uint16_t* ipAddrNbPtr   ///< [INOUT] IP addresses number
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Retrieve the list of the next-hop router IP addresses
 * This API treatment needs to have a procedural treatment
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 */
//--------------------------------------------------------------------------------------------------
LWM2MCORE_SHARED lwm2mcore_Sid_t lwm2mcore_GetRouterIpAddresses
(
    char ipAddrList[CONN_MONITOR_ROUTER_IP_ADDRESSES_MAX_NB][CONN_MONITOR_IP_ADDR_MAX_BYTES],
                            ///< [INOUT] IP addresses list
    uint16_t* ipAddrNbPtr   ///< [INOUT] IP addresses number
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Retrieve the average utilization of the link to the next-hop IP router in %
 * This API treatment needs to have a procedural treatment
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 */
//--------------------------------------------------------------------------------------------------
LWM2MCORE_SHARED lwm2mcore_Sid_t lwm2mcore_GetLinkUtilization
(
    uint8_t* valuePtr   ///< [INOUT] data buffer
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Retrieve the list of the Access Point Names
 * This API treatment needs to have a procedural treatment
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 */
//--------------------------------------------------------------------------------------------------
LWM2MCORE_SHARED lwm2mcore_Sid_t lwm2mcore_GetAccessPointNames
(
    char apnList[CONN_MONITOR_APN_MAX_NB][CONN_MONITOR_APN_MAX_BYTES],  ///< [INOUT] APN list
    uint16_t* apnNbPtr                                                  ///< [INOUT] APN number
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Retrieve the serving cell ID
 * This API treatment needs to have a procedural treatment
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 */
//--------------------------------------------------------------------------------------------------
LWM2MCORE_SHARED lwm2mcore_Sid_t lwm2mcore_GetCellId
(
    uint32_t* valuePtr  ///< [INOUT] data buffer
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Retrieve the serving Mobile Network Code and/or the serving Mobile Country Code
 * This API treatment needs to have a procedural treatment
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 */
//--------------------------------------------------------------------------------------------------
LWM2MCORE_SHARED lwm2mcore_Sid_t lwm2mcore_GetMncMcc
(
    uint16_t* mncPtr,   ///< [INOUT] MNC buffer, NULL if not needed
    uint16_t* mccPtr    ///< [INOUT] MCC buffer, NULL if not needed
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Retrieve the signal bars (range 0-5)
 * This API treatment needs to have a procedural treatment
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 */
//--------------------------------------------------------------------------------------------------
LWM2MCORE_SHARED lwm2mcore_Sid_t lwm2mcore_GetSignalBars
(
    uint8_t* valuePtr   ///< [INOUT] data buffer
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Retrieve the currently used cellular technology
 * This API treatment needs to have a procedural treatment
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *  - @ref LWM2MCORE_ERR_OVERFLOW in case of buffer overflow
 */
//--------------------------------------------------------------------------------------------------
LWM2MCORE_SHARED lwm2mcore_Sid_t lwm2mcore_GetCellularTechUsed
(
    char*   bufferPtr,  ///< [IN]    data buffer pointer
    size_t* lenPtr      ///< [INOUT] length of input buffer and length of the returned data
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Retrieve the roaming indicator (0: home, 1: roaming)
 * This API treatment needs to have a procedural treatment
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *  - @ref LWM2MCORE_ERR_OVERFLOW in case of buffer overflow
 */
//--------------------------------------------------------------------------------------------------
LWM2MCORE_SHARED lwm2mcore_Sid_t lwm2mcore_GetRoamingIndicator
(
    uint8_t* valuePtr   ///< [INOUT] data buffer
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Retrieve the signal to noise Ec/Io ratio (in dBm)
 * This API treatment needs to have a procedural treatment
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *  - @ref LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 */
//--------------------------------------------------------------------------------------------------
LWM2MCORE_SHARED lwm2mcore_Sid_t lwm2mcore_GetEcIo
(
    int32_t* valuePtr   ///< [INOUT] data buffer
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Retrieve the Reference Signal Received Power (in dBm) if LTE is used
 * This API treatment needs to have a procedural treatment
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *  - @ref LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 */
//--------------------------------------------------------------------------------------------------
LWM2MCORE_SHARED lwm2mcore_Sid_t lwm2mcore_GetRsrp
(
    int32_t* valuePtr   ///< [INOUT] data buffer
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Retrieve the Reference Signal Received Quality (in dB) if LTE is used
 * This API treatment needs to have a procedural treatment
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *  - @ref LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 */
//--------------------------------------------------------------------------------------------------
LWM2MCORE_SHARED lwm2mcore_Sid_t lwm2mcore_GetRsrq
(
    int32_t* valuePtr   ///< [INOUT] data buffer
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Retrieve the Received Signal Code Power (in dBm) if UMTS is used
 * This API treatment needs to have a procedural treatment
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *  - @ref LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 */
//--------------------------------------------------------------------------------------------------
LWM2MCORE_SHARED lwm2mcore_Sid_t lwm2mcore_GetRscp
(
    int32_t* valuePtr   ///< [INOUT] data buffer
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Retrieve the Location Area Code
 * This API treatment needs to have a procedural treatment
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *  - @ref LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 */
//--------------------------------------------------------------------------------------------------
LWM2MCORE_SHARED lwm2mcore_Sid_t lwm2mcore_GetLac
(
    uint32_t* valuePtr  ///< [INOUT] data buffer
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Retrieve the Tracking Area Code (LTE)
 * This API treatment needs to have a procedural treatment
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 *  - @ref LWM2MCORE_ERR_INVALID_STATE in case of invalid state to treat the resource handler
 */
//--------------------------------------------------------------------------------------------------
LWM2MCORE_SHARED lwm2mcore_Sid_t lwm2mcore_GetServingCellLteTracAreaCode
(
    uint16_t* valuePtr  ///< [INOUT] data buffer
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Retrieve the total number of SMS successfully transmitted during the collection period
 * This API treatment needs to have a procedural treatment
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 */
//--------------------------------------------------------------------------------------------------
LWM2MCORE_SHARED lwm2mcore_Sid_t lwm2mcore_GetSmsTxCount
(
    uint64_t* valuePtr  ///< [INOUT] data buffer
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Retrieve the total number of SMS successfully received during the collection period
 * This API treatment needs to have a procedural treatment
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 */
//--------------------------------------------------------------------------------------------------
LWM2MCORE_SHARED lwm2mcore_Sid_t lwm2mcore_GetSmsRxCount
(
    uint64_t* valuePtr  ///< [INOUT] data buffer
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Retrieve the total amount of data transmitted during the collection period (in kilobytes)
 * This API treatment needs to have a procedural treatment
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 */
//--------------------------------------------------------------------------------------------------
LWM2MCORE_SHARED lwm2mcore_Sid_t lwm2mcore_GetTxData
(
    uint64_t* valuePtr  ///< [INOUT] data buffer
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Retrieve the total amount of data received during the collection period (in kilobytes)
 * This API treatment needs to have a procedural treatment
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 */
//--------------------------------------------------------------------------------------------------
LWM2MCORE_SHARED lwm2mcore_Sid_t lwm2mcore_GetRxData
(
    uint64_t* valuePtr  ///< [INOUT] data buffer
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Reset SMS and data counters and start to collect information
 * This API treatment needs to have a procedural treatment
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 */
//--------------------------------------------------------------------------------------------------
LWM2MCORE_SHARED lwm2mcore_Sid_t lwm2mcore_StartConnectivityCounters
(
    void
);

//--------------------------------------------------------------------------------------------------
/**
 * @brief Stop SMS and data counters without resetting the counters
 * This API treatment needs to have a procedural treatment
 *
 * @remark Platform adaptor function which needs to be defined on client side.
 *
 * @return
 *  - @ref LWM2MCORE_ERR_COMPLETED_OK if the treatment succeeds
 *  - @ref LWM2MCORE_ERR_GENERAL_ERROR if the treatment fails
 *  - @ref LWM2MCORE_ERR_NOT_YET_IMPLEMENTED if the resource is not yet implemented
 *  - @ref LWM2MCORE_ERR_OP_NOT_SUPPORTED  if the resource is not supported
 *  - @ref LWM2MCORE_ERR_INVALID_ARG if a parameter is invalid in resource handler
 */
//--------------------------------------------------------------------------------------------------
LWM2MCORE_SHARED lwm2mcore_Sid_t lwm2mcore_StopConnectivityCounters
(
    void
);

/**
  * @}
  */

#endif /* __LWM2MCORE_CONNECTIVITY_H__ */
