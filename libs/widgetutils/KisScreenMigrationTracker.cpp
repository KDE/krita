/*
 *  SPDX-FileCopyrightText: 2023 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisScreenMigrationTracker.h"

#include <QWidget>
#include <QWindow>
#include <QScreen>

KisScreenMigrationTracker::KisScreenMigrationTracker(QWidget *trackedWidget)
    : QObject(trackedWidget)
    , m_trackedWidget(trackedWidget)
{
    QWindow *window = trackedWidget->topLevelWidget()->windowHandle();
    KIS_SAFE_ASSERT_RECOVER_RETURN(window);

    connect(window, &QWindow::screenChanged, this, &KisScreenMigrationTracker::slotScreenChanged);
}

void KisScreenMigrationTracker::slotScreenChanged(QScreen *screen)
{
    disconnect(m_screenConnection);
    m_screenConnection =
        connect(screen, &QScreen::physicalDotsPerInchChanged,
                this, &KisScreenMigrationTracker::slotScreenResolutionChanged);

    emit sigScreenChanged(screen);
    emit sigScreenOrResolutionChanged(screen);
}

void KisScreenMigrationTracker::slotScreenResolutionChanged()
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_trackedWidget->screen());

    emit sigScreenOrResolutionChanged(m_trackedWidget->screen());
}
