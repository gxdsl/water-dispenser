#include "All_Init.h"
#include "Usart_dsl.h"
#include "stdio.h"
#include "string.h"
#include "usart.h"
#include <stdarg.h>
#include "ESP8266.h"
#include "cmsis_os.h"
#include "cJSON.h"      // ȷ��cJSON���ͷ�ļ���ȷ����  Heap_Size EQU 0xC00
#include <stdlib.h>
#include <string.h>
#include <RC522.h>
#include <YFS401.h>

extern osSemaphoreId_t HMI_BinarySemHandle;     //��������ֵ�ź���
extern osSemaphoreId_t WiFi_BinarySemHandle;    //WiFi��ֵ�ź���


//����1ESP8266������2LoRa������3������

char jsonBuffer[255];           // ���ڴ洢��ȡ�� JSON ����
unsigned char HMI_Flag = 0;     // ����Ƿ��ڴ��������ݰ���


uint8_t Rx1Data;
//uint8_t Uart1_RxCnt = 0;      //���崮��1���ջ������
//uint8_t Rx1Buff[255];         //���崮��1���ݽ��ջ�������
uint8_t Uart1TxBuf[255];      //���崮��1���ݷ��ͻ�������

uint8_t Rx2Data;
uint8_t Uart2_RxCnt = 0;      //���崮��2���ջ������
uint8_t Rx2Buff[64];         //���崮��2���ݽ��ջ�������
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
//    UNUSED(huart);
    if(huart->Instance == huart1.Instance)
    {
        if(ESP8266_struct.ESP_cnt >= sizeof(ESP8266_struct.ESP_usartbuf))
            ESP8266_struct.ESP_cnt = 0; //��ֹ���ڱ�ˢ��
        ESP8266_struct.ESP_usartbuf[ESP8266_struct.ESP_cnt++] = Rx1Data;
        
        // ������յ� JSON ���ݵĽ������ '}'�����ͷ��ź���
        if (Rx1Data == '}') {
            osSemaphoreRelease(WiFi_BinarySemHandle); // �ͷ��ź�����֪ͨ���������ݵ���
        }

        // �ٴ����������ж��Խ�����һ���ַ�
        HAL_UART_Receive_IT(&huart1, (uint8_t *)&Rx1Data, 1);
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
                Rx2Buff[Uart2_RxCnt-2] = 0x00;  //ȥ������λ/r/n
                
                Uart2_RxCnt = 0;
                osSemaphoreRelease(WiFi_BinarySemHandle);   //�ź����ļ���ֵ����1
            }
        }

        HAL_UART_Receive_IT(&huart2, (uint8_t *)&Rx2Data, 1);   //�ٿ��������ж�
    }
    if(huart->Instance == huart3.Instance)
    {
        if(Uart3_RxCnt >= 255)  //����ж�
        {
            Uart3_RxCnt = 0;
            memset(Rx3Buff,0x00,sizeof(Rx3Buff));
            HAL_UART_Transmit(&huart3, (uint8_t *)"�������", 10, 0xFFFF);
        }
        else
        {
            Rx3Buff[Uart3_RxCnt++] = Rx3Data;   //��������ת��
            if((Rx3Buff[Uart3_RxCnt-1] == 0x0A)&&(Rx3Buff[Uart3_RxCnt-2] == 0x0D)) //�жϽ���λ/r/n
            {
                Rx3Buff[Uart3_RxCnt-2] = 0x00;
                Uart3_RxCnt = 0;
                osSemaphoreRelease(HMI_BinarySemHandle);
//                memset(Rx3Buff,0x00,sizeof(Rx3Buff)); //�������
            }
        }
        HAL_UART_Receive_IT(&huart3, (uint8_t *)&Rx3Data, 1);   //�ٿ��������ж�
    }
}


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
// @����: ����3��Printf�ض��� �����Ƽ���ʹ��DMA��ʱ���մ�ͳ�ķ�ʽ�����ض��壩
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


