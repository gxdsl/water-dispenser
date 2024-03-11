#include "TDS.h"
#include "adc.h"
#include "All_Init.h"
#include "cmsis_os.h"
#include "core_delay.h"
#include "Usart_dsl.h"
#include "stdio.h"


uint8_t DMA_Status = 0;              //DMA中断标志位
uint32_t ADC_DMABuffer[(NUM_CHANNELS*NUM_SAMPLES_PER_CHANNEL)];  //数组用于保存DMA数值
uint16_t ADC_DMABuffer_AV[NUM_CHANNELS];                         //ADC平均值数组

float compensationCoefficient;      // 温度补偿系数
float compensationVolatge;          // 温度补偿电压

//TDS=133.42*compensationVoltage^3-255.86*compensationVoltage^2+857.39*compensationVoltage*0.5

//==============================================================================
// @函数: void TDS_Init(void)
// @描述: TDS水质检测初始化函数
// @参数: None
// @返回: None
// @时间: 2024.3.11
//==============================================================================
void TDS_Init(void)
{
    CPU_TS_TmrInit();   //微秒延时函数初始化
    
    HAL_ADC_Start_DMA(&hadc1,(uint32_t *)ADC_DMABuffer,(NUM_CHANNELS*NUM_SAMPLES_PER_CHANNEL)*2);  //启动ADC转换和DMA数据传输
    
    printf("TDS水质检测初始化成功\r\n");
    
    DSL.Start +=20;
    Usart3Printf("TDS水质检测初始化成功\r\n");
    
    
    HAL_Delay(500);
}


//==============================================================================
// @函数: void TDS_GetValue(void)
// @描述: TDS值采集函数
// @参数: None
// @返回: None
// @时间: 2023.11.11
//==============================================================================
void TDS_GetValue(void)
{
    if(DMA_Status == 1)
    {
        DMA_Status = 0;
        for(int channel = 0; channel < NUM_CHANNELS; channel++)
        {
            uint32_t sum = 0;
            for (int i = 0; i < NUM_SAMPLES_PER_CHANNEL; i++) 
            {
                sum += ADC_DMABuffer[NUM_CHANNELS*i+channel];
            }
            ADC_DMABuffer_AV[channel] = sum / NUM_SAMPLES_PER_CHANNEL;     //输出每个通道的平均值
        }
        DSL.Adc = ADC_DMABuffer_AV[0]*3.3/4096;
        compensationCoefficient = 1.0f + 0.02f * (DSL.Tem - 25.0f); // 温度补偿系数
        compensationVolatge = DSL.Adc / compensationCoefficient; // 经过温度补偿的电压值
        
        DSL.Tds=133.42f*compensationVolatge*compensationVolatge*compensationVolatge-255.86f*compensationVolatge*compensationVolatge+857.39f*compensationVolatge*0.5f;
//        printf("Voltage = %0.3fmV\r\n",ADC_DMABuffer_AV[0]*3300.0/4096);
        osDelay(1000);
        HAL_ADC_Start_DMA(&hadc1,(uint32_t *)ADC_DMABuffer,(NUM_CHANNELS*NUM_SAMPLES_PER_CHANNEL)*2);  //启动ADC转换和DMA数据传输
    }
}









