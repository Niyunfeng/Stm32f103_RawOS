#include <raw_api.h>
#include <stm32f10x.h>

void create_init_task(void);

void bsp_init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA| RCC_APB2Periph_GPIOB| RCC_APB2Periph_GPIOC
                         | RCC_APB2Periph_GPIOD| RCC_APB2Periph_GPIOE| RCC_APB2Periph_GPIOF
                         | RCC_APB2Periph_GPIOG, ENABLE);
}

int main(void)
{   
    bsp_init();
    
	raw_os_init();
	
	create_init_task();
	
	raw_os_start();
	
	return 0;
}
