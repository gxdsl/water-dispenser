//Ӳ������
#include "stm32f4xx.h"                  // Device header
#include "ESP8266.h"
#include "usart.h"
#include "usart_dsl.h"
#include "All_Init.h"

//C��
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cmsis_os.h"

//ȫ�ֱ���
ESP8266_STRUCT ESP8266_struct;


/*
************************************************************
*	�������ƣ�	ESP8266_Init
*	�������ܣ�	ESP8266��ʼ��
*	��ڲ�����	
*	���ز�����	��
*	˵����		
************************************************************
*/
void ESP8266_Init(void)
{
	ESP8266_Clear();
	
	while(ESP8266_SendCMD("AT\r\n", "OK"))
		HAL_Delay(500);
//	UsartPrintf(&ESP_DebugUsart,"AT��Ӧ�ɹ�\r\n");
	printf("AT��Ӧ�ɹ�\r\n");
	while(ESP8266_SendCMD("AT+CWMODE=1\r\n", "OK"))
		HAL_Delay(500);
//	UsartPrintf(&ESP_DebugUsart,"AT+CWMODE��Ӧ�ɹ�\r\n"); 
	printf("AT+CWMODE��Ӧ�ɹ�\r\n");
    
	while(ESP8266_SendCMD("AT+CWDHCP=1,1\r\n", "OK"))
		HAL_Delay(500);
//	UsartPrintf(&ESP_DebugUsart,"AT+CWDHCP��Ӧ�ɹ�\r\n");
	printf("AT+CWDHCP��Ӧ�ɹ�\r\n");
    
	while(ESP8266_SendCMD(WIFI_Password, "GOT IP"))
		HAL_Delay(500);
//	UsartPrintf(&ESP_DebugUsart,"AT+CWJAP��Ӧ�ɹ�\r\n");
    printf("AT+CWJAP��Ӧ�ɹ�\r\n");
	
	while(ESP8266_SendCMD(WIFI_IPAdder_IPPort, "CONNECT"));
		HAL_Delay(500);
//	UsartPrintf(&ESP_DebugUsart,"AT+CIPSTART��Ӧ�ɹ�\r\n");
    printf("AT+CIPSTART��Ӧ�ɹ�\r\n");
    
    printf("ESP8266��ʼ���ɹ�\r\n");
    
    DSL.Start +=20;
    Usart3Printf("ESP8266��ʼ���ɹ�\r\n");
    
	HAL_Delay(500);
}

/*
************************************************************
*	�������ƣ�	ESP8266_Clear
*	�������ܣ�	���յ���������
*	��ڲ�����
*	���ز�����	��
*	˵����
************************************************************
*/
void ESP8266_Clear(void)
{
	ESP8266_struct.ESP_cnt=0;
	ESP8266_struct.ESP_cntPre=0;
	memset(ESP8266_struct.ESP_usartbuf,0,sizeof(ESP8266_struct.ESP_usartbuf));
}

/*
************************************************************
*	�������ƣ�	ESP8266_WaitRecive
*	�������ܣ�	�ȴ����ݽ������
*	��ڲ�����	
*	���ز�����	��
*	˵����		
************************************************************
*/
_Bool ESP8266_WaitRecive(void)
{
	if(ESP8266_struct.ESP_cnt == 0) 							//������ռ���Ϊ0 ��˵��û�д��ڽ��������У�����ֱ����������������
		return ESP_REVWAIT;

	if(ESP8266_struct.ESP_cnt == ESP8266_struct.ESP_cntPre)				//�����һ�ε�ֵ�������ͬ����˵���������
	{
		ESP8266_struct.ESP_cnt = 0;							//��0���ռ���
		return ESP_REVOK;								//���ؽ�����ɱ�־
	}

	ESP8266_struct.ESP_cntPre = ESP8266_struct.ESP_cnt;					//��Ϊ��ͬ

	return ESP_REVWAIT;								//���ؽ���δ��ɱ�־
}

/*
************************************************************
*	�������ƣ�	ESP8266_SendCMD
*	�������ܣ�	��������
*	��ڲ�����	
*	���ز�����	��
*	˵����
************************************************************
*/
_Bool ESP8266_SendCMD(char *CMD,char *keyword)
{
	unsigned char Time=100;
	ESP8266_Clear(); //�������

    HAL_UART_Transmit(&ESP_SendUsart, (uint8_t *)CMD, strlen((const char *)CMD),0xffff);//��������
    
	while(Time--)     //ѭ���Ƿ������ɣ�ѭ��ִ�����ٴΣ�ֱ���ɹ�ִ��return 0,�˳���
	{
		if(ESP8266_WaitRecive() == ESP_REVOK)     //����������
		{

			if(strstr((const char *)ESP8266_struct.ESP_usartbuf,keyword)!=NULL)     //��ô����Ƿ��������ؼ���
			{
				ESP8266_Clear(); //�������
				return 0;        //����ֵΪ0���˳�
			}
		}
		HAL_Delay(20);
	}
	return 1;
}

