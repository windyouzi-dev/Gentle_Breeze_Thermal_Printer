#include "em_timer.h"    
#include "em_config.h"   // 工程总配置。这里主要是为了拿到 CMSIS-RTOS 的类型定义（osTimerId、osTimerDef 等）

bool read_state_timeout = false;   // 周期上报 10s 定时器 标志
bool printer_timeout = false;      // 打印超时标志：20s 一次性定时器置 true → 打印循环看到后判定"卡死"并退出

osTimerId myStateTimerHandle;      // 周期定时器句柄（凭据）
osTimerId myTimeoutTimerHandle;    // 打印超时定时器句柄

//状态上报定时器对象
void read_state_timer_callbackfun(void const * argument)  // 读状态定时器——回调函数
{
  printf("正在读取设备状态...\r\n");   
  read_state_timeout = true;         
}

void init_timer(){  
	osTimerDef(myStateTimer, read_state_timer_callbackfun);  //定义一个系统定时器对象（定时器规格名，实际回调函数）（仅定义 不占用实际内存）
  myStateTimerHandle = osTimerCreate(osTimer(myStateTimer), osTimerPeriodic, NULL); //osTimerCreate(取规格卡, 模式（周期调用，到点后自动装填）, 参数) 并且返回句柄
	osTimerStart(myStateTimerHandle,10000); //开始计时 10000 tick,
}

bool get_state_timeout(){      
    return read_state_timeout;  
}

void clean_state_timeout(){     
    read_state_timeout = false; 
}

//超时定时器对象
void read_timeout_timer_callbackfun(void const * argument)  // 打印超时回调，一次性触发
{
  printf("触发打印超时错误...\r\n");  // 20 秒还没走完，就认定为卡住
  printer_timeout = true;             // 只置标志。不在这里停机，是因为定时器任务里不该直接操作电机和打印头
}

bool get_printer_timeout_status(){  // 打印循环每打一行都查一次（em_printer.c:318、356）
  return printer_timeout;         
}

void open_printer_timeout_timer(){  // 开始打印时调用
  printer_timeout = false;          // 先清上次遗留的标志，否则一进循环就被判超时
	osTimerDef(myTimeoutTimer, read_timeout_timer_callbackfun);  // 定义第二个定时器对象
  myTimeoutTimerHandle = osTimerCreate(osTimer(myTimeoutTimer), osTimerOnce, NULL);  // osTimerOnce = 只触发一次，触发后不自动重装
	osTimerStart(myTimeoutTimerHandle,20000);  // 20 秒（20000 tick），给整次打印一个总时限
}

void close_printer_timeout_timer(void)  // 打印正常结束时调用（stop_printing 里，em_printer.c:75）
{
    if (myTimeoutTimerHandle != NULL)
    {
        osTimerDelete(myTimeoutTimerHandle);// 删除定时器并释放它占的堆内存（configTOTAL_HEAP_SIZE 只有 3072B，必须还回去）
        myTimeoutTimerHandle = NULL;   /* 立即作废，防止二次使用 */
    }
}

