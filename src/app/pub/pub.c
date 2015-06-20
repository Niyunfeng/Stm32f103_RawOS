#include "pub.h"

/*-----------------------------------------------------------
** ��������: ����Ƕ��ʽ�жϵ����ȼ�
** ��ڲ���: Irq_Chanl  :�жϺ�
             Preemp     :��ռ���ȼ�
             Sub        :��Ӧ���ȼ�
** ���ڲ���: ��         
-------------------------------------------------------------*/
void Pub_Nvic_Config(uint8_t Irq_Chanl,uint8_t Preemp,uint8_t Sub)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	
	NVIC_InitStructure.NVIC_IRQChannel 					= Irq_Chanl;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority= Preemp;	//�����ȼ�
	NVIC_InitStructure.NVIC_IRQChannelSubPriority 		= Sub;		//�����ȼ�
	NVIC_InitStructure.NVIC_IRQChannelCmd 				= ENABLE;

	NVIC_Init(&NVIC_InitStructure);
}

/*-----------------------------------------------------------
** ��������: ���ö˿�Ϊ���������
** ��ڲ���: GPIOx  :�˿ں�
             Pin    :���ź�
** ���ڲ���: ��
-------------------------------------------------------------*/
void Pub_Gpio_Output_OUTPP(GPIO_TypeDef* GPIOx,uint16_t Pin)
{
	GPIO_InitTypeDef tmp;
	tmp.GPIO_Pin	= Pin;
	tmp.GPIO_Mode	= GPIO_Mode_Out_PP; //�������
	tmp.GPIO_Speed	= GPIO_Speed_50MHz;
	GPIO_Init(GPIOx,&tmp);
}

/*-----------------------------------------------------------
** ��������: ���ö˿�Ϊ�������������
** ��ڲ���: GPIOx  :�˿ں�
             Pin    :���ź�
** ���ڲ���: ��
-------------------------------------------------------------*/
void Pub_Gpio_Output_AFPP(GPIO_TypeDef* GPIOx,uint16_t Pin)
{
	GPIO_InitTypeDef tmp;
	tmp.GPIO_Pin	= Pin;
	tmp.GPIO_Mode	= GPIO_Mode_AF_PP;   //�����������
	tmp.GPIO_Speed	= GPIO_Speed_50MHz;
	GPIO_Init(GPIOx,&tmp);
}

/*-----------------------------------------------------------
** ��������: ���ö˿�Ϊ��©�����
** ��ڲ���: GPIOx  :�˿ں�
             Pin    :���ź�
** ���ڲ���: ��
-------------------------------------------------------------*/
void Pub_Gpio_Output_OD(GPIO_TypeDef* GPIOx,uint16_t Pin)
{
	GPIO_InitTypeDef tmp;
	tmp.GPIO_Pin	= Pin;
	tmp.GPIO_Mode	= GPIO_Mode_Out_OD;   //�����������
	tmp.GPIO_Speed	= GPIO_Speed_50MHz;
	GPIO_Init(GPIOx,&tmp);
}

/*-----------------------------------------------------------
** ��������: ���ö˿�Ϊģ�������
** ��ڲ���: GPIOx  :�˿ں�
             Pin    :���ź�
** ���ڲ���: ��
-------------------------------------------------------------*/
void Pub_Gpio_Input_AIN(GPIO_TypeDef* GPIOx,uint16_t Pin)
{
	GPIO_InitTypeDef tmp;
	tmp.GPIO_Pin	=Pin;
	tmp.GPIO_Mode	=GPIO_Mode_AIN; //ģ������
	GPIO_Init(GPIOx,&tmp);
}

