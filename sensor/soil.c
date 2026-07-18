/******************************************************************************
 * @file    soil.c
 * @brief   土壤湿度传感器应用层实现
 *
 * @author  李宇坤
 * @date    2026-07
 * @version V1.0
 *
 * @details
 * 本文件实现电容式土壤湿度传感器的数据采集功能：
 * 1. 通过 ADC 读取土壤湿度传感器的模拟输出；
 * 2. 将 ADC 原始值转换为土壤相对湿度百分比。
 *
 ******************************************************************************/
#include "soil.h"
#include "config.h"
#include "adc.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>//errno
#include <stddef.h>// NULL
#include <fcntl.h>//open
#include <unistd.h>//close

static int soil_adc_fd = -1;   /* 土壤传感器 ADC 文件描述符 */
int soil_init(void)
{
    soil_adc_fd = adc_open(SOIL_ADC_AO_PATH);
    if (soil_adc_fd < 0) {
        perror("adc_open");
        return soil_adc_fd;
    }
    return 0;
}
 
int soil_read_adc(uint16_t *adc_value)
{
    int raw;
    int ret;
    if(adc_value == NULL)
    {
        return -1;
    }
    if (soil_adc_fd < 0) {
        return -1;
    }
    ret = adc_read_raw(SOIL_ADC_AO_PATH, &raw);
    if (ret < 0) {
        perror("adc_read_raw");
        return ret;
    }
    *adc_value = (uint16_t)raw;
    return 0;
}

int soil_get_moisture(uint16_t adc_value, float *moisture)
{
    float percent;
    percent = adc_raw_to_percent(adc_value, SOIL_ADC_DRY_VALUE, SOIL_ADC_WET_VALUE);
    if(percent < 0.0f)
    {
        return -1;
    }
    *moisture = percent;
    return 0;
}

void soil_deinit(void)
{
    if(soil_adc_fd >= 0) {
        close(soil_adc_fd);
        soil_adc_fd = -1;
    }
}


