#include "All_Init.h"
#include "Usart_dsl.h"
#include "stdio.h"
#include "string.h"
#include "usart.h"
#include <stdarg.h>
#include "ESP8266.h"
#include "cmsis_os.h"
#include "cJSON.h"      // 确保cJSON库的头文件正确引入  Heap_Size EQU 0xC00
#include <stdlib.h>
#include <string.h>
#include <RC522.h>
#include <YFS401.h>

extern osSemaphoreId_t HMI_BinarySemHandle;     //串口屏二值信号量
extern osSemaphoreId_t WiFi_BinarySemHandle;    //WiFi二值信号量


//串口1ESP8266，串口2LoRa，串口3串口屏

char jsonBuffer[255];           // 用于存储提取的 JSON 数据
unsigned char HMI_Flag = 0;     // 标记是否在串口屏数据包内


uint8_t Rx1Data;
//uint8_t Uart1_RxCnt = 0;      //定义串口1接收缓冲计数
//uint8_t Rx1Buff[255];         //定义串口1数据接收缓存数组
uint8_t Uart1TxBuf[255];      //定义串口1数据发送缓存数组

uint8_t Rx2Data;
uint8_t Uart2_RxCnt = 0;      //定义串口2接收缓冲计数
uint8_t Rx2Buff[64];         //定义串口2数据接收缓存数组
uint8_t Uart2TxBuf[255];      //定义串口2数据发送缓存数组

uint8_t Rx3Data;
uint8_t Uart3_RxCnt = 0;      //定义串口3接收缓冲计数
uint8_t Rx3Buff[255];         //定义串口3数据接收缓存数组
uint8_t Uart3TxBuf[255];      //定义串口3数据发送缓存数组

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
//    UNUSED(huart);
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
// @描述: 串口3的Printf重定义 （不推荐在使用DMA的时候按照传统的方式进行重定义）
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
// @函数: void LoRaPrintf(const char *format, ...)
// @描述: LoRa串口发送函数（包头为@，包尾为\r\n）
// @参数: None
// @返回: None
// @时间: 2024.3.22
//==============================================================================
void LoRaPrintf(const char *format, ...) 
{
    char buffer[256]; // 假设最大长度为256个字符
    va_list args;
    va_start(args, format);
    
    // 在参数前加上@字符
    buffer[0] = '@';
    vsnprintf(&buffer[1], sizeof(buffer) - 3, format, args); // -3是为了留出@字符和\r\n的位置
    va_end(args);
    
    // 在尾部加上\r\n
    int len = strlen(buffer);
    buffer[len] = '\r';
    buffer[len + 1] = '\n';
    buffer[len + 2] = '\0';

    HAL_UART_Transmit(&huart2, (uint8_t *)buffer, strlen(buffer), 0xFFFF);
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
    LoRaPrintf("HMIBuff = \"%s\"",Rx3Buff); // 使用双引号包裹字符串，方便观察是否有额外的字符
    
    
    // 检查 Rx3Buff 中的内容是否为空
    if (strcmp((const char*)Rx3Buff, "") == 0)
    {
//        printf("无数据\n");
    }
    // 检查 Rx3Buff 中的内容是否为 "close"
    else if (strcmp((const char*)Rx3Buff, "close") == 0)
    {
        if(DSL.Mode == 1 && DSL.Flow == true)   //处于用户操作模式，电机开启状态
        {
            DSL.Flow = false;
            YFS401_Stop();      //关闭YFS401水流器
            HAL_Delay(100);     //延时避免卡死
            HAL_GPIO_WritePin(relay_GPIO_Port,relay_Pin,GPIO_PIN_SET);      //继电器关
            HAL_Delay(100);     //延时避免卡死
            
//            printf("UserID is %s",UserID);
            char amount_message[50];
            sprintf(amount_message,"{\"dispenser_id\":%d,\"card\":\"%s\",\"amount\":%.1lf}",DSL.ID,UserID,golbal_flow.acculat);     //结算余额
            ESP8266_SendData(amount_message);
        }
    }
    // 检查 Rx3Buff 中的内容是否为 "open"
    else if (strcmp((const char*)Rx3Buff, "open") == 0)
    {
        if(DSL.Mode == 1 && DSL.Flow == false)  //处于用户操作模式，电机关闭状态
        {
//            printf("开\n");
            DSL.Flow = true;
            YFS401_Start();     //启动YFS401水流器
            HAL_GPIO_WritePin(relay_GPIO_Port,relay_Pin,GPIO_PIN_RESET);      //继电器开
        }
    }
    // 如果 Rx3Buff 不是以上任何一种情况，打印 "未知数据"
    else
    {
        LoRaPrintf("串口屏未知数据");
    }
    
    // 处理完数据后清空数组
    memset(Rx3Buff, 0x00, sizeof(Rx3Buff));
}


