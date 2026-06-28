#include <QApplication>
#include <QMainWindow>
#include <QDebug>

#include "ui_mainwindow.h"
#include "mainwindow.h"
#include <iostream>


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    MainWindow window;
    window.show();

    return app.exec();
}
