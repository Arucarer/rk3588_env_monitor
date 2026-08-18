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
#include <unistd.h>

static MQTTClient mqtt_client = NULL;//MQTT客户端对象

static int mqtt_build_sensor_json(const sensor_data_t *data, char *json_buf, size_t buf_size);
static int mqtt_build_status_json(int online, char *json_buf, size_t buf_size);
static int mqtt_build_alarm_json(const alarm_record_t *alarm, char *json_buf, size_t buf_size);

static void mqtt_connection_lost(void *context, char *cause);
static int mqtt_message_arrived(void *context, char *topicName, int topicLen, MQTTClient_message *message);
static void mqtt_delivery_complete(void *context, MQTTClient_deliveryToken token);


/******************** 初始化与连接 ********************/
/* MQTT 客户端初始化 */
int mqtt_client_init(void)
{
    int ret = 0;
    
    /* #1. 检查 MQTT 客户端是否已经初始化 */
    if (mqtt_client != NULL)
    {
        return MQTTCLIENT_SUCCESS;
    }
    /* #2. 创建 MQTT 客户端对象，设置 Broker 地址和客户端 ID */
    ret = MQTTClient_create(&mqtt_client, MQTT_BROKER_ADDRESS, MQTT_CLIENT_ID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    if (ret != MQTTCLIENT_SUCCESS) {
        fprintf(stderr, "MQTTClient_create failed, ret=%d\n", ret);
        mqtt_client = NULL;
        return -1;
    }
    /* #3. 设置 MQTT 回调函数 */
    ret = MQTTClient_setCallbacks(mqtt_client, NULL, mqtt_connection_lost, mqtt_message_arrived, mqtt_delivery_complete);

    if (ret != MQTTCLIENT_SUCCESS) {
        fprintf(stderr, "MQTTClient_setCallbacks failed, ret=%d\n", ret);
        MQTTClient_destroy(&mqtt_client);
        return ret;
    }
    /* #4. MQTT 客户端初始化成功 */
    return MQTTCLIENT_SUCCESS;
}

int mqtt_client_connect(void)
{
    int ret;
    /* #1. 检查 MQTT 客户端是否已经初始化 */
    if (mqtt_client == NULL)
    {
        fprintf(stderr, "MQTT client not initialized\n");
        return -1;
    }
    /* #2. 如果已经连接，则直接返回成功 */
    if (MQTTClient_isConnected(mqtt_client))
    {
        return MQTTCLIENT_SUCCESS;
    }
    /* #3. 初始化 MQTT 连接参数 */
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;

     /* #4. 配置 MQTT 连接参数 */
    conn_opts.keepAliveInterval = MQTT_KEEP_ALIVE_INTERVAL;//心跳间隔
    conn_opts.cleansession = MQTT_CLEAN_SESSION;//是否清除会话
    // conn_opts.username = MQTT_USER;//用户名
    // conn_opts.password = MQTT_PASS;//登录密码
    conn_opts.connectTimeout = (int)(MQTT_TIMEOUT_MS / 1000L);//最大等候时间
    
     /* #5. 连接 MQTT Broker */
    ret = MQTTClient_connect(mqtt_client, &conn_opts);
    if (ret != MQTTCLIENT_SUCCESS)
    {
        fprintf(stderr, "MQTTClient_connect failed, ret=%d\n", ret);
        return ret;
    }
    /* #7. 连接成功 */
    return MQTTCLIENT_SUCCESS;
}




/******************** 订阅与消息接收 ********************/

/* 订阅云端控制主题 */
int mqtt_client_subscribe(void)
{
    int ret;

    /* #1. 检查 MQTT 客户端是否已经初始化 */
    if (mqtt_client == NULL) {
        fprintf(stderr, "MQTT client not initialized\n");
        return -1;
    }
    /* #2. 检查 MQTT 连接状态 */
    if (!mqtt_client_is_connected()) {
        fprintf(stderr, "MQTT client not connected\n");
        return -1;
    }
    /* #3. 订阅云端控制主题 */
    ret = MQTTClient_subscribe(mqtt_client, MQTT_SUBSCRIBE_TOPIC, MQTT_QOS);

    /* #4. 检查订阅结果 */
    if (ret != MQTTCLIENT_SUCCESS) {
        fprintf(stderr, "MQTTClient_subscribe failed, ret=%d\n", ret);
        return ret;
    }

    return MQTTCLIENT_SUCCESS;
}

/******************** 数据发布 ********************/

/* 发布传感器数据 */
int mqtt_client_publish_sensor_data(const sensor_data_t *data)//构建JSON数据
{
    int ret = 0;
    char json_buf[512];
    /* #1. 检查参数和 MQTT 连接状态 */
    if (data == NULL || mqtt_client == NULL) {
        fprintf(stderr, "Invalid parameter or MQTT client not initialized\n");
        return -1;
    }
    
    if (!mqtt_client_is_connected()) {
        fprintf(stderr, "MQTT client not connected\n");
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
    ret = MQTTClient_waitForCompletion(mqtt_client, token, MQTT_TIMEOUT_MS);
    if (ret != MQTTCLIENT_SUCCESS) {
        fprintf(stderr, "等待MQTT消息发送完成失败，ret=%d\n", ret);
        return ret;
    }
    return MQTTCLIENT_SUCCESS;
}


/* 发布设备在线状态 */
int mqtt_client_publish_status(int online)
{
    int ret;
    char json_buf[128];
    MQTTClient_deliveryToken token;
    /* #1. 检查 MQTT 客户端连接状态 */
    if (mqtt_client == NULL || !mqtt_client_is_connected()) {
        fprintf(stderr, "MQTT client not connected\n");
        return -1;
    }
    /* #2. 构建设备状态 JSON */
    ret = mqtt_build_status_json(online, json_buf, sizeof(json_buf));
    if (ret != 0){
        fprintf(stderr, "Failed to build MQTT status JSON\n");
        return -1;
    }
    
    /* #3. 发布设备状态消息 */
    ret = MQTTClient_publish(mqtt_client, MQTT_STATUS_TOPIC, (int)strlen(json_buf), json_buf, MQTT_QOS, MQTT_RETAIN, &token);
    if (ret != MQTTCLIENT_SUCCESS) {
        fprintf(stderr, "MQTT status publish failed, ret=%d\n", ret);
        return ret;
    }    
    /* #4. 等待消息发送完成 */
    ret = MQTTClient_waitForCompletion(mqtt_client, token, MQTT_TIMEOUT_MS);
    if (ret != MQTTCLIENT_SUCCESS) {
        fprintf(stderr, "MQTT status delivery failed, ret=%d\n", ret);
        return ret;
    }

    return MQTTCLIENT_SUCCESS;
}

/* 发布告警消息 */
int mqtt_client_publish_alarm(const char *alarm_type, const char *message)
{
    int ret;
    char json_buf[256];
    MQTTClient_deliveryToken token;

    /* #1. 检查参数和 MQTT 连接状态 */
    if (alarm_type == NULL || message == NULL || mqtt_client == NULL || !mqtt_client_is_connected()) {
        fprintf(stderr, "Invalid alarm parameter or MQTT client not connected\n");
        return -1;
    }

    /* #2. 构建告警 JSON */
    ret = mqtt_build_alarm_json(alarm_type, message, json_buf, sizeof(json_buf));
    if (ret != 0) {
        fprintf(stderr, "Failed to build MQTT alarm JSON\n");
        return -1;
    }

    /* #3. 发布告警消息 */
    ret = MQTTClient_publish(mqtt_client, MQTT_ALARM_TOPIC, (int)strlen(json_buf), json_buf, MQTT_QOS, MQTT_RETAIN, &token);
    if (ret != MQTTCLIENT_SUCCESS) {
        fprintf(stderr, "MQTT alarm publish failed, ret=%d\n", ret);
        return ret;
    }

    /* #4. 等待消息发送完成 */
    ret = MQTTClient_waitForCompletion(mqtt_client, token, MQTT_TIMEOUT_MS);
    if (ret != MQTTCLIENT_SUCCESS) {
        fprintf(stderr, "MQTT alarm delivery failed, ret=%d\n", ret);
        return ret;
    }

    return MQTTCLIENT_SUCCESS;
}

/******************** 连接状态维护 ********************/

/* 检测 MQTT 当前连接状态 */
int mqtt_client_is_connected(void)
{
    if (mqtt_client == NULL) return 0;
    return MQTTClient_isConnected(mqtt_client);
}

/* MQTT 断线重连 */
int mqtt_client_reconnect(void)
{
    int ret;
    if(mqtt_client == NULL){
        fprintf(stderr, "MQTT client not initialized\n");
        return -1;
    }
    /* #2. 如果已经连接，则无需重连 */
    if (mqtt_client_is_connected()) return MQTTCLIENT_SUCCESS;
    /* #3. 重新连接 MQTT Broker */
    ret = mqtt_client_connect();
    if (ret != MQTTCLIENT_SUCCESS) {
        fprintf(stderr, "MQTT reconnect failed, ret=%d\n", ret);
        return ret;
    }
    /* #4. 重连成功后重新订阅云端控制主题 */
    ret = mqtt_client_subscribe();
    if (ret != MQTTCLIENT_SUCCESS) {
        fprintf(stderr, "MQTT resubscribe failed, ret=%d\n", ret);
        return ret;
    }
    return MQTTCLIENT_SUCCESS;
}

/* MQTT 网络循环 / 状态维护 */
int mqtt_client_loop(void)
{
    int ret;
    if(mqtt_client == NULL){
        fprintf(stderr, "MQTT client not initialized\n");
        return -1;
    }
    if (mqtt_client_is_connected()) return MQTTCLIENT_SUCCESS;  
    /* #3. MQTT 已断线，等待后执行重新连接 */
    sleep(MQTT_RECONNECT_INTERVAL);     
    /* #4. MQTT 已断线，执行重新连接 */
    ret = mqtt_client_reconnect();
    if (ret != MQTTCLIENT_SUCCESS) {
        fprintf(stderr, "MQTT connection maintenance failed, ret=%d\n", ret);
        return ret;
    }
    return MQTTCLIENT_SUCCESS;    
}




/******************** 资源释放 ********************/

/* 释放 MQTT 客户端资源 */
void mqtt_client_deinit(void)//释放MQTT客户端资源
{
    if (mqtt_client != NULL) {
        MQTTClient_disconnect(mqtt_client, 0);
        MQTTClient_destroy(&mqtt_client);
    }
}




/******************** JSON构建 ********************/
/* 本文件函数构建 MQTT JSON 数据 */
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
        "\"device_id\":\"%s\","
        "\"timestamp\":%ld,"
        "\"bme_humidity\":%.2f,"
        "\"bme_temperature\":%.2f,"
        "\"humidity\":%.2f,"
        "\"temperature\":%.2f,"
        "\"pressure\":%.2f,"
        "\"rain_detected\":%d,"
        "\"rainfall\":%.2f,"
        "\"soil_humidity\":%.2f,"
        "\"valid\":%d,"
        "\"is_abnormal\":%d"
        "}",
        MQTT_CLIENT_ID,
        (long)data->timestamp,
        data->bme_humidity,
        data->bme_temperature,
        data->humidity,
        data->temperature,
        data->pressure,
        data->rain_detected,
        data->rainfall,
        data->soil_humidity,
        data->valid,
        data->is_abnormal
    );
    if (len < 0 || (size_t)len >= buf_size) {
        fprintf(stderr, "MQTT JSON构造失败或缓冲区不足\n");
        return -1;
    }
    return 0;
}

