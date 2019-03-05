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
#include <QPointer>
#include <KoResourceModel.h>

#include "kis_layer.h"
#include "kis_config.h"

#include "kis_global.h"
#include "kis_debug.h"
#include "kis_image.h"
#include "kis_image_animation_interface.h"
#include "kis_undo_adapter.h"
#include "kis_node_dummies_graph.h"
#include "kis_dummies_facade_base.h"
#include "kis_signal_compressor.h"
#include "kis_signal_compressor_with_param.h"
#include "kis_keyframe_channel.h"
#include "kundo2command.h"
#include "kis_post_execution_undo_adapter.h"
#include <commands/kis_node_property_list_command.h>
#include <commands_new/kis_switch_current_time_command.h>

#include "kis_animation_utils.h"
#include "timeline_color_scheme.h"
#include "kis_node_model.h"
#include "kis_projection_leaf.h"
#include "kis_time_range.h"

#include "kis_node_view_color_scheme.h"
#include "krita_utils.h"
#include <QApplication>
#include "kis_processing_applicator.h"
#include <KisImageBarrierLockerWithFeedback.h>
#include "kis_node_uuid_info.h"


struct TimelineFramesModel::Private
{
    Private()
        : activeLayerIndex(0),
          dummiesFacade(0),
          needFinishInsertRows(false),
          needFinishRemoveRows(false),
          updateTimer(200, KisSignalCompressor::FIRST_INACTIVE),
          parentOfRemovedNode(0)
    {}

    int activeLayerIndex;

    QPointer<KisDummiesFacadeBase> dummiesFacade;
    KisImageWSP image;
    bool needFinishInsertRows;
    bool needFinishRemoveRows;

    QList<KisNodeDummy*> updateQueue;
    KisSignalCompressor updateTimer;

    KisNodeDummy* parentOfRemovedNode;
    QScopedPointer<TimelineNodeListKeeper> converter;

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

