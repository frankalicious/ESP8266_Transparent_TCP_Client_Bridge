#ifndef __LED_WATCHDOG_H__
#define __LED_WATCHDOG_H__

#include <ip_addr.h>
#include <c_types.h>
#include "os_type.h"

static os_timer_t led_timer;

static uint8_t ledon = 0;

void ICACHE_FLASH_ATTR LedTimer (void *arg);

#endif /* __LED_WATCHDOG_H__ */



