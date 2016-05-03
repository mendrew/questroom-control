/******************************************************************************
 *
 * Quest Room Control
 *
 * Connection to hardware
 *
 * (c) Roman A. Bulygin 2016
 *
 ******************************************************************************/

#pragma once

#ifndef _QRC_CONNECTION_HPP_
#define _QRC_CONNECTION_HPP_

#include <QList>
#include <QObject>
#include <QScopedPointer>
#include <QStringList>

#include "qrc_protocol.hpp"

namespace qrc {

class Protocol :public QObject
{
    Q_OBJECT
    int address {1};
public:
    explicit Protocol(QObject *parent = 0);
    void setAddress(int value) { address = value & 0xF; }
    int getAddress() const { return address; }
signals:
    void request(int address, int command, const QByteArray& data);
public slots:
    void requestHello();
    void requestSetLeds();
    void requestSetXLeds();
};

class Connection : public QObject
{
    Q_OBJECT

    struct Impl;
    QScopedPointer<Impl> pImpl;
public:
    explicit Connection(QObject *parent = 0);
    virtual ~Connection() override;
public:
    QStringList getPorList(); // return list of available ports

    enum Relay // Константы для установки релюх
    {
        RELAY_NONE = 0x00,
        RELAY_0    = 0x10,
        RELAY_1    = 0x20,
        RELAY_2    = 0x40,
        RELAY_3    = 0x80,
    };

signals:
    void error(const QString& message);
    void parseError(int error, const QByteArray& data); // ошибка разбора
    void replySilent(int address, int command); // типа ответ от команд которые не возвращают ответа (при посылке на адреса 0x00, 0x0F)
    void replyTicketSuccess();
    void replyTicketUnknown();
    void reply(int address, int command, const QByteArray& data);
    void timeout(int address, int command, const QByteArray& data); // ответа не дождались

    void started();
    void stopped();
//    void replySetBaudRate(int address);
    // Ответы
    void replyHello();
    void replyBaudrate();


    void replyKeys(QList<bool>);
    void replySliders(QList<int>);
    void replySensors(QList<int>);
    void replyEncoders(QList<int>);
    void replyStikyKeys(QList<bool>);
    void replyState(QList<bool> keys,
                    QList<int> sliders,
                    QList<int> encoders,
                    QList<int> sensors,
                    QList<bool> stiky);
public slots:
    void start(int index, int baudrate);
    void stop();

    void requestSetBaudRate(int baudrate);

    void requestHello(int address);
    void requestSetLeds(int address, const QByteArray& leds);
    void requestSetSmartLeds(int address, const QByteArray& leds);

    void requestSmartLed(int address, int group, int r, int g , int b);

    void requestSetRelays(int address, unsigned char relays);

    // Получение значений
    void requestGetKeys(int address);
    void requestGetSliders(int address);
    void requestGetSensors(int address);
    void requestGetEncoders(int address);
    void requestGetStikyKeys(int address);
    void requestGetState(int address);

private slots:
    void parseReply(int address, int command, const QByteArray& data);

};

} // namespace qrc

#endif // _QRC_CONNECTION_HPP_
