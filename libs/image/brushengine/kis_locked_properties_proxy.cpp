/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *  SPDX-FileCopyrightText: 2014 Mohit Goyal <mohit.bits2011@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include <brushengine/kis_locked_properties_proxy.h>

#include <KoResource.h>
#include <KisDirtyStateSaver.h>

#include <brushengine/kis_locked_properties.h>
#include <brushengine/kis_locked_properties_server.h>
#include <brushengine/kis_paintop_settings.h>
#include <brushengine/kis_paintop_preset.h>


KisLockedPropertiesProxy::KisLockedPropertiesProxy(KisPropertiesConfiguration *p, KisLockedPropertiesSP l)
{
    m_parent = p;
    m_lockedProperties = l;
}

KisLockedPropertiesProxy::~KisLockedPropertiesProxy()
{
}

QVariant KisLockedPropertiesProxy::getProperty(const QString &name) const
{
    KisPaintOpSettings *t = dynamic_cast<KisPaintOpSettings*>(m_parent);
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(t, m_parent->getProperty(name));
    if (!t->updateListener()) return m_parent->getProperty(name);

    // restores the dirty state on returns automagically
    KisPaintOpSettings::UpdateListenerSP updateProxy = t->updateListener().toStrongRef();
    KisDirtyStateSaver<KisPaintOpSettings::UpdateListenerSP> dirtyStateSaver(updateProxy);

    if (m_lockedProperties->lockedProperties()) {
        if (m_lockedProperties->lockedProperties()->hasProperty(name)) {
            KisLockedPropertiesServer::instance()->setPropertiesFromLocked(true);

            if (!m_parent->hasProperty(name + "_previous")) {
                m_parent->setProperty(name + "_previous", m_parent->getProperty(name));
                m_parent->setPropertyNotSaved(name + "_previous");
            }

            const QVariant lockedProp = m_lockedProperties->lockedProperties()->getProperty(name);

            if (m_parent->getProperty(name) != lockedProp) {
                m_parent->setProperty(name, lockedProp);
            }

            return lockedProp;
        } else {
            if (m_parent->hasProperty(name + "_previous")) {
                m_parent->setProperty(name, m_parent->getProperty(name + "_previous"));
                m_parent->removeProperty(name + "_previous");
            }
        }
    }

    return m_parent->getProperty(name);
}

void KisLockedPropertiesProxy::setProperty(const QString & name, const QVariant & value)
{
    KisPaintOpSettings *t = dynamic_cast<KisPaintOpSettings*>(m_parent);
    KIS_SAFE_ASSERT_RECOVER_RETURN(t);
    if (!t->updateListener()) return;

    if (m_lockedProperties->lockedProperties()) {
        if (m_lockedProperties->lockedProperties()->hasProperty(name)) {
            m_lockedProperties->lockedProperties()->setProperty(name, value);
            m_parent->setProperty(name, value);

            if (!m_parent->hasProperty(name + "_previous")) {
                // restores the dirty state on returns automagically
                KisPaintOpSettings::UpdateListenerSP updateProxy = t->updateListener().toStrongRef();
                KisDirtyStateSaver<KisPaintOpSettings::UpdateListenerSP> dirtyStateSaver(updateProxy);
                m_parent->setProperty(name + "_previous", m_parent->getProperty(name));
                m_parent->setPropertyNotSaved(name + "_previous");
            }
            return;
        }
    }

    m_parent->setProperty(name, value);
}

bool KisLockedPropertiesProxy::hasProperty(const QString &name) const
{
    KisPaintOpSettings *t = dynamic_cast<KisPaintOpSettings*>(m_parent);
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(t, m_parent->hasProperty(name));
    if (!t->updateListener()) return m_parent->hasProperty(name);

    return (m_lockedProperties->lockedProperties() &&
            m_lockedProperties->lockedProperties()->hasProperty(name)) ||
            m_parent->hasProperty(name);

}

QList<QString> KisLockedPropertiesProxy::getPropertiesKeys() const
{
    KisPaintOpSettings *t = dynamic_cast<KisPaintOpSettings*>(m_parent);
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(t, m_parent->getPropertiesKeys());
    if (!t->updateListener()) return m_parent->getPropertiesKeys();

    QList<QString> result = m_parent->getPropertiesKeys();

    if (m_lockedProperties->lockedProperties() && !m_lockedProperties->lockedProperties()->getPropertiesKeys().isEmpty()) {
#if QT_VERSION >= QT_VERSION_CHECK(5,14,0)
        QSet<QString> properties(result.begin(), result.end());
        auto lockedPropertiesKeys = m_lockedProperties->lockedProperties()->getPropertiesKeys();
        QSet<QString> lockedProperties(lockedPropertiesKeys.begin(), lockedPropertiesKeys.end());
#else
        QSet<QString> properties = QSet<QString>::fromList(result);
        QSet<QString> lockedProperties = QSet<QString>::fromList(m_lockedProperties->lockedProperties()->getPropertiesKeys());
#endif
        properties += lockedProperties;
#if QT_VERSION >= QT_VERSION_CHECK(5,14,0)
        result = QList<QString>(properties.begin(), properties.end()) ;
#else
        result = QList<QString>::fromSet(properties);
#endif
    }

    return result;
}

void KisLockedPropertiesProxy::dump() const
{
    qDebug() << "=== KisLockedPropertiesProxy::dump() ===";
    qDebug() << "parent properties:";
    m_parent->dump();

    if (m_lockedProperties->lockedProperties()) {
        qDebug() << "locked properties:";
        m_lockedProperties->lockedProperties()->dump();
    }
}



