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

KisScreenMigrationTracker::KisScreenMigrationTracker(QWidget *trackedWidget, QObject *parent)
    : QObject(parent)
    , m_trackedWidget(trackedWidget)
    , m_resolutionChangeCompressor(new KisSignalCompressor(100, KisSignalCompressor::POSTPONE, this))
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(trackedWidget);

    QWindow *window = trackedWidget->topLevelWidget()->windowHandle();
    KIS_SAFE_ASSERT_RECOVER_RETURN(window);

    connect(window, &QWindow::screenChanged, this, &KisScreenMigrationTracker::slotScreenChanged);
    connectScreenSignals(window->screen());

    connect(m_resolutionChangeCompressor, &KisSignalCompressor::timeout,
            this, &KisScreenMigrationTracker::slotResolutionCompressorTriggered);
}

QScreen* KisScreenMigrationTracker::currentScreen() const
{
    QWindow *window = m_trackedWidget->topLevelWidget()->windowHandle();
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(window, qApp->screens().first());

    return window->screen();
}

void KisScreenMigrationTracker::connectScreenSignals(QScreen *screen)
{
    m_screenConnections.clear();
    m_screenConnections.addConnection(screen, &QScreen::physicalDotsPerInchChanged,
                                      this, &KisScreenMigrationTracker::slotScreenResolutionChanged);
    m_screenConnections.addConnection(screen, &QScreen::logicalDotsPerInchChanged,
                                      this, &KisScreenMigrationTracker::slotScreenLogicalResolutionChanged);
}

void KisScreenMigrationTracker::slotScreenChanged(QScreen *screen)
{
    connectScreenSignals(screen);

    emit sigScreenChanged(screen);
    emit sigScreenOrResolutionChanged(screen);
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
    QWindow *window = m_trackedWidget->topLevelWidget()->windowHandle();
    KIS_SAFE_ASSERT_RECOVER_RETURN(window);

    Q_EMIT sigScreenOrResolutionChanged(window->screen());
}
