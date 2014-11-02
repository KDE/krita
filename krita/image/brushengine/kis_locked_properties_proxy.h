/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
 *  Copyright (c) 2014 Mohit Goyal <mohit.bits2011@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef KIS_LOCKED_PROPERTIES_PROXY_H
#define KIS_LOCKED_PROPERTIES_PROXY_H

#include "kis_locked_properties.h"
#include "kis_properties_configuration.h"
#include "kis_locked_properties_server.h"
#include "kis_paintop_preset.h"
#include "kis_paintop_settings.h"

/**
 * This class acts as a proxy for all transfers between KisLockedPropertiesServer and KisPaintOpSettings
 * while using setConfiguration and writeConfiguration methods. It is used to override the local settings of a
 * paintop with the settings present in the KisLockedProperties List.
 * Settings with the "_previous" suffix are used to save the local settings of a preset in the preset itself.
 * Whenever the user selects the option of going back to the previous configuration of the preset or "unlocks" an option -- these settings are restored
 * and the settings with the "_previous" suffix are destroyed.
 */
class KisLockedPropertiesServer;

class KisLockedPropertiesProxy: public KisPropertiesConfiguration
{
public:
    KisLockedPropertiesProxy() ;
    KisLockedPropertiesProxy(KisLockedProperties* p);
    KisLockedPropertiesProxy(const KisPropertiesConfiguration *, KisLockedProperties *);
    using KisPropertiesConfiguration::getProperty;
    QVariant getProperty(const QString &name) const;
    using KisPropertiesConfiguration::setProperty;
    void setProperty(const QString & name, const QVariant & value);
private:
    KisLockedProperties* m_lockedProperties;
    const KisPropertiesConfiguration* m_parent;
};

#endif // KIS_LOCKED_PROPERTIES_PROXY_H
