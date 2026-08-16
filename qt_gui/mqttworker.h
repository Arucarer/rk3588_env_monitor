/******************************************************************************
 * @file    mqttworker.h
 * @brief   MQTT 通信工作类声明
 *
 * @author  李宇坤
 * @date    2026-08
 * @version V1.0
 *
 * @details
 * 本文件定义 MQTT 通信工作类 MqttWorker，主要负责：
 * 1. 在独立 MQTT 线程中执行网络通信；
 * 2. 接收 SensorData 并上传环境监测数据；
 * 3. 调用现有 MQTT Client 模块连接 MQTT Broker；
 * 4. 后续实现断线重连、Topic 订阅和远程命令处理；
 * 5. 向 Qt 主线程反馈 MQTT 连接及通信状态。
 *
 * MqttWorker 作为 Qt 与底层 MQTT 模块之间的线程包装层，
 * 不直接操作界面控件。
 ******************************************************************************/

#ifndef MQTTWORKER_H
#define MQTTWORKER_H

#include <QObject>

#include "sensordata.h"

class MqttWorker : public QObject
{
    Q_OBJECT

public:
    explicit MqttWorker(QObject *parent = nullptr);

public slots:
    void start();
    void stop();

    void publishSensorData(const SensorData &data);
};

#endif // MQTTWORKER_H