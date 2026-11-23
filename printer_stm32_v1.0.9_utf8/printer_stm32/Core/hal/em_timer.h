#ifndef _EM_TIMER_H_
#define _EM_TIMER_H_

#include "em_config.h"

/**
 * @brief 初始化Timer
 * 
 */
void init_timer(void);

/**
* @brief 获取状态上报定时器的 到点 情况
 * 
 * @return true 
 * @return false 
 */
bool get_state_timeout(void);

/**
* @brief 清掉状态上报定时器的旧状态
 * 
 */
void clean_state_timeout(void);

/**
 * @brief 开启打印超时计时器（打印任务开始时调用）
 * 
 */
void open_printer_timeout_timer(void);

/**
 * @brief 关闭打印超时计时器（ 打印正常结束时调用）
 * 
 */
void close_printer_timeout_timer(void);

/**
 * @brief 获取打印超时计时器情况
 * 
 * @return true  打印任务超时 失败
 * @return false  = 尚未超时（可能是正在打印）
 */
bool get_printer_timeout_status(void);

#endif
