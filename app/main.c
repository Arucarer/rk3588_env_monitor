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
#include <string.h>

#include "sqlite_manager.h"
#include "mqtt_client.h"




int main(int argc, char *argv[])
{
    int ret;
    sensor_mode_t mode;
    sensor_data_t env_data;
    sensor_data_t latest_data;
    printf("RK3588 Linux智能环境综合监测系统\n"); 

    /*
    * 命令格式：
    * ./env_monitor --mode mock
    * ./env_monitor --mode real
    */
    if (argc != 3 || strcmp(argv[1], "--mode") != 0) {
        printf("Usage: %s --mode <mock|real>\n", argv[0]);
        return -1;
    }
    /*参数数量不是3个或者 第二个参数不是 --mode ，就是错误*/

    if (strcmp(argv[2], "mock") == 0) {
        mode = SENSOR_MODE_MOCK;
        printf("Sensor mode: mock\n");
    } else if (strcmp(argv[2], "real") == 0) {
        mode = SENSOR_MODE_REAL;
        printf("Sensor mode: real\n");
    } else {
        printf("Invalid sensor mode: %s\n", argv[2]);
        printf("Usage: %s --mode <mock|real>\n", argv[0]);
        return -1;
    }/*既不是 mock 也不是 real，程序会输出错误并退出*/


    /* 初始化虚拟传感器 */
    ret = sensor_manager_init(mode);
    if (ret != 0) {
        printf("传感器初始化失败, ret=%d\n", ret);
        return -1;
    }

    /* 初始化 SQLite */
    ret = sqlite_manager_init(SQLITE_DB_PATH);
    if (ret != 0) {
        printf("SQLite initialization failed, ret=%d\n", ret);
        sensor_manager_deinit();
        return -1;
    }
    printf("SQLite initialized: %s\n", SQLITE_DB_PATH);
    /* 初始化 MQTT */
    ret = mqtt_client_init();
    if (ret != 0) {
        printf("MQTT initialization failed, ret=%d\n", ret);
        sqlite_manager_deinit();
        sensor_manager_deinit();
        return -1;
    }

    /* 连接 MQTT Broker */
    ret = mqtt_client_connect();
    if (ret != 0) {
        printf("MQTT connection failed, ret=%d\n", ret);
        mqtt_client_deinit();
        sqlite_manager_deinit();
        sensor_manager_deinit();
        return -1;
    }

    /* 订阅云端控制主题 */
    ret = mqtt_client_subscribe();
    if (ret != 0) {
        printf("MQTT subscribe failed, ret=%d\n", ret);
        mqtt_client_deinit();
        sqlite_manager_deinit();
        sensor_manager_deinit();
        return -1;
    }

    printf("MQTT connected: %s\n", MQTT_BROKER_ADDRESS);


    while (1)
    {
        /* 1. 采集环境数据 */
        ret = sensor_manager_collect(&env_data);
        if (ret != 0) {
            printf("传感器数据采集失败, ret=%d\n", ret);
            sleep(1);
            continue;
        }

        /* 2. 输出本次数据 */
        printf(
            "time=%llu, "
            "BME[temp=%.2f C, humidity=%.2f %%RH, pressure=%.2f hPa], "
            "SHT30[temp=%.2f C, humidity=%.2f %%RH], "
            "soil=%.2f %%, rain=%d, rainfall=%.2f mm, valid=%d\n",
            (unsigned long long)env_data.timestamp,
            env_data.bme_temperature,
            env_data.bme_humidity,
            env_data.pressure,
            env_data.temperature,
            env_data.humidity,
            env_data.soil_humidity,
            env_data.rain_detected ? 1 : 0,
            env_data.rainfall,
            env_data.valid ? 1 : 0
        );

        /* 3. 写入 SQLite */
        ret = sqlite_manager_insert(&env_data);
        if (ret != 0) {
            printf("SQLite insert failed\n");
        } else {
            printf("SQLite insert success\n");
        }

        /* 4. 查询 SQLite 中最新一条记录 */
        ret = sqlite_manager_query_latest(&latest_data);
        if (ret != 0) {
            printf("SQLite query failed\n");
        } else {
            printf(
                "DB latest: time=%llu, BME temp=%.2f, "
                "SHT30 temp=%.2f, soil=%.2f, rainfall=%.2f\n",
                (unsigned long long)latest_data.timestamp,
                latest_data.bme_temperature,
                latest_data.temperature,
                latest_data.soil_humidity,
                latest_data.rainfall
            );
            ret = mqtt_client_publish_sensor_data(&env_data);
            if (ret != 0) {
                printf("MQTT publish failed, ret=%d\n", ret);
            } else {
                printf("MQTT publish success\n");
            }
        }
        sleep(1);
    }

    mqtt_client_deinit();
    sqlite_manager_deinit();
    sensor_manager_deinit();

    return 0;
}
