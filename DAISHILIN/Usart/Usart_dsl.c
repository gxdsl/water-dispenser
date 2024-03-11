#include "Usart_dsl.h"
#include "stdio.h"
#include "string.h"
#include "usart.h"
#include <stdarg.h>
#include "ESP8266.h"


//uint8_t UartTxBuf[255];      //���崮�����ݷ��ͻ�������

//����1ESP8266������2LoRa������3������

uint8_t Rx1Data;
uint8_t Uart1_RxCnt = 0;      //���崮��1���ջ������
uint8_t Rx1Buff[255];         //���崮��1���ݽ��ջ�������
uint8_t Uart1TxBuf[255];      //���崮��1���ݷ��ͻ�������

uint8_t Rx2Data;
uint8_t Uart2_RxCnt = 0;      //���崮��2���ջ������
uint8_t Rx2Buff[255];         //���崮��2���ݽ��ջ�������
uint8_t Uart2TxBuf[255];      //���崮��2���ݷ��ͻ�������

uint8_t Rx3Data;
uint8_t Uart3_RxCnt = 0;      //���崮��3���ջ������
uint8_t Rx3Buff[255];         //���崮��3���ݽ��ջ�������
uint8_t Uart3TxBuf[255];      //���崮��3���ݷ��ͻ�������

int fputc(int ch,FILE *f)
{
    //������ѯ��ʽ����1�ֽ����ݣ���ʱʱ������Ϊ���޵ȴ�
    HAL_UART_Transmit(&huart2,(uint8_t *)&ch,1,HAL_MAX_DELAY);
    return ch;
}
int fgetc(FILE *f)
{
    uint8_t ch;
    // ������ѯ��ʽ���� 1�ֽ����ݣ���ʱʱ������Ϊ���޵ȴ�
    HAL_UART_Receive( &huart2,(uint8_t*)&ch,1, HAL_MAX_DELAY );
    return ch;
}

//==============================================================================
// @����: void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle)
// @����: �����жϻص�����
// @����: huart
// @����: None
// @ʱ��: 2023.10.25
//==============================================================================
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    UNUSED(huart);
    if(huart->Instance == huart1.Instance)
    {
        if(ESP8266_struct.ESP_cnt >= sizeof(ESP8266_struct.ESP_usartbuf))
                ESP8266_struct.ESP_cnt = 0; //��ֹ���ڱ�ˢ��
        ESP8266_struct.ESP_usartbuf[ESP8266_struct.ESP_cnt++] = Rx1Data;
        HAL_UART_Receive_IT(&huart1, (uint8_t *)&Rx1Data, 1);   //�ٿ��������ж�
    }
    if(huart->Instance == huart2.Instance)
    {
        if(Uart2_RxCnt >= 255)  //����ж�
        {
            Uart2_RxCnt = 0;
            memset(Rx2Buff,0x00,sizeof(Rx2Buff));
            HAL_UART_Transmit(&huart2, (uint8_t *)"�������", 10,0xFFFF);
        }
        else
        {
            Rx2Buff[Uart2_RxCnt++] = Rx2Data;   //��������ת��
            if((Rx2Buff[Uart2_RxCnt-1] == 0x0A)&&(Rx2Buff[Uart2_RxCnt-2] == 0x0D)) //�жϽ���λ/r/n
            {
                HAL_UART_Transmit(&huart2, (uint8_t *)&Rx2Buff, Uart2_RxCnt,0xFFFF); //���յ�����Ϣ���ͳ�ȥ
                Uart2_RxCnt = 0;
                memset(Rx2Buff,0x00,sizeof(Rx2Buff)); //�������
            }
        }
        HAL_UART_Receive_IT(&huart2, (uint8_t *)&Rx2Data, 1);   //�ٿ��������ж�
    }
}

////==============================================================================
//// @����: void UsartPrintf(UART_HandleTypeDef *huart,const char *format,...)
//// @����: ���ڵ�Printf�ض��� 
//// @����: None
//// @����: None
//// @ʱ��: 2023.10.25
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
// @����: void Usart1Printf(const char *format,...)
// @����: ����1��Printf�ض��� �����Ƽ���ʹ��DMA��ʱ���մ�ͳ�ķ�ʽ�����ض��壩
// @����: None
// @����: None
// @ʱ��: 2023.10.25
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
// @����: void Usart2Printf(const char *format,...)
// @����: ����2��Printf�ض��� �����Ƽ���ʹ��DMA��ʱ���մ�ͳ�ķ�ʽ�����ض��壩
// @����: None
// @����: None
// @ʱ��: 2023.10.25
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
// @����: void Usart2Printf(const char *format,...)
// @����: ����2��Printf�ض��� �����Ƽ���ʹ��DMA��ʱ���մ�ͳ�ķ�ʽ�����ض��壩
// @����: None
// @����: None
// @ʱ��: 2023.10.25
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




