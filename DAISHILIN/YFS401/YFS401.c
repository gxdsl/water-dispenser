#include "YFS401.h"
#include "tim.h"
#include "Usart_dsl.h"

GOLBAL_FLOW golbal_flow;

//四分管
#define MODE_4_K            5.0f        //流量系K
#define MODE_4_PLUSE_CNT_1L 300.0f      //每升水脉冲数

//六分管
#define MODE_6_K            5.5f
#define MODE_6_PLUSE_CNT_1L 330.0f

//6mm管
#define MODE_6M_K            750.0f
#define MODE_6M_PLUSE_CNT_1L 45000.0f

#define	FLOW_FULL			1000000


// 定义流量模型的枚举类型
typedef enum {
    MODE_4_PIPE = 0,   // 四分管
    MODE_6_PIPE = 1,   // 六分管
    MODE_6MM_PIPE = 2  // 6mm管
} FlowModel;

// 定义流量参数
float flowK[3] = {5.0f, 5.5f, 750.0f};                  // 流量系数 K
float pulseCntPerLiter[3] = {300.0f, 330.0f, 45000.0f}; // 每升水脉冲数

// 定义 Flow_Model
FlowModel flowModel = MODE_6MM_PIPE; // 默认使用6mm管


uint32_t pluse1L;           //测试1L水的脉冲数

//==============================================================================
// @函数: Flow_Read(void)
// @描述: 读取流量
// @参数: None
// @返回: None
// @时间: 2024.2.26
//==============================================================================
void Flow_Read(void)
{
    // 根据 Flow_Model 选择不同的流量参数
    float flowKValue = flowK[flowModel];
    float pulseCntValue = pulseCntPerLiter[flowModel];
    
	if(golbal_flow.pluse_1s > 0)
	{
    /*计算公式：
			累计流量 = 对瞬时流量做积分
							 = (脉冲频率 / 每升水脉冲数)
		*/
		golbal_flow.acculat += (golbal_flow.pluse_1s * 1000 / pulseCntValue);   //单位mL
		pluse1L+=golbal_flow.pluse_1s;
		/*计算公式：
					瞬时流量 = ((脉冲频率 ) / 每升水脉冲数) * 60s 
									 = (脉冲频率) / (流量系K)
		*/
		golbal_flow.instant = golbal_flow.pluse_1s / flowKValue;  //单位（L/min）

        if(golbal_flow.acculat >= 1000000)        //最大累计流量1000L
		{
			golbal_flow.acculat = 0;
		}
	}
	else
	{
		golbal_flow.instant  = 0;
	}
	
	printf("瞬间流量：%.2f（L/min） 累计流量：%.2f mL   脉冲数：%d \n",golbal_flow.instant,golbal_flow.acculat,golbal_flow.pluse_1s);
   
    Usart3Printf("t5.txt=\"%0.2f\"\xFF\xFF\xFFt6.txt=\"%0.0f\"\xFF\xFF\xFF",golbal_flow.instant,golbal_flow.acculat);     //回到页面0
	
//	golbal_flow.receive_flag = 0;     			 //接收完成标志位清零
	
    golbal_flow.pluse_1s = 0;
}


//==============================================================================
// @函数: YFS401_Start(void)
// @描述: 启动YFS401流量传感器
// @参数: None
// @返回: None
// @时间: 2024.3.15
//==============================================================================
void YFS401_Start(void)
{
    golbal_flow.pluse_1s = 0; // 清零1s内脉冲数
    golbal_flow.acculat = 0;  // 清零累计流量
    golbal_flow.instant = 0;  // 清零瞬时流量

    HAL_TIM_Base_Start(&htim2);  // 开启定时器2

    __HAL_TIM_CLEAR_FLAG(&htim7, TIM_SR_UIF);  // 防止TIM7启动时进一次中断
    HAL_TIM_Base_Start_IT(&htim7);             // 启动定时器7，每秒一次中断
}


//==============================================================================
// @函数: void YFS401_Stop(void)
// @描述: 关闭YFS401流量传感器
// @参数: None
// @返回: None
// @时间: 2024.3.15
//==============================================================================
void YFS401_Stop(void)
{
    HAL_TIM_Base_Stop(&htim2);     // 关闭定时器2
    HAL_TIM_Base_Stop_IT(&htim7);  // 关闭定时器7
}



