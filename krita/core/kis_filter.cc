/*
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "kis_filter.h"

#include <qstring.h>

#include "kis_types.h"
#include "kis_filter_configuration.h"

KisFilter::KisFilter(const KisID& id, const QString & category, const QString & entry)
    : KisProgressSubject(0, id.id().latin1())
    , m_id(id)
    , m_progressDisplay(0)
    , m_category(category)
    , m_entry(entry)
{
}

KisFilterConfiguration * KisFilter::configuration(QWidget*)
{
    return new KisFilterConfiguration(m_id.id(), 0);
}

KisFilterConfiguration * KisFilter::configuration()
{
    return new KisFilterConfiguration(m_id.id(), 0);
}

KisFilterConfigWidget * KisFilter::createConfigurationWidget(QWidget *, KisPaintDeviceSP)
{
    return 0;
}

void KisFilter::setProgressDisplay(KisProgressDisplayInterface * progressDisplay)
{
    m_progressDisplay = progressDisplay;
}


void KisFilter::enableProgress() {
    m_progressEnabled = true;
    m_cancelRequested = false;
}

void KisFilter::disableProgress() {
    m_progressEnabled = false;
    m_cancelRequested = false;
}

void KisFilter::setProgressTotalSteps(Q_INT32 totalSteps)
{
    if (m_progressEnabled) {

        m_progressTotalSteps = totalSteps;
        m_lastProgressPerCent = 0;
        m_progressSteps = 0;
        emit notifyProgress(0);
    }
}

void KisFilter::setProgress(Q_INT32 progress)
{
    if (m_progressEnabled) {
        Q_INT32 progressPerCent = (progress * 100) / m_progressTotalSteps;
        m_progressSteps = progress;

        if (progressPerCent != m_lastProgressPerCent) {

            m_lastProgressPerCent = progressPerCent;
            emit notifyProgress(progressPerCent);
        }
    }
}

void KisFilter::incProgress()
{
    setProgress(++m_progressSteps);

}

void KisFilter::setProgressStage(const QString& stage, Q_INT32 progress)
{
    if (m_progressEnabled) {

        Q_INT32 progressPerCent = (progress * 100) / m_progressTotalSteps;

        m_lastProgressPerCent = progressPerCent;
        emit notifyProgressStage(stage, progressPerCent);
    }
}

void KisFilter::setProgressDone()
{
    if (m_progressEnabled) {
        emit notifyProgressDone();
    }
}


bool KisFilter::autoUpdate() {
    return m_autoUpdate;
}

void KisFilter::setAutoUpdate(bool set) {
    m_autoUpdate = set;
}


#include "kis_filter.moc"
