#include "timer.h"
#include "led.h"
#include "usart.h"
#include "delay.h"
#include "gray.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK Mini STM32开发板
//PWM  驱动代码			   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2010/12/03
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 正点原子 2009-2019
//All rights reserved
////////////////////////////////////////////////////////////////////////////////// 	  

//通用定时器中断初始化
//这里时钟选择为APB1的2倍，而APB1为36M
//arr：自动重装值。
//psc：时钟预分频数
//这里使用的是定时器3!

void TIM3_Int_Init(u16 arr,u16 psc)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); //时钟使能

	TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	 计数到5000为500ms
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值  10Khz的计数频率  
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位
 
	TIM_ITConfig(  //使能或者失能指定的TIM中断
		TIM3, //TIM2
		TIM_IT_Update  |  //TIM 中断源
		TIM_IT_Trigger,   //TIM 触发中断源 
		ENABLE  //使能
		);
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;  //TIM3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //先占优先级0级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //从优先级3级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);  //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器

	TIM_Cmd(TIM3, ENABLE);  //使能TIMx外设
							 
}

void TIM3_IRQHandler(void)   //TIM3中断
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) //检查指定的TIM中断发生与否:TIM 中断源 
		{
			TIM_ClearITPendingBit(TIM3, TIM_IT_Update);  //清除TIMx的中断待处理位:TIM 中断源 
			LED0=!LED0;
			
		}
}




//PWM输出初始化
//arr：自动重装值
//psc：时钟预分频数

void TIM3_PWM_Init(u16 arr,u16 psc)
{  
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB  | RCC_APB2Periph_AFIO, ENABLE);  //使能GPIO外设和AFIO复用功能模块时钟使能
	
	GPIO_PinRemapConfig(GPIO_PartialRemap_TIM3, ENABLE); //Timer3部分重映射  TIM3_CH2->PB5                                                                       	 //用于TIM3的CH2输出的PWM通过该LED显示
 
   //设置该引脚为复用输出功能,输出TIM3 CH2的PWM脉冲波形
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5; //TIM_CH2
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //复用推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	//GPIO_WriteBit(GPIOA, GPIO_Pin_7,Bit_SET); // PA7上拉	

	TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	 80K
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值  不分频
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位
	
	 
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2; //选择定时器模式:TIM脉冲宽度调制模式2
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //比较输出使能
	TIM_OCInitStructure.TIM_Pulse = 0; //设置待装入捕获比较寄存器的脉冲值
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //输出极性:TIM输出比较极性高
	TIM_OC2Init(TIM3, &TIM_OCInitStructure);  //根据TIM_OCInitStruct中指定的参数初始化外设TIMx
	TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable);  //使能TIMx在CCR2上的预装载寄存器
	
	TIM_ARRPreloadConfig(TIM3, ENABLE); //使能TIMx在ARR上的预装载寄存器
	
 
	TIM_Cmd(TIM3, ENABLE);  //使能TIMx外设
 

}

//定时器5通道1输入捕获配置

TIM_ICInitTypeDef  TIM5_ICInitStructure;

void TIM5_Cap_Init(u16 arr,u16 psc)
{	 
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
   	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);	//使能TIM5时钟
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);  //使能GPIOA时钟
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_1;  //PA1 清除之前设置  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //PA1 输入  
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOA,GPIO_Pin_1);						 //PA1 下拉


 	
	//初始化定时器5 TIM5	 
	TIM_TimeBaseStructure.TIM_Period = arr; //设定计数器自动重装值 
	TIM_TimeBaseStructure.TIM_Prescaler =psc; 	//预分频器   
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位
  
	//初始化TIM5输入捕获参数
	TIM5_ICInitStructure.TIM_Channel = TIM_Channel_2; //CC1S=01 	选择输入端 IC1映射到TI1上
  	TIM5_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;	//上升沿捕获
  	TIM5_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI; //映射到TI1上
  	TIM5_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;	 //配置输入分频,不分频 
  	TIM5_ICInitStructure.TIM_ICFilter = 0x11;//IC1F=0000 配置输入滤波器 不滤波
  	TIM_ICInit(TIM5, &TIM5_ICInitStructure);
	
	//中断分组初始化
	NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;  //TIM3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;  //先占优先级2级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;  //从优先级0级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);  //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器 
	
	TIM_ITConfig(TIM5,TIM_IT_Update|TIM_IT_CC2,ENABLE);//TIM_IT_CC1,允许更新中断 ,允许CC1IE捕获中断	
	
   	TIM_Cmd(TIM5,ENABLE ); 	//使能定时器5
   


}

