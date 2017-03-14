#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "usart.h"
#include "timer.h"
#include "gray.h"
#include "wkup.h"//���ѣ�˯��
#include "encrypt.h"//AES����
#include "stmflash.h"

#define RADIO_ID_START 34095233
#define RADIO_ID_END 1073741823
#define AREA_TERMINAL_ID_START 4353
#define AREA_TERMINAL_ID_END 262143


#define FLASH_SAVE_ADDR  0x08008000 //0X08070000 0X08000000//����FLASH �����ַ(����Ϊż��)
u8 TEXT_Buffer[4]={0};
#define SIZE sizeof(TEXT_Buffer)//���鳤��
u8 flash_temp[4]={0};
#define FLASH_STATE_ADDR  FLASH_SAVE_ADDR+10 //�������֡���������Ƿ��ǳ��γ�ʼ�����״γ�ʼ���󣬽�ֵ���Ϊ0x33����λʱ���ȼ���ֵ�Ƿ���0x30
u8 STATE_Buffer[2]={0};
#define STATE_SIZE sizeof(STATE_Buffer)//���鳤��
u8 STATE_temp[2]={0};

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
void DKA065(int index);
u16 alarm_frame_index=0;//����������ȫ�ֱ���������timer.c��

void communication_right(void);//���ѳɹ�
void communication_wrong(void);//�㲥����

u8 ecc_right=1;//eccͨ����־λ��>1��δͨ����0��ͨ����

u32 frame_counters=0;//����֡��bitsת֡������
u32 source_addres=0;//����֡��bitsתԴ��ַ
const u32 my_broad_ID=RADIO_ID_START;//��������̨ID��34095233~1073741823
u32 target_address_start=0;//����֡��bitsתĿ����ʼ��ַ
u32 target_address_end=0;//����֡��bitsתĿ����ֹ��ַ
const u32 my_ID=AREA_TERMINAL_ID_START;//�նˣɣı�š��նˣɣĶ���ʼ�ɣ�
													   
u8 flag_main_busy=0;//����������æ��ֹTIM3flash�жϱ�־λ
u8 sage_confirmd=0;//��ȫ��֤�Ƿ��յ���0��δ�յ���1�����յ���

//�նˣɣ�
//double terminal_ID=my_broad_ID<<18+my_ID;
int main(void)
{	
 	u16 index=0;
	u8 i=0,t=0;//forѭ��
	float sample_rate=0;
	u8 gray_decode_buf1[24]={0};//�������룬24����ԪΪһ��
	u8 gray_decoded_buf1[24]={0};//�������������24λ��Ԫ����ǰ12λΪԭʼ��Ϣ�ĵ�������

	u8 decoded_frame[DECODE_FRAMESIZE]={0};//������������ݵĻ�����
	u8 frame_type=0;//֡����
	u16 test=0,test1=0;		
	u16 len;

	u8 index_frame_send=0;//���ڻظ���Ϣ֡�±�
	u8 frame_send_buf[100]={0};//���ڻش�������

	u8 xor_sum=0;//���У���
		
	delay_init();	    	 //��ʱ������ʼ��	  
	NVIC_Configuration(); 	 //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	uart_init(256000);	 //���ڳ�ʼ��Ϊ9600
	uart2_init(9600);	 //����2��ʼ��Ϊ9600
 	LED_Init();			     //LED�˿ڳ�ʼ��	
//	WKUP_Init(); //�������ѳ�ʼ��
	LED1=0;
 	TIM2_Cap_Init(0XFFFF,72-1);	//��1Mhz��Ƶ�ʼ���,0xFFFFΪ65.535ms
	TIM3_Int_Init(9999,7199);//���ڴ洢֡����ֵ����ȫ��֤���ڲ�ѯ��1s�ж�һ�Σ�10sͳ��һ��
	TIM4_Int_Init(7999,7199); //DKAռ�����ȵ�ʱ��
	///////////////////��ȡFLASH��״̬��־λ���ж��Ƿ��״γ�ʼ��////////////////////////////////
	STMFLASH_Read(FLASH_STATE_ADDR,(u16*)STATE_temp,STATE_SIZE);
	if(STATE_temp[0]!=0x33){//�״γ�ʼ��
		TEXT_Buffer[0]=0;//�ȶ�flash֡��������ʼ��Ϊ0
		TEXT_Buffer[1]=0;
		TEXT_Buffer[2]=0;
		TEXT_Buffer[3]=0;
		STMFLASH_Write(FLASH_SAVE_ADDR,(u16*)TEXT_Buffer,SIZE);//��֡����ֵ���ĸ��ֽڱ��浽flash��
		frame_counters=0;
		STATE_temp[0]=0x33;
		STMFLASH_Write(FLASH_STATE_ADDR,(u16*)STATE_temp,STATE_SIZE);
	}else{
		STMFLASH_Read(FLASH_SAVE_ADDR,(u16*)flash_temp,SIZE);
		frame_counters=flash_temp[0]*256*256*256+flash_temp[1]*256*256+flash_temp[2]*256+flash_temp[3];//����ʱ����flash�е�֡������ֵ
		
	}
	frame_window_counter=0;
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
			TIM_Cmd(TIM3, ENABLE);
			printf("\r\nmain������!TIM5CH1_CAPTURE_STA=%d,frame_window_counter=%d,decoded_frame_index=%d\r\n",TIM5CH1_CAPTURE_STA,frame_window_counter,decoded_frame_index);
			test1=0;
		
		}


		if((TIM5CH1_CAPTURE_STA&0X0200)&&(TIM5CH1_CAPTURE_STA&0X08))//������������
		{
			flag_main_busy=1;//׼�����ڷ��ͽ�ֹTIM3�ж�
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
		   TIM2_Cap_Init(0XFFFF,72-1);
		   TIM_Cmd(TIM2,ENABLE);
		   
		flag_main_busy=0;   
		}//end of ������������֡����



/******************************************************************����2��������************************************************************************/
		if(USART2_RX_STA&0x8000)
		{
			flag_main_busy=1;//����������æ��ֹTIM3flash�жϱ�־λ			   
			len=USART2_RX_STA&0x3fff;//�õ��˴ν��յ������ݳ���

			if((USART2_RX_BUF[0]=='$')&&(len>0)){
				xor_sum=XOR(USART2_RX_BUF,len-1);
				if((xor_sum=='$')||(xor_sum==0x0d)){xor_sum++;}
				if(USART2_RX_BUF[len-1]==xor_sum){
 					/*******************************************��ȫоƬȷ��֡******************************************************************/
					if((USART2_RX_BUF[1]=='e')&&(USART2_RX_BUF[2]=='c')&&(USART2_RX_BUF[3]=='c')&&(USART2_RX_BUF[4]=='_')&&(USART2_RX_BUF[5]=='_')){//����֡
						LED0=1;//�رվ�ʾ��
						ecc_right=USART2_RX_BUF[9];//�����Ƿ�ͨ��У��
						if(ecc_right==0){//eccͨ��
							printf("ecc access!");
							sage_confirmd=1;//��֤ͨ��
							index_frame_send=0;
							frame_send_buf[index_frame_send]='$';
							index_frame_send++;
							frame_send_buf[index_frame_send]='s';
							index_frame_send++;
							frame_send_buf[index_frame_send]='t';
							index_frame_send++;
							frame_send_buf[index_frame_send]='m';
							index_frame_send++;
							frame_send_buf[index_frame_send]='_';
							index_frame_send++;
							frame_send_buf[index_frame_send]=2;//2����֡��֪ͨMSP430�ǶԳ���֤�Ƿ�ͨ����֡;1:����ͨ��Ƶ��֡;
							index_frame_send++;
							frame_send_buf[index_frame_send]=1;//1����ʾͨ��
							index_frame_send++;
							xor_sum=XOR(frame_send_buf,index_frame_send);
							if((xor_sum==0x0D)||(xor_sum=='$'))xor_sum++;
							frame_send_buf[index_frame_send]=xor_sum;
							index_frame_send++;
							frame_send_buf[index_frame_send]='\r';
							index_frame_send++;
							frame_send_buf[index_frame_send]='\n';
							index_frame_send++;
						
							for(t=0;t<index_frame_send;t++)
							{
								USART_SendData(USART1, frame_send_buf[t]);//�򴮿ڷ�������
								while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);//�ȴ����ͽ���
							}
						}else{//eccδͨ��
							printf("ecc wrong!");
							sage_confirmd=0;//��֤ʧ��
							index_frame_send=0;
							frame_send_buf[index_frame_send]='$';
							index_frame_send++;
							frame_send_buf[index_frame_send]='s';
							index_frame_send++;
							frame_send_buf[index_frame_send]='t';
							index_frame_send++;
							frame_send_buf[index_frame_send]='m';
							index_frame_send++;
							frame_send_buf[index_frame_send]='_';
							index_frame_send++;
							frame_send_buf[index_frame_send]=2;//2����֡��֪ͨMSP430�ǶԳ���֤�Ƿ�ͨ����֡;1:����ͨ��Ƶ��֡;
							index_frame_send++;
							frame_send_buf[index_frame_send]=0;//1����ʾͨ��,0:δͨ��;
							index_frame_send++;
							xor_sum=XOR(frame_send_buf,index_frame_send);
							if((xor_sum==0x0D)||(xor_sum=='$'))xor_sum++;
							frame_send_buf[index_frame_send]=xor_sum;
							index_frame_send++;
							frame_send_buf[index_frame_send]='\r';
							index_frame_send++;
							frame_send_buf[index_frame_send]='\n';
							index_frame_send++;
						
							for(t=0;t<index_frame_send;t++)
							{
								USART_SendData(USART1, frame_send_buf[t]);//�򴮿ڷ�������
								while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);//�ȴ����ͽ���
							}
						}
						USART2_RX_STA=0;//������ϣ����������һ֡
						USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//���ж�
					}
 					/*******************************************��ȫоƬ�����ش�*******************************************************************/
				 	else if((USART2_RX_BUF[1]=='r')&&(USART2_RX_BUF[2]=='t')&&(USART2_RX_BUF[3]=='s')&&(USART2_RX_BUF[4]=='_')&&(USART2_RX_BUF[5]=='_')){//�ش�֡
						LED0=1;//�رվ�ʾ��,1���ش�������Ҫ���ܴ�������
						frame_secure(decoded_frame_index,decoded_frame);//��֤֡������
						USART2_RX_STA=0;//������ϣ����������һ֡
						USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//���ж�

					}else {USART2_RX_STA=0;USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);}//֡���ͳ���,�����ش�
				 }else{USART2_RX_STA=0;USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);}//end of XOR��XOR���������ش�
				 }else {USART2_RX_STA=0;USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);}//end of check '$'

		flag_main_busy=0;
		}else {USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);}//�˴��ȴ�����2�ش����ݣ���usart2_works�������㣬��Ҫ��Ӷ���Ϊ0�������ж�



	}//end of while
}