/*
************************************************************
*	�������ƣ�	ESP8266_SendData
*	�������ܣ�	��������
*	��ڲ�����	
*	���ز�����	��
*	˵����
************************************************************
*/
void ESP8266_SendData(char *DATA)
{
	char Buffer[500];  //���ڻ�������
	
	ESP8266_Clear();
	
	sprintf(Buffer,"AT+CIPSEND=%d\r\n",strlen((const char *)DATA));   //�����ݻ��浽Buffer��
    
	if(!ESP8266_SendCMD(Buffer,"OK"))             //�жϼ�⵽���ַ��Ƿ���ȷ
	{
//        HAL_Delay(10);
        HAL_UART_Transmit(&ESP_SendUsart, (uint8_t *)DATA, strlen((const char *)DATA),0xFFFF);//��������
	}
}

void ESP8266_SendData1(char *DATA)
{
	char Buffer[500];  //���ڻ�������
//	static unsigned char conect = 0;
	
	ESP8266_Clear();
	
	sprintf(Buffer,"AT+CIPSEND=%d\r\n",strlen((const char *)DATA));   //�����ݻ��浽Buffer��
	
	if(!ESP8266_SendCMD(Buffer,">"))             //�жϼ�⵽���ַ��Ƿ���ȷ
	{
//		Usart1_SendString(ESP_SendUsart,(unsigned char *)DATA,strlen((const char *)DATA));    //��������
//        Usart1Printf(DATA);
        HAL_UART_Transmit(&ESP_SendUsart, (uint8_t *)DATA, strlen((const char *)DATA),0xFFFF);//��������
	}
	else {
//		conect++;
//		if(conect == 1)
//		{
//			xTaskNotify((TaskHandle_t	)flash_Task_Handler,//��������֪ͨ��������
//      (uint32_t		)flash_EVENT,			//Ҫ�������¼�
//      (eNotifyAction)eSetBits);			//��������ֵ֪ͨ�е�λ
////		 HAL_Delay(3000);
////			while(ESP8266_SendCMD(WIFI_Password, "GOT IP"));
////		 HAL_Delay(200);
////			Usart1_Printf(ESP_DebugUsart,"AT+CWJAP��Ӧ�ɹ�\r\n"); 
//			
//			while(ESP8266_SendCMD(WIFI_IPAdder_IPPort, "CONNECT"));
//		 HAL_Delay(200);
//			Usart1_Printf(ESP_DebugUsart, "AT+CIPSTART��Ӧ�ɹ�\r\n");
//		 HAL_Delay(100);
//
//			flashread_net();
//			conect = 0;	
//		}
	}	
}

