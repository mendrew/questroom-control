/******************************************************************************
 *
 * Quest Room Control
 *
 * Model for TableView of smart LEDs
 *
 * (c) Roman A. Bulygin 2016
 *
 ******************************************************************************/
#ifndef _QRC_SMARTLEDMODEL_HPP_
#define _QRC_SMARTLEDMODEL_HPP_

#include <QAbstractTableModel>
#include <QVector>
#include "qrc_protocol.hpp"

class QrcSmartLedModel : public QAbstractTableModel
{
Q_OBJECT
    qrc::XLedHelper leds;
public:
    QrcSmartLedModel(QObject * parent = 0);
    ~QrcSmartLedModel();

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    int columnCount(const QModelIndex & parent = QModelIndex()) const;
    int rowCount(const QModelIndex & parent = QModelIndex()) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex & idx, const QVariant & value, int role);

signals:
    void ledsChanged(const QByteArray& leds);
    void ledChanged(int group, int r, int g, int b);
};

#endif // _QRC_SMARTLEDMODEL_HPP_
