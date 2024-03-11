#ifndef __ESP8266_H
#define __ESP8266_H

//#define WIFI_Password			"AT+CWJAP=\"DSL\",\"2722945475\"\r\n"
//#define WIFI_IPAdder_IPPort		"AT+CIPSTART=\"TCP\",\"192.168.101.5\",9999\r\n" 

//#define WIFI_Password			"AT+CWJAP=\"1002\",\"18273698235\"\r\n"
//#define WIFI_IPAdder_IPPort		"AT+CIPSTART=\"TCP\",\"192.168.2.103\",9999\r\n" 

//#define WIFI_Password			"AT+CWJAP=\"HIKVISION_C700F8\",\"3177944670\"\r\n"
//#define WIFI_IPAdder_IPPort		"AT+CIPSTART=\"TCP\",\"192.168.0.195\",502\r\n"

//#define WIFI_Password			"AT+CWJAP=\"643\",\"64388888\"\r\n"
//#define WIFI_IPAdder_IPPort		"AT+CIPSTART=\"TCP\",\"192.168.1.11\",8000\r\n" 

#define WIFI_Password			"AT+CWJAP=\"ASUS\",\"1234567890\"\r\n"
#define WIFI_IPAdder_IPPort		"AT+CIPSTART=\"TCP\",\"192.168.100.174\",9999\r\n" 

#define ESP_REVOK		0	//接收完成标志
#define ESP_REVWAIT		1	//接收未完成标志

#define ESP_SendUsart		huart1 
#define ESP_DebugUsart		huart2

#define FLASH_SAVE_ADDR_net  0X08040000
#define FLASH_SAVE_ADDR      0X08060000     //设置FLASH 保存地址(必须为偶数，且所在扇区,要大于本代码所占用到的扇区.
//否则,写操作的时候,可能会导致擦除整个扇区,从而引起部分程序丢失.引起死机.
#define flash_EVENT (0x01 << 0)//设置事件掩码的位0

//ESP8266结构体
typedef struct
{
	unsigned char ESP_usartbuf[500];
	unsigned char ESP_RecvCMDStat;
	unsigned short ESP_cnt, ESP_cntPre;
	unsigned char shuju[500];
	unsigned char ju[500];
}ESP8266_STRUCT;

extern ESP8266_STRUCT ESP8266_struct;

//extern RTC_TimeTypeDef RTC_TimeStruct;//时间
extern volatile unsigned char flag_net;
extern unsigned char flag_net1;
//函数声明
void ESP8266_Init(void);

void ESP8266_Clear(void);

_Bool ESP8266_WaitRecive(void);

_Bool ESP8266_SendCMD(char *CMD,char *keyword);

void ESP8266_SendData(char *DATA);
void ESP8266_SendData1(char *DATA);
//void TCP_Release(void);

//void flashwrite_ele(void);
//void flashread(void);
//void flashwrite_net(void);
//void flashread_net(void);
//void ramwrite_net(void);
//void ramread_net(void);
#endif
