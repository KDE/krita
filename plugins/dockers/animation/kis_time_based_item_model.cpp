/*
 *  Copyright (c) 2016 Jouni Pentikäinen <joupent@gmail.com>
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

#include "kis_time_based_item_model.h"

#include <QPointer>
#include <kis_config.h>

#include "kis_animation_frame_cache.h"
#include "kis_animation_player.h"
#include "kis_signal_compressor_with_param.h"
#include "kis_image.h"
#include "kis_image_animation_interface.h"
#include "kis_time_range.h"
#include "kis_animation_utils.h"
#include "kis_keyframe_channel.h"
#include "kis_processing_applicator.h"
#include "KisImageBarrierLockerWithFeedback.h"
#include "commands_new/kis_switch_current_time_command.h"
#include "kis_command_utils.h"

struct KisTimeBasedItemModel::Private
{
    Private()
        : animationPlayer(0)
        , numFramesOverride(0)
        , activeFrameIndex(0)
        , scrubInProgress(false)
        , scrubStartFrame(-1)
    {}

    KisImageWSP image;
    KisAnimationFrameCacheWSP framesCache;
    QPointer<KisAnimationPlayer> animationPlayer;

    QVector<bool> cachedFrames;

    int numFramesOverride;
    int activeFrameIndex;

    bool scrubInProgress;
    int scrubStartFrame;

    QScopedPointer<KisSignalCompressorWithParam<int> > scrubbingCompressor;

    int baseNumFrames() const {

        auto imageSP = image.toStrongRef();
        if (!imageSP) return 0;

        KisImageAnimationInterface *i = imageSP->animationInterface();
        if (!i) return 1;

        return i->totalLength();
    }

    int effectiveNumFrames() const {
        if (image.isNull()) return 0;

        return qMax(baseNumFrames(), numFramesOverride);
    }

    int framesPerSecond() {
        return image->animationInterface()->framerate();
    }
};

KisTimeBasedItemModel::KisTimeBasedItemModel(QObject *parent)
    : QAbstractTableModel(parent)
    , m_d(new Private())
{
    KisConfig cfg;

    using namespace std::placeholders;
    std::function<void (int)> callback(
        std::bind(&KisTimeBasedItemModel::slotInternalScrubPreviewRequested, this, _1));

    m_d->scrubbingCompressor.reset(
        new KisSignalCompressorWithParam<int>(cfg.scrubbingUpdatesDelay(), callback, KisSignalCompressor::FIRST_ACTIVE));
}

KisTimeBasedItemModel::~KisTimeBasedItemModel()
{}

void KisTimeBasedItemModel::setImage(KisImageWSP image)
{
    KisImageWSP oldImage = m_d->image;

    m_d->image = image;

    if (image) {
        KisImageAnimationInterface *ai = image->animationInterface();

        slotCurrentTimeChanged(ai->currentUITime());

        connect(ai, SIGNAL(sigFramerateChanged()), SLOT(slotFramerateChanged()));
        connect(ai, SIGNAL(sigUiTimeChanged(int)), SLOT(slotCurrentTimeChanged(int)));
    }

    if (image != oldImage) {
        beginResetModel();
        endResetModel();
    }
}

void KisTimeBasedItemModel::setFrameCache(KisAnimationFrameCacheSP cache)
{
    if (KisAnimationFrameCacheSP(m_d->framesCache) == cache) return;

    if (m_d->framesCache) {
        m_d->framesCache->disconnect(this);
    }

    m_d->framesCache = cache;

    if (m_d->framesCache) {
        connect(m_d->framesCache, SIGNAL(changed()), SLOT(slotCacheChanged()));
    }
}

void KisTimeBasedItemModel::setAnimationPlayer(KisAnimationPlayer *player)
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

void KisTimeBasedItemModel::setLastVisibleFrame(int time)
{
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

int KisTimeBasedItemModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_d->effectiveNumFrames();
}

QVariant KisTimeBasedItemModel::data(const QModelIndex &index, int role) const
{
    switch (role) {
    case ActiveFrameRole: {
        return index.column() == m_d->activeFrameIndex;
    }
    }

    return QVariant();
}

bool KisTimeBasedItemModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid()) return false;

    switch (role) {
    case ActiveFrameRole: {
        setHeaderData(index.column(), Qt::Horizontal, value, role);
        break;
    }
    }

    return false;
}

QVariant KisTimeBasedItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal) {
        switch (role) {
        case ActiveFrameRole:
            return section == m_d->activeFrameIndex;
        case FrameCachedRole:
            return m_d->cachedFrames.size() > section ? m_d->cachedFrames[section] : false;
        case FramesPerSecondRole:
            return m_d->framesPerSecond();
        }
    }

    return QVariant();
}

bool KisTimeBasedItemModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
    if (orientation == Qt::Horizontal) {
        switch (role) {
        case ActiveFrameRole:
            if (value.toBool() &&
                section != m_d->activeFrameIndex) {

                int prevFrame = m_d->activeFrameIndex;
                m_d->activeFrameIndex = section;

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
        }
    }

    return false;
}

bool KisTimeBasedItemModel::removeFrames(const QModelIndexList &indexes)
{
    KisAnimationUtils::FrameItemList frameItems;

    {
        KisImageBarrierLockerWithFeedback locker(m_d->image);

        Q_FOREACH (const QModelIndex &index, indexes) {
            int time = index.column();
            Q_FOREACH(KisKeyframeChannel *channel, channelsAt(index)) {
                if (channel->keyframeAt(time)) {
                    frameItems << KisAnimationUtils::FrameItem(channel->node(), channel->id(), index.column());
                }
            }
        }
    }

    if (frameItems.isEmpty()) return false;

    KisAnimationUtils::removeKeyframes(m_d->image, frameItems);

    return true;
}

KUndo2Command* KisTimeBasedItemModel::createOffsetFramesCommand(QModelIndexList srcIndexes,
                                                                const QPoint &offset,
                                                                bool copyFrames,
                                                                bool moveEmptyFrames,
                                                                KUndo2Command *parentCommand)
{
    if (srcIndexes.isEmpty()) return 0;
    if (offset.isNull()) return 0;

    KisAnimationUtils::sortPointsForSafeMove(&srcIndexes, offset);

    KisAnimationUtils::FrameItemList srcFrameItems;
    KisAnimationUtils::FrameItemList dstFrameItems;

    Q_FOREACH (const QModelIndex &srcIndex, srcIndexes) {
        QModelIndex dstIndex = index(
                srcIndex.row() + offset.y(),
                srcIndex.column() + offset.x());

        KisNodeSP srcNode = nodeAt(srcIndex);
        KisNodeSP dstNode = nodeAt(dstIndex);
        if (!srcNode || !dstNode) return 0;

        Q_FOREACH(KisKeyframeChannel *channel, channelsAt(srcIndex)) {
            if (moveEmptyFrames || channel->keyframeAt(srcIndex.column())) {
                srcFrameItems << KisAnimationUtils::FrameItem(srcNode, channel->id(), srcIndex.column());
                dstFrameItems << KisAnimationUtils::FrameItem(dstNode, channel->id(), dstIndex.column());
            }
        }
    }

    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(srcFrameItems.size() == dstFrameItems.size(), 0);
    if (srcFrameItems.isEmpty()) return 0;

    return
        KisAnimationUtils::createMoveKeyframesCommand(srcFrameItems,
                                                      dstFrameItems,
                                                      copyFrames,
                                                      moveEmptyFrames,
                                                      parentCommand);
}

bool KisTimeBasedItemModel::removeFramesAndOffset(QModelIndexList indicesToRemove)
{
    if (indicesToRemove.isEmpty()) return true;

    std::sort(indicesToRemove.begin(), indicesToRemove.end(),
              [] (const QModelIndex &lhs, const QModelIndex &rhs) {
                  return lhs.column() > rhs.column();
              });

    const int minColumn = indicesToRemove.last().column();

    KUndo2Command *parentCommand = new KUndo2Command(kundo2_i18np("Remove frame and shift", "Remove %1 frames and shift", indicesToRemove.size()));

    {
        KisImageBarrierLockerWithFeedback locker(m_d->image);

        Q_FOREACH (const QModelIndex &index, indicesToRemove) {
            QModelIndexList indicesToOffset;
            for (int column = index.column() + 1; column < columnCount(); column++) {
                indicesToOffset << this->index(index.row(), column);
            }
            createOffsetFramesCommand(indicesToOffset, QPoint(-1, 0), false, true, parentCommand);
        }

        const int oldTime = m_d->image->animationInterface()->currentUITime();
        const int newTime = minColumn;

        new KisSwitchCurrentTimeCommand(m_d->image->animationInterface(),
                                        oldTime,
                                        newTime,
                                        parentCommand);
    }

    KisProcessingApplicator::runSingleCommandStroke(m_d->image, parentCommand, KisStrokeJobData::BARRIER);
    return true;
}

bool KisTimeBasedItemModel::mirrorFrames(QModelIndexList indexes)
{
    QScopedPointer<KUndo2Command> parentCommand(new KUndo2Command(kundo2_i18n("Mirror Frames")));

    {
        KisImageBarrierLockerWithFeedback locker(m_d->image);

        QMap<int, QModelIndexList> rowsList;

        Q_FOREACH (const QModelIndex &index, indexes) {
            rowsList[index.row()].append(index);
        }


        Q_FOREACH (int row, rowsList.keys()) {
            QModelIndexList &list = rowsList[row];

            KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(!list.isEmpty(), false);

            std::sort(list.begin(), list.end(),
                [] (const QModelIndex &lhs, const QModelIndex &rhs) {
                    return lhs.column() < rhs.column();
                });

            auto srcIt = list.begin();
            auto dstIt = list.end();

            KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(srcIt != dstIt, false);
            --dstIt;

            QList<KisKeyframeChannel*> channels = channelsAt(*srcIt).values();

            while (srcIt < dstIt) {
                Q_FOREACH (KisKeyframeChannel *channel, channels) {
                    channel->swapFrames(srcIt->column(), dstIt->column(), parentCommand.data());
                }

                srcIt++;
                dstIt--;
            }
        }
    }

    KisProcessingApplicator::runSingleCommandStroke(m_d->image,
                                                    new KisCommandUtils::SkipFirstRedoWrapper(parentCommand.take()),
                                                    KisStrokeJobData::BARRIER);
    return true;
}

void KisTimeBasedItemModel::slotInternalScrubPreviewRequested(int time)
{
    if (m_d->animationPlayer && !m_d->animationPlayer->isPlaying()) {
        m_d->animationPlayer->displayFrame(time);
    }
}

void KisTimeBasedItemModel::setScrubState(bool active)
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

void KisTimeBasedItemModel::scrubTo(int time, bool preview)
{
    if (m_d->animationPlayer && m_d->animationPlayer->isPlaying()) return;

    KIS_ASSERT_RECOVER_RETURN(m_d->image);

    if (preview) {
        if (m_d->animationPlayer) {
            m_d->scrubbingCompressor->start(time);
        }
    } else {
        m_d->image->animationInterface()->requestTimeSwitchWithUndo(time);
    }
}

void KisTimeBasedItemModel::slotFramerateChanged()
{
    emit headerDataChanged(Qt::Horizontal, 0, columnCount() - 1);
}

void KisTimeBasedItemModel::slotCurrentTimeChanged(int time)
{
    if (time != m_d->activeFrameIndex) {
        setHeaderData(time, Qt::Horizontal, true, ActiveFrameRole);
    }
}

void KisTimeBasedItemModel::slotCacheChanged()
{
    const int numFrames = columnCount();
    m_d->cachedFrames.resize(numFrames);

    for (int i = 0; i < numFrames; i++) {
        m_d->cachedFrames[i] =
            m_d->framesCache->frameStatus(i) == KisAnimationFrameCache::Cached;
    }

    emit headerDataChanged(Qt::Horizontal, 0, numFrames);
}


void KisTimeBasedItemModel::slotPlaybackFrameChanged()
{
    if (!m_d->animationPlayer->isPlaying()) return;
    setData(index(0, m_d->animationPlayer->currentTime()), true, ActiveFrameRole);
}

void KisTimeBasedItemModel::slotPlaybackStopped()
{
    setData(index(0, m_d->image->animationInterface()->currentUITime()), true, ActiveFrameRole);
}

void KisTimeBasedItemModel::setPlaybackRange(const KisTimeRange &range)
{
    if (m_d->image.isNull()) return;

    KisImageAnimationInterface *i = m_d->image->animationInterface();
    i->setPlaybackRange(range);
}

bool KisTimeBasedItemModel::isPlaybackActive() const
{
    return m_d->animationPlayer && m_d->animationPlayer->isPlaying();
}

int KisTimeBasedItemModel::currentTime() const
{
    return m_d->image->animationInterface()->currentUITime();
}

KisImageWSP KisTimeBasedItemModel::image() const
{
    return m_d->image;
}
