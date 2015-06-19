#ifndef _KEY_H
#define _KEY_H

#include <stm32f10x.h>
#include <raw_api.h>

#include <pub.h>

//默认支持8个独立按键
//按键默认端口模式为上拉/下拉输入
#define KEY0_GPIOx      GPIOA  
#define KEY0_GPIO_Pin   GPIO_Pin_0

#define KEY1_GPIOx      GPIOA  
#define KEY1_GPIO_Pin   GPIO_Pin_1

#define KEY2_GPIOx      GPIOA  
#define KEY2_GPIO_Pin   GPIO_Pin_2

#define KEY3_GPIOx      GPIOA  
#define KEY3_GPIO_Pin   GPIO_Pin_3

#define KEY4_GPIOx      GPIOA  
#define KEY4_GPIO_Pin   GPIO_Pin_4

#define KEY5_GPIOx      GPIOA  
#define KEY5_GPIO_Pin   GPIO_Pin_5

#define KEY6_GPIOx      GPIOA  
#define KEY6_GPIO_Pin   GPIO_Pin_6

#define KEY7_GPIOx      GPIOA  
#define KEY7_GPIO_Pin   GPIO_Pin_7

#define KEY0_OUT        PAout(0)
#define KEY1_OUT        PAout(1)
#define KEY2_OUT        PAout(2)
#define KEY3_OUT        PAout(3)
#define KEY4_IN         PAin(4)
#define KEY5_IN         PAin(5)
#define KEY6_IN         PAin(6)
#define KEY7_IN         PAin(7)

#define DOUBLE_CLICK    1   //连按标志位

#endif
