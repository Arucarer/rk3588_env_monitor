# RK3588 Linux 智能环境综合监测系统

## 项目简介

本项目基于正点原子 RK3588B 开发板，设计并实现一套运行于 Linux 平台的智能环境综合监测系统。

系统采用模块化设计，集成多种环境传感器数据采集、本地可视化、数据库存储、云端通信以及远程升级等功能，完整覆盖嵌入式 Linux 工业物联网终端的开发流程。

本项目目前正在持续开发中。

---

## 项目目标

- 基于 Linux Kernel 完成底层驱动开发
- 完成 Device Tree (DTS) 配置
- 实现 I2C、UART、GPIO、ADC 外设驱动
- 实现 Modbus RTU 多传感器采集
- 开发 Qt 本地监控界面
- 使用 SQLite 存储历史数据
- 基于 MQTT 上传云平台
- 实现 HTTP OTA 在线升级
- 完成系统工程化部署

---

## 开发平台

| 项目 | 内容 |
|------|------|
| CPU | Rockchip RK3588 |
| 开发板 | 正点原子 RK3588B |
| 操作系统 | Linux |
| 开发语言 | C / C++ |
| GUI | Qt5 |

---

## 技术栈

- Linux Kernel
- Device Tree (DTS)
- UART
- I2C
- ADC
- GPIO
- Modbus RTU
- SQLite
- Qt5
- MQTT
- HTTP OTA
- Shell Script

---

## 已购买硬件

| 模块 | 接口 |
|------|------|
| BMP280/BME280 | I2C |
| 电容式土壤湿度传感器 | ADC |
| 雨滴检测模块 | GPIO |
| TTL → RS485 模块 | UART |
| RS485 臭氧传感器 | Modbus RTU |
| RS485 PM1.0 / PM2.5 / PM10 模块 | Modbus RTU |

---

## 项目目录

```text
rk3588_env_monitor/
├── app/
├── include/
├── sensor/
├── driver/
├── modbus/
├── database/
├── mqtt/
├── qt/
├── ota/
├── shell/
├── log/
└── test/
```

---

## 当前开发进度

- [x] 项目框架搭建
- [ ] 底层驱动开发
- [ ] Modbus RTU
- [ ] Sensor Manager
- [ ] Qt 上位机
- [ ] SQLite
- [ ] MQTT
- [ ] OTA
- [ ] 系统部署

---

## 更新日志

### V1.0

- 创建项目
- 建立工程目录
- 初始化 README

---

## 作者

**李宇坤**

2026/7/7