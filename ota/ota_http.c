/******************************************************************************
 * @file    ota_http.c
 * @brief   OTA HTTP文件下载模块实现
 *
 * @author  李宇坤
 * @date    2026-08
 * @version V1.0
 ******************************************************************************/

 #include "ota_http.h"
 #include "ota_config.h"
 
 #include <stdio.h>
 #include <sys/stat.h>
 #include <curl/curl.h>

 static int ota_http_progress_callback(void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow);

 static volatile int ota_http_cancel_flag = 0; // HTTP下载取消标志：0继续下载，1取消下载
 static size_t ota_downloaded_size = 0; // 当前已经下载的字节数
 static size_t ota_total_size = 0;      // 当前文件总字节数
/* HTTP模块初始化 */
int ota_http_init(void)
{
    CURLcode res;

    /* #1. 初始化libcurl全局环境 */
    res = curl_global_init(CURL_GLOBAL_DEFAULT);
    if (res != CURLE_OK) {
        printf("curl_global_init failed: %s\n", curl_easy_strerror(res));
        return -1;
    }

    /* #2. 初始化HTTP模块状态 */
    ota_http_cancel_flag = 0; // 清除下载取消标志
    ota_downloaded_size = 0;  // 清空当前已下载字节数
    ota_total_size = 0;       // 清空当前文件总字节数

    printf("OTA HTTP init success\n");
    return 0;
}

/* 普通HTTP文件下载 */
int ota_http_download(const char *url, const char *save_path) //输入下载地址和本地保存路径
{
    CURL *curl = NULL;//libcurl 里的“会话句柄类型”
    CURLcode res;
    FILE *fp = NULL;
    long http_code = 0;
    int ret = 0;
    /* #1. 检查输入参数 */
    if (url == NULL || save_path == NULL) {
        printf("OTA HTTP invalid parameter\n");
        return -1;
    }

    /* #2. 打开本地目标文件 */
    fp = fopen(save_path, "wb");
    if (fp == NULL) {
        printf("open download file failed: %s\n", save_path);
        return -1;
    }

    /* #3. 创建CURL句柄 */
    curl = curl_easy_init();
    if (curl == NULL) {
        printf("curl_easy_init failed\n");
        fclose(fp);
        return -1;
    }
    /* #4. 配置下载参数 */
    curl_easy_setopt(curl, CURLOPT_URL, url);                                  // 设置HTTP请求的下载地址
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fwrite);                     // 设置接收数据后的写入函数，使用fwrite写入文件
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);                             // 设置fwrite写入的目标文件指针
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);                        // 开启HTTP重定向，遇到301/302等状态码时自动跳转
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);                       // 设置连接服务器超时时间为10秒
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 300L);                             // 设置整个HTTP请求最大执行时间为300秒
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);                           // HTTP返回4xx/5xx错误码时，让curl_easy_perform返回失败
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, ota_http_progress_callback);
    curl_easy_setopt(curl, CURLOPT_XFERINFODATA, NULL);

    /* #5. 执行HTTP下载 */
    ota_http_cancel_flag = 0; // 重置取消标志
    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        if (res == CURLE_ABORTED_BY_CALLBACK) {
            printf("OTA HTTP download canceled\n");
        } else {
            printf("OTA HTTP download failed: %s\n", curl_easy_strerror(res));
        }
    
        curl_easy_cleanup(curl); // 释放CURL资源
        fclose(fp);              // 关闭本地文件
        return -1;
    }
    /* #6. 获取HTTP响应状态码 */
    if (curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code) != CURLE_OK) {
        printf("get HTTP response code failed\n");
        curl_easy_cleanup(curl);
        fclose(fp);
        remove(save_path);
        return -1;
    }

    /* #7. 检查HTTP状态码 */
    if (http_code < 200 || http_code >= 300) {
        printf("HTTP response error: %ld\n", http_code);
        curl_easy_cleanup(curl);
        fclose(fp);
        remove(save_path);
        return -1;
    }

    /* #8. 释放HTTP和文件资源 */
    curl_easy_cleanup(curl);
    fclose(fp);

    printf("OTA HTTP download success: %s\n", save_path);
    return 0;

}

