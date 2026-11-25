#ifndef _EM_MOTOR_H_
#define _EM_MOTOR_H_

#include "em_config.h"

void motor_run_step(uint32_t steps);//按换相表连续输出 steps 个状态，每个状态之间延时 MOTOR_WATI_TIME。适合按指定步数走纸
void motor_start(void);		//创建并启动周期软件定时器
void motor_stop(void);		//停止这个软件定时器，并把四路电机控制输入写为低电平。
void init_motor(void);		//初始化电机控制输出状态：把四路控制输入写为低电平
void motor_run(void);			//输出换相表当前行，。它只推进一拍，

#endif