void frame_continues(void){//����֡������

}

void frame_wakeup_broadcast(u16 decoded_frame_index,u8 decoded_frame[DECODE_FRAMESIZE]){//�㲥����֡������
	
	u8 i=0,t=0;
	u16 index=0;
	u8 aes_bits[128]={0};//AES�������ݱ�����
	u8 aes_char[4][4]={0};//AES	128bitsת��λ4*4��u8����
	double fre_point=0;//��ӡ����,����֡����ȡ����ͨ��Ƶ��
	u8 communication_point=0;//����֡����ȡ����ͨ��Ƶ��
	u8 xorsum=0;
	u8 index_frame_send=0;//���ڻظ���Ϣ֡�±�
	u8 frame_send_buf[100]={0};//���ڻش�������
	u32 frame_nums=0;//��֡����ȡ��֡������ֵ�����ڱȽϵ���֡�Ƿ������µ�


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
//			 TIM_Cmd(TIM2,ENABLE);
			 printf("\r\nWakeup AES wrong!!\r\n"); 
			 SAFE_CHIP=0;//�رհ�ȫоƬ
			 return;//��⵽��ͬ��������ֹ��֤
		  }
		  i++;
	}
	if(i==36){TIM5CH1_CAPTURE_STA|=0X0400;}//AES��֤ͨ��
	SAFE_CHIP=1;//�򿪰�ȫоƬ
