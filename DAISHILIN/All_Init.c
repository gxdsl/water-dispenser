#include "main.h"
#include "All_Init.h"
#include "DS18B20.h" 
#include "stm32f4xx.h"                  // Device header
#include "ESP8266.h"
#include "usart.h"
#include "usart_dsl.h"
#include "adc.h"
#include "core_delay.h"
#include "tim.h"
#include "TDS.h"
#include "RC522.h"
#include "YFS401.h"

// ��������ʼ���ṹ��ʵ�� DSL
DATA_DSL DSL =
{
    .Mode = 0,   // δ�����û�����ģʽ
    .Flow = false,   // ��ˮ��δ��ˮ
    .ID = 1,     // �豸IDΪ1
    .Start = 0,  // ϵͳ��ʼ������Ϊ0
    .Tem = 0.0f, // ˮ��Ϊ0.0
    .Adc = 0.0f, // TDSģ��ADC��ֵΪ0.0
    .Tds = 0.0f  // TDSˮ��Ϊ0.0
};

void Ds18b20_Init(void);
void LoRa_Init(void);



//==============================================================================
// @����: void ALL_Init(void)
// @����: ��Ŀ���к�����ʼ��
// @����: None
// @����: None
// @ʱ��: 2023.10.30
//==============================================================================
void ALL_Init(void)
{
    HAL_GPIO_WritePin(relay_GPIO_Port,relay_Pin,GPIO_PIN_SET);      //�̵�����
    
    HAL_UART_Receive_IT(&huart1, (uint8_t *)&Rx1Data, 1);//����һ�δ����жϺ���
    HAL_UART_Receive_IT(&huart2, (uint8_t *)&Rx2Data, 1);//����һ�δ����жϺ���
    HAL_UART_Receive_IT(&huart3, (uint8_t *)&Rx3Data, 1);//����һ�δ����жϺ���
    
    
    Usart3Printf("page 0\xFF\xFF\xFF");     //�ص���Ļ��ʼ����
    HAL_Delay(1000);
    
    TDS_Init();
    
    Ds18b20_Init();
    Init_Progress(20);
    
    ESP8266_Init();
    
    PCD_Init();
    
    LoRa_Init();
    
}


void LoRa_Init(void)
{
    LoRaPrintf("LoRa��ʼ���ɹ�");
    HAL_Delay(500);
    
    DSL.Start = 100;
    Usart3Printf("t0.txt=\"LoRa��ʼ���ɹ�\"\xFF\xFF\xFFj0.val=%d\xFF\xFF\xFF", DSL.Start);
    
    HAL_Delay(700);
}


void Init_Progress(unsigned int Progress)
{
    DSL.Start += Progress;
    
    Usart3Printf("j0.val=%d\xFF\xFF\xFF", DSL.Start);
    
    HAL_Delay(1000);
}





