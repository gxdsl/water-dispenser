#include "Usart_dsl.h"
#include "stdio.h"
#include "string.h"
#include "usart.h"
#include <stdarg.h>
#include "ESP8266.h"


//uint8_t UartTxBuf[255];      //定义串口数据发送缓存数组

//串口1ESP8266，串口2LoRa，串口3串口屏

uint8_t Rx1Data;
uint8_t Uart1_RxCnt = 0;      //定义串口1接收缓冲计数
uint8_t Rx1Buff[255];         //定义串口1数据接收缓存数组
uint8_t Uart1TxBuf[255];      //定义串口1数据发送缓存数组

uint8_t Rx2Data;
uint8_t Uart2_RxCnt = 0;      //定义串口2接收缓冲计数
uint8_t Rx2Buff[255];         //定义串口2数据接收缓存数组
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
    UNUSED(huart);
    if(huart->Instance == huart1.Instance)
    {
        if(ESP8266_struct.ESP_cnt >= sizeof(ESP8266_struct.ESP_usartbuf))
                ESP8266_struct.ESP_cnt = 0; //防止串口被刷爆
        ESP8266_struct.ESP_usartbuf[ESP8266_struct.ESP_cnt++] = Rx1Data;
        HAL_UART_Receive_IT(&huart1, (uint8_t *)&Rx1Data, 1);   //再开启接收中断
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
                HAL_UART_Transmit(&huart2, (uint8_t *)&Rx2Buff, Uart2_RxCnt,0xFFFF); //将收到的信息发送出去
                Uart2_RxCnt = 0;
                memset(Rx2Buff,0x00,sizeof(Rx2Buff)); //清空数组
            }
        }
        HAL_UART_Receive_IT(&huart2, (uint8_t *)&Rx2Data, 1);   //再开启接收中断
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