/*************************************Ŀ���ַ��ȡ************************************************************/
	if(TIM5CH1_CAPTURE_STA&0X0400){//AES���ͨ��
		communication_point=decoded_frame[4]*128+decoded_frame[5]*64+decoded_frame[6]*32+decoded_frame[7]*16+decoded_frame[8]*8+decoded_frame[9]*4+decoded_frame[10]*2+decoded_frame[11];
		index_frame_send=0;
		frame_send_buf[index_frame_send]='$';
		index_frame_send++;
		frame_send_buf[index_frame_send]='s';
		index_frame_send++;
		frame_send_buf[index_frame_send]='t';
		index_frame_send++;
		frame_send_buf[index_frame_send]='m';
		index_frame_send++;
		frame_send_buf[index_frame_send]='_';
		index_frame_send++;
		frame_send_buf[index_frame_send]=1;//��ʾ��֡��֪ͨMSP430Ƶ�������,��ΪSTM32��֪��ͨ��Ƶ���Ƕ���.1:�㲥��2��������3���鲥��
		index_frame_send++;
		frame_send_buf[index_frame_send]=communication_point+0x30;//ͨ��Ƶ��ֵ���ܳ���0x0d,0x24
		index_frame_send++;
		xorsum=XOR(frame_send_buf,index_frame_send);
		if((xorsum=='$')||(xorsum==0x0d))xorsum++;
		frame_send_buf[index_frame_send]=xorsum;
		index_frame_send++;
		frame_send_buf[index_frame_send]='\r';
		index_frame_send++;
		frame_send_buf[index_frame_send]='\n';
		index_frame_send++;
	
		for(t=0;t<index_frame_send;t++)
		{
			USART_SendData(USART1, frame_send_buf[t]);//�򴮿ڷ�������
			while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);//�ȴ����ͽ���
		}

		fre_point=76+(double)communication_point/10.0;
		printf("comunation point:%fMHz\r\n",fre_point);//��ӡͨ��Ƶ���ֵ

		frame_nums=0;
		for(index=0;index<32;index++){
			frame_nums|=decoded_frame[decoded_frame_index-72+index]<<(31-index);
		}
		printf("frame_nums:%d\r\n",frame_nums);//��ӡ֡������ֵ

		if(frame_nums<=frame_counters){
			printf("frame is old.");
			return;
		}else{
			frame_counters=frame_nums;
		}
		source_addres=0;
		for(index=0;index<30;index++){
			source_addres|=decoded_frame[12+index]<<(29-index);//Դ��ַ������֡�дӵ�12bits��ʼ����30bits
		}

		if(source_addres==my_broad_ID){//�жϵ�̨ID�Ƿ�Ϸ�
			printf("redio ID correct.\r\n");
		//	communication_right();
		}else{
			printf("redio ID wrong.\r\n");
			communication_wrong();
		}
		printf("source_addres:%d\r\n",source_addres);//��ӡ��̨ID


	//	communication_right(); //���ѳɹ�

		if((decoded_frame[2]*2+decoded_frame[3])==2){//�㲥����֡����
			communication_right();
			printf("terminal ID correct. broadcast wakeup AES SUCCESS\r\n\r\n");
		} 
		else if((decoded_frame[2]*2+decoded_frame[3])==1){//��������֡����
			printf("unicast wakeup AES SUCCESS\r\n\r\n");

			target_address_start=0;
			for(index=0;index<24;index++){
				target_address_start|=decoded_frame[48+index]<<(23-index);//������ַ������֡�дӵ�48bits��ʼ����24bits
			}
			if(target_address_start==my_ID){
				communication_right();
				printf("redio ID correct.\r\n");
			}else{
				printf("redio ID wrong.\r\n");
				communication_wrong();
			}
			printf("target_address_start:%d\r\n",target_address_start);//��ӡ������ַ

		}else if((decoded_frame[2]*2+decoded_frame[3])==3){//�鲥����֡����
			printf("multicast wakeup AES SUCCESS\r\n\r\n");

			target_address_start=0;
			for(index=0;index<24;index++){
				target_address_start|=decoded_frame[48+index]<<(23-index);//�鲥��ʼ��ַ������֡�дӵ�48bits��ʼ����24bits
			}
			printf("target_address_start:%d\r\n",target_address_start);//��ӡ������ַ

			target_address_end=0;
			for(index=0;index<24;index++){
				target_address_end|=decoded_frame[72+index]<<(23-index);//�鲥��ֹ��ַ������֡�дӵ�72bits��ʼ����24bits
			}

			if((my_ID>=target_address_start)&&(my_ID<=target_address_end)){				
				communication_right();
				printf("redio ID correct.\r\n");
			}else{
				printf("redio ID wrong.\r\n");
				communication_wrong();
			}

			printf("target_address_end:%d\r\n",target_address_end);//��ӡ������ַ

		}else printf("\r\nfinal wrong!!\r\n");   
		   
		   
	}
	return;
}

//void frame_wakeup_unicast(u16 decoded_frame_index,u8 decoded_frame[DECODE_FRAMESIZE]){//�����鲥����֡������
//	
//}

