//硬件驱动
#include "stm32f4xx.h"                  // Device header
#include "ESP8266.h"
#include "usart.h"
#include "usart_dsl.h"
#include "All_Init.h"

//C库
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cmsis_os.h"

//全局变量
ESP8266_STRUCT ESP8266_struct;


/*
************************************************************
*	函数名称：	ESP8266_Init
*	函数功能：	ESP8266初始化
*	入口参数：	
*	返回参数：	无
*	说明：		
************************************************************
*/
void ESP8266_Init(void)
{
	ESP8266_Clear();
	
	while(ESP8266_SendCMD("AT\r\n", "OK"))
		HAL_Delay(500);
//	UsartPrintf(&ESP_DebugUsart,"AT响应成功\r\n");
	printf("AT响应成功\r\n");
	while(ESP8266_SendCMD("AT+CWMODE=1\r\n", "OK"))
		HAL_Delay(500);
//	UsartPrintf(&ESP_DebugUsart,"AT+CWMODE响应成功\r\n"); 
	printf("AT+CWMODE响应成功\r\n");
    
	while(ESP8266_SendCMD("AT+CWDHCP=1,1\r\n", "OK"))
		HAL_Delay(500);
//	UsartPrintf(&ESP_DebugUsart,"AT+CWDHCP响应成功\r\n");
	printf("AT+CWDHCP响应成功\r\n");
    
	while(ESP8266_SendCMD(WIFI_Password, "GOT IP"))
		HAL_Delay(500);
//	UsartPrintf(&ESP_DebugUsart,"AT+CWJAP响应成功\r\n");
    printf("AT+CWJAP响应成功\r\n");
	
	while(ESP8266_SendCMD(WIFI_IPAdder_IPPort, "CONNECT"));
		HAL_Delay(500);
//	UsartPrintf(&ESP_DebugUsart,"AT+CIPSTART响应成功\r\n");
    printf("AT+CIPSTART响应成功\r\n");
    
    printf("ESP8266初始化成功\r\n");
    
    DSL.Start +=20;
    Usart3Printf("ESP8266初始化成功\r\n");
    
	HAL_Delay(500);
}

/*
************************************************************
*	函数名称：	ESP8266_Clear
*	函数功能：	接收的数据清理
*	入口参数：
*	返回参数：	无
*	说明：
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
*	函数名称：	ESP8266_WaitRecive
*	函数功能：	等待数据接收完成
*	入口参数：	
*	返回参数：	无
*	说明：		
************************************************************
*/
_Bool ESP8266_WaitRecive(void)
{
	if(ESP8266_struct.ESP_cnt == 0) 							//如果接收计数为0 则说明没有处于接收数据中，所以直接跳出，结束函数
		return ESP_REVWAIT;

	if(ESP8266_struct.ESP_cnt == ESP8266_struct.ESP_cntPre)				//如果上一次的值和这次相同，则说明接收完毕
	{
		ESP8266_struct.ESP_cnt = 0;							//清0接收计数
		return ESP_REVOK;								//返回接收完成标志
	}

	ESP8266_struct.ESP_cntPre = ESP8266_struct.ESP_cnt;					//置为相同

	return ESP_REVWAIT;								//返回接收未完成标志
}

/*
************************************************************
*	函数名称：	ESP8266_SendCMD
*	函数功能：	发送命令
*	入口参数：	
*	返回参数：	无
*	说明：
************************************************************
*/
_Bool ESP8266_SendCMD(char *CMD,char *keyword)
{
	unsigned char Time=100;
	ESP8266_Clear(); //清除缓存

    HAL_UART_Transmit(&ESP_SendUsart, (uint8_t *)CMD, strlen((const char *)CMD),0xffff);//发送数据
    
	while(Time--)     //循环是否接收完成，循环执行两百次，直至成功执行return 0,退出。
	{
		if(ESP8266_WaitRecive() == ESP_REVOK)     //如果接收完成
		{

			if(strstr((const char *)ESP8266_struct.ESP_usartbuf,keyword)!=NULL)     //那么检测是否搜索到关键字
			{
				ESP8266_Clear(); //清除缓存
				return 0;        //返回值为0，退出
			}
		}
		HAL_Delay(20);
	}
	return 1;
}

/*
************************************************************
*	函数名称：	ESP8266_SendData
*	函数功能：	发送数据
*	入口参数：	
*	返回参数：	无
*	说明：
************************************************************
*/
void ESP8266_SendData(char *DATA)
{
	char Buffer[500];  //用于缓存数据
	
	ESP8266_Clear();
	
	sprintf(Buffer,"AT+CIPSEND=%d\r\n",strlen((const char *)DATA));   //将数据缓存到Buffer中
    
	if(!ESP8266_SendCMD(Buffer,"OK"))             //判断检测到的字符是否正确
	{
//        HAL_Delay(10);
        HAL_UART_Transmit(&ESP_SendUsart, (uint8_t *)DATA, strlen((const char *)DATA),0xFFFF);//发送数据
	}
}

