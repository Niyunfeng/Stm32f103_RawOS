#include "pub.h"
#include "uart.h"

static __inline void RCC_Configuration(void)
{
	ErrorStatus HSEStartUpStatus;
    /*RCC system reset(for debug purpose) */
	RCC_DeInit(); 					    		//时钟系统复位
    /*Enable HSE */
  	RCC_HSEConfig(RCC_HSE_ON);	    			//开启HSE时钟
    /*Wait till HSE is ready */
  	HSEStartUpStatus = RCC_WaitForHSEStartUp(); //等待HSE时钟
  	if(HSEStartUpStatus == SUCCESS)             //如果HSE时间正常
  	{
                                                //使能预读取缓冲区
    	FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
		FLASH_SetLatency(FLASH_Latency_2);      //Flash等待状态
  
    	RCC_HCLKConfig(RCC_SYSCLK_Div1);	    //配置AHB(HCLK)时钟=SYSCLK
    	RCC_PCLK2Config(RCC_HCLK_Div1); 	    //配置APB2(PCLK2)钟=AHB时钟
    	RCC_PCLK1Config(RCC_HCLK_Div2); 	    //配置APB1(PCLK1)钟=AHB 1/2时钟 	
        /* PLLCLK = 8MHz * 9 = 72MHz */
    	RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);//配置PLL时钟 == 外部高速晶体时钟*9
    	RCC_PLLCmd(ENABLE); 				                //使能PLL时钟 
    	while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET); //等待PLL时钟就绪
    	RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);          //配置系统时钟 = PLL时钟 
    	while(RCC_GetSYSCLKSource() != 0x08);               //检查PLL时钟是否作为系统时钟
  	}    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC 
                          |RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE | RCC_APB2Periph_GPIOF, ENABLE);
}

static __inline void NVIC_Configuration(void)
{   
	#ifdef  VECT_TAB_RAM  
  		/* Set the Vector Table base location at 0x20000000 */ 
  		NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0); 
    #elif defined(VECT_TAB_FLASH_IAP)
        NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x2000); 
	#else  /* VECT_TAB_FLASH  */
  		/* Set the Vector Table base location at 0x08000000 */ 
  		NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);   
	#endif
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);   
    
    Pub_Nvic_Config(UARTX_IRQ, 0, 0);   //串口1中断配置
    Pub_Nvic_Config(UARTX_DMA_IRQ,1,0); //串口1发送DMA中断配置
}

void BSP_Init(void)
{
    RCC_Configuration();
    NVIC_Configuration();
}


