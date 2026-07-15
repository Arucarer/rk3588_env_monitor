/******************************************************************************
 * @file    mqtt.h
 * @brief   MQTT 云端通信模块接口定义
 *
 * @author  李宇坤
 * @date    2026-07
 * @version V1.0
 *
 * @details
 * 本文件定义 RK3588 Linux 智能环境综合监测系统中的 MQTT 通信接口，
 * 负责建立与 MQTT Broker 的连接，并完成环境传感器数据的发布以及
 * 云端控制命令的订阅。
 *
 * 主要功能包括：
 * 1. 初始化 MQTT 客户端；
 * 2. 连接 MQTT Broker；
 * 3. 将传感器数据转换为 JSON 格式；
 * 4. 发布环境监测数据；
 * 5. 订阅云端控制命令；
 * 6. 处理 MQTT 网络消息；
 * 7. 检测连接状态并进行断线重连；
 * 8. 释放 MQTT 客户端相关资源。
 *
 ******************************************************************************/
#ifndef __MQTT_CLIENT_H__
#define __MQTT_CLIENT_H__

#include <stdint.h>
#include "sensor_manager.h"
#include "sensor.h"

int mqtt_client_init(void);

int mqtt_client_publish_sensor_data(const sensor_data_t *data);//构建JSON数据

int mqtt_client_is_connected(void);//检测连接状态

void mqtt_client_deinit(void);//释放MQTT客户端资源

#endif
