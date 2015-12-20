#include "raw_api.h"
#include "pub.h"
#include "lcd.h"
#include "adc.h"
#include "string.h"

#define KERNEL_TEST_STK_SIZE    256
static PORT_STACK               kernel_test_task_stk[KERNEL_TEST_STK_SIZE];
static RAW_TASK_OBJ             kernel_test_task_obj;

#define TEST_TASK1          64
static PORT_STACK               test_task1_stk[TEST_TASK1];
static RAW_TASK_OBJ             test_task1_obj;

#define TEST_TASK2          512
static PORT_STACK               test_task2_stk[TEST_TASK2];
static RAW_TASK_OBJ             test_task2_obj;

#define Task1_Msg_Size  64
//RAW_SEMAPHORE Semaphore1;

extern RAW_U16 AdcVaule[3];

RAW_QUEUE_SIZE  Task1_Q;
RAW_MSG_SIZE    Task1_Msg[Task1_Msg_Size];

void sys_kerneltest_init(RAW_U8 prio);

struct
{
    GPIO_TypeDef* GPIOx;
    uint16_t GPIO_Pin;
}KEY_TEST[] = 
{
    GPIOE,GPIO_Pin_4,
    GPIOE,GPIO_Pin_3,
    GPIOE,GPIO_Pin_2,
};

void key_test_init(void)
{
    RAW_U8 i = 0;
    
    
    for(i=0;i<ARRAY_SIZE(KEY_TEST);i++)
    {
        Pub_Gpio_Input_IPU(KEY_TEST[i].GPIOx,KEY_TEST[i].GPIO_Pin);
    }
}

void kerneltest_task(void *pdat)
{
    (void)pdat;
    
    RAW_U32 freestack   = 0;
    RAW_U8  key_val     = 0;
    FlagStatus keyflag  = RESET;
    
    raw_printf("sys_kerneltest init...\r\t\t\t\t");
    
    key_test_init();
    
    raw_printf("[OK]\n");
    
    
    for(;;)
    {
       if(!GPIO_ReadInputDataBit(KEY_TEST[0].GPIOx,KEY_TEST[0].GPIO_Pin))
       {
           raw_sleep(10);
           {
               if(keyflag == RESET)
               {
                   if(!GPIO_ReadInputDataBit(KEY_TEST[0].GPIOx,KEY_TEST[0].GPIO_Pin))
                   {
                       keyflag = SET;
                        raw_task_stack_check(&kernel_test_task_obj,&freestack);
                        raw_printf("current task freestack : %d",freestack);
                   }
               }
           }
       }
       else
       {
           keyflag = RESET;
       }

        raw_sleep(50);
    }
    
}



void task_1(void *pdat)
{
    (void)pdat;   
    
    RAW_U32 *msg;
    
    RAW_U32 task1_msg_size;
    
    raw_task_qsize_create(&test_task1_obj, &Task1_Q, (RAW_U8 *)"Task1_Q", Task1_Msg, Task1_Msg_Size);
    
    for(;;)
    {
        raw_task_qsize_receive(RAW_WAIT_FOREVER, (void *)&msg, &task1_msg_size);
        raw_printf("raw_get_msg success ,msg : %d", *msg);
    }
}

void task_2(void *pdat)
{
    (void)pdat;
    
    RAW_U8 str[5] = "12345";
    RAW_U32 tmp_ad = 0;
    RAW_U8 i = 0;
    RAW_U8 tmp = 0;
    
    LCD_Init();
    Init_Adc();
//    RAW_U32 msg_2 = 0x1;
    POINT_COLOR=RED;
    LCD_Clear(BLUE);
//    LCD_ShowString(32,32,16,16,5,str);
    LCD_ShowString(64,64,240,32,16,str);
    tmp = sizeof(str);
    LCD_ShowxNum(32,360,tmp,4,24,1);
    for(;;)
    {
        for(i=0;i<=2;i++)
        {
            tmp_ad = Adc_Filter(i);
            LCD_ShowxNum(32,128+i*64,tmp_ad,4,24,0);
            raw_sleep(5);
        }
        raw_sleep(500);
    }    
}

void sys_kerneltest_init(RAW_U8 prio)
{
    /*
    raw_task_create(&kernel_test_task_obj,
                    (RAW_U8  *)"sys_kerneltest_init",
                    0, 
                    prio,
                    0,
                    kernel_test_task_stk, 
                    KERNEL_TEST_STK_SIZE,
                    kerneltest_task, 
                    1);
    
    raw_task_create(&test_task1_obj,
                    (RAW_U8  *)"test_task1",
                    0, 
                    prio-2,
                    0,
                    test_task1_stk, 
                    TEST_TASK1,
                    task_1, 
                    1);    */
    raw_task_create(&test_task2_obj,
                    (RAW_U8  *)"test_task2",
                    0, 
                    prio,
                    0,
                    test_task2_stk, 
                    TEST_TASK2,
                    task_2, 
                    1);       
}
















