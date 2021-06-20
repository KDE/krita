/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisAnimTimelineFramesModel.h"
#include <QFont>
#include <QSize>
#include <QColor>
#include <QMimeData>
#include <QPointer>
#include <KisResourceModel.h>

#include "kis_layer.h"
#include "kis_config.h"

#include "kis_global.h"
#include "kis_debug.h"
#include "kis_image.h"
#include "kis_image_animation_interface.h"
#include "kis_undo_adapter.h"
#include "kis_node_dummies_graph.h"
#include "kis_dummies_facade_base.h"
#include "KisNodeDisplayModeAdapter.h"
#include "kis_signal_compressor.h"
#include "kis_signal_compressor_with_param.h"
#include "kis_keyframe_channel.h"
#include "kis_raster_keyframe_channel.h"
#include "kundo2command.h"
#include "kis_post_execution_undo_adapter.h"
#include <commands/kis_node_property_list_command.h>
#include <commands_new/kis_switch_current_time_command.h>

#include "KisAnimUtils.h"
#include "KisAnimTimelineColors.h"
#include "kis_node_model.h"
#include "kis_projection_leaf.h"
#include "kis_time_span.h"

#include "kis_node_view_color_scheme.h"
#include <kis_painting_tweaks.h>
#include "KisPart.h"
#include <QApplication>
#include "KisDocument.h"
#include "KisViewManager.h"
#include "kis_processing_applicator.h"
#include <KisImageBarrierLockerWithFeedback.h>
#include "kis_node_uuid_info.h"


