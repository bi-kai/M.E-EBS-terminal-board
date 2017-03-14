#ifndef __TIMER_H
#define __TIMER_H
#include "sys.h"

#define FSK_SPEED 1000//��Ԫ���1ms
#define min_interval FSK_SPEED-300//�������С��������,100Ϊ�������
#define max_interval FSK_SPEED+300//��������������
#define BIT_SYNC_GROUPS 7//λͬ��10��Ե�����

#define ORIGEN_FRAMESIZE 1100//��������֡����󻺳���
#define DECODE_FRAMESIZE ORIGEN_FRAMESIZE/2//�������������ݻ�������С

#define LENGTHS_WAKEUPFRAME_BROADCAST 265-11-2*BIT_SYNC_GROUPS  //�㲥����֡��
#define LENGTHS_WAKEUPFRAME_UNICAST 313-11-2*BIT_SYNC_GROUPS   //��������֡��
#define LENGTHS_WAKEUPFRAME_MULTICAST 361-11-2*BIT_SYNC_GROUPS   //�鲥����֡��
#define LENGTHS_CONTROLFRAME 193-11-2*BIT_SYNC_GROUPS 	//����֡��
#define LENGTHS_SECUREFRAME 961-11-2*BIT_SYNC_GROUPS	//��֤֡��

#define FLASH_CHECK_TIMES 10

void TIM3_Int_Init(u16 arr,u16 psc);
void TIM3_PWM_Init(u16 arr,u16 psc);
void TIM2_Cap_Init(u16 arr,u16 psc);
void TIM4_Int_Init(u16 arr,u16 psc);

extern void communication_right(void);//���ѳɹ�
extern void communication_wrong(void);//�㲥����
extern void DKA065(int index);
#endif
