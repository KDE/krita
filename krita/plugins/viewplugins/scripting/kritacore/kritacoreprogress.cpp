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

#include "kritacoreprogress.h"

#include <kis_progress_display_interface.h>
#include <kis_view.h>

using namespace Kross::KritaCore;

KritaCoreProgress::KritaCoreProgress(KisView* view)
    : KisProgressSubject()
    , m_view(view)
    , m_progressTotalSteps(0)
{
    //kDebug() << "KritaCoreProgress::KritaCoreProgress" << endl;
}

KritaCoreProgress::~KritaCoreProgress()
{
    //kDebug() << "KritaCoreProgress::~KritaCoreProgress" << endl;
}

void KritaCoreProgress::activateAsSubject()
{
    // set this class as the KisProgressSubject in view.
    m_view->canvasSubject()->progressDisplay()->setSubject( this, true, false /* TODO: how to cancel a script ? */ );
    m_progressTotalSteps = 100; // let's us 100 as default (=100%)
}

void KritaCoreProgress::setProgressTotalSteps(uint totalSteps)
{
    if(m_progressTotalSteps < 1)
        activateAsSubject();

    m_progressTotalSteps = totalSteps > 1 ? totalSteps : 1;
    m_progressSteps = 0;
    m_lastProgressPerCent = 0;
    emit notifyProgress(0);
}

void KritaCoreProgress::setProgress(uint progress)
{
    if(m_progressTotalSteps < 1)
        return;

    m_progressSteps = progress;
    qint32 progressPerCent = (m_progressSteps * 100) / m_progressTotalSteps;

    if (progressPerCent != m_lastProgressPerCent) {

        m_lastProgressPerCent = progressPerCent;
        emit notifyProgress(progressPerCent);
    }
}

void KritaCoreProgress::incProgress()
{
    setProgress( ++m_progressSteps );
}

void KritaCoreProgress::setProgressStage(const QString& stage, uint progress)
{
    if(m_progressTotalSteps < 1)
        return;

    qint32 progressPerCent = (progress * 100) / m_progressTotalSteps;
    m_lastProgressPerCent = progress;
    emit notifyProgressStage( stage, progressPerCent);
}

void KritaCoreProgress::progressDone()
{
    kDebug() << "KritaCoreProgress::progressDone" << endl;
    emit notifyProgressDone();
}

#include "kritacoreprogress.moc"
