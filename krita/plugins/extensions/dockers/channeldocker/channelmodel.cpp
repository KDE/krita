/*
 *  Copyright (c) 2011 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "channelmodel.h"

#include <KoColorSpace.h>
#include <kis_layer.h>
#include <kis_paint_layer.h>

ChannelModel::ChannelModel(QObject* parent): QAbstractTableModel(parent), m_currentLayer(0)
{
}

ChannelModel::~ChannelModel()
{
}

QVariant ChannelModel::data(const QModelIndex& index, int role) const
{
    if (m_currentLayer.isValid() && index.isValid())
    {
        QList<KoChannelInfo*> channels = m_currentLayer->colorSpace()->channels();
        int channelIndex = KoChannelInfo::displayPositionToChannelIndex(index.row(), channels);

        switch (role) {
        case Qt::DisplayRole:
        {
            return channels.at(channelIndex)->name();
        }   
        case Qt::CheckStateRole: {
            Q_ASSERT(index.row() < rowCount());
            Q_ASSERT(index.column() < columnCount());
            
            if (index.column() == 0) {
                QBitArray flags = m_currentLayer->channelFlags();
                return (flags.isEmpty() || flags.testBit(channelIndex)) ? Qt::Checked : Qt::Unchecked;
            }
            
            QBitArray flags = dynamic_cast<const KisPaintLayer*>(m_currentLayer.data())->channelLockFlags();
            return (flags.isEmpty() || flags.testBit(channelIndex)) ? Qt::Unchecked : Qt::Checked;
        }
        }
    }
    return QVariant();
}

QVariant ChannelModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        if(section == 0)
            return i18n("Enabled");
        
        return i18n("Locked");
    }
    
    return QAbstractItemModel::headerData(section, orientation, role);
}


int ChannelModel::rowCount(const QModelIndex& /*parent*/) const
{
    return m_currentLayer.isValid() ? m_currentLayer->colorSpace()->channelCount() : 0;
}

int ChannelModel::columnCount(const QModelIndex& /*parent*/) const
{
    return dynamic_cast<const KisPaintLayer*>(m_currentLayer.data()) ? 2 : 1;
}


bool ChannelModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (m_currentLayer.isValid() && index.isValid())
    {
        QList<KoChannelInfo*> channels = m_currentLayer->colorSpace()->channels();
        int channelIndex = KoChannelInfo::displayPositionToChannelIndex(index.row(), channels);
        
        if (role == Qt::CheckStateRole) {
            Q_ASSERT(index.row() < rowCount());
            Q_ASSERT(index.column() < columnCount());
            
            if (index.column() == 0) {
                QBitArray flags = m_currentLayer->channelFlags();

                flags = flags.isEmpty() ? m_currentLayer->colorSpace()->channelFlags(true, true) : flags;
                flags.setBit(channelIndex, value.toInt() == Qt::Checked);
                m_currentLayer->setChannelFlags(flags);
            }
            else { //if (index.column() == 1)
                KisPaintLayer* paintLayer = dynamic_cast<KisPaintLayer*>(m_currentLayer.data());
                QBitArray      flags      = paintLayer->channelLockFlags();
                flags = flags.isEmpty() ? m_currentLayer->colorSpace()->channelFlags(true, true) : flags;
                flags.setBit(channelIndex, value.toInt() == Qt::Unchecked);
                paintLayer->setChannelLockFlags(flags);
            }
            
            m_currentLayer->setDirty();
            return true;
        }
    }
    return false;
}

Qt::ItemFlags ChannelModel::flags(const QModelIndex& /*index*/) const
{
    Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
    return flags;
}

void ChannelModel::slotLayerActivated(KisLayerSP layer)
{
    m_currentLayer = layer;
    reset();
}

#include "channelmodel.moc"
