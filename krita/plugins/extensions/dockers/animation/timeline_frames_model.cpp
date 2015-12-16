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

#include "timeline_frames_model.h"
#include <QFont>
#include <QSize>
#include <QColor>
#include <QMimeData>

#include "kis_layer.h"

#include "kis_global.h"
#include "kis_debug.h"
#include "kis_image.h"
#include "kis_image_animation_interface.h"
#include "kis_undo_adapter.h"
#include "kis_node_dummies_graph.h"
#include "kis_dummies_facade_base.h"
#include "kis_signal_compressor.h"
#include "kis_keyframe_channel.h"
#include "kundo2command.h"
#include "kis_post_execution_undo_adapter.h"
#include "kis_animation_frame_cache.h"
#include "kis_animation_player.h"
#include <commands/kis_node_property_list_command.h>

#include "kis_animation_utils.h"
#include "timeline_color_scheme.h"
#include "kis_node_model.h"
#include "kis_projection_leaf.h"
#include "kis_time_range.h"

struct TimelineFramesModel::Private
{
    Private()
        : activeLayerIndex(0),
          activeFrameIndex(0),
          dummiesFacade(0),
          needFinishInsertRows(false),
          needFinishRemoveRows(false),
          numFramesOverride(0),
          updateTimer(200, KisSignalCompressor::FIRST_INACTIVE),
          parentOfRemovedNode(0),
          scrubInProgress(false),
          scrubStartFrame(-1),
          animationPlayer(0)
    {}

    QVector<bool> cachedFrames;

    int activeLayerIndex;
    int activeFrameIndex;

    KisDummiesFacadeBase *dummiesFacade;
    KisImageWSP image;
    bool needFinishInsertRows;
    bool needFinishRemoveRows;

    int numFramesOverride;

    QList<KisNodeDummy*> updateQueue;
    KisSignalCompressor updateTimer;

    KisNodeDummy* parentOfRemovedNode;
    QScopedPointer<TimelineNodeListKeeper> converter;

    bool scrubInProgress;
    int scrubStartFrame;

    KisAnimationFrameCacheSP framesCache;
    KisAnimationPlayer *animationPlayer;

    QScopedPointer<NodeManipulationInterface> nodeInterface;

    QPersistentModelIndex lastClickedIndex;

    QVariant layerName(int row) const {
        KisNodeDummy *dummy = converter->dummyFromRow(row);
        if (!dummy) return QVariant();
        return dummy->node()->name();
    }

    bool layerEditable(int row) const {
        KisNodeDummy *dummy = converter->dummyFromRow(row);
        if (!dummy) return true;
        return dummy->node()->visible() && !dummy->node()->userLocked();
    }

    bool frameExists(int row, int column) const {
        KisNodeDummy *dummy = converter->dummyFromRow(row);
        if (!dummy) return false;

        KisKeyframeChannel *content =
            dummy->node()->getKeyframeChannel(KisKeyframeChannel::Content.id());

        if (!content) return false;

        KisKeyframeSP frame = content->keyframeAt(column);
        return frame;
    }

    int baseNumFrames() const {
        KisImageAnimationInterface *i = image->animationInterface();
        if (!i) return 1;

        return i->totalLength();
    }

    int effectiveNumFrames() const {
        return qMax(baseNumFrames(), numFramesOverride);
    }

    QVariant layerProperties(int row) const {
        KisNodeDummy *dummy = converter->dummyFromRow(row);
        if (!dummy) return QVariant();

        PropertyList props = dummy->node()->sectionModelProperties();
        return QVariant::fromValue(props);
    }

    bool setLayerProperties(int row, PropertyList props) {
        KisNodeDummy *dummy = converter->dummyFromRow(row);
        if (!dummy) return false;

        KisNodePropertyListCommand::setNodePropertiesNoUndo(dummy->node(), image, props);
        return true;
    }

    bool addKeyframe(int row, int column, bool copy) {
        KisNodeDummy *dummy = converter->dummyFromRow(row);
        if (!dummy) return false;

        return KisAnimationUtils::createKeyframeLazy(image, dummy->node(), column, copy);
    }

    bool addNewLayer(int row) {
        Q_UNUSED(row);

        if (nodeInterface) {
            KisLayerSP layer = nodeInterface->addPaintLayer();
            layer->setUseInTimeline(true);
        }

        return true;
    }

    bool removeLayer(int row) {
        KisNodeDummy *dummy = converter->dummyFromRow(row);
        if (!dummy) return false;

        if (nodeInterface) {
            nodeInterface->removeNode(dummy->node());
        }

        return true;
    }

