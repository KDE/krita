/*
 *  Copyright (c) 2013 Sven Langkamp <sven.langkamp@gmail.com>
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
    , m_colorSet(0)
    , m_displayRenderer(KoDumbColorDisplayRenderer::instance())
{
}

KisPaletteModel::~KisPaletteModel()
{
}

QVariant KisPaletteModel::data(const QModelIndex& index, int role) const
{
    // row is set to -1 when it's group name row
    bool groupNameRow = index.row() != 0 && m_groupNameRows.contains(index.row());
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
            + m_groupNameRows.size()  // rows for names
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
    KisSwatchGroup *group = Q_NULLPTR;
    int groupNameRow = groupNameRowForRow(row);
    group = m_colorSet->getGroup(m_groupNameRows[groupNameRow]);
    Q_ASSERT(group);
    qDebug() << "KisPaletteModel::index" << row << column << group;
    return createIndex(row, column, group);
}

void KisPaletteModel::setColorSet(KoColorSet* colorSet)
{
    beginResetModel();
    m_groupNameRows.clear();
    m_colorSet = colorSet;
    int row = -1;
    for (const QString &groupName : m_colorSet->getGroupNames()) {
        m_groupNameRows[row] = groupName;
        row += m_colorSet->getGroup(groupName)->rowCount();
        row += 1; // row for group name
    }
    endResetModel();
}

KoColorSet* KisPaletteModel::colorSet() const
{
    return m_colorSet;
}

int KisPaletteModel::rowNumberInGroup(int rowInModel) const
{
    if (m_groupNameRows.contains(rowInModel)) {
        return -1;
    }
    for (auto it = m_groupNameRows.keys().rbegin(); it != m_groupNameRows.keys().rend(); it++) {
        if (*it < rowInModel) {
            return rowInModel - *it - 1;
        }
    }
    return rowInModel;
}

bool KisPaletteModel::addEntry(const KisSwatch &entry, const QString &groupName)
{
    KisSwatchGroup *group = m_colorSet->getGroup(groupName);
    if (group->checkEntry(group->columnCount(), group->rowCount())) {
        beginInsertRows(QModelIndex(), rowCount(), rowCount());
        m_colorSet->add(entry, groupName);
        endInsertRows();
    } else {
        beginResetModel();
        m_colorSet->add(entry, groupName);
        endResetModel();
    }
    m_colorSet->save();
    return true;
}

bool KisPaletteModel::removeEntry(const QModelIndex &index, bool keepColors)
{
    if (!qvariant_cast<bool>(data(index, IsGroupNameRole))) {
        static_cast<KisSwatchGroup*>(index.internalPointer())->removeEntry(index.column(),
                                                                           rowNumberInGroup(index.row()));
        emit dataChanged(index, index);
    } else {
        beginResetModel();
        int groupNameRow = groupNameRowForRow(index.row());
        QString groupName = m_groupNameRows[groupNameRow];
        m_colorSet->removeGroup(groupName, keepColors);
        m_groupNameRows.remove(groupNameRow);
        endResetModel();
    }
    return true;
}

bool KisPaletteModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
                                   int row, int column, const QModelIndex &parent)
{
    return true;
    /*
    if (!data->hasFormat("krita/x-colorsetentry") && !data->hasFormat("krita/x-colorsetgroup")) {
        return false;
    }
    if (action == Qt::IgnoreAction) {
        return false;
    }

    int endRow;
    int endColumn;

    if (!parent.isValid()) {
        if (row < 0) {
            endRow = indexFromId(m_colorSet->nColors()).row();
            endColumn = indexFromId(m_colorSet->nColors()).column();
        } else {
            endRow = qMin(row, indexFromId(m_colorSet->nColors()).row());
            endColumn = qMin(column, m_colorSet->columnCount());
        }
    } else {
        endRow = qMin(parent.row(), rowCount());
        endColumn = qMin(parent.column(), columnCount());
    }

    if (data->hasFormat("krita/x-colorsetgroup")) {
        QByteArray encodedData = data->data("krita/x-colorsetgroup");
        QDataStream stream(&encodedData, QIODevice::ReadOnly);

        while (!stream.atEnd()) {
            QString groupName;
            stream >> groupName;
            QModelIndex index = this->index(endRow, 0);
            if (index.isValid()) {
                QStringList entryList = qvariant_cast<QStringList>(index.data(RetrieveEntryRole));
                QString groupDroppedOn = QString();
                if (!entryList.isEmpty()) {
                    groupDroppedOn = entryList.at(0);
                }
                int groupIndex = colorSet()->getGroupNames().indexOf(groupName);
                beginMoveRows(  QModelIndex(), groupIndex, groupIndex, QModelIndex(), endRow);
                m_colorSet->moveGroup(groupName, groupDroppedOn);
                m_colorSet->save();
                endMoveRows();

                ++endRow;
            }
        }
    } else {
        QByteArray encodedData = data->data("krita/x-colorsetentry");
        QDataStream stream(&encodedData, QIODevice::ReadOnly);

        while (!stream.atEnd()) {
            KoColorSetEntry entry;
            QString oldGroupName;
            int indexInGroup;
            QString colorXml;

            QString name, id;
            bool spotColor;
            stream >> name
                    >> id
                    >> spotColor
                    >> indexInGroup
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

            QModelIndex index = this->index(endRow, endColumn);
            if (qvariant_cast<bool>(index.data(IsGroupNameRole))){
                endRow+=1;
            }
            if (index.isValid()) {
                // this is to figure out the row of the old color.
                // That way we can in turn avoid moverows from complaining the
                // index is out of bounds when using index.
                // Makes me wonder if we shouldn't just insert the index of the
                // old color when requesting the mimetype...
                int i = indexInGroup;
                if (oldGroupName != QString()) {
                    colorSet()->nColorsGroup("");
                    //find at which position the group is.
                    int groupIndex = colorSet()->getGroupNames().indexOf(oldGroupName);
                    //add all the groupsizes onto it till we get to our group.
                    for(int g=0; g<groupIndex; g++) {
                        i+=colorSet()->nColorsGroup(colorSet()->getGroupNames().at(g));
                    }
                }
                QModelIndex indexOld = indexFromId(i);
                if (action == Qt::MoveAction){
                    if (indexOld.row()!=qMax(endRow, 0) && indexOld.row()!=qMax(endRow+1,1)) {
                    beginMoveRows(QModelIndex(), indexOld.row(), indexOld.row(), QModelIndex(), qMax(endRow+1,1));
                    }
                    if (indexOld.column()!=qMax(endColumn, 0) && indexOld.column()!=qMax(endColumn+1,1)) {
                    beginMoveColumns(QModelIndex(), indexOld.column(), indexOld.column(), QModelIndex(), qMax(endColumn+1,1));
                    }
                } else {
                    beginInsertRows(QModelIndex(), endRow, endRow);
                }
                QStringList entryList = qvariant_cast<QStringList>(index.data(RetrieveEntryRole));
                QString entryInGroup = "0";
                QString groupName = QString();
                if (!entryList.isEmpty()) {
                    groupName = entryList.at(0);
                    entryInGroup = entryList.at(1);
                }

                int location = entryInGroup.toInt();
                // Insert the entry
                if (groupName==oldGroupName && qvariant_cast<bool>(index.data(IsGroupNameRole))==true) {
                    groupName=QString();
                    location=m_colorSet->nColorsGroup();
                }
                m_colorSet->insertBefore(entry, location, groupName);
                if (groupName==oldGroupName && location<indexInGroup) {
                    indexInGroup+=1;
                }
                if (action == Qt::MoveAction){
                    m_colorSet->removeAt(indexInGroup, oldGroupName);
                }
                m_colorSet->save();
                if (action == Qt::MoveAction){
                    if (indexOld.row()!=qMax(endRow, 0) && indexOld.row()!=qMax(endRow+1,1)) {
                        endMoveRows();
                    }
                    if (indexOld.column()!=qMax(endColumn, 0) && indexOld.column()!=qMax(endColumn+1,1)) {
                        endMoveColumns();
                    }

                } else {
                    endInsertRows();
                }

                ++endRow;
            }
        }
    }

    return true;
    */
}

