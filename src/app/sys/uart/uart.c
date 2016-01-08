#include "uart.h"

#define SYS_UART_TASK_STK_SIZE		(512)
static	PORT_STACK					sys_uart_task_stk[SYS_UART_TASK_STK_SIZE];
static	RAW_TASK_OBJ				sys_uart_task_obj;

_DEF_Uart Usart;

RAW_SEMAPHORE RxdSem;   //接收完成信号量	


#if UART_PORT == 1
    static void USART1_Config(void);
    static void USART1_DMA_Configuration(void);
#elif UART_PORT == 2
    static void USART2_Config(void);
    static void USART2_DMA_Configuration(void);
#elif UART_PORT == 3
    static void USART3_Config(void);
    static void USART3_DMA_Configuration(void);
#endif

void USART_Configuration(void)
{
    #if UART_PORT == 1
        USART_DeInit(USART1);
        USART1_Config();
        USART1_DMA_Configuration();
    #elif UART_PORT == 2
        USART_DeInit(USART2);
        USART2_Config();
        USART2_DMA_Configuration();
    #elif UART_PORT == 3
        USART_DeInit(USART3);
        USART3_Config();
        USART3_DMA_Configuration();
    #endif  
}

/********************宏定义为串口1********************/
#if UART_PORT == 1

static void USART1_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure; 
	
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA,ENABLE);

    //Tx PA.2
    GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA,&GPIO_InitStructure);
    //Rx PA.3
    GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA,&GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate              = UART_BAUDRATE;
    USART_InitStructure.USART_WordLength            = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits              = USART_StopBits_1;
    USART_InitStructure.USART_Parity                = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl   = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode                  = USART_Mode_Rx| USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);

    /* Enable the USART Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel                      = USART1_IRQn;     // 串口中断配置
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority    = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority           = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd                   = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    USART_ITConfig(USART1, USART_IT_IDLE , ENABLE);
    USART_ITConfig(USART1, USART_IT_PE | USART_IT_ERR, ENABLE);     //开启PE错误接收中断,开启ERR中断
    USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);
    
    USART_Cmd(USART1, ENABLE);
}

void USART1_DMA_Configuration(void)
{
    DMA_InitTypeDef DMA_InitStructure;
    
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    
    DMA_DeInit(DMA1_Channel4);  
    DMA_InitStructure.DMA_PeripheralBaseAddr    = (u32)(&USART1->DR);       
    DMA_InitStructure.DMA_MemoryBaseAddr        = (uint32_t)Usart.TxdBuf;
    DMA_InitStructure.DMA_DIR                   = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize            = TX_LENGTH;
    DMA_InitStructure.DMA_PeripheralInc         = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc             = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize    = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize        = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode                  = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority              = DMA_Priority_VeryHigh;
    DMA_InitStructure.DMA_M2M                   = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel4, &DMA_InitStructure);
    
    DMA_Cmd(DMA1_Channel4, DISABLE);
    DMA_ITConfig(DMA1_Channel4, DMA_IT_TC, ENABLE);
    DMA_ClearFlag(DMA1_FLAG_TC4);
    
    /* DMA1 Channel5 (triggered by USART1 Rx event) Config */
    
    DMA_Cmd(DMA1_Channel5, DISABLE);
    DMA_DeInit(DMA1_Channel5);  
    
    DMA_InitStructure.DMA_PeripheralBaseAddr    = (u32)(&USART1->DR);
    DMA_InitStructure.DMA_MemoryBaseAddr        = (uint32_t)Usart.RxdBuf;
    DMA_InitStructure.DMA_DIR                   = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize            = RX_LENGTH;
    DMA_InitStructure.DMA_PeripheralInc         = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc             = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize    = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize        = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode                  = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority              = DMA_Priority_Medium;
    DMA_InitStructure.DMA_M2M                   = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel5, &DMA_InitStructure);
    
    DMA_ClearFlag(DMA1_FLAG_GL5);  
    
    DMA_Cmd(DMA1_Channel5, ENABLE);
}