static int mqtt_build_status_json(int online, char *json_buf, size_t buf_size)
{
    int len;

    /* #1. 检查参数 */
    if (json_buf == NULL || buf_size < 64) return -1;

    /* #2. 构建设备状态 JSON */
    len = snprintf(
        json_buf,
        buf_size,
        "{"
        "\"device_id\":\"%s\","
        "\"online\":%d"
        "}",
        MQTT_CLIENT_ID,
        online
    );

    /* #3. 检查 JSON 构建结果 */
    if (len < 0 || (size_t)len >= buf_size) {
        fprintf(stderr, "MQTT status JSON构造失败或缓冲区不足\n");
        return -1;
    }

    return 0;
}

static int mqtt_build_alarm_json(const alarm_record_t *alarm, char *json_buf, size_t buf_size)
{
    int len;

    /* #1. 检查参数 */
    /* #1. 检查参数 */
    if (alarm == NULL || json_buf == NULL || buf_size < 256) {
        return -1;
    }

    /* #2. 构建告警 JSON */
    len = snprintf(
        json_buf,
        buf_size,
        "{"
        "\"device_id\":\"%s\","
        "\"timestamp\":%llu,"
        "\"sensor_name\":\"%s\","
        "\"parameter\":\"%s\","
        "\"alarm_level\":\"%s\","
        "\"current_value\":%.2f,"
        "\"threshold_value\":%.2f,"
        "\"message\":\"%s\""
        "}",
        MQTT_CLIENT_ID,
        (unsigned long long)alarm->timestamp,
        alarm->sensor_name,
        alarm->parameter,
        alarm->alarm_level,
        alarm->current_value,
        alarm->threshold_value,
        alarm->message
    );

    /* #3. 检查 JSON 构建结果 */
    if (len < 0 || (size_t)len >= buf_size) {
        fprintf(stderr, "MQTT alarm JSON构造失败或缓冲区不足\n");
        return -1;
    }

    return 0;
}



