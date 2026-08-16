/******************************************************************************
 * @file    databaseworker.cpp
 * @brief   SQLite 数据库工作类实现
 *
 * @author  李宇坤
 * @date    2026-08
 * @version V1.0
 *
 * @details
 * 本文件实现 DatabaseWorker 的数据库业务逻辑。
 * 后续将封装现有 sqlite_manager 模块，并通过 Qt 信号与槽完成
 * 传感器数据存储、历史数据查询以及数据库状态反馈。
 ******************************************************************************/

#include "databaseworker.h"

DatabaseWorker::DatabaseWorker(QObject *parent)
    : QObject(parent)
{
}

void DatabaseWorker::start()
{
    qDebug() << "[DatabaseWorker] started";
}

void DatabaseWorker::stop()
{
    qDebug() << "[DatabaseWorker] stopped";
}

void DatabaseWorker::saveSensorData(const SensorData &data)
{
    qDebug() << "[DB]"
             << "temperature =" << data.temperature
             << "humidity =" << data.humidity;
}