struct KisAnimTimelineFramesModel::Private
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
        
        if (image->isIsolatingLayer()) {
            return dummy->node()->isIsolatedRoot() && !dummy->node()->userLocked();
        } else {
            return dummy->node()->visible() && !dummy->node()->userLocked();
        }
    }

    bool frameExists(int row, int column) const {
        KisNodeDummy *dummy = converter->dummyFromRow(row);
        if (!dummy) return false;

        KisKeyframeChannel *primaryChannel = dummy->node()->getKeyframeChannel(KisKeyframeChannel::Raster.id());
        return (primaryChannel && primaryChannel->keyframeAt(column));
    }

    bool frameHasContent(int row, int column) {
        KisNodeDummy *dummy = converter->dummyFromRow(row);

        KisKeyframeChannel *primaryChannel = dummy->node()->getKeyframeChannel(KisKeyframeChannel::Raster.id());
        if (!primaryChannel) return false;

        // first check if we are a key frame
        KisRasterKeyframeSP frame = primaryChannel->activeKeyframeAt<KisRasterKeyframe>(column);
        if (!frame) return false;

        return frame->hasContent();
    }

    bool specialKeyframeExists(int row, int column) {
        KisNodeDummy *dummy = converter->dummyFromRow(row);
        if (!dummy) return false;
        Q_FOREACH(KisKeyframeChannel *channel, dummy->node()->keyframeChannels()) {
            if (channel->id() != KisKeyframeChannel::Raster.id() && channel->keyframeAt(column)) {
                return true;
            }
        }
        return false;
    }

    int frameColorLabel(int row, int column) {
        KisNodeDummy *dummy = converter->dummyFromRow(row);
        if (!dummy) return -1;

        KisKeyframeChannel *primaryChannel = dummy->node()->getKeyframeChannel(KisKeyframeChannel::Raster.id());
        if (!primaryChannel) return -1;

        KisKeyframeSP frame = primaryChannel->activeKeyframeAt(column);
        if (!frame) return -1;

        return frame->colorLabel();
    }

    void setFrameColorLabel(int row, int column, int color) {
        KisNodeDummy *dummy = converter->dummyFromRow(row);
        if (!dummy) return;

        KisKeyframeChannel *primaryChannel = dummy->node()->getKeyframeChannel(KisKeyframeChannel::Raster.id());
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
        if (!KisAnimUtils::supportsContentFrames(node)) return false;

        KisAnimUtils::createKeyframeLazy(image, node, KisKeyframeChannel::Raster.id(), column, copy);
        return true;
    }

    bool addNewLayer(int row) {
        Q_UNUSED(row);

        if (nodeInterface) {
            KisLayerSP layer = nodeInterface->addPaintLayer();
            layer->setPinnedToTimeline(true);
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

KisAnimTimelineFramesModel::KisAnimTimelineFramesModel(QObject *parent)
    : ModelWithExternalNotifications(parent),
      m_d(new Private)
{
    connect(&m_d->updateTimer, SIGNAL(timeout()), SLOT(processUpdateQueue()));
}

KisAnimTimelineFramesModel::~KisAnimTimelineFramesModel()
{
}

bool KisAnimTimelineFramesModel::hasConnectionToCanvas() const
{
    return m_d->dummiesFacade;
}

void KisAnimTimelineFramesModel::setNodeManipulationInterface(NodeManipulationInterface *iface)
{
    m_d->nodeInterface.reset(iface);
}

KisNodeSP KisAnimTimelineFramesModel::nodeAt(QModelIndex index) const
{
    /**
     * The dummy might not exist because the user could (quickly) change
     * active layer and the list of the nodes in m_d->converter will change.
     */
    KisNodeDummy *dummy = m_d->converter->dummyFromRow(index.row());
    return dummy ? dummy->node() : 0;
}

QMap<QString, KisKeyframeChannel*> KisAnimTimelineFramesModel::channelsAt(QModelIndex index) const
{
    KisNodeDummy *srcDummy = m_d->converter->dummyFromRow(index.row());
    return srcDummy->node()->keyframeChannels();
}

KisKeyframeChannel *KisAnimTimelineFramesModel::channelByID(QModelIndex index, const QString &id) const
{
    KisNodeDummy *srcDummy = m_d->converter->dummyFromRow(index.row());
    return srcDummy->node()->getKeyframeChannel(id);
}

void KisAnimTimelineFramesModel::setDummiesFacade(KisDummiesFacadeBase *dummiesFacade,
                                           KisImageSP image,
                                           KisNodeDisplayModeAdapter *displayModeAdapter)
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
        m_d->converter.reset(new TimelineNodeListKeeper(this, m_d->dummiesFacade, displayModeAdapter));
        connect(m_d->dummiesFacade, SIGNAL(sigDummyChanged(KisNodeDummy*)),
                SLOT(slotDummyChanged(KisNodeDummy*)));
        connect(m_d->image->animationInterface(),
                SIGNAL(sigFullClipRangeChanged()), SIGNAL(sigInfiniteTimelineUpdateNeeded()));
        connect(m_d->image->animationInterface(),
                SIGNAL(sigAudioChannelChanged()), SIGNAL(sigAudioChannelChanged()));
        connect(m_d->image->animationInterface(),
                SIGNAL(sigAudioVolumeChanged()), SIGNAL(sigAudioChannelChanged()));
        connect(m_d->image, SIGNAL(sigImageModified()), SLOT(slotImageContentChanged()));
        connect(m_d->image, SIGNAL(sigIsolatedModeChanged()), SLOT(slotImageContentChanged()));
    }

    if (m_d->dummiesFacade != oldDummiesFacade) {
        beginResetModel();
        endResetModel();
    }

    if (m_d->dummiesFacade) {
        emit sigInfiniteTimelineUpdateNeeded();
        emit sigAudioChannelChanged();
        slotCurrentTimeChanged(m_d->image->animationInterface()->currentUITime());
    }
}

void KisAnimTimelineFramesModel::slotDummyChanged(KisNodeDummy *dummy)
{
    if (!m_d->updateQueue.contains(dummy)) {
        m_d->updateQueue.append(dummy);
    }
    m_d->updateTimer.start();
}

void KisAnimTimelineFramesModel::slotImageContentChanged()
{
    if (m_d->activeLayerIndex < 0) return;

    KisNodeDummy *dummy = m_d->converter->dummyFromRow(m_d->activeLayerIndex);
    if (!dummy) return;

    slotDummyChanged(dummy);
}