QMimeData *KisPaletteModel::mimeData(const QModelIndexList &indexes) const
{
    return new QMimeData();
    /*
    QMimeData *mimeData = new QMimeData();
    QByteArray encodedData;

    QDataStream stream(&encodedData, QIODevice::WriteOnly);
    QString mimeTypeName = "krita/x-colorsetentry";
    //Q_FOREACH(const QModelIndex &index, indexes) {
    QModelIndex index = indexes.last();
    if (index.isValid()) {
        if (qvariant_cast<bool>(index.data(IsGroupNameRole))==false) {
            KoColorSetEntry entry = colorSetEntryFromIndex(index);
            QStringList entryList = qvariant_cast<QStringList>(index.data(RetrieveEntryRole));
            QString groupName = QString();
            int indexInGroup = 0;
            if (!entryList.isEmpty()) {
                groupName = entryList.at(0);
                QString iig = entryList.at(1);
                indexInGroup = iig.toInt();
            }

            QDomDocument doc;
            QDomElement root = doc.createElement("Color");
            root.setAttribute("bitdepth", entry.color().colorSpace()->colorDepthId().id());
            doc.appendChild(root);
            entry.color().toXML(doc, root);

            stream << entry.name()
                   << entry.id()
                   << entry.spotColor()
                   << indexInGroup
                   << groupName
                   << doc.toString();
        } else {
            mimeTypeName = "krita/x-colorsetgroup";
            QStringList entryList = qvariant_cast<QStringList>(index.data(RetrieveEntryRole));
            QString groupName = QString();
            if (!entryList.isEmpty()) {
                groupName = entryList.at(0);
            }
            stream << groupName;
        }
    }

    mimeData->setData(mimeTypeName, encodedData);
    return mimeData;
    */
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
    emit dataChanged(index, index);
    if (m_colorSet->isGlobal()) {
        m_colorSet->save();
    }
}

