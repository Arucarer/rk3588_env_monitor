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
    float bme_temperature;
    float temperature;          // 温度(℃)
    float bme_humidity;
    float humidity;             // 湿度(%RH)
    /* 气压 */
    float pressure;             // 气压(hPa)
    /* 土壤 */
    float soil_humidity;        // 土壤湿度(%)
    /* 降雨 */
    float rainfall;             // 降雨量(mm)
    bool rain_detected
    /* PM */
    float pm25;                 // PM2.5(ug/m³)
    float pm10;                 // PM10(ug/m³)
    /* 臭氧 */
    float ozone;                // 臭氧(ppm)
    /* 数据状态 */
    bool valid;                 // 数据是否有效
    /* 时间戳 */
    uint64_t timestamp;         // 采集时间(s)
}sensor_data_t;

#endif /* __SENSOR_H__ */