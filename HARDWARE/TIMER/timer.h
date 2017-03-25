#ifndef __TIMER_H
#define __TIMER_H
#include "sys.h"

#define min_interval 900//脉冲的最小波特周期
#define max_interval 1100//脉冲的最大波特周期，中心时间30ns，码速率33.3kbps
#define BIT_SYNC_GROUPS 7//位同步10码对的数量

#define ORIGEN_FRAMESIZE 1010//接收数据帧的最大缓冲区
#define DECODE_FRAMESIZE 500//格雷译码后的数据缓冲区大小
#define FSK_SPEED 1000//码元间隔1ms
#define LENGTHS_WAKEUPFRAME_BROADCAST 97-11-2*BIT_SYNC_GROUPS
#define LENGTHS_WAKEUPFRAME_UNICAST 193-11-2*BIT_SYNC_GROUPS
#define LENGTHS_CONTROLFRAME 145-11-2*BIT_SYNC_GROUPS 
#define LENGTHS_SECUREFRAME 1009-11-2*BIT_SYNC_GROUPS

void TIM3_Int_Init(u16 arr,u16 psc);
void TIM3_PWM_Init(u16 arr,u16 psc);
void TIM5_Cap_Init(u16 arr,u16 psc);
#endif
