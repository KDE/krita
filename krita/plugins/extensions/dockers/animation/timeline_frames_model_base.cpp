/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#include "timeline_frames_model_base.h"
#include <QFont>
#include <QSize>
#include <QColor>
#include <QMimeData>


#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_smallint.hpp>

#include "kis_debug.h"


static const int numFrames = 200;

struct Layer {
    QString name;
    QVector<int> frames;
    bool visible;
    bool locked;
    bool alphaLocked;
    bool inheritAlpha;
    bool onionSkins;
};

struct TimelineFramesModelBase::Private
{
    Private() : activeLayerIndex(0), activeFrameIndex(0) {}

    QVector<Layer> layers;
    QVector<bool> cachedFrames;

    int activeLayerIndex;
    int activeFrameIndex;

    QVector<Layer> otherLayers;
};

TimelineFramesModelBase::TimelineFramesModelBase(QObject *parent)
    : QAbstractTableModel(parent),
      m_d(new Private)
{
    boost::uniform_smallint<int> smallint(1,4);
    boost::mt11213b rnd;


    for (int i = 0; i < 5; i++) {
        Layer l;
        l.name = QString("Layer %1").arg(i);
        l.frames.resize(numFrames);
        for (int j = i; j < numFrames; j += smallint(rnd)) {
            l.frames[j] = j / 3;
        }
        l.visible = smallint(rnd) != 1;
        l.locked = smallint(rnd) == 1;
        l.alphaLocked = smallint(rnd) == 1;
        l.inheritAlpha = smallint(rnd) == 1;
        l.onionSkins = smallint(rnd) == 1;
        m_d->layers << l;
    }

    for (int i = 0; i < 7; i++) {
        Layer l;
        l.name = QString("Ext Layer %1").arg(i);
        l.frames.resize(numFrames);
        for (int j = i; j < numFrames; j += smallint(rnd)) {
            l.frames[j] = j / 3;
        }
        l.visible = smallint(rnd) != 1;
        l.locked = smallint(rnd) == 1;
        l.alphaLocked = smallint(rnd) == 1;
        l.inheritAlpha = smallint(rnd) == 1;
        l.onionSkins = smallint(rnd) == 1;
        m_d->otherLayers << l;
    }

    m_d->cachedFrames.resize(numFrames);
    for (int j = 0; j < numFrames; j += smallint(rnd) / 2) {
        m_d->cachedFrames[j] = true;
    }
}

TimelineFramesModelBase::~TimelineFramesModelBase()
{
}

int TimelineFramesModelBase::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_d->layers.size();
}

int TimelineFramesModelBase::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return numFrames;
}

QVariant TimelineFramesModelBase::data(const QModelIndex &index, int role) const
{
    //if (index.row() > m_d->layers.size()) return QVariant();

    switch (role) {
    case ActiveLayerRole: {
        return index.row() == m_d->activeLayerIndex;
    }
    case ActiveFrameRole: {
        return index.column() == m_d->activeFrameIndex;
    }
    case FrameEditableRole: {
        const Layer &layer = m_d->layers[index.row()];
        return layer.visible && !layer.locked;
    }
    case Qt::DisplayRole: {
        int frameId = m_d->layers[index.row()].frames[index.column()];

        return frameId > 0 ? QString::number(frameId) : "";

    }
    case Qt::BackgroundRole: {
        const Layer &layer = m_d->layers[index.row()];
        int frameId = layer.frames[index.column()];

        QColor baseColor = QColor(200, 220, 150);
        bool active = data(index, ActiveLayerRole).toBool();

        bool present = frameId > 0;

        QColor color = Qt::transparent;

        if (present && !active) {
            color = baseColor;
        } else if (present && active) {
            color = baseColor.darker(130);
        } else if (!present && active) {
            color = baseColor.lighter(140);
        }

        if (!data(index, FrameEditableRole).toBool() && color.alpha() > 0) {
            const int l = color.lightness();
            color = QColor(l, l, l);
        }

        return color;
    }
    case Qt::TextAlignmentRole: {
        return QVariant(Qt::AlignHCenter | Qt::AlignVCenter);
    }
    }


    return QVariant();
}

