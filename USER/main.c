#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "usart.h"
#include "timer.h"
#include "gray.h"
#include "wkup.h"//���ѣ�˯��
#include "encrypt.h"//AES����

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

extern unsigned char cipherkey_radio[16];//��С����̨��˽Կ 

u8 frame_window_counter=0;	//��ʾ������Ļ�������ͳ��������������24λλһ������
u16 decoded_frame_index=0;//����������󻺳������������ֵ
void frame_continues(void);//����֡������
void frame_wakeup_broadcast(u16 decoded_frame_index,u8 decoded_frame[DECODE_FRAMESIZE]);//�㲥����֡������
//void frame_wakeup_unicast(u16 decoded_frame_index,u8 decoded_frame[DECODE_FRAMESIZE]);//�����鲥����֡������
void frame_control(u16 decoded_frame_index,u8 decoded_frame[DECODE_FRAMESIZE]);//����֡������
void frame_secure(u16 decoded_frame_index,u8 decoded_frame[DECODE_FRAMESIZE]);//��֤֡������
unsigned char XOR(unsigned char *BUFF, u16 len);


int main(void)
{	
 	u16 index=0;
	u8 i=0;//forѭ��
	float sample_rate=0;
	u8 gray_decode_buf1[24]={0};//�������룬24����ԪΪһ��
	u8 gray_decoded_buf1[24]={0};//�������������24λ��Ԫ����ǰ12λΪԭʼ��Ϣ�ĵ�������

	u8 decoded_frame[DECODE_FRAMESIZE]={0};//������������ݵĻ�����
	u8 frame_type=0;//֡����
	u16 test=0,test1=0;
		
	delay_init();	    	 //��ʱ������ʼ��	  
	NVIC_Configuration(); 	 //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	uart_init(256000);	 //���ڳ�ʼ��Ϊ9600
	uart2_init(9600);	 //����2��ʼ��Ϊ9600
 	LED_Init();			     //LED�˿ڳ�ʼ��	
//	WKUP_Init(); //�������ѳ�ʼ��
	LED1=0;
 	TIM5_Cap_Init(0XFFFF,72-1);	//��1Mhz��Ƶ�ʼ���,0xFFFFΪ65.535ms
	TIM3_Int_Init(9999,7199);//5hz�ļ���Ƶ�� 
   	while(1)
	{	  		 		 
//�������ŵ��е�����һ֡�󣬶���������룬��ԭ��ԭʼ���ݣ�����decoded_frame[]��
		if(((TIM5CH1_CAPTURE_STA&0X200)==0)&&(TIM5CH1_CAPTURE_STA&0X0800)&&(((frame_index+1)/24)>frame_window_counter)){ //δ�������ݴ���һ����
		
		for(i=0;i<24;i++){
			gray_decode_buf1[i]=receive_frame[i+frame_window_counter*24];//�����ջ�������δ��������ݷŵ����崰�У�׼����������
		}
		decode_error_catch(gray_decode_buf1,gray_decoded_buf1);//��������,������Ϊgray_decoded_buf[]��ǰ12λ��ԪΪԭʼ���ݵĵ�������
		for(i=0;i<12;i++){
			decoded_frame[decoded_frame_index]=gray_decoded_buf1[11-i];//������������ݵĻ�����
			decoded_frame_index++;//����������󻺳������������ֵ
		}
		frame_window_counter++;//���봰����һ��
		if(decoded_frame_index*2==frame_lengths){
			TIM5CH1_CAPTURE_STA|=0X0200;
			frame_window_counter=0;
		}
//printf("\r\n���ף�TIM5CH1_CAPTURE_STA=%d,frame_window_counter=%d,decoded_frame_index=%d,frame_lengths=%d\r\n",TIM5CH1_CAPTURE_STA,frame_window_counter,decoded_frame_index,frame_lengths);
		}

		if(test<65533)test++;
		else if(test>=65533){test1++;test=0;}
		if(test1==90){
		printf("\r\nmain������!TIM5CH1_CAPTURE_STA=%d,frame_window_counter=%d,decoded_frame_index=%d\r\n",TIM5CH1_CAPTURE_STA,frame_window_counter,decoded_frame_index);
		test1=0;
		}


		if((TIM5CH1_CAPTURE_STA&0X0200)&&(TIM5CH1_CAPTURE_STA&0X08))//������������
		{
/**************************************��ӡ��������*********************************************************/			
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
/**************************************����������*********************************************************/
		   printf("\r\n����֡��\r\n");

		   for(index=0;index<decoded_frame_index;index++)//���׽���������֡
		   {
		 	  printf("%d",decoded_frame[index]);
		   }

		   	frame_type=decoded_frame[0]*2+decoded_frame[1];
			switch(frame_type){
				case 0:
					frame_continues();//����֡������
					break;//����֡
				case 1:
					frame_wakeup_broadcast(decoded_frame_index,decoded_frame);//�㲥����֡������(�����鲥�㲥һ������)
					break;//����֡
				case 2:
					frame_control(decoded_frame_index,decoded_frame);//����֡������
					break;//����֡
				case 3:
					frame_secure(decoded_frame_index,decoded_frame);//��֤֡������
					break;//��֤֡
			}

/**************************************��֡������ϣ�������һ֡************************************************/
		   TIM5CH1_CAPTURE_STA=0;//֡������ϵ�����
		   decoded_frame_index=0; 
		   frame_window_counter=0;
		   TIM5_Cap_Init(0XFFFF,72-1);
		   TIM_Cmd(TIM5,ENABLE);
		   
		   
		}//end of ������������֡����







	}//end of while
}


