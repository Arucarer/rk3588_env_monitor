/******************************************************************************
 * @file    sensor_manager.h
 * @brief   传感器管理模块接口定义
 *
 * @author  李宇坤
 * @date    2026-07
 * @version V1.0
 *
 * @details
 * 本文件负责统一管理系统中的各类环境传感器，
 * 为上层应用提供统一的数据采集接口。
 *
 * 主要功能：
 * 1. 初始化所有传感器；
 * 2. 周期性采集环境数据；
 * 3. 释放传感器资源。
 *
 ******************************************************************************/

 #ifndef __SENSOR_MANAGER_H__
 #define __SENSOR_MANAGER_H__
 
 
 #include "sensor.h"
 
 #include "bme280.h"
 #include "sht30_rs485.h"
 #include "soil.h"
 #include "rain.h"

 int sensor_manager_init(void);
 
 int sensor_manager_collect(sensor_data_t *data);
 
 void sensor_manager_deinit(void);
 
 
 #endif