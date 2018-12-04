/*
 *  Copyright (c) 2015 Jouni Pentikäinen <joupent@gmail.com>
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

#ifndef KIS_KEYFRAME_CHANNEL_H
#define KIS_KEYFRAME_CHANNEL_H

#include <QVariant>
#include <QDomElement>
#include <kundo2command.h>

#include "kis_types.h"
#include "KoID.h"
#include "kritaimage_export.h"
#include "kis_keyframe.h"
#include "kis_default_bounds.h"

class KisFrameSet;
class KisTimeSpan;
class KisAnimationCycle;
class KisRepeatFrame;
class KisVisibleKeyframeIterator;
class KisDefineCycleCommand;

class KRITAIMAGE_EXPORT KisKeyframeChannel : public QObject
{
    Q_OBJECT

public:
    // Standard Keyframe Ids

    static const KoID Content;
    static const KoID Opacity;
    static const KoID TransformArguments;
    static const KoID TransformPositionX;
    static const KoID TransformPositionY;

    static const KoID TransformScaleX;
    static const KoID TransformScaleY;
    static const KoID TransformShearX;
    static const KoID TransformShearY;
    static const KoID TransformRotationX;
    static const KoID TransformRotationY;
    static const KoID TransformRotationZ;
public:
    KisKeyframeChannel(const KoID& id, KisDefaultBoundsBaseSP defaultBounds);
    KisKeyframeChannel(const KisKeyframeChannel &rhs, KisNode *newParentNode);
    ~KisKeyframeChannel() override;

    QString id() const;
    QString name() const;

    void setNode(KisNodeWSP node);
    KisNodeWSP node() const;

    KisKeyframeSP addKeyframe(int time, KUndo2Command *parentCommand = 0);
    bool deleteKeyframe(KisKeyframeBaseSP keyframe, KUndo2Command *parentCommand = 0);
    bool moveKeyframe(KisKeyframeBaseSP keyframe, int newTime, KUndo2Command *parentCommand = 0);
    bool swapFrames(int lhsTime, int rhsTime, KUndo2Command *parentCommand = 0);
    KisKeyframeBaseSP copyItem(const KisKeyframeBaseSP item, int newTime, KUndo2Command *parentCommand = 0);
    KisKeyframeSP copyAsKeyframe(const KisKeyframeBaseSP item, int originalTime, int newTime, KUndo2Command *parentCommand = 0);
    virtual KisKeyframeSP linkKeyframe(const KisKeyframeBaseSP keyframe, int newTime, KUndo2Command *parentCommand = 0);
    KisKeyframeSP copyExternalKeyframe(KisKeyframeChannel *srcChannel, int srcTime, int dstTime, KUndo2Command *parentCommand = 0);

    KisDefineCycleCommand * createCycle(KisKeyframeSP firstKeyframe, KisKeyframeSP lastKeyframe, KUndo2Command *parentCommand = 0);
    KUndo2Command * deleteCycle(QSharedPointer<KisAnimationCycle> cycle, KUndo2Command *parentCommand = 0);
    QSharedPointer<KisRepeatFrame> addRepeat(QSharedPointer<KisAnimationCycle> cycle, int time, KUndo2Command *parentCommand);

    bool swapExternalKeyframe(KisKeyframeChannel *srcChannel, int srcTime, int dstTime, KUndo2Command *parentCommand = 0);

    KisKeyframeSP keyframeAt(int time) const;
    KisKeyframeSP activeKeyframeAt(int time) const;
    KisKeyframeSP visibleKeyframeAt(int time) const;
    KisKeyframeSP currentlyActiveKeyframe() const;

    KisKeyframeSP firstKeyframe() const;
    KisKeyframeSP nextKeyframe(KisKeyframeSP keyframe) const;
    KisKeyframeSP previousKeyframe(KisKeyframeSP keyframe) const;
    KisKeyframeSP nextKeyframe(const KisKeyframeBase &keyframe) const;
    KisKeyframeSP previousKeyframe(const KisKeyframeBase &keyframe) const;
    KisKeyframeSP lastKeyframe() const;

    KisKeyframeBaseSP itemAt(int time) const;
    KisKeyframeBaseSP activeItemAt(int time) const;
    KisKeyframeBaseSP nextItem(const KisKeyframeBase &item) const;
    KisKeyframeBaseSP previousItem(const KisKeyframeBase &item) const;

    KisVisibleKeyframeIterator visibleKeyframesFrom(int time) const;

    /**
     * Finds the original range of the cycle defined or repeated at the given time.
     * @arg time a time at any frame within the original cycle or any repeat of it.
     */
    KisTimeSpan cycledRangeAt(int time) const;

    /**
     * Finds the cycle defined at time, if any.
     * @arg time a time within the original range of the cycle.
     */
    QSharedPointer<KisAnimationCycle> cycleAt(int time) const;

    /**
     * Finds the repeat of a cycle at the time, if any.
     */
    QSharedPointer<KisRepeatFrame> activeRepeatAt(int time) const;

    /**
     * Finds the span of time of the keyframe active at given time.
     * If there is no active keyframe, first will be -1.
     * If the keyframe continues indefinitely, last will be -1.
     */
    void activeKeyframeRange(int time, int *first, int *last) const;

    /**
     * Calculates a pseudo-unique keyframes hash. The hash changes
     * every time any frame is added/removed/moved
     */
    int framesHash() const;

    QSet<int> allKeyframeIds() const;
    /**
     * Get the set of frames affected by any changes to the value
     * of the active keyframe at the given time.
     */
    virtual KisFrameSet affectedFrames(int time) const;

    /**
     * Get a set of frames for which the channel gives identical
     * results, compared to the given frame.
     *
     * Note: this set may be different than the set of affected frames
     * due to interpolation.
     */
    virtual KisFrameSet identicalFrames(int time, const KisTimeSpan range) const;
    virtual bool areFramesIdentical(int time1, int time2) const;
    virtual bool isFrameAffectedBy(int targetFrame, int changedFrame) const;

    int keyframeCount() const;

    virtual bool hasScalarValue() const = 0;
    virtual qreal minScalarValue() const;
    virtual qreal maxScalarValue() const;
    virtual qreal scalarValue(const KisKeyframeSP keyframe) const;
    virtual void setScalarValue(KisKeyframeSP keyframe, qreal value, KUndo2Command *parentCommand = 0);

    virtual QDomElement toXML(QDomDocument doc, const QString &layerFilename);
    virtual void loadXML(const QDomElement &channelNode);

    int currentTime() const;

