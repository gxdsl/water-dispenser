#ifndef __TDS_H__
#define __TDS_H__

#include "stm32f4xx.h"                  // Device header

#define NUM_CHANNELS 1               //通道数
#define NUM_SAMPLES_PER_CHANNEL 50  //采样数

extern uint8_t DMA_Status;              //DMA中断标志位
extern uint32_t ADC_DMABuffer[];        //数组用于保存DMA数值
extern uint16_t ADC_DMABuffer_AV[];     //ADC平均值数组

void TDS_Init(void);
void TDS_GetValue(void);


#endif
