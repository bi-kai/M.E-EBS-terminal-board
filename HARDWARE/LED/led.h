#ifndef __LED_H
#define __LED_H	 
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEKս��STM32������
//LED��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2012/9/2
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 
//#define LED0 PBout(5)// PB5
//#define LED1 PEout(5)// PE5	
#define LED0 PCout(7)// PA8
#define LED1 PCout(6)// PD2

#define SAFE_CHIP PCout(9)//��ȫоƬ����

#define DKA_SWITCH PBout(12)//DKA/�����л����ء�1���л���DKA��0���л���������
#define DKA_RTS PBout(14)//DKA RTS
#define DKA_DATA PBout(13)//DKA	DATA

void LED_Init(void);//��ʼ��

		 				    
#endif
