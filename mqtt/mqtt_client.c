/******************************************************************************
 * @file    mqtt.c
 * @brief   MQTT 云端通信模块实现
 *
 * @author  李宇坤
 * @date    2026-07
 * @version V1.0
 *
 * @details
 * 本文件实现 RK3588 Linux 智能环境综合监测系统中的 MQTT 通信功能，
 * 基于 MQTT 客户端库完成设备与 MQTT Broker 之间的数据交互。
 *
 * 主要功能包括：
 * 1. 创建并初始化 MQTT 客户端；
 * 2. 配置 MQTT Broker 地址、客户端 ID 及连接参数；
 * 3. 建立与 MQTT Broker 的网络连接；
 * 4. 将环境传感器数据转换为 JSON 格式；
 * 5. 发布环境监测数据；
 * 6. 订阅云端控制命令；
 * 7. 处理 MQTT 消息回调及连接状态；
 * 8. 实现断线检测与重新连接；
 * 9. 释放 MQTT 客户端相关资源。
 *
 ******************************************************************************/
#include "mqtt_client.h"
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <MQTTClient.h>

static MQTTClient mqtt_client = NULL;//MQTT客户端对象
static int mqtt_build_sensor_json(const sensor_data_t *data, char *json_buf, size_t buf_size);

int mqtt_client_init(void)
{
    int ret = 0;
    
    /* #1. 初始化 MQTT 连接参数结构体 */
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;

    /* #2. 创建 MQTT 客户端对象，设置 Broker 地址和客户端 ID */
    ret = MQTTClient_create(&mqtt_client, MQTT_BROKER_ADDRESS, MQTT_CLIENT_ID, MQTT_CLEAN_SESSION, NULL);
    if (ret != MQTTCLIENT_SUCCESS) {
        fprintf(stderr, "MQTTClient_create failed, ret=%d\n", ret);
        mqtt_client = NULL;
        return -1;
    }
    /* #3. 配置心跳、清洁会话等连接参数 */
    conn_opts.keepAliveInterval = MQTT_KEEP_ALIVE_INTERVAL;
    conn_opts.cleansession = MQTT_CLEAN_SESSION;
    // conn_opts.username = MQTT_USER;//用户名
    // conn_opts.password = MQTT_PASS;//登录密码
    conn_opts.connectTimeout = MQTT_TIMEOUT_MS;//最大等候时间
    
    /* #4. 建立与 MQTT Broker 的网络连接 */
    ret = MQTTClient_connect(mqtt_client, &conn_opts);
    if (ret != MQTTCLIENT_SUCCESS)
    {
        fprintf(stderr, "MQTTClient_connect failed, ret=%d\n", ret);
        MQTTClient_destroy(&mqtt_client);//释放MQTT客户端对象
        mqtt_client = NULL;    
        return -1;
    }
    /* #5. 返回初始化结果 */
    return ret;
}

int mqtt_client_publish_sensor_data(const sensor_data_t *data)//构建JSON数据
{
    int ret = 0;
    char json_buf[512];
    /* #1. 检查参数和 MQTT 连接状态 */
    if (!data || !mqtt_client) {
        fprintf(stderr, "Invalid parameter or MQTT client not initialized\n");
        return -1;
    }
    /* #2. 将环境传感器数据转换为 JSON 格式 */
    ret = mqtt_build_sensor_json(data, json_buf, sizeof(json_buf));
    if (ret != 0) {
        fprintf(stderr, "Failed to build MQTT JSON\n");
        return -1;
    }
    /* #3. 配置 MQTT 消息 */
    MQTTClient_deliveryToken token;//定义一个“消息发送令牌”变量，用来唯一标识这一次 MQTT 消息发布
    /* #4. 发布 MQTT 消息 */
    ret = MQTTClient_publish(mqtt_client,
                            MQTT_PUBLISH_TOPIC,//发布的 MQTT 主题
                            (int)strlen(json_buf),//消息长度
                            json_buf,//消息内容
                            MQTT_QOS,//QoS等级
                            MQTT_RETAIN,//保留消息
                            &token);//消息发送令牌
    if (ret != MQTTCLIENT_SUCCESS) {
        fprintf(stderr, "MQTTClient_publish failed, ret=%d\n", ret);
        return ret;
    }
    /* #5. 等待消息发送完成 */
    MQTTClient_waitForCompletion(mqtt_client, token, MQTT_TIMEOUT_MS);
    if (ret != MQTTCLIENT_SUCCESS) {
        fprintf(stderr, "等待MQTT消息发送完成失败，ret=%d\n", ret);
        return ret;
    }
    return MQTTCLIENT_SUCCESS;
}

int mqtt_client_is_connected(void)//检测连接状态
{
    if (mqtt_client == NULL)
    {
        return 0;
    }
    /* 调用Paho接口查询当前连接状态 */
    return MQTTClient_isConnected(mqtt_client);//返回当前连接状态
}

void mqtt_client_deinit(void)//释放MQTT客户端资源
{
    if (mqtt_client != NULL) {
        MQTTClient_disconnect(mqtt_client, 0);
        MQTTClient_destroy(&mqtt_client);
    }
}


static int mqtt_build_sensor_json(const sensor_data_t *data, char *json_buf, size_t buf_size)
{
    int len;
    if (data == NULL || json_buf == NULL || buf_size < 128)
    {
        return -1;
    }
    
    len = snprintf(
        json_buf,
        buf_size,
        "{"
        "\"timestamp\":%ld,"
        "\"bme_humidity\":%.2f,"
        "\"bme_temperature\":%.2f,"
        "\"humidity\":%.2f,"
        "\"temperature\":%.2f,"
        "\"pressure\":%.2f,"
        "\"rain_detected\":%d,"
        "\"rainfall\":%.2f,"
        "\"soil_humidity\":%.2f,"
        "\"valid\":%d"
        "}",
        (long)data->timestamp,
        data->bme_humidity,
        data->bme_temperature,
        data->humidity,
        data->temperature,
        data->pressure,
        data->rain_detected,
        data->rainfall,
        data->soil_humidity,
        data->valid
    );
    if (len < 0 || (size_t)len >= buf_size) {
        fprintf(stderr, "MQTT JSON构造失败或缓冲区不足\n");
        return -1;
    }
    return 0;
}
