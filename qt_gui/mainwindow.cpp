/******************************************************************************
 * @file    mainwindow.cpp
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

#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "acquisitionworker.h"
#include "databaseworker.h"
#include "mqttworker.h"

#include <QTimer>
#include <QThread>
#include <QButtonGroup>
#include <QMetaObject>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , clockTimer(nullptr)
    , acquisitionThread(nullptr)
    , acquisitionWorker(nullptr)
    , databaseThread(nullptr)
    , databaseWorker(nullptr)
    , mqttThread(nullptr)
    , mqttWorker(nullptr)
{
    ui->setupUi(this);

    setupConnections();
    setupTimers();

    setupDatabaseThread();
    setupMqttThread();
    setupAcquisitionThread();

    updateClock();
}

//析构函数,安全退出三个线程
MainWindow::~MainWindow()
{
    if (acquisitionThread != nullptr &&
        acquisitionThread->isRunning()) {

        QMetaObject::invokeMethod(
            acquisitionWorker,
            "stop",
            Qt::BlockingQueuedConnection);

        acquisitionThread->quit();
        acquisitionThread->wait();
    }

    if (databaseThread != nullptr &&
        databaseThread->isRunning()) {
        //停止这个 Worker 自己的业务
        QMetaObject::invokeMethod(
            databaseWorker,
            "stop",
            Qt::BlockingQueuedConnection);
        //让线程事件循环退出
        databaseThread->quit();
        //主线程等它真正结束
        databaseThread->wait();
    }

    if (mqttThread != nullptr &&
        mqttThread->isRunning()) {

        QMetaObject::invokeMethod(
            mqttWorker,
            "stop",
            Qt::BlockingQueuedConnection);

        mqttThread->quit();
        mqttThread->wait();
    }

    delete ui;
}



void MainWindow::setupConnections()
{
    auto *navigationGroup = new QButtonGroup(this);

    navigationGroup->setExclusive(true);

    navigationGroup->addButton(ui->realtimeButton);
    navigationGroup->addButton(ui->trendButton);
    navigationGroup->addButton(ui->historyButton);
    navigationGroup->addButton(ui->alarmButton);
    navigationGroup->addButton(ui->settingsButton);

    ui->realtimeButton->setCheckable(true);
    ui->trendButton->setCheckable(true);
    ui->historyButton->setCheckable(true);
    ui->alarmButton->setCheckable(true);
    ui->settingsButton->setCheckable(true);

    ui->realtimeButton->setChecked(true);

    connect(ui->realtimeButton, &QPushButton::clicked, this, [this]() {
        ui->pageStackedWidget->setCurrentWidget(ui->realtimePage);
        ui->pageTitleLabel->setText(QStringLiteral("实时环境监测"));
    });

    connect(ui->trendButton, &QPushButton::clicked, this, [this]() {
        ui->pageStackedWidget->setCurrentWidget(ui->trendPage);
        ui->pageTitleLabel->setText(QStringLiteral("趋势曲线"));
    });

    connect(ui->historyButton, &QPushButton::clicked, this, [this]() {
        ui->pageStackedWidget->setCurrentWidget(ui->historyPage);
        ui->pageTitleLabel->setText(QStringLiteral("历史数据"));
    });

    connect(ui->alarmButton, &QPushButton::clicked, this, [this]() {
        ui->pageStackedWidget->setCurrentWidget(ui->alarmPage);
        ui->pageTitleLabel->setText(QStringLiteral("告警日志"));
    });

    connect(ui->settingsButton, &QPushButton::clicked, this, [this]() {
        ui->pageStackedWidget->setCurrentWidget(ui->settingsPage);
        ui->pageTitleLabel->setText(QStringLiteral("系统设置"));
    });
}

void MainWindow::setupTimers()
{
    clockTimer = new QTimer(this);
    connect(clockTimer, &QTimer::timeout,
            this, &MainWindow::updateClock);
    clockTimer->start(1000);

}

void MainWindow::updateClock()
{
    ui->timeLabel->setText(
        QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));
}


void MainWindow::updateSensorDisplay(const SensorData &data)
{
    ui->bmeTemperatureValueLabel->setText(
        QString::number(data.temperature, 'f', 1) + QStringLiteral(" °C"));

    ui->bmeHumidityValueLabel->setText(
        QString::number(data.humidity, 'f', 1) + QStringLiteral(" %"));

    ui->pressureValueLabel->setText(
        QString::number(data.pressure, 'f', 1) + QStringLiteral(" hPa"));

    ui->rs485TemperatureValueLabel->setText(
        QString::number(data.rs485Temperature, 'f', 1) + QStringLiteral(" °C"));

    ui->rs485HumidityValueLabel->setText(
        QString::number(data.rs485Humidity, 'f', 1) + QStringLiteral(" %"));

    ui->soilHumidityValueLabel->setText(
        QString::number(data.soilHumidity, 'f', 1) + QStringLiteral(" %"));

    ui->rainfallValueLabel->setText(
        QString::number(data.rainfall, 'f', 1) + QStringLiteral(" mm"));

    const bool allOnline =
        data.bme280Online &&
        data.rs485Online &&
        data.soilSensorOnline &&
        data.rainSensorOnline;

    if (allOnline) {
        ui->deviceStatusValueLabel->setText(QStringLiteral("在线"));
        ui->deviceStatusValueLabel->setStyleSheet(
            "color: rgb(39, 174, 96); font-size: 24px; font-weight: 600;");

        ui->systemStatusLabel->setText(QStringLiteral("● 系统正常"));
        ui->systemStatusLabel->setStyleSheet(
            "color: rgb(39, 174, 96);");
    } else {
        ui->deviceStatusValueLabel->setText(QStringLiteral("部分离线"));
        ui->deviceStatusValueLabel->setStyleSheet(
            "color: rgb(231, 76, 60); font-size: 24px; font-weight: 600;");

        ui->systemStatusLabel->setText(QStringLiteral("● 设备异常"));
        ui->systemStatusLabel->setStyleSheet(
            "color: rgb(231, 76, 60);");
    }
}


void MainWindow::setupDatabaseThread()
{
    /*以数据库这个为例*/
    databaseThread = new QThread(this);
    /*创建一条数据库线程*/
    databaseWorker = new DatabaseWorker();
    /*创建一个真正负责数据库工作的对象*/
    databaseWorker->moveToThread(databaseThread);
    /*databaseThread 启动-自动调用DatabaseWorker::start()*/
    connect(databaseThread,
            &QThread::started,
            databaseWorker,
            &DatabaseWorker::start);
    /*数据库线程结束以后，让 Qt 安全地销毁 databaseWorker*/
    connect(databaseThread,
            &QThread::finished,
            databaseWorker,
            &QObject::deleteLater);
    /**/
    databaseThread->start();
}