/*-----------------------------------------------------------
** ��������: ���ö˿�Ϊ���������
** ��ڲ���: GPIOx  :�˿ں�
             Pin    :���ź�
** ���ڲ���: ��
-------------------------------------------------------------*/
void Pub_Gpio_Input_IPD(GPIO_TypeDef* GPIOx,uint16_t Pin)
{
	GPIO_InitTypeDef tmp;
	tmp.GPIO_Pin	=Pin;
	tmp.GPIO_Mode	=GPIO_Mode_IPD; //��������
	GPIO_Init(GPIOx,&tmp);
}

/*-----------------------------------------------------------
** ��������: ���ö˿�Ϊ���������
** ��ڲ���: GPIOx  :�˿ں�
             Pin    :���ź�
** ���ڲ���: ��
-------------------------------------------------------------*/
void Pub_Gpio_Input_INFLOATING(GPIO_TypeDef* GPIOx,uint16_t Pin)
{
	GPIO_InitTypeDef tmp;
	tmp.GPIO_Pin	=Pin;
	tmp.GPIO_Mode	=GPIO_Mode_IN_FLOATING; //��������
	GPIO_Init(GPIOx,&tmp);
}

/*-----------------------------------------------------------
** ��������: ���ö˿�Ϊ���������
** ��ڲ���: GPIOx  :�˿ں�
             Pin    :���ź�
** ���ڲ���: ��
-------------------------------------------------------------*/
void Pub_Gpio_Input_IPU(GPIO_TypeDef* GPIOx,uint16_t Pin)
{
	GPIO_InitTypeDef tmp;
	tmp.GPIO_Pin	=Pin;
	tmp.GPIO_Mode	=GPIO_Mode_IPU; //��������
	GPIO_Init(GPIOx,&tmp);
}

/*-----------------------------------------------------------
** ��������: ���ô��ڵ�Ӳ�����ý�
** ��ڲ���: USARTx :���ں�
             Bsp    :������
** ���ڲ���: ��
-------------------------------------------------------------*/
void Pub_Uart_Hardware_Cfg(USART_TypeDef* USARTx,uint32_t Bps)
{
	USART_InitTypeDef 	USART_InitStructure;  		//���ڳ�ʼ���ṹ������
    USART_ClockInitTypeDef USART_ClockInitStructure;
	
	USART_InitStructure.USART_BaudRate 				= Bps;
	USART_InitStructure.USART_WordLength 			= USART_WordLength_8b;
	USART_InitStructure.USART_StopBits 				= USART_StopBits_1;
	USART_InitStructure.USART_Parity 				= USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl 	= USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode 					= USART_Mode_Rx | USART_Mode_Tx;
	
    USART_ClockInitStructure.USART_Clock            = USART_Clock_Disable;
    USART_ClockInitStructure.USART_CPOL             = USART_CPOL_Low;
    USART_ClockInitStructure.USART_CPHA             = USART_CPHA_2Edge;
    USART_ClockInitStructure.USART_LastBit          = USART_LastBit_Disable;
    
    USART_ClockInit(USARTx, &USART_ClockInitStructure);
	USART_Init(USARTx, &USART_InitStructure);
}

/*-----------------------------------------------------------
** ��������: ���ô��ڵ��ж�����
** ��ڲ���: USARTx :���ں�
** ���ڲ���: ��
-------------------------------------------------------------*/
void Pub_Uart_Int_Cfg(USART_TypeDef* USARTx)
{
    #if EN_UART_IT_RX == 1
        USART_ITConfig(USARTx, USART_IT_RXNE , ENABLE);	    //�������ڽ����ж�
    #endif
    
    //USART_ITConfig(USARTx, USART_IT_IDLE , ENABLE);	//��������,֡��,����,У����ж� 
	USART_Cmd(USARTx, ENABLE); 						    //�򿪴���
    //CPU��Сȱ�ݣ��������úã����ֱ��Send�����1���ֽڷ��Ͳ���ȥ
    //�����������1���ֽ��޷���ȷ���ͳ�ȥ������ 
  	USART_ClearFlag(USARTx, USART_FLAG_TC);     	    // �巢����ɱ�־��Transmission Complete flag 
}

