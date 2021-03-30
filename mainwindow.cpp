#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSerialPortInfo>
#include <QSerialPort>
#include <QDebug>
#include <QMetaEnum>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , modbusDevice(nullptr)
{
    ui->setupUi(this);
    on_btnUpdate_clicked();
//    ui->btnConnect->setDisabled(true);
//    ui->cbxPort->setDisabled(true);
    if(ui->cbxPort->count()>0)
    {
        fillPortParams();
//        ui->btnConnect->setDisabled(false);
//        ui->cbxPort->setDisabled(false);
    }
    if(ui->cbxPort->count() == 1)
    {
        on_cbxPort_activated(ui->cbxPort->currentText());
//        ui->btnConnect->setDisabled(false);
//        ui->cbxPort->setDisabled(false);
    }
    ui->spinBoxCountReg->setValue(10);
    initRTU();
    ui->spinBoxId->setValue(16);
    timer = new QTimer(this);
    timer->setInterval(1000);
    connect(timer, SIGNAL(timeout()), this, SLOT(on_btnReadOne_clicked()));
}

MainWindow::~MainWindow()
{
    if(modbusDevice)
    {
        modbusDevice->disconnectDevice();
        delete modbusDevice;
    }
    modbusDevice = nullptr;
    delete port;
    delete ui;
}

void MainWindow::fillPortParams()
{
    QMetaEnum baudRateENUM = QMetaEnum::fromType<QSerialPort::BaudRate>();
    for (int i = 0; i < baudRateENUM.keyCount() - 1 ; i++ )
    {
        baudRate.append(QString::number(baudRateENUM.value(i)));
        ui->cbxBaudRate->addItem(QString::number(baudRateENUM.value(i)),
                                 baudRateENUM.value(i));
    }

    QMetaEnum dataBitsENUM = QMetaEnum::fromType<QSerialPort::DataBits>();
    for (int i = 0; i < dataBitsENUM.keyCount() - 1 ; i++ )
    {
        dataBits.append(QString::number(dataBitsENUM.value(i)));
        ui->cbxDataBits->addItem(QString::number(dataBitsENUM.value(i)),
                                 dataBitsENUM.value(i));
    }

    QMetaEnum stopBitsENUM = QMetaEnum::fromType<QSerialPort::StopBits>();
    for (int i = 0; i < stopBitsENUM.keyCount() - 1 ; i++ )
    {
        stopBits.append(QString::number(stopBitsENUM.value(i)));
        ui->cbxStopBits->addItem(stopBitsENUM.key(i), stopBitsENUM.value(i));
    }

    QMetaEnum parityENUM = QMetaEnum::fromType<QSerialPort::Parity>();
    for (int i = 0; i < parityENUM.keyCount() - 1 ; i++ )
    {
        parity.append(QString::number(parityENUM.value(i)));
        ui->cbxParity->addItem(parityENUM.key(i), parityENUM.value(i));
    }

    QMetaEnum flowControlENUM = QMetaEnum::fromType<QSerialPort::FlowControl>();
    for (int i = 0; i < flowControlENUM.keyCount() - 1 ; i++)
    {
        flowControl.append(QString::number(flowControlENUM.value(i)));
        ui->cbxFlowControl->addItem(flowControlENUM.key(i), flowControlENUM.value(i));
    }
    responseTimeout = 1000;
    ui->edtResponseTimeout->setText(QString::number(responseTimeout));

}

void MainWindow::initRTU()
{
    modbusDevice = new QModbusRtuSerialMaster(this);
    connect(modbusDevice, &QModbusClient::errorOccurred, [this](QModbusDevice::Error) {
                statusBar()->showMessage(modbusDevice->errorString(), 5000);
            });

    if(modbusDevice) {
        connect(modbusDevice, &QModbusClient::stateChanged,
                this, &MainWindow::onStateChanged);
    }
}

void MainWindow::onStateChanged(int state) {
    if (state == QModbusDevice::UnconnectedState)
        ui->btnConnect->setText(tr("Connect"));
    else if (state == QModbusDevice::ConnectedState)
        ui->btnConnect->setText(tr("Disconnect"));
}

void MainWindow::readReady()
{
    auto lastRequest = qobject_cast<QModbusReply *>(sender());
    if (!lastRequest)
        return;

    if (lastRequest->error() == QModbusDevice::NoError) {
        const QModbusDataUnit unit = lastRequest->result();
        for (uint i = 0; i < unit.valueCount(); i++) {
            const QString entry = tr("Address: %1, Value: %2").arg(unit.startAddress())
                                     .arg(QString::number(unit.value(i)));
            ui->listDataWidjet->addItem(entry);
        }
    } else if (lastRequest->error() == QModbusDevice::ProtocolError) {
        statusBar()->showMessage(tr("Read response error: %1 (Mobus exception: 0x%2)").
                                    arg(lastRequest->errorString()).
                                    arg(lastRequest->rawResult().exceptionCode(), -1, 16), 5000);
    } else {
        statusBar()->showMessage(tr("Read response error: %1 (code: 0x%2)").
                                    arg(lastRequest->errorString()).
                                    arg(lastRequest->error(), -1, 16), 5000);
    }

    lastRequest->deleteLater();
}

void MainWindow::setDataBits(int data)
{
    switch (data) {
    case 5:
        port->setDataBits(QSerialPort::Data5);
        break;
    case 6:
        port->setDataBits(QSerialPort::Data6);
        break;
    case 7:
        port->setDataBits(QSerialPort::Data7);
        break;
    case 8:
        port->setDataBits(QSerialPort::Data8);
        break;
    default:
        port->setDataBits(QSerialPort::UnknownDataBits);
        break;
    }
    qDebug() << port->dataBits();
}