//==============================================================================
// @����: void LoRaPrintf(const char *format, ...)
// @����: LoRa���ڷ��ͺ�������ͷΪ@����βΪ\r\n��
// @����: None
// @����: None
// @ʱ��: 2024.3.22
//==============================================================================
void LoRaPrintf(const char *format, ...) 
{
    char buffer[256]; // ������󳤶�Ϊ256���ַ�
    va_list args;
    va_start(args, format);
    
    // �ڲ���ǰ����@�ַ�
    buffer[0] = '@';
    vsnprintf(&buffer[1], sizeof(buffer) - 3, format, args); // -3��Ϊ������@�ַ���\r\n��λ��
    va_end(args);
    
    // ��β������\r\n
    int len = strlen(buffer);
    buffer[len] = '\r';
    buffer[len + 1] = '\n';
    buffer[len + 2] = '\0';

    HAL_UART_Transmit(&huart2, (uint8_t *)buffer, strlen(buffer), 0xFFFF);
}


//==============================================================================
// @����: void HMI_Handle(void)
// @����: ����HMI���͵�����
// @����: None
// @����: None
// @ʱ��: 2024.3.12
//==============================================================================
void HMI_Handle(void)
{
    LoRaPrintf("HMIBuff = \"%s\"",Rx3Buff); // ʹ��˫���Ű����ַ���������۲��Ƿ��ж�����ַ�
    
    
    // ��� Rx3Buff �е������Ƿ�Ϊ��
    if (strcmp((const char*)Rx3Buff, "") == 0)
    {
//        printf("������\n");
    }
    // ��� Rx3Buff �е������Ƿ�Ϊ "close"
    else if (strcmp((const char*)Rx3Buff, "close") == 0)
    {
        if(DSL.Mode == 1 && DSL.Flow == true)   //�����û�����ģʽ���������״̬
        {
            DSL.Flow = false;
            YFS401_Stop();      //�ر�YFS401ˮ����
            HAL_Delay(100);     //��ʱ���⿨��
            HAL_GPIO_WritePin(relay_GPIO_Port,relay_Pin,GPIO_PIN_SET);      //�̵�����
            HAL_Delay(100);     //��ʱ���⿨��
            
//            printf("UserID is %s",UserID);
            char amount_message[50];
            sprintf(amount_message,"{\"dispenser_id\":%d,\"card\":\"%s\",\"amount\":%.1lf}",DSL.ID,UserID,golbal_flow.acculat);     //�������
            ESP8266_SendData(amount_message);
        }
    }
    // ��� Rx3Buff �е������Ƿ�Ϊ "open"
    else if (strcmp((const char*)Rx3Buff, "open") == 0)
    {
        if(DSL.Mode == 1 && DSL.Flow == false)  //�����û�����ģʽ������ر�״̬
        {
//            printf("��\n");
            DSL.Flow = true;
            YFS401_Start();     //����YFS401ˮ����
            HAL_GPIO_WritePin(relay_GPIO_Port,relay_Pin,GPIO_PIN_RESET);      //�̵�����
        }
    }
    // ��� Rx3Buff ���������κ�һ���������ӡ "δ֪����"
    else
    {
        LoRaPrintf("������δ֪����");
    }
    
    // ���������ݺ��������
    memset(Rx3Buff, 0x00, sizeof(Rx3Buff));
}


