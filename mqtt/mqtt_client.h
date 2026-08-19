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
#include "sensor.h"
#include "alarm_record_t.h"
#include "device_config_t.h"


/******************** 初始化与连接 ********************/

/* MQTT 客户端初始化 */
int mqtt_client_init(void);

/* 连接 MQTT Broker */
int mqtt_client_connect(void);


/******************** 订阅与消息接收 ********************/

/* 订阅云端控制主题 */
int mqtt_client_subscribe(void);


/******************** 数据发布 ********************/

/* 发布传感器数据 */
int mqtt_client_publish_sensor_data(const sensor_data_t *data);

/* 发布设备在线状态 */
int mqtt_client_publish_status(int online);

/* 发布告警消息 */
int mqtt_client_publish_alarm(const alarm_record_t *alarm);


/******************** 连接状态维护 ********************/

/* 检测 MQTT 当前连接状态 */
int mqtt_client_is_connected(void);

/* MQTT 断线重连 */
int mqtt_client_reconnect(void);

/* MQTT 网络循环 / 状态维护 */
int mqtt_client_loop(void);


/******************** 资源释放 ********************/

/* 释放 MQTT 客户端资源 */
void mqtt_client_deinit(void);

#endif
