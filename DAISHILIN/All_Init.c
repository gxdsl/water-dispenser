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

DATA_DSL DSL = {0};   //�����ṹ������

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
    HAL_UART_Receive_IT(&huart1, (uint8_t *)&Rx1Data, 1);//����һ�δ����жϺ���
    HAL_UART_Receive_IT(&huart2, (uint8_t *)&Rx2Data, 1);//����һ�δ����жϺ���
    HAL_UART_Receive_IT(&huart3, (uint8_t *)&Rx3Data, 1);//����һ�δ����жϺ���
    
//    HAL_TIM_Base_Start(&htim2);
//  
//    __HAL_TIM_CLEAR_FLAG(&htim7, TIM_SR_UIF);       //��ֹTIM7����ʱ��һ���ж�
//    HAL_TIM_Base_Start_IT(&htim7);                  //������ʱ��7,ÿ��һ���ж�
    
    TDS_Init();
    
    Ds18b20_Init();
    
    ESP8266_Init();
    
    PCD_Init();
    
    LoRa_Init();
}


void LoRa_Init(void)
{
    printf("LoRa��ʼ���ɹ�\r\n");
    
    DSL.Start = 100;
    Usart3Printf("t0.txt=\"LoRa��ʼ���ɹ�\"\xFF\xFF\xFFj0.val=%d\xFF\xFF\xFF", DSL.Start);
    
    HAL_Delay(500);
}

