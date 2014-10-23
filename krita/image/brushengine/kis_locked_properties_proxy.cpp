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
#include "kis_locked_properties_proxy.h"

KisLockedPropertiesProxy ::KisLockedPropertiesProxy()
{
    m_lockedProperties = NULL;
    m_parent = NULL;

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

    bool saveDirtyState = false;
    if (t->preset()) {
        t->preset()->isPresetDirty();

        if (m_lockedProperties->lockedProperties()) {
            if (m_lockedProperties->lockedProperties()->hasProperty(name)) {
                KisLockedPropertiesServer::instance()->setPropertiesFromLocked(true);

                if (!m_parent->hasProperty(name + "_previous")) {
                    temp->setProperty(name + "_previous", m_parent->getProperty(name));
                }
                temp->setProperty(name, m_lockedProperties->lockedProperties()->getProperty(name));
                t->preset()->setPresetDirty(saveDirtyState);
                return m_lockedProperties->lockedProperties()->getProperty(name);
            } else {
                if (m_parent->hasProperty(name + "_previous")) {
                    KisPropertiesConfiguration* temp = const_cast<KisPropertiesConfiguration*>(m_parent);
                    temp->setProperty(name, m_parent->getProperty(name + "_previous"));
                    temp->removeProperty(name + "_previous");
                }
            }
        }

        t->preset()->setPresetDirty(saveDirtyState);
    }
    return m_parent->getProperty(name);
}
void KisLockedPropertiesProxy::setProperty(const QString & name, const QVariant & value)
{
    KisPropertiesConfiguration* temp = const_cast<KisPropertiesConfiguration*>(m_parent);
    KisPaintOpSettings* t = dynamic_cast<KisPaintOpSettings*>(temp);
    if (t->preset()) {
        bool saveDirtyState = t->preset()->isPresetDirty();
        if (m_lockedProperties->lockedProperties()) {
            if (m_lockedProperties->lockedProperties()->hasProperty(name)) {
                m_lockedProperties->lockedProperties()->setProperty(name, value);
                t->setProperty(name, value);
                if (!m_parent->hasProperty(name + "_previous")) {
                    t->setProperty(name + "_previous", m_parent->getProperty(name));
                }
                t->preset()->setPresetDirty(saveDirtyState);
                return;
            }


        }
    }
    t->setProperty(name, value);

}





