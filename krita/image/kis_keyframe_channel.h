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

#include "krita_export.h"
#include "kis_keyframe.h"

class KisKeyframeSequence;

class KRITAIMAGE_EXPORT KisKeyframeChannel : public QObject
{
    Q_OBJECT

public:
    KisKeyframeChannel(const QString& name, const QString& displayName, KisKeyframeSequence *sequence);
    ~KisKeyframeChannel();

    QString name() const;
    QString displayName() const;
    KisKeyframeSequence *sequence() const;

    void setKeyframe(int time, const QVariant &value);
    void deleteKeyframe(int time);
    bool hasKeyframeAt(int time);
    bool moveKeyframe(KisKeyframe *keyframe, int time);

    QVariant getValueAt(int time);

    QList<KisKeyframe*> keyframes() const;

signals:
    void sigKeyframeAboutToBeAdded(KisKeyframe *keyframe);
    void sigKeyframeAdded(KisKeyframe *keyframe);
    void sigKeyframeAboutToBeRemoved(KisKeyframe *keyframe);
    void sigKeyframeRemoved(KisKeyframe *keyframe);
    void sigKeyframeAboutToBeMoved(KisKeyframe *keyframe, int toTime);
    void sigKeyframeMoved(KisKeyframe *keyframe);

private:

    struct Private;
    Private * const m_d;
};

#endif // KIS_KEYFRAME_CHANNEL_H
