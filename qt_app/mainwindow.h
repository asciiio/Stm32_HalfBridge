#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSerialPort/QtSerialPort>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void freq_btn_clicked();
    void amp_btn_clicked();
    void ph_btn_clicked();
    void start_btn_clicked();
    void stop_btn_clicked();
    void on_off_usb_btn_clicked();
    void readSerialData();


private:
    QSerialPort *serial;
    Ui::MainWindow *ui;
    void findPorts();
};
#endif // MAINWINDOW_H