/* HTTP Range断点续传 */
int ota_http_download_resume(const char *url, const char *save_path)//输入OTA升级包服务器地址、本地断点文件路径
{
    CURL *curl = NULL;             // libcurl请求句柄，用于配置和执行本次HTTP下载任务
    CURLcode res;                  // libcurl执行结果，用于保存curl_easy_perform()等函数的返回状态
    FILE *fp = NULL;               // 本地文件指针，用于把服务器返回的数据写入save_path文件
    struct stat file_stat;         // 文件状态信息结构体，用于获取本地已下载文件的大小
    curl_off_t local_size = 0;     // 本地已经下载的字节数，作为HTTP断点续传的起始位置
    long http_code = 0;            // HTTP响应状态码，例如200表示成功，206表示断点续传成功

    /* #1. 检查输入参数 */
    if (url == NULL || save_path == NULL) {
        printf("OTA HTTP resume invalid parameter\n");
        return -1;
    }

    /* #2. 获取本地已下载文件大小，不存在则从0开始下载 */
    if (stat(save_path, &file_stat) == 0) local_size = (curl_off_t)file_stat.st_size;

    /* #3. 以追加模式打开本地文件 */
    fp = fopen(save_path, "ab");//a是追加模式、b是二进制模式，ab表示以二进制追加模式打开文件，追加模式是不清空原文件，而是从文件末尾继续往后写。
    if (fp == NULL) {
        printf("open resume file failed: %s\n", save_path);
        return -1;
    }

    /* #4. 创建CURL句柄 */
    curl = curl_easy_init();
    if (curl == NULL) {
        printf("curl_easy_init failed\n");
        fclose(fp);
        return -1;
    }

    /* #5. 配置HTTP下载参数 */
    curl_easy_setopt(curl, CURLOPT_URL, url);                                      // 设置升级包下载地址
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fwrite);                         // 使用fwrite接收服务器返回的数据
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);                                 // 将接收到的数据追加写入本地文件
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);                            // 自动跟随HTTP重定向
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, (long)OTA_HTTP_CONNECT_TIMEOUT); // 设置服务器连接超时时间
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, (long)OTA_HTTP_TIMEOUT);                // 设置整个HTTP请求最大执行时间
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);                               // HTTP 4xx/5xx时判定请求失败
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, ota_http_progress_callback);
    curl_easy_setopt(curl, CURLOPT_XFERINFODATA, NULL);

    /* #6. 本地已有数据时，从断点位置继续下载 */
    if (local_size > 0) {
        curl_easy_setopt(curl, CURLOPT_RESUME_FROM_LARGE, local_size);              // 请求服务器从local_size字节处继续发送
        printf("OTA HTTP resume from: %lld bytes\n", (long long)local_size);
    }

    /* #7. 执行HTTP下载  先判断“网络传输是否成功”*/
    ota_http_cancel_flag = 0; // 重置取消标志
    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        if (res == CURLE_ABORTED_BY_CALLBACK) {
            printf("OTA HTTP download canceled\n");
        } else {
            printf("OTA HTTP download failed: %s\n", curl_easy_strerror(res));
        }
    
        curl_easy_cleanup(curl); // 释放CURL资源
        fclose(fp);              // 关闭本地文件
        return -1;
    }

    /* #8. 获取HTTP响应状态码  判断“服务器业务响应是否正确”*/
    if (curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code) != CURLE_OK) {
        printf("get HTTP response code failed\n");
        curl_easy_cleanup(curl);
        fclose(fp);
        return -1;
    }

    /* #9. 检查服务器是否正确支持断点续传 */
    if (local_size > 0 && http_code != 206) {
        printf("server does not support HTTP Range, response=%ld\n", http_code);
        curl_easy_cleanup(curl);
        fclose(fp);
        return -1;
    }

    /* #10. 首次完整下载必须返回2xx */
    if (local_size == 0 && (http_code < 200 || http_code >= 300)) {
        printf("HTTP response error: %ld\n", http_code);
        curl_easy_cleanup(curl);
        fclose(fp);
        return -1;
    }

    /* #11. 释放HTTP和文件资源 */
    curl_easy_cleanup(curl);
    fclose(fp);

    printf("OTA HTTP resume download success: %s\n", save_path);
    return 0;
}

