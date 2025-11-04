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

#include <kis_debug.h>


KisScreenMigrationTracker::KisScreenMigrationTracker(QWidget *trackedWidget, QObject *parent)
    : KisRootSurfaceTrackerBase(trackedWidget, parent)
    , m_resolutionChangeCompressor(new KisSignalCompressor(100, KisSignalCompressor::POSTPONE, this))
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(trackedWidget);
    // WARNING: we potentially call virtual functions here!
    initialize();

    connect(m_resolutionChangeCompressor, &KisSignalCompressor::timeout,
            this, &KisScreenMigrationTracker::slotResolutionCompressorTriggered);
}

QScreen* KisScreenMigrationTracker::currentScreen() const
{
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(m_connectedTopLevelWindow, qApp->screens().first());
    return m_connectedTopLevelWindow->screen();
}

QScreen *KisScreenMigrationTracker::currentScreenSafe() const
{
    return m_connectedTopLevelWindow ? m_connectedTopLevelWindow->screen() : qApp->screens().first();
}

void KisScreenMigrationTracker::connectScreenSignals(QScreen *screen)
{
    m_screenConnections.clear();
    m_screenConnections.addConnection(screen, &QScreen::physicalDotsPerInchChanged,
                                      this, &KisScreenMigrationTracker::slotScreenResolutionChanged);
    m_screenConnections.addConnection(screen, &QScreen::logicalDotsPerInchChanged,
                                      this, &KisScreenMigrationTracker::slotScreenLogicalResolutionChanged);
}

void KisScreenMigrationTracker::connectToNativeWindow(QWindow *window)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(window);

    if (window != m_connectedTopLevelWindow) {
        m_topLevelWindowConnection =
            connect(window, &QWindow::screenChanged, this, &KisScreenMigrationTracker::slotScreenChanged);
        connectScreenSignals(window->screen());
        m_connectedTopLevelWindow = window;

        Q_EMIT sigScreenChanged(window->screen());
        Q_EMIT sigScreenOrResolutionChanged(window->screen());
    }
}
void KisScreenMigrationTracker::disconnectFromNativeWindow()
{
    disconnect(m_topLevelWindowConnection);
    m_connectedTopLevelWindow.clear();
    m_screenConnections.clear();
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
    if (!m_connectedTopLevelWindow) {
        // the tracker is not yet initialized, it should happen only
        // when the widget have not yet been shown at least once
        KIS_SAFE_ASSERT_RECOVER_NOOP(!trackedWidget()->isVisible());
        return;
    }
    Q_EMIT sigScreenOrResolutionChanged(m_connectedTopLevelWindow->screen());
}
