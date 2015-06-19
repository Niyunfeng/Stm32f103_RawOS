#include "key.h"

/******************************************************************************/
#define SYS_KEY_TASK_STK_SIZE 	(128)
static PORT_STACK 				sys_key_task_stk[SYS_KEY_TASK_STK_SIZE];
static RAW_TASK_OBJ 			sys_key_task_obj;
/******************************************************************************/

//按键数据结构
_DEF_UINT4_BIT KeyIn;
_DEF_UINT4_BIT KeyOut;

/*--------------------------------矩阵键盘初始化----------------------------------------
* 功能： 初始化 stm32 单片机 GPIO 
* 参数传递：
* 输入： 无
* 返回值：无
--------------------------------------------------------------------------------------*/
void KeyBoard_Init(void)
{
    Pub_Gpio_Output_OUTPP(KEY0_GPIOx, KEY0_GPIO_Pin | KEY1_GPIO_Pin | KEY2_GPIO_Pin | KEY3_GPIO_Pin);
    Pub_Gpio_Input_IPU(KEY0_GPIOx, KEY4_GPIO_Pin | KEY5_GPIO_Pin | KEY6_GPIO_Pin | KEY7_GPIO_Pin);
}

void Load_KeyPort(u8 val)
{
    KeyOut.Val = val;
    KEY0_OUT = KeyOut.Bit.b0;
    KEY1_OUT = KeyOut.Bit.b1;
    KEY2_OUT = KeyOut.Bit.b2;
    KEY3_OUT = KeyOut.Bit.b3;
}

u8 Read_KeyPort(void)
{
    KeyIn.Bit.b0 = KEY4_IN;
    KeyIn.Bit.b1 = KEY5_IN;
    KeyIn.Bit.b2 = KEY6_IN;
    KeyIn.Bit.b3 = KEY7_IN;
    return KeyIn.Val;
}

u8 Read_KeyValue(void)
{
    u8 scan[4] = {0x0e,0x0d,0x0b,0x07};
    u8 KeyValue = 0,i,j;

#if DOUBLE_CLICK == 1

    static FlagStatus KeyFlag = RESET;

    Load_KeyPort(0);
    if(Read_KeyPort() != 0xf)               //判断是否有按键按下
    {
        
        if(KeyFlag == RESET)
        {
            for(i = 0;i < 4;i ++)
            {
                Load_KeyPort(scan[i]);      //进行逐行扫描
                for(j = 0;j < 4;j ++)
                {
                        if(Read_KeyPort() == scan[j])
                        {
                            if(Read_KeyPort() == scan[j])
                            {
                                KeyFlag = SET;//置位按键标识位，防止连续按下
                                KeyValue = j*4+i+1;
                                return KeyValue;
                            }
                        }
                }
            }
        }
    }
    else 
    {
        KeyFlag = RESET;
    }
    return 0;
#else
    Load_KeyPort(0);
    if(Read_KeyPort() != 0xf)               //判断是否有按键按下
    {
            for(i = 0;i < 4;i ++)
            {
                Load_KeyPort(scan[i]);      //进行逐行扫描
                for(j = 0;j < 4;j ++)
                {
                        if(Read_KeyPort() == scan[j])
                        {
                            if(Read_KeyPort() == scan[j])
                            {
                                KeyValue = j*4+i+1;
                                break;
                            }
                        }
                }
            }
    }
    return KeyValue;        
#endif
    
    
}

static void sys_key_task(void *pdat)
{
	(void)pdat;
	u8 KeyVal = 0;
	raw_printf("sys_key init...\r\t\t\t\t");
    KeyBoard_Init();
	raw_printf("[OK]\n");
	
	for(;;)
	{
        KeyVal = Read_KeyValue();
        if(KeyVal != 0)
        {
            raw_printf("raw-os# KeyVal: %d \n",KeyVal);
        }
		raw_sleep(50);
	}
}

void sys_key_init(RAW_U8 prio)
{
	raw_task_create(&sys_key_task_obj, 			/* 任务控制块地址 	*/
					(RAW_U8  *)"sys_key_task", 	/* 任务名 			*/
					(void *)0,					/* 任务参数 		*/
					prio, 						/* 优先级 			*/
					0,							/* 时间片 			*/
					sys_key_task_stk,			/* 任务栈首地址 	*/
					SYS_KEY_TASK_STK_SIZE ,		/* 任务栈大小 		*/
					sys_key_task,				/* 任务入口地址 	*/
					1);							/* 是否立即运行 	*/
}

