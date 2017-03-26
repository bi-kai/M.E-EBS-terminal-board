#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "usart.h"
#include "timer.h"
#include "gray.h"
#include "wkup.h"//唤醒，睡眠
#include "encrypt.h"//AES加密
#include "stmflash.h"

#define RADIO_ID_START 34095233
#define RADIO_ID_END 1073741823
#define AREA_TERMINAL_ID_START 4353
#define AREA_TERMINAL_ID_END 262143


#define FLASH_SAVE_ADDR  0x08008000 //0X08070000 0X08000000//设置FLASH 保存地址(必须为偶数)
u8 TEXT_Buffer[4]={0};
#define SIZE sizeof(TEXT_Buffer)//数组长度
u8 flash_temp[4]={0};
#define FLASH_STATE_ADDR  FLASH_SAVE_ADDR+10 //用来标记帧计数数组是否是初次初始化，首次初始化后，将值标记为0x33，复位时首先检测该值是否是0x30
u8 STATE_Buffer[2]={0};
#define STATE_SIZE sizeof(STATE_Buffer)//数组长度
u8 STATE_temp[2]={0};

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

extern unsigned char cipherkey_radio[16];//本小区电台的私钥 

u8 frame_window_counter=0;	//表示处理完的滑窗数，统计完整窗口数，24位位一个窗口
u16 decoded_frame_index=0;//格雷码译码后缓冲区数组的索引值
void frame_continues(void);//续传帧处理函数
void frame_wakeup_broadcast(u16 decoded_frame_index,u8 decoded_frame[DECODE_FRAMESIZE]);//广播唤醒帧处理函数
//void frame_wakeup_unicast(u16 decoded_frame_index,u8 decoded_frame[DECODE_FRAMESIZE]);//单播组播唤醒帧处理函数
void frame_control(u16 decoded_frame_index,u8 decoded_frame[DECODE_FRAMESIZE]);//控制帧处理函数
void frame_secure(u16 decoded_frame_index,u8 decoded_frame[DECODE_FRAMESIZE]);//认证帧处理函数
unsigned char XOR(unsigned char *BUFF, u16 len);
void DKA065(int index);
u16 alarm_frame_index=0;//报警索引的全局变量，用于timer.c中

void communication_right(void);//唤醒成功
void communication_wrong(void);//广播结束

u8 ecc_right=1;//ecc通过标志位。>1：未通过；0：通过；

u32 frame_counters=0;//唤醒帧中bits转帧计数器
u32 source_addres=0;//唤醒帧中bits转源地址
const u32 my_broad_ID=RADIO_ID_START;//本社区电台ID。34095233~1073741823
u32 target_address_start=0;//唤醒帧中bits转目标起始地址
u32 target_address_end=0;//唤醒帧中bits转目标终止地址
const u32 my_ID=AREA_TERMINAL_ID_START;//终端ＩＤ编号。终端ＩＤ段起始ＩＤ
													   
u8 flag_main_busy=0;//主函数正在忙禁止TIM3flash中断标志位
u8 sage_confirmd=0;//安全认证是否收到。0：未收到；1：已收到；

