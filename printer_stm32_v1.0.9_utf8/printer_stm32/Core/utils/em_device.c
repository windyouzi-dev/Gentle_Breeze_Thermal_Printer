#include "em_device.h"

device_state_t g_device_state;

device_state_t * get_device_state(){	//获取设备状态的指针
    return &g_device_state;
}

void set_device_paper_status(paper_state_e status){//将传入的纸张状态更新到全局状态中
    g_device_state.paper_state = status;	
}

void set_read_ble_finish(bool finish){		//更新蓝牙读取完成标志位（true/false）
    g_device_state.read_ble_finish = finish;
}

void init_device_state(){
    memset(&g_device_state,0,sizeof(device_state_t));	//内存清零
	g_device_state.printer_state = PRINTER_STATUS_INIT;	//打印机初始化
    g_device_state.battery = 100;
    g_device_state.temperature = 30;
    g_device_state.paper_state = PAPER_STATUS_NORMAL;		//纸张状态正常
		g_device_state.read_ble_finish = false;
}

