/******************************************************************************
 * @file    mqttworker.cpp
 * @brief   MQTT 通信工作类实现
 *
 * @author  李宇坤
 * @date    2026-08
 * @version V1.0
 *
 * @details
 * 本文件实现 MqttWorker 的 MQTT 通信逻辑。
 * 后续将封装现有 mqtt_client 模块，实现传感器数据发布、
 * Broker 状态管理、自动重连、消息订阅及远程命令处理等功能。
 ******************************************************************************/

#include "mqttworker.h"

MqttWorker::MqttWorker(QObject *parent)
    : QObject(parent)
{
}

void MqttWorker::start()
{
    qDebug() << "[MqttWorker] started";
}

void MqttWorker::stop()
{
    qDebug() << "[MqttWorker] stopped";
}

void MqttWorker::publishSensorData(const SensorData &data)
{
    qDebug() << "[MQTT]"
             << "temperature =" << data.temperature
             << "humidity =" << data.humidity;
}