//6 extern u8 alarm_over;//10�α����Ƿ񲥷���ϡ�0����ϣ�1��δ�ꣻ
extern u8 alarm_times;//�������Ŵ���
void frame_control(u16 decoded_frame_index,u8 decoded_frame[DECODE_FRAMESIZE]){//����֡������
	u8 i=0;
	u16 index=0;
	u8 aes_bits[128]={0};//AES�������ݱ�����
	u8 aes_char[4][4]={0};//AES	128bitsת��λ4*4��u8����
	u32 frame_nums=0;//���ر����֡������ֵ�����ڱȽϵ���֡�Ƿ������µ�
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
			 communication_wrong();
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
		communication_right();//Ҳ�㻽�ѳɹ�
		if(index<99){
//4			if(alarm_over==0){ //��ֹ��������֡��Ӱ�졣�յ�һ�Σ��Ͳ������������
//3				alarm_over=1;
				alarm_times=0;//7
				if(alarm_frame_index!=index){//ͬһ�����ظ����͵ı���֡����һ���Ժ�ı�����
					alarm_frame_index=index;
					DKA_SWITCH=1;//����ѡ��DKA��
					DKA065(index);//��������Ƶ
			//		delay_ms(1300);
					TIM4->ARR=0X1F3F;
			//		TIM4_Int_Init(7999,7199); //DKAռ�����ȵ�ʱ��
					TIM_Cmd(TIM4, ENABLE);  //��TIMx //��DKAоƬ��������ʱ��
				}
//5			}
		}
		else if (index==100){
			communication_wrong();//ͨ�Ž���
			alarm_frame_index=0;	
		}
		

		frame_nums=0;
		for(index=0;index<32;index++){
			frame_nums|=decoded_frame[decoded_frame_index-72+index]<<(31-index);
		}
		printf("frame_nums:%d\r\n",frame_nums);//��ӡ֡������ֵ

		if(frame_nums<=frame_counters){
			printf("frame is old.");
			return;
		}else{
			frame_counters=frame_nums;
		}
		


//		switch(index){
//			case 0:
//
//			break;
//			case 1:
//
//			break;
//			case 2:
//
//			break;
//			case 3:
//
//			break;
//			case 4:
//
//			break;
//			case 5:
//
//			break;
//			case 6:
//
//			break;
//			case 7:
//
//			break;
//			case 8:
//
//			break;
//			case 9:
//
//			break;
//		
//		}
		
	} 
}

u8 index_safe_times=0;//��֤֡���ʹ���������
void frame_secure(u16 decoded_frame_index,u8 decoded_frame[DECODE_FRAMESIZE]){//��֤֡������,�����������468bits��ÿ4bitsת1byte����117�ֽ�
   u16 t=0;
   u8 index_frame_safe=0;//�ն���ȫоƬ���������֡
   u8 frame_safe[130]={0};//����֡buffer
   u8 xorsum=0;//������ʹ�õ��������ֵ
   LED0=0;//�򿪾�ʾ��
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
		if((index_safe_times=='$')||(index_safe_times==0x0d))index_safe_times++;
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
		frame_safe[index_frame_safe]=decoded_frame[t*4]*8+decoded_frame[t*4+1]*4+decoded_frame[t*4+2]*2+decoded_frame[t*4+3]+0x30;//ASCII 0���Ӧʮ������0x30
		index_frame_safe++;
	}

//	for(t=0;t<fm_frame_index_byte;t++){
//		frame_safe[index_frame_safe]=fm_frame_byte[t]+0x30;
//		index_frame_safe++;
//	}
	xorsum=XOR(frame_safe,index_frame_safe);
	if((xorsum=='$')||(xorsum==0x0d))xorsum++;
	frame_safe[index_frame_safe]=xorsum;
	index_frame_safe++;
	frame_safe[index_frame_safe]='\r';
	index_frame_safe++;
	frame_safe[index_frame_safe]='\n';
	index_frame_safe++;
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//��ʱ�򿪣�ֻΪ����
	printf("\r\nsecure frame:\r\n");
	for(t=0;t<(index_frame_safe-2);t++)//��֤֡����ͨ����1��ӡ,�ų���\r\n
	{
		printf("%x ",frame_safe[t]);
	}
	for(t=0;t<index_frame_safe;t++)//��֤֡ͨ������2���͸���ȫоƬ
	{
		USART_SendData(USART2, frame_safe[t]);//�򴮿ڷ�������
		while(USART_GetFlagStatus(USART2,USART_FLAG_TC)!=SET);//�ȴ����ͽ���
	}
	printf("\r\n");
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

void DKA065(int index){
	int i=0;
	DKA_RTS=1;
	DKA_DATA=0; /* �ȸ�λ*/
	delay_us(40); /* 100us */
	DKA_RTS=0;
	delay_ms(5); /* 5ms ����*/
	for(i=0;i<index;i++)
	{
		DKA_DATA=1; //��������
		delay_us(40); //�ȴ�100us
		DKA_DATA=0; //��������
		delay_us(40); //�ȴ�100us�����һ�����巢��
	}
}


void communication_right(void){
	GPIO_SetBits(GPIOB,GPIO_Pin_10);//1	ͨ�ųɹ���֪ͨ430
	GPIO_ResetBits(GPIOB,GPIO_Pin_11);//0
}

void communication_wrong(void){
	GPIO_ResetBits(GPIOB,GPIO_Pin_10);//0	ͨ�Ž�����֪ͨ430
	GPIO_SetBits(GPIOB,GPIO_Pin_11);//1
}

