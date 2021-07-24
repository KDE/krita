/*
 *  SPDX-FileCopyrightText: 2021 Eoin O 'Neill <eoinoneill1991@gmail.com>
 *  SPDX-FileCopyrightText: 2021 Emmet O 'Neill <emmetoneill.pdx@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_PROPERTY_WRAPPER_H
#define KIS_PROPERTY_WRAPPER_H

#include <QObject>
#include <QPointer>
#include <QVariant>

#include "kis_scalar_keyframe_channel.h"
#include "kis_time_span.h"
#include "kis_image.h"
#include "KoProperties.h"

#include "kritaimage_export.h"

class KRITAIMAGE_EXPORT KisAnimatedOpacityProperty : public QObject {
    Q_OBJECT
public:
    KisAnimatedOpacityProperty(KoProperties* const props, quint8 defaultValue, QObject *parent = nullptr);

    quint8 get();
    void set(const quint8 value);

    bool hasChannel() { return !m_channel.isNull(); }
    KisScalarKeyframeChannel* channel() const { return m_channel.data(); }

    void makeAnimated(KisNode* parentNode);
    void transferKeyframeData(const KisAnimatedOpacityProperty &rhs, KisBaseNode* node);

Q_SIGNALS:
    void changed(quint8 value);

public Q_SLOTS:
    void slotKeyChanged(const KisKeyframeChannel*, int time);
    void slotKeyRemoval(const KisKeyframeChannel*, int);

private:
    KoProperties* const m_props;
    QScopedPointer<KisScalarKeyframeChannel> m_channel;
    quint8 m_defaultValue;
};

#endif // KISLAMBDAPROPERTY_H
