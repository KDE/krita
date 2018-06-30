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
    : QAbstractTableModel(parent),
      m_colorSet(0),
      m_displayRenderer(KoDumbColorDisplayRenderer::instance())
{
}

KisPaletteModel::~KisPaletteModel()
{
}

void KisPaletteModel::setDisplayRenderer(KoColorDisplayRendererInterface *displayRenderer)
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

QModelIndex KisPaletteModel::getLastEntryIndex()
{
    int endRow = rowCount();
    int endColumn = columnCount();
    if (m_colorSet->nColors()>0) {
        QModelIndex i = this->index(endRow, endColumn, QModelIndex());
        while (qvariant_cast<QStringList>(i.data(RetrieveEntryRole)).isEmpty()) {
            i = this->index(endRow, endColumn);
            endColumn -=1;
            if (endColumn<0) {
                endColumn = columnCount();
                endRow-=1;
            }
        }
        return i;
    }
    return QModelIndex();
}

QVariant KisPaletteModel::data(const QModelIndex& index, int role) const
{
    KoColorSetEntry entry;
    if (m_colorSet && m_displayRenderer) {
        //now to figure out whether we have a groupname row or not.
        bool groupNameRow = false;
        quint32 indexInGroup = 0;
        QString indexGroupName = QString();

        int rowstotal = m_colorSet->nColorsGroup()/columnCount();
        if (index.row()<=rowstotal && (quint32)(index.row()*columnCount()+index.column())<m_colorSet->nColorsGroup()) {
            indexInGroup = (quint32)(index.row()*columnCount()+index.column());
        }
        if (m_colorSet->nColorsGroup()==0) {
            rowstotal+=1; //always add one for the default group when considering groups.
        }
        Q_FOREACH (QString groupName, m_colorSet->getGroupNames()){
            //we make an int for the rows added by the current group.
            int newrows = 1+m_colorSet->nColorsGroup(groupName)/columnCount();
            if (m_colorSet->nColorsGroup(groupName)%columnCount() > 0) {
                newrows+=1;
            }
            if (newrows==0) {
                newrows+=1; //always add one for the group when considering groups.
            }
            quint32 tempIndex = (quint32)((index.row()-(rowstotal+2))*columnCount()+index.column());
            if (index.row() == rowstotal+1) {
                //rowstotal+1 is taken up by the groupname.
                indexGroupName = groupName;
                groupNameRow = true;
            } else if (index.row() > (rowstotal+1) && index.row() <= rowstotal+newrows &&
                       tempIndex<m_colorSet->nColorsGroup(groupName)){
                //otherwise it's an index to the colors in the group.
                indexGroupName = groupName;
                indexInGroup = tempIndex;
            }
            //add the new rows to the totalrows we've looked at.
            rowstotal += newrows;
        }
        if (groupNameRow) {
            switch (role) {
            case Qt::ToolTipRole:
            case Qt::DisplayRole: {
                return indexGroupName;
            }
            case IsHeaderRole: {
                return true;
            }
            case RetrieveEntryRole: {
                QStringList entryList;
                entryList.append(indexGroupName);
                entryList.append(QString::number(0));
                return entryList;
            }
            }
        } else {
            if (indexInGroup < m_colorSet->nColorsGroup(indexGroupName)) {
                entry = m_colorSet->getColorGroup(indexInGroup, indexGroupName);
                switch (role) {
                case Qt::ToolTipRole:
                case Qt::DisplayRole: {
                    return entry.name();
                }
                case Qt::BackgroundRole: {
                    QColor color = m_displayRenderer->toQColor(entry.color());
                    return QBrush(color);
                }
                case IsHeaderRole: {
                    return false;
                }
                case RetrieveEntryRole: {
                    QStringList entryList;
                    entryList.append(indexGroupName);
                    entryList.append(QString::number(indexInGroup));
                    return entryList;
                }
                }
            }
        }
    }
    return QVariant();
}

int KisPaletteModel::rowCount(const QModelIndex& /*parent*/) const
{
    if (!m_colorSet) {
        return 0;
    }
    if (m_colorSet->nColors()==0) {
        return 0;
    }
    if (columnCount() > 0) {
        int countedrows = m_colorSet->nColorsGroup("")/columnCount();
        if (m_colorSet->nColorsGroup()%columnCount() > 0) {
            countedrows+=1;
        }
        if (m_colorSet->nColorsGroup()==0) {
            countedrows+=1;
        }
        Q_FOREACH (QString groupName, m_colorSet->getGroupNames()) {
            countedrows += 1; //add one for the name;
            countedrows += 1+(m_colorSet->nColorsGroup(groupName)/ columnCount());
            if (m_colorSet->nColorsGroup(groupName)%columnCount() > 0) {
                countedrows+=1;
            }
            if (m_colorSet->nColorsGroup(groupName)==0) {
                countedrows+=1;
            }
        }
        countedrows +=1; //Our code up till now doesn't take 0 into account.
        return countedrows;
    }
    return m_colorSet->nColors()/15 + 1;
}

