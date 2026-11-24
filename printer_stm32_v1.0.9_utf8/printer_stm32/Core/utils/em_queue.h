#ifndef _EM_QUEUE_H_
#define _EM_QUEUE_H_

#include "em_config.h"

//1行容量byte
#define MAX_ONELINE_BYTE 48
//最大行数
#define MAX_LINE 275

typedef struct{
    uint8_t buffer[MAX_ONELINE_BYTE];
}ble_rx_buffer_t;

typedef struct{
    ble_rx_buffer_t printer_buffer[MAX_LINE];//二维数组
    uint32_t r_index;		//读指针
    uint32_t w_index;		//写指针
    uint32_t left_line; //剩余没打印的行数
}ble_rx_t;

void write_to_printbuffer(uint8_t *pdata,size_t length);

uint32_t get_ble_rx_leftline(void);

void clean_printbuffer(void);

uint8_t * read_to_printer(void);

void init_queue(void);

#endif

