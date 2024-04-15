/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "string.h"
#include "stdio.h"
#include "usart.h"
#include "adc.h"
#include "All_Init.h"
#include "DS18B20.h"
#include "usart_dsl.h"
#include "TDS.h"
#include "ESP8266.h"
#include "RC522.h"
#include "YFS401.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

float Ds18b20_Get_Temp(void);   //声明，否则有警告
void Trace_Task(void);

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

char taskInfoBuffer[512]; // 用于存储任务信息的较大缓冲
char ESP8266_message[500] = {0};

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for HMI_Task */
osThreadId_t HMI_TaskHandle;
const osThreadAttr_t HMI_Task_attributes = {
  .name = "HMI_Task",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityLow2,
};
/* Definitions for WiFi_Task */
osThreadId_t WiFi_TaskHandle;
const osThreadAttr_t WiFi_Task_attributes = {
  .name = "WiFi_Task",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityLow2,
};
/* Definitions for RFID_Task */
osThreadId_t RFID_TaskHandle;
const osThreadAttr_t RFID_Task_attributes = {
  .name = "RFID_Task",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for Value_Task */
osThreadId_t Value_TaskHandle;
const osThreadAttr_t Value_Task_attributes = {
  .name = "Value_Task",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for Flow_Task */
osThreadId_t Flow_TaskHandle;
const osThreadAttr_t Flow_Task_attributes = {
  .name = "Flow_Task",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityLow3,
};
/* Definitions for Mutex_Trace */
osMutexId_t Mutex_TraceHandle;
const osMutexAttr_t Mutex_Trace_attributes = {
  .name = "Mutex_Trace"
};
/* Definitions for HMI_BinarySem */
osSemaphoreId_t HMI_BinarySemHandle;
const osSemaphoreAttr_t HMI_BinarySem_attributes = {
  .name = "HMI_BinarySem"
};
/* Definitions for WiFi_BinarySem */
osSemaphoreId_t WiFi_BinarySemHandle;
const osSemaphoreAttr_t WiFi_BinarySem_attributes = {
  .name = "WiFi_BinarySem"
};
/* Definitions for Flow_BinarySem */
osSemaphoreId_t Flow_BinarySemHandle;
const osSemaphoreAttr_t Flow_BinarySem_attributes = {
  .name = "Flow_BinarySem"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void HMI_StartTask(void *argument);
void WiFi_StartTask(void *argument);
void RFID_StartTask(void *argument);
void Value_StartTask(void *argument);
void Flow_StartTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* Hook prototypes */
void configureTimerForRunTimeStats(void);
unsigned long getRunTimeCounterValue(void);

/* USER CODE BEGIN 1 */
/* Functions needed when configGENERATE_RUN_TIME_STATS is on */
__weak void configureTimerForRunTimeStats(void)
{

}

__weak unsigned long getRunTimeCounterValue(void)
{
return 0;
}
/* USER CODE END 1 */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */
  /* Create the mutex(es) */
  /* creation of Mutex_Trace */
  Mutex_TraceHandle = osMutexNew(&Mutex_Trace_attributes);

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* creation of HMI_BinarySem */
  HMI_BinarySemHandle = osSemaphoreNew(1, 1, &HMI_BinarySem_attributes);

  /* creation of WiFi_BinarySem */
  WiFi_BinarySemHandle = osSemaphoreNew(1, 1, &WiFi_BinarySem_attributes);

  /* creation of Flow_BinarySem */
  Flow_BinarySemHandle = osSemaphoreNew(1, 1, &Flow_BinarySem_attributes);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of HMI_Task */
  HMI_TaskHandle = osThreadNew(HMI_StartTask, NULL, &HMI_Task_attributes);

  /* creation of WiFi_Task */
  WiFi_TaskHandle = osThreadNew(WiFi_StartTask, NULL, &WiFi_Task_attributes);

  /* creation of RFID_Task */
  RFID_TaskHandle = osThreadNew(RFID_StartTask, NULL, &RFID_Task_attributes);

  /* creation of Value_Task */
  Value_TaskHandle = osThreadNew(Value_StartTask, NULL, &Value_Task_attributes);

  /* creation of Flow_Task */
  Flow_TaskHandle = osThreadNew(Flow_StartTask, NULL, &Flow_Task_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */

  /* Infinite loop */
  for(;;)
  {
//    Trace_Task();

    osDelay(1000);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_HMI_StartTask */
/**
* @brief Function implementing the HMI_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_HMI_StartTask */
void HMI_StartTask(void *argument)
{
  /* USER CODE BEGIN HMI_StartTask */
  /* Infinite loop */
  for(;;)
  {
//      printf("HMI_StartTask");
      // 当串口3(串口屏)接收到数据时，任务被唤醒
      osSemaphoreAcquire(HMI_BinarySemHandle, osWaitForever);
      
      HMI_Handle();
      
  }
  /* USER CODE END HMI_StartTask */
}

/* USER CODE BEGIN Header_WiFi_StartTask */
/**
* @brief Function implementing the WiFi_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_WiFi_StartTask */
void WiFi_StartTask(void *argument)
{
  /* USER CODE BEGIN WiFi_StartTask */
  /* Infinite loop */
  for(;;)
  {
      // 当串口1(ESP8266)接收到数据时，任务被唤醒
      osSemaphoreAcquire(WiFi_BinarySemHandle, osWaitForever);
      
      WiFi_Handle();
  }
  /* USER CODE END WiFi_StartTask */
}

/* USER CODE BEGIN Header_RFID_StartTask */
/**
* @brief Function implementing the RFID_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_RFID_StartTask */
void RFID_StartTask(void *argument)
{
  /* USER CODE BEGIN RFID_StartTask */
  /* Infinite loop */
  for(;;)
  {
    RC522_Read();
    osDelay(1000);
  }
  /* USER CODE END RFID_StartTask */
}

/* USER CODE BEGIN Header_Value_StartTask */
/**
* @brief Function implementing the Value_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Value_StartTask */
void Value_StartTask(void *argument)
{
  /* USER CODE BEGIN Value_StartTask */
  /* Infinite loop */
  for(;;)
  {
      DSL.Tem = Ds18b20_Get_Temp();
      TDS_GetValue();
//    printf("温度：%f°C",DSL.Tem);
      
      sprintf(ESP8266_message,"{\"status\": \"action\",\"dispenser_id\":%d,\"temperature\":%0.1f,\
      \"tds\":%0.1f,\"flow\":%s}",DSL.ID,DSL.Tem,DSL.Tds,DSL.Flow? "true" : "false");
      ESP8266_SendData(ESP8266_message);
      
      Usart3Printf("t1.txt=\"%0.2f\"\xff\xff\xfft2.txt=\"%0.2f\"\xff\xff\xff",DSL.Tem,DSL.Tds);
      
      osDelay(1000);
      HAL_ADC_Start_DMA(&hadc1,(uint32_t *)ADC_DMABuffer,(NUM_CHANNELS*NUM_SAMPLES_PER_CHANNEL)*2);  //启动ADC转换和DMA数据传输
  }
  /* USER CODE END Value_StartTask */
}

/* USER CODE BEGIN Header_Flow_StartTask */
/**
* @brief Function implementing the Flow_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Flow_StartTask */
void Flow_StartTask(void *argument)
{
  /* USER CODE BEGIN Flow_StartTask */
  /* Infinite loop */
  for(;;)
  {
      // 当TIM7每1秒时，任务被唤醒
      osSemaphoreAcquire(Flow_BinarySemHandle, osWaitForever);
      
      Flow_Read();
  }
  /* USER CODE END Flow_StartTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

//==============================================================================
// @函数: void Trace_Task(void)
// @描述: 打印任务
// @参数: None
// @返回: None
// @时间: 2023.11.1
//==============================================================================
void Trace_Task(void)
{
    if (osMutexWait(Mutex_TraceHandle, osWaitForever) == osOK) {
        // 清空任务信息缓冲区
        memset(taskInfoBuffer, 0, sizeof(taskInfoBuffer));

        // 获取任务列表
        vTaskList(taskInfoBuffer);

        // 释放互斥量
        osMutexRelease(Mutex_TraceHandle);
    }
    // 打印所有任务的信息
    printf("任务名称\t\t状态    优先级    剩余栈    编号\n%s\n", taskInfoBuffer);
}
/* USER CODE END Application */

