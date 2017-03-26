#include "timer.h"
#include "led.h"
#include "usart.h"
#include "delay.h"
#include "gray.h"
#include "stmflash.h"
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

extern u32 frame_counters;//����֡��bitsת֡������
u16 flash_framecounter_nums=0;//ÿ��5�룬������300���洢һ��֡��������ֵ
extern u8 flag_main_busy;//����������æ��ֹTIM3flash�жϱ�־λ

extern u8 sage_confirmd;//��ȫ��֤�Ƿ��յ���0��δ�յ���1�����յ���

void TIM3_IRQHandler(void)   //TIM3�ж�
{
	u8 flash_temp[4]={0};
	u32 framecounter=0;

	static u8 count_safe=0;//����ֵ

	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET) //���ָ����TIM�жϷ������:TIM �ж�Դ 
		{
			TIM_ClearITPendingBit(TIM3, TIM_IT_Update);  //���TIMx���жϴ�����λ:TIM �ж�Դ 
			LED1=!LED1;
			if(flash_framecounter_nums<FLASH_CHECK_TIMES){
				flash_framecounter_nums++;
			}else if((flash_framecounter_nums>FLASH_CHECK_TIMES)&&(flag_main_busy==0)){ //��ʼ�洢��flash��
				STMFLASH_Read(0x08008000,(u16*)flash_temp,4);
				framecounter=flash_temp[0]*256*256*256+flash_temp[1]*256*256+flash_temp[2]*256+flash_temp[3];//����ʱ����flash�е�֡������ֵ
				if(framecounter<frame_counters){
					TIM_Cmd(TIM2,DISABLE ); 	//ʧ�ܶ�ʱ��2
				 	flash_temp[0]=frame_counters/(256*256*256);
					flash_temp[1]=frame_counters/(256*256);
					flash_temp[2]=frame_counters/256;
					flash_temp[3]=frame_counters%256;
					STMFLASH_Write(0x08008000,(u16*)flash_temp,4);//��֡����ֵ���ĸ��ֽڱ��浽flash��
					flash_framecounter_nums=0;
					TIM_Cmd(TIM2,ENABLE ); 	//ʹ�ܶ�ʱ��2
				}
				
			}
		  /***********************����֤֡��ͳ�ƣ�10sһ��****************************/
			if(count_safe==10){//1s�ж�һ�Σ�����10�Σ�����һ��
				count_safe=0;
				if(sage_confirmd==1){
					communication_right();
				}
				else{
//					communication_wrong();
				}
				sage_confirmd=0;
				
			}else{
				count_safe++;	
			}
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

//��ʱ��2ͨ��1���벶������

TIM_ICInitTypeDef  TIM5_ICInitStructure;

void TIM2_Cap_Init(u16 arr,u16 psc)
{	 
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
   	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);	//ʹ��TIM5ʱ��
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);  //ʹ��GPIOAʱ��
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_1;  //PA1 ���֮ǰ����  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //PA1 ����  
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOA,GPIO_Pin_1);						 //PA1 ����


 	
	//��ʼ����ʱ��5 TIM5	 
	TIM_TimeBaseStructure.TIM_Period = arr; //�趨�������Զ���װֵ 
	TIM_TimeBaseStructure.TIM_Prescaler =psc; 	//Ԥ��Ƶ��   
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure); //����TIM_TimeBaseInitStruct��ָ���Ĳ�����ʼ��TIMx��ʱ�������λ
  
	//��ʼ��TIM5���벶�����
	TIM5_ICInitStructure.TIM_Channel = TIM_Channel_2; //CC1S=01 	ѡ������� IC1ӳ�䵽TI1��
  	TIM5_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;	//�����ز���
  	TIM5_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI; //ӳ�䵽TI1��
  	TIM5_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;	 //���������Ƶ,����Ƶ 
  	TIM5_ICInitStructure.TIM_ICFilter = 0x11;//IC1F=0000 ���������˲��� ���˲�
  	TIM_ICInit(TIM2, &TIM5_ICInitStructure);
	
	//�жϷ����ʼ��
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;  //TIM3�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;  //��ռ���ȼ�2��
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;  //�����ȼ�0��
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQͨ����ʹ��
	NVIC_Init(&NVIC_InitStructure);  //����NVIC_InitStruct��ָ���Ĳ�����ʼ������NVIC�Ĵ��� 
	
	TIM_ITConfig(TIM2,TIM_IT_Update|TIM_IT_CC2,ENABLE);//TIM_IT_CC1,��������ж� ,����CC1IE�����ж�	
	
   	TIM_Cmd(TIM2,ENABLE ); 	//ʹ�ܶ�ʱ��5
   


}

