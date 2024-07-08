#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "pwm.h"
#include "key.h"
#include "adc.h"
int main(void)
{
	  u8 t;
	  u8 len;
	  u16 x = 0;
	  u8 k = 0;
    u8 key;           //保存键值 
    u8 SD_Mode = 0;  //手动模式
	  u16 adcx0;
	  u16 adcx1;
    KEY_Init();       //初始化与按键连接的硬件接口
	  Adc_Init(); 				//adc初始化
	  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
	  delay_init(168);  //初始化延时函数
	  uart_init(9600);//初始化串口波特率为9600
 	  TIM14_PWM_Init(10000-1,84-1);	//84M/84=1Mhz的计数频率,重装载值100，所以PWM频率为 1M/100=10Khz.  
    TIM_SetCompare1(TIM14,x);	//修改比较值，修改占空比
	  //delay_ms(5000); //5秒
	
while(1)
 {	  
	  while (SD_Mode == 0)//自动模式
	  {
			key=KEY_Scan(0);		//得到键值
			if(key||USART_RX_STA&0x8000)  //自动模式下，按键进入手动模式
					{
					 SD_Mode = 1;
					}
				TIM_SetCompare1(TIM14,x);	//修改比较值，修改占空比
				adcx0=Get_Adc_Average(ADC_Channel_1,10);		//通过ADC1探测得到红外探测模块的输出值，当有人活动时会监测得到高电平
				adcx1=Get_Adc_Average(ADC_Channel_2,10);//采集得到光敏电阻的值，环境亮度越暗得到的值越小，亮度越高得到的值越大
					delay_ms(2000); //2秒
					  //当探测得到高电平时，根据 ADC通道2采集得到光敏电阻的值来调节亮度值
							if(adcx0>1800 && adcx1>4000) k=5;
							if(adcx0>1800 && adcx1<2000) k=1;
					    if(adcx0<1200) k=0;
			    x=k*2000;
					TIM_SetCompare1(TIM14,x);	//修改比较值，修改占空比
		}
		
	  while (SD_Mode == 1)//手动模式
    {
        key=KEY_Scan(0);		//得到键值
        if(key)++k;
        if(k == 6)k = 0;

	  	if(USART_RX_STA&0x8000)
	  	{					   
		  	len=USART_RX_STA&0x3fff;//得到此次接收到的数据长度
		  	for(t=0;t<len;t++)
			   {
//				  USART_SendData(USART1, 0x01);         //向串口1发送数据
//				  while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);//等待发送结束
				   if(USART_RX_BUF[t] == '1') k=5;
				   if(USART_RX_BUF[t] == '2') k=0;
					 if(USART_RX_BUF[t] == '3')
              {++k;
							if(k == 6) k = 5;
							}
					 if(USART_RX_BUF[t] == '4')
					    {--k;
							if(k == 255) k = 0;
							}
			   }
		  	USART_RX_STA=0;
      }
			  x=k*2000;
        TIM_SetCompare1(TIM14,x); 
			
	  }
  }
}