    int framesPerSecond() {
        return image->animationInterface()->framerate();
    }
};

TimelineFramesModel::TimelineFramesModel(QObject *parent)
    : ModelWithExternalNotifications(parent),
      m_d(new Private)
{
    connect(&m_d->updateTimer, SIGNAL(timeout()), SLOT(processUpdateQueue()));
}

TimelineFramesModel::~TimelineFramesModel()
{
}

void TimelineFramesModel::setFrameCache(KisAnimationFrameCacheSP cache)
{
    if (m_d->framesCache == cache) return;

    if (m_d->framesCache) {
        m_d->framesCache->disconnect(this);
    }

    m_d->framesCache = cache;

    if (m_d->framesCache) {
        connect(m_d->framesCache, SIGNAL(changed()), SLOT(slotCacheChanged()));
    }
}

void TimelineFramesModel::setAnimationPlayer(KisAnimationPlayer *player)
{
    if (m_d->animationPlayer == player) return;

    if (m_d->animationPlayer) {
        m_d->animationPlayer->disconnect(this);
    }

    m_d->animationPlayer = player;

    if (m_d->animationPlayer) {
        connect(m_d->animationPlayer, SIGNAL(sigPlaybackStopped()), SLOT(slotPlaybackStopped()));
        connect(m_d->animationPlayer, SIGNAL(sigFrameChanged()), SLOT(slotPlaybackFrameChanged()));
    }
}

void TimelineFramesModel::setNodeManipulationInterface(NodeManipulationInterface *iface)
{
    m_d->nodeInterface.reset(iface);
}

void TimelineFramesModel::setDummiesFacade(KisDummiesFacadeBase *dummiesFacade, KisImageSP image)
{
    KisDummiesFacadeBase *oldDummiesFacade = m_d->dummiesFacade;

    if (m_d->dummiesFacade) {
        m_d->image->disconnect(this);
        m_d->dummiesFacade->disconnect(this);
    }

    m_d->image = image;
    m_d->dummiesFacade = dummiesFacade;
    m_d->converter.reset();

    if (m_d->dummiesFacade) {
        m_d->converter.reset(new TimelineNodeListKeeper(this, m_d->dummiesFacade));
        connect(m_d->dummiesFacade, SIGNAL(sigDummyChanged(KisNodeDummy*)),
                SLOT(slotDummyChanged(KisNodeDummy*)));
        connect(m_d->image->animationInterface(),
                SIGNAL(sigFramerateChanged()), SLOT(slotFramerateChanged()));
        connect(m_d->image->animationInterface(),
                SIGNAL(sigTimeChanged(int)), SLOT(slotCurrentTimeChanged(int)));
        connect(m_d->image->animationInterface(),
                SIGNAL(sigFullClipRangeChanged()), SIGNAL(sigInfiniteTimelineUpdateNeeded()));
    }

    if (m_d->dummiesFacade != oldDummiesFacade) {
        reset();
    }

    if (m_d->dummiesFacade) {
        slotCurrentTimeChanged(m_d->image->animationInterface()->currentUITime());
    }
}

void TimelineFramesModel::slotDummyChanged(KisNodeDummy *dummy)
{
    if (!m_d->updateQueue.contains(dummy)) {
        m_d->updateQueue.append(dummy);
    }
    m_d->updateTimer.start();
}

void TimelineFramesModel::processUpdateQueue()
{
    Q_FOREACH (KisNodeDummy *dummy, m_d->updateQueue) {
        int row = m_d->converter->rowForDummy(dummy);

        if (row >= 0) {
            emit headerDataChanged (Qt::Vertical, row, row);
            emit dataChanged(this->index(row, 0), this->index(row, columnCount() - 1));
        }
    }
    m_d->updateQueue.clear();
}

void TimelineFramesModel::slotFramerateChanged()
{
    emit headerDataChanged(Qt::Horizontal, 0, columnCount() - 1);
}

void TimelineFramesModel::slotCurrentTimeChanged(int time)
{
    if (time != m_d->activeFrameIndex) {

        QModelIndex newIndex = index(m_d->activeLayerIndex, time);
        if (newIndex.isValid()) {
            setData(newIndex, true, ActiveFrameRole);
        } else {
            m_d->activeFrameIndex = time;
        }
    }
}

