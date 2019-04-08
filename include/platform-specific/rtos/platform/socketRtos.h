#ifndef _SOCKETRTOS_H
#define _SOCKETRTOS_H

#include <stdlib.h>
#include <stdint.h>
#include <lwip/sockets.h>

// Macro definitions which are needed by LWM2MCORE but not provided by lwip
#define IN6_IS_ADDR_V4MAPPED(v6addr) ip6_addr_isipv4mappedipv6((ip6_addr_t*)v6addr)

#endif /* _SOCKETRTOS_H */