void KisAnimTimelineFramesModel::processUpdateQueue()
{
    if (!m_d->converter) return;

    Q_FOREACH (KisNodeDummy *dummy, m_d->updateQueue) {
        int row = m_d->converter->rowForDummy(dummy);

        if (row >= 0) {
            emit headerDataChanged (Qt::Vertical, row, row);
            emit dataChanged(this->index(row, 0), this->index(row, columnCount() - 1));
        }
    }
    m_d->updateQueue.clear();
}

void KisAnimTimelineFramesModel::slotCurrentNodeChanged(KisNodeSP node)
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
    
    int lastActiveLayerIndex = m_d->activeLayerIndex;
    const bool prevActiveWasPinned = headerData(m_d->activeLayerIndex, Qt::Vertical, PinnedToTimelineRole).toBool();
    m_d->converter->updateActiveDummy(dummy);

    const int row = m_d->converter->rowForDummy(dummy);
    if (row < 0) {
        qWarning() << "WARNING: TimelineFramesModel::slotCurrentNodeChanged: node not found!";
    }

    if (row >= 0 && m_d->activeLayerIndex != row) {
        setData(index(row, 0), true, ActiveLayerRole);
    } else if (row >= 0 ){
        sigEnsureRowVisible(row);
        
        // If the activeLayerIndex is considered to be the same "row", but the last one was pinned,
        // it means that the previousActiveLayer has actually moved down the list because a new
        // layer was inserted.
        if (prevActiveWasPinned) {
            lastActiveLayerIndex += 1;
        }
    }
    
    // NOTE: Not in love with exposing 'selection' concepts to the model,
    // but since this issue already existed, this is alright for now.
    // Especially since it's just a signal...
    requestTransferSelectionBetweenRows(lastActiveLayerIndex, m_d->activeLayerIndex);
}

int KisAnimTimelineFramesModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    if(!m_d->dummiesFacade) return 0;

    return m_d->converter->rowCount();
}

QVariant KisAnimTimelineFramesModel::data(const QModelIndex &index, int role) const
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
    case Qt::UserRole + KisAbstractResourceModel::LargeThumbnail: {
        KisNodeDummy *dummy = m_d->converter->dummyFromRow(index.row());
        if (!dummy) {
            return  QVariant();
        }
        const int maxSize = 200;

        QImage image(dummy->node()->createThumbnailForFrame(maxSize, maxSize, index.column(), Qt::KeepAspectRatio));
        return image;
    }
    }

    return ModelWithExternalNotifications::data(index, role);
}

bool KisAnimTimelineFramesModel::setData(const QModelIndex &index, const QVariant &value, int role)
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

QVariant KisAnimTimelineFramesModel::headerData(int section, Qt::Orientation orientation, int role) const
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
            if (node->projectionLeaf()->isDroppedNode()) {
                baseFont.setStrikeOut(true);
            } else if (m_d->image && m_d->image->isolationRootNode() &&
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
        case PinnedToTimelineRole: {
            KisNodeDummy *dummy = m_d->converter->dummyFromRow(section);
            if (!dummy) return QVariant();
            return dummy->node()->isPinnedToTimeline();
        }
        case Qt::BackgroundRole: {
            int label = m_d->layerColorLabel(section);
            if (label > 0) {
                KisNodeViewColorScheme scm;
                QColor color = scm.colorFromLabelIndex(label);
                QPalette pal = qApp->palette();
                color = KisPaintingTweaks::blendColors(color, pal.color(QPalette::Button), 0.3);
                return QBrush(color);
            } else {
                return QVariant();
            }
        }
        }
    }

    return ModelWithExternalNotifications::headerData(section, orientation, role);
}

bool KisAnimTimelineFramesModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
    if (!m_d->dummiesFacade) return false;

    if (orientation == Qt::Vertical) {
        switch (role) {
        case ActiveLayerRole: {
            setData(index(section, 0), value, role);
            break;
        }
        case TimelinePropertiesRole: {
            KisAnimTimelineFramesModel::PropertyList props = value.value<KisAnimTimelineFramesModel::PropertyList>();

            int result = m_d->setLayerProperties(section, props);
            emit headerDataChanged (Qt::Vertical, section, section);
            return result;
        }
        case PinnedToTimelineRole: {
            KisNodeDummy *dummy = m_d->converter->dummyFromRow(section);
            if (!dummy) return false;
            dummy->node()->setPinnedToTimeline(value.toBool());
            return true;
        }
        }
    }

    return ModelWithExternalNotifications::setHeaderData(section, orientation, value, role);
}

Qt::DropActions KisAnimTimelineFramesModel::supportedDragActions() const
{
    return Qt::MoveAction | Qt::CopyAction | Qt::LinkAction;
}

Qt::DropActions KisAnimTimelineFramesModel::supportedDropActions() const
{
    return Qt::MoveAction | Qt::CopyAction | Qt::LinkAction;
}

QStringList KisAnimTimelineFramesModel::mimeTypes() const
{
    QStringList types;
    types << QLatin1String("application/x-krita-frame");
    return types;
}

void KisAnimTimelineFramesModel::setLastClickedIndex(const QModelIndex &index)
{
    m_d->lastClickedIndex = index;
}

QMimeData* KisAnimTimelineFramesModel::mimeData(const QModelIndexList &indexes) const
{
    return mimeDataExtended(indexes, m_d->lastClickedIndex, UndefinedPolicy);
}

QMimeData *KisAnimTimelineFramesModel::mimeDataExtended(const QModelIndexList &indexes,
                                                 const QModelIndex &baseIndex,
                                                 KisAnimTimelineFramesModel::MimeCopyPolicy copyPolicy) const
{
    QMimeData *data = new QMimeData();

    QByteArray encoded;
    QDataStream stream(&encoded, QIODevice::WriteOnly);

    const int baseRow = baseIndex.row();
    const int baseColumn = baseIndex.column();

    const QByteArray uuidDataRoot = m_d->image->root()->uuid().toRfc4122();
    stream << int(uuidDataRoot.size());
    stream.writeRawData(uuidDataRoot.data(), uuidDataRoot.size());

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

bool KisAnimTimelineFramesModel::canDropFrameData(const QMimeData */*data*/, const QModelIndex &index)
{
    if (!index.isValid()) return false;

    if ( !m_d->layerEditable(index.row()) ) return false;

    /**
     * Now we support D&D around any layer, so just return 'true' all
     * the time.
     */
    return true;
}

bool KisAnimTimelineFramesModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    Q_UNUSED(row);
    Q_UNUSED(column);

    return dropMimeDataExtended(data, action, parent);
}