void TimelineFramesModel::slotCurrentNodeChanged(KisNodeSP node)
{
    if (!node) {
        m_d->activeLayerIndex = -1;
        return;
    }

    KisNodeDummy *dummy = m_d->dummiesFacade->dummyForNode(node);
    KIS_ASSERT_RECOVER_RETURN(dummy);

    m_d->converter->updateActiveDummy(dummy);

    const int row = m_d->converter->rowForDummy(dummy);
    if (row < 0) {
        qWarning() << "WARNING: TimelineFramesModel::slotCurrentNodeChanged: node not found!";
    }

    if (row >= 0 && m_d->activeLayerIndex != row) {
        setData(index(row, 0), true, ActiveLayerRole);
    }
}

void TimelineFramesModel::slotCacheChanged()
{
    const int numFrames = columnCount();
    m_d->cachedFrames.resize(numFrames);

    for (int i = 0; i < numFrames; i++) {
        m_d->cachedFrames[i] =
            m_d->framesCache->frameStatus(i) == KisAnimationFrameCache::Cached;
    }

    emit headerDataChanged(Qt::Horizontal, 0, numFrames);
}

int TimelineFramesModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    if(!m_d->dummiesFacade) return 0;

    return m_d->converter->rowCount();
}

int TimelineFramesModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    if(!m_d->dummiesFacade) return 0;

    return m_d->effectiveNumFrames();
}

QVariant TimelineFramesModel::data(const QModelIndex &index, int role) const
{
    if(!m_d->dummiesFacade) return QVariant();

    switch (role) {
    case ActiveLayerRole: {
        return index.row() == m_d->activeLayerIndex;
    }
    case ActiveFrameRole: {
        return index.column() == m_d->activeFrameIndex;
    }
    case FrameEditableRole: {
        return m_d->layerEditable(index.row());
    }
    case FrameExistsRole: {
        return m_d->frameExists(index.row(), index.column());
    }
    case Qt::DisplayRole: {
        return QVariant();
    }
    case Qt::TextAlignmentRole: {
        return QVariant(Qt::AlignHCenter | Qt::AlignVCenter);
    }
    }


    return QVariant();
}

bool TimelineFramesModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || !m_d->dummiesFacade) return false;

    switch (role) {
    case ActiveLayerRole: {
        if (value.toBool() &&
            index.row() != m_d->activeLayerIndex) {

            int prevLayer = m_d->activeLayerIndex;
            m_d->activeLayerIndex = index.row();

            emit dataChanged(this->index(prevLayer, 0), this->index(prevLayer, columnCount() - 1));
            emit dataChanged(this->index(m_d->activeLayerIndex, 0), this->index(m_d->activeLayerIndex, columnCount() - 1));

            emit headerDataChanged(Qt::Vertical, prevLayer, prevLayer);
            emit headerDataChanged(Qt::Vertical, m_d->activeLayerIndex, m_d->activeLayerIndex);

            KisNodeDummy *dummy = m_d->converter->dummyFromRow(m_d->activeLayerIndex);
            KIS_ASSERT_RECOVER(dummy) { return true; }

            emit requestCurrentNodeChanged(dummy->node());
        }
        break;
    }
    case ActiveFrameRole: {
        if (value.toBool() &&
            index.column() != m_d->activeFrameIndex) {

            int prevFrame = m_d->activeFrameIndex;
            m_d->activeFrameIndex = index.column();

            scrubTo(m_d->activeFrameIndex, m_d->scrubInProgress);


            /**
             * Optimization Hack Alert:
             *
             * ideally, we should emit all four signals, but... The
             * point is this code is used in a tight loop during
             * playback, so it should run as fast as possible. To tell
             * the story short, commenting out these three lines makes
             * playback run 15% faster ;)
             */

            if (m_d->scrubInProgress) {
                //emit dataChanged(this->index(0, prevFrame), this->index(rowCount() - 1, prevFrame));
                emit dataChanged(this->index(0, m_d->activeFrameIndex), this->index(rowCount() - 1, m_d->activeFrameIndex));
                //emit headerDataChanged (Qt::Horizontal, prevFrame, prevFrame);
                //emit headerDataChanged (Qt::Horizontal, m_d->activeFrameIndex, m_d->activeFrameIndex);
            } else {
                emit dataChanged(this->index(0, prevFrame), this->index(rowCount() - 1, prevFrame));
                emit dataChanged(this->index(0, m_d->activeFrameIndex), this->index(rowCount() - 1, m_d->activeFrameIndex));
                emit headerDataChanged (Qt::Horizontal, prevFrame, prevFrame);
                emit headerDataChanged (Qt::Horizontal, m_d->activeFrameIndex, m_d->activeFrameIndex);
            }
        }
        break;
    }
    }

    return ModelWithExternalNotifications::setData(index, value, role);
}