//终端ＩＤ
//double terminal_ID=my_broad_ID<<18+my_ID;
int main(void)
{	
 	u16 index=0;
	u8 i=0,t=0;//for循环
	float sample_rate=0;
	u8 gray_decode_buf1[24]={0};//格雷译码，24个码元为一组
	u8 gray_decoded_buf1[24]={0};//格雷译码纠错后的24位码元，其前12位为原始信息的倒叙排列

	u8 decoded_frame[DECODE_FRAMESIZE]={0};//格雷译码后数据的缓冲区
	u8 frame_type=0;//帧类型
	u16 test=0,test1=0;		
	u16 len;

	u8 index_frame_send=0;//串口回复信息帧下标
	u8 frame_send_buf[100]={0};//串口回传缓冲区

	u8 xor_sum=0;//异或校验和
		
	delay_init();	    	 //延时函数初始化	  
	NVIC_Configuration(); 	 //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	uart_init(256000);	 //串口初始化为9600
	uart2_init(9600);	 //串口2初始化为9600
 	LED_Init();			     //LED端口初始化	
//	WKUP_Init(); //待机唤醒初始化
	LED1=0;
 	TIM2_Cap_Init(0XFFFF,72-1);	//以1Mhz的频率计数,0xFFFF为65.535ms
	TIM3_Int_Init(9999,7199);//定期存储帧计数值；安全认证定期查询，1s中断一次，10s统计一次
	TIM4_Int_Init(7999,7199); //DKA占用喇叭的时间
	///////////////////读取FLASH中状态标志位，判断是否首次初始化////////////////////////////////
	STMFLASH_Read(FLASH_STATE_ADDR,(u16*)STATE_temp,STATE_SIZE);
	if(STATE_temp[0]!=0x33){//首次初始化
		TEXT_Buffer[0]=0;//先对flash帧计数器初始化为0
		TEXT_Buffer[1]=0;
		TEXT_Buffer[2]=0;
		TEXT_Buffer[3]=0;
		STMFLASH_Write(FLASH_SAVE_ADDR,(u16*)TEXT_Buffer,SIZE);//把帧计数值分四个字节保存到flash中
		frame_counters=0;
		STATE_temp[0]=0x33;
		STMFLASH_Write(FLASH_STATE_ADDR,(u16*)STATE_temp,STATE_SIZE);
	}else{
		STMFLASH_Read(FLASH_SAVE_ADDR,(u16*)flash_temp,SIZE);
		frame_counters=flash_temp[0]*256*256*256+flash_temp[1]*256*256+flash_temp[2]*256+flash_temp[3];//启动时读入flash中的帧计数器值
		
	}
	frame_window_counter=0;
   	while(1)
	{	  		 		 
//接收完信道中的完整一帧后，对其格雷译码，还原出原始数据，存入decoded_frame[]中
		if(((TIM5CH1_CAPTURE_STA&0X200)==0)&&(TIM5CH1_CAPTURE_STA&0X0800)&&(((frame_index+1)/24)>frame_window_counter)){ //未处理数据大于一个窗
		
		for(i=0;i<24;i++){
			gray_decode_buf1[i]=receive_frame[i+frame_window_counter*24];//将接收缓冲区中未处理的数据放到缓冲窗中，准备格雷译码
		}
		decode_error_catch(gray_decode_buf1,gray_decoded_buf1);//格雷译码,纠错结果为gray_decoded_buf[]，前12位码元为原始数据的倒序排列
		for(i=0;i<12;i++){
			decoded_frame[decoded_frame_index]=gray_decoded_buf1[11-i];//格雷译码后数据的缓冲区
			decoded_frame_index++;//格雷码译码后缓冲区数组的索引值
		}
		frame_window_counter++;//解码窗滑动一次
		if(decoded_frame_index*2==frame_lengths){
			TIM5CH1_CAPTURE_STA|=0X0200;
			frame_window_counter=0;
		}
//printf("\r\n格雷：TIM5CH1_CAPTURE_STA=%d,frame_window_counter=%d,decoded_frame_index=%d,frame_lengths=%d\r\n",TIM5CH1_CAPTURE_STA,frame_window_counter,decoded_frame_index,frame_lengths);
		}

		if(test<65533)test++;
		else if(test>=65533){test1++;test=0;}
		if(test1==90){
			TIM_Cmd(TIM3, ENABLE);
			printf("\r\nmain运行中!TIM5CH1_CAPTURE_STA=%d,frame_window_counter=%d,decoded_frame_index=%d\r\n",TIM5CH1_CAPTURE_STA,frame_window_counter,decoded_frame_index);
			test1=0;
		
		}


		if((TIM5CH1_CAPTURE_STA&0X0200)&&(TIM5CH1_CAPTURE_STA&0X08))//格雷码解码完毕
		{
			flag_main_busy=1;//准备串口发送禁止TIM3中断
/**************************************打印测试数据*********************************************************/			
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
/**************************************格雷译码结果*********************************************************/
		   printf("\r\n解码帧：\r\n");

		   for(index=0;index<decoded_frame_index;index++)//格雷解码后的数据帧
		   {
		 	  printf("%d",decoded_frame[index]);
		   }

		   	frame_type=decoded_frame[0]*2+decoded_frame[1];
			switch(frame_type){
				case 0:
					frame_continues();//续传帧处理函数
					break;//续传帧
				case 1:
					frame_wakeup_broadcast(decoded_frame_index,decoded_frame);//广播唤醒帧处理函数(单播组播广播一并处理)
					break;//唤醒帧
				case 2:
					frame_control(decoded_frame_index,decoded_frame);//控制帧处理函数
					break;//控制帧
				case 3:
					frame_secure(decoded_frame_index,decoded_frame);//认证帧处理函数
					break;//认证帧
			}

/**************************************本帧处理完毕，接收下一帧************************************************/
		   TIM5CH1_CAPTURE_STA=0;//帧处理完毕的清零
		   decoded_frame_index=0; 
		   frame_window_counter=0;
		   TIM2_Cap_Init(0XFFFF,72-1);
		   TIM_Cmd(TIM2,ENABLE);
		   
		flag_main_busy=0;   
		}//end of 格雷码译码后的帧处理



/******************************************************************串口2接收数据************************************************************************/
		if(USART2_RX_STA&0x8000)
		{
			flag_main_busy=1;//主函数正在忙禁止TIM3flash中断标志位			   
			len=USART2_RX_STA&0x3fff;//得到此次接收到的数据长度

			if((USART2_RX_BUF[0]=='$')&&(len>0)){
				xor_sum=XOR(USART2_RX_BUF,len-1);
				if((xor_sum=='$')||(xor_sum==0x0d)){xor_sum++;}
				if(USART2_RX_BUF[len-1]==xor_sum){
 					/*******************************************安全芯片确认帧******************************************************************/
					if((USART2_RX_BUF[1]=='e')&&(USART2_RX_BUF[2]=='c')&&(USART2_RX_BUF[3]=='c')&&(USART2_RX_BUF[4]=='_')&&(USART2_RX_BUF[5]=='_')){//连接帧
						LED0=1;//关闭警示灯
						ecc_right=USART2_RX_BUF[9];//返回是否通过校验
						if(ecc_right==0){//ecc通过
							printf("ecc access!");
							sage_confirmd=1;//认证通过
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
							frame_send_buf[index_frame_send]=2;//2：本帧是通知MSP430非对称认证是否通过的帧;1:更改通信频点帧;
							index_frame_send++;
							frame_send_buf[index_frame_send]=1;//1：表示通过
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
								USART_SendData(USART1, frame_send_buf[t]);//向串口发送数据
								while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);//等待发送结束
							}
						}else{//ecc未通过
							printf("ecc wrong!");
							sage_confirmd=0;//认证失败
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
							frame_send_buf[index_frame_send]=2;//2：本帧是通知MSP430非对称认证是否通过的帧;1:更改通信频点帧;
							index_frame_send++;
							frame_send_buf[index_frame_send]=0;//1：表示通过,0:未通过;
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
								USART_SendData(USART1, frame_send_buf[t]);//向串口发送数据
								while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);//等待发送结束
							}
						}
						USART2_RX_STA=0;//处理完毕，允许接收下一帧
						USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//打开中断
					}
 					/*******************************************安全芯片请求重传*******************************************************************/
				 	else if((USART2_RX_BUF[1]=='r')&&(USART2_RX_BUF[2]=='t')&&(USART2_RX_BUF[3]=='s')&&(USART2_RX_BUF[4]=='_')&&(USART2_RX_BUF[5]=='_')){//重传帧
						LED0=1;//关闭警示灯,1、重传这里需要可能存在问题
						frame_secure(decoded_frame_index,decoded_frame);//认证帧处理函数
						USART2_RX_STA=0;//处理完毕，允许接收下一帧
						USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//打开中断

					}else {USART2_RX_STA=0;USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);}//帧类型出错,请求重传
				 }else{USART2_RX_STA=0;USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);}//end of XOR，XOR出错请求重传
				 }else {USART2_RX_STA=0;USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);}//end of check '$'

		flag_main_busy=0;
		}else {USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);}//此处等待串口2回传数据，故usart2_works不能清零，但要添加对其为0的条件判断



	}//end of while
}


