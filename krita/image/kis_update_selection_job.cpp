/*
 *  Copyright (c) 2013 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_update_selection_job.h"


KisUpdateSelectionJob::KisUpdateSelectionJob(KisSelectionSP selection, const QRect &updateRect)
    : m_selection(selection),
      m_updateRect(updateRect)
{
}

bool KisUpdateSelectionJob::overrides(const KisSpontaneousJob *_otherJob)
{
    const KisUpdateSelectionJob *otherJob =
        dynamic_cast<const KisUpdateSelectionJob*>(_otherJob);

    bool retval = false;

    if (otherJob && otherJob->m_selection == m_selection) {
        if (!m_updateRect.isEmpty()) {
            m_updateRect |= otherJob->m_updateRect;
        }
        retval = true;
    }

    return retval;
}

void KisUpdateSelectionJob::run()
{
    if (!m_updateRect.isEmpty()) {
        m_selection->updateProjection(m_updateRect);
    } else {
        m_selection->updateProjection();
    }

    m_selection->notifySelectionChanged();
}