int KisPaletteModel::columnCount(const QModelIndex& /*parent*/) const
{
    if (m_colorSet && m_colorSet->columnCount() > 0) {
        return m_colorSet->columnCount();
    }
    return 15;
}

Qt::ItemFlags KisPaletteModel::flags(const QModelIndex& index) const
{
    if (index.isValid()) {
        return Qt::ItemIsSelectable | Qt::ItemIsEnabled
                | Qt::ItemIsUserCheckable
                | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
    }
    return Qt::ItemIsDropEnabled;
}

QModelIndex KisPaletteModel::index(int row, int column, const QModelIndex& parent) const
{
    if (m_colorSet) {

        //make an int to hold the amount of rows we've looked at. The initial is the total rows in the default group.
        int rowstotal = m_colorSet->nColorsGroup()/columnCount();
        if (row<=rowstotal && (quint32)(row*columnCount()+column)<m_colorSet->nColorsGroup()) {
            //if the total rows are in the default group, we just return an index.
            return QAbstractTableModel::index(row, column, parent);
        } else if(row<0 && column<0) {
            return QAbstractTableModel::index(0, 0, parent);
        }
        if (m_colorSet->nColorsGroup()==0) {
            rowstotal+=1; //always add one for the default group when considering groups.
        }
        Q_FOREACH (QString groupName, m_colorSet->getGroupNames()){
            //we make an int for the rows added by the current group.
            int newrows = 1+m_colorSet->nColorsGroup(groupName)/columnCount();
            if (m_colorSet->nColorsGroup(groupName)%columnCount() > 0) {
                newrows+=1;
            }
            if (m_colorSet->nColorsGroup(groupName)==0) {
                newrows+=1; //always add one for the group when considering groups.
            }
            if (rowstotal + newrows>rowCount()) {
                newrows = rowCount() - rowstotal;
            }
            quint32 tempIndex = (quint32)((row-(rowstotal+2))*columnCount()+column);
            if (row == rowstotal+1) {
                //rowstotal+1 is taken up by the groupname.
                return QAbstractTableModel::index(row, 0, parent);
            } else if (row > (rowstotal+1) && row <= rowstotal+newrows && tempIndex<m_colorSet->nColorsGroup(groupName)){
                //otherwise it's an index to the colors in the group.
                return QAbstractTableModel::index(row, column, parent);
            }
            //add the new rows to the totalrows we've looked at.
            rowstotal += newrows;
        }
    }
    return QModelIndex();
}

void KisPaletteModel::setColorSet(KoColorSet* colorSet)
{
    m_colorSet = colorSet;
    beginResetModel();
    endResetModel();
}

KoColorSet* KisPaletteModel::colorSet() const
{
    return m_colorSet;
}

QModelIndex KisPaletteModel::indexFromId(int i) const
{
    QModelIndex index = QModelIndex();
    if (!colorSet() || colorSet()->nColors() == 0) {
        return index;
    }

    if (i > (int)colorSet()->nColors()) {
        qWarning()<<"index is too big"<<i<<"/"<<colorSet()->nColors();
        index = this->index(0,0);
    }

    if (i < (int)colorSet()->nColorsGroup(0)) {
        index = QAbstractTableModel::index(i/columnCount(), i % columnCount());
        if (!index.isValid()) {
            index = QAbstractTableModel::index(0, 0, QModelIndex());
        }
        return index;
    } else {
        int rowstotal = 1 + m_colorSet->nColorsGroup() / columnCount();
        if (m_colorSet->nColorsGroup() == 0) {
            rowstotal += 1;
        }
        int totalIndexes = colorSet()->nColorsGroup();
        Q_FOREACH (QString groupName, m_colorSet->getGroupNames()){
            if (i + 1 <= (int)(totalIndexes + colorSet()->nColorsGroup(groupName)) && i + 1 > (int)totalIndexes) {
                int col = (i - totalIndexes) % columnCount();
                int row = rowstotal + 1 + ((i - totalIndexes) / columnCount());
                index = this->index(row, col);
                return index;
            } else {
                rowstotal += 1 + m_colorSet->nColorsGroup(groupName) / columnCount();
                totalIndexes += colorSet()->nColorsGroup(groupName);
                if (m_colorSet->nColorsGroup(groupName)%columnCount() > 0) {
                    rowstotal += 1;
                }
                if (m_colorSet->nColorsGroup(groupName)==0) {
                    rowstotal += 1; //always add one for the group when considering groups.
                }
            }
        }
    }
    return index;
}

int KisPaletteModel::idFromIndex(const QModelIndex &index) const
{
    if (index.isValid()==false) {
        return -1;
        qWarning()<<"invalid index";
    }
    int i=0;
    QStringList entryList = qvariant_cast<QStringList>(data(index, RetrieveEntryRole));
    if (entryList.isEmpty()) {
        return -1;
        qWarning()<<"invalid index, there's no data to retrieve here";
    }
    if (entryList.at(0)==QString()) {
        return entryList.at(1).toUInt();
    }

    i = colorSet()->nColorsGroup("");
    //find at which position the group is.
    int groupIndex = colorSet()->getGroupNames().indexOf(entryList.at(0));
    //add all the groupsizes onto it till we get to our group.
    for(int g=0; g<groupIndex; g++) {
        i+=colorSet()->nColorsGroup(colorSet()->getGroupNames().at(g));
    }
    //then add the index.
    i += entryList.at(1).toUInt();
    return i;
}

