/******************************************************************************
 * @file    mainwindow.h
 * @brief   Qt 环境监测主窗口类声明
 *
 * @author  李宇坤
 * @date    2026-08
 * @version V1.0
 *
 * @details
 * 本文件定义环境监测系统主窗口 MainWindow。
 * MainWindow 主要负责：
 * 1. 实时监测、趋势曲线、历史数据、告警日志和系统设置页面管理；
 * 2. 接收 SensorData 并刷新界面；
 * 3. 管理各 Worker 与 QThread；
 * 4. 处理用户界面事件和系统状态显示。
 *
 * MainWindow 只负责界面展示和线程组织，不直接执行传感器采集、
 * SQLite 数据库操作或 MQTT 网络通信。
 ******************************************************************************/

/*头文件保护*/
#ifndef MAINWINDOW_H
#define MAINWINDOW_H
/*要使用 Qt 提供的 QMainWindow 类*/
#include <QMainWindow>

#include "sensordata.h"

class QTimer;
class QThread;
class AcquisitionWorker;
class DatabaseWorker;
class MqttWorker;

/*mainwindow.ui在编译时会自动生成一个类，大概相当于Ui::MainWindow*/
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

/*定义你自己的MainWindow 类，继承QMainWindow*/
class MainWindow : public QMainWindow
{
    Q_OBJECT

    //构造函数
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;
    //析构函数

private slots:
    // 将一份完整传感器数据刷新到界面
    void updateSensorDisplay(const SensorData &data);

    // 更新右上角时间
    void updateClock();

private:
    // 初始化页面按钮等连接
    void setupConnections();

    // 初始化界面定时器
    void setupTimers();

    // 初始化采集线程
    void setupAcquisitionThread();

    void setupDatabaseThread();

    void setupMqttThread();

private:
    Ui::MainWindow *ui;

    // UI 时间定时器
    QTimer *clockTimer;

    // 采集线程
    QThread *acquisitionThread;

    // 采集工作对象
    AcquisitionWorker *acquisitionWorker;

    QThread *databaseThread;
    DatabaseWorker *databaseWorker;

    QThread *mqttThread;
    MqttWorker *mqttWorker;
};

#endif // MAINWINDOW_H