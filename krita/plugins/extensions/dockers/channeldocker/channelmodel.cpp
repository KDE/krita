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

ChannelModel::ChannelModel(QObject* parent): QAbstractListModel(parent), m_currentLayer(0)
{
}

ChannelModel::~ChannelModel()
{
}

QVariant ChannelModel::data(const QModelIndex& index, int role) const
{
    if (m_currentLayer && index.isValid()) {
        switch (role) {
            case Qt::DisplayRole: {
                return m_currentLayer->colorSpace()->channels().at(index.row())->name();
            }
            case Qt::CheckStateRole: {
                if (m_currentLayer->channelFlags().isEmpty()) {
                    return Qt::Checked;
                }
                Q_ASSERT(index.row() < m_currentLayer->channelFlags().size());
                return m_currentLayer->channelFlags().testBit(index.row()) ? Qt::Checked : Qt::Unchecked;
                break;
            }
        }
    }
    return QVariant();
}

int ChannelModel::rowCount(const QModelIndex& parent) const
{
    if (m_currentLayer) {
        return m_currentLayer->colorSpace()->channels().size();
    }
    return 0;
}

bool ChannelModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (m_currentLayer && index.isValid())
    {
        if (role == Qt::CheckStateRole) {
            QBitArray channelFlags = m_currentLayer->channelFlags();
            if (channelFlags.isEmpty()) {
                channelFlags.resize(m_currentLayer->colorSpace()->channels().size());
                for (int i = 0; i < m_currentLayer->colorSpace()->channels().size(); i++) {
                    channelFlags.setBit(i, true);
                }
            }
            channelFlags.setBit(index.row(), value.toInt() == Qt::Checked);
            m_currentLayer->setChannelFlags(channelFlags);
            m_currentLayer->setDirty();
            return true;
        }
    }
    return false;
}

Qt::ItemFlags ChannelModel::flags(const QModelIndex& index) const
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
