/******************************************************************************
 * @file    ota_types.h
 * @brief   OTA远程升级公共数据类型定义
 *
 * @author  李宇坤
 * @date    2026-08
 * @version V1.0
 *
 * @details
 * 本文件定义OTA模块公共数据类型，包括：
 * 1. OTA升级类型；
 * 2. OTA升级级别；
 * 3. OTA运行状态；
 * 4. OTA错误码；
 * 5. OTA升级信息结构体；
 * 6. OTA运行状态结构体。
 ******************************************************************************/

 #ifndef __OTA_TYPES_H__
 #define __OTA_TYPES_H__
 
 #include <stddef.h>
 
 /* OTA字符串长度 */
 #define OTA_DEVICE_NAME_MAX_LEN      64
 #define OTA_VERSION_MAX_LEN          32
 #define OTA_URL_MAX_LEN             256
 #define OTA_MD5_MAX_LEN              33
 #define OTA_ERROR_MESSAGE_MAX_LEN   128
 
 /* OTA升级类型 */
 typedef enum
 {
     OTA_TYPE_FULL = 0,   // 全量升级
     OTA_TYPE_DELTA       // 差分升级
 } ota_update_type_t;
 
 /* OTA升级级别 */
 typedef enum
 {
     OTA_LEVEL_NORMAL = 0, // 普通升级
     OTA_LEVEL_IMPORTANT,  // 重要升级
     OTA_LEVEL_FORCE       // 强制升级
 } ota_update_level_t;
 
 /* OTA运行状态 */
 typedef enum
 {
     OTA_STATE_IDLE = 0,      // 空闲
     OTA_STATE_CHECKING,      // 正在检查版本
     OTA_STATE_DOWNLOADING,   // 正在下载升级包
     OTA_STATE_VERIFYING,     // 正在校验升级包
     OTA_STATE_PATCHING,      // 正在应用差分补丁
     OTA_STATE_BACKUP,        // 正在备份旧版本
     OTA_STATE_INSTALLING,    // 正在安装新版本
     OTA_STATE_HEALTH_CHECK,  // 正在执行健康检查
     OTA_STATE_COMMITTING,    // 正在提交升级结果
     OTA_STATE_ROLLBACK,      // 正在回滚
     OTA_STATE_SUCCESS,       // 升级成功
     OTA_STATE_FAILED,        // 升级失败
     OTA_STATE_CANCELLED      // 升级取消
 } ota_state_t;
 
 /* OTA错误码 */
 typedef enum
 {
     OTA_ERR_NONE = 0,              // 无错误
 
     OTA_ERR_INVALID_PARAM,         // 参数错误
 
     OTA_ERR_HTTP_INIT,             // HTTP模块初始化失败
     OTA_ERR_HTTP_DOWNLOAD,         // HTTP下载失败
     OTA_ERR_HTTP_REMOTE_SIZE,      // 获取远程文件大小失败
 
     OTA_ERR_VERSION_FILE,          // version.json文件异常
     OTA_ERR_VERSION_PARSE,         // version.json解析失败
     OTA_ERR_VERSION_INVALID,       // 版本号非法
     OTA_ERR_NO_UPDATE,             // 当前无新版本
 
     OTA_ERR_DEVICE_MISMATCH,       // 设备型号不匹配
     OTA_ERR_BASE_VERSION,          // 差分基础版本不匹配
     OTA_ERR_DUPLICATE_VERSION,     // 重复升级
 
     OTA_ERR_LOCK,                  // 获取升级锁失败
 
     OTA_ERR_FILE_SIZE,             // 文件大小校验失败
     OTA_ERR_MD5,                   // MD5校验失败
 
     OTA_ERR_PATCH,                 // 差分补丁应用失败
     OTA_ERR_BACKUP,                // 当前程序备份失败
     OTA_ERR_INSTALL,               // 新程序安装失败
     OTA_ERR_HEALTH_CHECK,          // 新版本健康检查失败
     OTA_ERR_COMMIT,                // 提交升级结果失败
     OTA_ERR_ROLLBACK,              // 回滚失败
 
     OTA_ERR_CANCELLED,             // OTA被取消
     OTA_ERR_INTERNAL               // OTA内部错误
 } ota_error_t;
 
 /* OTA服务器升级信息 */
 typedef struct
 {
     char device[OTA_DEVICE_NAME_MAX_LEN];      // 目标设备型号
     char version[OTA_VERSION_MAX_LEN];         // 目标版本号
     char base_version[OTA_VERSION_MAX_LEN];    // 差分基础版本号
 
     ota_update_type_t type;                    // 升级类型
     ota_update_level_t level;                  // 升级级别
 
     char url[OTA_URL_MAX_LEN];                 // 升级包下载地址
     char md5[OTA_MD5_MAX_LEN];                 // 升级包MD5
 
     size_t size;                               // 升级包大小
 } ota_update_info_t;
 
 /* OTA当前运行状态 */
 typedef struct
 {
     ota_state_t state;                         // 当前OTA状态
 
     char current_version[OTA_VERSION_MAX_LEN]; // 当前版本号
     char target_version[OTA_VERSION_MAX_LEN];  // 目标版本号
 
     int progress;                              // 当前进度，0~100
 
     ota_error_t error_code;                    // 当前错误码
     char error_message[OTA_ERROR_MESSAGE_MAX_LEN]; // 错误描述
 } ota_status_t;
 
 #endif