/********************USART1串口空闲中断********************/
void USART1_IRQHandler(void)
{
    NVIC_DisableIRQ(USART1_IRQn);
    raw_enter_interrupt();
    
    uint8_t num = 0;
    if(USART_GetITStatus(USART1, USART_IT_IDLE) != RESET)  // 空闲中断
    {
        num = USART1->SR;
        num = USART1->DR;                       //清USART_IT_IDLE标志        
        DMA_Cmd(DMA1_Channel5, DISABLE);        // 关闭DMA ，防止干扰
        Usart.RxdIndex = RX_LENGTH -  DMA_GetCurrDataCounter(DMA1_Channel5);
        DMA1_Channel5->CNDTR = RX_LENGTH;      //重新设置接收数据个数
        DMA_Cmd(DMA1_Channel5,ENABLE);          //开启DMA
        raw_semaphore_put(&RxdSem);
    }
    if(USART_GetFlagStatus(USART1, USART_FLAG_ORE | USART_FLAG_NE | USART_FLAG_FE | USART_FLAG_PE) != RESET)
    {
        //同  @arg USART_IT_ORE_ER : OverRun Error interrupt if the EIE bit is set  
        //同  @arg USART_IT_NE     : Noise Error interrupt
        //同   @arg USART_IT_FE     : Framing Error interrupt
        //同  @arg USART_IT_PE     : Parity Error interrupt
        USART_ClearFlag(USART1, USART_FLAG_ORE | USART_FLAG_NE | USART_FLAG_FE | USART_FLAG_PE);
        uint8_t Temp = USART_ReceiveData(USART1); //取出来扔掉
    }
    
	NVIC_EnableIRQ(USART1_IRQn);   
    raw_finish_int();
}

/********************UART1_DMA发送中断********************/
void DMA1_Channel4_IRQHandler(void)
{
    NVIC_DisableIRQ(DMA1_Channel4_IRQn);
    raw_enter_interrupt();
    
    if(DMA_GetITStatus(DMA1_FLAG_TC4))
    {
        DMA_ClearFlag(DMA1_FLAG_TC4);       //清Usart1_DMA发送完成标志位
        DMA_Cmd(DMA1_Channel4, DISABLE);    //关闭DMA1_Channel4通道
    }

	NVIC_EnableIRQ(DMA1_Channel4_IRQn); 
    raw_finish_int();
}

/********************宏定义为串口2********************/
#elif UART_PORT == 2
static void USART2_Config(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure; 
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE); 	
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);			

    //Tx PA.2
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA,&GPIO_InitStructure);
    //Rx PA.3
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA,&GPIO_InitStructure);

    USART_DeInit(USART2);
    USART_InitStructure.USART_BaudRate              = UART_BAUDRATE;
    USART_InitStructure.USART_HardwareFlowControl   = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode                  = USART_Mode_Rx|USART_Mode_Tx;
    USART_InitStructure.USART_Parity                = USART_Parity_No;
    USART_InitStructure.USART_StopBits              = USART_StopBits_1;
    USART_InitStructure.USART_WordLength            = USART_WordLength_8b;
    USART_Init(USART2,&USART_InitStructure);
	
        /* Enable the USART Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel                      = USART2_IRQn;     // 串口中断配置
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority    = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority           = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd                   = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    USART_ITConfig(USART2, USART_IT_IDLE , ENABLE);
    USART_ITConfig(USART2, USART_IT_PE | USART_IT_ERR, ENABLE);     //开启PE错误接收中断,开启ERR中断
    USART_DMACmd(USART2, USART_DMAReq_Rx, ENABLE);
    
    USART_Cmd(USART2, ENABLE); 
}

void USART2_DMA_Configuration(void)
{
    DMA_InitTypeDef DMA_InitStructure;
    
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    
    DMA_DeInit(DMA1_Channel7);  
    DMA_InitStructure.DMA_PeripheralBaseAddr    = (u32)(&USART2->DR);       
    DMA_InitStructure.DMA_MemoryBaseAddr        = (uint32_t)Usart.TxdBuf;
    DMA_InitStructure.DMA_DIR                   = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize            = TX_LENGTH;
    DMA_InitStructure.DMA_PeripheralInc         = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc             = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize    = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize        = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode                  = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority              = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M                   = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel7, &DMA_InitStructure);
    
    DMA_Cmd(DMA1_Channel7, DISABLE);
    DMA_ITConfig(DMA1_Channel7, DMA_IT_TC, ENABLE);
    DMA_ClearFlag(DMA1_FLAG_TC7);
    
    /* DMA1 Channel6 (triggered by USART2 Rx event) Config */
    DMA_Cmd(DMA1_Channel6, DISABLE);
    DMA_DeInit(DMA1_Channel6);  
    
    DMA_InitStructure.DMA_PeripheralBaseAddr    = (u32)(&USART2->DR);
    DMA_InitStructure.DMA_MemoryBaseAddr        = (uint32_t)Usart.RxdBuf;
    DMA_InitStructure.DMA_DIR                   = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize            = RX_LENGTH;
    DMA_InitStructure.DMA_PeripheralInc         = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc             = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize    = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize        = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode                  = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority              = DMA_Priority_Medium;
    DMA_InitStructure.DMA_M2M                   = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel6, &DMA_InitStructure);
    
    DMA_ClearFlag(DMA1_FLAG_GL6);  
    
    DMA_Cmd(DMA1_Channel6, ENABLE);
}

