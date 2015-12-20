#ifndef _PUB_H
#define _PUB_H

#include "stm32f10x.h"

#ifndef NULL
	#define NULL (void *)0	//��ָ��
#endif

#define BYTE0(dwTemp)       (*(char *)(&dwTemp))
#define BYTE1(dwTemp)       (*((char *)(&dwTemp) + 1))
#define BYTE2(dwTemp)       (*((char *)(&dwTemp) + 2))
#define BYTE3(dwTemp)       (*((char *)(&dwTemp) + 3))

//IO�ڲ����궨��
#define BITBAND(addr, bitnum) ((addr & 0xF0000000)+0x2000000+((addr &0xFFFFF)<<5)+(bitnum<<2)) 
#define MEM_ADDR(addr)  *((volatile unsigned long  *)(addr)) 
#define BIT_ADDR(addr, bitnum)   MEM_ADDR(BITBAND(addr, bitnum)) 
//IO�ڵ�ַӳ��
#define GPIOA_ODR_Addr    (GPIOA_BASE+12) //0x4001080C 
#define GPIOB_ODR_Addr    (GPIOB_BASE+12) //0x40010C0C 
#define GPIOC_ODR_Addr    (GPIOC_BASE+12) //0x4001100C 
#define GPIOD_ODR_Addr    (GPIOD_BASE+12) //0x4001140C 
#define GPIOE_ODR_Addr    (GPIOE_BASE+12) //0x4001180C 
#define GPIOF_ODR_Addr    (GPIOF_BASE+12) //0x40011A0C    
#define GPIOG_ODR_Addr    (GPIOG_BASE+12) //0x40011E0C    

#define GPIOA_IDR_Addr    (GPIOA_BASE+8) //0x40010808 
#define GPIOB_IDR_Addr    (GPIOB_BASE+8) //0x40010C08 
#define GPIOC_IDR_Addr    (GPIOC_BASE+8) //0x40011008 
#define GPIOD_IDR_Addr    (GPIOD_BASE+8) //0x40011408 
#define GPIOE_IDR_Addr    (GPIOE_BASE+8) //0x40011808 
#define GPIOF_IDR_Addr    (GPIOF_BASE+8) //0x40011A08 
#define GPIOG_IDR_Addr    (GPIOG_BASE+8) //0x40011E08 
 
//IO�ڲ���,ֻ�Ե�һ��IO��!
//ȷ��n��ֵС��16!
#define PAout(n)   BIT_ADDR(GPIOA_ODR_Addr,n)  //��� 
#define PAin(n)    BIT_ADDR(GPIOA_IDR_Addr,n)  //���� 

#define PBout(n)   BIT_ADDR(GPIOB_ODR_Addr,n)  //��� 
#define PBin(n)    BIT_ADDR(GPIOB_IDR_Addr,n)  //���� 

#define PCout(n)   BIT_ADDR(GPIOC_ODR_Addr,n)  //��� 
#define PCin(n)    BIT_ADDR(GPIOC_IDR_Addr,n)  //���� 

#define PDout(n)   BIT_ADDR(GPIOD_ODR_Addr,n)  //��� 
#define PDin(n)    BIT_ADDR(GPIOD_IDR_Addr,n)  //���� 

#define PEout(n)   BIT_ADDR(GPIOE_ODR_Addr,n)  //��� 
#define PEin(n)    BIT_ADDR(GPIOE_IDR_Addr,n)  //����

#define PFout(n)   BIT_ADDR(GPIOF_ODR_Addr,n)  //��� 
#define PFin(n)    BIT_ADDR(GPIOF_IDR_Addr,n)  //����

#define PGout(n)   BIT_ADDR(GPIOG_ODR_Addr,n)  //��� 
#define PGin(n)    BIT_ADDR(GPIOG_IDR_Addr,n)  //����

typedef union
{
    struct
    {
        u8 b0:1;
        u8 b1:1;
        u8 b2:1;
        u8 b3:1;   
    }Bit;
    u8 Val;
}_DEF_UINT4_BIT;


typedef union
{
    struct
    {
        u8 b0:1;
        u8 b1:1;
        u8 b2:1;
        u8 b3:1;
        u8 b4:1;
        u8 b5:1;
        u8 b6:1;
        u8 b7:1;    
    }Bit;
    u8 Val;
}_DEF_UINT8_BIT;


//16λ������+�ṹ������
typedef union
{
    struct
    {
        u8 b0:1;
        u8 b1:1;
        u8 b2:1;
        u8 b3:1;
        u8 b4:1;
        u8 b5:1;
        u8 b6:1;
        u8 b7:1;
        u8 b8:1;
        u8 b9:1;
        u8 b10:1;
        u8 b11:1;
        u8 b12:1;
        u8 b13:1;
        u8 b14:1;
        u8 b15:1;        
    }Bit;
    u16 Val;
}_DEF_UINT16_BIT;

void Pub_Nvic_Config(uint8_t Irq_Chanl,uint8_t Preemp,uint8_t Sub);         //�����жϵ����ȼ�
void Pub_Gpio_Output_OD(GPIO_TypeDef* GPIOx,uint16_t Pin);                  //���ö˿�Ϊ��©�����
void Pub_Gpio_Output_OUTPP(GPIO_TypeDef* GPIOx,uint16_t Pin);               //���ö˿�Ϊ���������
void Pub_Gpio_Output_AFPP(GPIO_TypeDef* GPIOx,uint16_t Pin);                //���ö˿�Ϊ�������������
void Pub_Gpio_Input_AIN(GPIO_TypeDef* GPIOx,uint16_t Pin);                  //���ö˿�Ϊģ�������
void Pub_Gpio_Input_IPD(GPIO_TypeDef* GPIOx,uint16_t Pin);                  //���ö˿�Ϊ���������
void Pub_Gpio_Input_INFLOATING(GPIO_TypeDef* GPIOx,uint16_t Pin);           //���ö˿�Ϊ���������
void Pub_Gpio_Input_IPU(GPIO_TypeDef* GPIOx,uint16_t Pin);                  //���ö˿�Ϊ���������
void Pub_Uart_Hardware_Cfg(USART_TypeDef* USARTx,uint32_t Bps);             //���ô��ڵ�Ӳ�����ý�
void Pub_Uart_Int_Cfg(USART_TypeDef* USARTx,FunctionalState NewState);      //���ô��ڵ��ж�����

#endif
