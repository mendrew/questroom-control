#include "mainwindow.hpp"
#include "ui_mainwindow.h"

#include <QSerialPort>
#include <QSerialPortInfo>

#include "qrc_protocol.hpp"

enum {
    REPEAT_INTERVAL = 500, // msec
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->tableViewLeds->setModel(&ledModel);
#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
    ui->tableViewLeds->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
#else
    ui->tableViewLeds->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
#endif
    connect(&ledModel,SIGNAL(ledsChanged(QByteArray)), SLOT(ledsChanged(QByteArray)));

    ui->tableViewSmartLeds->setModel(&smartLedModel);
#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
    ui->tableViewSmartLeds->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
#else
    ui->tableViewSmartLeds->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
#endif
    connect(&smartLedModel,SIGNAL(ledsChanged(QByteArray)), SLOT(smartLedsChanged(QByteArray)));
    connect(&smartLedModel,SIGNAL(ledChanged(int,int,int,int)), SLOT(smartLedChanged(int,int,int,int)));

    connect(ui->checkBoxRelay0, SIGNAL(clicked()), SLOT(relayClicked()));
    connect(ui->checkBoxRelay1, SIGNAL(clicked()), SLOT(relayClicked()));
    connect(ui->checkBoxRelay2, SIGNAL(clicked()), SLOT(relayClicked()));
    connect(ui->checkBoxRelay3, SIGNAL(clicked()), SLOT(relayClicked()));

    rescanAvailablePorts();

    connect(&hardware, SIGNAL(started()), SLOT(hardwareStarted()));
    connect(&hardware, SIGNAL(stopped()), SLOT(hardwareStopped()));

    connect(&hardware, SIGNAL(error(QString)),                SLOT(hardwareError(QString)));
    connect(&hardware, SIGNAL(parseError(int, QByteArray)),   SLOT(hardwareParseError(int, QByteArray)));
    connect(&hardware, SIGNAL(replySilent(int, int)),         SLOT(hardwareReplySilent(int, int)));
    connect(&hardware, SIGNAL(replyTicketSuccess()),          SLOT(hardwareTicketSuccess()));
    connect(&hardware, SIGNAL(replyTicketUnknown()),          SLOT(hardwareTicketUnknown()));
    connect(&hardware, SIGNAL(timeout(int, int, QByteArray)), SLOT(hardwareTimeout(int, int, QByteArray)));