void frame_continues(void){//续传帧处理函数

}

void frame_wakeup_broadcast(u16 decoded_frame_index,u8 decoded_frame[DECODE_FRAMESIZE]){//广播唤醒帧处理函数
	
	u8 i=0,t=0;
	u16 index=0;
	u8 aes_bits[128]={0};//AES加密数据比特流
	u8 aes_char[4][4]={0};//AES	128bits转换位4*4的u8矩阵
	double fre_point=0;//打印测试,数据帧中提取出的通信频点
	u8 communication_point=0;//数据帧中提取出的通信频点
	u8 xorsum=0;
	u8 index_frame_send=0;//串口回复信息帧下标
	u8 frame_send_buf[100]={0};//串口回传缓冲区
	u32 frame_nums=0;//从帧中提取的帧计数器值，用于比较到来帧是否是最新的


	/**************************************AES校验**************************************************************/
	for(i=0;i<128;i++){
	   if(i<(decoded_frame_index-36)){//把格雷译码后的数据中后36位前的内容放到aes_bits[]中
		  aes_bits[i]=decoded_frame[i];
	  }else{
 	  	  aes_bits[i]=0;//不够128位则补零，超过128，则此处不执行，仅取前128位
	  }
	}
	printf("\r\nAES string:\r\n");
	for(index=0;index<128;index++)//接收到的有用数据串，用于AES
	{
		  printf("%d",aes_bits[index]);
	}
	bit_char(aes_bits,aes_char);//比特流转4*4u8矩阵
	Encrypt(aes_char,cipherkey_radio);
	char_bit(aes_char,aes_bits);
	printf("\r\nAES native string:\r\n");
	for(index=0;index<128;index++)//AES本地校验串
	{
	     printf("%d",aes_bits[index]);
	}
	printf("\r\n");
	i=0;
	for(index=(decoded_frame_index-36);index<decoded_frame_index;index++){//本地计算的AES与传送的AES进行比对
	    if(decoded_frame[index]!=aes_bits[i]){//如果有某一位不同
//			 TIM5CH1_CAPTURE_STA=0;//各关键寄存器清零，等待接收帧
//			 decoded_frame_index=0; 
//			 frame_window_counter=0;
//			 TIM_Cmd(TIM2,ENABLE);
			 printf("\r\nWakeup AES wrong!!\r\n"); 
			 SAFE_CHIP=0;//关闭安全芯片
			 return;//检测到不同，立刻终止验证
		  }
		  i++;
	}
	if(i==36){TIM5CH1_CAPTURE_STA|=0X0400;}//AES验证通过
	SAFE_CHIP=1;//打开安全芯片
/*************************************目标地址提取************************************************************/
	if(TIM5CH1_CAPTURE_STA&0X0400){//AES检测通过
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
		frame_send_buf[index_frame_send]=1;//表示本帧是通知MSP430频点的问题,因为STM32不知道通信频点是多少.1:广播；2：单播；3：组播；
		index_frame_send++;
		frame_send_buf[index_frame_send]=communication_point+0x30;//通信频点值可能出现0x0d,0x24
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
			USART_SendData(USART1, frame_send_buf[t]);//向串口发送数据
			while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);//等待发送结束
		}

		fre_point=76+(double)communication_point/10.0;
		printf("comunation point:%fMHz\r\n",fre_point);//打印通信频点的值

		frame_nums=0;
		for(index=0;index<32;index++){
			frame_nums|=decoded_frame[decoded_frame_index-72+index]<<(31-index);
		}
		printf("frame_nums:%d\r\n",frame_nums);//打印帧计数器值

		if(frame_nums<=frame_counters){
			printf("frame is old.");
			return;
		}else{
			frame_counters=frame_nums;
		}
		source_addres=0;
		for(index=0;index<30;index++){
			source_addres|=decoded_frame[12+index]<<(29-index);//源地址在译码帧中从第12bits开始，共30bits
		}

		if(source_addres==my_broad_ID){//判断电台ID是否合法
			printf("redio ID correct.\r\n");
		//	communication_right();
		}else{
			printf("redio ID wrong.\r\n");
			communication_wrong();
		}
		printf("source_addres:%d\r\n",source_addres);//打印电台ID


	//	communication_right(); //唤醒成功

		if((decoded_frame[2]*2+decoded_frame[3])==2){//广播唤醒帧处理
			communication_right();
			printf("terminal ID correct. broadcast wakeup AES SUCCESS\r\n\r\n");
		} 
		else if((decoded_frame[2]*2+decoded_frame[3])==1){//单播唤醒帧处理
			printf("unicast wakeup AES SUCCESS\r\n\r\n");

			target_address_start=0;
			for(index=0;index<24;index++){
				target_address_start|=decoded_frame[48+index]<<(23-index);//单播地址在译码帧中从第48bits开始，共24bits
			}
			if(target_address_start==my_ID){
				communication_right();
				printf("redio ID correct.\r\n");
			}else{
				printf("redio ID wrong.\r\n");
				communication_wrong();
			}
			printf("target_address_start:%d\r\n",target_address_start);//打印单播地址

		}else if((decoded_frame[2]*2+decoded_frame[3])==3){//组播唤醒帧处理
			printf("multicast wakeup AES SUCCESS\r\n\r\n");

			target_address_start=0;
			for(index=0;index<24;index++){
				target_address_start|=decoded_frame[48+index]<<(23-index);//组播起始地址在译码帧中从第48bits开始，共24bits
			}
			printf("target_address_start:%d\r\n",target_address_start);//打印单播地址

			target_address_end=0;
			for(index=0;index<24;index++){
				target_address_end|=decoded_frame[72+index]<<(23-index);//组播终止地址在译码帧中从第72bits开始，共24bits
			}

			if((my_ID>=target_address_start)&&(my_ID<=target_address_end)){				
				communication_right();
				printf("redio ID correct.\r\n");
			}else{
				printf("redio ID wrong.\r\n");
				communication_wrong();
			}

			printf("target_address_end:%d\r\n",target_address_end);//打印单播地址

		}else printf("\r\nfinal wrong!!\r\n");   
		   
		   
	}
	return;
}

