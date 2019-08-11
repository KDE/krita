/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
 *  Copyright (c) 2014 Mohit Goyal <mohit.bits2011@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2.1 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include <brushengine/kis_locked_properties_server.h>
#include <brushengine/kis_locked_properties.h>

#include <QGlobalStatic>

Q_GLOBAL_STATIC(KisLockedPropertiesServer, s_instance)



KisLockedPropertiesServer::KisLockedPropertiesServer()
{
    m_lockedProperties = new KisLockedProperties();
    m_propertiesFromLocked = false;
}

KisLockedPropertiesServer::~KisLockedPropertiesServer()
{
}

KisLockedPropertiesProxySP KisLockedPropertiesServer::createLockedPropertiesProxy(KisPropertiesConfiguration *settings)
{
    return new KisLockedPropertiesProxy(settings, lockedProperties());
}

KisLockedPropertiesProxySP KisLockedPropertiesServer::createLockedPropertiesProxy(KisPropertiesConfigurationSP settings)
{
    return createLockedPropertiesProxy(settings.data());
}

KisLockedPropertiesServer* KisLockedPropertiesServer::instance()
{
    if (s_instance) {
        return s_instance;
    }

    return NULL;
}

KisLockedPropertiesSP KisLockedPropertiesServer::lockedProperties()
{
    return m_lockedProperties;
}

void KisLockedPropertiesServer::addToLockedProperties(KisPropertiesConfigurationSP p)
{
    lockedProperties()->addToLockedProperties(p);
}

void KisLockedPropertiesServer::removeFromLockedProperties(KisPropertiesConfigurationSP p)
{
    lockedProperties()->removeFromLockedProperties(p);
}

void KisLockedPropertiesServer::setPropertiesFromLocked(bool value)
{
    m_propertiesFromLocked = value;
}

bool KisLockedPropertiesServer::propertiesFromLocked()
{
    return m_propertiesFromLocked;
}
bool KisLockedPropertiesServer::hasProperty(const QString &p)
{
    return m_lockedProperties->hasProperty(p);
}



