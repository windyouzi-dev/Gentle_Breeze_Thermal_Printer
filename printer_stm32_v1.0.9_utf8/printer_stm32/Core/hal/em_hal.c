#include "em_hal.h"
#include "em_adc.h"
#include "main.h"
#include "math.h"

int freq = 2000;    //设置频率2000kHz
int channel = 0;    //通道号，取值0 ~ 15
int resolution = 8; //分辨率，取值0~20，占空比duty最大取值为2^resolution-1

bool need_report = false;


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
    case LED_CONNECT:
				HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_RESET);//亮灯
        break;
		case LED_DISCONNECT:
				HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_SET);	//灭灯
        break;
    case LED_WARN:
        HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_RESET);	//闪烁 2 次进行预警
        vTaskDelay(200);
        HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_SET);
        vTaskDelay(200);
        HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_RESET);
        vTaskDelay(200);
        HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_SET);
        vTaskDelay(200);
        HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_RESET);
        break;
    case LED_PRINTER_START:
				HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_SET);//打印开始 闪烁一次后常亮
				vTaskDelay(200);
        HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_RESET);
        vTaskDelay(200);
        break;
		case LED_BLE_INIT:
				HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_RESET);//蓝牙灯光初始化 亮一下后灭掉
        HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_RESET);
        HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_SET);
    default:
        break;
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if(GPIO_Pin & GPIO_PIN_8)
	{
		printf("--");
    need_report = true;
    set_device_paper_status(PAPER_STATUS_LACK);
	}
}

void read_paper_statue()
{
    if(HAL_GPIO_ReadPin(VHINT_GPIO_Port,VHINT_Pin) == PAPER_STATUS_NORMAL){
        get_device_state()->paper_state = PAPER_STATUS_NORMAL;
    }else{
        get_device_state()->paper_state = PAPER_STATUS_LACK;
    }
    printf("paper_state = %d\n",get_device_state()->paper_state);
}

bool read_paper_irq_need_report_status()
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

void init_hal()
{
	HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_SET);//把灯关了，因为初始化是拉低的
}

