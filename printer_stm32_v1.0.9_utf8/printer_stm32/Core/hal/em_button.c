#include "em_button.h"
#include "em_task.h"

//短按时间为 < 1s
#define SHORT_PRESS_TIME 1000

bool keyIsPress = false;
unsigned long clicktime = 0;	//记录按键按下的系统毫秒数
bool longPressIsStart = false;

void key_check_run(){
  if(keyIsPress == false){	//没按下，判断按键状态
    if( HAL_GPIO_ReadPin(BTN_GPIO_Port,BTN_Pin) == LOW)
		{
      vTaskDelay(10);
      if( HAL_GPIO_ReadPin(BTN_GPIO_Port,BTN_Pin) == LOW)
			{
        keyIsPress = true;
        clicktime = HAL_GetTick();
      }
    }
  }
  if(keyIsPress == true){		//按下了。
    if( HAL_GPIO_ReadPin(BTN_GPIO_Port,BTN_Pin) == HIGH)
		{
      if(HAL_GetTick() - clicktime > SHORT_PRESS_TIME)//松开按键后发现上一个是长按
			{
        printf("按键被长按——现在松开");
        key_long_click_free_handle();
        longPressIsStart = false;//清除长按标志
      }
			else
			{
        key_click_handle();
      }
      keyIsPress = false;
    }
		else
		{
      if(HAL_GetTick() - clicktime > SHORT_PRESS_TIME)
			{
        if(longPressIsStart == false)
				{
          key_long_click_handle();
          longPressIsStart = true;
        }
      }
    }
  }
}

