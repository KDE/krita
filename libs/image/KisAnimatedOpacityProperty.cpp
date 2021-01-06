#include "KisAnimatedOpacityProperty.h"

KisAnimatedOpacityProperty::KisAnimatedOpacityProperty(KoProperties * const props, quint8 defaultValue, QObject *parent)
    : QObject(parent),
      m_props(props),
      m_defaultValue(defaultValue)
{
}

quint8 KisAnimatedOpacityProperty::get() {
    QVariant variant;
    bool ok = m_props->property("opacity", variant);
    const quint8 value =  ok ? variant.toInt() : m_defaultValue;

    if (m_channel) {
        qreal chanValue = m_channel->currentValue();
        if (!qIsNaN(chanValue)){
            return (chanValue * 255 / 100);
        }
    }

    return value;
}

void KisAnimatedOpacityProperty::set(const quint8 value) {
    quint8 to_assign;
    if (m_channel) {
        const int time = m_channel->activeKeyframeTime();
        const int translatedOldValue = m_channel->keyframeAt<KisScalarKeyframe>(time)->value() * 255 / 100;

        if (translatedOldValue == value) {
            return;
        }

        m_channel->keyframeAt<KisScalarKeyframe>(time)->setValue(value * 100 / 255 );

        to_assign = m_channel->currentValue() * 255 / 100;
    } else {
        to_assign = value;
    }

    if (m_props->intProperty("opacity", m_defaultValue) == to_assign) {
        return;
    }

    m_props->setProperty("opacity", to_assign);

    emit changed(to_assign);
}

void KisAnimatedOpacityProperty::makeAnimated(KisNode *parentNode) {
    m_channel.reset( new KisScalarKeyframeChannel(
                         KisKeyframeChannel::Opacity,
                         new KisDefaultBoundsNodeWrapper(parentNode)
                         ));

    m_channel->setNode(parentNode);
    m_channel->setDefaultBounds(new KisDefaultBoundsNodeWrapper(parentNode));
    m_channel->setLimits(0, 100);
    m_channel->setDefaultInterpolationMode(KisScalarKeyframe::Linear);
    m_channel->setDefaultValue(100);

    connect(m_channel.data(), SIGNAL(sigKeyframeChanged(const KisKeyframeChannel*,int)), this, SLOT(slotKeyChanged(const KisKeyframeChannel*,int)));
}

void KisAnimatedOpacityProperty::transferKeyframeData(const KisAnimatedOpacityProperty &rhs){
    KisScalarKeyframeChannel* channel = rhs.channel();
    KIS_ASSERT_RECOVER(channel) {}
    KisScalarKeyframeChannel* channelNew = new KisScalarKeyframeChannel(*channel);
    KIS_ASSERT(channelNew);
    m_channel.reset(channelNew);

    connect(m_channel.data(), SIGNAL(sigKeyframeChanged(const KisKeyframeChannel*,int)), this, SLOT(slotKeyChanged(const KisKeyframeChannel*,int)));
}

void KisAnimatedOpacityProperty::slotKeyChanged(const KisKeyframeChannel *chan, int time) {
    Q_UNUSED(chan);

    if (m_channel->isCurrentTimeAffectedBy(time)) {
        emit changed(m_channel->currentValue() * 255 / 100);
    }
}
