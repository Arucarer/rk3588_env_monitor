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
#include "mock_sensor.h"

static sensor_mode_t current_mode = SENSOR_MODE_REAL;

int sensor_manager_init(sensor_mode_t mode)
{
    int ret = 0;

    current_mode = mode;

    /* 虚拟传感器模式：不访问任何硬件 */
    if (current_mode == SENSOR_MODE_MOCK) {
        return mock_sensor_init();
    }

    /* 真实传感器模式：只初始化配置中启用的传感器 */
    #if SENSOR_ENABLE_BME280
        if (bme280_init() < 0) {
            printf("BME280 init failed\n");
            ret = -1;
        }
    #endif
    /* 条件编译，只有不为0的时候，这个才会被编译 */
    #if SENSOR_ENABLE_SHT30
        if (rs485_sht30_init(uart_open(UART_DEV_PATH)) < 0) {
            printf("SHT30 init failed\n");
            ret = -1;
        }
    #endif

    #if SENSOR_ENABLE_SOIL
        if (soil_init() < 0) {
            printf("soil init failed\n");
            ret = -1;
        }
    #endif

    #if SENSOR_ENABLE_RAIN
        if (rain_init() < 0) {
            printf("rain init failed\n");
            ret = -1;
        }
    #endif

    
    if (ret < 0) {
        printf("sensor init failed\n");
        return -1;
    }

    return 0;
}
int sensor_manager_collect(sensor_data_t *data)
{
    int ret = 0;
#if SENSOR_ENABLE_BME280
    bme280_data_t bme_data;
#endif

#if SENSOR_ENABLE_SHT30
    rs485_sht30_data_t sht30_data;
#endif

#if SENSOR_ENABLE_SOIL
    uint16_t soil_adc;
#endif

#if SENSOR_ENABLE_RAIN
    uint16_t rain_adc;
#endif

    if (data == NULL) {
        return -1;
    }

    /* 虚拟模式保持原来的采集逻辑 */
    if (current_mode == SENSOR_MODE_MOCK) {
        return mock_sensor_collect(data);
    }

    /*
     * 真实模式先将全部字段清零。
     * 未启用传感器对应的字段将保持为 0。
     */
    memset(data, 0, sizeof(sensor_data_t));
    data->timestamp = time(NULL);

#if SENSOR_ENABLE_BME280
    if (bme280_read_data(&bme_data) < 0) {
        printf("BME280 read failed\n");
        ret = -1;
    } else {
        data->bme_temperature = bme_data.temperature;
        data->bme_humidity = bme_data.humidity;
        data->pressure = bme_data.pressure;
    }
#endif

#if SENSOR_ENABLE_SHT30
    if (rs485_sht30_read_data(&sht30_data) < 0) {
        printf("SHT30 read failed\n");
        ret = -1;
    } else {
        data->temperature = sht30_data.temperature;
        data->humidity = sht30_data.humidity;
    }
#endif

#if SENSOR_ENABLE_SOIL
    if (soil_read_adc(&soil_adc) < 0 ||
        soil_get_moisture(soil_adc, &data->soil_humidity) < 0) {
        printf("soil read failed\n");
        ret = -1;
    }
#endif

#if SENSOR_ENABLE_RAIN
    if (rain_read_adc(&rain_adc) < 0 ||
        rain_get_moisture(rain_adc, &data->rainfall) < 0) {
        printf("rain ADC read failed\n");
        ret = -1;
    }

    if (rain_detect(&data->rain_detected) < 0) {
        printf("rain detection failed\n");
        ret = -1;
    }
#endif
    data->valid = (ret == 0);

    return ret;
}

void sensor_manager_deinit(void)
{
    if (current_mode == SENSOR_MODE_MOCK) {
        mock_sensor_deinit();
        return;
    }

#if SENSOR_ENABLE_BME280
    bme280_deinit();
#endif

#if SENSOR_ENABLE_SHT30
    rs485_sht30_deinit();
#endif

#if SENSOR_ENABLE_SOIL
    soil_deinit();
#endif

#if SENSOR_ENABLE_RAIN
    rain_deinit();
#endif
}