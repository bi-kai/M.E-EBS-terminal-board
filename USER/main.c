#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "usart.h"
#include "timer.h"
#include "gray.h"
#include "wkup.h"
extern u16  TIM5CH1_CAPTURE_STA;		//输入捕获状态		    				
extern u16	TIM5CH1_CAPTURE_VAL;	//输入捕获值
extern u16	TIM5CH1_DOWN_CAPTURE_VAL;	//输入上升沿时间捕获值

extern u16 bit_SYCN_UP[BIT_SYNC_GROUPS];
extern u16 bit_SYCN_DOWN[BIT_SYNC_GROUPS];

extern u8 receive_frame[ORIGEN_FRAMESIZE];//接收帧的缓冲区
extern u16 frame_index;//数据帧中，比特位采样结果存储的索引值



extern signed char barker_sum;
extern u8 buf_barker[12];	
extern u16 frame_lengths;//当前帧总长度

u8 decoded_frame[DECODE_FRAMESIZE]={0};//格雷译码后数据的缓冲区
u16 decoded_frame_index=0;//格雷码译码后缓冲区数组的索引值
u8 frame_window_counter=0;	//表示处理完的滑窗数，统计完整窗口数，24位位一个窗口
extern u16 frame_lengths;//当前帧总长度
int main(void)
{	
 	u16 index=0;
	u8 i=0;//for循环
	float sample_rate=0;
	u8 gray_decode_buf1[24]={0};//格雷译码，24个码元为一组
	u8 gray_decoded_buf1[24]={0};//格雷译码纠错后的24位码元，其前12位为原始信息的倒叙排列
		
	delay_init();	    	 //延时函数初始化	  
	NVIC_Configuration(); 	 //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	uart_init(256000);	 //串口初始化为9600
 	LED_Init();			     //LED端口初始化	
	WKUP_Init(); //待机唤醒初始化
	LED1=0;
 	TIM5_Cap_Init(0XFFFF,72-1);	//以1Mhz的频率计数,0xFFFF为65.535ms
	TIM3_Int_Init(1999,7199);//1Khz的计数频率 
   	while(1)
	{	  		 		 
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


		if((TIM5CH1_CAPTURE_STA&0X80)&&(TIM5CH1_CAPTURE_STA&0X0200)&&(TIM5CH1_CAPTURE_STA&0X08))
		{
		   for(index=0;index<BIT_SYNC_GROUPS;index++)
		   {
		   		printf("%d UP %d\r\n",index,bit_SYCN_UP[index]);
		   		printf("%d down %d\r\n",index,bit_SYCN_DOWN[index]);
		   }
	
		   sample_rate=(float)(bit_SYCN_UP[0]+bit_SYCN_UP[1]+bit_SYCN_UP[2]+bit_SYCN_UP[3]+bit_SYCN_DOWN[0]+bit_SYCN_DOWN[1]+bit_SYCN_DOWN[2]+bit_SYCN_DOWN[3])/8;
		   printf("sample rate:%f,%d bit SYNC!\r\n",sample_rate,TIM5CH1_CAPTURE_STA);
		   printf("frame_index:%d \r\n",frame_index);
	
		   for(index=0;index<11;index++)
		   {
		 	  printf("%d",buf_barker[index]);
		   }
		   printf("\r\n原始数据：");
		   for(index=0;index<frame_index;index++)//接收的原始帧数据
		   {
		 	  printf("%d",receive_frame[index]);
		   }

		   printf("\r\n解码帧：");

			   for(index=0;index<decoded_frame_index;index++)//格雷解码后的数据帧
			   {
			 	  printf("%d",decoded_frame[index]);
			   }
			   printf("\r\n");
			   TIM5CH1_CAPTURE_STA=0;//帧处理完毕的清零
			   decoded_frame_index=0; 
			   frame_window_counter=0;
			   TIM_Cmd(TIM5,ENABLE);
		   
		   
		}



	   	//接收完信道中的完整一帧后，对其格雷译码，还原出原始数据，存入decoded_frame[]中
		if((TIM5CH1_CAPTURE_STA&0X10)&&(((frame_index+1)/24)>frame_window_counter)){ //未处理数据大于一个窗
		for(i=0;i<24;i++){
			gray_decode_buf1[i]=receive_frame[i+frame_window_counter*24];//将接收缓冲区中未处理的数据放到缓冲窗中，准备格雷译码
		}
		decode_error_catch(gray_decode_buf1,gray_decoded_buf1);//格雷译码,纠错结果为gray_decoded_buf[]，前12位码元为原始数据的倒序排列
		for(i=0;i<12;i++){
			decoded_frame[decoded_frame_index]=gray_decoded_buf1[11-i];//格雷译码后数据的缓冲区
			decoded_frame_index++;//格雷码译码后缓冲区数组的索引值
		}
		if(decoded_frame_index*2==frame_lengths){
			TIM5CH1_CAPTURE_STA|=0X0200;
		}
		frame_window_counter++;//解码窗滑动一次
		}



	}//end of while
}