//void frame_wakeup_unicast(u16 decoded_frame_index,u8 decoded_frame[DECODE_FRAMESIZE]){//单播组播唤醒帧处理函数
//	
//}

//6 extern u8 alarm_over;//10次报警是否播放完毕。0：完毕；1：未完；
extern u8 alarm_times;//报警播放次数
void frame_control(u16 decoded_frame_index,u8 decoded_frame[DECODE_FRAMESIZE]){//控制帧处理函数
	u8 i=0;
	u16 index=0;
	u8 aes_bits[128]={0};//AES加密数据比特流
	u8 aes_char[4][4]={0};//AES	128bits转换位4*4的u8矩阵
	u32 frame_nums=0;//本地保存的帧计数器值，用于比较到来帧是否是最新的
	/**************************************AES校验**************************************************************/
	for(i=0;i<128;i++){
	   if(i<(decoded_frame_index-36)){//把格雷译码后的数据中后36位前的内容放到aes_bits[]中
		  aes_bits[i]=decoded_frame[i];
	  }else{
	  	  aes_bits[i]=0;//不够128位则补零，超过128，则此处不执行，仅取前128位
	  }
	}
	printf("\r\nAES string:\r\n");
	for(index=0;index<128;index++)//接收到的有用数据串，用于AES
	{
		  printf("%d",aes_bits[index]);
	}
	bit_char(aes_bits,aes_char);//比特流转4*4u8矩阵
	Encrypt(aes_char,cipherkey_radio);
	char_bit(aes_char,aes_bits);
	printf("\r\nAES native string:\r\n");
	for(index=0;index<128;index++)//AES本地校验串
	{
	     printf("%d",aes_bits[index]);
	}
	printf("\r\n");
	i=0;
	for(index=(decoded_frame_index-36);index<decoded_frame_index;index++){//本地计算的AES与传送的AES进行比对
	    if(decoded_frame[index]!=aes_bits[i]){//如果有某一位不同
			 printf("\r\nControl AES wrong!!\r\n"); 
			 communication_wrong();
			 return;//检测到不同，立刻终止验证
		  }
		  i++;
	}
	if(i==36){TIM5CH1_CAPTURE_STA|=0X0400;}//AES验证通过
/*************************************控制代号提取************************************************************/
	index=0;//控制编号的索引值
	if(TIM5CH1_CAPTURE_STA&0X0400){//AES检测通过
	 	for(i=2;i<12;i++){
			index|=decoded_frame[i]<<(11-i);
		}
		printf("\r\nwaring index=%d\r\n",index);
		communication_right();//也算唤醒成功
		if(index<99){
//4			if(alarm_over==0){ //防止后续报警帧的影响。收到一次，就不处理后续的了
//3				alarm_over=1;
				alarm_times=0;//7
				if(alarm_frame_index!=index){//同一批次重复发送的报警帧，第一个以后的被忽略
					alarm_frame_index=index;
					DKA_SWITCH=1;//开关选择到DKA上
					DKA065(index);//发报警音频
			//		delay_ms(1300);
					TIM4->ARR=0X1F3F;
			//		TIM4_Int_Init(7999,7199); //DKA占用喇叭的时间
					TIM_Cmd(TIM4, ENABLE);  //打开TIMx //给DKA芯片流出播放时间
				}
//5			}
		}
		else if (index==100){
			communication_wrong();//通信结束
			alarm_frame_index=0;	
		}
		

		frame_nums=0;
		for(index=0;index<32;index++){
			frame_nums|=decoded_frame[decoded_frame_index-72+index]<<(31-index);
		}
		printf("frame_nums:%d\r\n",frame_nums);//打印帧计数器值

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

u8 index_safe_times=0;//认证帧发送次数计数器
void frame_secure(u16 decoded_frame_index,u8 decoded_frame[DECODE_FRAMESIZE]){//认证帧处理函数,格雷译码后是468bits，每4bits转1byte，共117字节
   u16 t=0;
   u8 index_frame_safe=0;//终端向安全芯片传输的数据帧
   u8 frame_safe[130]={0};//数据帧buffer
   u8 xorsum=0;//函数中使用的异或检验和值
   LED0=0;//打开警示灯
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
		frame_safe[index_frame_safe]=decoded_frame[t*4]*8+decoded_frame[t*4+1]*4+decoded_frame[t*4+2]*2+decoded_frame[t*4+3]+0x30;//ASCII 0码对应十进制是0x30
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
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//暂时打开，只为调试
	printf("\r\nsecure frame:\r\n");
	for(t=0;t<(index_frame_safe-2);t++)//认证帧内容通过串1打印,排除掉\r\n
	{
		printf("%x ",frame_safe[t]);
	}
	for(t=0;t<index_frame_safe;t++)//认证帧通过串口2发送给安全芯片
	{
		USART_SendData(USART2, frame_safe[t]);//向串口发送数据
		while(USART_GetFlagStatus(USART2,USART_FLAG_TC)!=SET);//等待发送结束
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
	DKA_DATA=0; /* 先复位*/
	delay_us(40); /* 100us */
	DKA_RTS=0;
	delay_ms(5); /* 5ms 以上*/
	for(i=0;i<index;i++)
	{
		DKA_DATA=1; //数据拉高
		delay_us(40); //等待100us
		DKA_DATA=0; //数据拉低
		delay_us(40); //等待100us，完成一个脉冲发送
	}
}


void communication_right(void){
	GPIO_SetBits(GPIOB,GPIO_Pin_10);//1	通信成功，通知430
	GPIO_ResetBits(GPIOB,GPIO_Pin_11);//0
}

void communication_wrong(void){
	GPIO_ResetBits(GPIOB,GPIO_Pin_10);//0	通信结束，通知430
	GPIO_SetBits(GPIOB,GPIO_Pin_11);//1
}