QVariant TimelineFramesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(!m_d->dummiesFacade) return QVariant();

    if (orientation == Qt::Horizontal) {
        switch (role) {
        case ActiveFrameRole:
            return section == m_d->activeFrameIndex;
        case FrameCachedRole:
            return m_d->cachedFrames.size() > section ? m_d->cachedFrames[section] : false;
        case TimelineFramesModel::FramesPerSecondRole:
            return m_d->framesPerSecond();
        }

    } else {
        switch (role) {
        case ActiveLayerRole:
            return section == m_d->activeLayerIndex;
        case Qt::DisplayRole: {
            QVariant value = headerData(section, orientation, Qt::ToolTipRole);
            if (!value.isValid()) return value;

            QString name = value.toString();
            const int maxNameSize = 13;

            if (name.size() > maxNameSize) {
                name = QString("%1...").arg(name.left(maxNameSize));
            }

            return name;
        }
        case Qt::TextColorRole: {
            // WARNING: this role doesn't work for header views! Use
            //          bold font to show isolated mode instead!
            return QVariant();
        }
        case Qt::FontRole: {
            KisNodeDummy *dummy = m_d->converter->dummyFromRow(section);
            if (!dummy) return QVariant();
            KisNodeSP node = dummy->node();

            QFont baseFont;
            if (node->projectionLeaf()->isDroppedMask()) {
                baseFont.setStrikeOut(true);
            } else if (m_d->image->isolatedModeRoot() &&
                       KisNodeModel::belongsToIsolatedGroup(m_d->image, node, m_d->dummiesFacade)) {
                baseFont.setBold(true);
            }
            return baseFont;
        }
        case Qt::ToolTipRole: {
            return m_d->layerName(section);
        }
        case TimelinePropertiesRole: {
            return QVariant::fromValue(m_d->layerProperties(section));
        }
        case OtherLayersRole: {
            TimelineNodeListKeeper::OtherLayersList list =
                m_d->converter->otherLayersList();

            return QVariant::fromValue(list);
        }
        case LayerUsedInTimelineRole: {
            KisNodeDummy *dummy = m_d->converter->dummyFromRow(section);
            if (!dummy) return QVariant();
            return dummy->node()->useInTimeline();
        }
        }
    }

    return QVariant();
}

bool TimelineFramesModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
    if (!m_d->dummiesFacade) return false;

    if (orientation == Qt::Horizontal) {
        switch (role) {
        case ActiveFrameRole: {
            setData(index(0, section), value, role);
            break;
        }
        }
    } else {
        switch (role) {
        case ActiveLayerRole: {
            setData(index(section, 0), value, role);
            break;
        }
        case TimelinePropertiesRole: {
            TimelineFramesModel::PropertyList props = value.value<TimelineFramesModel::PropertyList>();

            int result = m_d->setLayerProperties(section, props);
            emit headerDataChanged (Qt::Vertical, section, section);
            return result;
        }
        case LayerUsedInTimelineRole: {
            KisNodeDummy *dummy = m_d->converter->dummyFromRow(section);
            if (!dummy) return false;
            dummy->node()->setUseInTimeline(value.toBool());
            return true;
        }
        }
    }

    return ModelWithExternalNotifications::setHeaderData(section, orientation, value, role);
}

Qt::DropActions TimelineFramesModel::supportedDragActions() const
{
    return Qt::MoveAction | Qt::CopyAction;
}

Qt::DropActions TimelineFramesModel::supportedDropActions() const
{
    return Qt::MoveAction | Qt::CopyAction;
}

QStringList TimelineFramesModel::mimeTypes() const
{
    QStringList types;
    types << QLatin1String("application/x-krita-frame");
    return types;
}

void TimelineFramesModel::setLastClickedIndex(const QModelIndex &index)
{
    m_d->lastClickedIndex = index;
}

QMimeData* TimelineFramesModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *data = new QMimeData();

    QByteArray encoded;
    QDataStream stream(&encoded, QIODevice::WriteOnly);

    const int baseRow = m_d->lastClickedIndex.row();
    const int baseColumn = m_d->lastClickedIndex.column();

    stream << indexes.size();
    stream << baseRow << baseColumn;

    Q_FOREACH (const QModelIndex &index, indexes) {
        stream << index.row() - baseRow << index.column() - baseColumn;
    }

    data->setData("application/x-krita-frame", encoded);

    return data;
}