Q_SIGNALS:
    void sigKeyframeAboutToBeAdded(KisKeyframeBaseSP keyframe);
    void sigKeyframeAdded(KisKeyframeBaseSP keyframe);
    void sigKeyframeAboutToBeRemoved(KisKeyframeBaseSP keyframe);
    void sigKeyframeRemoved(KisKeyframeBaseSP keyframe);
    void sigKeyframeAboutToBeMoved(KisKeyframeBaseSP keyframe, int toTime);
    void sigKeyframeMoved(KisKeyframeBaseSP keyframe, int fromTime);
    void sigKeyframeChanged(KisKeyframeBaseSP keyframe);

protected:
    typedef QMap<int, KisKeyframeSP> KeyframesMap;

    KeyframesMap &keys();
    const KeyframesMap &constKeys() const;

    virtual KisKeyframeSP createKeyframe(int time, const KisKeyframeSP copySrc, KUndo2Command *parentCommand) = 0;
    virtual void destroyKeyframe(KisKeyframeSP key, KUndo2Command *parentCommand) = 0;
    virtual void uploadExternalKeyframe(KisKeyframeChannel *srcChannel, int srcTime, KisKeyframeSP dstFrame) = 0;

    virtual void requestUpdate(const KisFrameSet &range, const QRect &rect);

    virtual KisKeyframeSP loadKeyframe(const QDomElement &keyframeNode) = 0;
    virtual void saveKeyframe(KisKeyframeSP keyframe, QDomElement keyframeElement, const QString &layerFilename) = 0;

    void workaroundBrokenFrameTimeBug(int *time);

private:
    KisKeyframeBaseSP replaceKeyframeAt(int time, KisKeyframeBaseSP newKeyframe);
    void insertKeyframeLogical(KisKeyframeBaseSP keyframe);
    void removeKeyframeLogical(KisKeyframeBaseSP keyframe);
    bool deleteKeyframeImpl(KisKeyframeBaseSP keyframe, KUndo2Command *parentCommand, bool recreate);
    void moveKeyframeImpl(KisKeyframeBaseSP keyframe, int newTime);
    void swapKeyframesImpl(KisKeyframeBaseSP lhsKeyframe, KisKeyframeBaseSP rhsKeyframe);

    void addCycle(QSharedPointer<KisAnimationCycle> cycle);
    void removeCycle(QSharedPointer<KisAnimationCycle> cycle);

    friend class KisMoveFrameCommand;
    friend class KisReplaceKeyframeCommand;
    friend class KisSwapFramesCommand;
    friend class KisDefineCycleCommand;

private:
    KisKeyframeSP insertKeyframe(int time, const KisKeyframeBaseSP copySrc, KUndo2Command *parentCommand);
    QSharedPointer<KisAnimationCycle> loadCycle(const QDomElement &cycleElement);

    struct Private;
    QScopedPointer<Private> m_d;
};

class KisVisibleKeyframeIterator
{
public:
    KisVisibleKeyframeIterator();
    explicit KisVisibleKeyframeIterator(KisKeyframeSP keyframe);

    KisVisibleKeyframeIterator& operator--();
    KisVisibleKeyframeIterator& operator++();

    bool isValid() const;
    KisKeyframeSP operator->() const;
    KisKeyframeSP operator*() const;

private:
    KisVisibleKeyframeIterator &invalidate();

    KisKeyframeChannel *m_channel{nullptr};
    KisKeyframeSP m_keyframe;
    int m_time{-1};
};

#endif // KIS_KEYFRAME_CHANNEL_H
