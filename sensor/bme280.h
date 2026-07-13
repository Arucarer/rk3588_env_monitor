/******************************************************************************
 * @file    bme280.h
 * @brief   BME280 温湿度气压传感器应用层接口定义
 *
 * @author  李宇坤
 * @date    2026-07
 * @version V1.0
 *
 * @details
 * 本文件定义 Linux 平台下 BME280 传感器应用层访问接口，
 * 基于 I2C 通信方式实现对 BME280 温度、湿度以及大气压力数据的读取。
 *
 * BME280 通过 Linux I2C 设备节点进行访问，底层依赖 i2c.c 提供的
 * I2C 设备读写接口，实现传感器寄存器访问、数据校准以及环境参数转换。
 *
 * 主要功能包括：
 * 1. BME280 设备初始化；
 * 2. BME280 寄存器配置；
 * 3. 温度数据采集与转换；
 * 4. 湿度数据采集与转换；
 * 5. 气压数据采集与转换；
 * 6. 为环境监测系统提供标准化温湿度/气压数据接口。
 *
 * 通信接口：
 * I2C
 ******************************************************************************/
#ifndef __BME280_H__
#define __BME280_H__
#include <stdint.h>

typedef struct {
    float temperature;
    float humidity;
    float pressure;
} bme280_data_t;



int bme280_init(void);
int bme280_config(void);
int bme280_read_data(bme280_data_t *data);
int bme280_read_id(uint8_t *id);
void bme280_deinit(void);

#endif