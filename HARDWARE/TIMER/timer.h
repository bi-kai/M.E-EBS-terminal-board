#ifndef __TIMER_H
#define __TIMER_H
#include "sys.h"

#define FSK_SPEED 1000//��Ԫ���1ms
#define min_interval FSK_SPEED-100//�������С��������,100Ϊ�������
#define max_interval FSK_SPEED+100//��������������
#define BIT_SYNC_GROUPS 7//λͬ��10��Ե�����

#define ORIGEN_FRAMESIZE 1000//��������֡����󻺳���
#define DECODE_FRAMESIZE 500//�������������ݻ�������С

#define LENGTHS_WAKEUPFRAME_BROADCAST 265-11-2*BIT_SYNC_GROUPS  //�㲥����֡��
#define LENGTHS_WAKEUPFRAME_UNICAST 361-11-2*BIT_SYNC_GROUPS   //�������鲥����֡��
#define LENGTHS_CONTROLFRAME 193-11-2*BIT_SYNC_GROUPS 	//����֡��
#define LENGTHS_SECUREFRAME 965-11-2*BIT_SYNC_GROUPS	//��֤֡��

void TIM3_Int_Init(u16 arr,u16 psc);
void TIM3_PWM_Init(u16 arr,u16 psc);
void TIM5_Cap_Init(u16 arr,u16 psc);
#endif
