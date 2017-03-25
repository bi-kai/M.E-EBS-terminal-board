#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "usart.h"
#include "timer.h"
#include "gray.h"
#include "wkup.h"
extern u16  TIM5CH1_CAPTURE_STA;		//���벶��״̬		    				
extern u16	TIM5CH1_CAPTURE_VAL;	//���벶��ֵ
extern u16	TIM5CH1_DOWN_CAPTURE_VAL;	//����������ʱ�䲶��ֵ

extern u16 bit_SYCN_UP[BIT_SYNC_GROUPS];
extern u16 bit_SYCN_DOWN[BIT_SYNC_GROUPS];

extern u8 receive_frame[ORIGEN_FRAMESIZE];//����֡�Ļ�����
extern u16 frame_index;//����֡�У�����λ��������洢������ֵ



extern signed char barker_sum;
extern u8 buf_barker[12];	
extern u16 frame_lengths;//��ǰ֡�ܳ���

u8 decoded_frame[DECODE_FRAMESIZE]={0};//������������ݵĻ�����
u16 decoded_frame_index=0;//����������󻺳������������ֵ
u8 frame_window_counter=0;	//��ʾ������Ļ�������ͳ��������������24λλһ������
extern u16 frame_lengths;//��ǰ֡�ܳ���
int main(void)
{	
 	u16 index=0;
	u8 i=0;//forѭ��
	float sample_rate=0;
	u8 gray_decode_buf1[24]={0};//�������룬24����ԪΪһ��
	u8 gray_decoded_buf1[24]={0};//�������������24λ��Ԫ����ǰ12λΪԭʼ��Ϣ�ĵ�������
		
	delay_init();	    	 //��ʱ������ʼ��	  
	NVIC_Configuration(); 	 //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	uart_init(256000);	 //���ڳ�ʼ��Ϊ9600
 	LED_Init();			     //LED�˿ڳ�ʼ��	
	WKUP_Init(); //�������ѳ�ʼ��
	LED1=0;
 	TIM5_Cap_Init(0XFFFF,72-1);	//��1Mhz��Ƶ�ʼ���,0xFFFFΪ65.535ms
	TIM3_Int_Init(1999,7199);//1Khz�ļ���Ƶ�� 
   	while(1)
	{	  		 		 
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
		   printf("\r\nԭʼ���ݣ�");
		   for(index=0;index<frame_index;index++)//���յ�ԭʼ֡����
		   {
		 	  printf("%d",receive_frame[index]);
		   }

		   printf("\r\n����֡��");

			   for(index=0;index<decoded_frame_index;index++)//���׽���������֡
			   {
			 	  printf("%d",decoded_frame[index]);
			   }
			   printf("\r\n");
			   TIM5CH1_CAPTURE_STA=0;//֡������ϵ�����
			   decoded_frame_index=0; 
			   frame_window_counter=0;
			   TIM_Cmd(TIM5,ENABLE);
		   
		   
		}



	   	//�������ŵ��е�����һ֡�󣬶���������룬��ԭ��ԭʼ���ݣ�����decoded_frame[]��
		if((TIM5CH1_CAPTURE_STA&0X10)&&(((frame_index+1)/24)>frame_window_counter)){ //δ�������ݴ���һ����
		for(i=0;i<24;i++){
			gray_decode_buf1[i]=receive_frame[i+frame_window_counter*24];//�����ջ�������δ��������ݷŵ����崰�У�׼����������
		}
		decode_error_catch(gray_decode_buf1,gray_decoded_buf1);//��������,������Ϊgray_decoded_buf[]��ǰ12λ��ԪΪԭʼ���ݵĵ�������
		for(i=0;i<12;i++){
			decoded_frame[decoded_frame_index]=gray_decoded_buf1[11-i];//������������ݵĻ�����
			decoded_frame_index++;//����������󻺳������������ֵ
		}
		if(decoded_frame_index*2==frame_lengths){
			TIM5CH1_CAPTURE_STA|=0X0200;
		}
		frame_window_counter++;//���봰����һ��
		}



	}//end of while
}

