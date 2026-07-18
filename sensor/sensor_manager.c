/******************************************************************************
 * @file    sensor_manager.c
 * @brief   传感器管理模块实现
 *
 * @author  李宇坤
 * @date    2026-07
 * @version V1.0
 *
 * @details
 * 本文件实现环境监测系统传感器统一管理功能。
 * 负责协调各传感器模块的初始化、数据采集以及资源释放。
 *
 * 主要功能包括：
 * 1. 初始化系统中所有环境传感器；
 * 2. 调用各传感器接口完成周期性数据采集；
 * 3. 将采集结果统一更新到环境数据结构中；
 * 4. 释放传感器相关硬件资源。
 *
 * 当前管理传感器包括：
 * 1. BME280 温湿度/气压传感器（I2C）；
 * 2. SHT30 RS485 温湿度传感器（Modbus RTU）；
 * 3. 电容式土壤湿度传感器（ADC）；
 * 4. 雨滴检测传感器（ADC/GPIO）。
 *
 ******************************************************************************/

#include "sensor_manager.h"

#include "bme280.h"
#include "sht30_rs485.h"
#include "soil.h"
#include "rain.h"

#include "config.h"
#include "uart.h"

#include <stdio.h>
#include <string.h>
#include <time.h>
int sensor_manager_init(void)
{
    int ret = 0;
    if(bme280_init() < 0)
    {
        printf("BME280 init failed\n");
        ret = -1;
    }
    
    if(rs485_sht30_init(uart_open(UART_DEV_PATH)) < 0)
    {
        printf("SHT30 init failed\n");
        ret = -1;
    }
    if(soil_init() < 0)
    {
        printf("soil init failed\n");
        ret = -1;
    }
    
    if(rain_init() < 0)
    {
        printf("rain init failed\n");
        ret = -1;
    }

    if(ret < 0)
    {
        printf("sensor init failed\n");
        return -1;
    }    
    return ret;
}
 
int sensor_manager_collect(sensor_data_t *data)
{
    int ret = 0;
    uint16_t rain_adc;
    uint16_t soil_adc;
    bme280_data_t bme_data;
    rs485_sht30_data_t sht30_data;
    if(data == NULL)
    {
        return -1;
    }
    memset(data,0,sizeof(sensor_data_t)); // 清空数据结构
    data->valid=0;

    if(bme280_read_data(&bme_data) < 0){
        ret = -1;
    } else {
        data->bme_temperature = bme_data.temperature;
        data->pressure = bme_data.pressure;
        data->bme_humidity = bme_data.humidity;
    }
    if(rs485_sht30_read_data(&sht30_data) < 0){
        ret = -1;
    } else {
        data->temperature = sht30_data.temperature;
        data->humidity = sht30_data.humidity;
    }
    /* 土壤 */
    if(soil_read_adc(&soil_adc) < 0 ||
       soil_get_moisture(soil_adc, &data->soil_humidity) < 0){
        ret = -1;
    }
    /* 雨滴 */
    if(rain_read_adc(&rain_adc) < 0){
        ret = -1;
    }
    if(rain_get_moisture(rain_adc, &data->rainfall) < 0){
        ret = -1;
    }
    data->valid = (ret == 0);
    data->timestamp = time(NULL);
    if (rain_detect(&data->rain_detected) < 0) {
        ret = -1;
        data->valid = false;
    }
    return ret;
}

void sensor_manager_deinit(void)
{
    bme280_deinit();
    rs485_sht30_deinit();
    soil_deinit();
    rain_deinit();
}
