#include <raw_api.h>
#include <stm32f10x.h>

void create_init_task(void);

RAW_INLINE void BSP_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA 
                           |RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC
                           |RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE
                           |RCC_APB2Periph_ADC1  | RCC_APB2Periph_AFIO 
                           |RCC_APB2Periph_SPI1  | RCC_APB2Periph_TIM1 
                           |RCC_APB2Periph_TIM8,   ENABLE );

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4|RCC_APB1Periph_WWDG 
                        |RCC_APB1Periph_USART3|RCC_APB1Periph_TIM2, ENABLE );
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, ENABLE);	
	
}

int main(void)
{
    BSP_Init();
    
	raw_os_init();
	
	create_init_task();
	
	raw_os_start();
	
	return 0;
}
