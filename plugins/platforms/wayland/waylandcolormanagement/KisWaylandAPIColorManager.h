/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISWAYLANDAPICOLORMANAGER_H
#define KISWAYLANDAPICOLORMANAGER_H

#include <QWaylandClientExtension>
#include <qwayland-color-management-v1.h>

#include <QSet>

class KisWaylandAPIColorManager
    : public QWaylandClientExtensionTemplate<KisWaylandAPIColorManager, &QtWayland::wp_color_manager_v1::destroy>
    , public QtWayland::wp_color_manager_v1
{
    Q_OBJECT
public:
    explicit KisWaylandAPIColorManager();
    ~KisWaylandAPIColorManager();

    bool isReady() const {
        return m_isReady;
    }

    bool isFeatureSupported(feature f) const
    {
        return m_supportedFeatures.contains(f);
    }

    bool isIntentSupported(render_intent ri) const
    {
        return m_supportedIntents.contains(ri);
    }

    bool isTransferFunctionNamedSupported(transfer_function tf) const
    {
        return m_supportedTransferFunctionsNamed.contains(tf);
    }

    bool isPrimariesNamedSupported(primaries p) const
    {
        return m_supportedPrimariesNamed.contains(p);
    }

Q_SIGNALS:
    void sigReadyChanged(bool value);

protected:
    void wp_color_manager_v1_supported_intent(uint32_t _render_intent) override;
    void wp_color_manager_v1_supported_feature(uint32_t _feature) override;
    void wp_color_manager_v1_supported_tf_named(uint32_t tf) override;
    void wp_color_manager_v1_supported_primaries_named(uint32_t _primaries) override;
    void wp_color_manager_v1_done() override;

private:
    QSet<render_intent> m_supportedIntents;
    QSet<feature> m_supportedFeatures;
    QSet<transfer_function> m_supportedTransferFunctionsNamed;
    QSet<primaries> m_supportedPrimariesNamed;
    bool m_isReady {false};
};

#endif /* KISWAYLANDAPICOLORMANAGER_H */