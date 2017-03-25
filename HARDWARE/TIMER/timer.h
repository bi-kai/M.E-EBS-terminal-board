#ifndef __TIMER_H
#define __TIMER_H
#include "sys.h"

#define min_interval 900//�������С��������
#define max_interval 1100//�������������ڣ�����ʱ��30ns��������33.3kbps
#define BIT_SYNC_GROUPS 7//λͬ��10��Ե�����

#define ORIGEN_FRAMESIZE 1010//��������֡����󻺳���
#define DECODE_FRAMESIZE 500//�������������ݻ�������С
#define FSK_SPEED 1000//��Ԫ���1ms
#define LENGTHS_WAKEUPFRAME_BROADCAST 97-11-2*BIT_SYNC_GROUPS
#define LENGTHS_WAKEUPFRAME_UNICAST 193-11-2*BIT_SYNC_GROUPS
#define LENGTHS_CONTROLFRAME 145-11-2*BIT_SYNC_GROUPS 
#define LENGTHS_SECUREFRAME 1009-11-2*BIT_SYNC_GROUPS

void TIM3_Int_Init(u16 arr,u16 psc);
void TIM3_PWM_Init(u16 arr,u16 psc);
void TIM5_Cap_Init(u16 arr,u16 psc);
#endif
