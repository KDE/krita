/*
 *  SPDX-FileCopyrightText: 2021 Eoin O 'Neill <eoinoneill1991@gmail.com>
 *  SPDX-FileCopyrightText: 2021 Emmet O 'Neill <emmetoneill.pdx@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisAnimatedOpacityProperty.h"

KisAnimatedOpacityProperty::KisAnimatedOpacityProperty(KisDefaultBoundsBaseSP bounds, KoProperties * const props, quint8 defaultValue, QObject *parent)
    : QObject(parent),
      m_bounds(bounds),
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
    quint8 valueToAssign;
    if (m_channel && m_channel->keyframeCount() > 0) {
        const int currentTime = m_bounds->currentTime();
        const float currentValue = m_channel->valueAt(currentTime);
        KisScalarKeyframeSP key = m_channel->keyframeAt<KisScalarKeyframe>(currentTime);

        if (!key) {
            m_channel->addScalarKeyframe(currentTime, currentValue);
            key = m_channel->keyframeAt<KisScalarKeyframe>(currentTime);
            KIS_ASSERT(key);
        }

        const int translatedOldValue = key->value() * 255 / 100; //0..100 -> 0..255

        if (translatedOldValue == value) {
            return;
        }

        key->setValue(qreal(value) * 100 / 255);

        valueToAssign = qreal(m_channel->currentValue()) * 255 / 100;
    } else {
        valueToAssign = value;
    }

    if (m_props->intProperty("opacity", m_defaultValue) == valueToAssign) {
        return;
    }

    m_props->setProperty("opacity", valueToAssign);
    KIS_ASSERT(valueToAssign == value); //Sanity check.
    Q_EMIT changed(valueToAssign);
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
    connect(m_channel.data(), SIGNAL(sigKeyframeHasBeenRemoved(const KisKeyframeChannel*,int)), this, SLOT(slotKeyRemoval(const KisKeyframeChannel*,int)));
}

void KisAnimatedOpacityProperty::transferKeyframeData(const KisAnimatedOpacityProperty &rhs){
    KisScalarKeyframeChannel* channel = rhs.channel();
    KIS_ASSERT_RECOVER(channel) {}
    KisScalarKeyframeChannel* channelNew = new KisScalarKeyframeChannel(*channel);
    KIS_ASSERT(channelNew);
    m_channel.reset(channelNew);
    m_channel->setDefaultBounds(m_bounds);

    connect(m_channel.data(), SIGNAL(sigKeyframeChanged(const KisKeyframeChannel*,int)), this, SLOT(slotKeyChanged(const KisKeyframeChannel*,int)));
    connect(m_channel.data(), SIGNAL(sigKeyframeHasBeenRemoved(const KisKeyframeChannel*,int)), this, SLOT(slotKeyRemoval(const KisKeyframeChannel*,int)));
}

void KisAnimatedOpacityProperty::updateDefaultBounds(KisDefaultBoundsBaseSP bounds) {
    m_bounds = bounds;
    if (m_channel) {
        m_channel->setDefaultBounds(m_bounds);
    }
}

void KisAnimatedOpacityProperty::slotKeyChanged(const KisKeyframeChannel*, int time) {

    if (m_channel->isCurrentTimeAffectedBy(time)) {
        Q_EMIT changed(m_channel->currentValue() * 255 / 100);
    }
}

void KisAnimatedOpacityProperty::slotKeyRemoval(const KisKeyframeChannel*, int )
{
    //Key removed is the last one, we should let everyone know that we'll be
    //reverting to the previous opacity value.
    //This will either be the last keyframe value or the last cached value assignment.
    if (m_channel && m_channel->keyframeCount() == 0) {
        Q_EMIT changed(m_props->intProperty("opacity", 255));
    } else {
        Q_EMIT changed(m_channel->currentValue() * 255 / 100);
    }
}