u16 TIM5CH1_CAPTURE_STA=0;//���벶��״̬		    				
u16	TIM5CH1_CAPTURE_VAL=0;	//���벶��ֵ
u16	TIM5CH1_DOWN_CAPTURE_VAL=0;	//�����½���ʱ�䲶��ֵ 
u16 bit_SYCN_UP[BIT_SYNC_GROUPS]={0};//λͬ��4���ߵ�ƽ����ʱ�䣬����ͳ����ѳ���ʱ��
u16 bit_SYCN_DOWN[BIT_SYNC_GROUPS]={0};//λͬ��4���͵�ƽ����ʱ�䣬����ͳ����ѳ���ʱ��

u8 receive_frame[ORIGEN_FRAMESIZE]={0};//����֡�Ļ�����
u16 frame_index=0;//����֡�У�����λ��������洢������ֵ

u8 buf_barker[12]={0};//�Ϳ��뻺������11λ�Ϳ���+1���µ�bit����12λ�ռ�
u8 barker[]={1,1,1,0,0,0,1,0,0,1,0};//׼ȷ�İͿ�����
signed char barker_sum=0;//�Ϳ�����λ�Ĵ����������Ϊ����
u16 frame_lengths=0;//��ǰ֡�ܳ��� 
extern u8 frame_window_counter;//��ʾ������Ļ�������ͳ��������������24λλһ������
extern u16 decoded_frame_index;//����������󻺳������������ֵ


