/******************************************************************************
 * @file    acquisitionworker.cpp
 * @brief   传感器采集工作类实现
 *
 * @author  李宇坤
 * @date    2026-08
 * @version V1.0
 *
 * @details
 * 本文件实现 AcquisitionWorker 的传感器数据采集逻辑。
 * 支持 Mock 和 Real 两种采集模式。
 *
 * Windows 开发环境当前使用 Mock 模式验证 Qt 多线程及数据显示。
 * RK3588 Linux 环境后续使用 Real 模式，通过 Sensor Manager
 * 获取真实 sensor_data_t，并转换为 Qt SensorData。
 ******************************************************************************/

#include "acquisitionworker.h"

#include <QRandomGenerator>
#include <QtMath>
#include <QDateTime>
#include <QDebug>

#if defined(Q_OS_LINUX) && defined(ENABLE_REAL_SENSOR)
extern "C" {
#include "sensor_manager.h"
}
#endif


AcquisitionWorker::AcquisitionWorker(
    AcquisitionMode mode,
    QObject *parent)
    : QObject(parent)
    , dataTimer(nullptr)
    , mode(mode)
    , phase(0.0)
{
}


void AcquisitionWorker::start()
{
#if defined(Q_OS_LINUX) && defined(ENABLE_REAL_SENSOR)
    if (mode == AcquisitionMode::Real) {

        if (sensor_manager_init(SENSOR_MODE_REAL) != 0) {
            qWarning() << "[AcquisitionWorker]"
                       << "Sensor Manager init failed";

            return;
        }

        qDebug() << "[AcquisitionWorker]"
                 << "Real sensor mode started";
    }
#endif

    if (dataTimer == nullptr) {

        dataTimer = new QTimer(this);

        connect(dataTimer,
                &QTimer::timeout,
                this,
                &AcquisitionWorker::collectData);
    }

    dataTimer->start(1000);
}


void AcquisitionWorker::stop()
{
    if (dataTimer != nullptr) {
        dataTimer->stop();
    }

#if defined(Q_OS_LINUX) && defined(ENABLE_REAL_SENSOR)
    if (mode == AcquisitionMode::Real) {
        sensor_manager_deinit();
    }
#endif
}


void AcquisitionWorker::collectData()
{
    SensorData data;

    if (mode == AcquisitionMode::Mock) {

        data = generateMockData();

    } else {

        data = collectRealData();
    }

    emit sensorDataReady(data);
}


SensorData AcquisitionWorker::generateMockData()
{
    phase += 0.15;

    auto noise = []() {
        return
            (QRandomGenerator::global()->generateDouble() - 0.5)
            * 0.6;
    };

    SensorData data;

    data.temperature =
        25.0 + 2.0 * qSin(phase) + noise();

    data.humidity =
        55.0 + 5.0 * qSin(phase * 0.7) + noise();

    data.pressure =
        1013.0 + 4.0 * qSin(phase * 0.3) + noise();

    data.rs485Temperature =
        24.5 + 1.8 * qSin(phase * 0.9) + noise();

    data.rs485Humidity =
        53.0 + 4.0 * qSin(phase * 0.6) + noise();

    data.soilHumidity =
        42.0 + 3.0 * qSin(phase * 0.2) + noise();

    data.rainfall =
        qSin(phase * 0.4) > 0.75 ? 2.4 : 0.0;

    data.bme280Online = true;
    data.rs485Online = true;
    data.soilSensorOnline = true;
    data.rainSensorOnline = true;

    data.timestamp = QDateTime::currentDateTime();

    return data;
}


SensorData AcquisitionWorker::collectRealData()
{
#if defined(Q_OS_LINUX) && defined(ENABLE_REAL_SENSOR)

    sensor_data_t rawData {};

    if (sensor_manager_collect(&rawData) != 0) {

        qWarning() << "[AcquisitionWorker]"
                   << "Sensor collect failed";

        SensorData data;

        data.timestamp = QDateTime::currentDateTime();

        return data;
    }

    return convertToSensorData(rawData);

#else

    /*
     * Windows 当前不访问真实 RK3588 传感器。
     * Real 模式仅在 RK3588 Linux 环境启用。
     */
    qWarning() << "[AcquisitionWorker]"
               << "Real sensor mode is unavailable on Windows";

    SensorData data;

    data.timestamp = QDateTime::currentDateTime();

    return data;

#endif
}
