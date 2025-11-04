/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisRootSurfaceInfoProxy.h"

#include <QTimer>

#include <QEvent>
#include <QWidget>
#include <QWindow>
#include <KisPlatformPluginInterfaceFactory.h>
#include <KisSRGBSurfaceColorSpaceManager.h>


KisRootSurfaceInfoProxy::KisRootSurfaceInfoProxy(QWidget *watched, QObject *parent)
    : KisRootSurfaceTrackerBase(watched, parent)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(KisPlatformPluginInterfaceFactory::instance()->surfaceColorManagedByOS());
    m_rootSurfaceProfile = KoColorSpaceRegistry::instance()->p709SRGBProfile();
    // WARNING: we potentially call virtual functions here!
    initialize();
}

KisRootSurfaceInfoProxy::~KisRootSurfaceInfoProxy()
{
}

const KoColorProfile *KisRootSurfaceInfoProxy::rootSurfaceProfile() const
{
    return m_rootSurfaceProfile;
}

bool KisRootSurfaceInfoProxy::isReady() const
{
    return m_topLevelSurfaceManager && m_topLevelSurfaceManager->isReady();
}

QString KisRootSurfaceInfoProxy::colorManagementReport() const
{
    return m_topLevelSurfaceManager ? m_topLevelSurfaceManager->colorManagementReport()
                                    : QString("Top level surface color manager is not found!\n");
}

QString KisRootSurfaceInfoProxy::osPreferredColorSpaceReport() const
{
    return m_topLevelSurfaceManager ? m_topLevelSurfaceManager->osPreferredColorSpaceReport()
                                    : QString("Top level surface color manager is not found!\n");
}

void KisRootSurfaceInfoProxy::connectToNativeWindow(QWindow *nativeWindow)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(nativeWindow);
    if (auto manager = nativeWindow->findChild<KisSRGBSurfaceColorSpaceManager *>()) {
        if (manager != m_topLevelSurfaceManager) {
            m_topLevelSurfaceManager.clear();
            if (m_surfaceManagerConnection) {
                disconnect(m_surfaceManagerConnection);
            }
            m_topLevelSurfaceManager = manager;
            m_surfaceManagerConnection = connect(m_topLevelSurfaceManager,
                                                 &KisSRGBSurfaceColorSpaceManager::sigDisplayConfigChanged,
                                                 this,
                                                 &KisRootSurfaceInfoProxy::tryUpdateRootSurfaceProfile);
            tryUpdateRootSurfaceProfile();
        }
    }
}

void KisRootSurfaceInfoProxy::disconnectFromNativeWindow()
{
    m_topLevelSurfaceManager.clear();
    if (m_surfaceManagerConnection) {
        disconnect(m_surfaceManagerConnection);
    }
    tryUpdateRootSurfaceProfile();
}

void KisRootSurfaceInfoProxy::tryUpdateRootSurfaceProfile()
{
    const KoColorProfile *newProfile = KoColorSpaceRegistry::instance()->p709SRGBProfile();

    if (m_topLevelSurfaceManager) {
        newProfile = m_topLevelSurfaceManager->displayConfig().profile;
    }

    KIS_SAFE_ASSERT_RECOVER_RETURN(newProfile);

    if (newProfile != m_rootSurfaceProfile) {
        m_rootSurfaceProfile = newProfile;
        Q_EMIT sigRootSurfaceProfileChanged(m_rootSurfaceProfile);
    }
}
