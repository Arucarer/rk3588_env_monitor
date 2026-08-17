#ifndef __SYSTEM_LOG_H__
#define __SYSTEM_LOG_H__

#include <stdint.h>

typedef struct
{
    int id;                  // 数据库记录ID
    uint64_t timestamp;      // 日志时间

    char log_level[16];      // INFO / WARNING / ERROR
    char module[32];         // SENSOR / SQLITE / MQTT / SYSTEM
    char message[256];       // 日志内容

} system_log_t;

#endif /* __SYSTEM_LOG_H__ */