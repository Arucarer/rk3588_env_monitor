/******************************************************************************
 * @file    soil.h
 * @brief   土壤湿度传感器接口定义
 *
 * @author  李宇坤
 * @date    2026-07
 * @version V1.0
 *
 * @details
 * 本文件定义电容式土壤湿度传感器的应用层接口，用于读取 ADC
 * 原始数据，并将其转换为土壤相对湿度百分比。
 *
 ******************************************************************************/

 #ifndef __SOIL_H__
 #define __SOIL_H__
 
 #include <stdint.h>
 
 int soil_init(void);
 
 int soil_read_adc(uint16_t *adc_value);
 
 int soil_get_moisture(uint16_t adc_value, float *moisture);
 
 void soil_deinit(void);
 
 #endif