u16 TIM5CH1_CAPTURE_STA=0;//输入捕获状态		    				
u16	TIM5CH1_CAPTURE_VAL=0;	//输入捕获值
u16	TIM5CH1_DOWN_CAPTURE_VAL=0;	//输入下降沿时间捕获值 
u16 bit_SYCN_UP[BIT_SYNC_GROUPS]={0};//位同步4个高电平持续时间，用于统计最佳抽判时刻
u16 bit_SYCN_DOWN[BIT_SYNC_GROUPS]={0};//位同步4个低电平持续时间，用于统计最佳抽判时刻

u8 receive_frame[ORIGEN_FRAMESIZE]={0};//接收帧的缓冲区
u16 frame_index=0;//数据帧中，比特位采样结果存储的索引值

u8 buf_barker[12]={0};//巴克码缓冲区，11位巴克码+1个新到bit，共12位空间
u8 barker[]={1,1,1,0,0,0,1,0,0,1,0};//准确的巴克码组
signed char barker_sum=0;//巴克码移位寄存器结果（可为负）
u16 frame_lengths=0;//当前帧总长度 
extern u8 frame_window_counter;//表示处理完的滑窗数，统计完整窗口数，24位位一个窗口
extern u16 decoded_frame_index;//格雷码译码后缓冲区数组的索引值


