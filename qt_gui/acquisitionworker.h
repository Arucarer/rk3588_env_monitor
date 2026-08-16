/******************************************************************************
 * @file    acquisitionworker.h
 * @brief   传感器采集工作类声明
 *
 * @author  李宇坤
 * @date    2026-08
 * @version V1.0
 *
 * @details
 * 本文件定义传感器采集工作类 AcquisitionWorker，主要负责：
 * 1. 在独立采集线程中执行数据采集；
 * 2. 支持 Mock 模拟数据和 Real 真实数据两种模式；
 * 3. Mock 模式用于 Windows 界面及线程测试；
 * 4. Real 模式用于 RK3588 Linux 真实传感器采集；
 * 5. 统一输出 Qt SensorData。
 ******************************************************************************/

#ifndef ACQUISITIONWORKER_H
#define ACQUISITIONWORKER_H

#include <QObject>
#include <QTimer>

#include "sensordata.h"


enum class AcquisitionMode
{
    Mock,
    Real
};


class AcquisitionWorker : public QObject
{
    Q_OBJECT

public:
    explicit AcquisitionWorker(
        AcquisitionMode mode = AcquisitionMode::Mock,
        QObject *parent = nullptr);

public slots:
    void start();
    void stop();

private slots:
    void collectData();

signals:
    void sensorDataReady(const SensorData &data);

private:
    SensorData generateMockData();
    SensorData collectRealData();

private:
    QTimer *dataTimer;
    AcquisitionMode mode;
    double phase;
};

#endif // ACQUISITIONWORKER_H