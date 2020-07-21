/*
 *  Copyright (c) 2015 Jouni Pentikäinen <joupent@gmail.com>
 *  Copyright (c) 2020 Emmet O'Neill <emmetoneill.pdx@gmail.com>
 *  Copyright (c) 2020 Eoin O'Neill <eoinoneill1991@gmail.com>
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
#include "kis_keyframe.h"
#include "kis_default_bounds.h"
#include "kis_default_bounds_node_wrapper.h"

#include "kritaimage_export.h"

class KisTimeSpan;


/**
 * KisKeyframeChannel stores and manages KisKeyframes.
 * Maps units of time to virtual keyframe values.
 * This class is a key piece of Krita's animation backend.
 */
class KRITAIMAGE_EXPORT KisKeyframeChannel : public QObject
{
    Q_OBJECT

public:
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

    Q_DECL_DEPRECATED KisKeyframeChannel(const KoID& id, KisNodeWSP parent = 0);
    KisKeyframeChannel(const KoID& id, KisDefaultBoundsBaseSP bounds);
    KisKeyframeChannel(const KisKeyframeChannel &rhs, KisNodeWSP newParent);
    ~KisKeyframeChannel() override;

    void addKeyframe(int time, KUndo2Command *parentCmd = nullptr);
    void insertKeyframe(int time, KisKeyframeSP keyframe, KUndo2Command *parentCmd = nullptr);
    void removeKeyframe(int time, KUndo2Command *parentCmd = nullptr);

    // Inter-channel operations..
    static void moveKeyframe(KisKeyframeChannel *sourceChannel, int sourceTime, KisKeyframeChannel *targetChannel, int targetTime, KUndo2Command* parentCmd = nullptr);
    static void copyKeyframe(KisKeyframeChannel *sourceChannel, int sourceTime, KisKeyframeChannel *targetChannel, int targetTime, KUndo2Command* parentCmd = nullptr);
    static void swapKeyframes(KisKeyframeChannel *channelA, int timeA, KisKeyframeChannel *channelB, int timeB, KUndo2Command* parentCmd = nullptr);

    // Intra-channel convenience methods..
    void moveKeyframe(int sourceTime, int targetTime, KUndo2Command* parentCmd = nullptr) { moveKeyframe(this, sourceTime, this, targetTime, parentCmd); }
    void copyKeyframe(int sourceTime, int targetTime, KUndo2Command* parentCmd = nullptr) { copyKeyframe(this, sourceTime, this, targetTime, parentCmd); }
    void swapKeyframes(int timeA, int timeB, KUndo2Command* parentCmd = nullptr) { swapKeyframes(this, timeA, this, timeB, parentCmd); }

    KisKeyframeSP keyframeAt(int time) const;
    KisKeyframeSP activeKeyframeAt(int time) const { return keyframeAt(activeKeyframeTime(time)); }

    // Convenience templates..
    template <class KeyframeType>
    QSharedPointer<KeyframeType> keyframeAt(int time) const {
        return keyframeAt(time).dynamicCast<KeyframeType>();
    }

    template <class KeyframeType>
    QSharedPointer<KeyframeType> activeKeyframeAt(int time) const {
        return activeKeyframeAt(time).dynamicCast<KeyframeType>();
    }

    int activeKeyframeTime(int time) const;
    int activeKeyframeTime() const { return activeKeyframeTime(currentTime()); }

    int firstKeyframeTime() const;
    int previousKeyframeTime(const int time) const;
    int nextKeyframeTime(const int time) const;
    int lastKeyframeTime() const;

    QString id() const;
    QString name() const;

    Q_DECL_DEPRECATED void setNode(KisNodeWSP node);
    Q_DECL_DEPRECATED KisNodeWSP node() const;

    int keyframeCount() const;
    QSet<int> allKeyframeTimes() const;

    /**
     * Calculates a pseudo-unique keyframes hash. The hash changes
     * every time any frame is added/removed/moved
     */
    int framesHash() const;

    /**
     * Get the set of frames affected by any changes to the value
     * of the active keyframe at the given time.
     */
    KisTimeSpan affectedFrames(int time) const; //TEMP NOTE: scalar specific?

    /**
     * Get a set of frames for which the channel gives identical
     * results, compared to the given frame.
     *
     * Note: this set may be different than the set of affected frames
     * due to interpolation.
     */
    KisTimeSpan identicalFrames(int time) const; //TEMP NOTE: scalar specific?

    virtual QDomElement toXML(QDomDocument doc, const QString &layerFilename);
    virtual void loadXML(const QDomElement &channelNode);

Q_SIGNALS:
    void sigUpdated(const KisTimeSpan &affectedTimeSpan, const QRect &affectedArea);

    void sigKeyframeAboutToBeAdded(const KisKeyframeChannel *channel, KisKeyframeSP keyframe);
    void sigKeyframeAdded(const KisKeyframeChannel *channel,KisKeyframeSP keyframe, int index);
    void sigKeyframeAboutToBeRemoved(const KisKeyframeChannel *channel, KisKeyframeSP keyframe, int index);
    void sigKeyframeRemoved(const KisKeyframeChannel *channel, KisKeyframeSP keyframe);
    void sigKeyframeAboutToBeMoved(const KisKeyframeChannel *channel, KisKeyframeSP keyframe, int toTime);
    void sigKeyframeMoved(const KisKeyframeChannel *channel, KisKeyframeSP keyframe, int fromTime, int toTime);
    void sigKeyframeChanged(const KisKeyframeChannel *channel, KisKeyframeSP keyframe, int index);

protected:
    typedef QMap<int, KisKeyframeSP> KeyframesMap;
    KeyframesMap &keys();
    const KeyframesMap &constKeys() const;

    int currentTime() const;

    Q_DECL_DEPRECATED virtual void requestUpdate(const KisTimeSpan &range, const QRect &rect);

    Q_DECL_DEPRECATED void workaroundBrokenFrameTimeBug(int *time); //TEMP NOTE: scalar specific?

private:
    struct Private;
    QScopedPointer<Private> m_d;

    KeyframesMap::const_iterator activeKeyIterator(int time) const;

    virtual KisKeyframeSP createKeyframe() = 0;
    virtual QRect affectedRect(int time) = 0;
    virtual QPair<int, KisKeyframeSP> loadKeyframe(const QDomElement &keyframeNode) = 0;
    virtual void saveKeyframe(KisKeyframeSP keyframe, QDomElement keyframeElement, const QString &layerFilename) = 0;
};

#endif // KIS_KEYFRAME_CHANNEL_H
