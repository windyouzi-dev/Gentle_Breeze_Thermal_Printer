#include "em_printer.h"
#include "em_timer.h"

float addTime[6] = {0};//定义一个长度为 6 的全局数组，
//分别保存 STB1～STB6 六个加热通道需要增加的加热时间

// 点数-增加时间系数
#define kAddTime 0.001
// 根据打印头实际打印效果修改打印时间偏移值

//六个通道各自的固定补偿时间（某一通道浅色就补偿加热时间）
#define STB1_ADDTIME 0
#define STB2_ADDTIME 0
#define STB3_ADDTIME 0
#define STB4_ADDTIME 0
#define STB5_ADDTIME 0
#define STB6_ADDTIME 0

// 全局打印浓度 （heat_density %）
uint8_t heat_density = 64;


/**
 * @brief 失能所有通道(设置 选通 空闲)
 *
 */
static void set_stb_idle()
{
	HAL_GPIO_WritePin(STB1_GPIO_Port,STB1_Pin,GPIO_PIN_RESET);
  HAL_GPIO_WritePin(STB2_GPIO_Port,STB2_Pin,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(STB3_GPIO_Port,STB3_Pin,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(STB4_GPIO_Port,STB4_Pin,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(STB5_GPIO_Port,STB5_Pin,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(STB6_GPIO_Port,STB6_Pin,GPIO_PIN_RESET);
}

/**
 * @brief 打印前初始化 (升压+锁存信号初始化+通道失能+超时定时器开启)
 *
 */
static void init_printing()
{
    
		// POWER ON
		HAL_GPIO_WritePin(VH_EN_GPIO_Port,VH_EN_Pin,GPIO_PIN_SET);
		//锁存数据信号空闲
		HAL_GPIO_WritePin(LAT_GPIO_Port,LAT_Pin,GPIO_PIN_SET);
		//让所有通道空闲失能
    set_stb_idle();
		// 开启打印超时监听（ 20s ）
    open_printer_timeout_timer();
    
}

/**
 * @brief 打印后停止
 *
 */
static void stop_printing()
{
    
    // POWER OFF
		HAL_GPIO_WritePin(VH_EN_GPIO_Port,VH_EN_Pin,GPIO_PIN_RESET);\
		//锁存数据信号空闲
	  HAL_GPIO_WritePin(LAT_GPIO_Port,LAT_Pin,GPIO_PIN_SET);
		//让所有通道空闲失能
    set_stb_idle();
		// 关闭打印超时监听
    close_printer_timeout_timer();
}

/**
 * @brief 打印密度设置
 *
 * @param density
 */
void set_heat_density(uint8_t density)
{
    printf("打印密度设置 %d\n", density);
    heat_density = density;
}
/**
 * @brief 清除打印补偿时间
 *
 * @param 无
 */
void clearAddTime()
{
    addTime[0] = addTime[1] = addTime[2] = addTime[3] = addTime[4] = addTime[5] = 0;
}

/**
 * @brief 发送一行数据
 *
 * @param data
 */
static void send_one_line_data(uint8_t *data)
{
    float tmpAddTime = 0;
    clearAddTime();//清除打印补偿时间
	
		//48个字节
    for (uint8_t i = 0; i < 6; ++i)
    {
        for (uint8_t j = 0; j < 8; ++j)
        {
            addTime[i] += data[i * 8 + j];//8个字节值之和
        }
        tmpAddTime = addTime[i] * addTime[i];
        addTime[i] = kAddTime * tmpAddTime;//时间K值 × 一个通道8个字节的值之和
    }
		
		HAL_SPI_Transmit(&hspi1, data, TPH_DI_LEN, 0xFFFF);//SPI1句柄，发送地址起始地址，字节长度，超时时间
    
		//发送完数据后进行LAT锁存(高低高 低脉冲)
		HAL_GPIO_WritePin(LAT_GPIO_Port,LAT_Pin,GPIO_PIN_RESET);
		// 保持低电平 LAT_TIME 微秒
    us_delay(LAT_TIME);
		HAL_GPIO_WritePin(LAT_GPIO_Port,LAT_Pin,GPIO_PIN_SET);
}

/**
 * @brief 单通道打印运行
 *
 * @param now_stb_num
 */
static void run_stb(uint8_t now_stb_num)
{
    switch (now_stb_num)
    {
    case 0:
				HAL_GPIO_WritePin(STB1_GPIO_Port,STB1_Pin,GPIO_PIN_SET);
				// （基础打印时间+补偿时间（随字节值）+通道补偿时间）× 打印浓度（%）
        us_delay( (PRINT_TIME + addTime[0] + STB1_ADDTIME) * ((double)heat_density / 100));
				HAL_GPIO_WritePin(STB1_GPIO_Port,STB1_Pin,GPIO_PIN_RESET);
        us_delay(PRINT_END_TIME);//打印冷却时间
        break;
    case 1:
				HAL_GPIO_WritePin(STB2_GPIO_Port,STB2_Pin,GPIO_PIN_SET);
        us_delay((PRINT_TIME + addTime[1] + STB2_ADDTIME) * ((double)heat_density / 100));
				HAL_GPIO_WritePin(STB2_GPIO_Port,STB2_Pin,GPIO_PIN_RESET);
        us_delay(PRINT_END_TIME);
        break;
    case 2:
				HAL_GPIO_WritePin(STB3_GPIO_Port,STB3_Pin,GPIO_PIN_SET);
        us_delay((PRINT_TIME + addTime[2] + STB3_ADDTIME) * ((double)heat_density / 100));
				HAL_GPIO_WritePin(STB3_GPIO_Port,STB3_Pin,GPIO_PIN_RESET);
        us_delay(PRINT_END_TIME);
        break;
    case 3:
				HAL_GPIO_WritePin(STB4_GPIO_Port,STB4_Pin,GPIO_PIN_SET);
        us_delay((PRINT_TIME + addTime[3] + STB4_ADDTIME) * ((double)heat_density / 100));
				HAL_GPIO_WritePin(STB4_GPIO_Port,STB4_Pin,GPIO_PIN_RESET);
        us_delay(PRINT_END_TIME);
        break;
    case 4:
				HAL_GPIO_WritePin(STB5_GPIO_Port,STB5_Pin,GPIO_PIN_SET);
        us_delay((PRINT_TIME + addTime[4] + STB5_ADDTIME) * ((double)heat_density / 100));
				HAL_GPIO_WritePin(STB5_GPIO_Port,STB5_Pin,GPIO_PIN_RESET);
        us_delay(PRINT_END_TIME);
        break;
    case 5:
				HAL_GPIO_WritePin(STB6_GPIO_Port,STB6_Pin,GPIO_PIN_SET);
        us_delay((PRINT_TIME + addTime[5] + STB6_ADDTIME) * ((double)heat_density / 100));
				HAL_GPIO_WritePin(STB6_GPIO_Port,STB6_Pin,GPIO_PIN_RESET);
        us_delay(PRINT_END_TIME);
        break;
    default:
        break;
    }
}

/**
 * @brief 移动电机&开始打印
 *
 * @param need_stop 
 * @param stbnum
 */
bool move_and_start_std(bool need_stop, uint8_t stbnum)
{
    if (need_stop == true)
    {
				printf("打印停止\n");
				motor_stop();//AB通道全是低电平，驱动芯片给步进电机发送待机指令
				stop_printing();//关闭升压，锁存信号高点平复位，通道全关，关闭打印超时定时器
        return true;
    }
    // 4step一行
		//运行一拍
    motor_run();
		
    if (stbnum == ALL_STB_NUM)
    {
        // 所有通道打印
        for (uint8_t index = 0; index < 6; index++)
        {
            run_stb(index);//一个通道打印
            // 把电机运行信号插入通道加热中，减少打印卡顿和耗时
            if (index == 1 || index == 3 || index == 5)
            {
                motor_run();
            }
        }
				// motor_run_step(3);
    }
    else
    {
        // 单通道打印
        run_stb(stbnum);
        motor_run_step(3);//三拍
    }
    return false;
}

/**
 * @brief 打印错误（超时）检查
 * 
 * @param need_report 是否需BLE上报

 * @return true 打印出错
 * @return false 打印正常
 */
bool printing_error_check(bool need_report)
{
    if (get_printer_timeout_status())
    {
				printf("打印超时\n");
        return true;
    }
    if (get_device_state()->paper_state == PAPER_STATUS_LACK)
    {
        if(need_report){
            // 停止打印
            clean_printbuffer();//读写指针，剩余行数归零
            ble_report();
        }
        // 停止打印
        printf("缺纸\n");
        run_beep(BEEP_WARN);
				run_led(LED_WARN);//闪两次
        return true;
    }
    if (get_device_state()->temperature > 65)
    {
        if(need_report){
            // 停止打印
            clean_printbuffer();
            ble_report();
        }
        // 停止打印
        printf("温度异常\n");
        run_beep(BEEP_WARN);
				run_led(LED_WARN);
        return true;
    }
    return false;
}


/**
 * @brief 数组打印
 *
 * @param data
 * @param length 数据长度必须是整行 48*n
 */
void start_printing(uint8_t *data, uint32_t len)
{
    uint32_t offset = 0;//已发送的字节数（/48是行数）
    uint8_t *ptr = data;
    bool need_stop = false;
    init_printing();//电压，锁存，通道，超时
    while (1)
    {
        if (len > offset)
        {
            // 发送一行数据 48byte*8=384bit
            send_one_line_data(ptr);
						
            offset += TPH_DI_LEN; //字节数加48（1行）
            ptr += TPH_DI_LEN;  //指针后移
        }
        else//已经发送完数据了向打印头
            need_stop = true;
        if (move_and_start_std(need_stop, ALL_STB_NUM))//停止，电机待机，降压，锁存初始，选通关，定时器关
            break;
        if(printing_error_check(false))//查看是否打印超时了
            break;
    }
    motor_run_step(40);
    motor_stop();
    printf("打印完成\n");
}

/**
 * @brief 可变队列打印
 *
 */
void start_printing_by_queuebuf( )
{
    uint8_t *pdata = NULL;
		uint32_t printer_count = 0;
    init_printing();
    while (1)
    {
        if (get_ble_rx_leftline() > 0)//还有剩余行数
        {
            pdata = read_to_printer();//读一行，读指针+1
            if (pdata != NULL)
            {
								printer_count ++;
							send_one_line_data(pdata);//计算加热补偿时间，发送（spi句柄，起始地址，长度字节，超时时间），锁存低电平脉冲
                if (move_and_start_std(false, ALL_STB_NUM))//把一行 六个通道进行打印
                    break;
            }
        }
        else
        {
					if (move_and_start_std(true, ALL_STB_NUM))//没有要打印的了，进行电机和打印机停止
                break;
        }
        if(printing_error_check(true))//错误退出，（超时，缺纸，打印头过热）	
            break;
    }
    motor_run_step(140);
    motor_stop();
    clean_blepack_count();
    printf("printer finish !!! 打印机收到的总行数=%lu 实际打印行数:%lu 队列满后丢弃的行数=%lu\n",
                                    (unsigned long)get_blepack_count(),
                                    (unsigned long)printer_count,
                                    (unsigned long)drop_line_cnt);  
    clean_blepack_count();
    drop_line_cnt = 0;                             
}

/**
 * @brief 单通道数组打印
 *
 * @param stbnum
 * @param data
 * @param len
 */
void start_printing_by_onestb(uint8_t stbnum, uint8_t *data, uint32_t len)
{
    uint32_t offset = 0;
    uint8_t *ptr = data;
    bool need_stop = false;
    init_printing();
    while (1)
    {
        printf("printer 已打印的字节 %d\n", offset);
        if (len > offset)
        {
            // 发送一行数据 48byte*8=384bit
            send_one_line_data(ptr);
            offset += TPH_DI_LEN;
            ptr += TPH_DI_LEN;
        }
        else
            need_stop = true;
        if (move_and_start_std(need_stop, stbnum))
            break;
        if(printing_error_check(false))
            break;
    }
		printf("printer end\n");
    motor_run_step(40);
    motor_stop();
		printf("printer end2\n");
}

static void setDebugData(uint8_t *print_data)//5行测试数据
{
    for (uint32_t cleardata = 0; cleardata < 48 * 5; ++cleardata)
    {
        print_data[cleardata] = 0x55;
    }
}
//每个通道各打印 5 行，共测试六个通道
void testSTB()
{
    uint8_t print_data[48 * 6];
    uint32_t print_len;
    printf("开始打印打印头选通引脚测试\n顺序: 1  2  3  4  5  6");
    print_len = 48 * 5;
    setDebugData(print_data);
    start_printing_by_onestb(0, print_data, print_len);
    setDebugData(print_data);
    start_printing_by_onestb(1, print_data, print_len);
    setDebugData(print_data);
    start_printing_by_onestb(2, print_data, print_len);
    setDebugData(print_data);
    start_printing_by_onestb(3, print_data, print_len);
    setDebugData(print_data);
    start_printing_by_onestb(4, print_data, print_len);
    setDebugData(print_data);
    start_printing_by_onestb(5, print_data, print_len);
    printf("测试完成");
}

void init_printer()//仅一开始初始化使用
{
		init_motor();	//A B两H桥通道电平为0，为待机状态

    set_stb_idle();//全部 通道失能
	HAL_GPIO_WritePin(VH_EN_GPIO_Port,VH_EN_Pin,GPIO_PIN_RESET);//不要升压


}
