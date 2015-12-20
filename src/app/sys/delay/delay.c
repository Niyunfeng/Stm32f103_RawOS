#include "delay.h"

static u8  fac_us=0;							//us延时倍乘数			   
static u16 fac_ms=0;							//ms延时倍乘数,在os下,代表每个节拍的ms数

//延迟初始化，注意：需要在第一个任务启动系统tick定时器后初始化
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

//us级延迟
void delay_us(u32 nus)
{		
	u32 ticks;
	u32 told,tnow,tcnt=0;
	u32 reload=SysTick->LOAD;				    //LOAD的值	    	 
	ticks=nus*fac_us; 						    //需要的节拍数 
	raw_disable_sche();					        //阻止OS调度，防止打断us延时
	told=SysTick->VAL;        				    //刚进入时的计数器值
	while(1)
	{
		tnow=SysTick->VAL;	
		if(tnow!=told)
		{	    
			if(tnow<told)tcnt+=told-tnow;	    //这里注意一下SYSTICK是一个递减的计数器就可以了.
			else tcnt+=reload-tnow+told;	    
			told=tnow;
			if(tcnt>=ticks)break;			    //时间超过/等于要延迟的时间,则退出.
		}  
	};
	raw_enable_sche();					        //恢复OS调度									    
}

//ms级延时
void delay_ms(u16 nms)
{	
	if(delay_osrunning&&delay_osintnesting==0)  //如果OS已经在跑了,并且不是在中断里面(中断里面不能任务调度)	    
	{		 
		if(nms>=fac_ms)						    //延时的时间大于OS的最少时间周期 
		{ 
   			raw_sleep(nms/fac_ms);	            //OS延时
		}
		nms%=fac_ms;						    //OS已经无法提供这么小的延时了,采用普通方式延时    
	}
	delay_us((u32)(nms*1000));				    //普通方式延时  
}



