#ifndef __TIMER_H
#define __TIMER_H
#include "sys.h"

#define min_interval 40//�������С��������
#define max_interval 60//��������������
#define max_framesize 511//��������֡����󻺳���

void TIM3_Int_Init(u16 arr,u16 psc);
void TIM3_PWM_Init(u16 arr,u16 psc);
void TIM5_Cap_Init(u16 arr,u16 psc);
#endif
