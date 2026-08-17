/******************************************************************************
 * @file    sensor.h
 * @brief   传感器数据结构定义
 *
 * @author  李宇坤
 * @date    2026-07
 * @version V1.0
 *
 * @details
 * 本文件定义系统统一的传感器数据结构，包括：
 * 1. 各类环境传感器数据；
 * 2. 数据时间戳；
 * 3. 设备状态；
 * 4. 数据有效标志；
 * 5. 统一数据接口。
 *
 * 所有采集模块均按照统一数据结构进行数据交互，
 * 方便数据库存储、Qt界面显示、MQTT上传及后续扩展。
 ******************************************************************************/

#ifndef __SENSOR_H__
#define __SENSOR_H__

#include <stdint.h> 
#include <stdbool.h>

/* 传感器数据结构定义 */

typedef struct
{
    /* BME280：I2C */
    float bme_temperature; /* 空气温度，单位：°C */
    float bme_humidity;    /* 空气相对湿度，单位：%RH */
    float pressure;        /* 大气压力，单位：hPa */

    /* SHT30：RS485 / Modbus RTU */
    float temperature;     /* 空气温度，单位：°C */
    float humidity;        /* 空气相对湿度，单位：%RH */

    /* 土壤 */
    float soil_humidity;        // 土壤湿度(%)

    /* 雨量传感器 */
    bool rain_detected;    /* 当前是否检测到降雨 */
    float rainfall;        /* 降雨量，单位：mm */

    /* 数据状态 */
    bool valid;                 // 数据是否有效
    bool is_abnormal;           // 是否为异常数据   


    /* 时间戳 */
    uint64_t timestamp;         // 采集时间(s)

}sensor_data_t;

#endif /* __SENSOR_H__ */