        KisKeyframeChannel *primaryChannel = dummy->node()->getKeyframeChannel(KisKeyframeChannel::Content.id());
        return (primaryChannel && primaryChannel->keyframeAt(column));
    }

    bool frameHasContent(int row, int column) {

        KisNodeDummy *dummy = converter->dummyFromRow(row);

        KisKeyframeChannel *primaryChannel = dummy->node()->getKeyframeChannel(KisKeyframeChannel::Content.id());
        if (!primaryChannel) return false;

        // first check if we are a key frame
        KisKeyframeSP frame = primaryChannel->activeKeyframeAt(column);
        if (!frame) return false;

        return frame->hasContent();
    }

    bool specialKeyframeExists(int row, int column) {
        KisNodeDummy *dummy = converter->dummyFromRow(row);
        if (!dummy) return false;
        Q_FOREACH(KisKeyframeChannel *channel, dummy->node()->keyframeChannels()) {
            if (channel->id() != KisKeyframeChannel::Content.id() && channel->keyframeAt(column)) {
                return true;
            }
        }
        return false;
    }

    int frameColorLabel(int row, int column) {
        KisNodeDummy *dummy = converter->dummyFromRow(row);
        if (!dummy) return -1;

        KisKeyframeChannel *primaryChannel = dummy->node()->getKeyframeChannel(KisKeyframeChannel::Content.id());
        if (!primaryChannel) return -1;

        KisKeyframeSP frame = primaryChannel->activeKeyframeAt(column);
        if (!frame) return -1;

        return frame->colorLabel();
    }

    void setFrameColorLabel(int row, int column, int color) {
        KisNodeDummy *dummy = converter->dummyFromRow(row);
        if (!dummy) return;

        KisKeyframeChannel *primaryChannel = dummy->node()->getKeyframeChannel(KisKeyframeChannel::Content.id());
        if (!primaryChannel) return;

        KisKeyframeSP frame = primaryChannel->keyframeAt(column);
        if (!frame) return;

        frame->setColorLabel(color);
    }

    int layerColorLabel(int row) const {
        KisNodeDummy *dummy = converter->dummyFromRow(row);
        if (!dummy) return -1;
        return dummy->node()->colorLabelIndex();
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

        nodeInterface->setNodeProperties(dummy->node(), image, props);
        return true;
    }

    bool addKeyframe(int row, int column, bool copy) {
        KisNodeDummy *dummy = converter->dummyFromRow(row);
        if (!dummy) return false;

        KisNodeSP node = dummy->node();
        if (!KisAnimationUtils::supportsContentFrames(node)) return false;

        KisAnimationUtils::createKeyframeLazy(image, node, KisKeyframeChannel::Content.id(), column, copy);
        return true;
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

bool TimelineFramesModel::hasConnectionToCanvas() const
{
    return m_d->dummiesFacade;
}

void TimelineFramesModel::setNodeManipulationInterface(NodeManipulationInterface *iface)
{
    m_d->nodeInterface.reset(iface);
}

KisNodeSP TimelineFramesModel::nodeAt(QModelIndex index) const
{
    /**
     * The dummy might not exist because the user could (quickly) change
     * active layer and the list of the nodes in m_d->converter will change.
     */
    KisNodeDummy *dummy = m_d->converter->dummyFromRow(index.row());
    return dummy ? dummy->node() : 0;
}

QMap<QString, KisKeyframeChannel*> TimelineFramesModel::channelsAt(QModelIndex index) const
{
    KisNodeDummy *srcDummy = m_d->converter->dummyFromRow(index.row());
    return srcDummy->node()->keyframeChannels();
}

void TimelineFramesModel::setDummiesFacade(KisDummiesFacadeBase *dummiesFacade, KisImageSP image)
{
    KisDummiesFacadeBase *oldDummiesFacade = m_d->dummiesFacade;

    if (m_d->dummiesFacade && m_d->image) {
        m_d->image->animationInterface()->disconnect(this);
        m_d->image->disconnect(this);
        m_d->dummiesFacade->disconnect(this);
    }

    m_d->image = image;
    KisTimeBasedItemModel::setImage(image);

    m_d->dummiesFacade = dummiesFacade;
    m_d->converter.reset();

    if (m_d->dummiesFacade) {
        m_d->converter.reset(new TimelineNodeListKeeper(this, m_d->dummiesFacade));
        connect(m_d->dummiesFacade, SIGNAL(sigDummyChanged(KisNodeDummy*)),
                SLOT(slotDummyChanged(KisNodeDummy*)));
        connect(m_d->image->animationInterface(),
                SIGNAL(sigFullClipRangeChanged()), SIGNAL(sigInfiniteTimelineUpdateNeeded()));
        connect(m_d->image->animationInterface(),
                SIGNAL(sigAudioChannelChanged()), SIGNAL(sigAudioChannelChanged()));
        connect(m_d->image->animationInterface(),
                SIGNAL(sigAudioVolumeChanged()), SIGNAL(sigAudioChannelChanged()));
        connect(m_d->image, SIGNAL(sigImageModified()), SLOT(slotImageContentChanged()));
    }

    if (m_d->dummiesFacade != oldDummiesFacade) {
        beginResetModel();
        endResetModel();
    }

    if (m_d->dummiesFacade) {
        emit sigInfiniteTimelineUpdateNeeded();
        emit sigAudioChannelChanged();
    }
}

void TimelineFramesModel::slotDummyChanged(KisNodeDummy *dummy)
{
    if (!m_d->updateQueue.contains(dummy)) {
        m_d->updateQueue.append(dummy);
    }
    m_d->updateTimer.start();
}

void TimelineFramesModel::slotImageContentChanged()
{
    if (m_d->activeLayerIndex < 0) return;

    KisNodeDummy *dummy = m_d->converter->dummyFromRow(m_d->activeLayerIndex);
    if (!dummy) return;

    slotDummyChanged(dummy);
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

void TimelineFramesModel::slotCurrentNodeChanged(KisNodeSP node)
{
    if (!node) {
        m_d->activeLayerIndex = -1;
        return;
    }

    KisNodeDummy *dummy = m_d->dummiesFacade->dummyForNode(node);
    if (!dummy) {
        // It's perfectly normal that dummyForNode returns 0; that happens
        // when views get activated while Krita is closing down.
        return;
    }

    m_d->converter->updateActiveDummy(dummy);

    const int row = m_d->converter->rowForDummy(dummy);
    if (row < 0) {
        qWarning() << "WARNING: TimelineFramesModel::slotCurrentNodeChanged: node not found!";
    }

    if (row >= 0 && m_d->activeLayerIndex != row) {
        setData(index(row, 0), true, ActiveLayerRole);
    }
}

int TimelineFramesModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    if(!m_d->dummiesFacade) return 0;

    return m_d->converter->rowCount();
}

QVariant TimelineFramesModel::data(const QModelIndex &index, int role) const
{
    if(!m_d->dummiesFacade) return QVariant();

    switch (role) {
    case ActiveLayerRole: {
        return index.row() == m_d->activeLayerIndex;
    }
    case FrameEditableRole: {
        return m_d->layerEditable(index.row());
    }
    case FrameHasContent: {
        return m_d->frameHasContent(index.row(), index.column());
    }
    case FrameExistsRole: {
        return m_d->frameExists(index.row(), index.column());
    }
    case SpecialKeyframeExists: {
        return m_d->specialKeyframeExists(index.row(), index.column());
    }
    case FrameColorLabelIndexRole: {
        int label = m_d->frameColorLabel(index.row(), index.column());
        return label > 0 ? label : QVariant();
    }
    case Qt::DisplayRole: {
        return m_d->layerName(index.row());
    }
    case Qt::TextAlignmentRole: {
        return QVariant(Qt::AlignHCenter | Qt::AlignVCenter);
    }
    case KoResourceModel::LargeThumbnailRole: {
        KisNodeDummy *dummy = m_d->converter->dummyFromRow(index.row());
        if (!dummy) {
            return  QVariant();
        }
        const int maxSize = 200;

        QSize size = dummy->node()->extent().size();
        size.scale(maxSize, maxSize, Qt::KeepAspectRatio);
        if (size.width() == 0 || size.height() == 0) {
            // No thumbnail can be shown if there isn't width or height...
            return QVariant();
        }
        QImage image(dummy->node()->createThumbnailForFrame(size.width(), size.height(), index.column()));
        return image;
    }
    }

    return ModelWithExternalNotifications::data(index, role);
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
            emit sigEnsureRowVisible(m_d->activeLayerIndex);
        }
        break;
    }
    case FrameColorLabelIndexRole: {
        m_d->setFrameColorLabel(index.row(), index.column(), value.toInt());
    }
        break;
    }

    return ModelWithExternalNotifications::setData(index, value, role);
}

