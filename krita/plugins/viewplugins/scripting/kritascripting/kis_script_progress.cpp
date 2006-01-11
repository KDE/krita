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

#include "kis_script_progress.h"

#include <kis_progress_display_interface.h>
#include <kis_view.h>

void KisScriptProgress::activateAsSubject()
{
    m_view->canvasSubject()->progressDisplay()->setSubject( this, true, false /* TODO: how to cancel a script ? */ );
}

void KisScriptProgress::setProgressTotalSteps(Q_INT32 totalSteps)
{
    m_progressTotalSteps = totalSteps;
    m_progressSteps = 0;
    m_lastProgressPerCent = 0;
    emit notifyProgress(0);
}

void KisScriptProgress::setProgress(Q_INT32 progress)
{
    m_progressSteps = progress;
    Q_INT32 progressPerCent = (m_progressSteps * 100) / m_progressTotalSteps;

    if (progressPerCent != m_lastProgressPerCent) {

        m_lastProgressPerCent = progressPerCent;
        emit notifyProgress(progressPerCent);
    }
}

void KisScriptProgress::incProgress()
{
    setProgress( ++m_progressSteps );
}

void KisScriptProgress::setProgressStage(const QString& stage, Q_INT32 progress)
{
    Q_INT32 progressPerCent = (progress * 100) / m_progressTotalSteps;
    m_lastProgressPerCent = progress;
    emit notifyProgressStage( stage, progressPerCent);
}

void KisScriptProgress::progressDone()
{
    emit notifyProgressDone();
}
