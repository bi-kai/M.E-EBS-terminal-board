#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "usart.h"
#include "timer.h"
//ALIENTEKս��STM32������ʵ��10
//���벶��ʵ��  
//����֧�֣�www.openedv.com
//������������ӿƼ����޹�˾ 
extern u16  TIM5CH1_CAPTURE_STA;		//���벶��״̬		    				
extern u16	TIM5CH1_CAPTURE_VAL;	//���벶��ֵ
extern u16	TIM5CH1_DOWN_CAPTURE_VAL;	//����������ʱ�䲶��ֵ

extern u16 bit_SYCN_UP[4];
extern u16 bit_SYCN_DOWN[4];

extern u8 receive_frame[100];
	
 int main(void)
 {	
    u8 index=0;
	float sample_rate=0;	
	delay_init();	    	 //��ʱ������ʼ��	  
	NVIC_Configuration(); 	 //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	uart_init(256000);	 //���ڳ�ʼ��Ϊ9600
 	LED_Init();			     //LED�˿ڳ�ʼ��
 
// 	TIM3_PWM_Init(899,0); 		//����Ƶ��PWMƵ��=72000/(899+1)=80Khz
 	TIM5_Cap_Init(0X00FF,72-1);	//��1Mhz��Ƶ�ʼ��� 
   	while(1)
	{
// 		delay_ms(10);
//		TIM_SetCompare2(TIM3,TIM_GetCapture2(TIM3)+1);	
//		if(TIM_GetCapture2(TIM3)==300)TIM_SetCompare2(TIM3,0);
			
		 		 
// 		if(TIM5CH1_CAPTURE_STA&0X80)//�ɹ�������һ��������
//		{
//		  if(j<8){	 
//			temp=TIM5CH1_CAPTURE_STA&0X3F;
//			temp*=65536;//���ʱ���ܺ�
//			temp+=TIM5CH1_CAPTURE_VAL;//�õ��ܵĸߵ�ƽʱ��
//			i[j]=temp;
//			j++;
//			TIM5CH1_CAPTURE_STA=0;//������һ�β���
//			}
//		}else if(j>3&&j<8){
//		for(index=0;index<8;index++)
//		{
//			printf("%d\r\n",i[index]);//��ӡ�ܵĸߵ�ƽʱ��
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