void frame_continues(void){//����֡������

}

void frame_wakeup_broadcast(u16 decoded_frame_index,u8 decoded_frame[DECODE_FRAMESIZE]){//�㲥����֡������
	
	u8 i=0;
	u16 index=0;
	u8 aes_bits[128]={0};//AES�������ݱ�����
	u8 aes_char[4][4]={0};//AES	128bitsת��λ4*4��u8����
	/**************************************AESУ��**************************************************************/
	for(i=0;i<128;i++){
	   if(i<(decoded_frame_index-36)){//�Ѹ��������������к�36λǰ�����ݷŵ�aes_bits[]��
		  aes_bits[i]=decoded_frame[i];
	  }else{
 	  	  aes_bits[i]=0;//����128λ���㣬����128����˴���ִ�У���ȡǰ128λ
	  }
	}
	printf("\r\nAES string:\r\n");
	for(index=0;index<128;index++)//���յ����������ݴ�������AES
	{
		  printf("%d",aes_bits[index]);
	}
	bit_char(aes_bits,aes_char);//������ת4*4u8����
	Encrypt(aes_char,cipherkey_radio);
	char_bit(aes_char,aes_bits);
	printf("\r\nAES native string:\r\n");
	for(index=0;index<128;index++)//AES����У�鴮
	{
	     printf("%d",aes_bits[index]);
	}
	printf("\r\n");
	i=0;
	for(index=(decoded_frame_index-36);index<decoded_frame_index;index++){//���ؼ����AES�봫�͵�AES���бȶ�
	    if(decoded_frame[index]!=aes_bits[i]){//�����ĳһλ��ͬ
//			 TIM5CH1_CAPTURE_STA=0;//���ؼ��Ĵ������㣬�ȴ�����֡
//			 decoded_frame_index=0; 
//			 frame_window_counter=0;
//			 TIM_Cmd(TIM5,ENABLE);
			 printf("\r\nWakeup AES wrong!!\r\n"); 
			 return;//��⵽��ͬ��������ֹ��֤
		  }
		  i++;
	}
	if(i==36){TIM5CH1_CAPTURE_STA|=0X0400;}//AES��֤ͨ��
/*************************************Ŀ���ַ��ȡ************************************************************/
	if(TIM5CH1_CAPTURE_STA&0X0400){//AES���ͨ��
		if((decoded_frame[2]*2+decoded_frame[3])==2){//�㲥����֡����
			printf("broadcast wakeup AES SUCCESS\r\n\r\n");
		} 
		else if((decoded_frame[2]*2+decoded_frame[3])==1){//��������֡����
			printf("unicast wakeup AES SUCCESS\r\n\r\n");

		}else if((decoded_frame[2]*2+decoded_frame[3])==3){//�鲥����֡����
			printf("multicast wakeup AES SUCCESS\r\n\r\n");

		}else printf("\r\nfinal wrong!!\r\n");   
		   
		   
	}
	return;
}

//void frame_wakeup_unicast(u16 decoded_frame_index,u8 decoded_frame[DECODE_FRAMESIZE]){//�����鲥����֡������
//	
//}

