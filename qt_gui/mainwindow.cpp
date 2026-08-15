#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QRandomGenerator>
#include <QTimer>
#include <QtMath>
#include <QButtonGroup>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , clockTimer(nullptr)
    , mockTimer(nullptr)
{
    ui->setupUi(this);

    setupConnections();
    setupTimers();

    updateClock();
    updateMockData();
}

MainWindow::~MainWindow()
{
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

    mockTimer = new QTimer(this);
    connect(mockTimer, &QTimer::timeout,
            this, &MainWindow::updateMockData);
    mockTimer->start(1000);
}

void MainWindow::updateClock()
{
    ui->timeLabel->setText(
        QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));
}

void MainWindow::updateMockData()
{
    static double phase = 0.0;
    phase += 0.15;

    auto noise = []() {
        return (QRandomGenerator::global()->generateDouble() - 0.5) * 0.6;
    };

    SensorData data;

    data.temperature =
        25.0 + 2.0 * qSin(phase) + noise();

    data.humidity =
        55.0 + 5.0 * qSin(phase * 0.7) + noise();

    data.pressure =
        1013.0 + 4.0 * qSin(phase * 0.3) + noise();

    data.rs485Temperature =
        24.5 + 1.8 * qSin(phase * 0.9) + noise();

    data.rs485Humidity =
        53.0 + 4.0 * qSin(phase * 0.6) + noise();

    data.soilHumidity =
        42.0 + 3.0 * qSin(phase * 0.2) + noise();

    data.rainfall = qSin(phase * 0.4) > 0.75 ? 2.4 : 0.0;

    data.bme280Online = true;
    data.rs485Online = true;
    data.soilSensorOnline = true;
    data.rainSensorOnline = true;

    data.timestamp = QDateTime::currentDateTime();

    updateSensorDisplay(data);
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