//volatile u8 flag_net = 0;
//u8 flag_net1 = 0;
//void TCP_Release(void)
//{
//		char *data = NULL;
//		printf("XXX:%s\r\n",ESP8266_struct.ESP_usartbuf);
//		if((strstr((const char*)ESP8266_struct.ESP_usartbuf,"CLOSED")  != NULL) || (strstr((const char*)ESP8266_struct.ESP_usartbuf,"ERROR")  != NULL) || (strstr((const char*)ESP8266_struct.ESP_usartbuf,"SEND FAIL")  != NULL)){
//////		xTaskNotify((TaskHandle_t	)flash_Task_Handler,//��������֪ͨ��������
//////      (uint32_t		)flash_EVENT,			//Ҫ�������¼�
//////      (eNotifyAction)eSetBits);			//��������ֵ֪ͨ�е�λ
////			//ramwrite_net();
//////		HAL_Delay(3000);		
//////		while(ESP8266_SendCMD(WIFI_Password, "GOT IP")); 		HAL_Delay(200);
//			flag_net = 1;
//			while(ESP8266_SendCMD(WIFI_IPAdder_IPPort, "CONNECT"));
//////			HAL_Delay(200);
////			ramread_net();
//			flag_net = 0;
//			ESP8266_Clear(); //�������
//			Usart1_Printf(ESP_DebugUsart, "AT+CIPSTART��Ӧ�ɹ�\r\n");
//}	
///********************************************************/		
//		if(strstr((const char*)ESP8266_struct.ESP_usartbuf,"PD")  != NULL){		
//		data = strstr((const char*)ESP8266_struct.ESP_usartbuf,"lamp");
//		if(data != NULL)
//		{
//			data += 6;
//			s.lamp = atof(data);
//			if(s.lamp == 1)	GPIOD->BSRRL = GPIO_Pin_7;
//			else if(s.lamp == 0)	GPIOD->BSRRH = GPIO_Pin_7;
//		}
///********************************************************/			
//		data = strstr((const char*)ESP8266_struct.ESP_usartbuf,"LightH");
//		if(data != NULL)
//		{
//			data += 9;
//			t.lighth = atof(data);
//		}			
//		data = strstr((const char*)ESP8266_struct.ESP_usartbuf,"LightL");
//		if(data != NULL)
//		{
//			data += 9;
//			t.lightl = atof(data);
//		}	
///********************************************************/
//		data = strstr((const char*)ESP8266_struct.ESP_usartbuf,"TemH");
//		if(data != NULL)
//		{
//			data += 7;
//			t.TemH = atof(data);
//		}	
//		data = strstr((const char*)ESP8266_struct.ESP_usartbuf,"TemL");
//		if(data != NULL)
//		{
//			data += 7;
//			t.TemL = atof(data);
//		}	
///********************************************************/
//		flashwrite_ele();	
///********************************************************/		
//		ESP8266_Clear(); //�������
//		data = NULL;
//		ESP8266_struct.ESP_RecvCMDStat = 0; 
//	}		
//}

//void flashwrite_ele(void)
//{
//	  FLASH_Status status = FLASH_COMPLETE;
//		FLASH_Unlock();									//���� 
//		FLASH_DataCacheCmd(DISABLE);//FLASH�����ڼ�,�����ֹ���ݻ���
//		//while(1)		//ɨ��һ���ϰ�.(�Է�FFFFFFFF�ĵط�,�Ȳ���)
////		{  
////			status=FLASH_EraseSector(STMFLASH_GetFlashSector(FLASH_SAVE_ADDR),VoltageRange_3);//VCC=2.7~3.6V֮��!!
////			//if(status!=FLASH_COMPLETE)break;	//����������
////		}
//		status = FLASH_EraseSector(STMFLASH_GetFlashSector(FLASH_SAVE_ADDR),VoltageRange_3);//VCC=2.7~3.6V֮��!!
//		if(status==FLASH_COMPLETE)
//		{
//			if(FLASH_ProgramWord(FLASH_SAVE_ADDR,t.lighth)!=FLASH_COMPLETE);//д������
//			if(FLASH_ProgramWord(FLASH_SAVE_ADDR+4,t.lightl)!=FLASH_COMPLETE);//д������
//			if(FLASH_ProgramWord(FLASH_SAVE_ADDR+8,t.TemH)!=FLASH_COMPLETE);//д������
//			if(FLASH_ProgramWord(FLASH_SAVE_ADDR+12,t.TemL)!=FLASH_COMPLETE);//д������
//		}
//  FLASH_DataCacheCmd(ENABLE);	//FLASH��������,�������ݻ���
//	FLASH_Lock();//����
//}

//void flashread(void)
//{
//	t.lighth = STMFLASH_ReadWord(FLASH_SAVE_ADDR);
//	t.lightl = STMFLASH_ReadWord(FLASH_SAVE_ADDR + 4);
//	t.TemH = STMFLASH_ReadWord(FLASH_SAVE_ADDR + 8);
//	t.TemL = STMFLASH_ReadWord(FLASH_SAVE_ADDR + 12);
//}

//float data[1000] = { 0 };
//volatile unsigned int ADDR_count = 0;
//void ramwrite_net(void)
//{
//	unsigned int count = 0;
//		//for(ADDR_count = 0; ADDR_count < 30; ADDR_count++)//���ڴ�д100������
//		while(flag_net == 1)//��û���Ϸ�����
//		{
//			data[count++] = w.GY30;
//			data[count++] = w.tem;
//			data[count++] = w.pepole;
//			data[count++] = RTC_TimeStruct.RTC_Hours;
//			data[count++] = RTC_TimeStruct.RTC_Minutes;
//			data[count++] = RTC_TimeStruct.RTC_Seconds;
//			if(count >= 900){HAL_Delay(200);flag_net = 0;return;}
//			HAL_Delay(1000);
//			ADDR_count++;
//		}	
//}