//USART2串口空闲中断
void USART2_IRQHandler(void)
{
    NVIC_DisableIRQ(USART2_IRQn);
    raw_enter_interrupt();
    
    uint8_t num = 0;
    if(USART_GetITStatus(USART2, USART_IT_IDLE) != RESET)  // 空闲中断
    {
        num = USART2->SR;
        num = USART2->DR;                       //清USART_IT_IDLE标志        
        DMA_Cmd(DMA1_Channel6, DISABLE);        // 关闭DMA ，防止干扰
        Usart.RxdIndex = RX_LENGTH -  DMA_GetCurrDataCounter(DMA1_Channel6);
        DMA1_Channel6->CNDTR = RX_LENGTH;      //重新设置接收数据个数
        DMA_Cmd(DMA1_Channel6,ENABLE);          //开启DMA
        raw_semaphore_put(&RxdSem);
    }
    if(USART_GetFlagStatus(USART2, USART_FLAG_ORE | USART_FLAG_NE | USART_FLAG_FE | USART_FLAG_PE) != RESET)
    {
        //同  @arg USART_IT_ORE_ER : OverRun Error interrupt if the EIE bit is set  
        //同  @arg USART_IT_NE     : Noise Error interrupt
        //同   @arg USART_IT_FE     : Framing Error interrupt
        //同  @arg USART_IT_PE     : Parity Error interrupt
        USART_ClearFlag(USART2, USART_FLAG_ORE | USART_FLAG_NE | USART_FLAG_FE | USART_FLAG_PE);
        uint8_t Temp = USART_ReceiveData(USART2); //取出来扔掉
    }
    
	NVIC_EnableIRQ(USART2_IRQn);   
    raw_finish_int();
}

//UART2_DMA发送中断
void DMA1_Channel7_IRQHandler(void)
{
    NVIC_DisableIRQ(DMA1_Channel7_IRQn);
    raw_enter_interrupt();
    
    if(DMA_GetITStatus(DMA1_FLAG_TC7))
    {
        DMA_ClearFlag(DMA1_FLAG_TC7);      //清Usart2_DMA发送完成标志位
        DMA_Cmd(DMA1_Channel7, DISABLE);   //关闭DMA2_Channel7通道
    }

	NVIC_EnableIRQ(DMA1_Channel7_IRQn);    
    raw_finish_int();
}

/********************宏定义为串口3********************/
#elif UART_PORT == 3

static void USART3_Config(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure; 
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOB, ENABLE); 	
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);			

    //Tx PB.10
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB,&GPIO_InitStructure);
    //Rx PB.11
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB,&GPIO_InitStructure);

    USART_DeInit(USART3);
    USART_InitStructure.USART_BaudRate              = UART_BAUDRATE;
    USART_InitStructure.USART_HardwareFlowControl   = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode                  = USART_Mode_Rx|USART_Mode_Tx;
    USART_InitStructure.USART_Parity                = USART_Parity_No;
    USART_InitStructure.USART_StopBits              = USART_StopBits_1;
    USART_InitStructure.USART_WordLength            = USART_WordLength_8b;
    USART_Init(USART3,&USART_InitStructure);
	
        /* Enable the USART Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel                      = USART3_IRQn;     // 串口中断配置
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority    = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority           = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd                   = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    USART_ITConfig(USART3, USART_IT_IDLE , ENABLE);
    USART_ITConfig(USART3, USART_IT_PE | USART_IT_ERR, ENABLE);     //开启PE错误接收中断,开启ERR中断
    USART_DMACmd(USART3, USART_DMAReq_Rx, ENABLE);
    
    USART_Cmd(USART3, ENABLE); 
}

void USART3_DMA_Configuration(void)
{
    DMA_InitTypeDef DMA_InitStructure;
    
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    
    DMA_DeInit(DMA1_Channel2);  
    DMA_InitStructure.DMA_PeripheralBaseAddr    = (u32)(&USART3->DR);       
    DMA_InitStructure.DMA_MemoryBaseAddr        = (uint32_t)Usart.TxdBuf;
    DMA_InitStructure.DMA_DIR                   = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize            = TX_LENGTH;
    DMA_InitStructure.DMA_PeripheralInc         = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc             = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize    = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize        = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode                  = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority              = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M                   = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel2, &DMA_InitStructure);
    
    DMA_Cmd(DMA1_Channel2, DISABLE);
    DMA_ITConfig(DMA1_Channel2, DMA_IT_TC, ENABLE);
    DMA_ClearFlag(DMA1_FLAG_TC2);
    
    /* DMA1 Channel3 (triggered by USART3 Rx event) Config */
    
    DMA_Cmd(DMA1_Channel3, DISABLE);
    DMA_DeInit(DMA1_Channel3);  
    
    DMA_InitStructure.DMA_PeripheralBaseAddr    = (u32)(&USART3->DR);
    DMA_InitStructure.DMA_MemoryBaseAddr        = (uint32_t)Usart.RxdBuf;
    DMA_InitStructure.DMA_DIR                   = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize            = RX_LENGTH;
    DMA_InitStructure.DMA_PeripheralInc         = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc             = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize    = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize        = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode                  = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority              = DMA_Priority_Medium;
    DMA_InitStructure.DMA_M2M                   = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel3, &DMA_InitStructure);
    
    DMA_ClearFlag(DMA1_FLAG_GL3);  
    
    DMA_Cmd(DMA1_Channel3, ENABLE); 
}


