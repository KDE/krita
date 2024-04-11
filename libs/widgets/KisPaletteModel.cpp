/*
 *  SPDX-FileCopyrightText: 2013 Sven Langkamp <sven.langkamp@gmail.com>
 *  SPDX-FileCopyrightText: 2018 Michael Zhou <simeirxh@gmail.com>
 *  SPDX-FileCopyrightText: 2022 Halla Rempt <halla@valdyas.org>
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisPaletteModel.h"

#include <QBrush>
#include <QDomDocument>
#include <QDomElement>
#include <QMimeData>

#include <KoColor.h>

#include <KoColorSpace.h>
#include <KoColorModelStandardIds.h>
#include <resources/KoColorSet.h>
#include <KoColorDisplayRendererInterface.h>
#include <KisResourceModel.h>
#include <QFileInfo>


KisPaletteModel::KisPaletteModel(QObject* parent)
    : QAbstractTableModel(parent)
    , m_colorSet(0)
    , m_displayRenderer(KoDumbColorDisplayRenderer::instance())
{
    connect(this, SIGNAL(sigPaletteModified()), SLOT(slotPaletteModified()));
}

KisPaletteModel::~KisPaletteModel()
{
}

QVariant KisPaletteModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) { return QVariant(); }
    bool groupNameRow = m_colorSet->isGroupTitleRow(index.row());
    if (role == IsGroupNameRole) {
        return groupNameRow;
    }
    if (groupNameRow) {
        return dataForGroupNameRow(index, role);
    } else {
        return dataForSwatch(index, role);
    }
}

int KisPaletteModel::rowCount(const QModelIndex& /*parent*/) const
{
    if (!m_colorSet) return 0;
    return m_colorSet->rowCountWithTitles();
}

int KisPaletteModel::columnCount(const QModelIndex& /*parent*/) const
{
    if (m_colorSet && m_colorSet->columnCount() > 0) {
        return m_colorSet->columnCount();
    }
    if (!m_colorSet) {
        return 0;
    }
    return 16;
}

Qt::ItemFlags KisPaletteModel::flags(const QModelIndex& index) const
{
    if (index.isValid()) {
        return  Qt::ItemIsSelectable |
                Qt::ItemIsEnabled |
                Qt::ItemIsUserCheckable |
                Qt::ItemIsDragEnabled |
                Qt::ItemIsDropEnabled;
    }
    return Qt::ItemIsDropEnabled;
}

QModelIndex KisPaletteModel::index(int row, int column, const QModelIndex& parent) const
{
    Q_UNUSED(parent);

    KisSwatchGroupSP group = 0;
    if (m_colorSet) {
        group = m_colorSet->getGroup(row);
    }
    else {
        return {};
    }

    if (!group) {
        qDebug() << "no group for row" << row << "col" << column << "total rows in model" << rowCount() << "rows in colorset" << m_colorSet->rowCountWithTitles();
        return QModelIndex();
    }
    //KIS_ASSERT_RECOVER_RETURN_VALUE(group, QModelIndex());
    QModelIndex idx = createIndex(row, column);
    Q_ASSERT(idx.column() < columnCount());
    Q_ASSERT(idx.row() < rowCount());
    return idx;
}

void KisPaletteModel::setColorSet(KoColorSetSP colorSet)
{
    beginResetModel();
    m_colorSet = colorSet;
    if (colorSet) {
        connect(colorSet.data(), SIGNAL(modified()), this, SIGNAL(sigPaletteModified()));
    }
    endResetModel();
    Q_EMIT sigPaletteChanged();
}

KoColorSetSP KisPaletteModel::colorSet() const
{
    return m_colorSet;
}

int KisPaletteModel::rowNumberInGroup(int rowInModel) const
{
    return m_colorSet->rowNumberInGroup(rowInModel);
}


void KisPaletteModel::addSwatch(const KisSwatch &entry, const QString &groupName)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount() + 1);
    m_colorSet->addSwatch(entry, groupName);
    endInsertRows();
}

void KisPaletteModel::removeSwatch(const QModelIndex &index, bool keepColors)
{
    KisSwatchGroupSP group = m_colorSet->getGroup(index.row());
    if (!qvariant_cast<bool>(data(index, IsGroupNameRole))) {
        m_colorSet->removeSwatch(index.column(),
                                 rowNumberInGroup(index.row()),
                                 group);
        Q_EMIT dataChanged(index, index);
    } else {
        int groupNameRow = m_colorSet->startRowForGroup(group->name());
        QString groupName = m_colorSet->getGroup(groupNameRow)->name();
        removeGroup(groupName, keepColors);
    }
}

void KisPaletteModel::removeGroup(const QString &groupName, bool keepColors)
{
    int removeStart = m_colorSet->startRowForGroup(groupName);
    int removedRowCount = m_colorSet->getGroup(groupName)->rowCount();

    beginRemoveRows(QModelIndex(), removeStart, removeStart + removedRowCount);
    m_colorSet->removeGroup(groupName, keepColors);
    endRemoveRows();
}

