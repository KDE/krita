/*
 *  Copyright (c) 2005 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Library General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "krs_progress.h"

#include "krs_module.h"

#include <kis_debug.h>

#include <KoMainWindow.h>
//#include <KoProgressUpdater.h>
#include <kis_view2.h>

using namespace Scripting;

Progress::Progress(Module* module, KisView2* view)
        : QObject()
        , m_module(module)
        , m_view(view)
        , m_progressTotalSteps(0)
{
    m_mainwin = m_view ? m_view->shell() : 0;
}

Progress::~Progress()
{
}

void Progress::activateAsSubject()
{
    // set this class as the KisProgressSubject in view.
    //TODO: restore progress in scripting
//     m_view->canvasSubject()->progressDisplay()->setSubject( this, true, false /* TODO: how to cancel a script ? */ );
    m_progressTotalSteps = 100; // let's us 100 as default (=100%)
}

void Progress::updateProgress(int progressPerCent)
{
    if (m_mainwin) {
        if (m_progressSteps == m_progressTotalSteps)
            m_mainwin->slotProgress(-1);
        else
            m_mainwin->slotProgress(progressPerCent);
    }
}

void Progress::setProgressTotalSteps(uint totalSteps)
{
    if (m_progressTotalSteps < 1)
        activateAsSubject();

    m_progressTotalSteps = totalSteps > 1 ? totalSteps : 1;
    m_progressSteps = 0;
    m_lastProgressPerCent = 0;
    updateProgress(0);
}

void Progress::setProgress(uint progress)
{
    if (m_progressTotalSteps < 1)
        return;

    m_progressSteps = progress;
    uint progressPerCent = (m_progressSteps * 100) / m_progressTotalSteps;

    if (progressPerCent != m_lastProgressPerCent) {

        m_lastProgressPerCent = progressPerCent;
        updateProgress(progressPerCent);
    }
}

void Progress::incProgress()
{
    setProgress(++m_progressSteps);
}

void Progress::setProgressStage(const QString& stage, uint progress)
{
    Q_UNUSED(stage);
    if (m_progressTotalSteps < 1)
        return;

    uint progressPerCent = (progress * 100) / m_progressTotalSteps;
    m_lastProgressPerCent = progress;
    //TODO m_progressUpdater->setProgressStage( stage, progressPerCent);
    updateProgress(progressPerCent);
}

void Progress::progressDone()
{
    m_progressTotalSteps = 0;
    updateProgress(100);
}

#include "krs_progress.moc"
