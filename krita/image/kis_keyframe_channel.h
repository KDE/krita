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

#include "kis_types.h"
#include "KoID.h"
#include "krita_export.h"
#include "kis_keyframe.h"

class KisTimeRange;


class KRITAIMAGE_EXPORT KisKeyframeChannel : public QObject
{
    Q_OBJECT

public:
    // Standard Keyframe Ids

    static const KoID Content;

public:
    KisKeyframeChannel(const KoID& id, KisNodeWSP node);
    ~KisKeyframeChannel();

    QString id() const;
    QString name() const;

    KisNodeWSP node() const;

    KisKeyframe *addKeyframe(int time, KUndo2Command *parentCommand = 0);
    bool deleteKeyframe(KisKeyframe *keyframe, KUndo2Command *parentCommand = 0);
    bool moveKeyframe(KisKeyframe *keyframe, int newTime, KUndo2Command *parentCommand = 0);
    KisKeyframe *copyKeyframe(const KisKeyframe *keyframe, int newTime, KUndo2Command *parentCommand = 0);

    KisKeyframe *keyframeAt(int time);
    KisKeyframe *activeKeyframeAt(int time) const;
    KisKeyframe *nextKeyframeAfter(int time) const;

    /**
     * Get the set of frames affected by any changes to the value
     * of the active keyframe at the given time.
     */
    KisTimeRange affectedFrames(int time) const;

    /**
     * Get a set of frames for which the channel gives identical
     * results, compared to the given frame.
     *
     * Note: this set may be different than the set of affected frames
     * (eg. due to interpolation, once implemented)
     */
    KisTimeRange identicalFrames(int time) const;

    int keyframeCount() const;

    int keyframeRowIndexOf(KisKeyframe *keyframe) const;
    KisKeyframe* keyframeAtRow(int row) const;

    int keyframeInsertionRow(int time) const;

    virtual bool hasScalarValue() const = 0;
    virtual qreal minScalarValue() const = 0;
    virtual qreal maxScalarValue() const = 0;
    virtual qreal scalarValue(const KisKeyframe *keyframe) const = 0;
    virtual void setScalarValue(KisKeyframe *keyframe, qreal value, KUndo2Command *parentCommand = 0) = 0;

    virtual QDomElement toXML(QDomDocument doc, const QString &layerFilename);
    virtual void loadXML(const QDomElement &channelNode);

signals:
    void sigKeyframeAboutToBeAdded(KisKeyframe *keyframe);
    void sigKeyframeAdded(KisKeyframe *keyframe);
    void sigKeyframeAboutToBeRemoved(KisKeyframe *keyframe);
    void sigKeyframeRemoved(KisKeyframe *keyframe);
    void sigKeyframeAboutToBeMoved(KisKeyframe *keyframe, int toTime);
    void sigKeyframeMoved(KisKeyframe *keyframe, int fromTime);

protected:
    typedef QMap<int, KisKeyframeSP> KeyframesMap;

    KeyframesMap &keys();
    const KeyframesMap &constKeys() const;
    KeyframesMap::const_iterator activeKeyIterator(int time) const;

    virtual KisKeyframe* createKeyframe(int time, const KisKeyframe *copySrc, KUndo2Command *parentCommand) = 0;
    virtual bool canDeleteKeyframe(KisKeyframe *key) = 0;
    virtual void destroyKeyframe(KisKeyframe *key, KUndo2Command *parentCommand) = 0;

    virtual QRect affectedRect(KisKeyframe *key) = 0;
    virtual void requestUpdate(const KisTimeRange &range, const QRect &rect);

    virtual KisKeyframeSP loadKeyframe(const QDomElement &keyframeNode) = 0;
    virtual void saveKeyframe(KisKeyframe *keyframe, QDomElement keyframeElement, const QString &layerFilename) = 0;

private:
    void insertKeyframeImpl(KisKeyframeSP keyframe);
    void deleteKeyframeImpl(KisKeyframeSP keyframe);
    void moveKeyframeImpl(KisKeyframeSP keyframe, int newTime);

    struct InsertFrameCommand;
    struct MoveFrameCommand;

private:
    KisKeyframe * insertKeyframe(int time, const KisKeyframe *copySrc, KUndo2Command *parentCommand);

    struct Private;
    QScopedPointer<Private> m_d;
};

#endif // KIS_KEYFRAME_CHANNEL_H