bool KisPaletteModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
                                   int row, int column, const QModelIndex &parent)
{
    Q_UNUSED(row);
    Q_UNUSED(column);
    if (!data->hasFormat("krita/x-colorsetentry") && !data->hasFormat("krita/x-colorsetgroup")) {
        return false;
    }
    if (action == Qt::IgnoreAction) {
        return false;
    }

    QModelIndex finalIndex = parent;
    if (!finalIndex.isValid()) { return false; }

    if (data->hasFormat("krita/x-colorsetgroup")) {
        // dragging group not supported for now
        QByteArray encodedData = data->data("krita/x-colorsetgroup");
        QDataStream stream(&encodedData, QIODevice::ReadOnly);

        while (!stream.atEnd()) {
            QString groupNameDroppedOn = qvariant_cast<QString>(finalIndex.data(GroupNameRole));
            if (groupNameDroppedOn == KoColorSet::GLOBAL_GROUP_NAME) {
                return false;
            }
            QString groupNameDragged;
            stream >> groupNameDragged;
            KisSwatchGroupSP groupDragged = m_colorSet->getGroup(groupNameDragged);
            int start = m_colorSet->startRowForGroup(groupNameDragged);
            int end = start + groupDragged->rowCount();
            if (!beginMoveRows(QModelIndex(), start, end, QModelIndex(), m_colorSet->startRowForGroup(groupNameDroppedOn))) {
                return false;
            }
            m_colorSet->moveGroup(groupNameDragged, groupNameDroppedOn);
            endMoveRows();
            Q_EMIT sigPaletteModified();
        }
        return true;
    }

    if (qvariant_cast<bool>(finalIndex.data(KisPaletteModel::IsGroupNameRole))) {
        return true;
    }


    if (data->hasFormat("krita/x-colorsetentry")) {
        QByteArray encodedData = data->data("krita/x-colorsetentry");
        QString oldGroupName;
        int oriRow;
        int oriColumn;
        KisSwatch entry = KisSwatch::fromByteArray(encodedData, oldGroupName, oriRow, oriColumn);

        if (action == Qt::MoveAction){
            KisSwatchGroupSP g = m_colorSet->getGroup(oldGroupName);
            if (g) {
                if (qvariant_cast<bool>(finalIndex.data(KisPaletteModel::CheckSlotRole))) {
                    m_colorSet->addSwatch(getSwatch(finalIndex), g->name(), oriColumn, oriRow);
                } else {
                    m_colorSet->removeSwatch(oriColumn, oriRow, g);
                }
            }
            setSwatch(entry, finalIndex);
            Q_EMIT sigPaletteModified();
        }

        return true;
    }

    return false;
}

QMimeData *KisPaletteModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *mimeData = new QMimeData();
    QByteArray encodedData;

    QDataStream stream(&encodedData, QIODevice::WriteOnly);
    QModelIndex index = indexes.last();
    if (index.isValid() && qvariant_cast<bool>(index.data(CheckSlotRole))) {
        QString mimeTypeName = "krita/x-colorsetentry";
        if (qvariant_cast<bool>(index.data(IsGroupNameRole))==false) {
            KisSwatch entry = getSwatch(index);
            QString groupName = qvariant_cast<QString>(index.data(KisPaletteModel::GroupNameRole));
            entry.writeToStream(stream,
                                groupName,
                                rowNumberInGroup(index.row()),
                                index.column());
        } else {
            mimeTypeName = "krita/x-colorsetgroup";
            QString groupName = qvariant_cast<QString>(index.data(GroupNameRole));
            stream << groupName;
        }
        mimeData->setData(mimeTypeName, encodedData);
    }

    return mimeData;
}

QStringList KisPaletteModel::mimeTypes() const
{
    return QStringList() << "krita/x-colorsetentry" << "krita/x-colorsetgroup";
}

Qt::DropActions KisPaletteModel::supportedDropActions() const
{
    return Qt::MoveAction;
}

void KisPaletteModel::setSwatch(const KisSwatch &entry, const QModelIndex &index)
{
    if (m_colorSet->isGroupTitleRow(index.row())) return;

    Q_ASSERT(index.column() < m_colorSet->columnCount());
    Q_ASSERT(index.column() < columnCount());

    KisSwatchGroupSP group = m_colorSet->getGroup(index.row());
    Q_ASSERT(group);

    m_colorSet->addSwatch(entry, group->name(), index.column(), rowNumberInGroup(index.row()));

    Q_EMIT dataChanged(index, index);
}

void KisPaletteModel::changeGroupName(const QString &groupName, const QString &newName)
{
    beginResetModel();
    m_colorSet->changeGroupName(groupName, newName);
    endResetModel();
}

KisSwatchGroupSP KisPaletteModel::addGroup(const QString &groupName, int _columnCount, int _rowCount)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount() + _rowCount);
    m_colorSet->addGroup(groupName, _columnCount, _rowCount);
    endInsertRows();
    return m_colorSet->getGroup(groupName);
}