/******************** 回调函数 ********************/

/* MQTT 连接丢失回调函数 */
static void mqtt_connection_lost(void *context, char *cause)
{
    (void)context;

    /* #1. 打印 MQTT 断线原因 */
    fprintf(stderr, "MQTT connection lost");
    if (cause != NULL) {
        fprintf(stderr, ", cause: %s", cause);
    }

    fprintf(stderr, "\n");

    /* #2. 后续由 mqtt_client_loop() 或 mqtt_client_reconnect() 负责重新连接 */
}

/* MQTT 消息到达回调函数 */
static int mqtt_message_arrived(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
    (void)context; // 未使用参数
    (void)topicLen;// 未使用参数
    /* #1. 检查参数 */
    if(topicName == NULL || message == NULL) {
        return 1; // 返回非零值表示消息未处理
    }
    /* #2. 打印消息到达信息 */
    printf("MQTT message arrived, topic: %s\n", topicName);

    /* #3. 打印消息内容 */
    printf("MQTT payload: %.*s\n", message->payloadlen, (char*)message->payload);
    /* #4. 释放 MQTT 消息资源 */
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}

/* MQTT 消息发布完成回调函数 */
static void mqtt_delivery_complete(void *context, MQTTClient_deliveryToken token)
{
    (void)context;

    /* #1. 打印消息发送完成令牌 */
    printf("MQTT delivery complete, token=%d\n", token);
}