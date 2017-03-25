#ifndef __TIMER_H
#define __TIMER_H
#include "sys.h"

#define FSK_SPEED 1000//码元间隔1ms
#define min_interval FSK_SPEED-100//脉冲的最小波特周期,100为误差容限
#define max_interval FSK_SPEED+100//脉冲的最大波特周期
#define BIT_SYNC_GROUPS 7//位同步10码对的数量

#define ORIGEN_FRAMESIZE 1000//接收数据帧的最大缓冲区
#define DECODE_FRAMESIZE 500//格雷译码后的数据缓冲区大小

#define LENGTHS_WAKEUPFRAME_BROADCAST 265-11-2*BIT_SYNC_GROUPS  //广播唤醒帧长
#define LENGTHS_WAKEUPFRAME_UNICAST 361-11-2*BIT_SYNC_GROUPS   //单播、组播唤醒帧长
#define LENGTHS_CONTROLFRAME 193-11-2*BIT_SYNC_GROUPS 	//控制帧长
#define LENGTHS_SECUREFRAME 965-11-2*BIT_SYNC_GROUPS	//认证帧长

void TIM3_Int_Init(u16 arr,u16 psc);
void TIM3_PWM_Init(u16 arr,u16 psc);
void TIM5_Cap_Init(u16 arr,u16 psc);
#endif
