#include "Usart_dsl.h"
#include "stdio.h"
#include "string.h"
#include "usart.h"
#include <stdarg.h>
#include "ESP8266.h"
#include "cmsis_os.h"
#include "cJSON.h"      // 请确保 cJSON 库的头文件正确引入
#include <stdlib.h>

extern osSemaphoreId_t HMI_BinarySemHandle;     //串口屏二值信号量
extern osSemaphoreId_t WiFi_BinarySemHandle;    //WiFi二值信号量

//uint8_t UartTxBuf[255];      //定义串口数据发送缓存数组

//串口1ESP8266，串口2LoRa，串口3串口屏

char jsonBuffer[255]; // 用于存储提取的 JSON 数据

uint8_t Rx1Data;
//uint8_t Uart1_RxCnt = 0;      //定义串口1接收缓冲计数
//uint8_t Rx1Buff[255];         //定义串口1数据接收缓存数组
uint8_t Uart1TxBuf[255];      //定义串口1数据发送缓存数组

uint8_t Rx2Data;
uint8_t Uart2_RxCnt = 0;      //定义串口2接收缓冲计数
uint8_t Rx2Buff[255];         //定义串口2数据接收缓存数组
uint8_t Uart2TxBuf[255];      //定义串口2数据发送缓存数组

uint8_t Rx3Data;
uint8_t Uart3_RxCnt = 0;      //定义串口3接收缓冲计数
uint8_t Rx3Buff[255];         //定义串口3数据接收缓存数组
uint8_t Uart3TxBuf[255];      //定义串口3数据发送缓存数组

// 定义一个枚举类型来表示 JSON 数据的状态
typedef enum {
    JSON_IDLE,      // 等待接收 JSON 数据的起始 '{'
    JSON_RECEIVING, // 正在接收 JSON 数据
    JSON_COMPLETE   // 已接收到完整的 JSON 数据
} JSON_State;

// 定义一个变量来表示 JSON 数据的状态
JSON_State jsonState = JSON_IDLE;

int fputc(int ch,FILE *f)
{
    //采用轮询方式发送1字节数据，超时时间设置为无限等待
    HAL_UART_Transmit(&huart2,(uint8_t *)&ch,1,HAL_MAX_DELAY);
    return ch;
}
int fgetc(FILE *f)
{
    uint8_t ch;
    // 采用轮询方式接收 1字节数据，超时时间设置为无限等待
    HAL_UART_Receive( &huart2,(uint8_t*)&ch,1, HAL_MAX_DELAY );
    return ch;
}

//==============================================================================
// @函数: void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle)
// @描述: 串口中断回调函数
// @参数: huart
// @返回: None
// @时间: 2023.10.25
//==============================================================================
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    UNUSED(huart);
    if(huart->Instance == huart1.Instance)
    {
        if(ESP8266_struct.ESP_cnt >= sizeof(ESP8266_struct.ESP_usartbuf))
            ESP8266_struct.ESP_cnt = 0; //防止串口被刷爆
        ESP8266_struct.ESP_usartbuf[ESP8266_struct.ESP_cnt++] = Rx1Data;
        
        // 如果接收到 JSON 数据的结束标记 '}'，则释放信号量
        if (Rx1Data == '}') {
            osSemaphoreRelease(WiFi_BinarySemHandle); // 释放信号量，通知任务有数据到达
        }

        // 再次启动接收中断以接收下一个字符
        HAL_UART_Receive_IT(&huart1, (uint8_t *)&Rx1Data, 1);
    }
    if(huart->Instance == huart2.Instance)
    {
        if(Uart2_RxCnt >= 255)  //溢出判断
        {
            Uart2_RxCnt = 0;
            memset(Rx2Buff,0x00,sizeof(Rx2Buff));
            HAL_UART_Transmit(&huart2, (uint8_t *)"数据溢出", 10,0xFFFF);
        }
        else
        {
            Rx2Buff[Uart2_RxCnt++] = Rx2Data;   //接收数据转存
            if((Rx2Buff[Uart2_RxCnt-1] == 0x0A)&&(Rx2Buff[Uart2_RxCnt-2] == 0x0D)) //判断结束位/r/n
            {
                Rx2Buff[Uart2_RxCnt-2] = 0x00;  //去除结束位/r/n
                
                Uart2_RxCnt = 0;
                osSemaphoreRelease(WiFi_BinarySemHandle);   //信号量的计数值增加1
            }
        }
        HAL_UART_Receive_IT(&huart2, (uint8_t *)&Rx2Data, 1);   //再开启接收中断
    }
    if(huart->Instance == huart3.Instance)
    {
        if(Uart3_RxCnt >= 255)  //溢出判断
        {
            Uart3_RxCnt = 0;
            memset(Rx3Buff,0x00,sizeof(Rx3Buff));
            HAL_UART_Transmit(&huart3, (uint8_t *)"数据溢出", 10, 0xFFFF);
        }
        else
        {
            Rx3Buff[Uart3_RxCnt++] = Rx3Data;   //接收数据转存
            if((Rx3Buff[Uart3_RxCnt-1] == 0x0A)&&(Rx3Buff[Uart3_RxCnt-2] == 0x0D)) //判断结束位/r/n
            {
                Rx3Buff[Uart3_RxCnt-2] = 0x00;
                Uart3_RxCnt = 0;
                osSemaphoreRelease(HMI_BinarySemHandle);
//                memset(Rx3Buff,0x00,sizeof(Rx3Buff)); //清空数组
            }
        }
        HAL_UART_Receive_IT(&huart3, (uint8_t *)&Rx3Data, 1);   //再开启接收中断
    }
}