void frame_control(u16 decoded_frame_index,u8 decoded_frame[DECODE_FRAMESIZE]){//����֡������
	u8 i=0;
	u16 index=0;
	u8 aes_bits[128]={0};//AES�������ݱ�����
	u8 aes_char[4][4]={0};//AES	128bitsת��λ4*4��u8����
	/**************************************AESУ��**************************************************************/
	for(i=0;i<128;i++){
	   if(i<(decoded_frame_index-36)){//�Ѹ��������������к�36λǰ�����ݷŵ�aes_bits[]��
		  aes_bits[i]=decoded_frame[i];
	  }else{
	  	  aes_bits[i]=0;//����128λ���㣬����128����˴���ִ�У���ȡǰ128λ
	  }
	}
	printf("\r\nAES string:\r\n");
	for(index=0;index<128;index++)//���յ����������ݴ�������AES
	{
		  printf("%d",aes_bits[index]);
	}
	bit_char(aes_bits,aes_char);//������ת4*4u8����
	Encrypt(aes_char,cipherkey_radio);
	char_bit(aes_char,aes_bits);
	printf("\r\nAES native string:\r\n");
	for(index=0;index<128;index++)//AES����У�鴮
	{
	     printf("%d",aes_bits[index]);
	}
	printf("\r\n");
	i=0;
	for(index=(decoded_frame_index-36);index<decoded_frame_index;index++){//���ؼ����AES�봫�͵�AES���бȶ�
	    if(decoded_frame[index]!=aes_bits[i]){//�����ĳһλ��ͬ
			 printf("\r\nControl AES wrong!!\r\n"); 
			 return;//��⵽��ͬ��������ֹ��֤
		  }
		  i++;
	}
	if(i==36){TIM5CH1_CAPTURE_STA|=0X0400;}//AES��֤ͨ��
/*************************************���ƴ�����ȡ************************************************************/
	index=0;//���Ʊ�ŵ�����ֵ
	if(TIM5CH1_CAPTURE_STA&0X0400){//AES���ͨ��
	 	for(i=2;i<12;i++){
			index|=decoded_frame[i]<<(11-i);
		}
		printf("\r\nwaring index=%d\r\n",index);
		switch(index){
			case 0:

			break;
			case 1:

			break;
			case 2:

			break;
			case 3:

			break;
			case 4:

			break;
			case 5:

			break;
			case 6:

			break;
			case 7:

			break;
			case 8:

			break;
			case 9:

			break;
		
		}
		
	} 
}

u8 index_safe_times=0;//��֤֡���ʹ���������
void frame_secure(u16 decoded_frame_index,u8 decoded_frame[DECODE_FRAMESIZE]){//��֤֡������,�����������468bits��ÿ4bitsת1byte����117�ֽ�
   u16 t=0;
   u8 index_frame_safe=0;//�ն���ȫоƬ���������֡
   u8 frame_safe[130]={0};//����֡buffer
	frame_safe[index_frame_safe]='$';
	index_frame_safe++;
	frame_safe[index_frame_safe]='d';
	index_frame_safe++;
	frame_safe[index_frame_safe]='a';
	index_frame_safe++;
	frame_safe[index_frame_safe]='t';
	index_frame_safe++;
	frame_safe[index_frame_safe]='_';
	index_frame_safe++;
	if (index_safe_times<200)
	{
		index_safe_times++;
	} 
	else
	{
		index_safe_times=0;
	}
	frame_safe[index_frame_safe]=index_safe_times;
	index_frame_safe++;
	frame_safe[index_frame_safe]=(decoded_frame_index/4)/256;
	index_frame_safe++;
	frame_safe[index_frame_safe]=(decoded_frame_index/4)%256;
	index_frame_safe++;

	for (t=0;t<decoded_frame_index/4;t++)
	{
		frame_safe[index_frame_safe]=decoded_frame[t*4]*8+decoded_frame[t*4+1]*4+decoded_frame[t*4+2]*2+decoded_frame[t*4+3]*1+0x30;//ASCII 0���Ӧʮ������0x30
		index_frame_safe++;
	}

//	for(t=0;t<fm_frame_index_byte;t++){
//		frame_safe[index_frame_safe]=fm_frame_byte[t]+0x30;
//		index_frame_safe++;
//	}
	frame_safe[index_frame_safe]=XOR(frame_safe,index_frame_safe);
	index_frame_safe++;
	frame_safe[index_frame_safe]='\r';
	index_frame_safe++;
	frame_safe[index_frame_safe]='\n';
	index_frame_safe++;
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//��ʱ�򿪣�ֻΪ����
	for(t=0;t<index_frame_safe;t++)//��֤֡ͨ������2���͸���ȫоƬ
	{
		USART_SendData(USART2, frame_safe[t]);//�򴮿ڷ�������
		while(USART_GetFlagStatus(USART2,USART_FLAG_TC)!=SET);//�ȴ����ͽ���
	}
}


unsigned char XOR(unsigned char *BUFF, u16 len)
{
	unsigned char result=0;
	u16 i;
	for(result=BUFF[0],i=1;i<len;i++)
	{
		result ^= BUFF[i];
	}
	return result;
}



