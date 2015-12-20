#ifndef _DELAY_H
#define _DELAY_H

#include "raw_api.h"
#include "stm32f10x.h"

#define delay_osrunning         RAW_OS_RUNNING
#define delay_ostickspersec     RAW_TICKS_PER_SECOND
#define delay_osintnesting      raw_int_nesting

void delay_init(void);
void delay_us(u32 nus);
void delay_ms(u16 nms);


#endif