//==============================================================================
// @函数: void WiFi_Handle(void)
// @描述: 解析TCP服务器发送Json数据，并执行相应功能
// @参数: None
// @返回: None
// @时间: 2024.3.12
//==============================================================================
void WiFi_Handle(void)
{
    // 打印接收到的数据
//    printf("Received data: %s\r\n", ESP8266_struct.ESP_usartbuf);

    // 在需要处理 ESP8266 数据的地方调用提取函数
    extractJsonData((const char*)ESP8266_struct.ESP_usartbuf, jsonBuffer);
    
//    printf("Received JSON data: %s\r\n", jsonBuffer);    // 打印接收到的 JSON 数据
    
    // 解析 JSON 数据
    cJSON *root = cJSON_Parse((const char*)jsonBuffer);
    if (root != NULL) 
    {
        // 提取 "motor" 字段的值
        cJSON *motorJson = cJSON_GetObjectItem(root, "motor");
        if (motorJson != NULL && motorJson->type == cJSON_Number) 
        {
            // 解析 "motor" 字段的值
            int motorValue = motorJson->valueint;
            if (motorValue == 0) 
            {
                // "motor" 字段的值为 0，打印 "关"
                LoRaPrintf("Motor status: 关");
                
                DSL.Flow = false;
                YFS401_Stop();      //关闭YFS401水流器
                HAL_Delay(100);     //延时避免卡死
                HAL_GPIO_WritePin(relay_GPIO_Port,relay_Pin,GPIO_PIN_SET);      //继电器关
                HAL_Delay(100);     //延时避免卡死
                
                char amount_message[50];
                sprintf(amount_message,"{\"dispenser_id\":%d,\"APP\":\"%d\",\"amount\":%.1lf}",DSL.ID,true,golbal_flow.acculat);     //结算余额
                ESP8266_SendData(amount_message);
                
            } 
            else if (motorValue == 1) 
            {
                // "motor" 字段的值为 1，打印 "开"
                LoRaPrintf("Motor status: 开");
                
                DSL.Flow = true;
                YFS401_Start();     //启动YFS401水流器
                HAL_GPIO_WritePin(relay_GPIO_Port,relay_Pin,GPIO_PIN_RESET);      //继电器开
                
            }
        }
        else
        {
            // 提取 "balance" 和 "user" 字段的值
            cJSON *balanceJson = cJSON_GetObjectItem(root, "balance");
            cJSON *userJson = cJSON_GetObjectItem(root, "user");
            if (balanceJson != NULL && balanceJson->type == cJSON_Number &&
                userJson != NULL && userJson->type == cJSON_String) 
            {
                // 解析 "balance" 和 "user" 字段的值
                int balanceValue = balanceJson->valueint;
                const char* userValue = userJson->valuestring;

                // 打印 "balance" 和 "user" 字段的值
//                printf("Balance: %d, User: %s\r\n", balanceValue, userValue);
                
                if(strcmp((const char*)userValue, "None") == 0)
                {
                    LoRaPrintf("Unknown user");
                }
                else
                {
                    if(DSL.Mode == 0)
                    {
                        DSL.Mode = 1;   //设置进入用户模式
                        LoRaPrintf("Enter user operation mode");
                        strcpy(UserID, CardID);     //将用户卡复制，在后面进行验证
                    }
                    
                    Usart3Printf("page 2\xff\xff\xfft3.txt=\"%s\"\xff\xff\xfft4.txt=\"%d\"\xff\xff\xff",userValue,balanceValue);     //进入串口屏用户操作页面

                }

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





