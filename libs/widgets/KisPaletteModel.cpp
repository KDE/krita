/*
 *  Copyright (c) 2013 Sven Langkamp <sven.langkamp@gmail.com>
 *  Copyright (c) 2018 Michael Zhou <simeirxh@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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

KisPaletteModel::KisPaletteModel(QObject* parent)
    : QAbstractTableModel(parent)
    , m_colorSet(Q_NULLPTR)
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
    bool groupNameRow = m_rowGroupNameMap.contains(index.row());
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
    if (!m_colorSet)
        return 0;
    return m_colorSet->rowCount() // count of color rows
            + m_rowGroupNameMap.size()  // rows for names
            - 1; // global doesn't have a name
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
    Q_ASSERT(m_colorSet);
    int groupNameRow = groupNameRowForRow(row);
    KisSwatchGroup *group = m_colorSet->getGroup(m_rowGroupNameMap[groupNameRow]);
    Q_ASSERT(group);
    return createIndex(row, column, group);
}

void KisPaletteModel::resetGroupNameRows()
{
    m_rowGroupNameMap.clear();
    int row = -1;
    for (const QString &groupName : m_colorSet->getGroupNames()) {
        m_rowGroupNameMap[row] = groupName;
        row += m_colorSet->getGroup(groupName)->rowCount();
        row += 1; // row for group name
    }
}

void KisPaletteModel::setPalette(KoColorSet* palette)
{
    beginResetModel();
    m_colorSet = palette;
    if (palette) {
        resetGroupNameRows();
    }
    endResetModel();
    emit sigPaletteChanged();
}

KoColorSet* KisPaletteModel::colorSet() const
{
    return m_colorSet;
}

int KisPaletteModel::rowNumberInGroup(int rowInModel) const
{
    if (m_rowGroupNameMap.contains(rowInModel)) {
        return -1;
    }
    QList<int> rowNumberList = m_rowGroupNameMap.keys();
    for (auto it = rowNumberList.rbegin(); it != rowNumberList.rend(); it++) {
        if (*it < rowInModel) {
            return rowInModel - *it - 1;
        }
    }
    return rowInModel;
}

int KisPaletteModel::groupNameRowForName(const QString &groupName)
{
    for (auto it = m_rowGroupNameMap.begin(); it != m_rowGroupNameMap.end(); it++) {
        if (it.value() == groupName) {
            return it.key();
        }
    }
    return -1;
}

bool KisPaletteModel::addEntry(const KisSwatch &entry, const QString &groupName)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount() + 1);
    m_colorSet->add(entry, groupName);
    endInsertRows();
    if (m_colorSet->isGlobal()) {
        m_colorSet->save();
    }
    emit sigPaletteModified();
    return true;
}

bool KisPaletteModel::removeEntry(const QModelIndex &index, bool keepColors)
{
    if (!qvariant_cast<bool>(data(index, IsGroupNameRole))) {
        static_cast<KisSwatchGroup*>(index.internalPointer())->removeEntry(index.column(),
                                                                           rowNumberInGroup(index.row()));
        emit dataChanged(index, index);
    } else {
        int groupNameRow = groupNameRowForRow(index.row());
        QString groupName = m_rowGroupNameMap[groupNameRow];
        removeGroup(groupName, keepColors);
    }
    emit sigPaletteModified();
    return true;
}

void KisPaletteModel::removeGroup(const QString &groupName, bool keepColors)
{
    int removeStart = groupNameRowForName(groupName);
    int removedRowCount = m_colorSet->getGroup(groupName)->rowCount();
    int insertStart = m_colorSet->getGlobalGroup()->rowCount();
    beginRemoveRows(QModelIndex(),
                    removeStart,
                    removeStart + removedRowCount);
    m_colorSet->removeGroup(groupName, keepColors);
    resetGroupNameRows();
    endRemoveRows();
    beginInsertRows(QModelIndex(),
		    insertStart, m_colorSet->getGlobalGroup()->rowCount());
    endInsertRows();
    emit sigPaletteModified();
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
            KisSwatchGroup *groupDragged = m_colorSet->getGroup(groupNameDragged);
            int start = groupNameRowForName(groupNameDragged);
            int end = start + groupDragged->rowCount();
            if (!beginMoveRows(QModelIndex(), start, end, QModelIndex(), groupNameRowForName(groupNameDroppedOn))) {
                return false;
            }
            m_colorSet->moveGroup(groupNameDragged, groupNameDroppedOn);
            resetGroupNameRows();
            endMoveRows();
            emit sigPaletteModified();
            if (m_colorSet->isGlobal()) {
                m_colorSet->save();
            }
        }
        return true;
    }

    if (qvariant_cast<bool>(finalIndex.data(KisPaletteModel::IsGroupNameRole))) {
        return true;
    }

    QByteArray encodedData = data->data("krita/x-colorsetentry");
    QDataStream stream(&encodedData, QIODevice::ReadOnly);

    while (!stream.atEnd()) {
        KisSwatch entry;

        QString name, id;
        bool spotColor;
        QString oldGroupName;
        int oriRow;
        int oriColumn;
        QString colorXml;

        stream >> name >> id >> spotColor
                >> oriRow >> oriColumn
                >> oldGroupName
                >> colorXml;

        entry.setName(name);
        entry.setId(id);
        entry.setSpotColor(spotColor);

        QDomDocument doc;
        doc.setContent(colorXml);
        QDomElement e = doc.documentElement();
        QDomElement c = e.firstChildElement();
        if (!c.isNull()) {
            QString colorDepthId = c.attribute("bitdepth", Integer8BitsColorDepthID.id());
            entry.setColor(KoColor::fromXML(c, colorDepthId));
        }

        if (action == Qt::MoveAction){
            KisSwatchGroup *g = m_colorSet->getGroup(oldGroupName);
            if (g) {
                if (qvariant_cast<bool>(finalIndex.data(KisPaletteModel::CheckSlotRole))) {
                    g->setEntry(getEntry(finalIndex), oriColumn, oriRow);
                } else {
                    g->removeEntry(oriColumn, oriRow);
                }
            }
            setEntry(entry, finalIndex);
            emit sigPaletteModified();
            if (m_colorSet->isGlobal()) {
                m_colorSet->save();
            }
        }
    }

    return true;
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
            KisSwatch entry = getEntry(index);

            QDomDocument doc;
            QDomElement root = doc.createElement("Color");
            root.setAttribute("bitdepth", entry.color().colorSpace()->colorDepthId().id());
            doc.appendChild(root);
            entry.color().toXML(doc, root);

            stream << entry.name() << entry.id() << entry.spotColor()
                   << rowNumberInGroup(index.row()) << index.column()
                   << qvariant_cast<QString>(index.data(GroupNameRole))
                   << doc.toString();
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

void KisPaletteModel::setEntry(const KisSwatch &entry,
                               const QModelIndex &index)
{
    KisSwatchGroup *group = static_cast<KisSwatchGroup*>(index.internalPointer());
    Q_ASSERT(group);
    group->setEntry(entry, index.column(), rowNumberInGroup(index.row()));
    emit sigPaletteModified();
    emit dataChanged(index, index);
    if (m_colorSet->isGlobal()) {
        m_colorSet->save();
    }
}

bool KisPaletteModel::renameGroup(const QString &groupName, const QString &newName)
{
    beginResetModel();
    bool success = m_colorSet->changeGroupName(groupName, newName);
    for (auto it = m_rowGroupNameMap.begin(); it != m_rowGroupNameMap.end(); it++) {
        if (it.value() == groupName) {
            m_rowGroupNameMap[it.key()] = newName;
            break;
        }
    }
    endResetModel();
    emit sigPaletteModified();
    return success;
}

void KisPaletteModel::addGroup(const KisSwatchGroup &group)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount() + group.rowCount());
    m_colorSet->addGroup(group.name());
    *m_colorSet->getGroup(group.name()) = group;
    endInsertColumns();

    emit sigPaletteModified();
}

void KisPaletteModel::setRowNumber(const QString &groupName, int rowCount)
{
    beginResetModel();
    KisSwatchGroup *g = m_colorSet->getGroup(groupName);
    if (g) {
        g->setRowCount(rowCount);
    }
    endResetModel();
}

void KisPaletteModel::clear()
{
    beginResetModel();
    m_colorSet->clear();
    endResetModel();
}

QVariant KisPaletteModel::dataForGroupNameRow(const QModelIndex &idx, int role) const
{
    KisSwatchGroup *group = static_cast<KisSwatchGroup*>(idx.internalPointer());
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
    KisSwatchGroup *group = static_cast<KisSwatchGroup*>(idx.internalPointer());
    Q_ASSERT(group);
    int rowInGroup = rowNumberInGroup(idx.row());
    bool entryPresent = group->checkEntry(idx.column(), rowInGroup);
    KisSwatch entry;
    if (entryPresent) {
        entry = group->getEntry(idx.column(), rowInGroup);
    }
    switch (role) {
    case Qt::ToolTipRole:
    case Qt::DisplayRole: {
        return entryPresent ? entry.name() : i18n("Empty slot");
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

void KisPaletteModel::slotPaletteModified() {
    m_colorSet->setPaletteType(KoColorSet::KPL);
}

QModelIndex KisPaletteModel::indexForClosest(const KoColor &compare)
{
    KisSwatchGroup::SwatchInfo info = colorSet()->getClosestColorInfo(compare);
    return createIndex(indexRowForInfo(info), info.column, colorSet()->getGroup(info.group));
}

int KisPaletteModel::indexRowForInfo(const KisSwatchGroup::SwatchInfo &info)
{
    for (auto it = m_rowGroupNameMap.begin(); it != m_rowGroupNameMap.end(); it++) {
        if (it.value() == info.group) {
            return it.key() + info.row + 1;
        }
    }
    return info.row;
}

KisSwatch KisPaletteModel::getEntry(const QModelIndex &index) const
{
    KisSwatchGroup *group = static_cast<KisSwatchGroup*>(index.internalPointer());
    if (!group || !group->checkEntry(index.column(), rowNumberInGroup(index.row()))) {
        return KisSwatch();
    }
    return group->getEntry(index.column(), rowNumberInGroup(index.row()));
}

int KisPaletteModel::groupNameRowForRow(int rowInModel) const
{
    return rowInModel - rowNumberInGroup(rowInModel) - 1;
}