bool KisAnimTimelineFramesModel::dropMimeDataExtended(const QMimeData *data, Qt::DropAction action, const QModelIndex &parent, bool *dataMoved)
{
    bool result = false;

    if ((action != Qt::MoveAction && action != Qt::CopyAction && action != Qt::LinkAction) ||
        !parent.isValid()) return result;

    QByteArray encoded = data->data("application/x-krita-frame");
    QDataStream stream(&encoded, QIODevice::ReadOnly);

    int uuidLenRoot = 0;
    stream >> uuidLenRoot;
    QByteArray uuidDataRoot(uuidLenRoot, '\0');
    stream.readRawData(uuidDataRoot.data(), uuidLenRoot);
    QUuid nodeUuidRoot = QUuid::fromRfc4122(uuidDataRoot);

    KisPart *partInstance = KisPart::instance();
    QList<QPointer<KisDocument>> documents = partInstance->documents();

    KisImageSP srcImage = 0;
    Q_FOREACH(KisDocument *doc, documents) {
        KisImageSP tmpSrcImage = doc->image();
        if (tmpSrcImage->root()->uuid() == nodeUuidRoot) {
            srcImage = tmpSrcImage;
            break;
        }
    }

    if (!srcImage) {
        KisPart *kisPartInstance = KisPart::instance();
        kisPartInstance->currentMainwindow()->viewManager()->showFloatingMessage(
                    i18n("Dropped frames are not available in this Krita instance")
                    , QIcon());
        return false;
    }

    int size, baseRow, baseColumn;
    stream >> size >> baseRow >> baseColumn;

    const QPoint offset(parent.column() - baseColumn, parent.row() - baseRow);
    KisAnimUtils::FrameMovePairList frameMoves;
    int necessaryOffset = 0;  //Necessary offset to keep move above 0, used later.

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
            srcNode = nodeInfo.findNode(srcImage->root());
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
            KisAnimUtils::FrameItem srcItem(srcNode, channel->id(), srcColumn);
            KisAnimUtils::FrameItem dstItem(dstNode, channel->id(), srcColumn + offset.x());

            if ((srcColumn + offset.x()) * -1 > necessaryOffset ) {
                necessaryOffset = (srcColumn + offset.x()) * -1;
            }

            frameMoves << std::make_pair(srcItem, dstItem);
        }
    }

    MimeCopyPolicy copyPolicy = UndefinedPolicy;

    if (!stream.atEnd()) {
        int value = 0;
        stream >> value;
        copyPolicy = MimeCopyPolicy(value);
    }

    const bool copyFrames = copyPolicy == UndefinedPolicy ?
        action == Qt::CopyAction :
        copyPolicy == CopyFramesPolicy;

    const bool cloneFrames = action == Qt::LinkAction || copyPolicy == CloneFramesPolicy;

    if (dataMoved) {
        *dataMoved = !copyFrames;
    }

    KUndo2Command *cmd = 0;

    if (!frameMoves.isEmpty()) {

        // We need to make sure that no movement ever occurs into the negative values.
        // TODO: Probably a better way to fix this, I'm not happy with this fix.
        if (necessaryOffset > 0) {
            for (int i = 0; i < frameMoves.count(); i++){
                frameMoves[i].second.time += necessaryOffset;
            }
        }

        KisImageBarrierLockerWithFeedback locker(m_d->image);

        if (cloneFrames) {
            cmd = KisAnimUtils::createCloneKeyframesCommand(frameMoves, nullptr);
        } else {
            cmd = KisAnimUtils::createMoveKeyframesCommand(frameMoves, copyFrames, false, nullptr);
        }
    }

    if (cmd) {
        KisProcessingApplicator::runSingleCommandStroke(m_d->image, cmd,
                                                        KisStrokeJobData::BARRIER,
                                                        KisStrokeJobData::EXCLUSIVE);
    }

    return cmd;
}

Qt::ItemFlags KisAnimTimelineFramesModel::flags(const QModelIndex &index) const
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

bool KisAnimTimelineFramesModel::insertRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(parent);

    KIS_ASSERT_RECOVER(count == 1) { return false; }

    if (row < 0 || row > rowCount()) return false;

    bool result = m_d->addNewLayer(row);
    return result;
}

bool KisAnimTimelineFramesModel::removeRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(parent);
    KIS_ASSERT_RECOVER(count == 1) { return false; }

    if (row < 0 || row >= rowCount()) return false;

    bool result = m_d->removeLayer(row);
    return result;
}

bool KisAnimTimelineFramesModel::insertOtherLayer(int index, int dstRow)
{
    Q_UNUSED(dstRow);

    TimelineNodeListKeeper::OtherLayersList list =
        m_d->converter->otherLayersList();

    if (index < 0 || index >= list.size()) return false;

    list[index].dummy->node()->setPinnedToTimeline(true);
    dstRow = m_d->converter->rowForDummy(list[index].dummy);
    setData(this->index(dstRow, 0), true, ActiveLayerRole);

    return true;
}

int KisAnimTimelineFramesModel::activeLayerRow() const
{
    return m_d->activeLayerIndex;
}

