#include "em_ble.h"

bool need_reboot_ble = false;
bool bleConnected = false;
uint32_t packcount = 0;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart1;
volatile uint32_t drop_line_cnt = 0;//丢行计数

// AT指令字符串 \r\n是规定的结尾（CRLF）回车换行

char *ble_in_at_mode = "+++";				//让蓝牙模块进入 AT 指令模式
char *ble_close_status = "AT+STATUS=0\r\n"; //关闭设备状态显示
char *ble_query_status = "AT+STATUS?\r\n"; //查询设备状态显示
char *ble_query_name = "AT+NAME?\r\n"; // 查询蓝牙广播名称 
char *ble_set_name = "AT+NAME=Wind-Printer\r\n"; //修改名称
char *ble_out_at_mode = "AT+EXIT\r\n";	//退出AT指令模式

typedef enum{
        BLE_INIT_START = 0,
        BLE_IN_AT_MODE,
        BLE_IN_AT_MODE_SUCCESS,
        BLE_CLOSE_STATUS,						//关闭状态
        BLE_CLOSE_STATUS_SUCCESS,
				BLE_QUERY_STATUS,
				BLE_QUERY_STATUS0_SUCCESS,
        BLE_QUERY_NAME,
        BLE_NEED_SET_NAME,
        BLE_NONEED_SET_NAME,
        BLE_SET_NAME,
        BLE_SET_NAME_SUCCESS,
        BLE_OUT_AT_MODE,
        BLE_INIT_FINISH,
				BLE_RESET,
}e_ble_init_step;

e_ble_init_step g_ble_init_step = BLE_INIT_START;



void clean_blepack_count(){ //清空-收包计数
    packcount = 0;
}


uint32_t get_blepack_count(){//取数-收包计数
    return packcount;
}

bool get_ble_connect(){//获取-蓝牙连接状态
    return bleConnected;
}

void ble_report(){//上报 -设备状态
    if (get_ble_connect()){
        device_state_t *pdevice = get_device_state();//获取-设备状态指针
        uint8_t status[4];
        status[0] = pdevice->battery;
        status[1] = pdevice->temperature;
        status[2] = pdevice->paper_state;
        status[3] = pdevice->printer_state;
			HAL_UART_Transmit(&huart2,(uint8_t*)&status,sizeof(status),0xffff);//向手机发送设备状态
    }
}

int cmd_index = 0;//已收字节数
uint8_t cmd_buffer[100];
bool need_update_led_status = false;

