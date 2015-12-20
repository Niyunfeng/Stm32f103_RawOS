#include "pub.h"

/*-----------------------------------------------------------
** 功能描述: 设置嵌入式中断的优先级
** 入口参数: Irq_Chanl  :中断号
             Preemp     :抢占优先级
             Sub        :相应优先级
** 出口参数: 无         
-------------------------------------------------------------*/
void Pub_Nvic_Config(uint8_t Irq_Chanl,uint8_t Preemp,uint8_t Sub)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	
	NVIC_InitStructure.NVIC_IRQChannel 					= Irq_Chanl;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority= Preemp;	//主优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority 		= Sub;		//从优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd 				= ENABLE;

	NVIC_Init(&NVIC_InitStructure);
}

/*-----------------------------------------------------------
** 功能描述: 设置端口为推挽输出口
** 入口参数: GPIOx  :端口号
             Pin    :引脚号
** 出口参数: 无
-------------------------------------------------------------*/
void Pub_Gpio_Output_OUTPP(GPIO_TypeDef* GPIOx,uint16_t Pin)
{
	GPIO_InitTypeDef tmp;
	tmp.GPIO_Pin	= Pin;
	tmp.GPIO_Mode	= GPIO_Mode_Out_PP; //推挽输出
	tmp.GPIO_Speed	= GPIO_Speed_50MHz;
	GPIO_Init(GPIOx,&tmp);
}

/*-----------------------------------------------------------
** 功能描述: 设置端口为复用推挽输出口
** 入口参数: GPIOx  :端口号
             Pin    :引脚号
** 出口参数: 无
-------------------------------------------------------------*/
void Pub_Gpio_Output_AFPP(GPIO_TypeDef* GPIOx,uint16_t Pin)
{
	GPIO_InitTypeDef tmp;
	tmp.GPIO_Pin	= Pin;
	tmp.GPIO_Mode	= GPIO_Mode_AF_PP;   //复用推挽输出
	tmp.GPIO_Speed	= GPIO_Speed_50MHz;
	GPIO_Init(GPIOx,&tmp);
}

/*-----------------------------------------------------------
** 功能描述: 设置端口为开漏输出口
** 入口参数: GPIOx  :端口号
             Pin    :引脚号
** 出口参数: 无
-------------------------------------------------------------*/
void Pub_Gpio_Output_OD(GPIO_TypeDef* GPIOx,uint16_t Pin)
{
	GPIO_InitTypeDef tmp;
	tmp.GPIO_Pin	= Pin;
	tmp.GPIO_Mode	= GPIO_Mode_Out_OD;   //复用推挽输出
	tmp.GPIO_Speed	= GPIO_Speed_50MHz;
	GPIO_Init(GPIOx,&tmp);
}

/*-----------------------------------------------------------
** 功能描述: 设置端口为模拟输入口
** 入口参数: GPIOx  :端口号
             Pin    :引脚号
** 出口参数: 无
-------------------------------------------------------------*/
void Pub_Gpio_Input_AIN(GPIO_TypeDef* GPIOx,uint16_t Pin)
{
	GPIO_InitTypeDef tmp;
	tmp.GPIO_Pin	=Pin;
	tmp.GPIO_Mode	=GPIO_Mode_AIN; //模拟输入
	GPIO_Init(GPIOx,&tmp);
}

/*-----------------------------------------------------------
** 功能描述: 设置端口为下拉输入口
** 入口参数: GPIOx  :端口号
             Pin    :引脚号
** 出口参数: 无
-------------------------------------------------------------*/
void Pub_Gpio_Input_IPD(GPIO_TypeDef* GPIOx,uint16_t Pin)
{
	GPIO_InitTypeDef tmp;
	tmp.GPIO_Pin	=Pin;
	tmp.GPIO_Mode	=GPIO_Mode_IPD; //下拉输入
	GPIO_Init(GPIOx,&tmp);
}

/*-----------------------------------------------------------
** 功能描述: 设置端口为浮空输入口
** 入口参数: GPIOx  :端口号
             Pin    :引脚号
** 出口参数: 无
-------------------------------------------------------------*/
void Pub_Gpio_Input_INFLOATING(GPIO_TypeDef* GPIOx,uint16_t Pin)
{
	GPIO_InitTypeDef tmp;
	tmp.GPIO_Pin	=Pin;
	tmp.GPIO_Mode	=GPIO_Mode_IN_FLOATING; //浮空输入
	GPIO_Init(GPIOx,&tmp);
}

/*-----------------------------------------------------------
** 功能描述: 设置端口为上拉输入口
** 入口参数: GPIOx  :端口号
             Pin    :引脚号
** 出口参数: 无
-------------------------------------------------------------*/
void Pub_Gpio_Input_IPU(GPIO_TypeDef* GPIOx,uint16_t Pin)
{
	GPIO_InitTypeDef tmp;
	tmp.GPIO_Pin	=Pin;
	tmp.GPIO_Mode	=GPIO_Mode_IPU; //上拉输入
	GPIO_Init(GPIOx,&tmp);
}

/*-----------------------------------------------------------
** 功能描述: 设置串口的硬件配置脚
** 入口参数: USARTx :串口号
             Bsp    :波特率
** 出口参数: 无
-------------------------------------------------------------*/
void Pub_Uart_Hardware_Cfg(USART_TypeDef* USARTx,uint32_t Bps)
{
	USART_InitTypeDef 	USART_InitStructure;  		//串口初始化结构体声明
    USART_ClockInitTypeDef USART_ClockInitStructure;
	
	USART_InitStructure.USART_BaudRate 				= Bps;
	USART_InitStructure.USART_WordLength 			= USART_WordLength_8b;
	USART_InitStructure.USART_StopBits 				= USART_StopBits_1;
	USART_InitStructure.USART_Parity 				= USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl 	= USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode 					= USART_Mode_Rx | USART_Mode_Tx;
	
    USART_ClockInitStructure.USART_Clock            = USART_Clock_Disable;
    USART_ClockInitStructure.USART_CPOL             = USART_CPOL_Low;
    USART_ClockInitStructure.USART_CPHA             = USART_CPHA_2Edge;
    USART_ClockInitStructure.USART_LastBit          = USART_LastBit_Disable;
    
    USART_ClockInit(USARTx, &USART_ClockInitStructure);
	USART_Init(USARTx, &USART_InitStructure);
}



/*-----------------------------------------------------------
** 功能描述: 设置串口的中断配置
** 入口参数: USARTx :串口号
** 出口参数: 无
-------------------------------------------------------------*/
void Pub_Uart_Int_Cfg(USART_TypeDef* USARTx,FunctionalState NewState)
{
    USART_ITConfig(USARTx, USART_IT_RXNE , NewState);	    //开启串口接收中断
    
    //USART_ITConfig(USARTx, USART_IT_IDLE , ENABLE);	//开启空闲,帧错,噪声,校验错中断 
	USART_Cmd(USARTx, ENABLE); 						    //打开串口
    //CPU的小缺陷：串口配置好，如果直接Send，则第1个字节发送不出去
    //如下语句解决第1个字节无法正确发送出去的问题 
  	USART_ClearFlag(USARTx, USART_FLAG_TC);     	    // 清发送完成标志，Transmission Complete flag 
}

