/******************************************************************************
 * @file    gpio.c
 * @brief   GPIO 模块应用层实现
 *
 * @author  李宇坤
 * @date    2026-07
 * @version V1.0
 *
 * @details
 * 本文件实现 Linux GPIO 的应用层访问接口，负责 GPIO 的导出、
 * 方向配置、电平读取、电平输出等操作，为上层传感器模块提供
 * 统一的 GPIO 数据访问接口。
 *
 * 主要功能包括：
 * 1. 导出 GPIO；
 * 2. 取消导出 GPIO；
 * 3. 配置 GPIO 输入/输出方向；
 * 4. 读取 GPIO 电平状态；
 * 5. 设置 GPIO 输出电平。
 *
 ******************************************************************************/
#include "gpio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>


 int gpio_export(int gpio)
 {
    char path[64];
    int fd;
    fd = open(path, O_WRONLY);//O_WRONLY是只写
    sprintf(path, "/sys/class/gpio/export");
    ret = write(fd, &gpio, sizeof(gpio));//写入gpio
    close(fd);
    return ret;
 }
 int gpio_set_direction(int gpio, int direction)
 {
    int ret;
    char path[64];
    int fd;
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/direction", gpio);//创建gpio路径
    fd = open(path, O_WRONLY);
    if(fd < 0)
    {
        return -1;
    }
    if (direction == GPIO_IN)//输入
    {
        ret = write(fd, "in", 2);//写入in
    }
    else
    {
        ret = write(fd, "out", 3);
    }
    if (ret < 0)
    {
        return -1;
    }
    close(fd);
    return ret;
 }
 int gpio_read_value(int gpio, int *value)
 {

 }
 int gpio_write_value(int gpio, int value)
 {

 }
 int gpio_unexport(int gpio)
 {

 }