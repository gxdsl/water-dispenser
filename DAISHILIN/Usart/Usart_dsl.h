#ifndef __USART_DSL_H__
#define __USART_DSL_H__

#include "stm32f4xx.h"                  // Device header

extern uint8_t Rx1Data;
extern uint8_t Rx2Data;
extern uint8_t Rx3Data;

//void UsartPrintf(UART_HandleTypeDef *huart,const char *format,...);
void Usart1Printf(const char *format,...);
void Usart2Printf(const char *format,...);
void Usart3Printf(const char *format,...);

void HMI_Handle(void);
void WiFi_Handle(void);

void extractJsonData(const char* input, char* output);

#endif
