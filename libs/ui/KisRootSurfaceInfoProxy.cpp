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


namespace {
class ChildChangedEventFilter : public QObject
{
    Q_OBJECT
public:
    ChildChangedEventFilter(QObject *watched, QObject *parent)
        : QObject(parent)
        , m_watched(watched)
    {
    }

    bool eventFilter(QObject *watched, QEvent *event) override {
        if (watched == m_watched &&
            (event->type() == QEvent::ChildAdded || event->type() == QEvent::ChildRemoved)) {
                /**
                 * Upon receiving QChildEvent, the child object is not yet
                 * fully constructed, so we need to postpone the event handling
                 * till the next event loop cycle.
                 */
                QTimer::singleShot(0, this, &ChildChangedEventFilter::sigChildrenChanged);
        }

        return false;
    }

    QObject *m_watched {nullptr};

Q_SIGNALS:
    void sigChildrenChanged();
};
}

KisRootSurfaceInfoProxy::KisRootSurfaceInfoProxy(QWidget *watched, QObject *parent)
    : QObject(parent)
    , m_watched(watched)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(KisPlatformPluginInterfaceFactory::instance()->surfaceColorManagedByOS());
    m_rootSurfaceProfile = KoColorSpaceRegistry::instance()->p709SRGBProfile();
    tryUpdateHierarchy();
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

bool KisRootSurfaceInfoProxy::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::ParentChange && m_watchedHierarchy.contains(watched)) {
        tryUpdateHierarchy();
    }

    if (event->type() == QEvent::PlatformSurface && watched == m_watchedHierarchy.last().data()) {
        tryReconnectSurfaceManager();
    }

    return false;
}

void KisRootSurfaceInfoProxy::tryUpdateHierarchy()
{
    auto newHierarchy = getCurrentHierarchy(m_watched);
    if (newHierarchy != m_watchedHierarchy) {
        reconnectToHierarchy(newHierarchy);
        QWidget *topLevel = static_cast<QWidget *>(newHierarchy.last().data());
        if (topLevel != m_topLevelWidgetWithSurface) {
            m_topLevelWidgetWithSurface = topLevel;
            tryReconnectSurfaceManager();
        }
    }
}

void KisRootSurfaceInfoProxy::tryReconnectSurfaceManager()
{
    auto disconnectExistingManager = [this]() {
        m_topLevelSurfaceManager.clear();
        if (m_surfaceManagerConnection) {
            disconnect(m_surfaceManagerConnection);
        }
    };

    if (!m_topLevelWidgetWithSurface) {
        disconnectExistingManager();
        tryUpdateRootSurfaceProfile();
        return;
    }

    QWindow *nativeWindow = m_topLevelWidgetWithSurface->windowHandle();

    if (nativeWindow != m_topLevelNativeWindow) {
        if (m_childChangedFilter && m_topLevelNativeWindow) {
            m_topLevelNativeWindow->removeEventFilter(m_childChangedFilter);
            m_childChangedFilter->deleteLater();
            m_childChangedFilter.clear();
        }

        m_topLevelNativeWindow = nativeWindow;

        if (m_topLevelNativeWindow) {
            auto *filter = new ChildChangedEventFilter(m_topLevelNativeWindow, m_topLevelNativeWindow);
            connect(filter,
                    &ChildChangedEventFilter::sigChildrenChanged,
                    this,
                    &KisRootSurfaceInfoProxy::tryReconnectSurfaceManager);
            m_childChangedFilter = filter;
            m_topLevelNativeWindow->installEventFilter(m_childChangedFilter);
        }
    }

    if (nativeWindow) {
        if (auto manager = nativeWindow->findChild<KisSRGBSurfaceColorSpaceManager *>()) {
            if (manager != m_topLevelSurfaceManager) {
                disconnectExistingManager();
                m_topLevelSurfaceManager = manager;
                m_surfaceManagerConnection = connect(m_topLevelSurfaceManager,
                                                     &KisSRGBSurfaceColorSpaceManager::sigDisplayConfigChanged,
                                                     this,
                                                     &KisRootSurfaceInfoProxy::tryUpdateRootSurfaceProfile);
                tryUpdateRootSurfaceProfile();
            }
        }
    } else {
        disconnectExistingManager();
        tryUpdateRootSurfaceProfile();
    }
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

QVector<QPointer<QObject>> KisRootSurfaceInfoProxy::getCurrentHierarchy(QWidget *wdg)
{
    QVector<QPointer<QObject>> result;

    while (wdg) {
        result.append(wdg);
        wdg = wdg->parentWidget();
    }

    return result;
}

void KisRootSurfaceInfoProxy::reconnectToHierarchy(const QVector<QPointer<QObject>> newHierarchy)
{
    Q_FOREACH (QPointer<QObject> widget, m_watchedHierarchy) {
        KIS_SAFE_ASSERT_RECOVER(widget) continue;
        widget->removeEventFilter(this);
    }

    m_watchedHierarchy.clear();

    Q_FOREACH (QPointer<QObject> widget, newHierarchy) {
        widget->installEventFilter(this);
    }

    m_watchedHierarchy = newHierarchy;
}

#include <KisRootSurfaceInfoProxy.moc>