QVariant TimelineFramesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(!m_d->dummiesFacade) return QVariant();

    if (orientation == Qt::Vertical) {
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
            } else if (m_d->image && m_d->image->isolatedModeRoot() &&
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
        case Qt::BackgroundRole: {
            int label = m_d->layerColorLabel(section);
            if (label > 0) {
                KisNodeViewColorScheme scm;
                QColor color = scm.colorLabel(label);
                QPalette pal = qApp->palette();
                color = KritaUtils::blendColors(color, pal.color(QPalette::Button), 0.3);
                return QBrush(color);
            } else {
                return QVariant();
            }
        }
        }
    }

    return ModelWithExternalNotifications::headerData(section, orientation, role);
}

bool TimelineFramesModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
    if (!m_d->dummiesFacade) return false;

    if (orientation == Qt::Vertical) {
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
    return mimeDataExtended(indexes, m_d->lastClickedIndex, UndefinedPolicy);
}

QMimeData *TimelineFramesModel::mimeDataExtended(const QModelIndexList &indexes,
                                                 const QModelIndex &baseIndex,
                                                 TimelineFramesModel::MimeCopyPolicy copyPolicy) const
{
    QMimeData *data = new QMimeData();

    QByteArray encoded;
    QDataStream stream(&encoded, QIODevice::WriteOnly);

    const int baseRow = baseIndex.row();
    const int baseColumn = baseIndex.column();

    stream << indexes.size();
    stream << baseRow << baseColumn;

    Q_FOREACH (const QModelIndex &index, indexes) {
        KisNodeSP node = nodeAt(index);
        KIS_SAFE_ASSERT_RECOVER(node) { continue; }

        stream << index.row() - baseRow << index.column() - baseColumn;

        const QByteArray uuidData = node->uuid().toRfc4122();
        stream << int(uuidData.size());
        stream.writeRawData(uuidData.data(), uuidData.size());
    }

    stream << int(copyPolicy);
    data->setData("application/x-krita-frame", encoded);

    return data;
}