void MainWindow::setupMqttThread()
{
    mqttThread = new QThread(this);
    mqttWorker = new MqttWorker();

    mqttWorker->moveToThread(mqttThread);

    connect(mqttThread,
            &QThread::started,
            mqttWorker,
            &MqttWorker::start);

    connect(mqttThread,
            &QThread::finished,
            mqttWorker,
            &QObject::deleteLater);

    mqttThread->start();
}

void MainWindow::setupAcquisitionThread()
{
    acquisitionThread = new QThread(this);
    acquisitionWorker = new AcquisitionWorker();

    acquisitionWorker->moveToThread(acquisitionThread);

    connect(acquisitionThread,
            &QThread::started,
            acquisitionWorker,
            &AcquisitionWorker::start);

    connect(acquisitionWorker,
            &AcquisitionWorker::sensorDataReady,
            this,
            &MainWindow::updateSensorDisplay);
    connect(acquisitionWorker,
            &AcquisitionWorker::sensorDataReady,
            databaseWorker,
            &DatabaseWorker::saveSensorData);

    connect(acquisitionWorker,
            &AcquisitionWorker::sensorDataReady,
            mqttWorker,
            &MqttWorker::publishSensorData);

    connect(acquisitionThread,
            &QThread::finished,
            acquisitionWorker,
            &QObject::deleteLater);

    acquisitionThread->start();
}