u8 tmp_buf=0;
//定时器5中断服务程序	 
void TIM5_IRQHandler(void)
{ 
   static u8 bit_counter_up=0;//位同步时，统计第n个高脉冲
   static u8 bit_counter_down=0;//位同步时，统计第n个低脉冲
   static u8 timeout_flag=0;//0：TIM5 update溢出时间内没有码元捕获；1：溢出时间内有码元被捕获到。功能：防止帧过长，产生update清零	
   static u8 barker_counter=0;//巴克码移位比较次数，超过20次还没有帧同步上，则清空寄存器
   u16 bit_SYCN_sum=0;//位同步高低电平码元持续时间和

   u8 gray_decode_buf[24]={0};//格雷译码，24个码元为一组
   u8 gray_decoded_buf[24]={0};//格雷译码纠错后的24位码元，其前12位为原始信息的倒叙排列
 
   u8 i=0;  //数组for循环的索引
// signed char temp=0;
   
/*********************************位同步捕获*************************************/   
 	if((TIM5CH1_CAPTURE_STA&0X80)==0)//位同步捕获，还未成功捕获(优先捕获1码)	
	{	  
		if (TIM_GetITStatus(TIM5, TIM_IT_Update) != RESET) //65.535定时器超时，则全部清空；此外，还要考虑主函数中对阵处理的时间		 
		{	 
			if(timeout_flag==0){   //update时间内值保持为零，说明这段时间内没有码元，故不是帧的中间部位，寄存器清零，等待帧接收
				TIM5CH1_CAPTURE_STA=0;
				TIM5CH1_CAPTURE_VAL=0; 
				TIM_OC2PolarityConfig(TIM5,TIM_ICPolarity_Rising); //CC1P=0 设置为上升沿捕获
				bit_counter_up=0;//标记高电平数组为空
				bit_counter_down=0;//标记低电平数组为空
				frame_index=0;//帧缓冲清零，等待下一帧		
			}else{
				timeout_flag=0;
			}
		}

		 if(TIM5CH1_CAPTURE_STA==0)//控制寄存器若被清空，标志上一帧接收完毕/高电平码元超时/第一帧，开始接收新的帧
		 {
		 	TIM_ITConfig(TIM5,TIM_IT_CC2,ENABLE);//TIM_IT_CC1,接收新的帧，打开输入捕获
			TIM_OC2PolarityConfig(TIM5,TIM_ICPolarity_Rising); //CC1P=0 设置为上升沿捕获 
			bit_counter_up=0;//标记高电平数组为空
			bit_counter_down=0;
			TIM5CH1_CAPTURE_VAL=0; 			
		 }
		   
		if (TIM_GetITStatus(TIM5, TIM_IT_CC2) != RESET)//TIM_IT_CC1,捕获1，发生捕获事件
		{	
			timeout_flag=1;	//关闭update清零
			if(TIM5CH1_CAPTURE_STA&0X40)		//捕获到一个下降沿 		
			{	 	  			
				TIM5CH1_CAPTURE_VAL=TIM_GetCapture2(TIM5);//TIM_GetCapture1
				
				if((TIM5CH1_CAPTURE_VAL>min_interval)&&(TIM5CH1_CAPTURE_VAL<max_interval))//捕获到一个波特周期的高脉冲
				{				 					
					if((TIM5CH1_CAPTURE_STA&0X07)<BIT_SYNC_GROUPS)//捕获到合适的高电平和低电平了
					{
						TIM5CH1_CAPTURE_STA++;
						bit_SYCN_UP[bit_counter_up]=TIM5CH1_CAPTURE_VAL+3;  //高定平捕获时间有少3us误差，低电平有少1us的误差	
						bit_counter_up++;								   //2 
					}
				}else if((TIM5CH1_CAPTURE_VAL>2*min_interval)&&(TIM5CH1_CAPTURE_VAL<2*max_interval))//出现相邻2次误码，2个波特周期的高电平，算一次正确的高脉冲
				{
					if((TIM5CH1_CAPTURE_STA&0X07)<BIT_SYNC_GROUPS)//捕获到合适的高电平和低电平了
					{
						TIM5CH1_CAPTURE_STA++;
						bit_SYCN_UP[bit_counter_up]=TIM5CH1_CAPTURE_VAL/2+3;   //1
						bit_counter_up++;
						bit_SYCN_DOWN[bit_counter_down]=TIM5CH1_DOWN_CAPTURE_VAL/2-1;   //1
						bit_counter_down++;								   //2 
					}
				}
//				else if((TIM5CH1_CAPTURE_VAL>3*min_interval)&&(TIM5CH1_CAPTURE_VAL<3*max_interval))//出现误码，0->1，3个波特周期的高电平
//				{
//					if((TIM5CH1_CAPTURE_STA&0X07)<BIT_SYNC_GROUPS)//捕获到合适的高电平和低电平了
//					{
//						TIM5CH1_CAPTURE_STA+=2;
//						bit_SYCN_UP[bit_counter_up]=TIM5CH1_CAPTURE_VAL/3+3;   //1
//						bit_counter_up++;
//						bit_SYCN_DOWN[bit_counter_down]=TIM5CH1_DOWN_CAPTURE_VAL/3-1;   //1
//						bit_counter_down++;								   //2
//						bit_SYCN_UP[bit_counter_up]=TIM5CH1_CAPTURE_VAL/3+3;   //1
//						bit_counter_up++;								   //2 
//					}
//				}else if((TIM5CH1_CAPTURE_VAL>4*min_interval)&&(TIM5CH1_CAPTURE_VAL<4*max_interval))//位同步第八位0->1，与巴克码连接为4个波特周期高电平，根据情况看需不需要对5个连1做判断
//				{
//					if(((TIM5CH1_CAPTURE_STA&0X07)<BIT_SYNC_GROUPS)&&((TIM5CH1_CAPTURE_STA&0X07)>=2))//保证前面已经至少捕获到了2个有效高电平
//					{
////						TIM5CH1_CAPTURE_STA+=1;
////						bit_SYCN_UP[bit_counter_up]=TIM5CH1_CAPTURE_VAL/4;   //1
////						bit_counter_up++;								   //2 
//						bit_SYCN_DOWN[bit_counter_down]=TIM5CH1_DOWN_CAPTURE_VAL/4-1;   //1
//						bit_counter_down++;
//						
//					}
//				}
				else //清零，等待重新接收
				{
					TIM5CH1_CAPTURE_STA=0;//非法0码脉冲，关闭脉冲计数
					TIM5CH1_CAPTURE_VAL=0;
					bit_counter_up=0;//标记高电平数组为空
					bit_counter_down=0;							
				}
//				if((bit_counter_down>=BIT_SYNC_GROUPS)||(bit_counter_up>=BIT_SYNC_GROUPS)){bit_counter_down=0;bit_counter_up=0;}		   //3
				TIM5CH1_CAPTURE_STA&=0XBF; //清除上升沿捕获标志位，准备接收下一个上升沿
				TIM5CH1_CAPTURE_STA|=0X20;//低电平使能计数标志位，第一个1码后使能0码计数器
		   		TIM_OC2PolarityConfig(TIM5,TIM_ICPolarity_Rising); //CC1P=0 设置为上升沿捕获
			}else//还未开始,第一次捕获上升沿
			{  
				TIM5CH1_DOWN_CAPTURE_VAL=TIM_GetCapture2(TIM5)-TIM5CH1_CAPTURE_VAL;//TIM_GetCapture1,得到低电平持续的时间
				if((TIM5CH1_DOWN_CAPTURE_VAL>min_interval)&&(TIM5CH1_DOWN_CAPTURE_VAL<max_interval)) //捕获到1个波特周期的低电平
				{
					if(TIM5CH1_CAPTURE_STA&0X20) //低电平计数已经使能
					{
						bit_SYCN_DOWN[bit_counter_down]=TIM5CH1_DOWN_CAPTURE_VAL-1;   //1
						bit_counter_down++;								   //2
					}
					
				}else if((TIM5CH1_DOWN_CAPTURE_VAL>2*min_interval)&&(TIM5CH1_DOWN_CAPTURE_VAL<2*max_interval))//出现误码，2个波特周期的低电平
				{
					if(TIM5CH1_CAPTURE_STA&0X20) //低电平计数已经使能
					{
						TIM5CH1_CAPTURE_STA++;//出现两个低电平，其中一个发送的必然是高电平
						bit_SYCN_UP[bit_counter_up]=TIM5CH1_CAPTURE_VAL/2+3;   //1
						bit_counter_up++;
						bit_SYCN_DOWN[bit_counter_down]=TIM5CH1_DOWN_CAPTURE_VAL/2-1;   //1
						bit_counter_down++;								   //2
					}
				}
//				else if((TIM5CH1_DOWN_CAPTURE_VAL>3*min_interval)&&(TIM5CH1_DOWN_CAPTURE_VAL<3*max_interval))//出现误码，3个波特周期的低电平
//				{
//					if(TIM5CH1_CAPTURE_STA&0X20) //低电平计数已经使能
//					{
//						TIM5CH1_CAPTURE_STA++;//出现三个低电平，其中一个发送的必然是高电平
//						bit_SYCN_UP[bit_counter_up]=TIM5CH1_CAPTURE_VAL/3+3;   //1
//						bit_counter_up++;
//						bit_SYCN_DOWN[bit_counter_down]=TIM5CH1_DOWN_CAPTURE_VAL/3-1;   //1
//						bit_counter_down++;								   //2
//						bit_SYCN_DOWN[bit_counter_down]=TIM5CH1_DOWN_CAPTURE_VAL/3-1;   //1
//						bit_counter_down++;								   //2
//					}
//				}
				else//清零，第一个上升沿到来或同步失败
				{
					TIM5CH1_CAPTURE_STA=0;	//清空
					bit_counter_down=0;//标记低电平数组为空
					bit_counter_up=0;
				}

			    if((TIM5CH1_CAPTURE_STA&0X07)>(BIT_SYNC_GROUPS-1))//捕获到连续的3对10码或以上，上升沿时计算平均抽判时刻，则开始巴克码验证
				{
					TIM5CH1_CAPTURE_STA|=0X80; //得到位同步头！！！
					TIM_ITConfig(TIM5,TIM_IT_CC2,DISABLE);//TIM_IT_CC1,关闭输入捕获，准备接收巴克码
					bit_SYCN_sum=0;
					for(i=0;i<bit_counter_up;i++)
					{
						bit_SYCN_sum+=bit_SYCN_UP[i];
					}
					for(i=0;i<bit_counter_down;i++)
					{
						bit_SYCN_sum+=bit_SYCN_DOWN[i];
					}
					bit_SYCN_sum+=50;//由于采到的高低脉冲都会有一定的误差，比如1000us的采成998us，对该变量加50us为对这种情况的修正，避免其取模时进入if条件
					bit_SYCN_sum%=FSK_SPEED;
					if(bit_SYCN_sum>((max_interval+min_interval)/4))//判决同步时刻超过1/2码元间隔
					{
						delay_us(3*(max_interval+min_interval)/4-bit_SYCN_sum-10);//码元间隔1ms，码元的中间位置为最佳抽判时刻,50为由示波器观察抽判时刻后的修正
					}else
					{
						delay_us((max_interval+min_interval)/4-bit_SYCN_sum-10);//码元间隔1ms，码元的中间位置为最佳抽判时刻,50为由示波器观察抽判时刻后的修正
					}
					buf_barker[10]=GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_1);//位同步后，新到达的第一位存储在巴克码buf的第10位
					TIM5->ARR=(max_interval+min_interval)/2-1;//0X0031;//重新状态定时器的值，1ms中断，对PAin(0)抽判一次(0x0030则会隔几个码元多采样一次；0X0031效果比较好；0X0032则会隔几个少采样一次)
					bit_counter_down=0;
					bit_counter_up=0;
					barker_counter=0;//巴克码比较次数的清零
				//	printf("\r\n位同步!"); 					
				}
				if((bit_counter_down>=BIT_SYNC_GROUPS)||(bit_counter_up>=BIT_SYNC_GROUPS)){bit_counter_down=0;bit_counter_up=0;}		   //3
				TIM5CH1_CAPTURE_VAL=0;	
	 			TIM_SetCounter(TIM5,0);//计数器值清零
				TIM5CH1_CAPTURE_STA|=0X40;		//标记捕获到了上升沿
	   			TIM_OC2PolarityConfig(TIM5,TIM_ICPolarity_Falling);		//CC1P=1 设置为下降沿捕获
			}		    
		}//end of 捕获事件			     	    					   
 	}