u8 tmp_buf=0;
//��ʱ��2�жϷ������	 
void TIM2_IRQHandler(void)
{ 
   static u8 bit_counter_up=0;//λͬ��ʱ��ͳ�Ƶ�n��������
   static u8 bit_counter_down=0;//λͬ��ʱ��ͳ�Ƶ�n��������
   static u8 timeout_flag=0;//0��TIM5 update���ʱ����û����Ԫ����1�����ʱ��������Ԫ�����񵽡����ܣ���ֹ֡����������update����	
   static u8 barker_counter=0;//�Ϳ�����λ�Ƚϴ���������20�λ�û��֡ͬ���ϣ�����ռĴ���
   u16 bit_SYCN_sum=0;//λͬ���ߵ͵�ƽ��Ԫ����ʱ���

   u8 gray_decode_buf[24]={0};//�������룬24����ԪΪһ��
   u8 gray_decoded_buf[24]={0};//�������������24λ��Ԫ����ǰ12λΪԭʼ��Ϣ�ĵ�������
 
   u8 i=0;  //����forѭ��������
// signed char temp=0;
   TIM_Cmd(TIM3,DISABLE);//�ر�TIM3   
/*********************************λͬ������*************************************/   
 	if((TIM5CH1_CAPTURE_STA&0X80)==0)//λͬ�����񣬻�δ�ɹ�����(���Ȳ���1��)	
	{	  
		if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) //65.535��ʱ����ʱ����ȫ����գ����⣬��Ҫ�����������ж������ʱ��		 
		{	 
			if(timeout_flag==0){   //updateʱ����ֵ����Ϊ�㣬˵�����ʱ����û����Ԫ���ʲ���֡���м䲿λ���Ĵ������㣬�ȴ�֡����
				TIM5CH1_CAPTURE_STA=0;
				TIM_Cmd(TIM3, ENABLE);
				TIM5CH1_CAPTURE_VAL=0; 
				TIM_OC2PolarityConfig(TIM2,TIM_ICPolarity_Rising); //CC1P=0 ����Ϊ�����ز���
				bit_counter_up=0;//��Ǹߵ�ƽ����Ϊ��
				bit_counter_down=0;//��ǵ͵�ƽ����Ϊ��
				frame_index=0;//֡�������㣬�ȴ���һ֡		
			}else{
				timeout_flag=0;
			}
		}

		 if(TIM5CH1_CAPTURE_STA==0)//���ƼĴ���������գ���־��һ֡�������/�ߵ�ƽ��Ԫ��ʱ/��һ֡����ʼ�����µ�֡
		 {
		 	TIM_ITConfig(TIM2,TIM_IT_CC2,ENABLE);//TIM_IT_CC1,�����µ�֡�������벶��
			TIM_OC2PolarityConfig(TIM2,TIM_ICPolarity_Rising); //CC1P=0 ����Ϊ�����ز��� 
			bit_counter_up=0;//��Ǹߵ�ƽ����Ϊ��
			bit_counter_down=0;
			TIM5CH1_CAPTURE_VAL=0; 			
		 }
		   
		if (TIM_GetITStatus(TIM2, TIM_IT_CC2) != RESET)//TIM_IT_CC1,����1�����������¼�
		{	
			timeout_flag=1;	//�ر�update����
			if(TIM5CH1_CAPTURE_STA&0X40)		//����һ���½��� 		
			{	 	  			
				TIM5CH1_CAPTURE_VAL=TIM_GetCapture2(TIM2);//TIM_GetCapture1
				
				if((TIM5CH1_CAPTURE_VAL>min_interval)&&(TIM5CH1_CAPTURE_VAL<max_interval))//����һ���������ڵĸ�����
				{				 					
					if((TIM5CH1_CAPTURE_STA&0X07)<BIT_SYNC_GROUPS)//���񵽺��ʵĸߵ�ƽ�͵͵�ƽ��
					{
						TIM5CH1_CAPTURE_STA++;
						bit_SYCN_UP[bit_counter_up]=TIM5CH1_CAPTURE_VAL+3;  //�߶�ƽ����ʱ������3us���͵�ƽ����1us�����	
						bit_counter_up++;								   //2 
					}
				}else if((TIM5CH1_CAPTURE_VAL>2*min_interval)&&(TIM5CH1_CAPTURE_VAL<2*max_interval))//��������2�����룬2���������ڵĸߵ�ƽ����һ����ȷ�ĸ�����
				{
					if((TIM5CH1_CAPTURE_STA&0X07)<BIT_SYNC_GROUPS)//���񵽺��ʵĸߵ�ƽ�͵͵�ƽ��
					{
						TIM5CH1_CAPTURE_STA++;
						bit_SYCN_UP[bit_counter_up]=TIM5CH1_CAPTURE_VAL/2+3;   //1
						bit_counter_up++;
						bit_SYCN_DOWN[bit_counter_down]=TIM5CH1_DOWN_CAPTURE_VAL/2-1;   //1
						bit_counter_down++;								   //2 
					}
				}
//				else if((TIM5CH1_CAPTURE_VAL>3*min_interval)&&(TIM5CH1_CAPTURE_VAL<3*max_interval))//�������룬0->1��3���������ڵĸߵ�ƽ
//				{
//					if((TIM5CH1_CAPTURE_STA&0X07)<BIT_SYNC_GROUPS)//���񵽺��ʵĸߵ�ƽ�͵͵�ƽ��
//					{
//						TIM5CH1_CAPTURE_STA+=2;
//						bit_SYCN_UP[bit_counter_up]=TIM5CH1_CAPTURE_VAL/3+3;   //1
//						bit_counter_up++;
//						bit_SYCN_DOWN[bit_counter_down]=TIM5CH1_DOWN_CAPTURE_VAL/3-1;   //1
//						bit_counter_down++;								   //2
//						bit_SYCN_UP[bit_counter_up]=TIM5CH1_CAPTURE_VAL/3+3;   //1
//						bit_counter_up++;								   //2 
//					}
//				}else if((TIM5CH1_CAPTURE_VAL>4*min_interval)&&(TIM5CH1_CAPTURE_VAL<4*max_interval))//λͬ���ڰ�λ0->1����Ϳ�������Ϊ4���������ڸߵ�ƽ������������費��Ҫ��5����1���ж�
//				{
//					if(((TIM5CH1_CAPTURE_STA&0X07)<BIT_SYNC_GROUPS)&&((TIM5CH1_CAPTURE_STA&0X07)>=2))//��֤ǰ���Ѿ����ٲ�����2����Ч�ߵ�ƽ
//					{
////						TIM5CH1_CAPTURE_STA+=1;
////						bit_SYCN_UP[bit_counter_up]=TIM5CH1_CAPTURE_VAL/4;   //1
////						bit_counter_up++;								   //2 
//						bit_SYCN_DOWN[bit_counter_down]=TIM5CH1_DOWN_CAPTURE_VAL/4-1;   //1
//						bit_counter_down++;
//						
//					}
//				}
				else //���㣬�ȴ����½���
				{
					TIM5CH1_CAPTURE_STA=0;//�Ƿ�0�����壬�ر��������
					TIM_Cmd(TIM3, ENABLE);
					TIM5CH1_CAPTURE_VAL=0;
					bit_counter_up=0;//��Ǹߵ�ƽ����Ϊ��
					bit_counter_down=0;							
				}
//				if((bit_counter_down>=BIT_SYNC_GROUPS)||(bit_counter_up>=BIT_SYNC_GROUPS)){bit_counter_down=0;bit_counter_up=0;}		   //3
				TIM5CH1_CAPTURE_STA&=0XBF; //��������ز����־λ��׼��������һ��������
				TIM5CH1_CAPTURE_STA|=0X20;//�͵�ƽʹ�ܼ�����־λ����һ��1���ʹ��0�������
		   		TIM_OC2PolarityConfig(TIM2,TIM_ICPolarity_Rising); //CC1P=0 ����Ϊ�����ز���
			}else//��δ��ʼ,��һ�β���������
			{  
				TIM5CH1_DOWN_CAPTURE_VAL=TIM_GetCapture2(TIM2)-TIM5CH1_CAPTURE_VAL;//TIM_GetCapture1,�õ��͵�ƽ������ʱ��
				if((TIM5CH1_DOWN_CAPTURE_VAL>min_interval)&&(TIM5CH1_DOWN_CAPTURE_VAL<max_interval)) //����1���������ڵĵ͵�ƽ
				{
					if(TIM5CH1_CAPTURE_STA&0X20) //�͵�ƽ�����Ѿ�ʹ��
					{
						bit_SYCN_DOWN[bit_counter_down]=TIM5CH1_DOWN_CAPTURE_VAL-1;   //1
						bit_counter_down++;								   //2
					}
					
				}else if((TIM5CH1_DOWN_CAPTURE_VAL>2*min_interval)&&(TIM5CH1_DOWN_CAPTURE_VAL<2*max_interval))//�������룬2���������ڵĵ͵�ƽ
				{
					if(TIM5CH1_CAPTURE_STA&0X20) //�͵�ƽ�����Ѿ�ʹ��
					{
						TIM5CH1_CAPTURE_STA++;//���������͵�ƽ������һ�����͵ı�Ȼ�Ǹߵ�ƽ
						bit_SYCN_UP[bit_counter_up]=TIM5CH1_CAPTURE_VAL/2+3;   //1
						bit_counter_up++;
						bit_SYCN_DOWN[bit_counter_down]=TIM5CH1_DOWN_CAPTURE_VAL/2-1;   //1
						bit_counter_down++;								   //2
					}
				}
//				else if((TIM5CH1_DOWN_CAPTURE_VAL>3*min_interval)&&(TIM5CH1_DOWN_CAPTURE_VAL<3*max_interval))//�������룬3���������ڵĵ͵�ƽ
//				{
//					if(TIM5CH1_CAPTURE_STA&0X20) //�͵�ƽ�����Ѿ�ʹ��
//					{
//						TIM5CH1_CAPTURE_STA++;//���������͵�ƽ������һ�����͵ı�Ȼ�Ǹߵ�ƽ
//						bit_SYCN_UP[bit_counter_up]=TIM5CH1_CAPTURE_VAL/3+3;   //1
//						bit_counter_up++;
//						bit_SYCN_DOWN[bit_counter_down]=TIM5CH1_DOWN_CAPTURE_VAL/3-1;   //1
//						bit_counter_down++;								   //2
//						bit_SYCN_DOWN[bit_counter_down]=TIM5CH1_DOWN_CAPTURE_VAL/3-1;   //1
//						bit_counter_down++;								   //2
//					}
//				}
				else//���㣬��һ�������ص�����ͬ��ʧ��
				{
					TIM5CH1_CAPTURE_STA=0;	//���
					TIM_Cmd(TIM3, ENABLE);
					bit_counter_down=0;//��ǵ͵�ƽ����Ϊ��
					bit_counter_up=0;
				}

			    if((TIM5CH1_CAPTURE_STA&0X07)>=(BIT_SYNC_GROUPS-2))//����������3��10������ϣ�������ʱ����ƽ������ʱ�̣���ʼ�Ϳ�����֤
				{
					TIM5CH1_CAPTURE_STA|=0X80; //�õ�λͬ��ͷ������
					TIM_ITConfig(TIM2,TIM_IT_CC2,DISABLE);//TIM_IT_CC1,�ر����벶��׼�����հͿ���
					bit_SYCN_sum=0;
					for(i=0;i<bit_counter_up;i++)
					{
						bit_SYCN_sum+=bit_SYCN_UP[i];
					}
					for(i=0;i<bit_counter_down;i++)
					{
						bit_SYCN_sum+=bit_SYCN_DOWN[i];
					}
					bit_SYCN_sum+=50;//���ڲɵ��ĸߵ����嶼����һ����������1000us�Ĳɳ�998us���Ըñ�����50usΪ�����������������������ȡģʱ����if����
					bit_SYCN_sum%=FSK_SPEED;
					if(bit_SYCN_sum>((max_interval+min_interval)/4))//�о�ͬ��ʱ�̳���1/2��Ԫ���
					{
						delay_us(3*(max_interval+min_interval)/4-bit_SYCN_sum-10);//��Ԫ���1ms����Ԫ���м�λ��Ϊ��ѳ���ʱ��,50Ϊ��ʾ�����۲����ʱ�̺������
					}else
					{
						delay_us((max_interval+min_interval)/4-bit_SYCN_sum-10);//��Ԫ���1ms����Ԫ���м�λ��Ϊ��ѳ���ʱ��,50Ϊ��ʾ�����۲����ʱ�̺������
					}
					buf_barker[10]=GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_1);//λͬ�����µ���ĵ�һλ�洢�ڰͿ���buf�ĵ�10λ
					TIM2->ARR=(max_interval+min_interval)/2-1;//0X0031;//����״̬��ʱ����ֵ��1ms�жϣ���PAin(0)����һ��(0x0030����������Ԫ�����һ�Σ�0X0031Ч���ȽϺã�0X0032���������ٲ���һ��)
					bit_counter_down=0;
					bit_counter_up=0;
					barker_counter=0;//�Ϳ���Ƚϴ���������
				//	printf("\r\nλͬ��!"); 					
				}
				if((bit_counter_down>=BIT_SYNC_GROUPS)||(bit_counter_up>=BIT_SYNC_GROUPS)){bit_counter_down=0;bit_counter_up=0;}		   //3
				TIM5CH1_CAPTURE_VAL=0;	
	 			TIM_SetCounter(TIM2,0);//������ֵ����
				TIM5CH1_CAPTURE_STA|=0X40;		//��ǲ�����������
	   			TIM_OC2PolarityConfig(TIM2,TIM_ICPolarity_Falling);		//CC1P=1 ����Ϊ�½��ز���
			}		    
		}//end of �����¼�			     	    					   
 	}