////==============================================================================
//// @函数: void UsartPrintf(UART_HandleTypeDef *huart,const char *format,...)
//// @描述: 串口的Printf重定义 
//// @参数: None
//// @返回: None
//// @时间: 2023.10.25
////==============================================================================
//void UsartPrintf(UART_HandleTypeDef *huart,const char *format,...) 
//{
//    uint16_t len;
//    va_list args;
//    va_start(args,format);
//    len = vsnprintf((char*)UartTxBuf,sizeof(UartTxBuf)+1,(char*)format,args);
//    va_end(args);

//    HAL_UART_Transmit(huart, UartTxBuf,len,0x0FFF);
////    HAL_UART_Transmit_DMA(&USARTx, UartTxBuf, len);
//}

//==============================================================================
// @函数: void Usart1Printf(const char *format,...)
// @描述: 串口1的Printf重定义 （不推荐在使用DMA的时候按照传统的方式进行重定义）
// @参数: None
// @返回: None
// @时间: 2023.10.25
//==============================================================================
void Usart1Printf(const char *format,...) 
{
    uint16_t len;
    va_list args;
    va_start(args,format);
    len = vsnprintf((char*)Uart1TxBuf,sizeof(Uart1TxBuf)+1,(char*)format,args);
    va_end(args);
    HAL_UART_Transmit(&huart1, Uart1TxBuf, len,0xffff);
}

//==============================================================================
// @函数: void Usart2Printf(const char *format,...)
// @描述: 串口2的Printf重定义 （不推荐在使用DMA的时候按照传统的方式进行重定义）
// @参数: None
// @返回: None
// @时间: 2023.10.25
//==============================================================================
void Usart2Printf(const char *format,...) 
{
    uint16_t len;
    va_list args;
    va_start(args,format);
    len = vsnprintf((char*)Uart2TxBuf,sizeof(Uart2TxBuf)+1,(char*)format,args);
    va_end(args);
    HAL_UART_Transmit(&huart2, Uart2TxBuf, len,0xffff);
}

//==============================================================================
// @函数: void Usart2Printf(const char *format,...)
// @描述: 串口2的Printf重定义 （不推荐在使用DMA的时候按照传统的方式进行重定义）
// @参数: None
// @返回: None
// @时间: 2023.10.25
//==============================================================================
void Usart3Printf(const char *format,...) 
{
    uint16_t len;
    va_list args;
    va_start(args,format);
    len = vsnprintf((char*)Uart3TxBuf,sizeof(Uart3TxBuf)+1,(char*)format,args);
    va_end(args);
    HAL_UART_Transmit(&huart3, Uart3TxBuf, len,0xffff);
}