bool TimelineFramesModelBase::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid()) return false;

    switch (role) {
    case ActiveLayerRole: {
        if (value.toBool()) {
            int prevLayer = m_d->activeLayerIndex;
            m_d->activeLayerIndex = index.row();

            emit dataChanged(this->index(prevLayer, 0), this->index(prevLayer, columnCount() - 1));
            emit dataChanged(this->index(m_d->activeLayerIndex, 0), this->index(m_d->activeLayerIndex, columnCount() - 1));
        }
        break;
    }
    case ActiveFrameRole: {
        if (value.toBool()) {
            int prevFrame = m_d->activeFrameIndex;
            m_d->activeFrameIndex = index.column();

            emit dataChanged(this->index(0, prevFrame), this->index(rowCount() - 1, prevFrame));
            emit dataChanged(this->index(0, m_d->activeFrameIndex), this->index(rowCount() - 1, m_d->activeFrameIndex));

            emit headerDataChanged (Qt::Horizontal, prevFrame, prevFrame);
            emit headerDataChanged (Qt::Horizontal, m_d->activeFrameIndex, m_d->activeFrameIndex);
        }
        break;
    }
    }

    return QAbstractTableModel::setData(index, value, role);
}

QVariant TimelineFramesModelBase::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal) {
        switch (role) {
        case ActiveFrameRole: {
            return section == m_d->activeFrameIndex;
        }
        case FrameCachedRole:
            return m_d->cachedFrames[section];
        }

    } else {
        switch (role) {
        case Qt::DisplayRole: {
            QString name = m_d->layers[section].name;

            const int maxNameSize = 13;

            if (name.size() > maxNameSize) {
                name = QString("%1...").arg(name.left(maxNameSize));
            }

            return name;
        }
        case Qt::ToolTipRole: {
            return m_d->layers[section].name;
        }
        case PropertiesRole: {
            const Layer &l = m_d->layers[section];

            PropertyList props;
            props << Property("Visible",
                             QIcon::fromTheme("system-lock-screen"),
                             QIcon::fromTheme("window-close"),
                             l.visible);

            props << Property("Locked",
                             QIcon::fromTheme("system-lock-screen"),
                             QIcon::fromTheme("window-close"),
                             l.locked);

            props << Property("Alpha Locked",
                             QIcon::fromTheme("system-lock-screen"),
                             QIcon::fromTheme("window-close"),
                             l.alphaLocked);

            props << Property("Inherit Alpha",
                             QIcon::fromTheme("system-lock-screen"),
                             QIcon::fromTheme("window-close"),
                             l.inheritAlpha);

            props << Property("Onion Skins",
                             QIcon::fromTheme("system-lock-screen"),
                             QIcon::fromTheme("window-close"),
                             l.onionSkins);

            return QVariant::fromValue(props);

        }
        case OtherLayersRole: {
            OtherLayersList list;

            foreach (const Layer &l, m_d->otherLayers) {
                list << OtherLayer(l.name);
            }

            return QVariant::fromValue(list);
        }
        }
    }

    return QVariant();
}

bool TimelineFramesModelBase::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
    if (orientation == Qt::Horizontal) {
        // noop...
    } else {
        switch (role) {
        case ActiveLayerRole: {
            setData(index(section, 0), value, role);
            break;
        }
        case PropertiesRole: {
            TimelineFramesModelBase::PropertyList props = value.value<TimelineFramesModelBase::PropertyList>();

            Layer &l = m_d->layers[section];
            l.visible = props[0].state.toBool();
            l.locked = props[1].state.toBool();
            l.alphaLocked = props[2].state.toBool();
            l.inheritAlpha = props[3].state.toBool();
            l.onionSkins = props[4].state.toBool();

            emit headerDataChanged (Qt::Vertical, section, section);
            break;
        }
        }
    }

    return QAbstractTableModel::setHeaderData(section, orientation, value, role);
}

Qt::DropActions TimelineFramesModelBase::supportedDragActions() const
{
    return Qt::MoveAction;
}

Qt::DropActions TimelineFramesModelBase::supportedDropActions() const
{
    return Qt::MoveAction;
}

QStringList TimelineFramesModelBase::mimeTypes() const
{
    QStringList types;
    types << QLatin1String("application/x-krita-frame");
    return types;
}

QMimeData* TimelineFramesModelBase::mimeData(const QModelIndexList &indexes) const
{
    QModelIndex index = indexes.first();

    QMimeData *data = new QMimeData();

    QByteArray encoded;
    QDataStream stream(&encoded, QIODevice::WriteOnly);
    stream << index.row() << index.column();
    data->setData("application/x-krita-frame", encoded);

    QString text = QString("frame_%1_%2").arg(index.row(), index.column());
    data->setText(text);

    return data;
}

inline void decodeFrameId(QByteArray *encoded, int *row, int *col)
{
    QDataStream stream(encoded, QIODevice::ReadOnly);
    stream >> *row >> *col;
}

bool TimelineFramesModelBase::canDropFrameData(const QMimeData *data, const QModelIndex &index)
{
    if (!index.isValid()) return false;

    QByteArray encoded = data->data("application/x-krita-frame");
    int srcRow, srcColumn;
    decodeFrameId(&encoded, &srcRow, &srcColumn);

    return srcRow == index.row();
}

