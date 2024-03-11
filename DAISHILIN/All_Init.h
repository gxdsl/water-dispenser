#ifndef __DS18B20_H__
#define __DS18B20_H__

#include "stdint.h"

//数据DATA结构体
typedef struct{
    uint16_t Start;
	float Tem;
    float Adc;
    float Tds;
}DATA_DSL;

extern DATA_DSL DSL;   //声明结构体数据

void ALL_Init(void);

#endif
