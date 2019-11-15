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
#include "kis_image.h"
#include "kis_projection_leaf.h"


KisUpdateSelectionJob::KisUpdateSelectionJob(KisSelectionSP selection, const QRect &updateRect)
    : m_selection(selection),
      m_updateRect(updateRect)
{
    /**
     * TODO: we should implement correct KisShapeSelectionCanvas for
     * projection. See a comment in KisUpdateSelectionJob::run().
     *
     * Right now, since this job accesses some projections for write, we
     * should declare it as exclusive
     */

    setExclusive(true);
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
    QRect dirtyRect;

    KisNodeSP parentNode = m_selection->parentNode();
    if (parentNode) {
        dirtyRect = parentNode->extent();
    }

    if (!m_updateRect.isEmpty()) {
        m_selection->updateProjection(m_updateRect);
    } else {
        m_selection->updateProjection();
    }

    m_selection->notifySelectionChanged();

    /**
     * TODO: in the future we should remove selection projection calculation
     *       from this job and to reuse a fully-featured shape layer canvas
     *       from KisShapeLayer. Then projection calculation will be a little
     *       bit more efficient.
     */
    if (parentNode && parentNode->projectionLeaf()->isOverlayProjectionLeaf()) {
        dirtyRect |= parentNode->extent();
        parentNode->setDirty(dirtyRect);
    }
}

int KisUpdateSelectionJob::levelOfDetail() const
{
    return 0;
}

QString KisUpdateSelectionJob::debugName() const
{
    return "KisUpdateSelectionJob";
}
