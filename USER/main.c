#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "usart.h"
#include "timer.h"
#include "gray.h"
#include "wkup.h"//唤醒，睡眠
#include "encrypt.h"//AES加密

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


int main(void)
{	
 	u16 index=0;
	u8 i=0;//for循环
	float sample_rate=0;
	u8 gray_decode_buf1[24]={0};//格雷译码，24个码元为一组
	u8 gray_decoded_buf1[24]={0};//格雷译码纠错后的24位码元，其前12位为原始信息的倒叙排列

	u8 decoded_frame[DECODE_FRAMESIZE]={0};//格雷译码后数据的缓冲区
	u8 frame_type=0;//帧类型
	u16 test=0,test1=0;
		
	delay_init();	    	 //延时函数初始化	  
	NVIC_Configuration(); 	 //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	uart_init(256000);	 //串口初始化为9600
	uart2_init(9600);	 //串口2初始化为9600
 	LED_Init();			     //LED端口初始化	
//	WKUP_Init(); //待机唤醒初始化
	LED1=0;
 	TIM5_Cap_Init(0XFFFF,72-1);	//以1Mhz的频率计数,0xFFFF为65.535ms
	TIM3_Int_Init(9999,7199);//5hz的计数频率 
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
		printf("\r\nmain运行中!TIM5CH1_CAPTURE_STA=%d,frame_window_counter=%d,decoded_frame_index=%d\r\n",TIM5CH1_CAPTURE_STA,frame_window_counter,decoded_frame_index);
		test1=0;
		}


		if((TIM5CH1_CAPTURE_STA&0X0200)&&(TIM5CH1_CAPTURE_STA&0X08))//格雷码解码完毕
		{
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
		   TIM5_Cap_Init(0XFFFF,72-1);
		   TIM_Cmd(TIM5,ENABLE);
		   
		   
		}//end of 格雷码译码后的帧处理







	}//end of while
}


void frame_continues(void){//续传帧处理函数

}

void frame_wakeup_broadcast(u16 decoded_frame_index,u8 decoded_frame[DECODE_FRAMESIZE]){//广播唤醒帧处理函数
	
	u8 i=0;
	u16 index=0;
	u8 aes_bits[128]={0};//AES加密数据比特流
	u8 aes_char[4][4]={0};//AES	128bits转换位4*4的u8矩阵
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
//			 TIM_Cmd(TIM5,ENABLE);
			 printf("\r\nWakeup AES wrong!!\r\n"); 
			 return;//检测到不同，立刻终止验证
		  }
		  i++;
	}
	if(i==36){TIM5CH1_CAPTURE_STA|=0X0400;}//AES验证通过
/*************************************目标地址提取************************************************************/
	if(TIM5CH1_CAPTURE_STA&0X0400){//AES检测通过
		if((decoded_frame[2]*2+decoded_frame[3])==2){//广播唤醒帧处理
			printf("broadcast wakeup AES SUCCESS\r\n\r\n");
		} 
		else if((decoded_frame[2]*2+decoded_frame[3])==1){//单播唤醒帧处理
			printf("unicast wakeup AES SUCCESS\r\n\r\n");

		}else if((decoded_frame[2]*2+decoded_frame[3])==3){//组播唤醒帧处理
			printf("multicast wakeup AES SUCCESS\r\n\r\n");

		}else printf("\r\nfinal wrong!!\r\n");   
		   
		   
	}
	return;
}

//void frame_wakeup_unicast(u16 decoded_frame_index,u8 decoded_frame[DECODE_FRAMESIZE]){//单播组播唤醒帧处理函数
//	
//}

void frame_control(u16 decoded_frame_index,u8 decoded_frame[DECODE_FRAMESIZE]){//控制帧处理函数
	u8 i=0;
	u16 index=0;
	u8 aes_bits[128]={0};//AES加密数据比特流
	u8 aes_char[4][4]={0};//AES	128bits转换位4*4的u8矩阵
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

u8 index_safe_times=0;//认证帧发送次数计数器
void frame_secure(u16 decoded_frame_index,u8 decoded_frame[DECODE_FRAMESIZE]){//认证帧处理函数,格雷译码后是468bits，每4bits转1byte，共117字节
   u16 t=0;
   u8 index_frame_safe=0;//终端向安全芯片传输的数据帧
   u8 frame_safe[130]={0};//数据帧buffer
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
		frame_safe[index_frame_safe]=decoded_frame[t*4]*8+decoded_frame[t*4+1]*4+decoded_frame[t*4+2]*2+decoded_frame[t*4+3]*1+0x30;//ASCII 0码对应十进制是0x30
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
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//暂时打开，只为调试
	for(t=0;t<index_frame_safe;t++)//认证帧通过串口2发送给安全芯片
	{
		USART_SendData(USART2, frame_safe[t]);//向串口发送数据
		while(USART_GetFlagStatus(USART2,USART_FLAG_TC)!=SET);//等待发送结束
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



