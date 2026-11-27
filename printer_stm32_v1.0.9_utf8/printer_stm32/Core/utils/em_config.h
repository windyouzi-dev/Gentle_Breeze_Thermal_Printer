#ifndef _EM_CONFIG_H_
#define _EM_CONFIG_H_

	#include "stm32f1xx_hal.h"

	#include "FreeRTOS.h"
	#include "cmsis_os.h"			//osDelay等

	#include <string.h>				//memcpy
	#include <stdbool.h>			//bool
	#include <stdio.h>				//printf

	#define _FREERTOS_OPEN_ 

	#define BLE_NAME "Wind-Printer"

	//接收完成所有数据才开始打印
	#define START_PRINTER_WHEN_FINISH_RAED 1

	#ifdef _FREERTOS_OPEN_
	// #define MOTOR_WATI_TIME 4  	//ms
	// #define PRINT_TIME 2       	//ms
	// #define PRINT_END_TIME 2  	  //ms
	#define PRINT_TIME 2600         //打印加热时间
	#define PRINT_END_TIME 200      //冷却时间
	#define MOTOR_WATI_TIME 4000    //电机一步时间
	#define LAT_TIME 1              //数据锁存时间
	#else
	//打印头电机参数
	#define PRINT_TIME 1700         //打印加热时间
	#define PRINT_END_TIME 200      //冷却时间
	#define MOTOR_WATI_TIME 4000    //电机一步时间
	#define LAT_TIME 1              //数据锁存时间
	#endif


	#include "tim.h"
	#define DLY_TIM_Handle (&htim1)  // Timer handle
	#define hal_delay_us(nus) do { \
			__HAL_TIM_SET_COUNTER(DLY_TIM_Handle, 0); \
			__HAL_TIM_ENABLE(DLY_TIM_Handle); \
			while (__HAL_TIM_GET_COUNTER(DLY_TIM_Handle) < (nus)); \
			__HAL_TIM_DISABLE(DLY_TIM_Handle); \
	} while(0)


	#ifdef _FREERTOS_OPEN_
	#define us_delay(ms) hal_delay_us(ms)
	//#define us_delay(ms) delayMicroseconds(ms)
	#else
	#define us_delay(ms) delayMicroseconds(ms)
	#endif


#endif
