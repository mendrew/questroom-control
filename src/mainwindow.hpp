#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <QTimer>

#include "qrc_connection.hpp"
#include "qrc_ledmodel.hpp"
#include "qrc_smartledmodel.hpp"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    qrc::Connection hardware;
    QTimer repeatTimer;
    QrcLedModel ledModel;
    QrcSmartLedModel smartLedModel;

    void rescanAvailablePorts();
    void enableConnectControls(bool isConnected);

private slots:
    void hardwareStarted();
    void hardwareStopped();
    void hardwareError(const QString& message);
    void hardwareParseError(int error, const QByteArray& data);
    void hardwareReplySilent(int address, int command);
    void hardwareTicketSuccess();
    void hardwareTicketUnknown();
    void hardwareTimeout(int address, int command, const QByteArray& data);

    // Ответы на запрос состояния платы
    void hardwareHello();
    void hardwareKeys(QList<bool> keys);
    void hardwareSliders(QList<int> sliders);
    void hardwareEncoders(QList<int> encoders);
    void hardwareSensors(QList<int> sensors);
    void hardwareStikyKeys(QList<bool> stiky);
    void hardwareState(QList<bool> keys,
                       QList<int> sliders,
                       QList<int> encoders,
                       QList<int> sensors,
                       QList<bool> stiky);
    // реакция на диоды
    void ledsChanged(const QByteArray& leds);
    void smartLedsChanged(const QByteArray& leds);
    void smartLedChanged(int group, int r, int g, int b);
    void relayClicked();

private slots:
    void on_comboBoxPort_currentIndexChanged(int index);
    void on_checkBoxPortStart_clicked(bool checked);
    void on_comboBoxBaudRate_currentIndexChanged(int index);

    void on_pushButtonHello_clicked();
    void on_pushButtonKeys_clicked();
    void on_pushButtonSliders_clicked();
    void on_pushButtonEncoders_clicked();
    void on_pushButtonSensors_clicked();
    void on_pushButtonStikyKeys_clicked();
    void on_pushButtonState_clicked();
    void on_checkBoxTimer_clicked(bool checked);
};

#endif // MAINWINDOW_HPP
