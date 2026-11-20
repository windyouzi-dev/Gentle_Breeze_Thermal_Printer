# 微风迷你打印机

基于 STM32F103CBT6 的蓝牙热敏小票打印机，包含固件、PCB、结构件与生产资料。

手机端通过 BLE 下发点阵数据，MCU 驱动 JX-2R-01 热敏打印头（384 点/行）与步进电机，在 48mm 热敏纸上完成打印。

## 功能分工

| 部分 | 负责内容 |
|---|---|
| **软件（固件）** | CubeMX + HAL 外设初始化；FreeRTOS 任务调度；BLE 数据接收与协议解析；打印时序（SPI 移位 → LAT 锁存 → STB 分段加热）；步进电机走纸；按键、LED、电量与温度采集 |
| **硬件** | STM32 最小系统；1S 锂电池升压至 7.2V（VH）供打印头与电机，3.3V 供 MCU；打印头 FPC 接口；步进电机驱动；BLE 模块与调试串口 |
| **上位机** | 手机端 App：搜索并连接 BLE、编辑打印内容、下发点阵数据、显示连接状态。本目录未包含上位机源码 |

## 引脚定义

| 引脚 | 连接对象 | 作用 |
|---|---|---|
| PA0 | 电池分压电路 | ADC 采集电池电压，估算电量 |
| PA1 | NTC 热敏电阻 | ADC 采集打印头温度 |
| PA2 | 蓝牙模块 RX | USART2_TX，MCU → 蓝牙 |
| PA3 | 蓝牙模块 TX | USART2_RX，蓝牙 → MCU |
| PA4 | 按键 | 低电平有效，短按 / 长按检测 |
| PA5 | 打印头 SCK | SPI1 时钟 |
| PA7 | 打印头 DATA | SPI1 MOSI，输出点阵数据 |
| PA8 | 打印头 VHINT | 打印头异常 / 过温中断输入 |
| PA9 / PA10 | 调试串口 | USART1 TX / RX，115200 |
| PA11 | VH_EN | 打印头 7.2V 电源使能 |
| PB0 | LED | 运行 / 连接 / 报警指示，低电平亮 |
| PB1 | 打印头 LAT | 锁存一行 384 点数据 |
| PB6 ~ PB9 | 电机驱动 IN4 ~ IN1 | 两相步进电机四相控制 |
| PB10 ~ PB15 | 打印头 STB1 ~ STB6 | 6 段加热选通，分段加热 |

> `main.h` 中 PA2 / PA3 的宏名为 `USART2_BLE_RX / TX`，是按蓝牙模块一侧命名的交叉连接，实际方向以上表为准。

## 目录结构

```
printer_stm32_v1.0.9_utf8/printer_stm32/   固件工程（Keil MDK，源码 UTF-8）
PCB工程文件/                                立创EDA专业版工程（.epro2）
BOM清单/                                    板级 BOM 与元器件清单
3D打印文件/                                 外壳 STL 与齿刀 DXF
打印头资料/                                 JX-2R-01 规格书
```

## 编译与烧录

- Keil MDK 打开 `printer_stm32_v1.0.9_utf8/printer_stm32/MDK-ARM/printer.uvprojx`
- 编译后通过 J-Link（SWD）或者 ST-Link 烧录
- 源码为 UTF-8 编码，可直接查看中文注释