//void ramread_net(void)
//{
//	char message[500] = { 0 };
//	unsigned int count = 0;
//	unsigned int count2 = 0;
//	unsigned int count1 = ADDR_count;
//	//for(count = 0; count < count1; count++)//���ڴ��е�100������
//	while(data[count2] != 0)
//	{
//		w.GY301 = data[count2++];
//		w.tem1  = data[count2++];
//		w.pepole1  = data[count2++];
//		w.hours   = data[count2++];
//		w.minutes = data[count2++];
//		w.seconds = data[count2++];
//		if(count2 >= 900){HAL_Delay(200);flag_net = 0;return;};
//		sprintf(w.time1,"%02d:%02d:%02d",w.hours,w.minutes,w.seconds);
//		
//		sprintf(message,"{\"sunlit\":%f,\"temperature\":%f,\"personnel\":\"%s\",\"created_time\":\"%s\",\"people\":%d}",w.GY301,w.tem1,"abc",w.time1,w.pepole1);
//		ESP8266_SendData(message);
//		HAL_Delay(100);
//	}
//	memset(data,0,sizeof(data));
////	ADDR_count = 0;
//}


//void flashwrite_net(void)
//{
//	unsigned char count = 0;
//	unsigned int ADDR_count = 0;
//	FLASH_Status status = FLASH_COMPLETE;
//	FLASH_Unlock();									//���� 
//	FLASH_DataCacheCmd(DISABLE);//FLASH�����ڼ�,�����ֹ���ݻ���

//	status = FLASH_EraseSector(STMFLASH_GetFlashSector(FLASH_SAVE_ADDR_net),VoltageRange_3);//VCC=2.7~3.6V֮��!!
//	if(status==FLASH_COMPLETE)
//	{
//		for(count = 0; count < 10; count++)//���ڴ�д100������
//		{
//			//printf("Time:%02d:%02d:%02d\r\n",RTC_TimeStruct.RTC_Hours,RTC_TimeStruct.RTC_Minutes,RTC_TimeStruct.RTC_Seconds);
//			if(FLASH_ProgramWord(FLASH_SAVE_ADDR_net+(ADDR_count+=4),w.GY30)!=FLASH_COMPLETE);//д������
//			if(FLASH_ProgramWord(FLASH_SAVE_ADDR_net+(ADDR_count+=4),w.tem)!=FLASH_COMPLETE);//д������
//			if(FLASH_ProgramWord(FLASH_SAVE_ADDR_net+(ADDR_count+=4),RTC_TimeStruct.RTC_Hours)!=FLASH_COMPLETE);//д������
//			if(FLASH_ProgramWord(FLASH_SAVE_ADDR_net+(ADDR_count+=4),RTC_TimeStruct.RTC_Minutes)!=FLASH_COMPLETE);//д������
//			if(FLASH_ProgramWord(FLASH_SAVE_ADDR_net+(ADDR_count+=4),RTC_TimeStruct.RTC_Seconds)!=FLASH_COMPLETE);//д������
//			HAL_Delay(1000);
//		}
//	}
//  FLASH_DataCacheCmd(ENABLE);	//FLASH��������,�������ݻ���
//	FLASH_Lock();//����
//}

//void flashread_net(void)
//{
//	char message[500] = { 0 };
//	unsigned char count = 0;
//	unsigned int ADDR_count = 0;
//	for(count = 0; count < 10; count++)//���ڴ��е�100������
//	{
//		w.GY30 = STMFLASH_ReadWord(FLASH_SAVE_ADDR_net + (ADDR_count+=4));
//		w.tem  = STMFLASH_ReadWord(FLASH_SAVE_ADDR_net + (ADDR_count+=4));
//		w.hours   = STMFLASH_ReadWord(FLASH_SAVE_ADDR_net + (ADDR_count+=4));
//		w.minutes = STMFLASH_ReadWord(FLASH_SAVE_ADDR_net + (ADDR_count+=4));
//		w.seconds = STMFLASH_ReadWord(FLASH_SAVE_ADDR_net + (ADDR_count+=4));
//		
//		sprintf(w.time1,"%02d:%02d:%02d",w.hours,w.minutes,w.seconds);
//		
//		sprintf(message,"{\"sunlit\":%f,\"temperature\":%f,\"personnel\":\"%s\",\"created_time\":\"%s\",\"people\":%d}",w.GY30,w.tem,"abc",w.time1,w.pepole);
//		ESP8266_SendData1(message);
//		HAL_Delay(1000);
//	}
//}