void ESP8266_SendData1(char *DATA)
{
	char Buffer[500];  //用于缓存数据
//	static unsigned char conect = 0;
	
	ESP8266_Clear();
	
	sprintf(Buffer,"AT+CIPSEND=%d\r\n",strlen((const char *)DATA));   //将数据缓存到Buffer中
	
	if(!ESP8266_SendCMD(Buffer,">"))             //判断检测到的字符是否正确
	{
//		Usart1_SendString(ESP_SendUsart,(unsigned char *)DATA,strlen((const char *)DATA));    //发送数据
//        Usart1Printf(DATA);
        HAL_UART_Transmit(&ESP_SendUsart, (uint8_t *)DATA, strlen((const char *)DATA),0xFFFF);//发送数据
	}
	else {
//		conect++;
//		if(conect == 1)
//		{
//			xTaskNotify((TaskHandle_t	)flash_Task_Handler,//接收任务通知的任务句柄
//      (uint32_t		)flash_EVENT,			//要触发的事件
//      (eNotifyAction)eSetBits);			//设置任务通知值中的位
////		 HAL_Delay(3000);
////			while(ESP8266_SendCMD(WIFI_Password, "GOT IP"));
////		 HAL_Delay(200);
////			Usart1_Printf(ESP_DebugUsart,"AT+CWJAP响应成功\r\n"); 
//			
//			while(ESP8266_SendCMD(WIFI_IPAdder_IPPort, "CONNECT"));
//		 HAL_Delay(200);
//			Usart1_Printf(ESP_DebugUsart, "AT+CIPSTART响应成功\r\n");
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
//////		xTaskNotify((TaskHandle_t	)flash_Task_Handler,//接收任务通知的任务句柄
//////      (uint32_t		)flash_EVENT,			//要触发的事件
//////      (eNotifyAction)eSetBits);			//设置任务通知值中的位
////			//ramwrite_net();
//////		HAL_Delay(3000);		
//////		while(ESP8266_SendCMD(WIFI_Password, "GOT IP")); 		HAL_Delay(200);
//			flag_net = 1;
//			while(ESP8266_SendCMD(WIFI_IPAdder_IPPort, "CONNECT"));
//////			HAL_Delay(200);
////			ramread_net();
//			flag_net = 0;
//			ESP8266_Clear(); //清除缓存
//			Usart1_Printf(ESP_DebugUsart, "AT+CIPSTART响应成功\r\n");
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
//		ESP8266_Clear(); //清除缓存
//		data = NULL;
//		ESP8266_struct.ESP_RecvCMDStat = 0; 
//	}		
//}

//void flashwrite_ele(void)
//{
//	  FLASH_Status status = FLASH_COMPLETE;
//		FLASH_Unlock();									//解锁 
//		FLASH_DataCacheCmd(DISABLE);//FLASH擦除期间,必须禁止数据缓存
//		//while(1)		//扫清一切障碍.(对非FFFFFFFF的地方,先擦除)
////		{  
////			status=FLASH_EraseSector(STMFLASH_GetFlashSector(FLASH_SAVE_ADDR),VoltageRange_3);//VCC=2.7~3.6V之间!!
////			//if(status!=FLASH_COMPLETE)break;	//发生错误了
////		}
//		status = FLASH_EraseSector(STMFLASH_GetFlashSector(FLASH_SAVE_ADDR),VoltageRange_3);//VCC=2.7~3.6V之间!!
//		if(status==FLASH_COMPLETE)
//		{
//			if(FLASH_ProgramWord(FLASH_SAVE_ADDR,t.lighth)!=FLASH_COMPLETE);//写入数据
//			if(FLASH_ProgramWord(FLASH_SAVE_ADDR+4,t.lightl)!=FLASH_COMPLETE);//写入数据
//			if(FLASH_ProgramWord(FLASH_SAVE_ADDR+8,t.TemH)!=FLASH_COMPLETE);//写入数据
//			if(FLASH_ProgramWord(FLASH_SAVE_ADDR+12,t.TemL)!=FLASH_COMPLETE);//写入数据
//		}
//  FLASH_DataCacheCmd(ENABLE);	//FLASH擦除结束,开启数据缓存
//	FLASH_Lock();//上锁
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
//		//for(ADDR_count = 0; ADDR_count < 30; ADDR_count++)//往内存写100组数据
//		while(flag_net == 1)//还没连上服务器
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
//	//for(count = 0; count < count1; count++)//读内存中的100组数据
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
//	FLASH_Unlock();									//解锁 
//	FLASH_DataCacheCmd(DISABLE);//FLASH擦除期间,必须禁止数据缓存

//	status = FLASH_EraseSector(STMFLASH_GetFlashSector(FLASH_SAVE_ADDR_net),VoltageRange_3);//VCC=2.7~3.6V之间!!
//	if(status==FLASH_COMPLETE)
//	{
//		for(count = 0; count < 10; count++)//往内存写100组数据
//		{
//			//printf("Time:%02d:%02d:%02d\r\n",RTC_TimeStruct.RTC_Hours,RTC_TimeStruct.RTC_Minutes,RTC_TimeStruct.RTC_Seconds);
//			if(FLASH_ProgramWord(FLASH_SAVE_ADDR_net+(ADDR_count+=4),w.GY30)!=FLASH_COMPLETE);//写入数据
//			if(FLASH_ProgramWord(FLASH_SAVE_ADDR_net+(ADDR_count+=4),w.tem)!=FLASH_COMPLETE);//写入数据
//			if(FLASH_ProgramWord(FLASH_SAVE_ADDR_net+(ADDR_count+=4),RTC_TimeStruct.RTC_Hours)!=FLASH_COMPLETE);//写入数据
//			if(FLASH_ProgramWord(FLASH_SAVE_ADDR_net+(ADDR_count+=4),RTC_TimeStruct.RTC_Minutes)!=FLASH_COMPLETE);//写入数据
//			if(FLASH_ProgramWord(FLASH_SAVE_ADDR_net+(ADDR_count+=4),RTC_TimeStruct.RTC_Seconds)!=FLASH_COMPLETE);//写入数据
//			HAL_Delay(1000);
//		}
//	}
//  FLASH_DataCacheCmd(ENABLE);	//FLASH擦除结束,开启数据缓存
//	FLASH_Lock();//上锁
//}

//void flashread_net(void)
//{
//	char message[500] = { 0 };
//	unsigned char count = 0;
//	unsigned int ADDR_count = 0;
//	for(count = 0; count < 10; count++)//读内存中的100组数据
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


