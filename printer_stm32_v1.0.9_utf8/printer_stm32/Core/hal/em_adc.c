#include "em_config.h"
#include "main.h"	//引入引脚宏
#include "math.h"	//进行log运算

extern ADC_HandleTypeDef hadc1;

#define ADC_READ_TIME 10	//adc读取次数

uint32_t ADC_Value[2];	//电池，温度

void adc_init() {
	HAL_ADC_Start(&hadc1);										//启动ADC
	HAL_ADC_PollForConversion(&hadc1,1000);		//轮询等待结束，因为不取值，所以只是预热adc
}


uint32_t adc_alg_handle(uint32_t *adc, int size) {	//去掉最大值和最小值后，返回平均值
    uint32_t sum = 0;
    uint32_t min_val = adc[0];
    uint32_t max_val = adc[0];

    for (int i = 0; i < size; i++) {
        if (adc[i] < min_val) {
            min_val = adc[i];
        }
        else if (adc[i] > max_val) {
            max_val = adc[i];
        }
        sum += adc[i];
    }//选出最小值和最大值和adc多次的总和
    sum = sum - (min_val + max_val);
    uint32_t avg_val = sum / (size - 2);
    return avg_val;
}//去掉最小值和最大值后取平均值


/**
 * @brief 获取ADC引脚电压值，可根据需要加入滤波算法
 * 
 * @return int 
 */
int get_adc_volts()
{
  uint32_t data = 0;
  uint32_t adc1[ADC_READ_TIME];//电池
  uint32_t adc2[ADC_READ_TIME];//温度

  for (int sample_ptr = 0; sample_ptr < ADC_READ_TIME; sample_ptr++)
  {
		//由于上面设置的ADC是间断模式+扫描模式，所以每采集一次，需要执行一次HAL_ADC_Start，
		HAL_ADC_Start(&hadc1);
		if (HAL_ADC_PollForConversion(&hadc1, 100) == HAL_OK)	//仅处理成功时的数值，存到滤波数组
		{
			 adc1[sample_ptr] = HAL_ADC_GetValue(&hadc1);
		}
		HAL_ADC_Start(&hadc1);
		if (HAL_ADC_PollForConversion(&hadc1, 100) == HAL_OK)
		{
			 adc2[sample_ptr] = HAL_ADC_GetValue(&hadc1);
		}
		HAL_ADC_Stop(&hadc1);		//不用时停止
  }
  ADC_Value[0] = adc_alg_handle(adc1, ADC_READ_TIME);
  ADC_Value[1] = adc_alg_handle(adc2, ADC_READ_TIME);
	
  printf(" 电池电量（电压）	 ：	ADC channel0 end value = ->%1.3fV \r\n", ADC_Value[0] * 3.3f / 4096);	//打印函数在main.c重定义到了串口1,usb输出log
  printf(" 打印头温度（电压）： ADC channel1 end value = ->%1.3fV \r\n", ADC_Value[1] * 3.3f / 4096);
  data = ADC_Value[0] * 3300 / 4096;

  HAL_ADC_Stop(&hadc1);			//关两次，安全冗余
  return data;
}

/**
 公式法：
NTC 热敏电阻温度计算公式：Rt = Rp *EXP(B*(1/T1-1/T2))
可以得到温度T1与电阻Rt的关系T1=1/（log（Rt/Rp）/B+1/T2）

T1和T2指的是K度，即开尔文温度,K度=273.15(绝对温度)+摄氏度。
T2=(273.15+25)
Rt 是热敏电阻在T1温度下的阻值。
Rp 是热敏电阻在T2常温下的标称阻值。100K的热敏电阻25℃的值为100K（即R=100K）。T2=(273.15+25)
B值是热敏电阻的重要参数  B25/50= 3950K ±1%
最终对应的摄氏温度t=T1-273.15,同时+0.5的误差矫正。
*/

/**
 * @brief 阻值转换温度
 *
 * @param Rt 热敏电阻阻值
 * @return float 返回摄氏度温度
 */
float em_temp_calculate(float Rt)
{
    float Rp = 30000; // 30k
    float T2 = 273.15 + 25;
    float Bx = 3950; // B值
    float Ka = 273.15;
    float temp = 0.0f;

    temp = 1 / (log(Rt / Rp) / Bx + 1 / T2) - Ka + 0.5;
    return temp;
}

/**
 * @brief 计算温度
 * 
 * @return float 
 */
float get_adc_temperatrue(){
  float temp = 0.0f;
  float Rt=0;
  float vol=0;
  //ADC转换为电压 vol=AD/4096*VCC
  vol=(float)ADC_Value[1]*3.3f/4096;
	printf("ADC temperatrue analog value = %f\n",vol);
  //电压转换为阻值 原理图为10k 1%精度  vol/VCC=Rt/(R+Rt)  vol/3.3=Rt/(10000+Rt)
  Rt=(vol*10000)/(3.3-vol);
	printf("ADC temperatrue Rt = %f\n",Rt);
  temp = em_temp_calculate(Rt);
  return temp;
}