inline void decodeBaseIndex(QByteArray *encoded, int *row, int *col)
{
    int size_UNUSED = 0;

    QDataStream stream(encoded, QIODevice::ReadOnly);
    stream >> size_UNUSED >> *row >> *col;
}

bool TimelineFramesModel::canDropFrameData(const QMimeData *data, const QModelIndex &index)
{
    if (!index.isValid()) return false;

    /**
     * Now we support D&D around any layer, so just return 'true' all
     * the time.
     */
    return true;
}

bool TimelineFramesModel::offsetFrames(QVector<QPoint> srcIndexes, const QPoint &offset, bool copyFrames)
{
    bool result = false;
    if (srcIndexes.isEmpty()) return result;
    if (offset.isNull()) return result;

    KisAnimationUtils::sortPointsForSafeMove(&srcIndexes, offset);

    KisAnimationUtils::FrameItemList srcFrameItems;
    KisAnimationUtils::FrameItemList dstFrameItems;
    QModelIndexList updateIndexes;

    Q_FOREACH (const QPoint &point, srcIndexes) {
        int srcRow = point.y();
        int srcColumn = point.x();

        int dstRow = point.y() + offset.y();
        int dstColumn = point.x() + offset.x();

        KisNodeDummy *srcDummy = m_d->converter->dummyFromRow(srcRow);
        KisNodeDummy *dstDummy = m_d->converter->dummyFromRow(dstRow);
        if (!srcDummy || !dstDummy) continue;

        srcFrameItems << KisAnimationUtils::FrameItem(srcDummy->node(), srcColumn);
        dstFrameItems << KisAnimationUtils::FrameItem(dstDummy->node(), dstColumn);

        if (!copyFrames) {
            updateIndexes << index(srcRow, srcColumn);
        }

        updateIndexes << index(dstRow, dstColumn);
    }

    result = KisAnimationUtils::moveKeyframes(m_d->image,
                                              srcFrameItems,
                                              dstFrameItems,
                                              copyFrames);

    Q_FOREACH (const QModelIndex &index, updateIndexes) {
        emit dataChanged(index, index);
    }

    return result;
}

bool TimelineFramesModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    Q_UNUSED(row);
    Q_UNUSED(column);

    bool result = false;

    if ((action != Qt::MoveAction &&
         action != Qt::CopyAction) || !parent.isValid()) return result;

    const bool copyFrames = action == Qt::CopyAction;

    QByteArray encoded = data->data("application/x-krita-frame");
    QDataStream stream(&encoded, QIODevice::ReadOnly);


    int size, baseRow, baseColumn;
    stream >> size >> baseRow >> baseColumn;

    QVector<QPoint> srcIndexes;

    for (int i = 0; i < size; i++) {
        int relRow, relColumn;
        stream >> relRow >> relColumn;

        int srcRow = baseRow + relRow;
        int srcColumn = baseColumn + relColumn;

        srcIndexes << QPoint(srcColumn, srcRow);
    }

    const QPoint offset(parent.column() - baseColumn, parent.row() - baseRow);

    return offsetFrames(srcIndexes, offset, copyFrames);
}

Qt::ItemFlags TimelineFramesModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = ModelWithExternalNotifications::flags(index);
    if (!index.isValid()) return flags;

    if (m_d->frameExists(index.row(), index.column())) {
        if (data(index, FrameEditableRole).toBool()) {
            flags |= Qt::ItemIsDragEnabled;
        }
    }

    /**
     * Basically we should forbid overrides only if we D&D a single frame
     * and allow it when we D&D multiple frames. But we cannot distinguish
     * it here... So allow all the time.
     */
    flags |= Qt::ItemIsDropEnabled;

    return flags;
}

bool TimelineFramesModel::insertRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(parent);

    KIS_ASSERT_RECOVER(count == 1) { return false; }

    if (row < 0 || row > rowCount()) return false;

    bool result = m_d->addNewLayer(row);
    return result;
}

bool TimelineFramesModel::removeRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(parent);
    KIS_ASSERT_RECOVER(count == 1) { return false; }

    if (row < 0 || row >= rowCount()) return false;

    bool result = m_d->removeLayer(row);
    return result;
}

bool TimelineFramesModel::insertOtherLayer(int index, int dstRow)
{
    Q_UNUSED(dstRow);

    TimelineNodeListKeeper::OtherLayersList list =
        m_d->converter->otherLayersList();

    if (index < 0 || index >= list.size()) return false;

    list[index].dummy->node()->setUseInTimeline(true);
    dstRow = m_d->converter->rowForDummy(list[index].dummy);
    setData(this->index(dstRow, 0), true, ActiveLayerRole);

    return true;
}

