/******************************************************************************
 *
 * Quest Room Control
 *
 * Model for TableView of ordinar LEDs
 *
 * (c) Roman A. Bulygin 2016
 *
 ******************************************************************************/
#ifndef _QRC_LEDMODEL_HPP_
#define _QRC_LEDMODEL_HPP_

#include <QAbstractTableModel>
#include <QVector>

#include "qrc_protocol.hpp"

class QrcLedModel : public QAbstractTableModel
{
Q_OBJECT
    qrc::LedHelper leds;
public:
    QrcLedModel(QObject * parent = 0);
    ~QrcLedModel();

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    int columnCount(const QModelIndex & parent = QModelIndex()) const;
    int rowCount(const QModelIndex & parent = QModelIndex()) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex & idx, const QVariant & value, int role);

signals:
    void ledsChanged(const QByteArray& leds);
};

#endif // _QRC_LEDMODEL_HPP_
