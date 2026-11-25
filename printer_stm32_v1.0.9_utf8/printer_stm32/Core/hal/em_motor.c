#include "em_motor.h"
#include "main.h"

static void digitalWrite(int pin,int PinState){
	if(pin == PIN_MOTOR_AP){
		HAL_GPIO_WritePin(GPIOB,MOTOR_IN1_Pin,(GPIO_PinState)PinState);
	}else if(pin == PIN_MOTOR_AM){
		HAL_GPIO_WritePin(GPIOB,MOTOR_IN2_Pin,(GPIO_PinState)PinState);
	}else if(pin == PIN_MOTOR_BP){
		HAL_GPIO_WritePin(GPIOB,MOTOR_IN3_Pin,(GPIO_PinState)PinState);
	}else if(pin == PIN_MOTOR_BM){
		HAL_GPIO_WritePin(GPIOB,MOTOR_IN4_Pin,(GPIO_PinState)PinState);
	}
}

/*
| STM32 		| IC 输入| IC 输出  | 电机 |
| --- 			| --- 	 | ---	 		| ---	 |
| PB9 / IN1 | IND(7) | OUTD(9) 	| M_A−​ |
| PB8 / IN2 | INC(6) | OUTC(12) | M_A+​ |
| PB7 / IN3 | INB(3) | OUTB(13) | M_B−​ |
| PB6 / IN4 | INA(2) | OUTA(16) | M_B+​ |
*/
uint8_t motor_pos = 0;

uint8_t motor_table[8][4] =
    {
        {0, 1, 1, 0},
        {0, 0, 1, 0},
        {1, 0, 1, 0},
        {1, 0, 0, 0},
        {1, 0, 0, 1},
        {0, 0, 0, 1},
        {0, 1, 0, 1},
        {0, 1, 0, 0}};

osTimerId myMotorTimerHandle;
				
void read_motor_timer_callbackfun(void const * argument)
{
    // Serial.println("timer_motor is running...");
    digitalWrite(PIN_MOTOR_AP, motor_table[motor_pos][0]);
    digitalWrite(PIN_MOTOR_AM, motor_table[motor_pos][1]);
    digitalWrite(PIN_MOTOR_BP, motor_table[motor_pos][2]);
    digitalWrite(PIN_MOTOR_BM, motor_table[motor_pos][3]);
    motor_pos++;
    if (motor_pos >= 8)
    {
        motor_pos = 0;
    }
}	

void motor_start()
{
	if(myMotorTimerHandle == NULL){
		osTimerDef(myMotorTimer, read_motor_timer_callbackfun);
		myMotorTimerHandle = osTimerCreate(osTimer(myMotorTimer), osTimerPeriodic, NULL);
	}
	osTimerStart(myMotorTimerHandle,2); 
}

void motor_stop()
{
    digitalWrite(PIN_MOTOR_AP, 0);
    digitalWrite(PIN_MOTOR_AM, 0);
    digitalWrite(PIN_MOTOR_BP, 0);
    digitalWrite(PIN_MOTOR_BM, 0);
		if(myMotorTimerHandle != NULL)
			osTimerStop(myMotorTimerHandle);
}

void motor_run()
{
    digitalWrite(PIN_MOTOR_AP, motor_table[motor_pos][0]);
    digitalWrite(PIN_MOTOR_AM, motor_table[motor_pos][1]);
    digitalWrite(PIN_MOTOR_BP, motor_table[motor_pos][2]);
    digitalWrite(PIN_MOTOR_BM, motor_table[motor_pos][3]);
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
        digitalWrite(PIN_MOTOR_AP, motor_table[motor_pos][0]);
        digitalWrite(PIN_MOTOR_AM, motor_table[motor_pos][1]);
        digitalWrite(PIN_MOTOR_BP, motor_table[motor_pos][2]);
        digitalWrite(PIN_MOTOR_BM, motor_table[motor_pos][3]);
        motor_pos++;
        if (motor_pos >= 8)
        {
            motor_pos = 0;
        }
        us_delay(MOTOR_WATI_TIME);
        steps--;
    }
}

void init_motor()
{
    digitalWrite(PIN_MOTOR_AP, 0);
    digitalWrite(PIN_MOTOR_AM, 0);
    digitalWrite(PIN_MOTOR_BP, 0);
    digitalWrite(PIN_MOTOR_BM, 0);
}