    connect(&hardware, SIGNAL(replyHello()),                SLOT(hardwareHello()));
    connect(&hardware, SIGNAL(replyKeys(QList<bool>)),      SLOT(hardwareKeys(QList<bool>)));
    connect(&hardware, SIGNAL(replySliders(QList<int>)),    SLOT(hardwareSliders(QList<int>)));
    connect(&hardware, SIGNAL(replyEncoders(QList<int>)),   SLOT(hardwareEncoders(QList<int>)));
    connect(&hardware, SIGNAL(replySensors(QList<int>)),    SLOT(hardwareSensors(QList<int>)));
    connect(&hardware, SIGNAL(replyStikyKeys(QList<bool>)), SLOT(hardwareStikyKeys(QList<bool>)));
    connect(&hardware, SIGNAL(replyState(QList<bool>,QList<int>,QList<int>,QList<int>,QList<bool>))
            , SLOT(hardwareState(QList<bool>,QList<int>,QList<int>,QList<int>,QList<bool>)));
    repeatTimer.setInterval(REPEAT_INTERVAL) ;
    repeatTimer.setSingleShot(false);
    connect(&repeatTimer, SIGNAL(timeout()), this, SLOT(on_pushButtonState_clicked()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::hardwareStarted()
{
    ui->checkBoxPortStart->setEnabled(true);
    ui->checkBoxPortStart->setChecked(true);
    enableConnectControls(true);
}

void MainWindow::hardwareStopped()
{
    rescanAvailablePorts();
    ui->checkBoxPortStart->setEnabled(true);
    ui->checkBoxPortStart->setChecked(false);
    enableConnectControls(false);
}

void MainWindow::hardwareError(const QString& message)
{
    ui->labelErrorResult->setText(message);
}

void MainWindow::hardwareParseError(int error, const QByteArray& data)
{
    ui->labelErrorResult->setText(QString(tr("Ошибка разбора 0x%1. Пакет \"%2\"")).arg(error,2,16,QLatin1Char('0')).arg(data.toHex().constData()));
}

void MainWindow::hardwareReplySilent(int address, int command)
{
    Q_UNUSED(address)
    ui->labelErrorResult->setText(QString(tr("Завершено без ответа 0x%1")).arg(command, 2, 16, QLatin1Char('0')));
}

void MainWindow::hardwareTicketSuccess()
{
    ui->labelErrorResult->setText(QString(tr("Успех")));
}

void MainWindow::hardwareTicketUnknown()
{
    ui->labelErrorResult->setText(QString(tr("Неизвестная команда")));
}

void MainWindow::hardwareTimeout(int address, int command, const QByteArray& data)
{
    Q_UNUSED(data)
    ui->labelErrorResult->setText(QString(tr("Для команды 0x%2 на адрес 0x%1 превышено время ожидания ответа"))
                                  .arg(address, 2, 16, QLatin1Char('0'))
                                  .arg(command, 2, 16, QLatin1Char('0')));
}

static inline void boolListToForm(const QList<bool>& list, QListWidget* widget)
{
    if(!widget)
        return;
    QStringList strings;
    for(int i = 0; i < list.size(); ++i)
        strings.append(QString("%1: %2").arg(i+1).arg(list[i] ? "ВКЛ" : "выкл"));
    widget->clear();
    widget->insertItems(0, strings);
}

static inline void intListToForm(const QList<int>& list, QListWidget* widget)
{
    if(!widget)
        return;
    QStringList strings;
    for(int i = 0; i < list.size(); ++i)
        strings.append(QString("%1: %2").arg(i+1).arg(list[i]));
    widget->clear();
    widget->insertItems(0, strings);
}

void MainWindow::hardwareHello()
{
    ui->labelErrorResult->setText(QString(tr("Успех")));
}

void MainWindow::hardwareKeys(QList<bool> keys)
{
    ui->labelErrorResult->setText(QString(tr("Успех")));
    boolListToForm(keys, ui->listKeys);
}

void MainWindow::hardwareSliders(QList<int> sliders)
{
    ui->labelErrorResult->setText(QString(tr("Успех")));
    intListToForm(sliders, ui->listSliders);
}

void MainWindow::hardwareEncoders(QList<int> encoders)
{
    ui->labelErrorResult->setText(QString(tr("Успех")));
    intListToForm(encoders, ui->listEncoders);
}

void MainWindow::hardwareSensors(QList<int> sensors)
{
    ui->labelErrorResult->setText(QString(tr("Успех")));
    intListToForm(sensors, ui->listSensors);

}

void MainWindow::hardwareStikyKeys(QList<bool> stiky)
{
    ui->labelErrorResult->setText(QString(tr("Успех")));
    boolListToForm(stiky, ui->listStikyKeys);
}

void MainWindow::hardwareState(QList<bool> keys,
                               QList<int> sliders,
                               QList<int> encoders,
                               QList<int> sensors,
                               QList<bool> stiky)
{
    ui->labelErrorResult->setText(QString(tr("Успех")));
    boolListToForm(keys, ui->listKeys);
    intListToForm(sliders, ui->listSliders);
    intListToForm(encoders, ui->listEncoders);
    intListToForm(sensors, ui->listSensors);
    boolListToForm(stiky, ui->listStikyKeys);
}

void MainWindow::ledsChanged(const QByteArray& leds)
{
    hardware.requestSetLeds(ui->comboBoxAddress->currentIndex(), leds);
}

void MainWindow::smartLedsChanged(const QByteArray& leds)
{
    if(!ui->checkBoxSmartLedOne->isChecked())
        hardware.requestSetSmartLeds(ui->comboBoxAddress->currentIndex(), leds);
}

void MainWindow::smartLedChanged(int group, int r, int g, int b)
{
    if(ui->checkBoxSmartLedOne->isChecked())
        hardware.requestSmartLed(ui->comboBoxAddress->currentIndex(), group, r, g, b);
}

void MainWindow::relayClicked()
{
    unsigned char relays =
              (ui->checkBoxRelay0->isChecked() ? qrc::Connection::RELAY_0 : qrc::Connection::RELAY_NONE )
            | (ui->checkBoxRelay1->isChecked() ? qrc::Connection::RELAY_1 : qrc::Connection::RELAY_NONE )
            | (ui->checkBoxRelay2->isChecked() ? qrc::Connection::RELAY_2 : qrc::Connection::RELAY_NONE )
            | (ui->checkBoxRelay3->isChecked() ? qrc::Connection::RELAY_3 : qrc::Connection::RELAY_NONE );

    hardware.requestSetRelays(ui->comboBoxAddress->currentIndex(), relays);
}

void MainWindow::rescanAvailablePorts()
{
    ui->comboBoxPort->clear();
    ui->comboBoxPort->insertItems(0, hardware.getPorList());
    on_comboBoxPort_currentIndexChanged(ui->comboBoxPort->currentIndex());
}

void MainWindow::enableConnectControls(bool isConnected)
{
    ui->labelPort->setEnabled(!isConnected);
    ui->comboBoxPort->setEnabled(!isConnected);
    if(isConnected) // только в случае успешного подключения, чтобы не затереть ошибку
        ui->labelErrorResult->setText(tr("Порт подключён"));
}

void MainWindow::on_comboBoxPort_currentIndexChanged(int index)
{
    bool correctIndex((0 <= index) && (index < ui->comboBoxPort->count()));
    ui->checkBoxPortStart->setEnabled(correctIndex);
}

void MainWindow::on_checkBoxPortStart_clicked(bool checked)
{
    ui->checkBoxPortStart->setEnabled(false);
    enableConnectControls(checked);
    if (checked)
    {
        hardware.start(ui->comboBoxPort->currentIndex(), ui->comboBoxBaudRate->currentIndex());
        if (ui->checkBoxTimer->isChecked())
            repeatTimer.start();
    }
    else
    {
        repeatTimer.stop();
        hardware.stop();
    }
}

void MainWindow::on_comboBoxBaudRate_currentIndexChanged(int index)
{
    hardware.requestSetBaudRate(index);
}

void MainWindow::on_pushButtonHello_clicked()
{
    ui->labelErrorResult->setText("");
    hardware.requestHello(ui->comboBoxAddress->currentIndex());
}

void MainWindow::on_pushButtonKeys_clicked()
{
    hardware.requestGetKeys(ui->comboBoxAddress->currentIndex());
}

void MainWindow::on_pushButtonSliders_clicked()
{
    hardware.requestGetSliders(ui->comboBoxAddress->currentIndex());
}

void MainWindow::on_pushButtonEncoders_clicked()
{
    hardware.requestGetEncoders(ui->comboBoxAddress->currentIndex());
}

void MainWindow::on_pushButtonSensors_clicked()
{
    hardware.requestGetSensors(ui->comboBoxAddress->currentIndex());
}

void MainWindow::on_pushButtonStikyKeys_clicked()
{
    hardware.requestGetStikyKeys(ui->comboBoxAddress->currentIndex());
}

void MainWindow::on_pushButtonState_clicked()
{
    hardware.requestGetState(ui->comboBoxAddress->currentIndex());
}

void MainWindow::on_checkBoxTimer_clicked(bool checked)
{
    if (checked && ui->checkBoxPortStart->isChecked())
        repeatTimer.start();
    else
        repeatTimer.stop();
}
