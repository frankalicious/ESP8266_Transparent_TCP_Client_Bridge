//#include "espmissingincludes.h"
#include "c_types.h"
#include "user_interface.h"
//#include "espconn.h"
#include "mem.h"
#include "osapi.h"
//#include "driver/uart.h"

#include "client.h"
#include "user_config.h"
#include "LED_Watchdog.h"

extern os_timer_t led_timer;

struct espconn *pconn = NULL;

#define DEBUG_OUTPUT 0

void ICACHE_FLASH_ATTR networkServerFoundCb(const char *name, ip_addr_t *ip, void *arg) 
{

}

void ICACHE_FLASH_ATTR networkSentCb(void *arg) 
{
#if DEBUG_OUTPUT
  uart0_tx_buffer("sent",4);
#endif
}

void ICACHE_FLASH_ATTR networkRecvCb(void *arg, char *data, unsigned short len) 
{
  
#if DEBUG_OUTPUT
  uart0_tx_buffer("recv",4);
#endif
  struct espconn *conn=(struct espconn *)arg;
  uart0_tx_buffer(data,len);
}

void ICACHE_FLASH_ATTR networkConnectedCb(void *arg) 
{
#if DEBUG_OUTPUT
  uart0_tx_buffer("conn",4);
#endif
  struct espconn *conn=(struct espconn *)arg;
}

void ICACHE_FLASH_ATTR networkReconCb(void *arg, sint8 err) 
{
#if DEBUG_OUTPUT
  uart0_tx_buffer("rcon",4);
#endif
  network_init();
}

void ICACHE_FLASH_ATTR networkDisconCb(void *arg) 
{
#if DEBUG_OUTPUT
  uart0_tx_buffer("dcon",4);
#endif
}


void ICACHE_FLASH_ATTR network_start() 
{
  static struct espconn conn;
  static esp_tcp tcp;
  uint32_t target = ipaddr_addr(SERVERIP);
    
  pconn = &conn;
  
  
#if DEBUG_OUTPUT
  uart0_tx_buffer("look",4);
#endif
  conn.type=ESPCONN_TCP;
  conn.state=ESPCONN_NONE;
  conn.proto.tcp=&tcp;
  conn.proto.tcp->local_port=espconn_port();
  conn.proto.tcp->remote_port=SERVERPORT;

  
#if DEBUG_OUTPUT
  char page_buffer[20];
  os_sprintf(page_buffer,"IP: %d.%d.%d.%d",IP2STR(&target));
  uart0_tx_buffer(page_buffer,strlen(page_buffer));
#endif
  os_memcpy(conn.proto.tcp->remote_ip, &target, 4);
  espconn_regist_connectcb(&conn, networkConnectedCb);
  espconn_regist_disconcb(&conn, networkDisconCb);
  espconn_regist_reconcb(&conn, networkReconCb);
  espconn_regist_recvcb(&conn, networkRecvCb);
  espconn_regist_sentcb(&conn, networkSentCb);
  int  iRet = espconn_connect(&conn);  

#if DEBUG_OUTPUT
  os_sprintf(page_buffer,"\nConnected =0: %d\n\n",iRet);
  uart0_tx_buffer(page_buffer,strlen(page_buffer));
#endif
}

void ICACHE_FLASH_ATTR network_check_ip(void) 
{
  
  struct ip_info ipconfig;
  os_timer_disarm(&network_timer);
  
  wifi_get_ip_info(STATION_IF, &ipconfig);
  
  if (wifi_station_get_connect_status() == STATION_GOT_IP && ipconfig.ip.addr != 0) 
  {
   
    
#if DEBUG_OUTPUT
    char page_buffer[20];
    os_sprintf(page_buffer,"IP: %d.%d.%d.%d",IP2STR(&ipconfig.ip));
    uart0_tx_buffer(page_buffer,strlen(page_buffer));
#endif
    os_timer_disarm(&led_timer);
    os_timer_setfn(&led_timer, (os_timer_func_t *)LedTimer, NULL);
    os_timer_arm(&led_timer, 500, 1);
    
    network_start();
  } 
  else 
  {
 	  
#if DEBUG_OUTPUT
    uart0_tx_buffer("!!NOIP!!",8); 
#endif
    os_timer_disarm(&led_timer);
    os_timer_setfn(&led_timer, (os_timer_func_t *)LedTimer, NULL);
    os_timer_arm(&led_timer, 2000, 1);

    os_timer_setfn(&network_timer, (os_timer_func_t *)network_check_ip, NULL);
    os_timer_arm(&network_timer, 1000, 0);
  }
}

void ICACHE_FLASH_ATTR network_init() 
{
  
#if DEBUG_OUTPUT
  uart0_tx_buffer("net init",8);
#endif
  os_timer_disarm(&network_timer);
  os_timer_setfn(&network_timer, (os_timer_func_t *)network_check_ip, NULL);
  os_timer_arm(&network_timer, 1000, 0);
}