//==============================================================================
// @����: void WiFi_Handle(void)
// @����: ����TCP����������Json���ݣ���ִ����Ӧ����
// @����: None
// @����: None
// @ʱ��: 2024.3.12
//==============================================================================
void WiFi_Handle(void)
{
    // ��ӡ���յ�������
//    printf("Received data: %s\r\n", ESP8266_struct.ESP_usartbuf);

    // ����Ҫ���� ESP8266 ���ݵĵط�������ȡ����
    extractJsonData((const char*)ESP8266_struct.ESP_usartbuf, jsonBuffer);
    
//    printf("Received JSON data: %s\r\n", jsonBuffer);    // ��ӡ���յ��� JSON ����
    
    // ���� JSON ����
    cJSON *root = cJSON_Parse((const char*)jsonBuffer);
    if (root != NULL) 
    {
        // ��ȡ "motor" �ֶε�ֵ
        cJSON *motorJson = cJSON_GetObjectItem(root, "motor");
        if (motorJson != NULL && motorJson->type == cJSON_Number) 
        {
            // ���� "motor" �ֶε�ֵ
            int motorValue = motorJson->valueint;
            if (motorValue == 0) 
            {
                // "motor" �ֶε�ֵΪ 0����ӡ "��"
                LoRaPrintf("Motor status: ��");
                
                DSL.Flow = false;
                YFS401_Stop();      //�ر�YFS401ˮ����
                HAL_Delay(100);     //��ʱ���⿨��
                HAL_GPIO_WritePin(relay_GPIO_Port,relay_Pin,GPIO_PIN_SET);      //�̵�����
                HAL_Delay(100);     //��ʱ���⿨��
                
                char amount_message[50];
                sprintf(amount_message,"{\"dispenser_id\":%d,\"APP\":\"%d\",\"amount\":%.1lf}",DSL.ID,true,golbal_flow.acculat);     //�������
                ESP8266_SendData(amount_message);
                
            } 
            else if (motorValue == 1) 
            {
                // "motor" �ֶε�ֵΪ 1����ӡ "��"
                LoRaPrintf("Motor status: ��");
                
                DSL.Flow = true;
                YFS401_Start();     //����YFS401ˮ����
                HAL_GPIO_WritePin(relay_GPIO_Port,relay_Pin,GPIO_PIN_RESET);      //�̵�����
                
            }
        }
        else
        {
            // ��ȡ "balance" �� "user" �ֶε�ֵ
            cJSON *balanceJson = cJSON_GetObjectItem(root, "balance");
            cJSON *userJson = cJSON_GetObjectItem(root, "user");
            if (balanceJson != NULL && balanceJson->type == cJSON_Number &&
                userJson != NULL && userJson->type == cJSON_String) 
            {
                // ���� "balance" �� "user" �ֶε�ֵ
                int balanceValue = balanceJson->valueint;
                const char* userValue = userJson->valuestring;

                // ��ӡ "balance" �� "user" �ֶε�ֵ
//                printf("Balance: %d, User: %s\r\n", balanceValue, userValue);
                
                if(strcmp((const char*)userValue, "None") == 0)
                {
                    LoRaPrintf("Unknown user");
                }
                else
                {
                    if(DSL.Mode == 0)
                    {
                        DSL.Mode = 1;   //���ý����û�ģʽ
                        LoRaPrintf("Enter user operation mode");
                        strcpy(UserID, CardID);     //���û������ƣ��ں��������֤
                    }
                    
                    Usart3Printf("page 2\xff\xff\xfft3.txt=\"%s\"\xff\xff\xfft4.txt=\"%d\"\xff\xff\xff",userValue,balanceValue);     //���봮�����û�����ҳ��

                }

            }
        }
        
        // ���� cJSON �ṹ��
        cJSON_Delete(root);
    }
    
    // ��ս��ջ�������׼��������һ�� JSON ���ݰ�
    memset(ESP8266_struct.ESP_usartbuf, 0x00, sizeof(ESP8266_struct.ESP_usartbuf));
    ESP8266_struct.ESP_cnt = 0;
}


//==============================================================================
// @����: void extractJsonData(const char* input, char* output)
// @����: �������ַ�������ȡ�������ڵ�����
// @����: input - �����ַ��������������� {} ������
//        output - ��������������ڴ洢��ȡ������
// @����: None
// @ʱ��: 2024.3.12
//==============================================================================
void extractJsonData(const char* input, char* output)
{
    // �ҵ���һ�� '{'
    const char* start = strchr(input, '{');
    if (start == NULL)
    {
        return; // û���ҵ� '{'���˳�
    }
    
    // �ҵ����һ�� '}'
    const char* end = strrchr(input, '}');
    if (end == NULL)
    {
        return; // û���ҵ� '}'���˳�
    }

    // ���� { �� } �е����ݵ����������
    int length = end - start + 1; // ���� '{' �� '}'
    strncpy(output, start, length);
    output[length] = '\0'; // ����ַ���������
}





