/******************************************************************************
 *
 * Quest Room Control
 *
 * Exchange protocol (please see protocol.doc for the refereces)
 *
 * (c) Roman A. Bulygin 2016
 *
 ******************************************************************************/

#pragma once

#ifndef _QRC_PROTOCOL_HPP_
#define _QRC_PROTOCOL_HPP_

#include <QByteArray>
#include <QList>

namespace qrc {

QByteArray request(unsigned char address, unsigned char command, const QByteArray& data);

enum ost_parse_result {
    PARSE_NONE,       // nothing extracted
    PARSE_SUCCESS,    // packet extracted successfully
    PARSE_SKIPPED,    // not a packet. data - skipped bytes
    PARSE_SIZE_ERROR, // wrong size packet (even bytes in packet) data - wrong packet
    PARSE_TAG_ERROR,  // some tag skipped. data - wrong packet
    PARSE_CRC_ERROR,  // wrong crc in the packet. data - wrong packet
};

ost_parse_result parse(
    QByteArray& buffer,     // in-out buffer with stream data
    unsigned char& address, // address of packet
    unsigned char& command, // command of packet
    QByteArray& data        // payload of packet (or special meaning on error)
);

enum {
    // Общие команды
    CMD_HELLO = 0x00,  // Пинг. Проверка связи.
    CMD_SET_BAUDRATE = 0x01, // Установить скорость обмена.
    // Команды установки значений
    CMD_SET_LEDS = 0x10, // Установить значение простых светодиодов (горим – не горим).
    CMD_SET_SMART_LEDS = 0x11, //Установить значение умных светодиодов (см описание).
    CMD_SET_TEXT = 0x12, // Установить значения для отображения на ЖКИ
    CMD_SET_RELAY = 0x13, // Установить значение реле:
    SET_SPECIFIC_SMART_LEDS_8 = 0x14, // Установить выборочное значение 8ми умных светодиодов. В данных 1н байт номер драйвера (0-3) и 36 байт значения светодиодов по 12 бит на канал светодиода. При этом номер драйвера 0 управляет  LED8 — LED1, номер 1 – LED16 — LED9...
    SET_SPECIFIC_SMART_LEDS_4 = 0x15, // Установить выборочное значение 4х умных светодиодов. В данных 1н байт номер ПОЛОВИНЫ драйвера (0-7) и 18 байт значения светодиодов по 12 бит на канал светодиода. При этом номер ПОЛОВИНЫ драйвера 0 управляет  LED1 — LED4, номер 1 - LED5 — LED8...
    SET_SPECIFIC_SMART_LED = 0x16, // Установить значение 1го умного светодиода. В данных номер светодиода (0 — 31). 0 соответствует подписанному на плате 1му светодиоду. Далее 2 байта яркости на R канал, 2а байта на G, 2а байта на B. Из 2х байт используется только младшие 12 бит.
    // Команды получения значений
    CMD_GET_KEYS = 0x20, // Получить значения кнопок. В ответ 3 байта, используемые биты 0 – 17. Бит 0 всегда 0.
    CMD_GET_SLIDERS = 0x21, // Получить значения слайдеров и крутилок резистивных (каналы АЦП). В ответ 8 байт. 0 байт соответствует 0 каналу АЦП и т.д.
    CMD_GET_ENCODERS = 0x22, // Получить значение энкодеров. В ответ 8 байт (по 2а байта на крутилку). Значения LE. 0 счетчик (16 бит) соответствует 0 крутилке.
    CMD_GET_SENSORS = 0x23, // Получить значения сенсорных кнопок. В ответ 2а байта – попугаи сенсорной кнопки 1 и попугаи сенсорной кнопки 2.
    CMD_GET_STIKY_KEYS = 0x24, // Получить значения «залипших» кнопок. 3 байта.
    CMD_GET_STATE = 0x2F, //Получить все состояния одним пакетом. В ответ: 3и байта — состояния кнопок, 8 байт — АЦП, 8 байт — энкодеры, 2а байта — сенсорные кнопки, 3и байта — залипшие кнопки (в том порядке, как они в командах выше). Итого 24 байта.
    // Телеграммы
    CMD_SUCCESS = 0x80, // OK – телеграмма от слейва об успешном принятии пакета.
    CMD_UNKNOWN = 0x81, // Неизвестная команда
};

enum { 
    BAUDRATE_9600 = 0,
    BAUDRATE_14400 = 1,
    BAUDRATE_19200 = 2,
    BAUDRATE_28800 = 3,
    BAUDRATE_38400 = 4,
    BAUDRATE_57600 = 5,
    BAUDRATE_76800 = 6,
    BAUDRATE_115200 = 7,
};

enum qrc_led_color {
    LED_RED = 0,
    LED_GREEN = 1,
    LED_BLUE = 2
};

enum
{
    QRC_LED_COUNT = 80,
    QRC_XLED_COUNT = 32*3,
    QRC_RELAY_COUNT = 4,
    QRC_KEY_COUNT = 18,
    QRC_SLIDER_COUNT = 8,
    QRC_ENCODER_COUNT = 4,
    QRC_SENSOR_COUNT = 2,
    QRC_STIKY_COUNT = QRC_KEY_COUNT,
};

// Упаковка данных

// Распаковка значений из данных

// Из 3х байтного (или менее) массива получает 18 значений кнопок или умных кнопок
QList<bool> getKeys(const QByteArray& data);

// Из 8и байного (или менее) массива получает 8 значений резистивных элементов
QList<int> getSliders(const QByteArray& data);

// Из 8и байного (или менее) массива получает 4 значения энкодеров
QList<int> getEncoders(const QByteArray& data);

// Из 2х байного (или менее) массива получает 2 значения сенсоров
QList<int> getSensors(const QByteArray& data);

class LedHelper
{
    QByteArray mLeds;
    int mSize {0};
public:
    explicit LedHelper(int leds = QRC_LED_COUNT);
    int size() const;
    bool get(int index) const;
    void set(int index, bool value);
    const QByteArray& data() const;
};

class XLedHelper
{
    QByteArray mLeds;
    int mSize;
public:
    explicit XLedHelper(int leds = QRC_XLED_COUNT);
    int size() const;
    int get(int index) const;
    void set(int index, int value);
    const QByteArray& data() const;
};

/*
// Команды посложнее
static inline
qrc_byte_array requestHello(qrc_byte address)
{
    return request(address, CMD_HELLO, qrc_byte_array());
}

static inline
qrc_byte_array requestSetBaudRate(qrc_byte address, qrc_byte baudrate)
{
    qrc_byte_array data;
    data.push_back(baudrate);
    return request(address, CMD_SET_BAUDRATE, data);
}

static inline
qrc_byte_array requestSetLeds(qrc_byte address, const qrc_led_state& led_state)
{
    return request(address, CMD_SET_LEDS, led_state.data());
}

static inline
qrc_byte_array requestSetSmartLeds(qrc_byte address, const qrc_xled_state& led_state)
{
    return request(address, CMD_SET_SMART_LEDS, led_state.data());
}

static inline
qrc_byte_array requestSetText(qrc_byte address, const qrc_byte_array& text)
{
    return request(address, CMD_SET_TEXT, text);
}

struct qrc_relay
{
    qrc_relay()
    {
        state.resize(1);
    }

    inline const qrc_byte_array& data() const { return state; }

    void set(int index, bool value)
    {
        if ((0 <= index ) && (index < QRC_RELAY_COUNT))
        {
            qrc_byte mask = 0x10 << index;
            if(value)
            {
                state[0] = state[0] | mask;
            }
            else
            {
                state[0] = (state[0] & (~mask));
            }
        }
    }

    bool get(int index) const
    {
        if ((0 <= index ) && (index < QRC_RELAY_COUNT))
        {
            qrc_byte mask = 0x10 << index;
            return (state[0] & mask) == mask;
        }
        return false;
    }

private:
    qrc_byte_array state;
};

static inline
qrc_byte_array requestSetRelay(qrc_byte address, const qrc_byte_array& text)
{
    return request(address, CMD_SET_TEXT, text);
}


static inline
qrc_byte_array requestGetState(qrc_byte address)
{
    return request(address, CMD_GET_STATE, qrc_byte_array());
}

//

//qrc_byte_array request()

struct qrc_buttons
{
    qrc_buttons(const qrc_byte_array& data)
        : data(data)
    {
//        if(data.size() != 3)
//            data.resize(3, 0);
    }
    bool button(int index) const
    {
        qrc_byte mask = index % 8;
        return (0 <= index) && (index < QCR_BUTTONS_COUNT) ? (data[index /3] & mask) == mask  : false;
    }
private:
    qrc_byte_array data;

};

struct qrc_encoders
{

};
*/
} // namespace qrc

#endif // _QRC_PROTOCOL_HPP_

