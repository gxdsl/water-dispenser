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

DATA_DSL DSL = {0};   //声明结构体数据

void Ds18b20_Init(void);
void LoRa_Init(void);



//==============================================================================
// @函数: void ALL_Init(void)
// @描述: 项目所有函数初始化
// @参数: None
// @返回: None
// @时间: 2023.10.30
//==============================================================================
void ALL_Init(void)
{
    HAL_UART_Receive_IT(&huart1, (uint8_t *)&Rx1Data, 1);//调用一次串口中断函数
    HAL_UART_Receive_IT(&huart2, (uint8_t *)&Rx2Data, 1);//调用一次串口中断函数
    HAL_UART_Receive_IT(&huart3, (uint8_t *)&Rx3Data, 1);//调用一次串口中断函数
    
//    HAL_TIM_Base_Start(&htim2);
//  
//    __HAL_TIM_CLEAR_FLAG(&htim7, TIM_SR_UIF);       //防止TIM7启动时进一次中断
//    HAL_TIM_Base_Start_IT(&htim7);                  //启动定时器7,每秒一次中断
    
    TDS_Init();
    
    Ds18b20_Init();
    
    ESP8266_Init();
    
    PCD_Init();
    
    LoRa_Init();
}


void LoRa_Init(void)
{
    printf("LoRa初始化成功\r\n");
    
    DSL.Start = 100;
    Usart3Printf("t0.txt=\"LoRa初始化成功\"\xFF\xFF\xFFj0.val=%d\xFF\xFF\xFF", DSL.Start);
    
    HAL_Delay(500);
}

