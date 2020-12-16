/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *  SPDX-FileCopyrightText: 2014 Mohit Goyal <mohit.bits2011@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include <brushengine/kis_locked_properties.h>


KisLockedProperties::KisLockedProperties()
{
    m_lockedProperties = new KisPropertiesConfiguration();
}

KisLockedProperties::~KisLockedProperties()
{
}

void KisLockedProperties::addToLockedProperties(KisPropertiesConfigurationSP p)
{
    addToLockedProperties(p.data());
}

void KisLockedProperties::addToLockedProperties(const KisPropertiesConfiguration *p)
{
    QMapIterator<QString, QVariant> i(p->getProperties());
    while (i.hasNext()) {
        i.next();
        m_lockedProperties->setProperty(i.key(), QVariant(i.value()));
    }
}

void KisLockedProperties::removeFromLockedProperties(KisPropertiesConfigurationSP p)
{
    removeFromLockedProperties(p.data());
}

void KisLockedProperties::removeFromLockedProperties(const KisPropertiesConfiguration *p)
{
    KisPropertiesConfigurationSP temp = new KisPropertiesConfiguration();
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

KisPropertiesConfigurationSP KisLockedProperties::lockedProperties()
{
    return m_lockedProperties;
}