void MainWindow::setStopBits(int data)
{
    switch (data)
    {
    case 1:
        port->setStopBits(QSerialPort::OneStop);
        break;
    case 3:
        port->setStopBits(QSerialPort::OneAndHalfStop);
        break;
    case 2:
        port->setStopBits(QSerialPort::TwoStop);
        break;
    default:
        port->setStopBits(QSerialPort::UnknownStopBits);
        break;
    }
    qDebug() << port->stopBits();
}

void MainWindow::setParity(int data)
{
    switch (data)
    {
    case 0:
        port->setParity(QSerialPort::NoParity);
        break;
    case 2:
        port->setParity(QSerialPort::EvenParity);
        break;
    case 3:
        port->setParity(QSerialPort::OddParity);
        break;
    case 4:
        port->setParity(QSerialPort::SpaceParity);
        break;
    case 5:
        port->setParity(QSerialPort::MarkParity);
        break;
    default:
        port->setParity(QSerialPort::UnknownParity);
        break;
    }
    qDebug() << port->parity();
}


void MainWindow::setFlowControl(int data)
{
    switch (data)
    {
    case 0:
        port->setFlowControl(QSerialPort::NoFlowControl);
        break;
    case 1:
        port->setFlowControl(QSerialPort::HardwareControl);
        break;
    case 2:
        port->setFlowControl(QSerialPort::SoftwareControl);
        break;
    default:
        port->setFlowControl(QSerialPort::UnknownFlowControl);

    }
    qDebug() << port->flowControl();

}

void MainWindow::on_btnUpdate_clicked()
{
    listPortInfo = QSerialPortInfo::availablePorts();
    if(listPortInfo.count() > 0)
    {
        QStringList namePort;
//        ui->btnConnect->setDisabled(false);
//        ui->cbxPort->setDisabled(false);
        for (int i = 0; i < listPortInfo.size() ; i++ )
        {
            namePort.append(listPortInfo.at(i).portName());
        }
        ui->cbxPort->clear();
        ui->cbxPort->addItems(namePort);
    }

}

void MainWindow::on_cbxPort_activated(const QString &arg1)
{
    port = new QSerialPort(arg1, this);
    ui->cbxBaudRate->setCurrentIndex(baudRate.indexOf(QString::number(port->baudRate())));
    ui->cbxDataBits->setCurrentIndex(dataBits.indexOf(QString::number(port->dataBits())));
    ui->cbxStopBits->setCurrentIndex(stopBits.indexOf(QString::number(port->stopBits())));
    ui->cbxParity->setCurrentIndex(parity.indexOf(QString::number(port->parity())));
    ui->cbxFlowControl->setCurrentIndex(flowControl.indexOf(QString::number(port->flowControl())));
}

void MainWindow::on_btnApplySettings_clicked()
{
    port->setBaudRate(ui->cbxBaudRate->currentData().toUInt());
    setDataBits(ui->cbxDataBits->currentData().toInt());
    setStopBits(ui->cbxStopBits->currentData().toUInt());
    setParity(ui->cbxParity->currentData().toUInt());
    setFlowControl(ui->cbxFlowControl->currentData().toInt());
}

void MainWindow::on_btnConnect_clicked()
{
    if (!modbusDevice)
            return;
    if (modbusDevice->state() != QModbusDevice::ConnectedState)
    {
        modbusDevice->setConnectionParameter(QModbusDevice::SerialPortNameParameter,
                                             port->portName());
        modbusDevice->setConnectionParameter(QModbusDevice::SerialParityParameter,
                                             port->parity());
        modbusDevice->setConnectionParameter(QModbusDevice::SerialBaudRateParameter,
                                             port->baudRate());
        modbusDevice->setConnectionParameter(QModbusDevice::SerialDataBitsParameter,
                                             port->dataBits());
        modbusDevice->setConnectionParameter(QModbusDevice::SerialStopBitsParameter,
                                             port->stopBits());
        modbusDevice->setTimeout(responseTimeout);
        modbusDevice->setNumberOfRetries(1);
        if (!modbusDevice->connectDevice()) {
            statusBar()->showMessage(tr("Connect failed: ") + modbusDevice->errorString(), 5000);
        }
    }
    else
    {
        modbusDevice->disconnectDevice();
    }
}

void MainWindow::on_btnReadOne_clicked()
{
    if (!modbusDevice)
        return;
    ui->listDataWidjet->clear();
    statusBar()->clearMessage();
    if (auto *lastRequest = modbusDevice->sendReadRequest(readRequest(ui->spinBoxRegister->value(),
                                                                      ui->spinBoxCountReg->value())
                                                          , ui->spinBoxId->value())) {
        if (!lastRequest->isFinished())
            connect(lastRequest, &QModbusReply::finished, this, &MainWindow::readReady);
        else
            delete lastRequest; // broadcast replies return immediately
    } else {
        statusBar()->showMessage(tr("Read error: ") + modbusDevice->errorString(), 5000);
    }
}

QModbusDataUnit MainWindow::readRequest(int reg, int count) const
{
    return QModbusDataUnit(QModbusDataUnit::HoldingRegisters, reg, count);
}

QModbusDataUnit MainWindow::writeRequest(int reg, int count) const
{
    return QModbusDataUnit(QModbusDataUnit::HoldingRegisters, reg, count);
}

void MainWindow::on_btnReadLoop_clicked()
{
    if(timer->isActive())
    {

        ui->btnReadLoop->setText(tr("Start read"));
        ui->btnReadLoop->setStyleSheet("background-color: rgb(0,255,0);");
        timer->stop();
    }
    else
    {
        ui->btnReadLoop->setText(tr("Stop read"));
        ui->btnReadLoop->setStyleSheet("background-color: rgb(255,0,0);");
        timer->start();
    }
//    qDebug() << timer->isActive();

}
