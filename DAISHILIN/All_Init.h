#ifndef __DS18B20_H__
#define __DS18B20_H__

#include "stdint.h"

//����DATA�ṹ��
typedef struct{
    uint16_t Start;
	float Tem;
    float Adc;
    float Tds;
}DATA_DSL;

extern DATA_DSL DSL;   //�����ṹ������

void ALL_Init(void);

#endif
