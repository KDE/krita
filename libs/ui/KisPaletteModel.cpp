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

        int rowstotal = m_colorSet->nColorsGroup()/m_colorSet->columnCount();
        if (index.row()<=rowstotal) {
            indexInGroup = (quint32)(index.row()*columnCount()+index.column());
        }
        Q_FOREACH (QString groupName, m_colorSet->getGroupNames()){
            //we make an int for the rows added by the current group.
            int newrows = 1+m_colorSet->nColorsGroup(groupName)/m_colorSet->columnCount();
            if (m_colorSet->nColorsGroup(groupName)%m_colorSet->columnCount() > 0) {
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
            case Qt::BackgroundRole: {
                QColor color = QColor(Qt::white);
                return QBrush(color);
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
    if (m_colorSet->columnCount() > 0) {
        int countedrows = m_colorSet->nColorsGroup("")/m_colorSet->columnCount() +1;
        Q_FOREACH (QString groupName, m_colorSet->getGroupNames()) {
            countedrows += 1; //add one for the name;
            countedrows += (m_colorSet->nColorsGroup(groupName)/ m_colorSet->columnCount()) +1;
        }
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

Qt::ItemFlags KisPaletteModel::flags(const QModelIndex& /*index*/) const
{
    Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
    return flags;
}

QModelIndex KisPaletteModel::index(int row, int column, const QModelIndex& parent) const
{
    if (m_colorSet) {

        //make an int to hold the amount of rows we've looked at. The initial is the total rows in the default group.
        int rowstotal = m_colorSet->nColorsGroup()/m_colorSet->columnCount();
        if (row<=rowstotal) {
            //if the total rows are in the default group, we just return an index.
            return QAbstractTableModel::index(row, column, parent);
        }
        Q_FOREACH (QString groupName, m_colorSet->getGroupNames()){
            //we make an int for the rows added by the current group.
            int newrows = 1 + m_colorSet->nColorsGroup(groupName)/m_colorSet->columnCount();
            if (m_colorSet->nColorsGroup(groupName)%m_colorSet->columnCount() > 0) {
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
    const int width = columnCount();
    return width > 0 ? index(i / width, i & width) : QModelIndex();
}

int KisPaletteModel::idFromIndex(const QModelIndex &index) const
{
    return index.isValid() ? index.row() * columnCount() + index.column() : -1;
}


