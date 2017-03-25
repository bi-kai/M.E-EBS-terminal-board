#ifndef __TIMER_H
#define __TIMER_H
#include "sys.h"

#define min_interval 40//脉冲的最小波特周期
#define max_interval 60//脉冲的最大波特周期，中心时间30ns，码速率33.3kbps
#define max_framesize 19+511//接收数据帧的最大缓冲区

void TIM3_Int_Init(u16 arr,u16 psc);
void TIM3_PWM_Init(u16 arr,u16 psc);
void TIM5_Cap_Init(u16 arr,u16 psc);
#endif
