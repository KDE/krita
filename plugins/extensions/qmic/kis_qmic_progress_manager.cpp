/*
 * SPDX-FileCopyrightText: 2014 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_qmic_progress_manager.h"
#include <QApplication>

#include <KisViewManager.h>
#include <KoProgressUpdater.h>

static const int UPDATE_PROGRESS_TIMEOUT = 500;

KisQmicProgressManager::KisQmicProgressManager(KisViewManager* viewManager)
    : m_progressPulseRequest(0)
{
        m_progressUpdater = new KoProgressUpdater(viewManager->createUnthreadedUpdater(""));
        m_progressTimer.setInterval(UPDATE_PROGRESS_TIMEOUT);
        connect(&m_progressTimer, SIGNAL(timeout()), this, SIGNAL(sigProgress()));
}

KisQmicProgressManager::~KisQmicProgressManager()
{
    QApplication::restoreOverrideCursor();
    delete m_progressUpdater;
}


void KisQmicProgressManager::initProgress()
{
    m_progressTimer.start();
    QApplication::setOverrideCursor(Qt::WaitCursor);
    m_updater = m_progressUpdater->startSubtask();
    m_progressPulseRequest = 0;
}

void KisQmicProgressManager::updateProgress(float progress)
{
    int currentProgress = 0.0;
    if (progress >= 0.0)  {
        if (m_progressPulseRequest > 0)
        {
            m_progressUpdater->start(100);
            m_updater = m_progressUpdater->startSubtask();
            m_progressPulseRequest = 0;
        }
        currentProgress = (int)progress;
    }
    else  {
        // pulse
        m_progressPulseRequest++;
        if (m_updater->progress() >= 90)
        {
            m_progressUpdater->start(100);
            m_updater = m_progressUpdater->startSubtask();
        }
        currentProgress = (m_progressPulseRequest % 10) * 10;
    }

    dbgPlugins << "Current progress : " << currentProgress << " vs " << progress;
    m_updater->setProgress(currentProgress);
}

void KisQmicProgressManager::finishProgress()
{
    m_progressTimer.stop();
    QApplication::restoreOverrideCursor();
    m_updater->setProgress(100);
}

bool KisQmicProgressManager::inProgress()
{
    return m_progressTimer.isActive();
}
