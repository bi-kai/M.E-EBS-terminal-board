#include "timer.h"
#include "led.h"
#include "usart.h"
#include "delay.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK Mini STM32������
//PWM  ��������			   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2010/12/03
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ����ԭ�� 2009-2019
//All rights reserved
////////////////////////////////////////////////////////////////////////////////// 	  

//ͨ�ö�ʱ���жϳ�ʼ��
//����ʱ��ѡ��ΪAPB1��2������APB1Ϊ36M
//arr���Զ���װֵ��
//psc��ʱ��Ԥ��Ƶ��
//����ʹ�õ��Ƕ�ʱ��3!

void TIM3_Int_Init(u16 arr,u16 psc)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); //ʱ��ʹ��

	TIM_TimeBaseStructure.TIM_Period = arr; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ	 ������5000Ϊ500ms
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ  10Khz�ļ���Ƶ��  
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //����TIM_TimeBaseInitStruct��ָ���Ĳ�����ʼ��TIMx��ʱ�������λ
 
	TIM_ITConfig(  //ʹ�ܻ���ʧ��ָ����TIM�ж�
		TIM3, //TIM2
		TIM_IT_Update  |  //TIM �ж�Դ
		TIM_IT_Trigger,   //TIM �����ж�Դ 
		ENABLE  //ʹ��
		);
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;  //TIM3�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //��ռ���ȼ�0��
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //�����ȼ�3��
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQͨ����ʹ��
	NVIC_Init(&NVIC_InitStructure);  //����NVIC_InitStruct��ָ���Ĳ�����ʼ������NVIC�Ĵ���

	TIM_Cmd(TIM3, ENABLE);  //ʹ��TIMx����
							 
}

void TIM3_IRQHandler(void)   //TIM3�ж�
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) //���ָ����TIM�жϷ������:TIM �ж�Դ 
		{
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update  );  //���TIMx���жϴ�����λ:TIM �ж�Դ 
		LED1=!LED1;
		}
}




//PWM�����ʼ��
//arr���Զ���װֵ
//psc��ʱ��Ԥ��Ƶ��

void TIM3_PWM_Init(u16 arr,u16 psc)
{  
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB  | RCC_APB2Periph_AFIO, ENABLE);  //ʹ��GPIO�����AFIO���ù���ģ��ʱ��ʹ��
	
	GPIO_PinRemapConfig(GPIO_PartialRemap_TIM3, ENABLE); //Timer3������ӳ��  TIM3_CH2->PB5                                                                       	 //����TIM3��CH2�����PWMͨ����LED��ʾ
 
   //���ø�����Ϊ�����������,���TIM3 CH2��PWM���岨��
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5; //TIM_CH2
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //�����������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	//GPIO_WriteBit(GPIOA, GPIO_Pin_7,Bit_SET); // PA7����	

	TIM_TimeBaseStructure.TIM_Period = arr; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ	 80K
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ  ����Ƶ
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //����TIM_TimeBaseInitStruct��ָ���Ĳ�����ʼ��TIMx��ʱ�������λ
	
	 
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2; //ѡ��ʱ��ģʽ:TIM�����ȵ���ģʽ2
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //�Ƚ����ʹ��
	TIM_OCInitStructure.TIM_Pulse = 0; //���ô�װ�벶��ȽϼĴ���������ֵ
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //�������:TIM����Ƚϼ��Ը�
	TIM_OC2Init(TIM3, &TIM_OCInitStructure);  //����TIM_OCInitStruct��ָ���Ĳ�����ʼ������TIMx
	TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable);  //ʹ��TIMx��CCR2�ϵ�Ԥװ�ؼĴ���
	
	TIM_ARRPreloadConfig(TIM3, ENABLE); //ʹ��TIMx��ARR�ϵ�Ԥװ�ؼĴ���
	
 
	TIM_Cmd(TIM3, ENABLE);  //ʹ��TIMx����
 

}

//��ʱ��5ͨ��1���벶������

TIM_ICInitTypeDef  TIM5_ICInitStructure;

void TIM5_Cap_Init(u16 arr,u16 psc)
{	 
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
   	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);	//ʹ��TIM5ʱ��
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB, ENABLE);  //ʹ��GPIOAʱ��
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_0;  //PA0 ���֮ǰ����  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; //PA0 ����  
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOA,GPIO_Pin_0);						 //PA0 ����

