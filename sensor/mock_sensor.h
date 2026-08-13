#ifndef MOCK_SENSOR_H
#define MOCK_SENSOR_H

#include "sensor.h"

/* 初始化虚拟传感器 */
int mock_sensor_init(void);

/* 生成一组虚拟环境数据 */
int mock_sensor_collect(sensor_data_t *data);

/* 释放虚拟传感器资源 */
void mock_sensor_deinit(void);

#endif /* MOCK_SENSOR_H */