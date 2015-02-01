#include "espmissingincludes.h"
#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "user_config.h"
#include <user_interface.h>
#include "driver/uart.h"

#include "c_types.h"
//#include <espconn.h>
#include "mem.h"

#include "LED_Watchdog.h"
#include "client.h"

#define recvTaskPrio        0
#define recvTaskQueueLen    64

os_event_t recvTaskQueue[recvTaskQueueLen];

extern struct espconn *pconn;

/*
static void ICACHE_FLASH_ATTR networkConnectedCb(void *arg);
static void ICACHE_FLASH_ATTR networkDisconCb(void *arg);
static void ICACHE_FLASH_ATTR networkReconCb(void *arg, sint8 err);
static void ICACHE_FLASH_ATTR networkRecvCb(void *arg, char *data, unsigned short len);
static void ICACHE_FLASH_ATTR networkSentCb(void *arg);
void ICACHE_FLASH_ATTR network_init();
*/


//Read from UART0(requires special uart.c!)
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
	
	// send to Server if available
	if (pconn && i != 0) 
	{
	  espconn_sent(pconn, ch, i);
	}		
}

void wifi_config()
{
// Wifi configuration
  char ssid[32]     = SSID;
  char password[64] = SSID_PASSWORD;
  
  struct station_config stationConf;

  //Set station mode
  wifi_set_opmode(0x1);

  //Set ap settings
  os_memcpy(&stationConf.ssid, ssid, 32);
  os_memcpy(&stationConf.password, password, 64);
  
  stationConf.bssid_set = 0;
          
  wifi_station_set_config(&stationConf);
  wifi_station_set_auto_connect(1); 
}


//Init function 
void ICACHE_FLASH_ATTR user_init() 
{
   
    uart_init(BIT_RATE_57600,BIT_RATE_57600);

    //Set GPIO2 to output mode
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
               
    //Set GPIO2 low
    gpio_output_set(0, BIT2, BIT2, 0);

    
    /*############ RESET SEQUENZE START #####################*/
    //Set GPIO0 to output mode
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0);
                   
    //Set GPIO0 low
    gpio_output_set(0, BIT0, BIT0, 0);
  
    os_delay_us(250000); //250ms = 0,25sec! 
            
    //Set GPIO0 high
    gpio_output_set(BIT0, 0, BIT0, 0);
    
    os_delay_us(250000); //250ms = 0,25sec!
    /*############ RESET SEQUENZE END #######################*/
        
	  //Timer LED
	  os_timer_disarm(&led_timer);
	  os_timer_setfn(&led_timer, (os_timer_func_t *)LedTimer, NULL);
	  os_timer_arm(&led_timer, 500, 1);
 
    os_event_t *queue = os_malloc(sizeof(os_event_t) * recvTaskQueueLen);
    
    system_os_task(recvTask, recvTaskPrio, queue, recvTaskQueueLen); 
 
    wifi_config();
        
    network_init();  
}
