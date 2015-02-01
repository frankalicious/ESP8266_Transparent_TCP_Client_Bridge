#ifndef __CLIENT_H__
#define __CLIENT_H__

#include <ip_addr.h>
#include <c_types.h>
#include <espconn.h>

static os_timer_t network_timer;

void ICACHE_FLASH_ATTR networkConnectedCb(void *arg);
void ICACHE_FLASH_ATTR networkDisconCb(void *arg);
void ICACHE_FLASH_ATTR networkReconCb(void *arg, sint8 err);
void ICACHE_FLASH_ATTR networkRecvCb(void *arg, char *data, unsigned short len);
void ICACHE_FLASH_ATTR networkSentCb(void *arg);
void ICACHE_FLASH_ATTR network_init();


#endif /* __CLIENT_H__ */