//==============================================================================
// @函数: void HMI_Handle(void)
// @描述: 处理HMI发送的数据
// @参数: None
// @返回: None
// @时间: 2024.3.12
//==============================================================================
void HMI_Handle(void)
{
    printf("HMIBuff = \"%s\"\r\n",Rx3Buff); // 使用双引号包裹字符串，方便观察是否有额外的字符
    
    // 检查 Rx3Buff 中的内容是否为空
    if (strcmp((const char*)Rx3Buff, "") == 0)
    {
//        printf("无数据\n");
    }
    // 检查 Rx3Buff 中的内容是否为 "close"
    else if (strcmp((const char*)Rx3Buff, "close") == 0)
    {
        printf("关\n");
    }
    // 检查 Rx3Buff 中的内容是否为 "open"
    else if (strcmp((const char*)Rx3Buff, "open") == 0)
    {
        printf("开\n");
    }
    // 如果 Rx3Buff 不是以上任何一种情况，打印 "未知数据"
    else
    {
    //        printf("未知数据\n");
    }
    
    // 处理完数据后清空数组
    memset(Rx3Buff, 0x00, sizeof(Rx3Buff));
}


//==============================================================================
// @函数: WiFi_Handle
// @描述: 处理TCP服务器发送的数据
// @参数: None
// @返回: None
// @时间: 2024.3.12
//==============================================================================
void WiFi_Handle(void)
{
    // 打印接收到的数据
    printf("Received data: %s\r\n", ESP8266_struct.ESP_usartbuf);
    
    // 在需要处理 ESP8266 数据的地方调用提取函数
    extractJsonData((const char*)ESP8266_struct.ESP_usartbuf, jsonBuffer);
    
   // 打印接收到的 JSON 数据
    printf("Received JSON data: %s\r\n", jsonBuffer);
    
    
    // 解析 JSON 数据
    cJSON *root = cJSON_Parse((const char*)jsonBuffer);
    if (root != NULL) 
    {
        // 打印解析出的 JSON 数据
        char *parsed_json = cJSON_Print(root);
        printf("Parsed JSON data: %s\r\n", parsed_json);
        free(parsed_json);

        // 提取 "motor" 字段的值
        cJSON *motorJson = cJSON_GetObjectItem(root, "motor");
        if (motorJson != NULL && motorJson->type == cJSON_Number) 
        {
            // 解析 "motor" 字段的值
            int motorValue = motorJson->valueint;
            if (motorValue == 0) 
            {
                // "motor" 字段的值为 0，打印 "关"
                printf("Motor status: 关\r\n");
            } 
            else if (motorValue == 1) 
            {
                // "motor" 字段的值为 1，打印 "开"
                printf("Motor status: 开\r\n");
            }
        }

        // 清理 cJSON 结构体
        cJSON_Delete(root);
    }
    
    // 清空接收缓冲区，准备接收下一个 JSON 数据包
    memset(ESP8266_struct.ESP_usartbuf, 0x00, sizeof(ESP8266_struct.ESP_usartbuf));
    ESP8266_struct.ESP_cnt = 0;
}

//==============================================================================
// @函数: void extractJsonData(const char* input, char* output)
// @描述: 从输入字符串中提取大括号内的数据
// @参数: input - 输入字符串，包含大括号 {} 的数据
//        output - 输出缓冲区，用于存储提取的数据
// @返回: None
// @时间: 2024.3.12
//==============================================================================
void extractJsonData(const char* input, char* output)
{
    // 找到第一个 '{'
    const char* start = strchr(input, '{');
    if (start == NULL)
    {
        return; // 没有找到 '{'，退出
    }
    
    // 找到最后一个 '}'
    const char* end = strrchr(input, '}');
    if (end == NULL)
    {
        return; // 没有找到 '}'，退出
    }

    // 复制 { 和 } 中的数据到输出缓冲区
    int length = end - start + 1; // 包括 '{' 和 '}'
    strncpy(output, start, length);
    output[length] = '\0'; // 添加字符串结束符
}