void USART3_IRQHandler(void)
{
    NVIC_DisableIRQ(USART3_IRQn); 
    raw_enter_interrupt();
    
    uint8_t num = 0;
    if(USART_GetITStatus(USART3, USART_IT_IDLE) != RESET)   // 空闲中断
    {
        num = USART3->SR;
        num = USART3->DR;                                   //清USART_IT_IDLE标志
        DMA_Cmd(DMA1_Channel3, DISABLE);                    // 关闭DMA ，防止干扰
        Usart.RxdIndex = RX_LENGTH -  DMA_GetCurrDataCounter(DMA1_Channel3);   //获取串口接收数据个数
        DMA1_Channel3->CNDTR = RX_LENGTH;                  //重新设置接收数据个数
        DMA_Cmd(DMA1_Channel3,ENABLE);                      //开启DMA
        raw_semaphore_put(&RxdSem);
    }
    if(USART_GetFlagStatus(USART3, USART_FLAG_ORE | USART_FLAG_NE | USART_FLAG_FE | USART_FLAG_PE) != RESET)
    {
        //同  @arg USART_IT_ORE_ER : OverRun Error interrupt if the EIE bit is set  
        //同  @arg USART_IT_NE     : Noise Error interrupt
        //同  @arg USART_IT_FE     : Framing Error interrupt
        //同  @arg USART_IT_PE     : Parity Error interrupt
        USART_ClearFlag(USART3, USART_FLAG_ORE | USART_FLAG_NE | USART_FLAG_FE | USART_FLAG_PE);
        uint8_t Temp = USART_ReceiveData(USART3); //取出来扔掉
    }
    
	NVIC_EnableIRQ(USART3_IRQn); 
    raw_finish_int();    
}

//UART3_DMA发送中断
void DMA1_Channel2_IRQHandler(void)
{
    NVIC_DisableIRQ(DMA1_Channel2_IRQn);
    raw_enter_interrupt();
    
    if(DMA_GetITStatus(DMA1_FLAG_TC2))
    {
        DMA_ClearFlag(DMA1_FLAG_TC2);      //清Usart3_DMA发送完成标志位
        DMA_Cmd(DMA1_Channel2, DISABLE);   //关闭DMA1_Channel2通道
    }
    
	NVIC_EnableIRQ(DMA1_Channel2_IRQn);    
    raw_finish_int();
}
#endif  

void USARTx_SendBuf(u8* data,uint16_t len)
{
    vu16  i = 0;
      
    for(i=0;i<len;i++)
    {
        Usart.TxdBuf[i] = *(data++);
    }
    
    UARTX_DMA_Channel->CNDTR = len; 
    USART_DMACmd(UARTX, USART_DMAReq_Tx, ENABLE);
    DMA_Cmd(UARTX_DMA_Channel, ENABLE);    
}



void sys_uart_task(void  * pdat)
{
    (void) pdat;
    
    raw_printf("sys_usart init...\r\t\t\t\t");
	USART_Configuration();
    raw_semaphore_create(&RxdSem,(RAW_U8 *)"RxdSem", 0);
    raw_printf("[OK]\n");
	
	for(;;)
	{
        raw_semaphore_get(&RxdSem, RAW_WAIT_FOREVER);
        USARTx_SendBuf(Usart.RxdBuf,Usart.RxdIndex);//自发自收
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
