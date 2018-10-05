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
    bool deleteKeyframe(KisKeyframeSP keyframe, KUndo2Command *parentCommand = 0);
    bool moveKeyframe(KisKeyframeSP keyframe, int newTime, KUndo2Command *parentCommand = 0);
    bool swapFrames(int lhsTime, int rhsTime, KUndo2Command *parentCommand = 0);
    KisKeyframeSP copyKeyframe(const KisKeyframeSP keyframe, int newTime, KUndo2Command *parentCommand = 0);
    virtual KisKeyframeSP linkKeyframe(const KisKeyframeSP keyframe, int newTime, KUndo2Command *parentCommand = 0);
    KisKeyframeSP copyExternalKeyframe(KisKeyframeChannel *srcChannel, int srcTime, int dstTime, KUndo2Command *parentCommand = 0);

    bool swapExternalKeyframe(KisKeyframeChannel *srcChannel, int srcTime, int dstTime, KUndo2Command *parentCommand = 0);

    KisKeyframeSP keyframeAt(int time) const;
    KisKeyframeSP activeKeyframeAt(int time) const;
    KisKeyframeSP currentlyActiveKeyframe() const;

    KisKeyframeSP firstKeyframe() const;
    KisKeyframeSP nextKeyframe(KisKeyframeSP keyframe) const;
    KisKeyframeSP previousKeyframe(KisKeyframeSP keyframe) const;
    KisKeyframeSP lastKeyframe() const;

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
    virtual KisFrameSet identicalFrames(int time) const;

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
    void sigKeyframeAboutToBeAdded(KisKeyframeSP keyframe);
    void sigKeyframeAdded(KisKeyframeSP keyframe);
    void sigKeyframeAboutToBeRemoved(KisKeyframeSP keyframe);
    void sigKeyframeRemoved(KisKeyframeSP keyframe);
    void sigKeyframeAboutToBeMoved(KisKeyframeSP keyframe, int toTime);
    void sigKeyframeMoved(KisKeyframeSP keyframe, int fromTime);
    void sigKeyframeChanged(KisKeyframeSP keyframe);

protected:
    typedef QMap<int, KisKeyframeSP> KeyframesMap;

    KeyframesMap &keys();
    const KeyframesMap &constKeys() const;
    KeyframesMap::const_iterator activeKeyIterator(int time) const;

    virtual KisKeyframeSP createKeyframe(int time, const KisKeyframeSP copySrc, KUndo2Command *parentCommand) = 0;
    virtual void destroyKeyframe(KisKeyframeSP key, KUndo2Command *parentCommand) = 0;
    virtual void uploadExternalKeyframe(KisKeyframeChannel *srcChannel, int srcTime, KisKeyframeSP dstFrame) = 0;

    virtual QRect affectedRect(KisKeyframeSP key) = 0;
    virtual void requestUpdate(const KisFrameSet &range, const QRect &rect);

    virtual KisKeyframeSP loadKeyframe(const QDomElement &keyframeNode) = 0;
    virtual void saveKeyframe(KisKeyframeSP keyframe, QDomElement keyframeElement, const QString &layerFilename) = 0;

    void workaroundBrokenFrameTimeBug(int *time);

private:
    KisKeyframeSP replaceKeyframeAt(int time, KisKeyframeSP newKeyframe);
    void insertKeyframeLogical(KisKeyframeSP keyframe);
    void removeKeyframeLogical(KisKeyframeSP keyframe);
    bool deleteKeyframeImpl(KisKeyframeSP keyframe, KUndo2Command *parentCommand, bool recreate);
    void moveKeyframeImpl(KisKeyframeSP keyframe, int newTime);
    void swapKeyframesImpl(KisKeyframeSP lhsKeyframe, KisKeyframeSP rhsKeyframe);

    friend class KisMoveFrameCommand;
    friend class KisReplaceKeyframeCommand;
    friend class KisSwapFramesCommand;

private:
    KisKeyframeSP insertKeyframe(int time, const KisKeyframeSP copySrc, KUndo2Command *parentCommand);

    struct Private;
    QScopedPointer<Private> m_d;
};

#endif // KIS_KEYFRAME_CHANNEL_H