/****************************************�Ϳ���У��*******************************************/	
	else if((TIM5CH1_CAPTURE_STA&0X80)&&((TIM5CH1_CAPTURE_STA&0X10)==0))//λͬ��֮�󣬿�ʼ�жϰͿ����֡ͬ��
	{
	   if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)//λͬ��������ɺ�ĳ�ʱ����
	   {
		   buf_barker[11]=GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_1);//λͬ�����µ����bit
		   PBout(6)=!PBout(6);//����ʱ�̲���
		   for(i=0;i<11;i++)//�յ��������ݺ󣬽�bufѭ������һλ
		   {
		   		buf_barker[i]=buf_barker[i+1];
		   }
		   for(i=0;i<11;i++)
		   {
			   if(buf_barker[i]==barker[i]){barker_sum++;}
			   else barker_sum--;
		   }
		   barker_counter++;//��λ�Ƚ�һ��
//		   printf("\r\n�Ϳ���Ƚ�%d��!",barker_counter);
		   if(barker_sum>=9){
		   	   frame_index=0;
			   frame_lengths=ORIGEN_FRAMESIZE-20;//1100-20=24*45
			   TIM5CH1_CAPTURE_STA|=0X10;
//		   	   printf("\r\n֡ͬ��!");
		   }//֡�������㣬׼����������
		   else if(barker_counter>30){
			   frame_window_counter=0;
			   decoded_frame_index=0;
	
			   TIM2->ARR=0XFFFF;//0X0031;//����״̬��ʱ����ֵ
			   TIM_ITConfig(TIM2,TIM_IT_CC2,ENABLE);//TIM_IT_CC1,�����µ�֡�������벶��
			   TIM_OC2PolarityConfig(TIM2,TIM_ICPolarity_Rising); //CC1P=0 ����Ϊ�����ز���
	
			   TIM5CH1_CAPTURE_STA=0;
			   TIM_Cmd(TIM3, ENABLE);
			   printf("\r\n����������Ŷ!");
		   }//��λ30�λ�û��ͬ����������Ĵ�����λͬ�����õ��ı�������λͬ���ɹ�/ʧ���˳�ʱ���㣩
		   barker_sum=0;//��������֤������¿�ʼ
	   }
	}
