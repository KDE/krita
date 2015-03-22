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
#include "kis_locked_properties.h"


KisLockedProperties::KisLockedProperties()
{
    m_lockedProperties = new KisPropertiesConfiguration();
}

KisLockedProperties::KisLockedProperties(KisPropertiesConfiguration *p)
{
    m_lockedProperties = new KisPropertiesConfiguration();
    QMap<QString, QVariant>::Iterator i;
    for (i = p->getProperties().begin(); i != p->getProperties().end(); i++) {
        m_lockedProperties->setProperty(i.key(), i.value());
    }
}
void KisLockedProperties::addToLockedProperties(KisPropertiesConfiguration *p)
{
    QMapIterator<QString, QVariant> i(p->getProperties());
    while (i.hasNext()) {
        i.next();
        m_lockedProperties->setProperty(i.key(), QVariant(i.value()));
    }
}
void KisLockedProperties::removeFromLockedProperties(KisPropertiesConfiguration *p)
{
    KisPropertiesConfiguration *temp = new KisPropertiesConfiguration();
    QMapIterator<QString, QVariant> i(m_lockedProperties->getProperties());
    while (i.hasNext()) {
        i.next();
        temp->setProperty(i.key(), QVariant(i.value()));
    }
    m_lockedProperties->clearProperties();
    QMapIterator<QString, QVariant> j(temp->getProperties());
    while (j.hasNext()) {
        j.next();
        if (!p->hasProperty(j.key())) {
            m_lockedProperties->setProperty(j.key(), QVariant(j.value()));
        }

    }
}
bool KisLockedProperties::hasProperty(const QString &p)
{
    return m_lockedProperties->hasProperty(p);
}

KisPropertiesConfiguration *KisLockedProperties::lockedProperties()
{
    return m_lockedProperties;
}