inline void decodeBaseIndex(QByteArray *encoded, int *row, int *col)
{
    int size_UNUSED = 0;

    QDataStream stream(encoded, QIODevice::ReadOnly);
    stream >> size_UNUSED >> *row >> *col;
}

bool TimelineFramesModel::canDropFrameData(const QMimeData */*data*/, const QModelIndex &index)
{
    if (!index.isValid()) return false;

    /**
     * Now we support D&D around any layer, so just return 'true' all
     * the time.
     */
    return true;
}

bool TimelineFramesModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    Q_UNUSED(row);
    Q_UNUSED(column);

    return dropMimeDataExtended(data, action, parent);
}

bool TimelineFramesModel::dropMimeDataExtended(const QMimeData *data, Qt::DropAction action, const QModelIndex &parent, bool *dataMoved)
{
    bool result = false;

    if ((action != Qt::MoveAction && action != Qt::CopyAction) ||
        !parent.isValid()) return result;

    QByteArray encoded = data->data("application/x-krita-frame");
    QDataStream stream(&encoded, QIODevice::ReadOnly);

    int size, baseRow, baseColumn;
    stream >> size >> baseRow >> baseColumn;

    const QPoint offset(parent.column() - baseColumn, parent.row() - baseRow);

    KisAnimationUtils::FrameMovePairList frameMoves;

    for (int i = 0; i < size; i++) {
        int relRow, relColumn;
        stream >> relRow >> relColumn;

        const int srcRow = baseRow + relRow;
        const int srcColumn = baseColumn + relColumn;

        int uuidLen = 0;
        stream >> uuidLen;
        QByteArray uuidData(uuidLen, '\0');
        stream.readRawData(uuidData.data(), uuidLen);
        QUuid nodeUuid = QUuid::fromRfc4122(uuidData);

        KisNodeSP srcNode;

        if (!nodeUuid.isNull()) {
            KisNodeUuidInfo nodeInfo(nodeUuid);
            srcNode = nodeInfo.findNode(m_d->image->root());
        } else {
            QModelIndex index = this->index(srcRow, srcColumn);
            srcNode = nodeAt(index);
        }

        KIS_SAFE_ASSERT_RECOVER(srcNode) { continue; }

        const QModelIndex dstRowIndex = this->index(srcRow + offset.y(), 0);
        if (!dstRowIndex.isValid()) continue;

        KisNodeSP dstNode = nodeAt(dstRowIndex);
        KIS_SAFE_ASSERT_RECOVER(dstNode) { continue; }

        Q_FOREACH (KisKeyframeChannel *channel, srcNode->keyframeChannels().values()) {
            KisAnimationUtils::FrameItem srcItem(srcNode, channel->id(), srcColumn);
            KisAnimationUtils::FrameItem dstItem(dstNode, channel->id(), srcColumn + offset.x());
            frameMoves << std::make_pair(srcItem, dstItem);
        }
    }

    MimeCopyPolicy copyPolicy = UndefinedPolicy;

    if (!stream.atEnd()) {
        int value = 0;
        stream >> value;
        copyPolicy = MimeCopyPolicy(value);
    }

    const bool copyFrames =
        copyPolicy == UndefinedPolicy ?
        action == Qt::CopyAction :
        copyPolicy == CopyFramesPolicy;

    if (dataMoved) {
        *dataMoved = !copyFrames;
    }

    KUndo2Command *cmd = 0;

    if (!frameMoves.isEmpty()) {
        KisImageBarrierLockerWithFeedback locker(m_d->image);
        cmd = KisAnimationUtils::createMoveKeyframesCommand(frameMoves, copyFrames, false, 0);
    }

    if (cmd) {
        KisProcessingApplicator::runSingleCommandStroke(m_d->image, cmd, KisStrokeJobData::BARRIER);
    }

    return cmd;
}