/****************************************֡���ݽ���*******************************************/		
	else if((TIM5CH1_CAPTURE_STA&0X80)&&(TIM5CH1_CAPTURE_STA&0X10)&&((TIM5CH1_CAPTURE_STA&0X08)==0))//�Ϳ���У����ϣ�׼������֡������
	{
		if (TIM_GetITStatus(TIM2, TIM_IT_Update)!= RESET)//��ʼ���հͿ�����������
		{
			if(frame_index<frame_lengths)
			{					 
				receive_frame[frame_index]=GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_1); //��ǰʱ�̾���λͬ�����һ����Ԫ����ѳ���ʱ��
				PBout(6)=!PBout(6);//����ʱ�̲���

			//��ȡ֡�����趨֡������ʼ
			if(((TIM5CH1_CAPTURE_STA&0X0100)==0)&&(frame_index>24)){//�״ν���24����Ԫ�����������֡���͡�
				TIM5CH1_CAPTURE_STA|=0X0100;
				for(i=0;i<24;i++){
					gray_decode_buf[i]=	receive_frame[i];//�����ջ�������δ��������ݷŵ����崰�У�׼����������
				}
				decode_error_catch(gray_decode_buf,gray_decoded_buf);//��������,������Ϊgray_decoded_buf[]��ǰ12λ��ԪΪԭʼ���ݵĵ�������
				i=gray_decoded_buf[11]*2+gray_decoded_buf[10];
				printf("\r\n֡����i=%d\r\n",i);
				switch(i){
					case 0:
					frame_lengths=LENGTHS_SECUREFRAME;//���޸�
					break;//����֡
					case 1:
					tmp_buf=gray_decoded_buf[9]*2+gray_decoded_buf[8];
					if(tmp_buf==2){frame_lengths=LENGTHS_WAKEUPFRAME_BROADCAST;} //�㲥
					else if(tmp_buf==1){frame_lengths=LENGTHS_WAKEUPFRAME_UNICAST;}//����
					else if(tmp_buf==3){frame_lengths=LENGTHS_WAKEUPFRAME_MULTICAST;}//�鲥
					break;//����֡
					case 2:
					frame_lengths=LENGTHS_CONTROLFRAME;
					break;//����֡
					case 3:
					frame_lengths=LENGTHS_SECUREFRAME;
					break;//��֤֡
				}
				TIM5CH1_CAPTURE_STA|=0X0800;//֡ʵ�ʳ��Ȼ�ȡ���
			}	
			//��ȡ֡�����趨֡��������	

				frame_index++;
			}
			if(frame_index==frame_lengths) //�ȶ���һ��if��else�����¹���������һ���ж����ڣ�1ms
			{
				PBout(6)=0;
				TIM5CH1_CAPTURE_STA|=0X08;//֡����
				TIM2->ARR=0XFFFF;//0X0031;//����״̬��ʱ����ֵ
				TIM_ITConfig(TIM2,TIM_IT_CC2,ENABLE);//TIM_IT_CC1,�����µ�֡�������벶��
				TIM_OC2PolarityConfig(TIM2,TIM_ICPolarity_Rising); //CC1P=0 ����Ϊ�����ز���
				TIM_ITConfig(TIM2, TIM_IT_Update|TIM_IT_CC2, DISABLE); //���ж�
				TIM_Cmd(TIM2,DISABLE);//ԭʼ֡������ϣ��رն�ʱ����Լ��Դ���ȴ���֡�����������ʹ�ܽ�����һ֡
				
				printf("\r\n�㲥����G=%d\r\n",tmp_buf);
				printf("\r\n��������!\r\n");
				TIM_Cmd(TIM3, ENABLE);
			}
		}
	}

    TIM_ClearITPendingBit(TIM2, TIM_IT_CC2|TIM_IT_Update); //TIM_IT_CC1,����жϱ�־λ	  
}

