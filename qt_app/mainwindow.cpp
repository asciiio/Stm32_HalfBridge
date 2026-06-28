#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QtSerialPort/QSerialPort>

bool isRunning = 0;
bool isConnected = 0;
bool first_time_flag = 1;
QList<QSerialPortInfo> availablePorts;

void MainWindow::findPorts(){
    ui->port_choose_box->clear();
    availablePorts = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &portInfo : availablePorts){
        qDebug() << portInfo.portName();
        ui->port_choose_box->addItem(portInfo.portName());
    }
}


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , serial(new QSerialPort(this))
{
    ui->setupUi(this);
    findPorts();
    connect(serial, &QSerialPort::readyRead, this, &MainWindow::readSerialData);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::freq_btn_clicked()
{
    if (isRunning && isConnected){
        QString cmd_str;
        QString freq_input_val = ui->freq_in->toPlainText();

        qDebug() << freq_input_val;
        ui->freq_lcd->display(freq_input_val);

        cmd_str = "set freq ";
        cmd_str += freq_input_val;

        qDebug() << cmd_str << "  " << cmd_str.size();
        serial->write(cmd_str.toUtf8(), cmd_str.size());
    }
    else {
        ui->log_lable->setText("ОШИБКА: Сначала нажмите СТАРТ!");
    }
}

void MainWindow::amp_btn_clicked()
{
    if (isRunning && isConnected){
        QString cmd_str;
        QString amp_input_val = ui->amp_in->toPlainText();

        qDebug() << amp_input_val;
        ui->amp_lcd->display(amp_input_val);

        cmd_str = "set amp ";
        cmd_str += amp_input_val;


        qDebug() << cmd_str << "  " << cmd_str.size();
        serial->write(cmd_str.toUtf8(), cmd_str.size());
    }
    else {
        ui->log_lable->setText("ОШИБКА: Сначала нажмите СТАРТ!");
    }

}

void MainWindow::ph_btn_clicked()
{
    if (isRunning && isConnected){
        QString cmd_str;
        QString ph_input_val = ui->ph_in->toPlainText();

        qDebug() << ph_input_val;
        ui->ph_lcd->display(ph_input_val);

        cmd_str = "set ph ";
        cmd_str += ph_input_val;

        qDebug() << cmd_str << "  " << cmd_str.size();
        serial->write(cmd_str.toUtf8(), cmd_str.size());
    }
    else {
        ui->log_lable->setText("ОШИБКА: Сначала нажмите СТАРТ!");
    }
}

void MainWindow::start_btn_clicked()
{
    if (serial->isOpen()){
        serial->write("start", 5);
        isRunning = 1;
        if (first_time_flag){
            ui->amp_lcd->display(1.0);
            ui->freq_lcd->display(50);
            ui->ph_lcd->display(120);
            first_time_flag = 0;
        }
    }
    else {
        ui->log_lable->setText("ОШИБКА: Сначала откройте порт!");
    }

}

void MainWindow::stop_btn_clicked()
{
    if (isConnected){
        serial->write("stop", 4);
        isRunning = 0;
    }
    else {
        ui->log_lable->setText("ОШИБКА: Сначала откройте порт!");
    }

}

void MainWindow::on_off_usb_btn_clicked(){
    if (isConnected == 0){
        if (ui->port_choose_box->count() == 0){
            ui->log_lable->setText("Не найдено ни одного CDC у-ва!");
            return;
        }
        serial->setPortName(ui->port_choose_box->currentText());
        serial->open(QIODevice::ReadWrite);
        ui->on_off_usb_btn->setText("Откл.");
        isConnected = 1;
    }
    else {
        serial->close();
        ui->on_off_usb_btn->setText("Вкл.");
        isConnected = 0;

    }
}



void MainWindow::readSerialData(){
    QByteArray data = serial->readAll();
    if (!data.isEmpty()){
        QString receivedText = QString::fromUtf8(data);
        ui->log_lable->setText(receivedText);
    }
}




