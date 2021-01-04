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
    KisAnimatedOpacityProperty(KoProperties* const props, quint8 defaultValue, QObject *parent = nullptr);

    quint8 get();
    void set(const quint8 value);

    bool hasChannel() { return !m_channel.isNull(); }
    KisScalarKeyframeChannel* channel() const { return m_channel.data(); }

    void makeAnimated(KisNode* parentNode);
    void transferKeyframeData(const KisAnimatedOpacityProperty &rhs);

Q_SIGNALS:
    void changed(quint8 value);

public Q_SLOTS:
    void slotKeyChanged(const KisKeyframeChannel* chan, int time);

private:
    KoProperties* const m_props;
    QScopedPointer<KisScalarKeyframeChannel> m_channel;
    quint8 m_defaultValue;
};

#endif // KISLAMBDAPROPERTY_H
