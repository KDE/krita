#include "kis_keyframe_sequence.h"

#include <QHash>

struct KisKeyframeSequence::Private
{
    QHash<QString, KisKeyframeChannel*> channels;
};

KisKeyframeSequence::KisKeyframeSequence()
    : m_d(new Private)
{
}

KisKeyframeSequence::~KisKeyframeSequence()
{
    QHash<QString, KisKeyframeChannel*>::const_iterator it = m_d->channels.constBegin();
    while (it != m_d->channels.constEnd()) {
        delete it.value();
        ++it;
    }
}

KisKeyframeChannel * KisKeyframeSequence::createChannel(const QString& name, const QString& displayName)
{
    KisKeyframeChannel *channel = new KisKeyframeChannel(name, displayName);

    m_d->channels.insert(name, channel);

    return channel;
}

KisKeyframeChannel* KisKeyframeSequence::getChannel(const QString& name)
{
    if (m_d->channels.contains(name)) {
        return m_d->channels[name];
    } else {
        return 0;
    }
}
