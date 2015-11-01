#include "sys_uart.h"
#include <stdarg.h>

#define SYS_UART_TASK_STK_SIZE		(512)
static	PORT_STACK					sys_uart_task_stk[SYS_UART_TASK_STK_SIZE];
static	RAW_TASK_OBJ				sys_uart_task_obj;

/*
串口接收相关
*/
RAW_U8 TmpBuf[128];             //临时存储区
volatile RAW_U8 *pRead=TmpBuf;  //串口fifo读指针
volatile RAW_U8 *pWrite=TmpBuf; //串口fifo读指针

/*
缓冲区
*/
RAW_SEMAPHORE Sem1;	
struct raw_fifo Fifo;
RAW_U8 Buff_Fifo[128];//fifo的大小

static void USART_Configuration(void);
static void sys_usart1_init(void);
static void sys_usart2_init(void);
static void sys_usart3_init(void);
static void sys_usart4_init(void);


static void USART_Configuration(void)
{
    USART_DeInit(SYS_UARTX);
    
    if(SYS_UARTX == USART1)        sys_usart1_init();
    else if(SYS_UARTX == USART2)   sys_usart2_init();
    else if(SYS_UARTX == USART3)   sys_usart3_init();
    else if(SYS_UARTX == UART4)    sys_usart4_init();
}

static void sys_usart1_init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_AFIO, ENABLE);
    Pub_Gpio_Output_AFPP(GPIOA, GPIO_Pin_9);
    Pub_Gpio_Input_INFLOATING(GPIOA, GPIO_Pin_10);
    Pub_Uart_Hardware_Cfg(USART1,SYS_UART_BAUDRATE);
    Pub_Nvic_Config(USART1_IRQn, 0, 0);
    Pub_Uart_Int_Cfg(USART1,ENABLE);
}

static void sys_usart2_init(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    Pub_Gpio_Output_AFPP(GPIOA, GPIO_Pin_2);
    Pub_Gpio_Input_INFLOATING(GPIOA, GPIO_Pin_3);
    Pub_Uart_Hardware_Cfg(USART2,SYS_UART_BAUDRATE);
    Pub_Nvic_Config(USART2_IRQn, 0, 0);
    Pub_Uart_Int_Cfg(USART2,ENABLE);
}

static void sys_usart3_init(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    Pub_Gpio_Output_AFPP(GPIOB, GPIO_Pin_10);
    Pub_Gpio_Input_INFLOATING(GPIOB, GPIO_Pin_11);
    Pub_Uart_Hardware_Cfg(USART3,SYS_UART_BAUDRATE);
    Pub_Nvic_Config(USART3_IRQn, 0, 0);
    Pub_Uart_Int_Cfg(USART3,ENABLE);
}

static void sys_usart4_init(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    Pub_Gpio_Output_AFPP(GPIOC, GPIO_Pin_10);
    Pub_Gpio_Input_INFLOATING(GPIOC, GPIO_Pin_11);
    Pub_Uart_Hardware_Cfg(UART4,SYS_UART_BAUDRATE);
    Pub_Nvic_Config(UART4_IRQn, 0, 0);
    Pub_Uart_Int_Cfg(UART4,ENABLE);    
    
}

//*------------------------------------------------------------------------------------------------
//* 函数名称 : USART_SendBuf
//* 功能描述 : 串口字符发送
//* 入口参数 : USARTx   :通过具体串口进行发送
//*			   buf      :发送缓冲
//*			   len      :发送缓冲长度
//* 出口参数 : 无
//*------------------------------------------------------------------------------------------------
void USARTx_SendBuf(USART_TypeDef* USARTx, RAW_U8 * buf, RAW_U16 len)
{
	RAW_U8 t = 0;
	for (t=0; t<len; t++)
	{
		while(USART_GetFlagStatus(USARTx, USART_FLAG_TC) == RESET); 
		//while ((USART1->SR&0X40) == 0);//判断串口发送标志位是否置位，置位标示发送完成
		USART_SendData(USARTx,buf[t]);
	}
}

