#ifndef __DEBUG_UART_H__
#define __DEBUG_UART_H__

#include "stm32f10x.h"
#include "raw_api.h"
#include "serial_fifo.h"
#include "cli.h"
#include "pub.h"

//使用具体的串口
#define DEBUG_SER              1
//定义串口波特率
#define DEBUG_BaudRate          115200

#define DEBUG_UART_TXE 			(1<<7)
#define DEBUG_UART_RXNE 		(1<<5)

#if DEBUG_SER == 1

    #define DEBUG_UART 				USART1
    
    #define DEBUG_UART_IRQ 			USART1_IRQn
    #define DEBUG_UART_IRQ_HANDLER 	USART1_IRQHandler

#elif DEBUG_SER == 2

    #define DEBUG_UART 				USART2

    #define DEBUG_UART_IRQ 			USART2_IRQn
    #define DEBUG_UART_IRQ_HANDLER 	USART2_IRQHandler

#elif DEBUG_SER == 3

    #define DEBUG_UART 				USART3

    #define DEBUG_UART_IRQ 			USART3_IRQn
    #define DEBUG_UART_IRQ_HANDLER 	USART3_IRQHandler


#endif
void debug_serial_init(void);
RAW_TASK_OBJ *get_shell_task_obj(void);
void shell_init(unsigned char prio);

static inline void debug_putchar(unsigned char c)
{
	while( !(DEBUG_UART->SR & DEBUG_UART_TXE) );
	DEBUG_UART->DR = c;
}

void debug_putc(char c);
int debug_write(const void *buff, int len);

int debug_read(void *buff, int len);
int debug_read_at_least_one(void *buf, int len, RAW_TICK_TYPE wait_opt);
int debug_read_noblock(void *buf, int len, RAW_TICK_TYPE wait_opt);

int debug_cahce_empty(void);

#endif
