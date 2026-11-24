#include "em_sensors.h"
#include "em_adc.h"
#include "main.h"
#include "math.h"

int freq = 2000;    //设置频率2000kHz
int channel = 0;    //通道号，取值0 ~ 15
int resolution = 8; //分辨率，取值0~20，占空比duty最大取值为2^resolution-1

volatile bool need_report = false;//容易改变，不能被编译器优化掉


static long map(long x, long in_min, long in_max, long out_min, long out_max) {//把一个数从一段范围换算到另一段范围
    const long dividend = out_max - out_min;  //输出跨度
    const long divisor = in_max - in_min;			//输入跨度
    const long delta = x - in_min;						//数 离输入下限的距离
    if(divisor == 0){
        return -1;
    }
    return (delta * dividend + (divisor / 2)) / divisor + out_min;//先乘后除防截断，再加四舍五入
}

void read_battery()//更新电池电量（日志更新）
{
		int battery_temp = map(get_adc_volts()*2,3300,4200,0,100);
		
		if(battery_temp < 0)	battery_temp=0;
		if(battery_temp > 100)	battery_temp=100;

    get_device_state()->battery = battery_temp;
    printf("电池电量是 = %d %%\n",get_device_state()->battery);
}

void read_temperature()////更新电池电量（日志更新）
{
    float temperature = get_adc_temperatrue();
	
		printf("打印头温度为 = %f ℃\n",temperature);
	
    if ( isfinite(temperature) && temperature > -10.0f && temperature < 100.0f) //数字是有理数且温度范围正常
			{
					get_device_state()->temperature = (uint8_t)temperature;   
			} 
		else 
			{
					get_device_state()->temperature = 0;                  //视为无效 
			}
}

void run_beep(beep_type_e type){
	
}

void run_led(led_type_e type){
    switch (type)
    {
			case LED_CONNECT:		//手机连上————亮灯
				HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_RESET);
        break;
			case LED_DISCONNECT://手机断开————灭灯
				HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_SET);	
        break;
			case LED_WARN:			//缺纸，过热，忙碌————闪烁 2 次进行预警
        HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_RESET);	
        vTaskDelay(200);
        HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_SET);
        vTaskDelay(200);
        HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_RESET);
        vTaskDelay(200);
        HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_SET);
        vTaskDelay(200);
        HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_RESET);
        break;
    case LED_PRINTER_START://打印开始 闪烁一次后常亮
				HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_SET);
				vTaskDelay(200);
        HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_RESET);
        vTaskDelay(200);
        break;
		case LED_BLE_INIT:		//蓝牙灯光初始化 亮一下后灭掉
				HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_RESET);
				vTaskDelay(10); 
        HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_SET);
				break;
    default:
        break;
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if(GPIO_Pin & GPIO_PIN_8)
	{
    need_report = true;			//需要上报
    set_device_paper_status(PAPER_STATUS_LACK);//纸张状态更改———>缺失
	}
}

void read_paper_statue()//读引脚，更新纸张状态
{
    if(HAL_GPIO_ReadPin(VHINT_GPIO_Port,VHINT_Pin) == PAPER_STATUS_NORMAL){
        get_device_state()->paper_state = PAPER_STATUS_NORMAL;
    }else{
        get_device_state()->paper_state = PAPER_STATUS_LACK;
    }
    printf("纸张状态 = %d （0为正常 1为缺纸）\n",get_device_state()->paper_state);
}

bool read_paper_irq_need_report_status()//读取缺纸状态 并进行状态复位
{
    if (need_report)
    {
        need_report = false;
        return true;
    }
    else
        return false;
}

void read_all_sensors()
{
    read_battery();
    read_temperature();
    read_paper_statue();
}

void init_sensors()
{
	HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_SET);//把灯关了，因为初始化是拉低的
}