/***********************��ʱ��4***************************/
void TIM4_Int_Init(u16 arr,u16 psc)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE); //ʱ��ʹ��
	
	//��ʱ��TIM3��ʼ��
	TIM_TimeBaseStructure.TIM_Period = arr; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ	
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure); //����ָ���Ĳ�����ʼ��TIMx��ʱ�������λ
 
	TIM_ITConfig(TIM4,TIM_IT_Update,ENABLE ); //ʹ��ָ����TIM3�ж�,��������ж�

	//�ж����ȼ�NVIC����
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;  //TIM3�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //��ռ���ȼ�0��
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //�����ȼ�3��
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQͨ����ʹ��
	NVIC_Init(&NVIC_InitStructure);  //��ʼ��NVIC�Ĵ���


	TIM_Cmd(TIM4, DISABLE);  //�ر�TIMx							 
}

extern u16 alarm_frame_index;//����������ȫ�ֱ���������timer.c��
//2 u8 alarm_over=0;//10�α����Ƿ񲥷���ϡ�0����ϣ�1��δ�ꣻ
u8 alarm_times=0;//�������Ŵ���
//��ʱ��4�жϷ������
void TIM4_IRQHandler(void)   //TIM4�ж�
{
	static u8 count=0;
   	if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)  //���TIM4�����жϷ������
		{
			TIM_ClearITPendingBit(TIM4, TIM_IT_Update);  //���TIMx�����жϱ�־
			if(alarm_times<10){//�յ�һ������ָ���ʮ�� 
			if(count==2){//����3�Σ�����һ��
				DKA065(alarm_frame_index);//���ű�������
				alarm_times++;
				count=0;				
			}else{
				count++;	
			}
			}else{ //�������
//1				alarm_over=0;
				alarm_times=0;
				alarm_frame_index=0;
				DKA_SWITCH=0;//���ز�������ģʽ
				TIM_Cmd(TIM4,DISABLE);//�رճ�ʱ�ж϶�ʱ��
			}
				
		}
}







