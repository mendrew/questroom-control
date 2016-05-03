#include "qrc_device.hpp"

#include "qrc_protocol.hpp"

#include <QDateTime>
#include <QThread>

enum {
    TIMEOUT = 350
};

SerialWorker::SerialWorker(const QSerialPortInfo& info, QObject *parent)
    : QObject(parent)
    , info(info)
{
}

SerialWorker::~SerialWorker()
{}

void SerialWorker::request(int address, int command, const QByteArray& data)
{
    if (serial.isNull())
    {
        if (info.isNull())
        {
            emit error(QString(tr("Нет данных для открытия порта")));
            return;
        }
        if (info.isBusy())
        {
            emit error(QString(tr("Порт %1 занят").arg(info.portName())));
            return;
        }

        serial.reset(new QSerialPort(info));

        serial->setBaudRate(9600);
        serial->setDataBits(QSerialPort::Data8);
        serial->setParity(QSerialPort::NoParity);
        serial->setStopBits(QSerialPort::OneStop);
        serial->setFlowControl(QSerialPort::NoFlowControl);

        if (!serial->open(QIODevice::ReadWrite))
        {
            serial.reset();
            emit error(QString(tr("Не могу открыть порт %1").arg(info.portName())));
            return;
        }
    }

    QByteArray dataToSend = qrc::request(address, command, data);

    qint64 written = serial->write(dataToSend);
    if (written != dataToSend.size())
    {
        emit error(QString(tr("Ошибка записи. Записано %1 байт из %2")).arg(written).arg(dataToSend.size()));
        return;
    }

    if((address == 0) || (address == 15)) // Команды по этим адресам не возвращают ответа
    {
        emit reply_silent(address, command);
        return;
    }

    QByteArray readBuffer;

    qint64 startTime = QDateTime::currentMSecsSinceEpoch();
    do
    {
        if(( QDateTime::currentMSecsSinceEpoch() - startTime) > TIMEOUT) //
        {
            emit timeout(address, command, data);
            return;
        }

        if (!serial->waitForReadyRead(25))
            continue;

        readBuffer.append(serial->readAll());

        unsigned char reply_address;
        unsigned char reply_command;
        QByteArray reply_data;
        switch(int perror = qrc::parse(readBuffer, reply_address, reply_command, reply_data))
        {
        case qrc::PARSE_NONE: // Мало данных

            break;
        case qrc::PARSE_SUCCESS: // Отлично
            emit reply(reply_address, reply_command, reply_data);
            return;
        case qrc::PARSE_SKIPPED: // not a packet. data - skipped bytes
            emit parse_error(perror, reply_data);
            break; // possibly it may be more data
        case qrc::PARSE_SIZE_ERROR: // wrong size packet (even bytes in packet) data - wrong packet
        case qrc::PARSE_TAG_ERROR:  // some tag skipped. data -  wrong packet
        case qrc::PARSE_CRC_ERROR:  // wrong crc in the packet. data -  wrong packet
        default:
            emit parse_error(perror, reply_data);
            return;
        }
    }
    while(1);
}

struct Device::Impl
{
    QThread thread;
};

Device::Device(QObject *parent)
    : QObject(parent)
    , pImpl(new Impl)
{}

Device::~Device()
{
    close();
}

bool Device::open(const QSerialPortInfo& info)
{
    close();

    if (info.isNull() || info.isBusy())
        return false;

    // тут мы запускаем поток, который собственно будет обрабатывать ком-порт
    SerialWorker* worker = new SerialWorker(info);
    worker->moveToThread(&pImpl->thread);
    worker->moveToThread(&pImpl->thread);

    connect(&pImpl->thread, SIGNAL(finished()), worker, SLOT(deleteLater()));

    connect(this, SIGNAL(requestWorker(int, int, QByteArray)), worker, SLOT(request(int, int, QByteArray)));

    connect(worker, SIGNAL(error(QString)),                this, SIGNAL(error(QString)));
    connect(worker, SIGNAL(parse_error(int, QByteArray)),  this, SIGNAL(parse_error(int, QByteArray)));
    connect(worker, SIGNAL(reply_silent(int, int)),        this, SIGNAL(reply_silent(int, int)));
    connect(worker, SIGNAL(reply(int, int, QByteArray)),   this, SIGNAL(reply(int, int, QByteArray)));
    connect(worker, SIGNAL(timeout(int, int, QByteArray)), this, SIGNAL(timeout(int, int, QByteArray)));

    pImpl->thread.start();

    return true;
}

void Device::close()
{
    if(pImpl->thread.isRunning())
    {
        pImpl->thread.quit();
        pImpl->thread.wait();
    }
}

void Device::request(int address, int command, const QByteArray& data)
{
    if(!pImpl->thread.isRunning())
    {
        emit error(QString(tr("Порт не открыт")));
    }
    emit requestWorker(address, command, data);
}
