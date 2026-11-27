#ifndef _EM_BLE_H_
#define _EM_BLE_H_


#include "em_config.h"
#include "em_device.h"
#include "em_sensors.h"
#include "em_queue.h"
#include "em_printer.h"

extern volatile uint32_t drop_line_cnt;
#define Service_UUID 6E400001B5A3F393E0A9E50E24DCCA9E
#define BLE_Rx_UUID  6E400002B5A3F393E0A9E50E24DCCA9E
#define BLE_Tx_UUID  6E400003B5A3F393E0A9E50E24DCCA9E
#define AT_option_UUID  6E400004B5A3F393E0A9E50E24DCCA9E
/**
 * @brief 初始化BLE
 * 
 */
void init_ble(void);

/**
 * @brief 清空接收包统计
 * 
 */
void clean_blepack_count(void);

/**
 * 获取接收行数
*/
uint32_t get_blepack_count(void);

void ble_status_data_clean(void);

void uart_cmd_handle(uint8_t data);

/**
 * @brief Get the ble connect object
 * 
 * @return true 
 * @return false 
 */
bool get_ble_connect(void);

/**
 * @brief 
 * 
 */
void ble_report(void);


#endif

