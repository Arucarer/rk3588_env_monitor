#ifndef __ALARM_H__
#define __ALARM_H__

#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    int id;                          // 数据库记录ID

    uint64_t timestamp;              // 告警发生时间

    char sensor_name[32];            // 传感器名称
    char parameter[32];              // 告警参数
    char alarm_level[16];            // 告警级别

    float current_value;             // 当前值
    float threshold_value;           // 告警阈值

    char message[128];               // 告警描述

    bool handled;                    // 是否已经处理
    uint64_t handled_time;           // 处理时间，未处理时为0

} alarm_record_t;

#endif /* __ALARM_H__ */