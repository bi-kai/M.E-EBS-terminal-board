#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "usart.h"
#include "timer.h"
//ALIENTEK战舰STM32开发板实验10
//输入捕获实验  
//技术支持：www.openedv.com
//广州市星翼电子科技有限公司 
extern u16  TIM5CH1_CAPTURE_STA;		//输入捕获状态		    				
extern u16	TIM5CH1_CAPTURE_VAL;	//输入捕获值
extern u16	TIM5CH1_DOWN_CAPTURE_VAL;	//输入上升沿时间捕获值

extern u16 bit_SYCN_UP[4];
extern u16 bit_SYCN_DOWN[4];

extern u8 receive_frame[100];
	
 int main(void)
 {	
    u8 index=0;
	float sample_rate=0;	
	delay_init();	    	 //延时函数初始化	  
	NVIC_Configuration(); 	 //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	uart_init(256000);	 //串口初始化为9600
 	LED_Init();			     //LED端口初始化
 
// 	TIM3_PWM_Init(899,0); 		//不分频。PWM频率=72000/(899+1)=80Khz
 	TIM5_Cap_Init(0X00FF,72-1);	//以1Mhz的频率计数 
   	while(1)
	{
// 		delay_ms(10);
//		TIM_SetCompare2(TIM3,TIM_GetCapture2(TIM3)+1);	
//		if(TIM_GetCapture2(TIM3)==300)TIM_SetCompare2(TIM3,0);
			
		 		 
// 		if(TIM5CH1_CAPTURE_STA&0X80)//成功捕获到了一次上升沿
//		{
//		  if(j<8){	 
//			temp=TIM5CH1_CAPTURE_STA&0X3F;
//			temp*=65536;//溢出时间总和
//			temp+=TIM5CH1_CAPTURE_VAL;//得到总的高电平时间
//			i[j]=temp;
//			j++;
//			TIM5CH1_CAPTURE_STA=0;//开启下一次捕获
//			}
//		}else if(j>3&&j<8){
//		for(index=0;index<8;index++)
//		{
//			printf("%d\r\n",i[index]);//打印总的高点平时间
//			delay_ms(2);
//		}
//		j=8;
//	}

//	while(!(TIM5CH1_CAPTURE_STA&0X08));


	if(TIM5CH1_CAPTURE_STA&0X80)
	{
		

	   for(index=0;index<4;index++)
	   {
	   		printf("%d UP %d\r\n",index,bit_SYCN_UP[index]);
	   }
	   for(index=0;index<4;index++)
	   {
	   		printf("%d down %d\r\n",index,bit_SYCN_DOWN[index]);
	   }

	   sample_rate=(float)(bit_SYCN_UP[0]+bit_SYCN_UP[1]+bit_SYCN_UP[2]+bit_SYCN_UP[3]+bit_SYCN_DOWN[0]+bit_SYCN_DOWN[1]+bit_SYCN_DOWN[2])/7;
	   printf("sample rate:%f,%d bit SYNC!\r\n",sample_rate,TIM5CH1_CAPTURE_STA);
	   sample_rate=0;
		if(TIM5CH1_CAPTURE_STA&0X08)
		{
	   		TIM5CH1_CAPTURE_STA=0;
		}
	}
	
	
	}//end of while
 }

