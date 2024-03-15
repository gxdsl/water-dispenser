#include "YFS401.h"
#include "tim.h"
#include "Usart_dsl.h"

GOLBAL_FLOW golbal_flow;

//�ķֹ�
#define MODE_4_K            5.0f        //����ϵK
#define MODE_4_PLUSE_CNT_1L 300.0f      //ÿ��ˮ������

//���ֹ�
#define MODE_6_K            5.5f
#define MODE_6_PLUSE_CNT_1L 330.0f

//6mm��
#define MODE_6M_K            750.0f
#define MODE_6M_PLUSE_CNT_1L 45000.0f

#define	FLOW_FULL			1000000


// ��������ģ�͵�ö������
typedef enum {
    MODE_4_PIPE = 0,   // �ķֹ�
    MODE_6_PIPE = 1,   // ���ֹ�
    MODE_6MM_PIPE = 2  // 6mm��
} FlowModel;

// ������������
float flowK[3] = {5.0f, 5.5f, 750.0f};                  // ����ϵ�� K
float pulseCntPerLiter[3] = {300.0f, 330.0f, 45000.0f}; // ÿ��ˮ������

// ���� Flow_Model
FlowModel flowModel = MODE_6MM_PIPE; // Ĭ��ʹ��6mm��


uint32_t pluse1L;           //����1Lˮ��������

//==============================================================================
// @����: Flow_Read(void)
// @����: ��ȡ����
// @����: None
// @����: None
// @ʱ��: 2024.2.26
//==============================================================================
void Flow_Read(void)
{
    // ���� Flow_Model ѡ��ͬ����������
    float flowKValue = flowK[flowModel];
    float pulseCntValue = pulseCntPerLiter[flowModel];
    
	if(golbal_flow.pluse_1s > 0)
	{
    /*���㹫ʽ��
			�ۼ����� = ��˲ʱ����������
							 = (����Ƶ�� / ÿ��ˮ������)
		*/
		golbal_flow.acculat += (golbal_flow.pluse_1s * 1000 / pulseCntValue);   //��λmL
		pluse1L+=golbal_flow.pluse_1s;
		/*���㹫ʽ��
					˲ʱ���� = ((����Ƶ�� ) / ÿ��ˮ������) * 60s 
									 = (����Ƶ��) / (����ϵK)
		*/
		golbal_flow.instant = golbal_flow.pluse_1s / flowKValue;  //��λ��L/min��

        if(golbal_flow.acculat >= 1000000)        //����ۼ�����1000L
		{
			golbal_flow.acculat = 0;
		}
	}
	else
	{
		golbal_flow.instant  = 0;
	}
	
	printf("˲��������%.2f��L/min�� �ۼ�������%.2f mL   ��������%d \n",golbal_flow.instant,golbal_flow.acculat,golbal_flow.pluse_1s);
   
    Usart3Printf("t5.txt=\"%0.2f\"\xFF\xFF\xFFt6.txt=\"%0.0f\"\xFF\xFF\xFF",golbal_flow.instant,golbal_flow.acculat);     //�ص�ҳ��0
	
//	golbal_flow.receive_flag = 0;     			 //������ɱ�־λ����
	
    golbal_flow.pluse_1s = 0;
}


//==============================================================================
// @����: YFS401_Start(void)
// @����: ����YFS401����������
// @����: None
// @����: None
// @ʱ��: 2024.3.15
//==============================================================================
void YFS401_Start(void)
{
    golbal_flow.pluse_1s = 0; // ����1s��������
    golbal_flow.acculat = 0;  // �����ۼ�����
    golbal_flow.instant = 0;  // ����˲ʱ����

    HAL_TIM_Base_Start(&htim2);  // ������ʱ��2

    __HAL_TIM_CLEAR_FLAG(&htim7, TIM_SR_UIF);  // ��ֹTIM7����ʱ��һ���ж�
    HAL_TIM_Base_Start_IT(&htim7);             // ������ʱ��7��ÿ��һ���ж�
}


//==============================================================================
// @����: void YFS401_Stop(void)
// @����: �ر�YFS401����������
// @����: None
// @����: None
// @ʱ��: 2024.3.15
//==============================================================================
void YFS401_Stop(void)
{
    HAL_TIM_Base_Stop(&htim2);     // �رն�ʱ��2
    HAL_TIM_Base_Stop_IT(&htim7);  // �رն�ʱ��7
}



