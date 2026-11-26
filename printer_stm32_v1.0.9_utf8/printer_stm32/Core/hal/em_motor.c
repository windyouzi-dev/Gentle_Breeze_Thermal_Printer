#include "em_motor.h"
#include "main.h"

/*
| STM32 		| IC 输入| IC 输出  | 电机 |
| --- 			| --- 	 | ---	 		| ---	 |
| PB9 / IN1 | IND(7) | OUTD(9) 	| M_A−​ |
| PB8 / IN2 | INC(6) | OUTC(12) | M_A+​ |
| PB7 / IN3 | INB(3) | OUTB(13) | M_B−​ |
| PB6 / IN4 | INA(2) | OUTA(16) | M_B+​ |

		对应PIN
电机D  C B  A

		A- A B- B	
		0  1 1  0	进	退		A -> B
		0  0 1  0	待命	退	
		1  0 1  0	退	退	
		1  0 0  0	退  待命
		1  0 0  1	退 	进
		0  0 0  1	待命 进
		0  1 0  1	进  进
		0  1 0  0 进  待命
		4step 行进一点行

*/
uint8_t motor_pos = 0;//步进电机8拍

uint8_t motor_table[8][4] =
    {
        {0, 1, 1, 0},
        {0, 0, 1, 0},
        {1, 0, 1, 0},
        {1, 0, 0, 0},
        {1, 0, 0, 1},
        {0, 0, 0, 1},
        {0, 1, 0, 1},
        {0, 1, 0, 0}
		};

osTimerId myMotorTimerHandle;//定时器句柄
				
void read_motor_timer_callbackfun(void const * argument)//回调函数（定时器配置）
{
	// Serial.println("步进电机定时器运行中...");
   	HAL_GPIO_WritePin(GPIOB,MOTOR_IN1_Pin,(GPIO_PinState)motor_table[motor_pos][0]);
		HAL_GPIO_WritePin(GPIOB,MOTOR_IN2_Pin,(GPIO_PinState)motor_table[motor_pos][1]);
	  HAL_GPIO_WritePin(GPIOB,MOTOR_IN3_Pin,(GPIO_PinState)motor_table[motor_pos][2]);
		HAL_GPIO_WritePin(GPIOB,MOTOR_IN4_Pin,(GPIO_PinState)motor_table[motor_pos][3]);
    motor_pos++;
    if (motor_pos >= 8)
    {
        motor_pos = 0;
    }
}	

void motor_start()
{
	if(myMotorTimerHandle == NULL){//没有定时器对象就创建
		osTimerDef(myMotorTimer, read_motor_timer_callbackfun);//定时器定义 1句柄 2回调函数
		myMotorTimerHandle = osTimerCreate(osTimer(myMotorTimer), osTimerPeriodic, NULL);//定时器创建 ：（
		//取出myMotorTimer这个定义，交给 osTimerCreate()。），周期模式，没有参数给回调
	}
	osTimerStart(myMotorTimerHandle,2); //启动软件定时器对象，周期参数是 2 个 RTOS tick
}

void motor_stop()
{
	HAL_GPIO_WritePin(GPIOB,MOTOR_IN1_Pin,(GPIO_PinState)0);
	HAL_GPIO_WritePin(GPIOB,MOTOR_IN2_Pin,(GPIO_PinState)0);
	HAL_GPIO_WritePin(GPIOB,MOTOR_IN3_Pin,(GPIO_PinState)0);
	HAL_GPIO_WritePin(GPIOB,MOTOR_IN4_Pin,(GPIO_PinState)0);
		if(myMotorTimerHandle != NULL)
			osTimerStop(myMotorTimerHandle);
}

void motor_run()
{
	HAL_GPIO_WritePin(GPIOB,MOTOR_IN1_Pin,(GPIO_PinState)motor_table[motor_pos][0]);
	HAL_GPIO_WritePin(GPIOB,MOTOR_IN2_Pin,(GPIO_PinState)motor_table[motor_pos][1]);
	HAL_GPIO_WritePin(GPIOB,MOTOR_IN3_Pin,(GPIO_PinState)motor_table[motor_pos][2]);
	HAL_GPIO_WritePin(GPIOB,MOTOR_IN4_Pin,(GPIO_PinState)motor_table[motor_pos][3]);
    motor_pos++;
    if (motor_pos >= 8)
    {
        motor_pos = 0;
    }
}

void motor_run_step(uint32_t steps)
{
    while (steps)
    {
			HAL_GPIO_WritePin(GPIOB,MOTOR_IN1_Pin,(GPIO_PinState)motor_table[motor_pos][0]);
			HAL_GPIO_WritePin(GPIOB,MOTOR_IN2_Pin,(GPIO_PinState)motor_table[motor_pos][1]);
			HAL_GPIO_WritePin(GPIOB,MOTOR_IN3_Pin,(GPIO_PinState)motor_table[motor_pos][2]);
			HAL_GPIO_WritePin(GPIOB,MOTOR_IN4_Pin,(GPIO_PinState)motor_table[motor_pos][3]);
			
        motor_pos++;
        if (motor_pos >= 8)
        {
            motor_pos = 0;
        }
        us_delay(MOTOR_WATI_TIME);//电机相位之间延时
        steps--;
    }
}

void init_motor()
{
	HAL_GPIO_WritePin(GPIOB,MOTOR_IN1_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB,MOTOR_IN2_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB,MOTOR_IN3_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB,MOTOR_IN4_Pin, GPIO_PIN_RESET);
}
