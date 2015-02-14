#include "kis_keyframe_channel.h"

#include <QMap>

struct KisKeyframeChannel::Private
{
    QMap<int, QVariant> keys;
};

KisKeyframeChannel::KisKeyframeChannel(const QString& name, const QString& displayName)
    : m_d(new Private)
{
}

void KisKeyframeChannel::setKeyframe(int time, const QVariant& value)
{
    m_d->keys.insert(time, value);
}

void KisKeyframeChannel::deleteKeyframe(int time)
{
    m_d->keys.remove(time);
}

QVariant KisKeyframeChannel::getValueAt(int time)
{
    QMap<int, QVariant>::iterator nextKeyframe = m_d->keys.upperBound(time);

    if (nextKeyframe == m_d->keys.begin()) return QVariant();

    nextKeyframe--;
    return nextKeyframe.value();
}

QList<int> KisKeyframeChannel::times()
{
    return m_d->keys.keys();
}
