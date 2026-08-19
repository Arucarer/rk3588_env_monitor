/******************************************************************************
 * @file    ota_http.c
 * @brief   OTA HTTP文件下载模块实现
 *
 * @author  李宇坤
 * @date    2026-08
 * @version V1.0
 ******************************************************************************/

#include "ota_http.h"

#include <stdio.h>

/* HTTP模块初始化 */
int ota_http_init(void)
{
    /* TODO: 初始化libcurl等HTTP资源 */

    return 0;
}

/* 普通HTTP文件下载 */
int ota_http_download(const char *url, const char *save_path)
{
    /* TODO:
     * 1. 参数检查
     * 2. 创建本地文件
     * 3. 发起HTTP GET请求
     * 4. 写入本地文件
     * 5. 检查HTTP状态码
     */

    (void)url;
    (void)save_path;

    return 0;
}

/* HTTP Range断点续传 */
int ota_http_download_resume(const char *url, const char *save_path)
{
    /* TODO:
     * 1. 获取本地已下载文件大小
     * 2. 设置HTTP Range
     * 3. 从断点位置继续下载
     * 4. 更新下载进度
     */

    (void)url;
    (void)save_path;

    return 0;
}

/* 获取服务器文件大小 */
int ota_http_get_remote_size(const char *url, size_t *file_size)
{
    /* TODO:
     * 通过HEAD请求或者Content-Length获取文件大小
     */

    (void)url;
    (void)file_size;

    return 0;
}

/* 获取下载进度 */
int ota_http_get_progress(size_t *downloaded_size, size_t *total_size)
{
    /* TODO:
     * 返回当前HTTP任务的下载字节数和总字节数
     */

    (void)downloaded_size;
    (void)total_size;

    return 0;
}

/* 取消下载 */
int ota_http_cancel(void)
{
    /* TODO:
     * 设置取消标志，使正在运行的HTTP下载安全退出
     */

    return 0;
}

/* HTTP模块释放 */
void ota_http_deinit(void)
{
    /* TODO: 释放libcurl等资源 */
}