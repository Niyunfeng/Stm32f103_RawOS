#include "delay.h"

static u8  fac_us=0;							//us��ʱ������			   
static u16 fac_ms=0;							//ms��ʱ������,��os��,����ÿ�����ĵ�ms��

//�ӳٳ�ʼ����ע�⣺��Ҫ�ڵ�һ����������ϵͳtick��ʱ�����ʼ��
void delay_init(void)
{
    u32 reload;
    
    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);
    
    fac_us = SystemCoreClock/8000000;
    
    reload = SystemCoreClock/8000000;
    
    reload*=1000000/delay_ostickspersec;
    
    fac_ms=1000/delay_ostickspersec;
    
    SysTick->CTRL|=SysTick_CTRL_TICKINT_Msk;
    SysTick->LOAD=reload;
    SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk;
}

//us���ӳ�
void delay_us(u32 nus)
{		
	u32 ticks;
	u32 told,tnow,tcnt=0;
	u32 reload=SysTick->LOAD;				    //LOAD��ֵ	    	 
	ticks=nus*fac_us; 						    //��Ҫ�Ľ����� 
	raw_disable_sche();					        //��ֹOS���ȣ���ֹ���us��ʱ
	told=SysTick->VAL;        				    //�ս���ʱ�ļ�����ֵ
	while(1)
	{
		tnow=SysTick->VAL;	
		if(tnow!=told)
		{	    
			if(tnow<told)tcnt+=told-tnow;	    //����ע��һ��SYSTICK��һ���ݼ��ļ������Ϳ�����.
			else tcnt+=reload-tnow+told;	    
			told=tnow;
			if(tcnt>=ticks)break;			    //ʱ�䳬��/����Ҫ�ӳٵ�ʱ��,���˳�.
		}  
	};
	raw_enable_sche();					        //�ָ�OS����									    
}

//ms����ʱ
void delay_ms(u16 nms)
{	
	if(delay_osrunning&&delay_osintnesting==0)  //���OS�Ѿ�������,���Ҳ������ж�����(�ж����治���������)	    
	{		 
		if(nms>=fac_ms)						    //��ʱ��ʱ�����OS������ʱ������ 
		{ 
   			raw_sleep(nms/fac_ms);	            //OS��ʱ
		}
		nms%=fac_ms;						    //OS�Ѿ��޷��ṩ��ôС����ʱ��,������ͨ��ʽ��ʱ    
	}
	delay_us((u32)(nms*1000));				    //��ͨ��ʽ��ʱ  
}



