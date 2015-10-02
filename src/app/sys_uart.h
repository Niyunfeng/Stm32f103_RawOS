#ifndef _SYS_UART_H_
#define _SYS_UART_H_

#include "stm32f10x.h"
#include "raw_api.h"
#include "fifo.h"
#include "string.h"
#include "stdio.h"

#define	ENTER_KEY	0x0d
#define	BACK_KEY	0x08
#define BEEP_KEY	0x07
#define UP_KEY		0x41
#define DOWN_KEY	0x42
#define RIGHT_KEY	0x43
#define LEFT_KEY	0x44
#define HOME_KEY	0x48
#define END_KEY		0x4b
#define	ESC_KEY		0x1b

static void USART_Configuration(u32 bound);
void USART_SendBuf(USART_TypeDef* USARTx, RAW_U8 * buf, RAW_U16 len);
void Uart1_RxInt(void);
RAW_U32 Uart_Receive_T(RAW_U8 *buffer, RAW_U32 size, RAW_U32 timeout);
static RAW_U8 command_length_get(const RAW_S8 *command);
int GetCmd(RAW_S8 *str, RAW_S32 cnt, RAW_U32 timeout);

static char *itoa(int value, char *string, int radix);
void Uart_printf(uint8_t *Data,...);



#endif