bool KisAnimTimelineFramesModel::createFrame(const QModelIndex &dstIndex)
{
    if (!dstIndex.isValid()) return false;

    return m_d->addKeyframe(dstIndex.row(), dstIndex.column(), false);
}

bool KisAnimTimelineFramesModel::copyFrame(const QModelIndex &dstIndex)
{
    if (!dstIndex.isValid()) return false;

    return m_d->addKeyframe(dstIndex.row(), dstIndex.column(), true);
}

void KisAnimTimelineFramesModel::makeClonesUnique(const QModelIndexList &indices)
{
    KisAnimUtils::FrameItemList frameItems;

    foreach (const QModelIndex &index, indices) {
        const int time = index.column();
        KisKeyframeChannel *channel = channelByID(index, KisKeyframeChannel::Raster.id());
        frameItems << KisAnimUtils::FrameItem(channel->node(), channel->id(), time);
    }

    KisAnimUtils::makeClonesUnique(m_d->image, frameItems);
}

bool KisAnimTimelineFramesModel::insertFrames(int dstColumn, const QList<int> &dstRows, int count, int timing)
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
            if (!KisAnimUtils::supportsContentFrames(node)) continue;

            for (int column = dstColumn; column < dstColumn + (count * timing); column += timing) {
                KisAnimUtils::createKeyframeCommand(m_d->image, node, KisKeyframeChannel::Raster.id(), column, false, parentCommand);
            }
        }

        const int oldTime = m_d->image->animationInterface()->currentUITime();
        const int newTime = dstColumn > oldTime ? dstColumn : dstColumn + (count * timing) - 1;

        new KisSwitchCurrentTimeCommand(m_d->image->animationInterface(),
                                        oldTime,
                                        newTime, parentCommand);
    }

    KisProcessingApplicator::runSingleCommandStroke(m_d->image, parentCommand,
                                                    KisStrokeJobData::BARRIER,
                                                    KisStrokeJobData::EXCLUSIVE);

    return true;
}

