#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "sensordata.h"

class QTimer;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    // 将一份完整传感器数据刷新到界面
    void updateSensorDisplay(const SensorData &data);

    // 生成一份模拟传感器数据
    void updateMockData();

    // 更新右上角时间
    void updateClock();

private:
    // 初始化页面按钮、定时器等连接
    void setupConnections();
    void setupTimers();

private:
    Ui::MainWindow *ui;

    QTimer *clockTimer;
    QTimer *mockTimer;
};

#endif // MAINWINDOW_H