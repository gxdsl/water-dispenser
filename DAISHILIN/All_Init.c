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

// 声明并初始化结构体实例 DSL
DATA_DSL DSL =
{
    .Mode = 0,   // 未进入用户操作模式
    .Flow = false,   // 饮水机未出水
    .ID = 1,     // 设备ID为1
    .Start = 0,  // 系统初始化进度为0
    .Tem = 0.0f, // 水温为0.0
    .Adc = 0.0f, // TDS模块ADC的值为0.0
    .Tds = 0.0f  // TDS水质为0.0
};

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
    HAL_GPIO_WritePin(relay_GPIO_Port,relay_Pin,GPIO_PIN_SET);      //继电器关
    
    HAL_UART_Receive_IT(&huart1, (uint8_t *)&Rx1Data, 1);//调用一次串口中断函数
    HAL_UART_Receive_IT(&huart2, (uint8_t *)&Rx2Data, 1);//调用一次串口中断函数
    HAL_UART_Receive_IT(&huart3, (uint8_t *)&Rx3Data, 1);//调用一次串口中断函数
    
    
    Usart3Printf("page 0\xFF\xFF\xFF");     //回到屏幕初始界面
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
    LoRaPrintf("LoRa初始化成功");
    HAL_Delay(500);
    
    DSL.Start = 100;
    Usart3Printf("t0.txt=\"LoRa初始化成功\"\xFF\xFF\xFFj0.val=%d\xFF\xFF\xFF", DSL.Start);
    
    HAL_Delay(700);
}


void Init_Progress(unsigned int Progress)
{
    DSL.Start += Progress;
    
    Usart3Printf("j0.val=%d\xFF\xFF\xFF", DSL.Start);
    
    HAL_Delay(1000);
}