bool KisAnimTimelineFramesModel::insertHoldFrames(const QModelIndexList &selectedIndexes, int count)
{
    if (selectedIndexes.isEmpty() || count == 0) return true;

    QScopedPointer<KUndo2Command> parentCommand(new KUndo2Command(kundo2_i18np("Insert frame", "Insert %1 frames", count)));

    {
        KisImageBarrierLockerWithFeedback locker(m_d->image);

        QSet<TimelineSelectionEntry> uniqueKeyframesInSelection;

        int minSelectedTime = std::numeric_limits<int>::max();

        Q_FOREACH (const QModelIndex &index, selectedIndexes) {
            KisNodeSP node = nodeAt(index);
            KIS_SAFE_ASSERT_RECOVER(node) { continue; }

            KisRasterKeyframeChannel *channel = dynamic_cast<KisRasterKeyframeChannel*>(node->getKeyframeChannel(KisKeyframeChannel::Raster.id()));
            if (!channel) continue;

            minSelectedTime = qMin(minSelectedTime, index.column());

            int time = channel->activeKeyframeTime(index.column());
            KisRasterKeyframeSP keyframe = channel->activeKeyframeAt<KisRasterKeyframe>(index.column());

            if (keyframe) {
                uniqueKeyframesInSelection.insert(TimelineSelectionEntry{channel, time, keyframe});
            }
        }

        QList<TimelineSelectionEntry> keyframesToMove;

        for (auto it = uniqueKeyframesInSelection.begin(); it != uniqueKeyframesInSelection.end(); ++it) {
            TimelineSelectionEntry keyframeEntry = *it;

            KisRasterKeyframeChannel *channel = keyframeEntry.channel;
            int nextKeyframeTime = channel->nextKeyframeTime(keyframeEntry.time);
            KisRasterKeyframeSP nextKeyframe = channel->keyframeAt<KisRasterKeyframe>(nextKeyframeTime);

            if (nextKeyframe) {
                keyframesToMove << TimelineSelectionEntry{ channel, nextKeyframeTime, nextKeyframe };
            }
        }

        std::sort(keyframesToMove.begin(), keyframesToMove.end(),
            [] (TimelineSelectionEntry lhs, TimelineSelectionEntry rhs) {
                return lhs.time > rhs.time;
            });

        if (keyframesToMove.isEmpty()) return true;

        const int maxColumn = columnCount();

        if (count > 0) {
            setLastVisibleFrame(columnCount() + count);
        }

        Q_FOREACH (TimelineSelectionEntry entry, keyframesToMove) {
            int plannedFrameMove = count;

            if (count < 0) {
                KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(entry.time > 0, false);

                int prevKeyframeTime = entry.channel->previousKeyframeTime(entry.time);
                KisRasterKeyframeSP prevFrame = entry.channel->keyframeAt<KisRasterKeyframe>(prevKeyframeTime);
                KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(prevFrame, false);

                plannedFrameMove = qMax(count, prevKeyframeTime - entry.time + 1);

                minSelectedTime = qMin(minSelectedTime, prevKeyframeTime);
            }

            KisNodeDummy *dummy = m_d->dummiesFacade->dummyForNode(entry.channel->node());
            KIS_SAFE_ASSERT_RECOVER(dummy) { continue; }

            const int row = m_d->converter->rowForDummy(dummy);
            KIS_SAFE_ASSERT_RECOVER(row >= 0) { continue; }

            QModelIndexList indices;
            for (int column = entry.time; column < maxColumn; column++) {
                indices << index(row, column);
            }

            createOffsetFramesCommand(indices,
                                      QPoint(plannedFrameMove, 0),
                                      false,
                                      true,
                                      parentCommand.data());
        }

        const int oldTime = m_d->image->animationInterface()->currentUITime();
        const int newTime = minSelectedTime;

        new KisSwitchCurrentTimeCommand(m_d->image->animationInterface(),
                                        oldTime,
                                        newTime,
                                        parentCommand.data());
    }


    KisProcessingApplicator::runSingleCommandStroke(m_d->image, parentCommand.take(),
                                                    KisStrokeJobData::BARRIER,
                                                    KisStrokeJobData::EXCLUSIVE);
    return true;
}

QString KisAnimTimelineFramesModel::audioChannelFileName() const
{
    return m_d->image ? m_d->image->animationInterface()->audioChannelFileName() : QString();
}

void KisAnimTimelineFramesModel::setAudioChannelFileName(const QString &fileName)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_d->image);
    m_d->image->animationInterface()->setAudioChannelFileName(fileName);
}

bool KisAnimTimelineFramesModel::isAudioMuted() const
{
    return m_d->image ? m_d->image->animationInterface()->isAudioMuted() : false;
}

void KisAnimTimelineFramesModel::setAudioMuted(bool value)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_d->image);
    m_d->image->animationInterface()->setAudioMuted(value);
}

qreal KisAnimTimelineFramesModel::audioVolume() const
{
    return m_d->image ? m_d->image->animationInterface()->audioVolume() : 0.5;
}

void KisAnimTimelineFramesModel::setAudioVolume(qreal value)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_d->image);
    m_d->image->animationInterface()->setAudioVolume(value);
}

void KisAnimTimelineFramesModel::setFullClipRangeStart(int column)
{
    m_d->image->animationInterface()->setFullClipRangeStartTime(column);
}

void KisAnimTimelineFramesModel::setFullClipRangeEnd(int column)
{
    m_d->image->animationInterface()->setFullClipRangeEndTime(column);
}

void KisAnimTimelineFramesModel::clearEntireCache()
{
    m_d->image->animationInterface()->invalidateFrames(KisTimeSpan::infinite(0), m_d->image->bounds());
}

void KisAnimTimelineFramesModel::setActiveLayerSelectedTimes(const QSet<int> &times)
{
    m_d->image->animationInterface()->setActiveLayerSelectedTimes(times);
}
