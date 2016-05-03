#include <QByteArray>
#include <QDateTime>

#include <QSerialPort>
#include <QSerialPortInfo>

#include "qrc_connection.hpp"
#include "qrc_protocol.hpp"
#include "qrc_device.hpp"

using namespace qrc;

Protocol::Protocol(QObject *parent)
    : QObject(parent)
{}

void Protocol::requestHello() {}
void Protocol::requestSetLeds() {}
void Protocol::requestSetXLeds() {}

static const int serialBaudrates[8] =
{
    QSerialPort::Baud9600,
    14400,
    QSerialPort::Baud19200,
    28800,
    QSerialPort::Baud38400,
    QSerialPort::Baud57600,
    76800,
    QSerialPort::Baud115200,
};

enum {
    TIMEOUT = 300, // ms
};

struct Connection::Impl
{
    QList<QSerialPortInfo> ports;
    Device serial;
};

Connection::Connection(QObject *parent)
    : QObject(parent)
    , pImpl(new Impl)
{
    connect(&pImpl->serial, SIGNAL(error(QString)),                this, SIGNAL(error(QString)));
    connect(&pImpl->serial, SIGNAL(parse_error(int, QByteArray)),  this, SIGNAL(parseError(int, QByteArray)));
    connect(&pImpl->serial, SIGNAL(reply_silent(int, int)),        this, SIGNAL(replySilent(int, int)));
    connect(&pImpl->serial, SIGNAL(reply(int, int, QByteArray)),   this, SLOT(parseReply(int, int, QByteArray)));
    connect(&pImpl->serial, SIGNAL(timeout(int, int, QByteArray)), this, SIGNAL(timeout(int, int, QByteArray)));
}

Connection::~Connection()
{
    // Necessary to support pImpl destruction
}

QStringList Connection::getPorList()
{
    pImpl->ports = QSerialPortInfo::availablePorts();

    QStringList result;
    for (const auto& info : pImpl->ports)
    {
        result.append(info.portName());
    }
    return result;
}

void Connection::start(int index, int baudrate)
{
    if ((index < 0) || (pImpl->ports.size() <= index))
    {
        emit error(QString(tr("Неверный индекс устройства: %1")).arg(index));
        emit stopped();
    }
    if((baudrate < BAUDRATE_9600) || (BAUDRATE_115200 < baudrate))
    {
        emit error(QString(tr("Неверный индекс скорости %1")).arg(baudrate));
        emit stopped();
    }
    if(pImpl->ports[index].isNull())
    {
        emit error(QString(tr("Порт %1 не установлен")).arg(index));
        emit stopped();
    }
    if(pImpl->ports[index].isBusy())
    {
        emit error(QString(tr("Порт %1 занят")).arg(index));
        emit stopped();
    }

    // Connect to port

    if (pImpl->serial.open(pImpl->ports[index]))
    {
        emit started();
    }
    else
    {
        emit error(QString(tr("Порт %1 не соединяется")).arg(index));
        emit stopped();
    }
}

void Connection::stop()
{
    pImpl->serial.close();
    emit stopped();
}


void Connection::requestSetBaudRate(int baudrate)
{
    if((baudrate < BAUDRATE_9600) || (BAUDRATE_115200 < baudrate))
    {
        emit error(QString(tr("Неверный индекс скорости %1")).arg(baudrate));
    }
}

void Connection::requestHello(int address)
{
    pImpl->serial.request(address, qrc::CMD_HELLO, QByteArray());
}

void Connection::requestSetLeds(int address, const QByteArray& leds)
{
    pImpl->serial.request(address, qrc::CMD_SET_LEDS, leds);
}

void Connection::requestSetSmartLeds(int address, const QByteArray& leds)
{
    pImpl->serial.request(address, qrc::CMD_SET_SMART_LEDS, leds);
}

void Connection::requestSmartLed(int address, int group, int r, int g , int b)
{
    r = qBound(0, r, 0x0FFF);
    g = qBound(0, g, 0x0FFF);
    b = qBound(0, b, 0x0FFF);

    QByteArray data;
    data.append(char(qBound(0, group, 31)))
            .append(char((r>>8) & 0x0F))
            .append(char(r & 0xFF))
            .append(char((g>>8) & 0x0F))
            .append(char(g & 0xFF))
            .append(char((b>>8) & 0x0F))
            .append(char(b & 0xFF));

    pImpl->serial.request(address, qrc::SET_SPECIFIC_SMART_LED, data);
}

void Connection::requestSetRelays(int address, unsigned char relays)
{
    QByteArray data;
    data.append(char(relays));
    pImpl->serial.request(address, qrc::CMD_SET_RELAY, data);
}

void Connection::requestGetKeys(int address)
{
    pImpl->serial.request(address, qrc::CMD_GET_KEYS, QByteArray());
}

void Connection::requestGetSliders(int address)
{
    pImpl->serial.request(address, qrc::CMD_GET_SLIDERS, QByteArray());
}

void Connection::requestGetSensors(int address)
{
    pImpl->serial.request(address, qrc::CMD_GET_SENSORS, QByteArray());
}

void Connection::requestGetEncoders(int address)
{
    pImpl->serial.request(address, qrc::CMD_GET_ENCODERS, QByteArray());
}

void Connection::requestGetStikyKeys(int address)
{
    pImpl->serial.request(address, qrc::CMD_GET_STIKY_KEYS, QByteArray());
}

void Connection::requestGetState(int address)
{
    pImpl->serial.request(address, qrc::CMD_GET_STATE, QByteArray());
}

void Connection::parseReply(int address, int command, const QByteArray& data)
{
    Q_UNUSED(address)
    switch (command)
    {
    case CMD_HELLO:
        emit replyHello();
        break;
    case CMD_SET_BAUDRATE:
        emit replyBaudrate();
        break;

    // Команды установки значений
    case CMD_SET_LEDS:
    case CMD_SET_SMART_LEDS:
    case CMD_SET_TEXT:
    case CMD_SET_RELAY:
    case SET_SPECIFIC_SMART_LEDS_8:
    case SET_SPECIFIC_SMART_LEDS_4:
    case SET_SPECIFIC_SMART_LED:
        emit reply(address, command, data);
        break;
    // Команды получения значений
    case CMD_GET_KEYS:
    {
        emit replyKeys(getKeys(data));
        break;
    }
    case CMD_GET_SLIDERS:
    {
        emit replySliders(getSliders(data));
        break;
    }
    case CMD_GET_ENCODERS:
    {
        emit replyEncoders(getEncoders(data));
        break;
    }
    case CMD_GET_SENSORS:
    {
        emit replySensors(getSensors(data));
        break;
    }
    case CMD_GET_STIKY_KEYS:
    {
        emit replyStikyKeys(getKeys(data));
        break;
    }
    case CMD_GET_STATE:
    {
        emit replyState(
                    getKeys(data.mid(0, 3)),
                    getSliders(data.mid(3, 8)),
                    getEncoders(data.mid(11, 8)),
                    getSensors(data.mid(19, 2)),
                    getKeys(data.mid(21, 3))
                    );
        break;
    }

    // Телеграммы
    case CMD_SUCCESS:
        emit replyTicketSuccess();
        break;
    case CMD_UNKNOWN:
        emit replyTicketUnknown();
        break;
    default:
        emit reply(address, command, data);
        break;
    }
}
