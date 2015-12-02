#include <raw_api.h>
#include <stm32f10x.h>

void BSP_Init(void);
void create_init_task(void);

int main(void)
{
    BSP_Init();
    
	raw_os_init();
	
	create_init_task();
	
	raw_os_start();
	
	return 0;
}