void KisPaletteModel::setRowCountForGroup(const QString &groupName, int rowCount)
{
    beginResetModel();
    KisSwatchGroupSP g = m_colorSet->getGroup(groupName);
    if (g) {
        g->setRowCount(rowCount);
    }
    endResetModel();
}

void KisPaletteModel::setColumnCount(int colCount)
{
    beginResetModel();
    m_colorSet->setColumnCount(colCount);
    endResetModel();
}

void KisPaletteModel::clear()
{
    beginResetModel();
    m_colorSet->clear();
    endResetModel();
}

void KisPaletteModel::clear(int defaultColumnsCount)
{
    beginResetModel();
    m_colorSet->clear();
    m_colorSet->setColumnCount(defaultColumnsCount);
    endResetModel();
}

QVariant KisPaletteModel::dataForGroupNameRow(const QModelIndex &idx, int role) const
{
    KisSwatchGroupSP group = m_colorSet->getGroup(idx.row());
    Q_ASSERT(group);
    QString groupName = group->name();
    switch (role) {
    case Qt::ToolTipRole:
    case Qt::DisplayRole: {
        return groupName;
    }
    case GroupNameRole: {
        return groupName;
    }
    case CheckSlotRole: {
        return true;
    }
    case RowInGroupRole: {
        return -1;
    }
    default: {
        return QVariant();
    }
    }
}

QVariant KisPaletteModel::dataForSwatch(const QModelIndex &idx, int role) const
{
    KisSwatchGroupSP group = m_colorSet->getGroup(idx.row());
    Q_ASSERT(group);
    int rowInGroup = rowNumberInGroup(idx.row());
    bool entryPresent = group->checkSwatchExists(idx.column(), rowInGroup);
    KisSwatch entry;
    if (entryPresent) {
        entry = group->getSwatch(idx.column(), rowInGroup);
    }
    switch (role) {
    case Qt::ToolTipRole:
    case Qt::DisplayRole: {
        return entryPresent ? entry.name() + "\n(" + KoColor::toQString(entry.color()) + ")" : i18n("Empty slot");
    }
    case Qt::BackgroundRole: {
        QColor color(0, 0, 0, 0);
        if (entryPresent) {
            color = m_displayRenderer->toQColor(entry.color());
        }
        return QBrush(color);
    }
    case GroupNameRole: {
        return group->name();
    }
    case CheckSlotRole: {
        return entryPresent;
    }
    case RowInGroupRole: {
        return rowInGroup;
    }
    default: {
        return QVariant();
    }
    }
}

void KisPaletteModel::setDisplayRenderer(const KoColorDisplayRendererInterface *displayRenderer)
{
    if (displayRenderer) {
        if (m_displayRenderer) {
            disconnect(m_displayRenderer, 0, this, 0);
        }
        m_displayRenderer = displayRenderer;
        connect(m_displayRenderer, SIGNAL(displayConfigurationChanged()),
                SLOT(slotDisplayConfigurationChanged()), Qt::UniqueConnection);
    } else {
        m_displayRenderer = KoDumbColorDisplayRenderer::instance();
    }
}

void KisPaletteModel::slotDisplayConfigurationChanged()
{
    beginResetModel();
    endResetModel();
}

void KisPaletteModel::slotPaletteModified()
{
    /**
     * Until we implement resource->convertToSerializable() we should
     * explicitly convert all the palettes into Krita internal format
     */
    if (m_colorSet->paletteType() != KoColorSet::KPL || m_colorSet->paletteType() != KoColorSet::GPL) {
        m_colorSet->setPaletteType(KoColorSet::KPL);
    }

    if (m_colorSet->paletteType() == KoColorSet::KPL) {
        m_colorSet->setFilename(QFileInfo(m_colorSet->filename()).completeBaseName() + ".kpl");
    }
    else if (m_colorSet->paletteType() == KoColorSet::GPL) {
        m_colorSet->setFilename(QFileInfo(m_colorSet->filename()).completeBaseName() + ".gpl");
    }
}

void KisPaletteModel::slotExternalPaletteModified(QSharedPointer<KoColorSet> resource)
{
    if (resource && resource == m_colorSet) {
        slotPaletteModified();
        slotDisplayConfigurationChanged();
    }
}

QModelIndex KisPaletteModel::indexForClosest(const KoColor &compare)
{
    KisSwatchGroup::SwatchInfo info = colorSet()->getClosestSwatchInfo(compare);
    return createIndex(indexRowForInfo(info), info.column, colorSet()->getGroup(info.group).data());
}

int KisPaletteModel::indexRowForInfo(const KisSwatchGroup::SwatchInfo &info)
{
    int groupRow = m_colorSet->startRowForGroup(info.group);
    if (info.group.isEmpty()) {
        return groupRow + info.row;
    }
    return groupRow + info.row + 1;
}

KisSwatch KisPaletteModel::getSwatch(const QModelIndex &index) const
{
    if (index.row() >= rowCount()) return KisSwatch();
    if (index.row() < 0) return KisSwatch();

    return m_colorSet->getColorGlobal(index.column(), index.row());
}
