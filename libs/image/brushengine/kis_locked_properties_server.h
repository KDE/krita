/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *  SPDX-FileCopyrightText: 2014 Mohit Goyal <mohit.bits2011@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef KIS_LOCKED_PROPERTIES_SERVER_H
#define KIS_LOCKED_PROPERTIES_SERVER_H

#include <brushengine/kis_locked_properties_proxy.h>
#include "kis_properties_configuration.h"

class KisLockedPropertiesProxy;

/**
 * The KisLockedPropertiesServer class
 * This static class stores an object of KisLockedProperties and generates a KisLockedPropertiesProxy used
 * by other classes/objects to access the LockedProperties object.
 */

class KRITAIMAGE_EXPORT KisLockedPropertiesServer: public QObject
{
public:
    KisLockedPropertiesServer();
    ~KisLockedPropertiesServer() override;
    static KisLockedPropertiesServer* instance();

    KisLockedPropertiesSP lockedProperties();
    void addToLockedProperties(KisPropertiesConfigurationSP p);
    void removeFromLockedProperties(KisPropertiesConfigurationSP p);
    void setPropertiesFromLocked(bool value);
    bool propertiesFromLocked();
    KisLockedPropertiesProxySP createLockedPropertiesProxy(KisPropertiesConfiguration *settings);
    KisLockedPropertiesProxySP createLockedPropertiesProxy(KisPropertiesConfigurationSP settings);
    bool hasProperty(const QString &p);

private:

    KisLockedPropertiesSP m_lockedProperties;
    bool m_propertiesFromLocked;
};

#endif // KIS_LOCKED_PROPERTIES_SERVER_H