void uart_cmd_handle(uint8_t data){//接收命令并解析
		cmd_buffer[cmd_index++] = data;
		char *ptr_char = (char*)cmd_buffer;
						if(g_ble_init_step == BLE_INIT_FINISH){//如果初始化结束（业务模式）
									if(strstr(ptr_char, "CONNECTED") != NULL){//
										need_update_led_status = true;
										run_led(LED_CONNECT);
									}
									if(strstr(ptr_char, "DISCONNECTED") != NULL){
										need_update_led_status = true;
										run_led(LED_DISCONNECT);
									}
									if(strstr(ptr_char, "DEVICE ERROR") != NULL){
										need_update_led_status = true;
										run_led(LED_CONNECT);
									}
									if(cmd_index == 5){//如果接到了5个字节
											 if(cmd_buffer[0] == 0xA5 && cmd_buffer[1] == 0xA5 && 
												 cmd_buffer[2] == 0xA5 && cmd_buffer[3] == 0xA5)//确认前四个为A5,是浓度设置
												 {
														 if(cmd_buffer[4] == 1){
																	 set_heat_density(30);// 30% 低档
														 }
														 else if(cmd_buffer[4] == 2){
																	 set_heat_density(60); // 60% 中档
														 }
														 else{
																	 set_heat_density(100); // 100% 高档
														 }
														 cmd_index = 0;													//指针复位
														 memset(cmd_buffer,0,sizeof(cmd_buffer));//内存内容重置为零
														 return;
													}
											 if(cmd_buffer[0] == 0xA6 && cmd_buffer[1] == 0xA6 
												 && cmd_buffer[2] == 0xA6 && cmd_buffer[3] == 0xA6)//前四个是A6,蓝牙接收完毕
											 {
														set_read_ble_finish(true);//设置设备状态，蓝牙读取完毕
														//printf("---->read finish 1 = %d\n",packcount);
													
														cmd_index = 0;										//指针和内存复位
														memset(cmd_buffer,0,sizeof(cmd_buffer));
														return;
											 }
									}
									if(cmd_index >= 5){//冗余设置
											if(cmd_buffer[cmd_index-1] == 0x01){
													if(cmd_buffer[cmd_index-2] == 0xA6 && cmd_buffer[cmd_index-3] == 0xA6 &&
														cmd_buffer[cmd_index-4] == 0xA6 && cmd_buffer[cmd_index-5] == 0xA6){
														//printf("---->read finish 2 = %d\n",packcount);
															
														cmd_index = 0;
														memset(cmd_buffer,0,sizeof(cmd_buffer));
														set_read_ble_finish(true);
														return;
													}
											}
									}
									if (cmd_index >= 48) {
							packcount++;//收到一行

							if (!write_to_printbuffer(cmd_buffer, cmd_index)) {//没成功写入
								drop_line_cnt++;//丢弃行数+1
							}

							cmd_index = 0;
							memset(cmd_buffer, 0, sizeof(cmd_buffer));
						}
						}else{		//(AT配置模式)					
                if(strstr(ptr_char, "OK\r\n") != NULL){//C 标准库的子串查找函数,指向首次出现位置的 
										if(g_ble_init_step == BLE_IN_AT_MODE){
														g_ble_init_step = BLE_IN_AT_MODE_SUCCESS;
										}
										else if(g_ble_init_step == BLE_CLOSE_STATUS){
														g_ble_init_step = BLE_CLOSE_STATUS_SUCCESS;
										}
										else if(g_ble_init_step == BLE_QUERY_NAME){
											if(strstr(ptr_char,"RF-CRAZY") != NULL){//看蓝牙名字是否仍为出场设置的名字
																		g_ble_init_step = BLE_NEED_SET_NAME;
														}else{
																		g_ble_init_step = BLE_NONEED_SET_NAME;
														}
										}
										else if(g_ble_init_step == BLE_SET_NAME){
														g_ble_init_step = BLE_SET_NAME_SUCCESS;
										}
										else if(g_ble_init_step == BLE_OUT_AT_MODE){
														g_ble_init_step = BLE_INIT_FINISH;//收到“退出 AT 模式”的 OK 后，将状态设置为 BLE_INIT_FINISH
										}
										else if(g_ble_init_step == BLE_RESET){
														g_ble_init_step = BLE_INIT_START;
										}
										else if(g_ble_init_step == BLE_QUERY_STATUS){
														if(strstr(ptr_char,"AT+STATUS=0") != NULL){//状态显示功能关闭
																		g_ble_init_step = BLE_QUERY_STATUS0_SUCCESS;
														}else{
																		g_ble_init_step = BLE_CLOSE_STATUS;// 没关掉，回去重发
														}
										}
										cmd_index = 0;
										memset(cmd_buffer,0,sizeof(cmd_buffer));
										return;
                }
								if(strstr(ptr_char, "ERROR\r\n") != NULL){
									g_ble_init_step = BLE_RESET;
								}
								if(cmd_index >= sizeof(cmd_buffer)){//数组溢出_复位
										cmd_index = 0;
								}
        }
}
//int retry_count = 0;

