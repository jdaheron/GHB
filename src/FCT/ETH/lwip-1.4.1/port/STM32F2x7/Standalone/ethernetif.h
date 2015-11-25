#ifndef __ETHERNETIF_H__
#define __ETHERNETIF_H__


#include <ETH/lwip-1.4.1/src/include/lwip/err.h>
#include <ETH/lwip-1.4.1/src/include/lwip/netif.h>

err_t ethernetif_init(struct netif *netif, u8_t* MAC);
err_t ethernetif_input(struct netif *netif);

#endif
