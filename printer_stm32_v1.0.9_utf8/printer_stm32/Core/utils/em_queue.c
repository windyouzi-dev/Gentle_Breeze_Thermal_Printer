#include "em_queue.h"
#include "semphr.h"

ble_rx_t g_ble_rx;

//SemaphoreHandle_t xHandler = NULL;//FreeRTOS 中的信号量句柄类型

void init_queue()
{
		// clean_printbuffer();
    g_ble_rx.w_index = 0;
    g_ble_rx.r_index = 0;
    g_ble_rx.left_line = 0;
    //xHandler = xSemaphoreCreateMutex();// 创建一个互斥量，返回有效句柄
}

/**
* @brief 通过蓝牙向打印缓冲区写入一行数据
 *
 * @param pdata 	数据地址
 * @param length	字节数
 */
bool write_to_printbuffer(uint8_t *pdata, size_t length)
{
    if (pdata == NULL || length == 0)
        return false;

    if (g_ble_rx.left_line >= MAX_LINE)
        return false;

    if (length > MAX_ONELINE_BYTE)
        length = MAX_ONELINE_BYTE;

    memcpy(&g_ble_rx.printer_buffer[g_ble_rx.w_index], pdata, length);

    g_ble_rx.w_index++;
    g_ble_rx.left_line++;

    if (g_ble_rx.w_index >= MAX_LINE)
        g_ble_rx.w_index = 0;

    return true;
}


// void  write_to_printbuffer(uint8_t *pdata, size_t length)
// {
// 		static BaseType_t xHigherPriorityTaskWoken;//FRTOS中长整形的变量，看有没有更高优先级任务被触发
//     if (length == 0)
//         return;
//     if (g_ble_rx.left_line >= MAX_LINE)
//         return;
//     if (length > MAX_ONELINE_BYTE)//行截断
//         length = MAX_ONELINE_BYTE;
//     // 查看是否可以 从中断获得信号量
		
// 		//	xSemaphoreTakeFromISR(xHandler（要申请的那把锁）,
// 													//&xHigherPriorityTaskWoken（传入一个变量的地址，用于接收函数回传的结果：
// 																											//是否唤醒了更高优先级的任务）)
		
//     if (xSemaphoreTakeFromISR(xHandler, &xHigherPriorityTaskWoken) == pdPASS)	//pdPASS  操作成功	pdFALSE  操作失败
//     {
// 				memcpy(&g_ble_rx.printer_buffer[g_ble_rx.w_index], pdata, length);//内存复制 入队
//         g_ble_rx.w_index++;		//写指针加1																						
//         g_ble_rx.left_line++;	//剩余已经接收，但未打印的	行+1
//         if (g_ble_rx.w_index >= MAX_LINE)
//             g_ble_rx.w_index = 0;
//         if (g_ble_rx.left_line > MAX_LINE)
//             g_ble_rx.left_line = MAX_LINE;
//         xSemaphoreGiveFromISR(xHandler,&xHigherPriorityTaskWoken);//失败是errQUEUE_FULL
// 				/*
// 				*	① 归还互斥量
// 					把 xHandler 标记成“可用”，其他执行流可以拿

// 					② 唤醒等待者
// 					如果有任务正阻塞在这把锁上，内核把它从阻塞态移到就绪态				
// 				*/
//     }
		
// 			// 若 参数 为真 pdTRUE（有高优先级的任务比打断的任务还紧急），就写 PendSV 寄存器，请求中断退出时换任务
// 			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
// 			// 硬件平台 让出（控制权）从中断里
		
// }

/**
 * @brief 打印头从缓存中读取一行数据
 *
 * @return uint8_t*
 */
uint8_t *read_to_printer(void)
{
    static uint8_t line_buf[MAX_ONELINE_BYTE];
    uint8_t *ret = NULL;

    taskENTER_CRITICAL();//进入临界区
    if (g_ble_rx.left_line > 0) {
        g_ble_rx.left_line--;

        memcpy(line_buf, g_ble_rx.printer_buffer[g_ble_rx.r_index].buffer, MAX_ONELINE_BYTE);//读取一行数据

        g_ble_rx.r_index++;
        if (g_ble_rx.r_index >= MAX_LINE) g_ble_rx.r_index = 0;
        ret = line_buf;//返回行的首地址
    }
    taskEXIT_CRITICAL();//退出临界区
    return ret;
}

// uint8_t * read_to_printer()
// {
//     uint32_t index = 0;
//     // 查看任务中是否可以获得信号量，如果信号量不可用，则用10个时钟滴答来查看信号量是否可用
//     if (xSemaphoreTake(xHandler, (portTickType)10) == pdPASS)
// 			// 互斥锁 ，（强转成tick单位）超时时间
// 		/*
// 		锁空闲 							→ 立刻拿到，返回 pdPASS；
// 		锁被别人占着 				→ 当前任务阻塞、让出 CPU，最多等 10ms；
// 		等满 10ms 还没拿到	→ 返回 pdFAIL。
// 		*/
//     {
//         if (g_ble_rx.left_line > 0)//有数据
//         {
//             g_ble_rx.left_line--;
//             index = g_ble_rx.r_index;
//             g_ble_rx.r_index++;
					
//             if (g_ble_rx.r_index >= MAX_LINE)//防止取指针越界
//                 g_ble_rx.r_index = 0;
//             xSemaphoreGive(xHandler);
// 						//这个不用自己用portYIELD切换，内核会切。但是中断里需要自己切换
//             return g_ble_rx.printer_buffer[index].buffer;//返回行的首地址
//         }
//         xSemaphoreGive(xHandler);
//         return NULL;//NULL 就是 (void*)0
//     }else//没拿到互斥锁
//         return NULL;
// }

/**
 * @brief 清空接收缓存
 *
 */
void clean_printbuffer()//清空缓存区
{
    taskENTER_CRITICAL();//进入临界区
    g_ble_rx.w_index = 0;
    g_ble_rx.r_index = 0;
    g_ble_rx.left_line = 0;
    taskEXIT_CRITICAL();//退出临界区
}

uint32_t get_ble_rx_leftline()//还有多少行没有打印
{
    return g_ble_rx.left_line;
}