/****************************************巴克码校验*******************************************/	
	else if((TIM5CH1_CAPTURE_STA&0X80)&&((TIM5CH1_CAPTURE_STA&0X10)==0))//位同步之后，开始判断巴克码的帧同步
	{
	   if (TIM_GetITStatus(TIM5, TIM_IT_Update) != RESET)//位同步捕获完成后的超时采样
	   {
		   buf_barker[11]=GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_1);//位同步后，新到达的bit
		   PBout(6)=!PBout(6);//抽判时刻测试
		   for(i=0;i<11;i++)//收到最新数据后，将buf循环左移一位
		   {
		   		buf_barker[i]=buf_barker[i+1];
		   }
		   for(i=0;i<11;i++)
		   {
			   if(buf_barker[i]==barker[i]){barker_sum++;}
			   else barker_sum--;
		   }
		   barker_counter++;//移位比较一次
//		   printf("\r\n巴克码比较%d次!",barker_counter);
		   if(barker_sum>=9){
		   	   frame_index=0;
			   frame_lengths=ORIGEN_FRAMESIZE-20;//1100-20=24*45
			   TIM5CH1_CAPTURE_STA|=0X10;
//		   	   printf("\r\n帧同步!");
		   }//帧缓冲清零，准备接收数据
		   else if(barker_counter>30){
		   frame_window_counter=0;
		   decoded_frame_index=0;

		   TIM5->ARR=0XFFFF;//0X0031;//重新状态定时器的值
		   TIM_ITConfig(TIM5,TIM_IT_CC2,ENABLE);//TIM_IT_CC1,接收新的帧，打开输入捕获
		   TIM_OC2PolarityConfig(TIM5,TIM_ICPolarity_Rising); //CC1P=0 设置为上升沿捕获

		   TIM5CH1_CAPTURE_STA=0;
		   printf("\r\n这里有问题哦!");}//移位30次还没有同步，则清零寄存器（位同步所用到的变量已在位同步成功/失败退出时清零）
		   barker_sum=0;//不满足验证，则从新开始
	   }
	}
