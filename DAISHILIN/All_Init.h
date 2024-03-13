#ifndef __DS18B20_H__
#define __DS18B20_H__

#include "stdbool.h"
#include "stdint.h"

//��ˮ������DATA�ṹ��
typedef struct{
    bool Mode;      //�Ƿ�����û�����ģʽ
    uint8_t ID;     //�豸ID
    uint16_t Start; //ϵͳ��ʼ������
	float Tem;      //ˮ��
    float Adc;      //TDSģ��ADC��ֵ
    float Tds;      //TDSˮ��
}DATA_DSL;

extern DATA_DSL DSL;   //�����ṹ������

void ALL_Init(void);

#endif