Qt::ItemFlags TimelineFramesModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = ModelWithExternalNotifications::flags(index);
    if (!index.isValid()) return flags;

    if (m_d->frameExists(index.row(), index.column()) || m_d->specialKeyframeExists(index.row(), index.column())) {
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

    return m_d->addKeyframe(dstIndex.row(), dstIndex.column(), false);
}

bool TimelineFramesModel::copyFrame(const QModelIndex &dstIndex)
{
    if (!dstIndex.isValid()) return false;

    return m_d->addKeyframe(dstIndex.row(), dstIndex.column(), true);
}

bool TimelineFramesModel::insertFrames(int dstColumn, const QList<int> &dstRows, int count, int timing)
{
    if (dstRows.isEmpty() || count <= 0) return true;
    timing = qMax(timing, 1);

    KUndo2Command *parentCommand = new KUndo2Command(kundo2_i18np("Insert frame", "Insert %1 frames", count));

    {
        KisImageBarrierLockerWithFeedback locker(m_d->image);

        QModelIndexList indexes;

        Q_FOREACH (int row, dstRows) {
            for (int column = dstColumn; column < columnCount(); column++) {
                indexes << index(row, column);
            }
        }

        setLastVisibleFrame(columnCount() + (count * timing) - 1);

        createOffsetFramesCommand(indexes, QPoint((count * timing), 0), false, false, parentCommand);

        Q_FOREACH (int row, dstRows) {
            KisNodeDummy *dummy = m_d->converter->dummyFromRow(row);
            if (!dummy) continue;

            KisNodeSP node = dummy->node();
            if (!KisAnimationUtils::supportsContentFrames(node)) continue;

            for (int column = dstColumn; column < dstColumn + (count * timing); column += timing) {
                KisAnimationUtils::createKeyframeCommand(m_d->image, node, KisKeyframeChannel::Content.id(), column, false, parentCommand);
            }
        }

        const int oldTime = m_d->image->animationInterface()->currentUITime();
        const int newTime = dstColumn > oldTime ? dstColumn : dstColumn + (count * timing) - 1;

        new KisSwitchCurrentTimeCommand(m_d->image->animationInterface(),
                                        oldTime,
                                        newTime, parentCommand);
    }

    KisProcessingApplicator::runSingleCommandStroke(m_d->image, parentCommand, KisStrokeJobData::BARRIER);

    return true;
}

