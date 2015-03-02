/*
 *  Copyright (c) 2015 Jouni Pentikäinen <mctyyppi42@gmail.com>
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

#include "kis_timeline_model.h"
#include "kis_model_index_converter_base.h"
#include "kis_keyframe_channel.h"

struct KisTimelineModel::Private
{
    KisImageWSP image;

    // A placeholder for the combined channel of a layer, until the feature is implemented
    KisKeyframeChannel *dummyChannel;
};

KisTimelineModel::KisTimelineModel(QObject *parent)
    : KisNodeModel(parent)
    , m_d(new Private())
{
    m_d->dummyChannel = new KisKeyframeChannel("", "");
}

KisTimelineModel::~KisTimelineModel()
{
    delete m_d;
}

QModelIndex KisTimelineModel::index(int row, int column, const QModelIndex &parent) const
{
    QObject *parentObj = static_cast<QObject*>(parent.internalPointer());

    if (!parentObj) {
        if (column == 0) {
            return KisNodeModel::index(row, column, parent);
        } else {
            return createIndex(row, column, m_d->dummyChannel);
        }
    } else if (parentObj->inherits("KisKeyframe")) {
        return QModelIndex();
    } else if (parentObj->inherits("KisKeyframeChannel")) {
        if (parent.column() == 0) {
            return QModelIndex();
        } else {
            KisKeyframeChannel *channel = qobject_cast<KisKeyframeChannel*>(parentObj);

            KisKeyframe *keyframe = channel->keyframes().at(column);
            return createIndex(row, column, keyframe);
        }
    } else { // KisNodeDummy
        int childNodeCount = indexConverter()->rowCount(parent);
        if (row >= childNodeCount) {
            int channelIndex = row - childNodeCount;

            KisNodeSP parentNode = nodeFromIndex(parent);
            KisKeyframeChannel *channel = parentNode->keyframes()->channels().at(channelIndex);

            return createIndex(row, column, channel);
        }

        if (column == 0) {
            return KisNodeModel::index(row, column, parent);
        } else {
            return createIndex(row, column, m_d->dummyChannel);
        }
    }
}

QModelIndex KisTimelineModel::parent(const QModelIndex &child) const
{
    QObject *childObj = static_cast<QObject*>(child.internalPointer());

    if (!childObj) {
        return QModelIndex();
    } else if (childObj->inherits("KisKeyframe")) {
        KisKeyframe *keyframe = qobject_cast<KisKeyframe*>(childObj);
        KisKeyframeChannel *channel = keyframe->channel();

        int row = channel->sequence()->node()->childCount() + channel->sequence()->channels().indexOf(channel);
        return createIndex(row, 1, channel);
    } else if (childObj->inherits("KisKeyframeChannel")) {
        KisKeyframeChannel *channel = qobject_cast<KisKeyframeChannel*>(childObj);

        KisNodeWSP node = channel->sequence()->node();

        return KisNodeModel::indexFromNode(node);
    } else { // KisNodeDummy
        return KisNodeModel::parent(child);
    }
}

int KisTimelineModel::rowCount(const QModelIndex &parent) const
{
    QObject *parentObj = static_cast<QObject*>(parent.internalPointer());

    if (!parentObj) {
        return KisNodeModel::rowCount(parent);
    } else if (parentObj->inherits("KisKeyframe")) {
        return 0;
    } else if (parentObj->inherits("KisKeyframeChannel")) {
        return (parent.column() == 0) ? 0 : 1;
    } else { // KisNodeDummy
        KisNodeSP parentNode = nodeFromIndex(parent);
        int channelCount = parentNode->keyframes()->channels().count();
        return KisNodeModel::rowCount(parent) + channelCount;
    }
}

int KisTimelineModel::columnCount(const QModelIndex &parent) const
{
    QObject *parentObj = static_cast<QObject*>(parent.internalPointer());

    if (!parentObj) {
        return 2;
    } else if (parentObj->inherits("KisKeyframe")) {
        return 0;
    } else if (parentObj && parentObj->inherits("KisKeyframeChannel")) {
        if (parent.column() == 0) {
            return 0;
        } else {
            KisKeyframeChannel *channel = qobject_cast<KisKeyframeChannel*>(parentObj);
            return channel->keyframes().count();
        }
    } else { // KisNodeDummy
        return 2;
    }
}

QVariant KisTimelineModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::SizeHintRole) return QSize(42, 16); // TODO: good values?

    QObject *obj = static_cast<QObject*>(index.internalPointer());

    if (obj->inherits("KisKeyframe")) {
        KisKeyframe *keyframe = qobject_cast<KisKeyframe*>(obj);
        switch (role) {
        case KisTimelineModel::TimeRole: return keyframe->time();
        }
        return QVariant();
    } else if (obj->inherits("KisKeyframeChannel")) {
        KisKeyframeChannel *channel = qobject_cast<KisKeyframeChannel*>(obj);
        switch (role) {
        case Qt::DisplayRole: return channel->displayName();
        }
        return QVariant();
    } else { // KisNodeDummy
        return KisNodeModel::data(index, role);
    }
}

bool KisTimelineModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    QObject *obj = static_cast<QObject*>(index.internalPointer());
    if (!obj) return false;

    if (obj->inherits("KisKeyframe")) {
        KisKeyframe *keyframe = qobject_cast<KisKeyframe*>(obj);

        switch (role) {
        case KisTimelineModel::TimeRole:
            return keyframe->channel()->moveKeyframe(keyframe, value.toInt());
        }
    }

    return false;
}

Qt::ItemFlags KisTimelineModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) return Qt::NoItemFlags;

    QObject *obj = static_cast<QObject*>(index.internalPointer());

    if (obj->inherits("KisKeyframe")) {
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    } else if (obj->inherits("KisKeyframeChannel")) {
        return Qt::ItemIsEnabled;
    } else {
        return Qt::ItemIsEnabled;
    }
}

#include "kis_timeline_model.moc"
