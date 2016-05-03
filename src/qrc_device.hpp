/******************************************************************************
 *
 * Quest Room Control
 *
 * QT compatible fronted to send commands
 *
 * (c) Roman A. Bulygin 2016
 *
 ******************************************************************************/
#ifndef DEVICE_H
#define DEVICE_H

// QSerialPort should be installed additionaly, as it decribed here:
// http://wiki.qt.io/QtSerialPort

#include <QSerialPort>
#include <QSerialPortInfo>
#include <QObject>
#include <QScopedPointer>

class SerialWorker : public QObject
{
    Q_OBJECT

    QSerialPortInfo info;

    QScopedPointer<QSerialPort> serial;
public:
    SerialWorker(const QSerialPortInfo& info, QObject *parent = 0);
    ~SerialWorker();

signals:
    void error(const QString& message); // ошибка
    void parse_error(int error, const QByteArray& data); // ошибка разбора
    void reply_silent(int address, int command); // типа ответ от команд которые не возвращают ответа (при посылке на адреса 0x00, 0x0F)
    void reply(int address, int command, const QByteArray& data); // ответ на команду
    void timeout(int address, int command, const QByteArray& data); // ответа не дождались

public slots:
    void request(int address, int command, const QByteArray& data);
};

class Device : public QObject
{
    Q_OBJECT

    struct Impl;
    QScopedPointer<Impl> pImpl;
public:
    explicit Device(QObject *parent = 0);
    ~Device();

    bool open(const QSerialPortInfo& info);
    void close();
signals:
    void error(const QString& message);
    void parse_error(int error, const QByteArray& data); // ошибка разбора
    void reply_silent(int address, int command); // типа ответ от команд которые не возвращают ответа (при посылке на адреса 0x00, 0x0F)
    void reply(int address, int command, const QByteArray& data);
    void timeout(int address, int command, const QByteArray& data); // ответа не дождались

    void requestWorker(int address, int command, const QByteArray& data);
public slots:
    void request(int address, int command, const QByteArray& data);
};

#endif // DEVICE_H
