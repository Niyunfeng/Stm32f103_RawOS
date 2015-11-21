#ifndef _SYS_UART_H_
#define _SYS_UART_H_

#include "stm32f10x.h"
#include "raw_api.h"
#include "fifo.h"
#include "string.h"
#include "stdio.h"
#include "pub.h"

#define Sys_SerialPort       2

#define SYS_UART_BAUDRATE  115200

#if Sys_SerialPort == 1

    #define SYS_UARTX 				USART1
    
    #define SYS_UARTX_IRQ 			USART1_IRQn
    #define SYS_UARTX_IRQ_HANDLER 	USART1_IRQHandler

#elif Sys_SerialPort == 2

    #define SYS_UARTX 				USART2

    #define SYS_UARTX_IRQ 			USART2_IRQn
    #define SYS_UARTX_IRQ_HANDLER 	USART2_IRQHandler

#elif Sys_SerialPort == 3

    #define SYS_UARTX 				USART3

    #define SYS_UARTX_IRQ 			USART3_IRQn
    #define SYS_UARTX_IRQ_HANDLER 	USART3_IRQHandler

#endif

#define SYS_UART_TXE        (1<<7)
#define SYS_UART_RXNE 		(1<<5)

//#define	ENTER_KEY	0x0d
//#define	BACK_KEY	0x08
//#define BEEP_KEY	0x07
//#define UP_KEY		0x41
//#define DOWN_KEY	0x42
//#define RIGHT_KEY	0x43
//#define LEFT_KEY	0x44
//#define HOME_KEY	0x48
//#define END_KEY		0x4b
//#define	ESC_KEY		0x1b

void USARTx_SendBuf(USART_TypeDef* USARTx, RAW_U8 * buf, RAW_U16 len);
void Uartx_RxInt(void);
RAW_U32 Uartx_Receive_T(RAW_U8 *buffer, RAW_U32 size, RAW_U32 timeout);
int GetCmd(RAW_S8 *str, RAW_S32 cnt, RAW_U32 timeout);

void Uart_printf(uint8_t *Data,...);



#endif

