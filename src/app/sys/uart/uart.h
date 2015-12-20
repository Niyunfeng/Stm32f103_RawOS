#ifndef _UART_H_
#define _UART_H_

#include "pub.h"
#include "raw_api.h"

#define DataLength      128

typedef struct
{
    u8 TxdBuf[DataLength];  //串口发送缓冲
    u8 RxdBuf[DataLength];  //串口接收缓冲
    u8 RxdIndex;            //串口接收数据个数
    u8 RxdState;            //串口接收状态
}_DEF_Uart;

extern _DEF_Uart Uart1,Uart2,Uart3;

#define SerialPort       2

#define SYS_UART_BAUDRATE  115200

#if SerialPort == 1

    #define SYS_UARTX 				    USART1
    #define SYS_UARTX_IRQ 			    USART1_IRQn
    #define SYS_UARTX_IRQ_HANDLER 	    USART1_IRQHandler

    #define SYS_UARTX_DMA_IRQ           DMA1_Channel4_IRQn
    #define SYS_UARTX_DMA_Channel       DMA1_Channel4

    #define SYS_UARTX_TX_BUF            Usart1.TxdBuf
    #define SYS_UARTX_RX_BUF            Usart1.RxdBuf
    #define SYS_UARTX_RXD_INDEX         Usart1.RxdIndex
#elif SerialPort == 2

    #define SYS_UARTX 				    USART2
    #define SYS_UARTX_IRQ 			    USART2_IRQn
    #define SYS_UARTX_IRQ_HANDLER 	    USART2_IRQHandler
    
    #define SYS_UARTX_DMA_IRQ           DMA1_Channel7_IRQn
    #define SYS_UARTX_DMA_Channel       DMA1_Channel7
    
    #define SYS_UARTX_TX_BUF            Usart2.TxdBuf
    #define SYS_UARTX_RX_BUF            Usart2.RxdBuf 
    #define SYS_UARTX_RXD_INDEX         Usart2.RxdIndex    
#elif SerialPort == 3

    #define SYS_UARTX 				    USART3
    #define SYS_UARTX_IRQ 			    USART3_IRQn
    #define SYS_UARTX_IRQ_HANDLER 	    USART3_IRQHandler
        
    #define SYS_UARTX_DMA_IRQ           DMA1_Channel2_IRQn
    #define SYS_UARTX_DMA_Channel       DMA1_Channel2
    
    #define SYS_UARTX_TX_BUF            Usart3.TxdBuf
    #define SYS_UARTX_RX_BUF            Usart3.RxdBuf   
    #define SYS_UARTX_RXD_INDEX         Usart3.RxdIndex       
#endif

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

//void USARTx_SendBuf(USART_TypeDef* USARTx, RAW_U8 * buf, RAW_U16 len);
void USARTx_SendBuf(USART_TypeDef* USARTx,u8* data,uint16_t len);


#endif