bool TimelineFramesModel::insertHoldFrames(QModelIndexList selectedIndexes, int count)
{
    if (selectedIndexes.isEmpty() || count == 0) return true;

    QScopedPointer<KUndo2Command> parentCommand(new KUndo2Command(kundo2_i18np("Insert frame", "Insert %1 frames", count)));

    {
        KisImageBarrierLockerWithFeedback locker(m_d->image);

        QSet<KisKeyframeSP> uniqueKeyframesInSelection;

        int minSelectedTime = std::numeric_limits<int>::max();

        Q_FOREACH (const QModelIndex &index, selectedIndexes) {
            KisNodeSP node = nodeAt(index);
            KIS_SAFE_ASSERT_RECOVER(node) { continue; }

            KisKeyframeChannel *channel = node->getKeyframeChannel(KisKeyframeChannel::Content.id());
            if (!channel) continue;

            minSelectedTime = qMin(minSelectedTime, index.column());
            KisKeyframeSP keyFrame = channel->activeKeyframeAt(index.column());

            if (keyFrame) {
                uniqueKeyframesInSelection.insert(keyFrame);
            }
        }

        QList<KisKeyframeSP> keyframesToMove;

        for (auto it = uniqueKeyframesInSelection.begin(); it != uniqueKeyframesInSelection.end(); ++it) {
            KisKeyframeSP keyframe = *it;

            KisKeyframeChannel *channel = keyframe->channel();
            KisKeyframeSP nextKeyframe = channel->nextKeyframe(keyframe);

            if (nextKeyframe) {
                keyframesToMove << nextKeyframe;
            }
        }

        std::sort(keyframesToMove.begin(), keyframesToMove.end(),
            [] (KisKeyframeSP lhs, KisKeyframeSP rhs) {
                return lhs->time() > rhs->time();
            });

        if (keyframesToMove.isEmpty()) return true;

        const int maxColumn = columnCount();

        if (count > 0) {
            setLastVisibleFrame(columnCount() + count);
        }

        Q_FOREACH (KisKeyframeSP keyframe, keyframesToMove) {
            int plannedFrameMove = count;

            if (count < 0) {
                KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(keyframe->time() > 0, false);

                KisKeyframeSP prevFrame = keyframe->channel()->previousKeyframe(keyframe);
                KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(prevFrame, false);

                plannedFrameMove = qMax(count, prevFrame->time() - keyframe->time() + 1);

                minSelectedTime = qMin(minSelectedTime, prevFrame->time());
            }

            KisNodeDummy *dummy = m_d->dummiesFacade->dummyForNode(keyframe->channel()->node());
            KIS_SAFE_ASSERT_RECOVER(dummy) { continue; }

            const int row = m_d->converter->rowForDummy(dummy);
            KIS_SAFE_ASSERT_RECOVER(row >= 0) { continue; }

            QModelIndexList indexes;
            for (int column = keyframe->time(); column < maxColumn; column++) {
                indexes << index(row, column);
            }

            createOffsetFramesCommand(indexes,
                                      QPoint(plannedFrameMove, 0),
                                      false, true, parentCommand.data());
        }

        const int oldTime = m_d->image->animationInterface()->currentUITime();
        const int newTime = minSelectedTime;

        new KisSwitchCurrentTimeCommand(m_d->image->animationInterface(),
                                        oldTime,
                                        newTime, parentCommand.data());
    }


    KisProcessingApplicator::runSingleCommandStroke(m_d->image, parentCommand.take(), KisStrokeJobData::BARRIER);
    return true;
}

QString TimelineFramesModel::audioChannelFileName() const
{
    return m_d->image ? m_d->image->animationInterface()->audioChannelFileName() : QString();
}

void TimelineFramesModel::setAudioChannelFileName(const QString &fileName)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_d->image);
    m_d->image->animationInterface()->setAudioChannelFileName(fileName);
}

bool TimelineFramesModel::isAudioMuted() const
{
    return m_d->image ? m_d->image->animationInterface()->isAudioMuted() : false;
}

void TimelineFramesModel::setAudioMuted(bool value)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_d->image);
    m_d->image->animationInterface()->setAudioMuted(value);
}

qreal TimelineFramesModel::audioVolume() const
{
    return m_d->image ? m_d->image->animationInterface()->audioVolume() : 0.5;
}

void TimelineFramesModel::setAudioVolume(qreal value)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_d->image);
    m_d->image->animationInterface()->setAudioVolume(value);
}

void TimelineFramesModel::setFullClipRangeStart(int column)
{
    m_d->image->animationInterface()->setFullClipRangeStartTime(column);
}

void TimelineFramesModel::setFullClipRangeEnd(int column)
{
    m_d->image->animationInterface()->setFullClipRangeEndTime(column);
}
