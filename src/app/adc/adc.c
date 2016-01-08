#include "adc.h"

#define ADC1_DR_Address    ((uint32_t)0x4001244C)   //ADC数据寄存器的基地址
#define ADC_MAX_CHANL		3	                    //总共3路AD
#define ADC_CHK_CNT			50	                    //每路采集50次
uint16_t PubTmpAdc[ADC_CHK_CNT][ADC_MAX_CHANL];     //利用的临时变量进行存储

static void AdcChannelSelect(void);

//AD_GPIO初始化
static void GPIO_Configuration(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOA, ENABLE);
    Pub_Gpio_Input_AIN(GPIOA,GPIO_Pin_0 | GPIO_Pin_2 | GPIO_Pin_3);
}

//AD_DMA初始化
static void DMA_Configuration(void)
{
	DMA_InitTypeDef DMA_InitStructure;                                              //DMA初始化结构体声明
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);		                        //使能DMA时钟

	DMA_DeInit(DMA1_Channel1);		  												//开启DMA1的第一通道
	DMA_InitStructure.DMA_PeripheralBaseAddr 	= ADC1_DR_Address;		  			//DMA对应的外设基地址
	DMA_InitStructure.DMA_MemoryBaseAddr 		= (uint32_t)&PubTmpAdc;//Value;  	//内存存储基地址
	DMA_InitStructure.DMA_DIR 					= DMA_DIR_PeripheralSRC;			//DMA的转换模式为SRC模式，由外设搬移到内存
	DMA_InitStructure.DMA_BufferSize 			= ADC_MAX_CHANL*ADC_CHK_CNT; 		//DMA缓存大小，16个=通道号数*每个通道采集的次数
	DMA_InitStructure.DMA_PeripheralInc 		= DMA_PeripheralInc_Disable;		//接收一次数据后，设备地址禁止后移
	DMA_InitStructure.DMA_MemoryInc 			= DMA_MemoryInc_Enable;				//关闭接收一次数据后，目标内存地址后移
	DMA_InitStructure.DMA_PeripheralDataSize 	= DMA_PeripheralDataSize_HalfWord;  //定义外设数据宽度为16位
	DMA_InitStructure.DMA_MemoryDataSize 		= DMA_MemoryDataSize_HalfWord; 		//DMA搬移数据尺寸，HalfWord就是为16位
	DMA_InitStructure.DMA_Mode 					= DMA_Mode_Circular;   				//转换模式，循环缓存模式。
	DMA_InitStructure.DMA_Priority 				= DMA_Priority_High;				//DMA优先级高
	DMA_InitStructure.DMA_M2M 					= DMA_M2M_Disable;		  			//M2M模式禁用
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);          

	DMA_Cmd(DMA1_Channel1, ENABLE);                                                 //使能DMA通道1
}

static void ADC_Configuration(void)
{
	ADC_InitTypeDef TmpAdc;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);	  			//使能ADC时钟
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);
	ADC_DeInit(ADC1);//将外设 ADC1 的全部寄存器重设为缺省值 
	
	TmpAdc.ADC_Mode 				= ADC_Mode_Independent;				//独立的转换模式
	TmpAdc.ADC_ScanConvMode 		= ENABLE;		  					//开启扫描模式
	TmpAdc.ADC_ContinuousConvMode	= ENABLE;   						//开启连续转换模式
	TmpAdc.ADC_ExternalTrigConv 	= ADC_ExternalTrigConv_None;		//外部触发禁止
	TmpAdc.ADC_DataAlign 			= ADC_DataAlign_Right;   			//对齐方式,ADC为12位中，右对齐方式
	TmpAdc.ADC_NbrOfChannel 		= ADC_MAX_CHANL;	 				//开启通道数，3个
	ADC_Init(ADC1, &TmpAdc);
	
	AdcChannelSelect();			                //启动转换
}

static void AdcChannelSelect(void)
{
	//ADC通道组， 第x个通道 采样顺序1，转换时间 
	ADC_RegularChannelConfig(ADC1, ADC_Channel_0 , 1, ADC_SampleTime_239Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_2 , 2, ADC_SampleTime_239Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_3 , 3, ADC_SampleTime_239Cycles5);

	ADC_DMACmd(ADC1, ENABLE);	  				//ADC命令，使能
	ADC_Cmd(ADC1, ENABLE);  					//开启ADC1
	ADC_ResetCalibration(ADC1);	  				//重新校准
	while(ADC_GetResetCalibrationStatus(ADC1)); //等待重新校准完成
	ADC_StartCalibration(ADC1);					//开始校准
	while(ADC_GetCalibrationStatus(ADC1));		//等待校准完成
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);		//连续转换开始，ADC通过DMA方式不断的更新RAM区
	
	 /* Test on Channel 1 DMA1_FLAG_TC flag */
	while(!DMA_GetFlagStatus(DMA1_FLAG_TC1));
  
	/* Clear Channel 1 DMA1_FLAG_TC flag */
	DMA_ClearFlag(DMA1_FLAG_TC1);	
}

/*-----------------------------------------------------------
** 函数名称: Adc_Filter(void)
** 功能描述: ADC软件滤波
------------------------------------------------------------*/
uint16_t ADC_Filter(uint8_t ch)
{
	#define N 	ADC_CHK_CNT
		
	uint16_t ValueTemp=0;
	vu8 i=0,j=0;
//	//去除前后各(10*(总次数/50))，取中间值进行平均
//    Uint8 a = (ADC_CHK_CNT/50)*10;
//    Uint8 b = (ADC_CHK_CNT - a);
//    Uint8 k = (ADC_CHK_CNT - a*2);
//    
//    Uint32 sum[ADC_MAX_CHANL] = {0};
//    Uint16 avr[ADC_MAX_CHANL] = {0};
    
	for (j=0;j<N-1;j++)
	{
		for (i=0;i<N-j;i++)
		{
			if ( PubTmpAdc[i][ch] > PubTmpAdc[i+1][ch])
			{
				ValueTemp 			= 	PubTmpAdc[i][ch];
				PubTmpAdc[i][ch]	= 	PubTmpAdc[i+1][ch]; 
				PubTmpAdc[i+1][ch] 	= 	ValueTemp;
			}
		}
	}
//    for(i=a;i<b;i++)
//    {
//        sum[Ch] += PubTmpAdc[i][Ch];
//    }
//    avr[Ch] = sum[Ch]/k;
//    
//    return avr[Ch];
	return (PubTmpAdc[(N-1)>>1][ch]);
}

void ADC_InitConfiguration(void)
{        
   GPIO_Configuration();
   DMA_Configuration();
   ADC_Configuration();
}
