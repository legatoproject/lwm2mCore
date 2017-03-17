/**
 * @file clientConfig.h
 *
 * Header file for client-dependent configuration
 *
 * Copyright (C) Sierra Wireless Inc.
 *
 */

#ifndef _CLIENTCONFIG_H_
#define _CLIENTCONFIG_H_

//--------------------------------------------------------------------------------------------------
/**
 * Define the maximal number of PDP contexts
 */
//--------------------------------------------------------------------------------------------------
#define MAX_PDP_CONTEXTS    4

//--------------------------------------------------------------------------------------------------
/**
 * Define the maximal number of available network bearers (resource 1 of object 4)
 */
//--------------------------------------------------------------------------------------------------
#define CONN_MONITOR_AVAIL_NETWORK_BEARER_MAX_NB    20

//--------------------------------------------------------------------------------------------------
/**
 * Define the maximal number of IP addresses associated to the device:
 * one IPv4 and one IPv6 for each PDP context (resource 4 of object 4)
 */
//--------------------------------------------------------------------------------------------------
#define CONN_MONITOR_IP_ADDRESSES_MAX_NB            (2 * MAX_PDP_CONTEXTS)

//--------------------------------------------------------------------------------------------------
/**
 * Define the maximal number of router IP addresses associated to the device:
 * one IPv4 and one IPv6 for each PDP context (resource 5 of object 4)
 */
//--------------------------------------------------------------------------------------------------
#define CONN_MONITOR_ROUTER_IP_ADDRESSES_MAX_NB     (2 * MAX_PDP_CONTEXTS)

//--------------------------------------------------------------------------------------------------
/**
 * Define the maximal bytes number in an IP address (IPv4 and IPv6), including the null-terminator
 */
//--------------------------------------------------------------------------------------------------
#define CONN_MONITOR_IP_ADDR_MAX_BYTES              46

//--------------------------------------------------------------------------------------------------
/**
 * Define the maximal number of APN, one per PDP context
 * (resource 7 of object 4)
 */
//--------------------------------------------------------------------------------------------------
#define CONN_MONITOR_APN_MAX_NB                     MAX_PDP_CONTEXTS

//--------------------------------------------------------------------------------------------------
/**
 * Define the maximal bytes number in an APN, including the null-terminator
 */
//--------------------------------------------------------------------------------------------------
#define CONN_MONITOR_APN_MAX_BYTES                  101

#endif /* _CLIENTCONFIG_H_ */

