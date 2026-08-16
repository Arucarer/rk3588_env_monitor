/******************************************************************************
 * @file    databaseworker.h
 * @brief   SQLite 数据库工作类声明
 *
 * @author  李宇坤
 * @date    2026-08
 * @version V1.0
 *
 * @details
 * 本文件定义数据库工作类 DatabaseWorker，主要负责：
 * 1. 在独立数据库线程中执行 SQLite 操作；
 * 2. 接收采集线程发送的 SensorData；
 * 3. 调用现有 SQLite 管理模块完成数据写入；
 * 4. 提供历史数据查询接口；
 * 5. 后续支持告警记录、系统日志和 CSV 导出等功能。
 *
 * DatabaseWorker 作为 Qt 与底层 SQLite 模块之间的线程包装层，
 * 不直接操作界面控件。
 ******************************************************************************/

#ifndef DATABASEWORKER_H
#define DATABASEWORKER_H

#include <QObject>

#include "sensordata.h"

class DatabaseWorker : public QObject
{
    Q_OBJECT

public:
    explicit DatabaseWorker(QObject *parent = nullptr);

public slots:
    void start();
    void stop();

    void saveSensorData(const SensorData &data);
};

#endif // DATABASEWORKER_H
