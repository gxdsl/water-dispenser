#ifndef __TDS_H__
#define __TDS_H__

#include "stm32f4xx.h"                  // Device header

#define NUM_CHANNELS 1               //ͨ����
#define NUM_SAMPLES_PER_CHANNEL 50  //������

extern uint8_t DMA_Status;              //DMA�жϱ�־λ
extern uint32_t ADC_DMABuffer[];        //�������ڱ���DMA��ֵ
extern uint16_t ADC_DMABuffer_AV[];     //ADCƽ��ֵ����

void TDS_Init(void);
void TDS_GetValue(void);


#endif