//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6; //TIM_CH2
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //�����������
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_Init(GPIOB, &GPIO_InitStructure);
// 	GPIO_ResetBits(GPIOB,GPIO_Pin_6);
 	
	//��ʼ����ʱ��5 TIM5	 
	TIM_TimeBaseStructure.TIM_Period = arr; //�趨�������Զ���װֵ 
	TIM_TimeBaseStructure.TIM_Prescaler =psc; 	//Ԥ��Ƶ��   
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure); //����TIM_TimeBaseInitStruct��ָ���Ĳ�����ʼ��TIMx��ʱ�������λ
  
	//��ʼ��TIM5���벶�����
	TIM5_ICInitStructure.TIM_Channel = TIM_Channel_1; //CC1S=01 	ѡ������� IC1ӳ�䵽TI1��
  	TIM5_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;	//�����ز���
  	TIM5_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI; //ӳ�䵽TI1��
  	TIM5_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;	 //���������Ƶ,����Ƶ 
  	TIM5_ICInitStructure.TIM_ICFilter = 0x11;//IC1F=0000 ���������˲��� ���˲�
  	TIM_ICInit(TIM5, &TIM5_ICInitStructure);
	
	//�жϷ����ʼ��
	NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;  //TIM3�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;  //��ռ���ȼ�2��
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;  //�����ȼ�0��
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQͨ����ʹ��
	NVIC_Init(&NVIC_InitStructure);  //����NVIC_InitStruct��ָ���Ĳ�����ʼ������NVIC�Ĵ��� 
	
	TIM_ITConfig(TIM5,TIM_IT_Update|TIM_IT_CC1,ENABLE);//��������ж� ,����CC1IE�����ж�	
	
   	TIM_Cmd(TIM5,ENABLE ); 	//ʹ�ܶ�ʱ��5
   


}

u16  TIM5CH1_CAPTURE_STA=0;//���벶��״̬		    				
u16	TIM5CH1_CAPTURE_VAL;	//���벶��ֵ
u16	TIM5CH1_DOWN_CAPTURE_VAL;	//�����½���ʱ�䲶��ֵ 
u16 bit_SYCN_UP[4];	 //λͬ��4���ߵ�ƽ����ʱ�䣬����ͳ����ѳ���ʱ��
u16 bit_SYCN_DOWN[4];//λͬ��4���͵�ƽ����ʱ�䣬����ͳ����ѳ���ʱ��

u8 receive_frame[max_framesize];//����֡�Ļ�����
u16 frame_index=0;//����֡�У�����λ��������洢������ֵ

