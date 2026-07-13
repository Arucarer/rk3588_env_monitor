/******************************************************************************
 * @file    rain.c
 * @brief   雨滴传感器应用层实现
 *
 * @author  李宇坤
 * @date    2026-07
 * @version V1.0
 *
 * @details
 * 本文件实现雨滴传感器的数据采集功能：
 * 1. 通过 ADC 读取雨滴检测板的模拟输出；
 * 2. 通过 GPIO 读取雨滴检测板的数字输出；
 * 3. 将 ADC 原始值转换为湿润程度百分比。
 *
 ******************************************************************************/
#include "rain.h"
#include "gpio.h"
#include "adc.h"
#include "config.h"

#include <stdio.h>
#include <stddef.h>
#include <unistd.h>

static int rain_adc_fd = -1;   /* 雨滴传感器 ADC 文件描述符 */

int rain_init(void)
{
    int ret;
    /*1. 初始化GPIO*/
    ret = gpio_export(GPIO_RAIN_DO_PIN);
    if (ret < 0) {
        perror("gpio_export");
        return ret;
    }
    ret = gpio_set_direction(GPIO_RAIN_DO_PIN, GPIO_INPUT);
    if (ret < 0) {
        perror("gpio_set_direction");
        gpio_unexport(GPIO_RAIN_DO_PIN);
        return ret;
    }
    /*2. 打开AO\VO对应的ADC引脚*/
    rain_adc_fd = adc_open(RAIN_ADC_AO_PIN);
    if (rain_adc_fd < 0) {
        perror("adc_open");
        gpio_unexport(GPIO_RAIN_DO_PIN);
        return rain_adc_fd;
    }
    return 0;
}
// 1. 通过 ADC 读取雨滴检测板的模拟输出；
int rain_read_adc(uint16_t *adc_value)
{
    int ret;
    if (adc_value == NULL) {
        return -1;
    }

    if (rain_adc_fd < 0) {
        return -1;
    }
    ret = adc_read_raw(rain_adc_fd, adc_value);
    if (ret < 0) {
        perror("adc_read_raw");
        return ret;
    }
    return 0;
}
// 3. 将 ADC 原始值转换为湿润程度百分比。
int rain_get_moisture(uint16_t adc_value, float *moisture)
{
    float percent;
    if (moisture == NULL) {
        return -1;
    }
    percent = adc_raw_to_percent(adc_value,
                                 RAIN_ADC_DRY_VALUE,
                                 RAIN_ADC_WET_VALUE);

    if (percent < 0.0f) {
        return -1;
    }
    *moisture = percent;
    return 0;
}
//通过 GPIO 读取数字输出
int rain_detect(bool *detected)
{
    int ret;
    int gpio_value;
    if(detected == NULL)
    {
        return -1;
    }
    //1. 读取GPIO,然后赋值给gpio_value
    ret = gpio_read_value(GPIO_RAIN_DO_PIN, &gpio_value);
    if (ret < 0) {
        perror("gpio_read_value");
        return ret;
    }
    //如果检测到雨滴，则返回true
    if (gpio_value == 0) {
        *detected = true;
    } else {
        *detected = false;
    }
    return 0;
}
void rain_deinit(void)
{
    if (rain_adc_fd >= 0) {
        close(rain_adc_fd);
    }
    gpio_unexport(GPIO_RAIN_DO_PIN);
}