//串口中断函数
void SYS_UARTX_IRQ_HANDLER(void)                	
{
	if(USART_GetITStatus(SYS_UARTX,USART_IT_RXNE)!= RESET)//中断产生
	{
        USART_ClearITPendingBit(SYS_UARTX, USART_IT_RXNE);
		Uartx_RxInt();
	}
}

//*------------------------------------------------------------------------------------------------
//* 函数名称 : Uart1_RxInt
//* 功能描述 : uart1 中断函数第二部分(semaphore + fifo)
//* 入口参数 : 无
//* 出口参数 : 无
//*------------------------------------------------------------------------------------------------
void Uartx_RxInt(void)
{
	RAW_U8 aa[128]={0};
	RAW_U8 *bb=aa;
	RAW_U8 len=0;

	*bb = SYS_UARTX->DR;
	bb++;
	len++;
	fifo_in(&Fifo, aa, len);
	raw_semaphore_put(&Sem1);
}


//*------------------------------------------------------------------------------------------------
//* 函数名称	: Uart_Receive_T
//* 功能描述	: uart1接收函数(queue)
//* 入口参数	: buffer:接收数据存放指针
//*				: cnt  :接收数据长度
//*				: timeout:接收超时参数
//* 出口参数	: 无
//*------------------------------------------------------------------------------------------------
RAW_U32 Uartx_Receive_T(RAW_U8 *buffer, RAW_U32 size, RAW_U32 timeout)
{
    RAW_U32 len=0;
	RAW_U32 tmplen=0;
	RAW_U16 errstat;

	while((pWrite - pRead) < size)
	{
	
		errstat = raw_semaphore_get(&Sem1, timeout);
		if(errstat == RAW_BLOCK_TIMEOUT)
			break;
		if(errstat == RAW_SUCCESS)
		{
			len = fifo_out_all(&Fifo, (void *)pWrite);			
			pWrite += len;
		}
	}
	if((pWrite - pRead) >= size)
	{
		for(;tmplen < size; tmplen++)
		{
			*buffer++ = *pRead++;
			if(pRead == pWrite)
			{
				pRead = TmpBuf;
				pWrite = TmpBuf;
			}
		}
	}

	return tmplen;
}
//*================================================================================================
//*------------------------------------------------------------------------------------------------
//*	function name			: GetCmd
//*	description				: wait for a command input
//*	arguments				: str->container of the command will be input
//*							: cnt->max size of the command 
//*							: timeout->timeout
//*	return					: the length of the command or -1(esc)
//*------------------------------------------------------------------------------------------------
int GetCmd(RAW_S8 *str, RAW_S32 cnt, RAW_U32 timeout)
{
	int i = 0; 
	RAW_U8 key;	
	RAW_U32 len;
    
	while(1)
	{
		len = Uartx_Receive_T(&key, 1, timeout);
		if (len == 1) {
			str[i++] = key;
		}
        else {
            return i;//返回接收数据长度
        }
	}	
}



#pragma import(__use_no_semihosting)             
               
struct __FILE 
{ 
	int handle; 

}; 


FILE __stdout;       
  
int _sys_exit(int x) 
{ 
	x = x; 

	return 0;
} 



/*
 * 函数名：fputc
 * 描述  ：重定向c库函数printf到USART1
 * 输入  ：无
 * 输出  ：无
 * 调用  ：由printf调用
 */
int fputc(int ch, FILE *f)
{
/* 将Printf内容发往串口 */
  USART_SendData(SYS_UARTX, (unsigned char) ch);
  while (!(SYS_UARTX->SR & USART_FLAG_TXE));
 
  return (ch);
}

/*
 * 函数名：itoa
 * 描述  ：将整形数据转换成字符串
 * 输入  ：-radix =10 表示10进制，其他结果为0
 *         -value 要转换的整形数
 *         -buf 转换后的字符串
 *         -radix = 10
 * 输出  ：无
 * 返回  ：无
 * 调用  ：被USART1_printf()调用
 */
