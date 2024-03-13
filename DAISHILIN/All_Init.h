#ifndef __DS18B20_H__
#define __DS18B20_H__

#include "stdbool.h"
#include "stdint.h"

//饮水机数据DATA结构体
typedef struct{
    bool Mode;      //是否进入用户操作模式
    uint8_t ID;     //设备ID
    uint16_t Start; //系统初始化进度
	float Tem;      //水温
    float Adc;      //TDS模块ADC的值
    float Tds;      //TDS水质
}DATA_DSL;

extern DATA_DSL DSL;   //声明结构体数据

void ALL_Init(void);

#endif