void init_ble()
{
                while(1){
//												retry_count ++;
                        vTaskDelay(50);
                        if(g_ble_init_step == BLE_INIT_START ||
													g_ble_init_step == BLE_IN_AT_MODE){
																printf("BLE:正进入AT模式\n");
                                HAL_UART_Transmit(&huart2, //通过串口2配置蓝牙模块
														(uint8_t*)ble_in_at_mode, strlen(ble_in_at_mode), 0xffff);
                                g_ble_init_step = BLE_IN_AT_MODE;//更改蓝牙模块状态
                        }else if(g_ble_init_step == BLE_IN_AT_MODE_SUCCESS ||
													g_ble_init_step == BLE_CLOSE_STATUS){
																printf("BLE:正设置status为0 关闭状态显示\n");
                                HAL_UART_Transmit(&huart2, (uint8_t*)ble_close_status, strlen(ble_close_status), 0xffff);
                                g_ble_init_step = BLE_CLOSE_STATUS;
                        }else if(g_ble_init_step == BLE_CLOSE_STATUS_SUCCESS ||
													g_ble_init_step == BLE_QUERY_STATUS){
																printf("BLE:正查询状态是否为0\n");
                                HAL_UART_Transmit(&huart2, 
														(uint8_t*)ble_query_status, strlen(ble_query_status), 0xffff);
                                g_ble_init_step = BLE_QUERY_STATUS;
                        }else if(g_ble_init_step == BLE_QUERY_STATUS0_SUCCESS ||
													g_ble_init_step == BLE_QUERY_NAME){
																printf("BLE:正查询设备名称\n");
                                HAL_UART_Transmit(&huart2, 
														(uint8_t*)ble_query_name, strlen(ble_query_name), 0xffff);
                                g_ble_init_step = BLE_QUERY_NAME;
                        }
												else if(g_ble_init_step == BLE_NEED_SET_NAME || 
													g_ble_init_step == BLE_SET_NAME){
																printf("BLE:正设置设备名称\n");
                                HAL_UART_Transmit(&huart2, 
														(uint8_t*)ble_set_name, strlen(ble_set_name), 0xffff);
                                g_ble_init_step = BLE_SET_NAME;
                                need_reboot_ble = true;
                        }else if(g_ble_init_step == BLE_SET_NAME_SUCCESS ||
													g_ble_init_step == BLE_NONEED_SET_NAME || g_ble_init_step == BLE_OUT_AT_MODE){
																printf("BLE:正退出AT模式\n");
                                HAL_UART_Transmit(&huart2, 
														(uint8_t*)ble_out_at_mode, strlen(ble_out_at_mode), 0xffff);
                                g_ble_init_step = BLE_OUT_AT_MODE;
                        }else if(g_ble_init_step == BLE_INIT_FINISH){
                                break;
                        }else if(g_ble_init_step == BLE_RESET){
																printf("BLE:BLE RESET 退出AT模式\n");
																HAL_UART_Transmit(&huart2,
													(uint8_t*)ble_out_at_mode, strlen(ble_out_at_mode), 0xffff);
                                
												}
												printf("g_ble_init_step = %d\n",g_ble_init_step);
												run_led(LED_BLE_INIT);
                }
                if(need_reboot_ble){
                        printf("配置完成-请重启设备使用\n");
                }else{
                        printf("配置完成-可以正常使用\n");
												
                }
								vTaskDelay(1000);
								
								cmd_index = 0;
								memset(cmd_buffer,0,sizeof(cmd_buffer));
}

//这步操作是因为厂家的蓝牙模组，现在status只关了busy、connect timeout、device start、wake up
//所以需要把CONNECTED DISCONNECTED DEVICE ERROR这些业务无关数据清掉
void ble_status_data_clean(){
	if(need_update_led_status){
		vTaskDelay(200);
		printf("clean --->%s\n",cmd_buffer);//把收到的字节展示出来
		
		cmd_index = 0;
		memset(cmd_buffer,0,sizeof(cmd_buffer));
		need_update_led_status = false;
	}
	
}
