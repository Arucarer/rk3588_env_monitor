/******************************************************************************
 * @file    rain.h
 * @brief   雨滴传感器模块接口定义
 *
 * @author  李宇坤
 * @date    2026-07
 * @version V1.0
 *
 * @details
 * 本文件定义雨滴传感器模块的应用层接口，用于读取雨滴模块的
 * 模拟量输出和数字量输出。
 *
 * 主要功能包括：
 * 1. 初始化雨滴传感器；
 * 2. 读取 ADC 原始数据；
 * 3. 获取雨滴湿润程度；
 * 4. 判断是否检测到雨滴；
 * 5. 释放雨滴传感器相关资源。
 *
 ******************************************************************************/

 #ifndef __RAIN_H__
 #define __RAIN_H__
 

 #include <stdbool.h>
 #include <stdint.h>

 int rain_init(void);

 int rain_read_adc(uint16_t *adc_value);//读取 ADC 原始数据
 int rain_get_moisture(uint16_t adc_value, float *moisture);//获取雨滴湿润程度
 int rain_detect(bool *detected);
 void rain_deinit(void);


 #endif