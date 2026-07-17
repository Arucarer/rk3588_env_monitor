/******************************************************************************
 * @file    adc.h
 * @brief   ADC 设备访问接口
 *
 * @author  李宇坤
 * @date    2026-07
 * @version V1.0
 *
 * @details
 * 本文件封装 Linux IIO（Industrial I/O）ADC 设备的应用层访问接口，
 * 提供 ADC 原始值读取、电压值读取以及模拟量转换等功能。
 *
 * 主要功能包括：
 * 1. 打开 ADC 数据节点；
 * 2. 读取 ADC 原始采样值（raw）；
 * 3. 读取 ADC 电压值（mV）；
 * 4. 提供 ADC 数值转换接口；
 * 5. 为土壤湿度、雨滴等模拟量传感器提供统一访问接口。
 *
 ******************************************************************************/

 #ifndef __ADC_H__
 #define __ADC_H__
 
 #include <stdint.h>

 int adc_open(const char *dev_path);
 int adc_read_raw(const char *path, int *raw);                                          // 读取 ADC 原始采样值
 int adc_read_voltage(const char *raw_path, const char *scale_path, float *voltage);    // 读取 ADC 电压值
float adc_raw_to_percent(int raw, int dry_raw, int wet_raw);
 
 #endif