#ifndef __DEVICE_CONFIG_H__
#define __DEVICE_CONFIG_H__

#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    char device_id[64];              // 设备唯一ID
    char device_name[64];            // 设备名称

    int sample_period_ms;            // 传感器采集周期
    int sqlite_save_period_ms;       // SQLite保存周期
    int mqtt_upload_period_ms;       // MQTT上传周期

    float temperature_high;          // 温度上限
    float temperature_low;           // 温度下限

    float humidity_high;             // 湿度上限
    float humidity_low;              // 湿度下限

    float pressure_high;             // 气压上限
    float pressure_low;              // 气压下限

    float rs485_temperature_high;    // RS485温度上限
    float rs485_temperature_low;     // RS485温度下限

    float rs485_humidity_high;       // RS485湿度上限
    float rs485_humidity_low;        // RS485湿度下限

    float soil_humidity_low;         // 土壤湿度下限

    bool sqlite_enabled;             // 是否启用SQLite存储
    bool mqtt_enabled;               // 是否启用MQTT上传

    uint64_t updated_at;             // 参数最后更新时间

} device_config_t;

#endif /* __DEVICE_CONFIG_H__ */