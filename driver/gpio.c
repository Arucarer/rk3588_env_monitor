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


 int gpio_export(int gpio)//导出gpio
 {
    int fd;
    int ret;
    int len;
    char path[64];
    char buf[16];
    sprintf(path, "/sys/class/gpio/export");
    fd = open(path, O_WRONLY);
    if (fd < 0)
        return -1;
    len = sprintf(buf, "%d", gpio);//sprintf()函数将格式化的数据写入字符串buf中
    ret = write(fd, buf, len);//向 GPIO 的 export 文件写入 1 个字节
    close(fd);
    if (ret != len)
        return -1;
    return 0;
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
    if (direction == GPIO_IN)
        ret = write(fd, "in", 2);
    else
        ret = write(fd, "out", 3);
    close(fd);
    if (ret < 0)
    {
        return -1;
    }
    return 0;
 }
 int gpio_read_value(int gpio, int *value)//读取gpio电平
 {
    int ret;
    int fd;
    char ch;
    char path[64];

    if (value == NULL)
        return -1;
    sprintf(path, "/sys/class/gpio/gpio%d/value", gpio);

    fd = open(path, O_RDONLY);
    if (fd < 0)
        return -1;

    ret = read(fd, &ch, 1);
    close(fd);

    if (ret != 1)
        return -1;

    *value = ch - '0';

    return 0;
 }
 int gpio_write_value(int gpio, int value)
 {
    int ret;
    int fd;
    char ch;
    char path[64];

    sprintf(path, "/sys/class/gpio/gpio%d/value", gpio);

    fd = open(path, O_WRONLY);
    if (fd < 0)
        return -1;
    ch = value ? '1' : '0';//是判断写入高电平还是低电平
    ret = write(fd, &ch, 1);//向 GPIO 的 value 文件写入 1 个字节
    close(fd);
    if (ret != 1)
        return -1;

    return 0;
 }
 int gpio_unexport(int gpio)
 {
     int fd;
     int ret;
     int len;
     char path[64];
     char buf[16];
     sprintf(path, "/sys/class/gpio/unexport");//创建 unexport 路径
     fd = open(path, O_WRONLY);
     if (fd < 0)
         return -1;
     len = sprintf(buf, "%d", gpio);//sprintf()函数将格式化的数据写入字符串buf中
     ret = write(fd, buf, len);//向 GPIO 的 unexport 文件写入 1 个字节
     close(fd);
     if (ret != len)
         return -1;
 
     return 0;
 }