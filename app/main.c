/******************************************************************************
 * @file    main.c
 * @brief   RK3588 Linux智能环境综合监测系统主程序
 *
 * @author  李宇坤
 * @date    2026-07
 * @version V1.0
 *
 * @details
 * 系统入口，实现：
 * 1. 各传感器模块初始化；
 * 2. 周期性采集环境数据；
 * 3. 输出当前环境状态。
 *
 ******************************************************************************/
#include "common.h"
#include "config.h"

#include "sensor.h"
#include "sensor_manager.h"

#include <stdio.h>
#include <unistd.h>
#include <signal.h>

int main(int argc, char *argv[])
{
    int ret;
    sensor_data_t env_data;
    printf("RK3588 Linux智能环境综合监测系统\n"); 

    ret = sensor_manager_init();
    if (ret != 0) {
        printf("传感器初始化失败, ret=%d\n", ret);
        return -1;
    }
    while (1)
    {
        ret = sensor_manager_collect(&env_data);
        if(ret != 0) {
            printf("传感器数据采集失败, ret=%d\n", ret);
            continue;
        }

        sleep(1);
    }
    sensor_manager_deinit();
    return 0;
}
