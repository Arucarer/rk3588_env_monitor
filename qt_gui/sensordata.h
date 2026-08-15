#ifndef SENSORDATA_H
#define SENSORDATA_H

#include <QDateTime>
#include <QMetaType>

struct SensorData
{
    double temperature = 0.0;
    double humidity = 0.0;
    double pressure = 0.0;

    double rs485Temperature = 0.0;
    double rs485Humidity = 0.0;

    double soilHumidity = 0.0;
    double rainfall = 1.0;

    bool bme280Online = false;
    bool rs485Online = false;
    bool soilSensorOnline = false;
    bool rainSensorOnline = false;

    QDateTime timestamp;
};

Q_DECLARE_METATYPE(SensorData)

#endif // SENSORDATA_H