/* 获取服务器文件大小 */
int ota_http_get_remote_size(const char *url, size_t *file_size)
{
    CURL *curl = NULL;                 // libcurl请求句柄
    CURLcode res;                      // libcurl执行结果
    curl_off_t content_length = 0;     // 服务器返回的文件大小
    long http_code = 0;                // HTTP响应状态码

    /* #1. 检查输入参数 */
    if (url == NULL || file_size == NULL) {
        printf("OTA HTTP get remote size invalid parameter\n");
        return -1;
    }

    /* #2. 创建CURL句柄 */
    curl = curl_easy_init();
    if (curl == NULL) {
        printf("curl_easy_init failed\n");
        return -1;
    }

    /* #3. 配置HEAD请求 */
    curl_easy_setopt(curl, CURLOPT_URL, url);                                      // 设置远程文件地址
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);                                    // 只获取响应头，不下载文件正文
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);                            // 自动跟随HTTP重定向
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, (long)OTA_HTTP_CONNECT_TIMEOUT); // 设置连接超时时间
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, (long)OTA_HTTP_TIMEOUT);                // 设置整个请求超时时间
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);                               // HTTP 4xx/5xx时判定失败

    /* #4. 执行HTTP HEAD请求 */
    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        printf("get remote file size failed: %s\n", curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        return -1;
    }

    /* #5. 获取HTTP响应状态码 */
    if (curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code) != CURLE_OK) {
        printf("get HTTP response code failed\n");
        curl_easy_cleanup(curl);
        return -1;
    }

    /* #6. 检查HTTP响应 */
    if (http_code < 200 || http_code >= 300) {
        printf("HTTP response error: %ld\n", http_code);
        curl_easy_cleanup(curl);
        return -1;
    }
    /* #7. 获取Content-Length */
    res = curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &content_length);//从 curl 这个请求中获取服务器返回的 Content-Length保存到 content_length
    if (res != CURLE_OK || content_length < 0) {
        printf("get content length failed: %s\n", curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        return -1;
    }

    /* #8. 保存远程文件大小 */
    *file_size = (size_t)content_length;

    /* #9. 释放CURL资源 */
    curl_easy_cleanup(curl);

    return 0;
}

/* 获取下载进度 */
int ota_http_get_progress(size_t *downloaded_size, size_t *total_size)
{


    /* #1. 检查输入参数 */
    if (downloaded_size == NULL || total_size == NULL) {
        printf("OTA HTTP get progress invalid parameter\n");
        return -1;
    }
    /* #2. 获取下载进度 */
    *downloaded_size = ota_downloaded_size;
    *total_size = ota_total_size;
    return 0;
}

/* 取消下载 */
int ota_http_cancel(void)
{
    /* #1. 释放libcurl资源 */
    ota_http_cancel_flag = 1; // 设置取消标志，后续下载操作应检查此标志并中止
    printf("OTA HTTP cancel\n");
    return 0;
}

/* HTTP模块释放 */
void ota_http_deinit(void)
{
    /* TODO: 释放libcurl等资源 */
}

/**
 * @brief  HTTP进度回调函数
 * @param  [in] clientp: 用户自定义数据指针
 * @param  [in] dltotal: 文件总字节数
 * @param  [in] dlnow: 当前已下载的字节数
 * @param  [in] ultotal: 上传总字节数
 * @param  [in] ulnow: 当前已上传的字节数
 * @return 0: 成功
 * **/

static int ota_http_progress_callback(void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow)
{
    (void)clientp;
    (void)ultotal;
    (void)ulnow;

    /* #1. 检查取消标志 */
    if (ota_http_cancel_flag) {
    printf("OTA HTTP download canceled\n");
    return 1; // 返回非0，通知libcurl终止当前下载
    }

    /* #2. 更新下载进度 */
    ota_downloaded_size = (size_t)dlnow;
    ota_total_size = (size_t)dltotal;

    printf("OTA HTTP download progress: %zu/%zu\n", ota_downloaded_size, ota_total_size);

    return 0; // 返回0，继续下载
}