static char *itoa(int value, char *string, int radix)
{
    int     i, d;
    int     flag = 0;
    char    *ptr = string;

    /* This implementation only works for decimal numbers. */
    if (radix != 10)
    {
        *ptr = 0;
        return string;
    }

    if (!value)
    {
        *ptr++ = 0x30;
        *ptr = 0;
        return string;
    }

    /* if this is a negative value insert the minus sign. */
    if (value < 0)
    {
        *ptr++ = '-';

        /* Make the value positive. */
        value *= -1;
    }

    for (i = 10000; i > 0; i /= 10)
    {
        d = value / i;

        if (d || flag)
        {
            *ptr++ = (char)(d + 0x30);
            value -= (d * i);
            flag = 1;
        }
    }

    /* Null terminate the string. */
    *ptr = 0;

    return string;

} /* NCL_Itoa */

/*
 * 函数名：USART1_printf
 * 描述  ：格式化输出，类似于C库中的printf，但这里没有用到C库
 * 输入  ：-USARTx 串口通道，这里只用到了串口1，即USART1
 *		     -Data   要发送到串口的内容的指针
 *			   -...    其他参数
 * 输出  ：无
 * 返回  ：无 
 * 调用  ：外部调用
 *         典型应用USART1_printf( USART1, "\r\n this is a demo \r\n" );
 *            		 USART1_printf( USART1, "\r\n %d \r\n", i );
 *            		 USART1_printf( USART1, "\r\n %s \r\n", j );
 */
void Uart_printf(uint8_t *Data,...)
{

  const char *s;
  int d;   
  char buf[16];

  va_list ap;
  va_start(ap, Data);

	while ( *Data != 0)     // 判断是否到达字符串结束符
	{				                          
		if ( *Data == 0x5c )  //'\'
		{									  
			switch ( *++Data )
			{
				case 'r':							          //回车符
					USART_SendData(SYS_UARTX, 0x0d);
					Data ++;
					break;

				case 'n':							          //换行符
					USART_SendData(SYS_UARTX, 0x0a);	
					Data ++;
					break;
				
				default:
					Data ++;
				    break;
			}			 
		}
		else if ( *Data == '%')
		{									  //
			switch ( *++Data )
			{				
				case 's':										  //字符串
					s = va_arg(ap, const char *);
          for ( ; *s; s++) 
					{
						USART_SendData(SYS_UARTX,*s);
						while( USART_GetFlagStatus(SYS_UARTX, USART_FLAG_TC) == RESET );
          }
					Data++;
          break;

        case 'd':										//十进制
          d = va_arg(ap, int);
          itoa(d, buf, 10);
          for (s = buf; *s; s++) 
					{
						USART_SendData(SYS_UARTX,*s);
						while( USART_GetFlagStatus(SYS_UARTX, USART_FLAG_TC) == RESET );
          }
					Data++;
          break;
				 default:
						Data++;
				    break;
			}		 
		} /* end of else if */
		else USART_SendData(SYS_UARTX, *Data++);
		while( USART_GetFlagStatus(SYS_UARTX, USART_FLAG_TC) == RESET );
	}

}

void sys_uart_task(void  * pdat)
{
    (void) pdat;
    
	RAW_S8 cmd[128];
    RAW_U8 len = 0;
    
    raw_printf("sys_usart init...\r\t\t\t\t");
	USART_Configuration();
    
	fifo_init(&Fifo, Buff_Fifo, 128);
	raw_semaphore_create(&Sem1,(RAW_U8 *)"sem1", 0);
    
    raw_printf("[OK]\n");
	
	for(;;)
	{
		len = GetCmd(cmd, 128, 10);//超时10毫秒，接收数据长度在128个字节
        if(len)//判断是否接收到数据
        {
            //if(cmd[0] == 0 && cmd[1] == 1)//判断数据有效性
                USARTx_SendBuf(SYS_UARTX, (RAW_U8 *)cmd, len);
        }
	}
}

void sys_uart_init(RAW_U8 prio)
{
	raw_task_create(&sys_uart_task_obj,
				   (RAW_U8 *)"sys_uart_task",
				   (void *)0,
				   prio,
				   0,
				   sys_uart_task_stk,
				   SYS_UART_TASK_STK_SIZE,
				   sys_uart_task,
				   1);
}
