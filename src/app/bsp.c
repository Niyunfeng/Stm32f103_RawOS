#include "pub.h"
#include "uart.h"

static __inline void RCC_Configuration(void)
{
	ErrorStatus HSEStartUpStatus;
    /*RCC system reset(for debug purpose) */
	RCC_DeInit(); 					    		//ʱ��ϵͳ��λ
    /*Enable HSE */
  	RCC_HSEConfig(RCC_HSE_ON);	    			//����HSEʱ��
    /*Wait till HSE is ready */
  	HSEStartUpStatus = RCC_WaitForHSEStartUp(); //�ȴ�HSEʱ��
  	if(HSEStartUpStatus == SUCCESS)             //���HSEʱ������
  	{
                                                //ʹ��Ԥ��ȡ������
    	FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
		FLASH_SetLatency(FLASH_Latency_2);      //Flash�ȴ�״̬
  
    	RCC_HCLKConfig(RCC_SYSCLK_Div1);	    //����AHB(HCLK)ʱ��=SYSCLK
    	RCC_PCLK2Config(RCC_HCLK_Div1); 	    //����APB2(PCLK2)��=AHBʱ��
    	RCC_PCLK1Config(RCC_HCLK_Div2); 	    //����APB1(PCLK1)��=AHB 1/2ʱ�� 	
        /* PLLCLK = 8MHz * 9 = 72MHz */
    	RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);//����PLLʱ�� == �ⲿ���پ���ʱ��*9
    	RCC_PLLCmd(ENABLE); 				                //ʹ��PLLʱ�� 
    	while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET); //�ȴ�PLLʱ�Ӿ���
    	RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);          //����ϵͳʱ�� = PLLʱ�� 
    	while(RCC_GetSYSCLKSource() != 0x08);               //���PLLʱ���Ƿ���Ϊϵͳʱ��
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
    
    Pub_Nvic_Config(UARTX_IRQ, 0, 0);   //����1�ж�����
    Pub_Nvic_Config(UARTX_DMA_IRQ,1,0); //����1����DMA�ж�����
}

void BSP_Init(void)
{
    RCC_Configuration();
    NVIC_Configuration();
}