KoColorSetEntry KisPaletteModel::colorSetEntryFromIndex(const QModelIndex &index) const
{
    KoColorSetEntry blank =  KoColorSetEntry();
    if (!index.isValid()) {
        return blank;
    }
    QStringList entryList = qvariant_cast<QStringList>(data(index, RetrieveEntryRole));
    if (entryList.isEmpty()) {
        return blank;
    }
    QString groupName = entryList.at(0);
    quint32 indexInGroup = entryList.at(1).toUInt();
    return m_colorSet->getColorGroup(indexInGroup, groupName);
}

bool KisPaletteModel::addColorSetEntry(KoColorSetEntry entry, QString groupName)
{
    int col = m_colorSet->nColorsGroup(groupName)%columnCount();
    QModelIndex i = getLastEntryIndex();
    if (col+1 > columnCount()) {
        beginInsertRows(QModelIndex(), i.row(), i.row()+1);
    }
    if ((int)m_colorSet->nColors() < columnCount()) {
        beginInsertColumns(QModelIndex(), m_colorSet->nColors(), m_colorSet->nColors()+1);
    }
    m_colorSet->add(entry, groupName);
    if (col + 1 > columnCount()) {
        endInsertRows();
    }
    if (m_colorSet->nColors() < (quint32)columnCount()) {
        endInsertColumns();
    }
    return true;
}

bool KisPaletteModel::removeEntry(QModelIndex index, bool keepColors)
{
    QStringList entryList =  qvariant_cast<QStringList>(index.data(RetrieveEntryRole));
    if (entryList.empty()) {
        return false;
    }
    QString groupName = entryList.at(0);
    quint32 indexInGroup = entryList.at(1).toUInt();

    if (qvariant_cast<bool>(index.data(IsHeaderRole))==false) {
        if (index.column()-1<0
                && m_colorSet->nColorsGroup(groupName)%columnCount() <1
                && index.row()-1>0
                && m_colorSet->nColorsGroup(groupName)/columnCount()>0) {
            beginRemoveRows(QModelIndex(), index.row(), index.row()-1);
        }
        m_colorSet->removeAt(indexInGroup, groupName);
        if (index.column()-1<0
                && m_colorSet->nColorsGroup(groupName)%columnCount() <1
                && index.row()-1>0
                && m_colorSet->nColorsGroup(groupName)/columnCount()>0) {
            endRemoveRows();
        }
    } else {
        beginRemoveRows(QModelIndex(), index.row(), index.row()-1);
        m_colorSet->removeGroup(groupName, keepColors);
        endRemoveRows();
    }
    return true;
}

bool KisPaletteModel::addGroup(QString groupName)
{
    QModelIndex i = getLastEntryIndex();
    beginInsertRows(QModelIndex(), i.row(), i.row()+1);
    m_colorSet->addGroup(groupName);
    endInsertRows();
    return true;
}

bool KisPaletteModel::removeRows(int row, int count, const QModelIndex &parent)
{
    Q_ASSERT(!parent.isValid());

    int beginRow = qMax(0, row);
    int endRow = qMin(row + count - 1, (int)m_colorSet->nColors() - 1);
    beginRemoveRows(parent, beginRow, endRow);

    // Find the palette entry at row, count, remove from KoColorSet

    endRemoveRows();
    return true;
}

bool KisPaletteModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
                                   int row, int column, const QModelIndex &parent)
{
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
            if (qvariant_cast<bool>(index.data(IsHeaderRole))){
                endRow+=1;
            }
            if (index.isValid()) {
                /*this is to figure out the row of the old color.
                 * That way we can in turn avoid moverows from complaining the
                 * index is out of bounds when using index.
                 * Makes me wonder if we shouldn't just insert the index of the
                 * old color when requesting the mimetype...
                 */
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
                if (groupName==oldGroupName && qvariant_cast<bool>(index.data(IsHeaderRole))==true) {
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
}

QMimeData *KisPaletteModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *mimeData = new QMimeData();
    QByteArray encodedData;

    QDataStream stream(&encodedData, QIODevice::WriteOnly);
    QString mimeTypeName = "krita/x-colorsetentry";
    //Q_FOREACH(const QModelIndex &index, indexes) {
    QModelIndex index = indexes.last();
    if (index.isValid()) {
        if (qvariant_cast<bool>(index.data(IsHeaderRole))==false) {
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
}

QStringList KisPaletteModel::mimeTypes() const
{
    return QStringList() << "krita/x-colorsetentry" << "krita/x-colorsetgroup";
}

Qt::DropActions KisPaletteModel::supportedDropActions() const
{
    return Qt::MoveAction;
}