bool KisPaletteModel::renameGroup(const QString &groupName, const QString &newName)
{
    beginResetModel();
    bool success = m_colorSet->changeGroupName(groupName, newName);
    for (auto it = m_groupNameRows.begin(); it != m_groupNameRows.end(); it++) {
        if (it.value() == groupName) {
            m_groupNameRows[it.key()] = newName;
            break;
        }
    }
    endResetModel();
    return success;
}

QVariant KisPaletteModel::dataForGroupNameRow(const QModelIndex &idx, int role) const
{
    KisSwatchGroup *group = static_cast<KisSwatchGroup*>(idx.internalPointer());
    Q_ASSERT(group);
    qDebug() << "KisPaletteModel::dataForGroupNameRow" << idx.row() << idx.column() << group;
    QString groupName = group->name();
    switch (role) {
    case Qt::ToolTipRole:
    case Qt::DisplayRole: {
        return groupName;
    }
    case IsGroupNameRole: {
        return true;
    }
    case CheckSlotRole: {
        return true;
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
    bool entryPresent = group->checkEntry(idx.column(), rowNumberInGroup(idx.row()));
    KisSwatch entry;
    if (entryPresent) {
        entry = group->getEntry(idx.column(), rowNumberInGroup(idx.row()));
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
    case IsGroupNameRole: {
        return false;
    }
    case CheckSlotRole: {
        return entryPresent;
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
                SLOT(slotDisplayConfigurationChanged()));
    } else {
        m_displayRenderer = KoDumbColorDisplayRenderer::instance();
    }
}

void KisPaletteModel::slotDisplayConfigurationChanged()
{
    beginResetModel();
    endResetModel();
}

QModelIndex KisPaletteModel::indexForClosest(const KoColor &compare)
{
    KisSwatchGroup::SwatchInfo info = colorSet()->getClosestColorInfo(compare);
    return createIndex(info.row, info.column, colorSet()->getGroup(info.group));
}

KisSwatch KisPaletteModel::getEntry(const QModelIndex &index)
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
