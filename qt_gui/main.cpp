#include "mainwindow.h"
#include "sensordata.h"

#include <QApplication>
#include <QMetaType>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    qRegisterMetaType<SensorData>("SensorData");

    MainWindow w;
    w.show();

    return QApplication::exec();
}