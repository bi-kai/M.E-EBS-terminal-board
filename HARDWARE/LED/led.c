#include "led.h"

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

//��ʼ��PB5��PE5Ϊ�����.��ʹ���������ڵ�ʱ��		    
//LED IO��ʼ��
void LED_Init(void)
{
 
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOD|RCC_APB2Periph_GPIOC, ENABLE);	 //ʹ��PB,PE�˿�ʱ��
//PB	
 GPIO_InitStructure.GPIO_Pin =GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14;			
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
 GPIO_Init(GPIOB, &GPIO_InitStructure);					
 GPIO_ResetBits(GPIOB,GPIO_Pin_6);
 GPIO_ResetBits(GPIOB,GPIO_Pin_7);
GPIO_ResetBits(GPIOB,GPIO_Pin_10);//��430ͨ�Źܽ�0
GPIO_ResetBits(GPIOB,GPIO_Pin_11);//��430ͨ�Źܽ�1

GPIO_ResetBits(GPIOB,GPIO_Pin_12);//�������أ���0,�е�ģ������
GPIO_ResetBits(GPIOB,GPIO_Pin_13);	 //DKA RTS	  PB13
GPIO_ResetBits(GPIOB,GPIO_Pin_14);	 //DKA DATA	  PB14 
//PA
/*
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;//LED0
 GPIO_Init(GPIOA, &GPIO_InitStructure);
 GPIO_SetBits(GPIOA,GPIO_Pin_8);						 
//PD
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;//LED1
 GPIO_Init(GPIOD, &GPIO_InitStructure);	  				
 GPIO_SetBits(GPIOD,GPIO_Pin_2); 
 */						 
//PC
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9|GPIO_Pin_6|GPIO_Pin_7;
 GPIO_Init(GPIOC, &GPIO_InitStructure);	   
 GPIO_SetBits(GPIOC,GPIO_Pin_6); //LED1
 GPIO_SetBits(GPIOC,GPIO_Pin_7); //LED0
 GPIO_ResetBits(GPIOC,GPIO_Pin_9);//��ȫоƬ���硣0��ͣ�磻1�����磻 
  
   						 
}
 
