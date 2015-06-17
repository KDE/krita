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

#include "kis_types.h"
#include "KoID.h"
#include "krita_export.h"
#include "kis_keyframe.h"
#include "KoXmlReader.h"

class KRITAIMAGE_EXPORT KisKeyframeChannel : public QObject
{
    Q_OBJECT

public:
    KisKeyframeChannel(const KoID& id, const KisNodeWSP node);
    ~KisKeyframeChannel();

    QString id() const;
    QString name() const;

    KisNodeWSP node() const;

    KisKeyframe *addKeyframe(int time);
    bool deleteKeyframe(KisKeyframe *keyframe);
    bool moveKeyframe(KisKeyframe *keyframe, int newTime);
    KisKeyframe *copyKeyframe(const KisKeyframe *keyframe, int newTime);

    KisKeyframe *keyframeAt(int time);
    KisKeyframe *activeKeyframeAt(int time) const;
    KisKeyframe *nextKeyframeAfter(int time) const;

    int keyframeCount() const;
    QList<KisKeyframe*> keyframes() const;

    virtual bool hasScalarValue() const = 0;
    virtual qreal minScalarValue() const = 0;
    virtual qreal maxScalarValue() const = 0;
    virtual qreal scalarValue(const KisKeyframe *keyframe) const = 0;
    virtual void setScalarValue(KisKeyframe *keyframe, qreal value) = 0;

    QDomElement toXML(QDomDocument doc) const;
    void loadXML(KoXmlNode channelNode);

signals:
    void sigKeyframeAboutToBeAdded(KisKeyframe *keyframe);
    void sigKeyframeAdded(KisKeyframe *keyframe);
    void sigKeyframeAboutToBeRemoved(KisKeyframe *keyframe);
    void sigKeyframeRemoved(KisKeyframe *keyframe);
    void sigKeyframeAboutToBeMoved(KisKeyframe *keyframe, int toTime);
    void sigKeyframeMoved(KisKeyframe *keyframe, int fromTime);

protected:
    QMap<int, KisKeyframe *> keys();
    const QMap<int, KisKeyframe *> constKeys() const;

    virtual KisKeyframe * createKeyframe(int time, const KisKeyframe *copySrc) = 0;
    virtual bool canDeleteKeyframe(KisKeyframe *key) = 0;
    virtual void destroyKeyframe(KisKeyframe *key) {}

    virtual KisKeyframe * loadKeyframe(KoXmlNode keyframeNode) = 0;
    virtual void saveKeyframe(KisKeyframe *keyframe, QDomElement keyframeElement) const = 0;

private:
    KisKeyframe * insertKeyframe(int time, const KisKeyframe *copySrc);

    struct Private;
    Private * const m_d;
};

#endif // KIS_KEYFRAME_CHANNEL_H
