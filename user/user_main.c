#include "espmissingincludes.h"
#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "user_config.h"
#include <user_interface.h>
#include "driver/uart.h"

#include "c_types.h"
#include "espconn.h"
#include "mem.h"

#define recvTaskPrio        0
#define recvTaskQueueLen    64

os_event_t recvTaskQueue[recvTaskQueueLen];

static struct espconn *pconn = NULL;

static void ICACHE_FLASH_ATTR networkConnectedCb(void *arg);
static void ICACHE_FLASH_ATTR networkDisconCb(void *arg);
static void ICACHE_FLASH_ATTR networkReconCb(void *arg, sint8 err);
static void ICACHE_FLASH_ATTR networkRecvCb(void *arg, char *data, unsigned short len);
static void ICACHE_FLASH_ATTR networkSentCb(void *arg);
void ICACHE_FLASH_ATTR network_init();

LOCAL os_timer_t network_timer;
static os_timer_t led_timer;
uint8_t ledon = 0;


static void ICACHE_FLASH_ATTR recvTask(os_event_t *events)
{
	uint8_t c, i;
  char ch[1000];
  c = 0;
  i = 0;
  
  //uart0_tx_buffer("uart",4);
  
	while (READ_PERI_REG(UART_STATUS(UART0)) & (UART_RXFIFO_CNT << UART_RXFIFO_CNT_S))
	{
		WRITE_PERI_REG(0X60000914, 0x73); //WTD
		c = READ_PERI_REG(UART_FIFO(UART0)) & 0xFF; 
		
		ch[i] = c;
		i++;
  }

	if(UART_RXFIFO_FULL_INT_ST == (READ_PERI_REG(UART_INT_ST(UART0)) & UART_RXFIFO_FULL_INT_ST))
	{
		WRITE_PERI_REG(UART_INT_CLR(UART0), UART_RXFIFO_FULL_INT_CLR);
	}
	else if(UART_RXFIFO_TOUT_INT_ST == (READ_PERI_REG(UART_INT_ST(UART0)) & UART_RXFIFO_TOUT_INT_ST))
	{
		WRITE_PERI_REG(UART_INT_CLR(UART0), UART_RXFIFO_TOUT_INT_CLR);
	}
	ETS_UART_INTR_ENABLE();
	
	
	if (pconn && i != 0) 
	{
	  espconn_sent(pconn, ch, i);
	}		
}

//Timer event.
static void ICACHE_FLASH_ATTR 
LedTimer(void *arg)
{

    //Do blinky stuff
    if (ledon)
    {
        //Set GPIO2 to HIGH
        gpio_output_set(BIT2, 0, BIT2, 0);
        ledon=0;
        //uart0_sendStr("H");
    }
    else
    {
        //Set GPIO2 to LOW
        gpio_output_set(0, BIT2, BIT2, 0);
        ledon=1;
        //uart0_sendStr("L");
    } 
}



static void ICACHE_FLASH_ATTR networkServerFoundCb(const char *name, ip_addr_t *ip, void *arg) 
{

}

static void ICACHE_FLASH_ATTR networkSentCb(void *arg) 
{
  //uart0_tx_buffer("sent",4);
}

static void ICACHE_FLASH_ATTR networkRecvCb(void *arg, char *data, unsigned short len) 
{
  //uart0_tx_buffer("recv",4);
  
  struct espconn *conn=(struct espconn *)arg;
  uart0_tx_buffer(data,len);
}

static void ICACHE_FLASH_ATTR networkConnectedCb(void *arg) 
{
  //uart0_tx_buffer("conn",4);
  struct espconn *conn=(struct espconn *)arg;
}

static void ICACHE_FLASH_ATTR networkReconCb(void *arg, sint8 err) {
  //uart0_tx_buffer("rcon",4);
  network_init();
}

static void ICACHE_FLASH_ATTR networkDisconCb(void *arg) {
  //uart0_tx_buffer("dcon",4);
}


void ICACHE_FLASH_ATTR network_start() {
  static struct espconn conn;
  static esp_tcp tcp;
  uint32_t target = ipaddr_addr(SERVERIP);
  int iRet;
  
  pconn = &conn;
  
  //uart0_tx_buffer("look",4);

  conn.type=ESPCONN_TCP;
  conn.state=ESPCONN_NONE;
  conn.proto.tcp=&tcp;
  conn.proto.tcp->local_port=espconn_port();
  conn.proto.tcp->remote_port=SERVERPORT;

  //char page_buffer[20];
  //os_sprintf(page_buffer,"IP: %d.%d.%d.%d",IP2STR(&target));
  //uart0_tx_buffer(page_buffer,strlen(page_buffer));

  os_memcpy(conn.proto.tcp->remote_ip, &target, 4);
  espconn_regist_connectcb(&conn, networkConnectedCb);
  espconn_regist_disconcb(&conn, networkDisconCb);
  espconn_regist_reconcb(&conn, networkReconCb);
  espconn_regist_recvcb(&conn, networkRecvCb);
  espconn_regist_sentcb(&conn, networkSentCb);
  iRet = espconn_connect(&conn);  

 //os_sprintf(page_buffer,"\nConected =0: %d\n\n",iRet);
 //uart0_tx_buffer(page_buffer,strlen(page_buffer));
 
}

void ICACHE_FLASH_ATTR network_check_ip(void) {
  
  struct ip_info ipconfig;
  os_timer_disarm(&network_timer);
  
  wifi_get_ip_info(STATION_IF, &ipconfig);
  if (wifi_station_get_connect_status() == STATION_GOT_IP && ipconfig.ip.addr != 0) {
    //char page_buffer[20];
    //os_sprintf(page_buffer,"IP: %d.%d.%d.%d",IP2STR(&ipconfig.ip));
    //uart0_tx_buffer(page_buffer,strlen(page_buffer));
    network_start();
  } else {
    os_printf("No ip found\n\r");
    os_timer_setfn(&network_timer, (os_timer_func_t *)network_check_ip, NULL);
    os_timer_arm(&network_timer, 1000, 0);
  }
}

void ICACHE_FLASH_ATTR network_init() {
  os_timer_disarm(&network_timer);
  os_timer_setfn(&network_timer, (os_timer_func_t *)network_check_ip, NULL);
  os_timer_arm(&network_timer, 1000, 0);
}

//Init function 
void ICACHE_FLASH_ATTR user_init() {

     char ssid[32] = SSID;
    char password[64] = SSID_PASSWORD;
    
    struct station_config stationConf;

    //Set station mode
    wifi_set_opmode( 0x1 );

    //Set ap settings
    os_memcpy(&stationConf.ssid, ssid, 32);
    os_memcpy(&stationConf.password, password, 64);
    
    wifi_station_set_config(&stationConf);

    
    uart_init(BIT_RATE_57600,BIT_RATE_57600);

    //uart0_tx_buffer("init",4);
 
 
    //Set GPIO2 to output mode
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
               
    //Set GPIO2 low
    gpio_output_set(0, BIT2, BIT2, 0);

	  //Timer LED
	  os_timer_disarm(&led_timer);
	  os_timer_setfn(&led_timer, (os_timer_func_t *)LedTimer, NULL);
	  os_timer_arm(&led_timer, 500, 1);
 
    os_event_t *queue = os_malloc(sizeof(os_event_t) * recvTaskQueueLen);
    
    system_os_task(recvTask, recvTaskPrio, queue, recvTaskQueueLen);
    
     
    network_init();  
}