/****************************************帧内容接收*******************************************/		
	else if((TIM5CH1_CAPTURE_STA&0X80)&&(TIM5CH1_CAPTURE_STA&0X10)&&((TIM5CH1_CAPTURE_STA&0X08)==0))//巴克码校验完毕，准备接收帧中数据
	{
		if (TIM_GetITStatus(TIM5, TIM_IT_Update)!= RESET)//开始接收巴克码后面的数据
		{
			if(frame_index<frame_lengths)
			{					 
				receive_frame[frame_index]=GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_1); //当前时刻就是位同步后第一个码元的最佳抽判时刻
				PBout(6)=!PBout(6);//抽判时刻测试

			//提取帧类型设定帧长，开始
			if(((TIM5CH1_CAPTURE_STA&0X0100)==0)&&(frame_index>24)){//首次接收24个码元后，译码提出“帧类型”
				TIM5CH1_CAPTURE_STA|=0X0100;
				for(i=0;i<24;i++){
					gray_decode_buf[i]=	receive_frame[i];//将接收缓冲区中未处理的数据放到缓冲窗中，准备格雷译码
				}
				decode_error_catch(gray_decode_buf,gray_decoded_buf);//格雷译码,纠错结果为gray_decoded_buf[]，前12位码元为原始数据的倒序排列
				i=gray_decoded_buf[11]*2+gray_decoded_buf[10];
				printf("\r\n帧类型i=%d\r\n",i);
				switch(i){
					case 0:
					frame_lengths=ORIGEN_FRAMESIZE-20;//待修改
					break;//续传帧
					case 1:
					tmp_buf=gray_decoded_buf[9]*2+gray_decoded_buf[8];
					if(tmp_buf==2){frame_lengths=LENGTHS_WAKEUPFRAME_BROADCAST;} //广播
					else if(tmp_buf==1){frame_lengths=LENGTHS_WAKEUPFRAME_UNICAST;}//单播
					else if(tmp_buf==3){frame_lengths=LENGTHS_WAKEUPFRAME_MULTICAST;}//组播
					break;//唤醒帧
					case 2:
					frame_lengths=LENGTHS_CONTROLFRAME;
					break;//控制帧
					case 3:
					frame_lengths=LENGTHS_SECUREFRAME;
					break;//认证帧
				}
				TIM5CH1_CAPTURE_STA|=0X0800;//帧实际长度获取完毕
			}	
			//提取帧类型设定帧长，结束	

				frame_index++;
			}
			if(frame_index==frame_lengths) //比对上一个if用else做如下工作提速了一个中短周期：1ms
			{
				PBout(6)=0;
				TIM5CH1_CAPTURE_STA|=0X08;//帧收完
				TIM5->ARR=0XFFFF;//0X0031;//重新状态定时器的值
				TIM_ITConfig(TIM5,TIM_IT_CC2,ENABLE);//TIM_IT_CC1,接收新的帧，打开输入捕获
				TIM_OC2PolarityConfig(TIM5,TIM_ICPolarity_Rising); //CC1P=0 设置为上升沿捕获
				TIM_ITConfig(TIM5, TIM_IT_Update|TIM_IT_CC2, DISABLE); //关中断
				TIM_Cmd(TIM5,DISABLE);//原始帧接收完毕，关闭定时器节约资源，等待解帧处理，处理完才使能接受下一帧
				
				printf("\r\n广播类型G=%d\r\n",tmp_buf);
				printf("\r\n比特收完!\r\n");
			}
		}
	}

    TIM_ClearITPendingBit(TIM5, TIM_IT_CC2|TIM_IT_Update); //TIM_IT_CC1,清除中断标志位	  
}

