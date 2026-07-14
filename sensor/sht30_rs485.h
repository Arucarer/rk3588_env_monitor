/******************************************************************************
 * @file    rs485_sht30.h
 * @brief   RS485 SHT30 温湿度传感器接口定义
 *
 * @author  李宇坤
 * @date    2026-07
 * @version V1.0
 *
 * @details
 * 本文件定义 RS485 SHT30 温湿度传感器的应用层访问接口。
 *
 * 该传感器通过 RS485 接口与 RK3588B 开发板通信，采用标准
 * Modbus RTU 协议。传感器内部使用 SHT30 温湿度采集芯片，
 * 温度和湿度数据通过输入寄存器读取。
 *
 * 主要功能包括：
 * 1. 初始化 RS485 温湿度传感器；
 * 2. 读取温度和湿度原始寄存器数据；
 * 3. 将原始寄存器数据转换为实际温湿度；
 * 4. 一次性读取完整温湿度数据；
 * 5. 释放传感器模块相关资源。
 *
 ******************************************************************************/

 #ifndef __RS485_SHT30_H__
 #define __RS485_SHT30_H__
 
 #include <stdint.h>
 
 /* RS485 SHT30 默认 Modbus 从机地址 */
 #define RS485_SHT30_SLAVE_ADDR          0x01
 
 /* 输入寄存器地址 */
 #define RS485_SHT30_TEMP_REG_ADDR       0x0000
 #define RS485_SHT30_HUMI_REG_ADDR       0x0001
 
 /* 连续读取的寄存器数量 */
 #define RS485_SHT30_REG_COUNT           2
 
 /* 寄存器数据缩放系数，固定保留一位小数 */
 #define RS485_SHT30_DATA_SCALE          0.1f
 
 /**
  * @brief RS485 SHT30 温湿度数据结构体
  */
 typedef struct {
     float temperature;     /* 温度，单位：℃ */
     float humidity;        /* 相对湿度，单位：%RH */
 } rs485_sht30_data_t;
 

 int rs485_sht30_init(int uart_fd);
 
 int rs485_sht30_read_raw(int uart_fd, uint16_t *temp_raw, uint16_t *humi_raw)
 
 int rs485_sht30_convert_temperature(uint16_t temp_raw, float *temperature);
 
 int rs485_sht30_convert_humidity(uint16_t humi_raw, float *humidity);
 
 int rs485_sht30_read_data(rs485_sht30_data_t *data);
 
 void rs485_sht30_deinit(void);
 
 #endif /* __RS485_SHT30_H__ */