int TimelineFramesModel::activeLayerRow() const
{
    return m_d->activeLayerIndex;
}

bool TimelineFramesModel::createFrame(const QModelIndex &dstIndex)
{
    if (!dstIndex.isValid()) return false;

    bool result = m_d->addKeyframe(dstIndex.row(), dstIndex.column(), false);
    if (result) {
        emit dataChanged(dstIndex, dstIndex);
    }

    return result;
}

bool TimelineFramesModel::copyFrame(const QModelIndex &dstIndex)
{
    if (!dstIndex.isValid()) return false;

    bool result = m_d->addKeyframe(dstIndex.row(), dstIndex.column(), true);
    if (result) {
        emit dataChanged(dstIndex, dstIndex);
    }

    return result;
}

bool TimelineFramesModel::removeFrames(const QModelIndexList &indexes)
{
    KisAnimationUtils::FrameItemList frameItems;

    Q_FOREACH (const QModelIndex &index, indexes) {
        KisNodeDummy *dummy = m_d->converter->dummyFromRow(index.row());
        if (!dummy) continue;

        frameItems << KisAnimationUtils::FrameItem(dummy->node(), index.column());
    }

    if (frameItems.isEmpty()) return false;


    KisAnimationUtils::removeKeyframes(m_d->image, frameItems);

    Q_FOREACH (const QModelIndex &index, indexes) {
        if (index.isValid()) {
            emit dataChanged(index, index);
        }
    }

    return true;
}

void TimelineFramesModel::setLastVisibleFrame(int time)
{
    if (!m_d->dummiesFacade) return;

    const int growThreshold = m_d->effectiveNumFrames() - 3;
    const int growValue = time + 8;

    const int shrinkThreshold = m_d->effectiveNumFrames() - 12;
    const int shrinkValue = qMax(m_d->baseNumFrames(), qMin(growValue, shrinkThreshold));
    const bool canShrink = m_d->effectiveNumFrames() > m_d->baseNumFrames();

    if (time >= growThreshold) {
        beginInsertColumns(QModelIndex(), m_d->effectiveNumFrames(), growValue - 1);
        m_d->numFramesOverride = growValue;
        endInsertColumns();
    } else if (time < shrinkThreshold && canShrink) {
        beginRemoveColumns(QModelIndex(), shrinkValue, m_d->effectiveNumFrames() - 1);
        m_d->numFramesOverride = shrinkValue;
        endRemoveColumns();
    }
}

void TimelineFramesModel::setScrubState(bool active)
{
    if (!m_d->scrubInProgress && active) {
        m_d->scrubStartFrame = m_d->activeFrameIndex;
        m_d->scrubInProgress = true;
    }

    if (m_d->scrubInProgress && !active) {

        m_d->scrubInProgress = false;

        if (m_d->scrubStartFrame >= 0 &&
            m_d->scrubStartFrame != m_d->activeFrameIndex) {

            scrubTo(m_d->activeFrameIndex, false);
        }

        m_d->scrubStartFrame = -1;
    }
}

void TimelineFramesModel::scrubTo(int time, bool preview)
{
    if (m_d->animationPlayer && m_d->animationPlayer->isPlaying()) return;

    KIS_ASSERT_RECOVER_RETURN(m_d->image);

    if (preview) {
        if (m_d->animationPlayer) {
            m_d->animationPlayer->displayFrame(time);
        }
    } else {
        m_d->image->animationInterface()->requestTimeSwitchWithUndo(time);
    }
}

void TimelineFramesModel::slotPlaybackFrameChanged()
{
    if (!m_d->animationPlayer->isPlaying()) return;
    setData(index(0, m_d->animationPlayer->currentTime()), true, ActiveFrameRole);
}

void TimelineFramesModel::slotPlaybackStopped()
{
    setData(index(0, m_d->image->animationInterface()->currentUITime()), true, ActiveFrameRole);
}

void TimelineFramesModel::setPlaybackRange(const KisTimeRange &range)
{
    if (!m_d->dummiesFacade) return;

    KisImageAnimationInterface *i = m_d->image->animationInterface();
    i->setPlaybackRange(range);
}

bool TimelineFramesModel::isPlaybackActive() const
{
    return m_d->animationPlayer && m_d->animationPlayer->isPlaying();
}
