#include "debug_uart.h"

/******************************************************************************/
#define DEBUG_SHELL_TASK_STK_SIZE 	(512)
static  PORT_STACK 				debug_shell_task_stk[DEBUG_SHELL_TASK_STK_SIZE];
static  RAW_TASK_OBJ 			debug_shell_task_obj;
/******************************************************************************/

/******************************************************************************/

#define DEBUG_FIFO_DEPTH 			(4*16)
#define DEBUG_FIFO_BLOCK_SIZE 		4
DEFINE_FIFO(debug_rx, DEBUG_FIFO_DEPTH, DEBUG_FIFO_BLOCK_SIZE);

RAW_MUTEX debug_tx_mutex;

/******************************************************************************/
static void debug_usart1_init(void);
static void debug_usart2_init(void);
static void debug_usart3_init(void);

static void debug_uart_init(void)
{
    USART_DeInit(DEBUG_UART);
    
    if(DEBUG_UART == USART1)        debug_usart1_init();
    else if(DEBUG_UART == USART2)   debug_usart2_init();
    else if(DEBUG_UART == USART3)   debug_usart3_init();

}

static void debug_usart1_init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_AFIO, ENABLE);
    Pub_Gpio_Output_AFPP(GPIOA, GPIO_Pin_9);
    Pub_Gpio_Input_INFLOATING(GPIOA, GPIO_Pin_10);
    Pub_Uart_Hardware_Cfg(USART1,DEBUG_BaudRate);
    Pub_Nvic_Config(USART1_IRQn, 0, 0);
    Pub_Uart_Int_Cfg(USART1,ENABLE);
}

static void debug_usart2_init(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    Pub_Gpio_Output_AFPP(GPIOA, GPIO_Pin_2);
    Pub_Gpio_Input_INFLOATING(GPIOA, GPIO_Pin_3);
    Pub_Uart_Hardware_Cfg(USART2,DEBUG_BaudRate);
    Pub_Nvic_Config(USART2_IRQn, 0, 0);
    Pub_Uart_Int_Cfg(USART2,ENABLE);
}

static void debug_usart3_init(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    Pub_Gpio_Output_AFPP(GPIOB, GPIO_Pin_10);
    Pub_Gpio_Input_INFLOATING(GPIOB, GPIO_Pin_11);
    Pub_Uart_Hardware_Cfg(USART3,DEBUG_BaudRate);
    Pub_Nvic_Config(USART3_IRQn, 0, 0);
    Pub_Uart_Int_Cfg(USART3,ENABLE);
}

static int debug_fifo_read(unsigned char *buf)
{
	int len = 0;
	
	if ( buf )
	{
		while ((DEBUG_UART->SR & DEBUG_UART_RXNE))
		{
			buf[len++] = (DEBUG_UART->DR & 0xFF);
		}
	}
	else
	{
		while ((DEBUG_UART->SR & DEBUG_UART_RXNE))
		{
			len = (DEBUG_UART->DR & 0xFF);
		}
		len = 0;
	}
	
	return len;
}

static inline void debug_uart_int_recv(void)
{
	serial_fifo_int_recv(&debug_rx_fifo, debug_fifo_read);
}

void DEBUG_UART_IRQ_HANDLER(void)
{
	NVIC_DisableIRQ(DEBUG_UART_IRQ);
	raw_enter_interrupt();
	
	/* USART in Receiver mode */
	if (USART_GetITStatus(DEBUG_UART, USART_IT_RXNE) == SET)
	{
		debug_uart_int_recv();
	}
	/* USART in Tramitter mode */
	if (USART_GetITStatus(DEBUG_UART, USART_IT_TXE) == SET)
	{

	}
	
	NVIC_EnableIRQ(DEBUG_UART_IRQ);
	raw_finish_int();
}

/******************************************************************************/

static void debug_fifo_init(void)
{
	RAW_U16 ret;
	
	ret =  raw_mutex_create(&debug_tx_mutex, (RAW_U8 *)"debug_tx_mutex", RAW_MUTEX_INHERIT_POLICY, 0);
	RAW_ASSERT(RAW_SUCCESS == ret);
	
	FIFO_INIT(debug_rx, DEBUG_FIFO_DEPTH, DEBUG_FIFO_BLOCK_SIZE);
}	

/**
 * 缓存为空返回 1
 */
int debug_cahce_empty(void)
{
	MSG_SIZE_TYPE msg[3];
	
	if(0 == debug_rx_fifo.count)
	{
		if ( RAW_SUCCESS != raw_queue_size_get_information(&debug_rx_fifo.fifo, msg, msg + 1, msg + 2))
		{
			RAW_ASSERT(0);
		}
		
		if( 0 == msg[2] )
		{
			return 1;
		}
	}
	
	return 0;
}

#if 0
static inline void debug_putchar(unsigned char c)
{
	while( !(DEBUG_UART->SR & DEBUG_UART_TXE) );
	DEBUG_UART->DR = c;
}
#endif

void debug_putc(char c)
{
	debug_putchar(c);
}

int debug_write(const void *buff, int len)
{
	const unsigned char *buf = buff;
	int i;
	RAW_U16 ret;

	ret = raw_mutex_get(&debug_tx_mutex, RAW_WAIT_FOREVER);
	if( (RAW_SUCCESS != ret) && (RAW_MUTEX_OWNER_NESTED != ret) )
	{
		return 0;
	}
	
	for(i=0; i<len; i++)
	{
		debug_putchar(buf[i]);
	}
	
	ret = raw_mutex_put(&debug_tx_mutex);
	RAW_ASSERT(RAW_SUCCESS == ret);
	
	return i;
}

int debug_read(void *buff, int len)
{
	return serial_read(&debug_rx_fifo, buff, len);
}
int debug_read_at_least_one(void *buf, int len, RAW_TICK_TYPE wait_opt)
{
	return serial_read_at_least_one(&debug_rx_fifo, wait_opt, buf, len);
}
int debug_read_noblock(void *buf, int len, RAW_TICK_TYPE wait_opt)
{
	return serial_read_noblock(&debug_rx_fifo, wait_opt, buf, len);
}
/******************************************************************************/

RAW_TASK_OBJ *get_shell_task_obj(void)
{
	return &debug_shell_task_obj;
}

void debug_serial_init(void)
{
	debug_uart_init();
	debug_fifo_init();
}


static void debug_shell_task(void *pdat)
{	
	(void)pdat;
	
	raw_printf("shell start...\r\t\t\t\t");
	cli_init();
	raw_printf("[OK]\n");
	
	raw_task_suspend(raw_task_identify());	// 任务挂起, 等待sys_init唤醒

	for(;;)
	{
		cli_loop();
	}
}

void shell_init(RAW_U8 prio)
{
	raw_task_create(&debug_shell_task_obj, 				/* 任务控制块地址 	*/
					(RAW_U8  *)"shell_daemon", 			/* 任务名 			*/
					(void *)0,							/* 任务参数 		*/
					prio, 								/* 优先级 			*/
					0,									/* 时间片 			*/
					debug_shell_task_stk,				/* 任务栈首地址 	*/
					DEBUG_SHELL_TASK_STK_SIZE ,			/* 任务栈大小 		*/
					debug_shell_task,					/* 任务入口地址 	*/
					1);									/* 是否立即运行 	*/	
}
