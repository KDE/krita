/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisRootSurfaceTrackerBase.h"

#include <QTimer>

#include <QEvent>
#include <QWidget>
#include <QWindow>

#include <kis_assert.h>


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

KisRootSurfaceTrackerBase::KisRootSurfaceTrackerBase(QWidget *watched, QObject *parent)
    : QObject(parent)
    , m_watched(watched)
{
}

KisRootSurfaceTrackerBase::~KisRootSurfaceTrackerBase()
{
}

QWidget* KisRootSurfaceTrackerBase::trackedWidget() const
{
    return m_watched;
}

void KisRootSurfaceTrackerBase::initialize()
{
    tryUpdateHierarchy();
}

bool KisRootSurfaceTrackerBase::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::ParentChange && m_watchedHierarchy.contains(watched)) {
        tryUpdateHierarchy();
    }

    if (event->type() == QEvent::PlatformSurface) {
        if (watched == m_watchedHierarchy.last().data()) {
            tryReconnectToNativeWindow();
        } else {
            // some widget in the middle of the hierarchy just received
            // a native window. Let's promote that to toplevel now!
            tryUpdateHierarchy();
        }
    }

    return false;
}

void KisRootSurfaceTrackerBase::tryUpdateHierarchy()
{
    auto newHierarchy = getCurrentHierarchy(m_watched);
    if (newHierarchy != m_watchedHierarchy) {
        reconnectToHierarchy(newHierarchy);
        QWidget *topLevel = static_cast<QWidget *>(newHierarchy.last().data());
        if (topLevel != m_topLevelWidgetWithSurface) {
            m_topLevelWidgetWithSurface = topLevel;
            tryReconnectToNativeWindow();
        }
    }
}

void KisRootSurfaceTrackerBase::tryReconnectToNativeWindow()
{
    if (!m_topLevelWidgetWithSurface) {
        disconnectFromNativeWindow();
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
                    &KisRootSurfaceTrackerBase::tryReconnectToNativeWindow);
            m_childChangedFilter = filter;
            m_topLevelNativeWindow->installEventFilter(m_childChangedFilter);
        }
    }

    if (nativeWindow) {
        connectToNativeWindow(nativeWindow);
    } else {
        disconnectFromNativeWindow();
    }
}

QVector<QPointer<QObject>> KisRootSurfaceTrackerBase::getCurrentHierarchy(QWidget *wdg)
{
    QVector<QPointer<QObject>> result;

    while (wdg) {
        result.append(wdg);

        // break on the first native window
        if (wdg->windowHandle()) {
            break;
        } else {
            wdg = wdg->parentWidget();
        }
    }

    return result;
}

void KisRootSurfaceTrackerBase::reconnectToHierarchy(const QVector<QPointer<QObject>> newHierarchy)
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

#include <KisRootSurfaceTrackerBase.moc>