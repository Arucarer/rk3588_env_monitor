/******************************************************************************
 * @file    gpio.h
 * @brief   GPIO设备访问接口
 *
 * @author  李宇坤
 * @date    2026-07
 * @version V1.0
 *
 * @details
 * 本文件提供 Linux GPIO 应用层访问接口，
 * 基于 Linux GPIO Sysfs 接口实现 GPIO 的导出、方向配置、
 * 电平读取、电平输出等功能，为雨滴传感器、按键、LED
 * 等 GPIO 外设提供统一访问接口。
 *
 * 主要功能包括：
 * 1. 导出 GPIO；
 * 2. 取消导出 GPIO；
 * 3. 配置 GPIO 输入/输出方向；
 * 4. 读取 GPIO 电平状态；
 * 5. 设置 GPIO 输出电平。
 *
 ******************************************************************************/

 #ifndef __GPIO_H__
 #define __GPIO_H__
 
 #include <stdint.h>
 
 #define GPIO_INPUT   "in"
 #define GPIO_OUTPUT  "out"

 int gpio_export(int gpio);
 int gpio_set_direction(int gpio, int direction);
 int gpio_read_value(int gpio, int *value);
 int gpio_write_value(int gpio, int value);
 int gpio_unexport(int gpio);


 #include <stdint.h>