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
#include <resources/KoColorSet.h>
#include <KoColorDisplayRendererInterface.h>

#include <kis_layer.h>
#include <kis_paint_layer.h>

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
    reset();
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
        if (index.row()<=rowstotal) {
            indexInGroup = (quint32)(index.row()*columnCount()+index.column());
        }
        Q_FOREACH (QString groupName, m_colorSet->getGroupNames()){
            //we make an int for the rows added by the current group.
            int newrows = 1+m_colorSet->nColorsGroup(groupName)/columnCount();
            if (m_colorSet->nColorsGroup(groupName)%columnCount() > 0) {
                newrows+=1;
            }
            if (index.row() == rowstotal+1) {
                //rowstotal+1 is taken up by the groupname.
                indexGroupName = groupName;
                groupNameRow = true;
            } else if (index.row() > (rowstotal+1) && index.row() <= rowstotal+newrows){
                //otherwise it's an index to the colors in the group.
                indexGroupName = groupName;
                indexInGroup = (quint32)((index.row()-(rowstotal+2))*columnCount()+index.column());
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
                    return entry.name;
                }
                case Qt::BackgroundRole: {
                    QColor color = m_displayRenderer->toQColor(entry.color);
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
    if (columnCount() > 0) {
        int countedrows = m_colorSet->nColorsGroup("")/columnCount();
        Q_FOREACH (QString groupName, m_colorSet->getGroupNames()) {
            countedrows += 1; //add one for the name;
            countedrows += (m_colorSet->nColorsGroup(groupName)/ columnCount());
            if (m_colorSet->nColorsGroup(groupName)%columnCount() > 0) {
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
        if (row<=rowstotal) {
            //if the total rows are in the default group, we just return an index.
            return QAbstractTableModel::index(row, column, parent);
        }
        Q_FOREACH (QString groupName, m_colorSet->getGroupNames()){
            //we make an int for the rows added by the current group.
            int newrows = 1 + m_colorSet->nColorsGroup(groupName)/columnCount();
            if (m_colorSet->nColorsGroup(groupName)%columnCount() > 0) {
                newrows+=1;
            }
            if (row == rowstotal+1) {
                //rowstotal+1 is taken up by the groupname.
                return QAbstractTableModel::index(row, 0, parent);
            } else if (row > (rowstotal+1) && row <= rowstotal+newrows){
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
    reset();
}

KoColorSet* KisPaletteModel::colorSet() const
{
    return m_colorSet;
}

QModelIndex KisPaletteModel::indexFromId(int i) const
{
    QModelIndex index = QModelIndex();
    if (i < (int)colorSet()->nColorsGroup(0)) {
        index = QAbstractTableModel::index(i/columnCount(), i%columnCount());
        return index;
    } else {
        int rowstotal = m_colorSet->nColorsGroup()/columnCount();
        int totalIndexes = colorSet()->nColorsGroup();
        Q_FOREACH (QString groupName, m_colorSet->getGroupNames()){
            totalIndexes += colorSet()->nColorsGroup(groupName);
            if (totalIndexes<i) {
                rowstotal += m_colorSet->nColorsGroup(groupName)/columnCount();
                if (m_colorSet->nColorsGroup(groupName)%columnCount() > 0) {
                    rowstotal+=1;
                }
                rowstotal+=1;
            } else {
                index = QAbstractTableModel::index(rowstotal, i-(rowstotal*columnCount()));
            }
        }
    }
    return index;
}

int KisPaletteModel::idFromIndex(const QModelIndex &index) const
{
    if (index.isValid()==false || qVariantValue<bool>(index.data(IsHeaderRole))) {
        return -1;
    }
    int i=0;
    QStringList entryList = qVariantValue<QStringList>(data(index, RetrieveEntryRole));
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
    QStringList entryList = qVariantValue<QStringList>(data(index, RetrieveEntryRole));
    QString groupName = entryList.at(0);
    quint32 indexInGroup = entryList.at(1).toUInt();
    return m_colorSet->getColorGroup(indexInGroup, groupName);
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
                                   int row, int /*column*/, const QModelIndex &parent)
{
    if (!data->hasFormat("krita/x-colorsetentry")) {
        return false;
    }
    if (action == Qt::IgnoreAction) {
        return false;
    }

    int endRow;

    if (!parent.isValid()) {
        if (row < 0)
            endRow = m_colorSet->nColors();
        else
            endRow = qMin(row, (int)m_colorSet->nColors());
    } else {
        endRow = parent.row();
    }

    QByteArray encodedData = data->data("krita/x-colorsetentry");
    QDataStream stream(&encodedData, QIODevice::ReadOnly);

    while (!stream.atEnd()) {

        KoColorSetEntry entry;

        QString colorXml;

        stream >> entry.name
                >> entry.id
                >> entry.spotColor
                >> colorXml;

        QDomDocument doc;
        doc.setContent(colorXml);
        QDomElement e = doc.documentElement();
        QDomElement c = e.firstChildElement("Color");
        if (!c.isNull()) {
            QString colorDepthId = c.attribute("bitdepth", Integer8BitsColorDepthID.id());
            entry.color = KoColor::fromXML(c, colorDepthId);
        }


        beginInsertRows(QModelIndex(), endRow, endRow);

        // Insert the entry
        m_colorSet->insertBefore(entry, endRow, "Vogel Dit Maar Uit!");

        endInsertRows();

        ++endRow;
    }

    return true;
}

QMimeData *KisPaletteModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *mimeData = new QMimeData();
    QByteArray encodedData;

    QDataStream stream(&encodedData, QIODevice::WriteOnly);

    Q_FOREACH(const QModelIndex &index, indexes) {
        if (index.isValid()) {
            KoColorSetEntry entry = colorSetEntryFromIndex(index);

            QDomDocument doc;
            QDomElement root = doc.createElement("Color");
            root.setAttribute("bitdepth", entry.color.colorSpace()->colorDepthId().id());
            doc.appendChild(root);
            entry.color.toXML(doc, root);

            stream << entry.name
                   << entry.id
                   << entry.spotColor
                   << doc.toString();
        }
    }

    mimeData->setData("krita/x-colorsetentry", encodedData);
    return mimeData;
}

QStringList KisPaletteModel::mimeTypes() const
{
    return QStringList() << "krita/x-colorsetentry";
}

Qt::DropActions KisPaletteModel::supportedDropActions() const
{
    return Qt::MoveAction;
}
