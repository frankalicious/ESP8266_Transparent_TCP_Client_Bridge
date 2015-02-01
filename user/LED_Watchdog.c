//#include "espmissingincludes.h"
#include "ets_sys.h"
//#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
//#include "user_config.h"
//#include <user_interface.h>
#include "c_types.h"
#include "LED_Watchdog.h"


//Timer event.
void ICACHE_FLASH_ATTR LedTimer(void *arg)
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

