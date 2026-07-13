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
#include "driver/adc.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>//errno
#include <stddef.h>// NULL
#include <fcntl.h>//open




