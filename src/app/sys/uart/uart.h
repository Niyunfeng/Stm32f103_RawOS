#ifndef _UART_H_
#define _UART_H_

#include "pub.h"
#include "raw_api.h"

//串口配置，3个串口同时只支持一个
#define TX_LENGTH      128 //发送数组大小
#define RX_LENGTH      128 //接收数组大小

typedef struct
{
    u8 RxdBuf[RX_LENGTH];  //串口接收缓冲
    u8 TxdBuf[TX_LENGTH];  //串口发送缓冲
    vu16 RxdIndex;         //串口接收数据个数
}_DEF_Uart;

extern u8 RxdBuf[RX_LENGTH];
extern _DEF_Uart Usart;

#define UART_PORT       1       //定义串口号

#define UART_BAUDRATE  115200   //定义串口波特率

#if UART_PORT == 1
    #define UARTX 				    USART1
    #define UARTX_IRQ 			    USART1_IRQn
    #define UARTX_DMA_IRQ           DMA1_Channel4_IRQn
    #define UARTX_DMA_Channel       DMA1_Channel4
#elif UART_PORT == 2
    #define UARTX 				    USART2
    #define UARTX_IRQ 			    USART2_IRQn
    #define UARTX_DMA_IRQ           DMA1_Channel7_IRQn
    #define UARTX_DMA_Channel       DMA1_Channel7 
#elif UART_PORT == 3
    #define UARTX 				    USART3
    #define UARTX_IRQ 			    USART3_IRQn
    #define UARTX_DMA_IRQ           DMA1_Channel2_IRQn
    #define UARTX_DMA_Channel       DMA1_Channel2    
#endif

void USARTx_SendBuf(u8* data,uint16_t len);

#endif

