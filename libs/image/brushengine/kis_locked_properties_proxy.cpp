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
#include <brushengine/kis_locked_properties_proxy.h>
#include <brushengine/kis_locked_properties.h>
#include <brushengine/kis_locked_properties_server.h>
#include <brushengine/kis_paintop_settings.h>
#include <brushengine/kis_paintop_preset.h>

KisLockedPropertiesProxy ::KisLockedPropertiesProxy()
    : m_lockedProperties(0)
    , m_parent(0)
{
}

KisLockedPropertiesProxy::KisLockedPropertiesProxy(KisLockedProperties* p)
{
    m_lockedProperties = p;
}

KisLockedPropertiesProxy::KisLockedPropertiesProxy(const KisPropertiesConfiguration *p, KisLockedProperties *l)
{
    m_lockedProperties = l;
    m_parent = p;
}

QVariant KisLockedPropertiesProxy::getProperty(const QString &name) const
{
    KisPropertiesConfiguration* temp = const_cast<KisPropertiesConfiguration*>(m_parent);
    KisPaintOpSettings* t = dynamic_cast<KisPaintOpSettings*>(temp);

    if (t->preset()) {
        // restores the dirty state on returns automagically
        KisPaintOpPreset::DirtyStateSaver dirtyStateSaver(t->preset().data());

        if (m_lockedProperties->lockedProperties()) {
            if (m_lockedProperties->lockedProperties()->hasProperty(name)) {
                KisLockedPropertiesServer::instance()->setPropertiesFromLocked(true);

                if (!m_parent->hasProperty(name + "_previous")) {
                    temp->setProperty(name + "_previous", m_parent->getProperty(name));
                }
                temp->setProperty(name, m_lockedProperties->lockedProperties()->getProperty(name));
                return m_lockedProperties->lockedProperties()->getProperty(name);
            } else {
                if (m_parent->hasProperty(name + "_previous")) {
                    KisPropertiesConfiguration* temp = const_cast<KisPropertiesConfiguration*>(m_parent);
                    temp->setProperty(name, m_parent->getProperty(name + "_previous"));
                    temp->removeProperty(name + "_previous");
                }
            }
        }
    }
    return m_parent->getProperty(name);
}

void KisLockedPropertiesProxy::setProperty(const QString & name, const QVariant & value)
{
    KisPropertiesConfiguration* temp = const_cast<KisPropertiesConfiguration*>(m_parent);
    KisPaintOpSettings* t = dynamic_cast<KisPaintOpSettings*>(temp);
    if (t->preset()) {
        // restores the dirty state on returns automagically
        KisPaintOpPreset::DirtyStateSaver dirtyStateSaver(t->preset().data());

        if (m_lockedProperties->lockedProperties()) {
            if (m_lockedProperties->lockedProperties()->hasProperty(name)) {
                m_lockedProperties->lockedProperties()->setProperty(name, value);
                t->setProperty(name, value);
                if (!m_parent->hasProperty(name + "_previous")) {
                    t->setProperty(name + "_previous", m_parent->getProperty(name));
                }
                return;
            }
        }
    }
    t->setProperty(name, value);
}





