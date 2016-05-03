/******************************************************************************
 *
 * Quest Room Control
 *
 * Model for TableView of ordinar LEDs
 *
 * (c) Roman A. Bulygin 2016
 *
 ******************************************************************************/
#include "qrc_smartledmodel.hpp"

#include <QBrush>
#include <QColor>
#include <QSize>

enum {
    LEDS_TOTAL = qrc::QRC_XLED_COUNT,
    LEDS_PER_GROUP = 3,
    LEDS_GROUP_COLUMNS = 4,
    LEDS_GROUPS_PER_ROW = 4,
    LEDS_PER_ROW = LEDS_GROUPS_PER_ROW * LEDS_PER_GROUP,
    LEDS_GROUPS_TOTAL = (LEDS_TOTAL+LEDS_PER_GROUP-1)/LEDS_PER_GROUP, // С округлением вверх
    LEDS_COLUMNS = LEDS_GROUPS_PER_ROW*LEDS_GROUP_COLUMNS,
    LEDS_ROWS = (LEDS_TOTAL+LEDS_PER_ROW-1)/LEDS_PER_ROW, // С округлением вверх
    LEDS_BYTES = (LEDS_TOTAL*12+7)/8,
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

QrcSmartLedModel::QrcSmartLedModel(QObject * parent)
    : QAbstractTableModel(parent)
    , leds(LEDS_TOTAL)
{}

QrcSmartLedModel::~QrcSmartLedModel()
{}

QVariant
QrcSmartLedModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal)
        return QVariant();
    switch(role)
    {
    case Qt::TextAlignmentRole:
        return isLedNameColumn(section) ? Qt::AlignLeft : Qt::AlignCenter;
    case Qt::SizeHintRole:
        return !isLedNameColumn(section) ? QSize(50, -1) : QVariant();
    case Qt::DisplayRole:
    {
        static const QString colors[LEDS_GROUP_COLUMNS] = {tr("LED"), tr("R"), tr("G"), tr("B")};
        return colors[section%LEDS_GROUP_COLUMNS];
    }
    default:
        return QVariant();
    }
}

int
QrcSmartLedModel::columnCount(const QModelIndex & /*parent*/) const
{
    return LEDS_COLUMNS;
}

int
QrcSmartLedModel::rowCount(const QModelIndex & /*parent*/) const
{
    return LEDS_ROWS;
}

Qt::ItemFlags
QrcSmartLedModel::flags(const QModelIndex & index) const
{
    if(ledIndex(index) < LEDS_TOTAL)
        return isLedNameColumn(index) ? Qt::ItemIsEnabled : (Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
    else
        return Qt::NoItemFlags;
}

QVariant
QrcSmartLedModel::data(const QModelIndex & index, int role) const
{
    if (!index.isValid() || index.row()    >= rowCount() ||
            index.column() >= columnCount())
        return QVariant();

    switch(role)
    {
    case Qt::DisplayRole:
    {
        if (isLedNameColumn(index) && (groupIndex(index) <= LEDS_TOTAL / LEDS_PER_GROUP ))
            return groupName(index);

        int ledIdx = ledIndex(index);
        if(ledIdx < LEDS_TOTAL)
            return QString::number(leds.get(ledIdx));
        else
            return QVariant();
    }
    case Qt::TextAlignmentRole:
        return isLedNameColumn(index) ? Qt::AlignRight : Qt::AlignLeft ;
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
    default:
        return QVariant();
    }
    return QVariant();
}

bool
QrcSmartLedModel::setData(const QModelIndex & idx, const QVariant & value, int role)
{
    Q_UNUSED(value)

    if (!idx.isValid() || idx.row()    >= rowCount() ||
            idx.column() >= columnCount())
        return false;

    if (role !=  Qt::EditRole)
        return false;

    int ledIdx = ledIndex(idx);
    if(ledIdx < LEDS_TOTAL)
    {
        bool ok;
        int intVal = qBound(0, value.toInt(&ok), 0xFFF);
        if(ok)
            leds.set(ledIdx, intVal);
        emit dataChanged(index(idx.row(), idx.column()), index(idx.row(), idx.column()));
        // Все
        emit ledsChanged(leds.data());
        // Отдельная группа
        int group = ledIdx / LEDS_PER_GROUP;
        emit ledChanged(group, leds.get(group*LEDS_PER_GROUP), leds.get(group*LEDS_PER_GROUP+1), leds.get(group*LEDS_PER_GROUP+2));
    }
    return true;
}
