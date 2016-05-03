/******************************************************************************
 *
 * Quest Room Control
 *
 * Model for TableView of ordinar LEDs
 *
 * (c) Roman A. Bulygin 2016
 *
 ******************************************************************************/
#include "qrc_ledmodel.hpp"

#include <QBrush>
#include <QColor>

enum {
    LEDS_TOTAL = qrc::QRC_LED_COUNT,
    LEDS_PER_GROUP = 3,
    LEDS_GROUP_COLUMNS = 4,
    LEDS_GROUPS_PER_ROW = 5,
    LEDS_PER_ROW = LEDS_GROUPS_PER_ROW * LEDS_PER_GROUP,
    LEDS_GROUPS_TOTAL = (LEDS_TOTAL+LEDS_PER_GROUP-1)/LEDS_PER_GROUP, // С округлением вверх
    LEDS_COLUMNS = LEDS_GROUPS_PER_ROW*LEDS_GROUP_COLUMNS,
    LEDS_ROWS = (LEDS_TOTAL+LEDS_PER_ROW-1)/LEDS_PER_ROW, // С округлением вверх
    LEDS_BYTES = (LEDS_TOTAL+7)/8, // С округлением вверх
};

enum {
    COLOR_1 = 255,
    COLOR_0 = 230,
};

static inline int groupIndex(const QModelIndex & index)
{
    return LEDS_GROUPS_PER_ROW * index.row() + index.column()/LEDS_GROUP_COLUMNS;
}

static inline int ledIndex(const QModelIndex & index)
{
    return groupIndex(index)*LEDS_PER_GROUP + index.column()%LEDS_GROUP_COLUMNS-1;
}

static inline bool isLedNameColumn(int column)
{
    return (column % LEDS_GROUP_COLUMNS == 0);
}

static inline bool isLedNameColumn(const QModelIndex & index)
{
    return isLedNameColumn(index.column());
}

static inline QString groupName(const QModelIndex & index)
{
    return QString::number(groupIndex(index)+1);
}

QrcLedModel::QrcLedModel(QObject * parent)
    : QAbstractTableModel(parent)
    , leds(LEDS_TOTAL)
{}

QrcLedModel::~QrcLedModel()
{}

QVariant
QrcLedModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal)
    {
        static const QString colors[LEDS_GROUP_COLUMNS] = {tr("LED"), tr("R"), tr("G"), tr("B")};
        return colors[section%LEDS_GROUP_COLUMNS];
    }
    return QVariant();
}

int
QrcLedModel::columnCount(const QModelIndex & /*parent*/) const
{
    return LEDS_COLUMNS;
}

int
QrcLedModel::rowCount(const QModelIndex & /*parent*/) const
{
    return LEDS_ROWS;
}

Qt::ItemFlags
QrcLedModel::flags(const QModelIndex & index) const
{
    if(ledIndex(index) < LEDS_TOTAL)
        return isLedNameColumn(index) ? Qt::ItemIsEnabled : (Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable);
    else
        return Qt::NoItemFlags;
}

QVariant
QrcLedModel::data(const QModelIndex & index, int role) const
{
    if (!index.isValid() || index.row()    >= rowCount() ||
            index.column() >= columnCount())
        return QVariant();

    switch(role)
    {
    case Qt:: DisplayRole:
        return isLedNameColumn(index) && (groupIndex(index) <= LEDS_TOTAL / LEDS_PER_GROUP ) ? groupName(index) : QVariant() ;
    case Qt::TextAlignmentRole:
        return isLedNameColumn(index) ? Qt::AlignRight : QVariant() ;
    case Qt::BackgroundRole:
    {
        if(isLedNameColumn(index))
            return QVariant();
        int ledIdx = ledIndex(index);
        if (ledIdx < LEDS_TOTAL)
        {
            switch (ledIdx % LEDS_PER_GROUP)
            {
            case 0: return QBrush(QColor(COLOR_1, COLOR_0, COLOR_0));
            case 1: return QBrush(QColor(COLOR_0, COLOR_1, COLOR_0));
            case 2: return QBrush(QColor(COLOR_0, COLOR_0, COLOR_1));
            default:
                return QVariant();
            }
        }
        return QVariant();
    }
    case Qt::CheckStateRole:
    {
        int ledIdx = ledIndex(index);
        return (!isLedNameColumn(index)) && (ledIdx < LEDS_TOTAL)
                ? (leds.get(ledIdx) ? Qt::Checked : Qt::Unchecked)
                : QVariant();
    }
    default:
        return QVariant();
    }
    return QVariant();
}

bool
QrcLedModel::setData(const QModelIndex & idx, const QVariant & value, int role)
{
    Q_UNUSED(value)

    if (!idx.isValid() || idx.row()    >= rowCount() ||
            idx.column() >= columnCount())
        return false;

    if (role !=  Qt::CheckStateRole)
        return false;

    int ledIdx = ledIndex(idx);
    if(ledIdx < LEDS_TOTAL)
    {
        leds.set(ledIdx, value.toBool());
        emit dataChanged(index(idx.row(), idx.column()), index(idx.row(), idx.column()));
        emit ledsChanged(leds.data());
    }
    return true;
}
