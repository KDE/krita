#ifndef KIS_PROPERTY_WRAPPER_H
#define KIS_PROPERTY_WRAPPER_H

#include <QObject>
#include <QPointer>
#include <QVariant>

#include "kis_scalar_keyframe_channel.h"
#include "kis_time_span.h"
#include "kis_image.h"
#include "KoProperties.h"

#include "kritawidgetutils_export.h"

class KRITAWIDGETUTILS_EXPORT KisAnimatedOpacityProperty : public QObject {
    Q_OBJECT
public:
    KisAnimatedOpacityProperty(KoProperties* const props, quint8 defaultValue, QObject *parent = nullptr)
        : QObject(parent),
          m_props(props),
          m_defaultValue(defaultValue)
    {
    }

    quint8 get() {
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

    void set(const quint8& value) {
        if (m_channel) {
            const int time = m_channel->activeKeyframeTime();
            const int translatedOldValue = m_channel->keyframeAt<KisScalarKeyframe>(time)->value() * 255 / 100;

            if (translatedOldValue == value) {
                return;
            }

            m_channel->keyframeAt<KisScalarKeyframe>(time)->setValue(value * 100 / 255 );
        } else {

            if (m_props->intProperty("opacity", m_defaultValue) == value) {
                return;
            }

            m_props->setProperty("opacity", value);
        }

        emit changed(value);
    }

    bool hasChannel() {
        return !m_channel.isNull();
    }

    void makeAnimated(KisNode* parentNode) {
        m_channel.reset( new KisScalarKeyframeChannel(
                             KisKeyframeChannel::Opacity,
                             new KisDefaultBoundsNodeWrapper(parentNode)
                         ));

        m_channel->setNode(parentNode);
        m_channel->setBounds(new KisDefaultBoundsNodeWrapper(parentNode));
        m_channel->setLimits(0, 100);
        m_channel->setDefaultInterpolationMode(KisScalarKeyframe::Linear);
        m_channel->setDefaultValue(100);

        connect(m_channel.data(), SIGNAL(sigKeyframeChanged(const KisKeyframeChannel*,int)), this, SLOT(slotKeyChanged(const KisKeyframeChannel*,int)));
    }

    void transferKeyframeData(const KisAnimatedOpacityProperty &rhs){
        KisScalarKeyframeChannel* channel = rhs.channel();
        KIS_ASSERT_RECOVER(channel) {}
        KisScalarKeyframeChannel* channelNew = new KisScalarKeyframeChannel(*channel);
        KIS_ASSERT(channelNew);
        m_channel.reset(channelNew);

        connect(m_channel.data(), SIGNAL(sigKeyframeChanged(const KisKeyframeChannel*,int)), this, SLOT(slotKeyChanged(const KisKeyframeChannel*,int)));
    }

    KisScalarKeyframeChannel* channel() const { return m_channel.data(); }

Q_SIGNALS:
    void changed(quint8 value);

public Q_SLOTS:
    void slotKeyChanged(const KisKeyframeChannel* chan, int time) {
        Q_UNUSED(chan);

        if (m_channel->isCurrentTimeAffectedBy(time)) {
            emit changed(m_channel->currentValue() * 255 / 100);
        }
    }

private:
    KoProperties* const m_props;
    QScopedPointer<KisScalarKeyframeChannel> m_channel;
    quint8 m_defaultValue;
};

#endif // KISLAMBDAPROPERTY_H
