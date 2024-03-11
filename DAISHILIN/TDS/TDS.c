#include "TDS.h"
#include "adc.h"
#include "All_Init.h"
#include "cmsis_os.h"
#include "core_delay.h"
#include "Usart_dsl.h"
#include "stdio.h"


uint8_t DMA_Status = 0;              //DMA�жϱ�־λ
uint32_t ADC_DMABuffer[(NUM_CHANNELS*NUM_SAMPLES_PER_CHANNEL)];  //�������ڱ���DMA��ֵ
uint16_t ADC_DMABuffer_AV[NUM_CHANNELS];                         //ADCƽ��ֵ����

float compensationCoefficient;      // �¶Ȳ���ϵ��
float compensationVolatge;          // �¶Ȳ�����ѹ

//TDS=133.42*compensationVoltage^3-255.86*compensationVoltage^2+857.39*compensationVoltage*0.5

//==============================================================================
// @����: void TDS_Init(void)
// @����: TDSˮ�ʼ���ʼ������
// @����: None
// @����: None
// @ʱ��: 2024.3.11
//==============================================================================
void TDS_Init(void)
{
    CPU_TS_TmrInit();   //΢����ʱ������ʼ��
    
    HAL_ADC_Start_DMA(&hadc1,(uint32_t *)ADC_DMABuffer,(NUM_CHANNELS*NUM_SAMPLES_PER_CHANNEL)*2);  //����ADCת����DMA���ݴ���
    
    printf("TDSˮ�ʼ���ʼ���ɹ�\r\n");
    
    DSL.Start +=20;
    Usart3Printf("TDSˮ�ʼ���ʼ���ɹ�\r\n");
    
    
    HAL_Delay(500);
}


//==============================================================================
// @����: void TDS_GetValue(void)
// @����: TDSֵ�ɼ�����
// @����: None
// @����: None
// @ʱ��: 2023.11.11
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
            ADC_DMABuffer_AV[channel] = sum / NUM_SAMPLES_PER_CHANNEL;     //���ÿ��ͨ����ƽ��ֵ
        }
        DSL.Adc = ADC_DMABuffer_AV[0]*3.3/4096;
        compensationCoefficient = 1.0f + 0.02f * (DSL.Tem - 25.0f); // �¶Ȳ���ϵ��
        compensationVolatge = DSL.Adc / compensationCoefficient; // �����¶Ȳ����ĵ�ѹֵ
        
        DSL.Tds=133.42f*compensationVolatge*compensationVolatge*compensationVolatge-255.86f*compensationVolatge*compensationVolatge+857.39f*compensationVolatge*0.5f;
//        printf("Voltage = %0.3fmV\r\n",ADC_DMABuffer_AV[0]*3300.0/4096);
        osDelay(1000);
        HAL_ADC_Start_DMA(&hadc1,(uint32_t *)ADC_DMABuffer,(NUM_CHANNELS*NUM_SAMPLES_PER_CHANNEL)*2);  //����ADCת����DMA���ݴ���
    }
}









