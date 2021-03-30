#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QSerialPort>
#include <QSerialPortInfo>
#include <QModbusDataUnit>
#include <QModbusRtuSerialMaster>
#include <QMetaEnum>
#include <QTimer>

class QModbusClient;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QModbusDataUnit readRequest(int, int) const;
    QModbusDataUnit writeRequest(int, int) const;

    void fillPortParams();
    void initRTU();
    void setDataBits(int data);
    void setStopBits(int data);
    void setParity(int data);
    void setFlowControl(int data);
private slots:
    void on_btnUpdate_clicked();
    void on_cbxPort_activated(const QString &arg1);
    void on_btnApplySettings_clicked();
    void onStateChanged(int state);
    void on_btnConnect_clicked();
    void on_btnReadOne_clicked();

    void readReady();

    void on_btnReadLoop_clicked();

private:
    QSerialPort *port;
    Ui::MainWindow *ui;
    QModbusClient *modbusDevice;
    QList <QSerialPortInfo> listPortInfo;

    int responseTimeout;

    QTimer *timer;

    QStringList baudRate, dataBits, stopBits, parity, flowControl, direction;

};
#endif // MAINWINDOW_H
