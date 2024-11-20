/*
 *  SPDX-FileCopyrightText: 2023 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisScreenMigrationTracker.h"

#include <QGuiApplication>
#include <QWidget>
#include <QWindow>
#include <QScreen>
#include <kis_assert.h>
#include <kis_signal_compressor.h>

namespace {
QWindow *findNearestParentWithNativeWindow(QWidget *widget)
{
    do {
        QWindow *nativeWindow = widget->windowHandle();

        if (nativeWindow) {
            return nativeWindow;
        }

    } while ((widget = widget->parentWidget()));

    return nullptr;
}
}

KisScreenMigrationTracker::KisScreenMigrationTracker(QWidget *trackedWidget, QObject *parent)
    : QObject(parent)
    , m_trackedWidget(trackedWidget)
    , m_resolutionChangeCompressor(new KisSignalCompressor(100, KisSignalCompressor::POSTPONE, this))
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(trackedWidget);

    /**
     * The window of the tracked widget may be not initialized at the construction
     * of the widget. Hence we should postpone initialization of the window handle
     * till the widget gets shown on screen.
     */
    m_trackedTopLevelWindow = findNearestParentWithNativeWindow(trackedWidget);
    if (m_trackedTopLevelWindow) {
        connectTopLevelWindow(m_trackedTopLevelWindow);
    } else {
        trackedWidget->installEventFilter(this);
    }

    connect(m_resolutionChangeCompressor, &KisSignalCompressor::timeout,
            this, &KisScreenMigrationTracker::slotResolutionCompressorTriggered);
}

QScreen* KisScreenMigrationTracker::currentScreen() const
{
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(m_trackedTopLevelWindow, qApp->screens().first());
    return m_trackedTopLevelWindow->screen();
}

QScreen *KisScreenMigrationTracker::currentScreenSafe() const
{
    return m_trackedTopLevelWindow ? m_trackedTopLevelWindow->screen() : qApp->screens().first();
}

void KisScreenMigrationTracker::connectScreenSignals(QScreen *screen)
{
    m_screenConnections.clear();
    m_screenConnections.addConnection(screen, &QScreen::physicalDotsPerInchChanged,
                                      this, &KisScreenMigrationTracker::slotScreenResolutionChanged);
    m_screenConnections.addConnection(screen, &QScreen::logicalDotsPerInchChanged,
                                      this, &KisScreenMigrationTracker::slotScreenLogicalResolutionChanged);
}

void KisScreenMigrationTracker::connectTopLevelWindow(QWindow *window)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(window);
    connect(window, &QWindow::screenChanged, this, &KisScreenMigrationTracker::slotScreenChanged);
    connectScreenSignals(window->screen());

    Q_EMIT sigScreenChanged(window->screen());
    Q_EMIT sigScreenOrResolutionChanged(window->screen());
}

bool KisScreenMigrationTracker::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_trackedWidget && event->type() == QEvent::Show) {
        m_trackedTopLevelWindow = findNearestParentWithNativeWindow(m_trackedWidget);
        if (m_trackedTopLevelWindow) {
            connectTopLevelWindow(m_trackedTopLevelWindow);
            m_trackedWidget->removeEventFilter(this);
        }
    }

    return QObject::eventFilter(watched, event);
}

void KisScreenMigrationTracker::slotScreenChanged(QScreen *screen)
{
    connectScreenSignals(screen);

    Q_EMIT sigScreenChanged(screen);
    Q_EMIT sigScreenOrResolutionChanged(screen);
}

void KisScreenMigrationTracker::slotScreenResolutionChanged(qreal value)
{
    Q_UNUSED(value)
    m_resolutionChangeCompressor->start();
}

void KisScreenMigrationTracker::slotScreenLogicalResolutionChanged(qreal value)
{
    Q_UNUSED(value)
    m_resolutionChangeCompressor->start();
}

void KisScreenMigrationTracker::slotResolutionCompressorTriggered()
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_trackedTopLevelWindow);
    Q_EMIT sigScreenOrResolutionChanged(m_trackedTopLevelWindow->screen());
}
