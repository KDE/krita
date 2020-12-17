/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *  SPDX-FileCopyrightText: 2014 Mohit Goyal <mohit.bits2011@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */
#ifndef KIS_LOCKED_PROPERTIES_PROXY_H
#define KIS_LOCKED_PROPERTIES_PROXY_H

#include "kis_properties_configuration.h"

/**
 * This class acts as a proxy for all transfers between KisLockedPropertiesServer
 * and KisPaintOpSettings while using setConfiguration and writeConfiguration
 * methods. It is used to override the local settings of a * paintop with the
 * settings present in the KisLockedProperties List.
 *
 * Settings with the "_previous" suffix are used to save the local settings of
 * a preset in the preset itself. Whenever the user selects the option of going
 *  back to the previous configuration of the preset or "unlocks" an option
 * -- these settings are restored and the settings with the "_previous"
 * suffix are destroyed.
 */
class KisLockedPropertiesServer;

class KisLockedPropertiesProxy: public KisPropertiesConfiguration
{
public:
    KisLockedPropertiesProxy(KisPropertiesConfiguration *, KisLockedPropertiesSP);
    ~KisLockedPropertiesProxy() override;

    using KisPropertiesConfiguration::getProperty;
    QVariant getProperty(const QString &name) const override;
    using KisPropertiesConfiguration::setProperty;
    void setProperty(const QString & name, const QVariant & value) override;

    bool hasProperty(const QString& name) const override;

    QList<QString> getPropertiesKeys() const override;

private:
    Q_DISABLE_COPY(KisLockedPropertiesProxy)
    mutable KisLockedPropertiesSP m_lockedProperties;
    KisPropertiesConfiguration* m_parent;
};

typedef KisPinnedSharedPtr<KisLockedPropertiesProxy> KisLockedPropertiesProxySP;
typedef KisWeakSharedPtr<KisLockedPropertiesProxy> KisLockedPropertiesProxyWSP;

#endif // KIS_LOCKED_PROPERTIES_PROXY_H
