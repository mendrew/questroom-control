#include "qrc_protocol.hpp"

#include <QtEndian>

namespace qrc {

enum {
    START_MASK = 0x80,
    START_PACKET = 0x80, // Признак начала посылки
    ADDRESS_MASK = 0x0F, // Маска адреса
    PAYLOAD_KIND_MASK = 0x40,
    PAYLOAD_KIND_HI = 0x40, // Признак старшей тетерады
    PAYLOAD_KIND_LO = 0x00, // Признак младшей тетрады как отсутствие признака старшей
    TYPE_MASK = 0x30, // Маска типа
    TYPE_CMD = 0x00, // Это команда
    TYPE_DATA = 0x10, // Это байт данных
    TYPE_CRC = 0x20, // Это CRC
    TYPE_RESERVED = 0x30, // Неиспользуется
    PAYLOAD_MASK = 0xF,
    TAG_MASK = START_MASK | PAYLOAD_KIND_MASK | TYPE_MASK,
    //--
    MINIMAL_PACKET_SIZE = 5,

};

static inline QByteArray encode_byte(unsigned char type, unsigned char data)
{
    return QByteArray()
            .append(char(PAYLOAD_KIND_HI | type | ((data >> 4) & PAYLOAD_MASK)))
            .append(char(PAYLOAD_KIND_LO | type | (data & PAYLOAD_MASK)));
}

QByteArray request(unsigned char address, unsigned char command, const QByteArray& data)
{
    QByteArray result;
    result.reserve(data.size()*2+5);
    // Сначала признак начала команды и адрес
    result.append(char(START_PACKET | (address & ADDRESS_MASK)));
    // Код команды
    result.append(encode_byte(TYPE_CMD, command));
    // Байты данных
    for(unsigned char x : data)
    {
        result.append(encode_byte(TYPE_DATA, x));
    }
    // CRC
    unsigned int crc = 0;
    for(unsigned char x : result)
    {
        crc += x;
    }
    crc = crc & 0xFF;
    result.append(encode_byte(TYPE_CRC, crc));

    return result;
}

static inline
bool glue_byte(unsigned char hi_byte, unsigned char lo_byte, unsigned char type, unsigned char& out_data)
{
    if (((hi_byte & TAG_MASK) != (PAYLOAD_KIND_HI | type))
            || ((lo_byte & TAG_MASK) != (PAYLOAD_KIND_LO | type)))
        return false;
    out_data = ((hi_byte & PAYLOAD_MASK) << 4) | (lo_byte & PAYLOAD_MASK);
    return true;
}

static inline
ost_parse_result parse_packet(
        const QByteArray& packet, // in packet to parse
        unsigned char& address,   // address of packet
        unsigned char& command,   // command of packet
        QByteArray& data // payload of packet (or special meaning on error)
)
{
    // check size
    if((packet.size() < MINIMAL_PACKET_SIZE) || ( (packet.size() % 2) == 0))
    {
        data = packet;
        return PARSE_SIZE_ERROR;
    }
    address = packet[0] & ADDRESS_MASK;
    // check crc
    unsigned int expected_crc = 0;
    for(int i = 0; i < packet.size() - 2; ++i)
        expected_crc += (unsigned char)(packet[i]);
    expected_crc = expected_crc & 0xFF;

    unsigned char actual_crc;

    if (!glue_byte(packet[packet.size()-2], packet[packet.size()-1], TYPE_CRC, actual_crc))
    {
        data = packet;
        return PARSE_TAG_ERROR;
    }
    // extract command
    if (!glue_byte(packet[1], packet[2], TYPE_CMD, command))
    {
        data = packet;
        return PARSE_TAG_ERROR;
    }

    data.clear();
    for(int i = 3; i < packet.size()-2; i += 2)
    {
        unsigned char data_byte;
        if (!glue_byte(packet[i], packet[i+1], TYPE_DATA, data_byte))
        {
            data = packet;
            return PARSE_TAG_ERROR;
        }
        data.push_back(data_byte);
    }
    return PARSE_SUCCESS;
}

ost_parse_result parse(
    QByteArray& buffer,     // in-out buffer with stream data
    unsigned char& address, // address of packet
    unsigned char& command, // command of packet
    QByteArray& data        // payload of packet (or special meaning on error)
)
{
    // skip all before packet start
    int pos = 0;
    while ((pos < buffer.size()) && ((buffer[pos] & TAG_MASK) != START_PACKET))
        ++pos;

    if (pos > 0) // we was moved
    {
        if(pos < buffer.size())
        {
        data = buffer.mid(0, pos);
        buffer = buffer.mid(pos);
        }
        else
        {
            data = buffer;
            buffer.clear();
        }
        return PARSE_SKIPPED;
    }
    auto end_pos = pos;
    while((end_pos < buffer.size()) && ((buffer[end_pos] & TAG_MASK) != (PAYLOAD_KIND_LO | TYPE_CRC)))
        ++end_pos;
    if (end_pos == buffer.size())
        return PARSE_NONE; // Need more data
    ++end_pos;
    QByteArray packet = buffer.mid(pos, end_pos-pos);
    if(end_pos == buffer.size())
        buffer.clear();
    else
        buffer = buffer.mid(end_pos);

    return parse_packet(packet, address, command, data);
}

QList<bool> getKeys(const QByteArray& data)
{
    QList<bool> keys;
    for(unsigned char x : data)
    {
        for (int i = 0; (i < 8) && (keys.size() < QRC_KEY_COUNT); ++i)
        {
            unsigned char p = 1 << i;
            keys.append((x & p) == p);
        }
    }
    while(keys.size() < QRC_KEY_COUNT)
        keys.append(false);
    return keys;
}

QList<int> getSliders(const QByteArray& data)
{
    QList<int> sliders;
    for(unsigned char x : data)
        sliders.append(x);
    while (sliders.size() < QRC_SLIDER_COUNT)
        sliders.append(0);
    return sliders;
}

QList<int> getEncoders(const QByteArray& data)
{
    QList<int> encoders;
    for(int i = 0; i < 4; ++i)
    {
        quint16* p = ((quint16*)data.constData()) + i;

        encoders.append(qFromLittleEndian<quint16>(*p));
    }
    while (encoders.size() < QRC_ENCODER_COUNT)
        encoders.append(0);
    return encoders;
}

QList<int> getSensors(const QByteArray& data)
{
    QList<int> sensors;
    for(unsigned char x : data)
        sensors.append(x);
    while (sensors.size() < QRC_SENSOR_COUNT)
        sensors.append(0);
    return sensors;
}

/******************************************************************************
 * LedHelper
 ******************************************************************************/

LedHelper::LedHelper(int leds)
    : mLeds((leds+7)%8, 0)
    , mSize(leds)
{}

int LedHelper::size() const
{
    return mSize;
}

bool LedHelper::get(int index) const
{
    if ((index < 0) || (mSize <= index))
        return false;
    unsigned char mask = 1 << (index % 8);
    return (mLeds[index/8] & mask) ==  mask;
}

void LedHelper::set(int index, bool value)
{
    if ((index < 0) || (mSize <= index))
        return;

    unsigned char mask = 1 << (index % 8);
    mLeds[index/8] = value ? (mLeds[index/8] | mask) : (mLeds[index/8] & (~mask)) ;
}

const QByteArray& LedHelper::data() const
{
    return mLeds;
}

/******************************************************************************
 * LedHelper
 ******************************************************************************/

XLedHelper::XLedHelper(int leds)
    : mLeds((leds*3+1)/2, 0)
    , mSize(leds)
{}

int XLedHelper::size() const
{
    return mSize;
}

int XLedHelper::get(int index) const
{
    int pos = mSize-index-1;
    int octet = pos*3;
    int byte = octet/2;
    int shift = octet%2;
    if ((byte < 0) || (mLeds.size() <= byte+1))
        return 0;

    int ret = (unsigned char)mLeds[byte];
    ret = (ret << 8) | (unsigned char)mLeds[byte+1];
    return  ((shift == 1) ? ret : (ret >> 4)) & 0xFFF;
}

void XLedHelper::set(int index, int value)
{
    int pos = mSize-index-1;
    int octet = pos*3;
    int byte = octet/2;
    int shift = octet%2;
    if ((byte < 0) || (mLeds.size() <= byte+1))
        return;
    if (shift == 1) // не c начала байта
    {
        mLeds[byte] = (mLeds[byte] & 0xF0) | ((value >> 8) & 0x0F);
        mLeds[byte+1] = value & 0xFF;
    }
    else // с начала байта
    {
        mLeds[byte] = (value >> 4) & 0xFF;
        mLeds[byte+1] = ((value << 4) & 0xF0) | (mLeds[byte+1] & 0x0F);
    }
}

const QByteArray& XLedHelper::data() const
{
    return mLeds;
}

} //namespace qrc