bool TimelineFramesModelBase::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    Q_UNUSED(row);
    Q_UNUSED(column);

    if (action != Qt::MoveAction || !parent.isValid()) return false;

    QByteArray encoded = data->data("application/x-krita-frame");
    int srcRow, srcColumn;
    decodeFrameId(&encoded, &srcRow, &srcColumn);

    if (srcRow != parent.row()) return false;

    // fake part

    int *src = &m_d->layers[srcRow].frames[srcColumn];
    int *dst = &m_d->layers[parent.row()].frames[parent.column()];
    qSwap(*src, *dst);

    emit dataChanged(this->index(srcRow, srcColumn), this->index(srcRow, srcColumn));
    emit dataChanged(parent, parent);

    return true;
}

Qt::ItemFlags TimelineFramesModelBase::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = QAbstractTableModel::flags(index);
    if (!index.isValid()) return flags;

    const Layer &layer = m_d->layers[index.row()];
    int frameId = layer.frames[index.column()];

    if (frameId > 0) {
        if (index.column() > 0 &&
            data(index, FrameEditableRole).toBool()) {

            flags |= Qt::ItemIsDragEnabled;
        }
    } else {
        flags |= Qt::ItemIsDropEnabled;
    }

    return flags;
}

bool TimelineFramesModelBase::insertRows(int row, int count, const QModelIndex &parent)
{
    KIS_ASSERT_RECOVER(count == 1) { return false; }

    if (row < 0 || row > m_d->layers.size()) return false;

    Layer l;
    l.name = QString("New Layer %1").arg(m_d->layers.size());
    l.frames.resize(numFrames);

    boost::uniform_smallint<int> smallint(1,4);
    boost::mt11213b rnd;

    for (int j = 0; j < numFrames; j += smallint(rnd)) {
        l.frames[j] = j / 3;
    }
    l.visible = true;
    l.locked = false;
    l.alphaLocked = false;
    l.inheritAlpha = false;
    l.onionSkins = true;

    beginInsertRows(parent, row, row);
    m_d->layers.insert(row, l);
    endInsertRows();

    setData(index(row, 0), true, ActiveLayerRole);

    return true;
}

bool TimelineFramesModelBase::insertOtherLayer(int index, int dstRow)
{
    if (index < 0 || index >= m_d->otherLayers.size()) return false;
    if (dstRow < 0 || dstRow > m_d->layers.size()) return false;

    const Layer &l = m_d->otherLayers[index];

    beginInsertRows(QModelIndex(), dstRow, dstRow);
    m_d->layers.insert(dstRow, l);
    endInsertRows();

    setData(this->index(dstRow, 0), true, ActiveLayerRole);

    m_d->otherLayers.remove(index);

    return true;
}

bool TimelineFramesModelBase::removeRows(int row, int count, const QModelIndex &parent)
{
    KIS_ASSERT_RECOVER(count == 1) { return false; }

    if (row < 0 || row >= m_d->layers.size()) return false;

    beginRemoveRows(parent, row, row);
    m_d->layers.remove(row);
    endRemoveRows();

    return true;
}

bool TimelineFramesModelBase::hideLayer(int row)
{
    if (row < 0 || row >= m_d->layers.size()) return false;

    m_d->otherLayers.append(m_d->layers[row]);

    beginRemoveRows(QModelIndex(), row, row);
    m_d->layers.remove(row);
    endRemoveRows();

    return true;
}

bool TimelineFramesModelBase::createFrame(const QModelIndex &dstIndex)
{
    if (!dstIndex.isValid()) return false;
    Layer &l = m_d->layers[dstIndex.row()];

    if (l.frames[dstIndex.column()] != 0) return false;
    l.frames[dstIndex.column()] = l.frames.size();
    emit dataChanged(dstIndex, dstIndex);

    return true;
}

bool TimelineFramesModelBase::copyFrame(const QModelIndex &dstIndex)
{
    if (!dstIndex.isValid()) return false;
    Layer &l = m_d->layers[dstIndex.row()];

    if (l.frames[dstIndex.column()] != 0) return false;

    int newValue = l.frames.size();

    for (int i = dstIndex.column() - 1; i >= 0; i--) {
        if (l.frames[i] > 0) {
            newValue = l.frames[i];
            break;
        }
    }

    l.frames[dstIndex.column()] = newValue;
    emit dataChanged(dstIndex, dstIndex);

    return true;
}

bool TimelineFramesModelBase::removeFrame(const QModelIndex &dstIndex)
{
    if (!dstIndex.isValid()) return false;
    Layer &l = m_d->layers[dstIndex.row()];

    if (l.frames[dstIndex.column()] == 0) return false;
    l.frames[dstIndex.column()] = 0;
    emit dataChanged(dstIndex, dstIndex);

    return true;
}