u8 buf_barker[12];//�Ϳ��뻺������11λ�Ϳ���+1���µ�bit����12λ�ռ�
u8 barker[]={1,1,1,0,0,0,1,0,0,1,0};//׼ȷ�İͿ�����
signed char barker_sum=0;//�Ϳ�����λ�Ĵ����������Ϊ����
//��ʱ��5�жϷ������	 
void TIM5_IRQHandler(void)
{ 
   static u8 bit_counter_up=0;//λͬ��ʱ��ͳ�Ƶ�n��������
   static u8 bit_counter_down=0;//λͬ��ʱ��ͳ�Ƶ�n��������
 
   u8 i;//����forѭ��������
/*********************************λͬ������*************************************/   
 	if((TIM5CH1_CAPTURE_STA&0X80)==0)//λͬ�����񣬻�δ�ɹ�����(���Ȳ���1��)	
	{	  
		if (TIM_GetITStatus(TIM5, TIM_IT_Update) != RESET) //��ʱ����ʱ����ȫ�����		 
		{	    
			TIM5CH1_CAPTURE_STA=0;
			TIM5CH1_CAPTURE_VAL=0; 
			TIM_OC1PolarityConfig(TIM5,TIM_ICPolarity_Rising); //CC1P=0 ����Ϊ�����ز���

			bit_counter_up=0;//��Ǹߵ�ƽ����Ϊ��
			bit_counter_down=0;//��ǵ͵�ƽ����Ϊ��
			frame_index=0;//֡�������㣬�ȴ���һ֡
		}

		 if(TIM5CH1_CAPTURE_STA==0)//���ƼĴ���������գ���־��һ֡������ϣ���ʼ�����µ�֡
		 {
		 	TIM_ITConfig(TIM5,TIM_IT_CC1,ENABLE);//�����µ�֡�������벶��
			TIM_OC1PolarityConfig(TIM5,TIM_ICPolarity_Rising); //CC1P=0 ����Ϊ�����ز���
			
		 }
		   
		if (TIM_GetITStatus(TIM5, TIM_IT_CC1) != RESET)//����1���������¼�
		{	
			if(TIM5CH1_CAPTURE_STA&0X40)		//����һ���½��� 		
			{	 
 			
				TIM5CH1_CAPTURE_VAL=TIM_GetCapture1(TIM5);		

				
				if((TIM5CH1_CAPTURE_VAL>min_interval)&&(TIM5CH1_CAPTURE_VAL<max_interval))//����һ���������ڵĸ�����
				{
					
					if((TIM5CH1_CAPTURE_STA&0X07)<4)//���񵽺��ʵĸߵ�ƽ�͵͵�ƽ��
					{
						TIM5CH1_CAPTURE_STA++;
						bit_SYCN_UP[bit_counter_up]=TIM5CH1_CAPTURE_VAL+4;  //�߶�ƽ����ʱ����4ns���͵�ƽ����	
						bit_counter_up++;								   //2 
					}
				}else if((TIM5CH1_CAPTURE_VAL>2*min_interval)&&(TIM5CH1_CAPTURE_VAL<2*max_interval))//��������2�����룬2���������ڵĸߵ�ƽ����һ����ȷ�ĸ�����
				{
					if((TIM5CH1_CAPTURE_STA&0X07)<4)//���񵽺��ʵĸߵ�ƽ�͵͵�ƽ��
					{
						TIM5CH1_CAPTURE_STA++;
						bit_SYCN_UP[bit_counter_up]=TIM5CH1_CAPTURE_VAL/2;   //1
						bit_counter_up++;								   //2 
					}
				}else if((TIM5CH1_CAPTURE_VAL>3*min_interval)&&(TIM5CH1_CAPTURE_VAL<3*max_interval))//�������룬0->1��3���������ڵĸߵ�ƽ
				{
					if((TIM5CH1_CAPTURE_STA&0X07)<4)//���񵽺��ʵĸߵ�ƽ�͵͵�ƽ��
					{
						TIM5CH1_CAPTURE_STA+=2;
						bit_SYCN_UP[bit_counter_up]=TIM5CH1_CAPTURE_VAL/3;   //1
						bit_counter_up++;								   //2
						bit_SYCN_UP[bit_counter_up]=TIM5CH1_CAPTURE_VAL/3;   //1
						bit_counter_up++;								   //2 
					}
				}else if((TIM5CH1_CAPTURE_VAL>4*min_interval)&&(TIM5CH1_CAPTURE_VAL<4*max_interval))//λͬ���ڰ�λ0->1����Ϳ�������Ϊ4���������ڸߵ�ƽ������������費��Ҫ��5����1���ж�
				{
					if(((TIM5CH1_CAPTURE_STA&0X07)<4)&&((TIM5CH1_CAPTURE_STA&0X07)>=2))//��֤ǰ���Ѿ����ٲ�����2����Ч�ߵ�ƽ
					{
						TIM5CH1_CAPTURE_STA+=2;
						bit_SYCN_UP[bit_counter_up]=TIM5CH1_CAPTURE_VAL/4;   //1
						bit_counter_up++;								   //2 
						bit_SYCN_UP[bit_counter_up]=TIM5CH1_CAPTURE_VAL/4;   //1
						bit_counter_up++;								   //2 
					}
				}else //���㣬�ȴ����½���
				{
					TIM5CH1_CAPTURE_STA=0;//�Ƿ�0�����壬�ر��������
					TIM5CH1_CAPTURE_VAL=0;
						bit_counter_up=0;//��Ǹߵ�ƽ����Ϊ��							
				}
				if(bit_counter_up==4){bit_counter_up=0;}		   //3
				TIM5CH1_CAPTURE_STA&=0XBF; //��������ز����־λ
				TIM5CH1_CAPTURE_STA|=0X20;//�͵�ƽʹ�ܼ�����־λ
		   		TIM_OC1PolarityConfig(TIM5,TIM_ICPolarity_Rising); //CC1P=0 ����Ϊ�����ز���
			}else//��δ��ʼ,��һ�β���������
			{  
				TIM5CH1_DOWN_CAPTURE_VAL=TIM_GetCapture1(TIM5)-TIM5CH1_CAPTURE_VAL;//�õ��͵�ƽ������ʱ��
				if((TIM5CH1_DOWN_CAPTURE_VAL>min_interval)&&(TIM5CH1_DOWN_CAPTURE_VAL<max_interval)) //����1���������ڵĵ͵�ƽ
				{
					if(TIM5CH1_CAPTURE_STA&0X20) //�͵�ƽ�����Ѿ�ʹ��
					{
						bit_SYCN_DOWN[bit_counter_down]=TIM5CH1_DOWN_CAPTURE_VAL;   //1
						bit_counter_down++;								   //2
					}
					
				}else if((TIM5CH1_DOWN_CAPTURE_VAL>2*min_interval)&&(TIM5CH1_DOWN_CAPTURE_VAL<2*max_interval))//�������룬2���������ڵĵ͵�ƽ
				{
					if(TIM5CH1_CAPTURE_STA&0X20) //�͵�ƽ�����Ѿ�ʹ��
					{
						TIM5CH1_CAPTURE_STA++;//���������͵�ƽ������һ�����͵ı�Ȼ�Ǹߵ�ƽ
						bit_SYCN_DOWN[bit_counter_down]=TIM5CH1_DOWN_CAPTURE_VAL/2;   //1
						bit_counter_down++;								   //2
					}
				}else if((TIM5CH1_DOWN_CAPTURE_VAL>3*min_interval)&&(TIM5CH1_DOWN_CAPTURE_VAL<3*max_interval))//�������룬3���������ڵĵ͵�ƽ
				{
					if(TIM5CH1_CAPTURE_STA&0X20) //�͵�ƽ�����Ѿ�ʹ��
					{
						TIM5CH1_CAPTURE_STA++;//���������͵�ƽ������һ�����͵ı�Ȼ�Ǹߵ�ƽ
						bit_SYCN_DOWN[bit_counter_down]=TIM5CH1_DOWN_CAPTURE_VAL/3;   //1
						bit_counter_down++;								   //2
					}
				}else//���㣬��һ�������ص�����ͬ��ʧ��
				{
					TIM5CH1_CAPTURE_STA=0;	//���
					bit_counter_down=0;//��ǵ͵�ƽ����Ϊ��
				}

			    if((TIM5CH1_CAPTURE_STA&0X07)>3)//����������3��10������ϣ���ʼ�Ϳ�����֤
				{
					TIM5CH1_CAPTURE_STA|=0X80; //�õ�λͬ��ͷ������
					TIM_ITConfig(TIM5,TIM_IT_CC1,DISABLE);//�ر����벶��׼�����հͿ���
					delay_us((max_interval+min_interval)/4-1);//��Ԫ���50ns����Ԫ���м�λ��Ϊ��ѳ���ʱ��
					TIM5->ARR=(max_interval+min_interval)/2-1;//0X0031;//����״̬��ʱ����ֵ��50ms�жϣ���PAin(0)����һ��(0x0030����������Ԫ�����һ�Σ�0X0031Ч���ȽϺã�0X0032���������ٲ���һ��)
					buf_barker[10]=GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0);//λͬ�����µ���ĵ�һλ�洢�ڰͿ���buf�ĵ�10λ					
				}
				if(bit_counter_down==4){bit_counter_down=0;}		   //3
				TIM5CH1_CAPTURE_VAL=0;	
	 			TIM_SetCounter(TIM5,0);
				TIM5CH1_CAPTURE_STA|=0X40;		//��ǲ�����������

				if(TIM5CH1_CAPTURE_STA&0X80)
				{
					 TIM_OC1PolarityConfig(TIM5,TIM_ICPolarity_Rising); //CC1P=0 ����Ϊ�����ز���
				}
				else
				{
		   			TIM_OC1PolarityConfig(TIM5,TIM_ICPolarity_Falling);		//CC1P=1 ����Ϊ�½��ز���
				}
			}		    
		}			     	    					   
 	}
