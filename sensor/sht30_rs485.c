/******************************************************************************
 * @file    rs485_sht30.c
 * @brief   RS485 SHT30 温湿度传感器应用层实现
 *
 * @author  李宇坤
 * @date    2026-07
 * @version V1.0
 *
 * @details
 * 本文件实现 RS485 SHT30 温湿度传感器的数据采集与解析功能。
 *
 * 该传感器通过 RS485 接口与 RK3588B 开发板通信，并采用标准
 * Modbus RTU 协议完成温度和湿度数据读取。
 *
 * 主要功能包括：
 * 1. 初始化 RS485 SHT30 温湿度传感器；
 * 2. 通过 Modbus RTU 读取温湿度输入寄存器；
 * 3. 解析温度和湿度原始寄存器数据；
 * 4. 将原始数据转换为实际温度和相对湿度；
 * 5. 释放传感器模块相关资源。
 *
 * 传感器寄存器说明：
 * 1. 输入寄存器 0x0000：温度数据；
 * 2. 输入寄存器 0x0001：湿度数据；
 * 3. 温度和湿度均采用固定一位小数表示；
 * 4. 负温度采用大于等于 10000 的特殊编码方式。
 *
 ******************************************************************************/
#include "sht30_rs485.h"
#include "modbus_master.h"
#include "uart.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

static int sht30_uart_fd = -1;


int rs485_sht30_init(int uart_fd)
{
    uint16_t registers[RS485_SHT30_REG_COUNT];
    int ret;
    if (uart_fd < 0) {
        printf("UART open failed: %s\n", strerror(errno));
        return -1;
    }
    sht30_uart_fd = uart_fd;
    ret = modbus_master_read_input_registers(uart_fd,
                                              RS485_SHT30_SLAVE_ADDR,
                                              RS485_SHT30_TEMP_REG_ADDR,
                                              RS485_SHT30_REG_COUNT,
                                              registers);

    if (ret != MODBUS_OK) {
        fprintf(stderr,"sensor error,ret = %d\n", ret);
    }
    printf("RS485 SHT30 initialized successfully\n");
    return MODBUS_OK;
}
 

int rs485_sht30_read_raw(int uart_fd, uint16_t *temp_raw, uint16_t *humi_raw)
{
    uint16_t registers[RS485_SHT30_REG_COUNT];
    int ret;
    if(uart_fd < 0 || (temp_raw == NULL)|| (humi_raw == NULL)){
        printf("UART open failed: %s\n", strerror(errno));
        return -1;
    }
    ret = modbus_master_read_input_registers(uart_fd,
                                              RS485_SHT30_SLAVE_ADDR,
                                              RS485_SHT30_TEMP_REG_ADDR,
                                              RS485_SHT30_REG_COUNT,
                                              registers);

    if (ret != MODBUS_OK) {
        fprintf(stderr,"sensor error,ret = %d\n", ret);
        return ret;
    }
    *temp_raw = registers[0];
    *humi_raw = registers[1];
    return MODBUS_OK;
}
int rs485_sht30_convert_temperature(uint16_t temp_raw, float *temperature)
{
    if (temperature == NULL) {
        return -1;
    }
    if (temp_raw < 10000) {
        *temperature = temp_raw * RS485_SHT30_DATA_SCALE;
    }
    else {
        *temperature = -(temp_raw - 10000) * RS485_SHT30_DATA_SCALE;
    }
    return MODBUS_OK;
}

int rs485_sht30_convert_humidity(uint16_t humi_raw, float *humidity)
{
    if (humidity == NULL) {
        return -1;
    }
    *humidity = humi_raw * RS485_SHT30_DATA_SCALE;
    return MODBUS_OK;
}


int rs485_sht30_read_data(rs485_sht30_data_t *data)
{
    uint16_t temp_raw, humi_raw;
    int ret;

    if (data == NULL) {
        return -1;
    }
    ret = rs485_sht30_read_raw(sht30_uart_fd, &temp_raw, &humi_raw);
    if (ret != MODBUS_OK)
    {
        return ret;
    }
    ret = rs485_sht30_convert_temperature(temp_raw, &data->temperature);
    if (ret != MODBUS_OK) {
        return ret;
    }
    ret = rs485_sht30_convert_humidity(humi_raw, &data->humidity);
    if (ret != MODBUS_OK) {
        return ret;
    }
    return MODBUS_OK;
}

void rs485_sht30_deinit(void)
{
    if (sht30_uart_fd >= 0) {
        uart_close(sht30_uart_fd);
        sht30_uart_fd = -1;
    }
}
