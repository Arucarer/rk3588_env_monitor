/******************************************************************************
 * @file    ota_http.h
 * @brief   OTA HTTP文件下载模块接口
 *
 * @author  李宇坤
 * @date    2026-08
 * @version V1.0
 *
 * @details
 * 本模块负责OTA升级过程中的HTTP网络通信，包括：
 * 1. 普通HTTP文件下载；
 * 2. HTTP Range断点续传；
 * 3. 获取远程文件大小；
 * 4. 查询下载进度；
 * 5. 取消当前下载任务。
 *
 * 本模块仅负责网络文件传输，不负责版本比较、MD5校验、
 * 程序安装和升级回滚。
 ******************************************************************************/

#ifndef __OTA_HTTP_H__
#define __OTA_HTTP_H__

#include <stddef.h>

/* 初始化HTTP下载模块 */
int ota_http_init(void);

/* 普通HTTP文件下载 */
int ota_http_download(const char *url, const char *save_path);

/* HTTP Range断点续传下载 */
int ota_http_download_resume(const char *url, const char *save_path);

/* 获取远程文件大小 */
int ota_http_get_remote_size(const char *url, size_t *file_size);

/* 获取当前下载进度 */
int ota_http_get_progress(size_t *downloaded_size, size_t *total_size);

/* 请求取消当前下载 */
int ota_http_cancel(void);

/* 释放HTTP模块资源 */
void ota_http_deinit(void);

#endif