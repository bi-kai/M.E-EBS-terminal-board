#ifndef __TIMER_H
#define __TIMER_H
#include "sys.h"

#define min_interval 40//�������С��������
#define max_interval 60//�������������ڣ�����ʱ��30ns��������33.3kbps
#define max_framesize 19+511//��������֡����󻺳���

void TIM3_Int_Init(u16 arr,u16 psc);
void TIM3_PWM_Init(u16 arr,u16 psc);
void TIM5_Cap_Init(u16 arr,u16 psc);
#endif