/****************************************�Ϳ���У��*******************************************/	
	if((TIM5CH1_CAPTURE_STA&0X80)&&((TIM5CH1_CAPTURE_STA&0X10)==0))//λͬ��֮�󣬿�ʼ�жϰͿ����֡ͬ��
	{
	   if (TIM_GetITStatus(TIM5, TIM_IT_Update) != RESET)//λͬ��������ɺ�ĳ�ʱ����
	   {
		   buf_barker[11]=GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0);//λͬ�����µ����bit

		   for(i=0;i<11;i++)//�յ��������ݺ󣬽�bufѭ������һλ
		   {
		   		buf_barker[i]=buf_barker[i+1];
		   }
		   for(i=0;i<11;i++)
		   {
			   if(buf_barker[i]==barker[i]){barker_sum++;}
			   else barker_sum--;
		   }
		   if(barker_sum>=10){TIM5CH1_CAPTURE_STA|=0X10; frame_index=0;}//֡�������㣬׼����������
		   else barker_sum=0;//��������֤������¿�ʼ
	   }
	}else if((TIM5CH1_CAPTURE_STA&0X80)&&(TIM5CH1_CAPTURE_STA&0X10)&&((TIM5CH1_CAPTURE_STA&0X08)==0))
	{
		if (TIM_GetITStatus(TIM5, TIM_IT_Update)!= RESET)//��ʼ���հͿ�����������
		{
			if(frame_index<(max_framesize))
			{
				receive_frame[frame_index]=GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0); //��ǰʱ�̾���λͬ�����һ����Ԫ����ѳ���ʱ��
				frame_index++;
			}else 
			{
				TIM5CH1_CAPTURE_STA|=0X08;//֡����
				bit_counter_up=0;
				bit_counter_down=0;
				TIM5->ARR=0XFFFF;//0X0031;//����״̬��ʱ����ֵ
				TIM_Cmd(TIM5,DISABLE);
			}
		}
	}




    TIM_ClearITPendingBit(TIM5, TIM_IT_CC1|TIM_IT_Update); //����жϱ�־λ	  
}
