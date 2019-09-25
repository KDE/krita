/*
 *  Copyright (c) 2019 Dmitry Kazakov <dimula73@gmail.com>
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

#include "KisMoveBoundsCalculationJob.h"
#include "kis_node.h"
#include "kis_selection.h"
#include "kis_layer_utils.h"

KisMoveBoundsCalculationJob::KisMoveBoundsCalculationJob(KisNodeList nodes,
                                                         KisSelectionSP selection,
                                                         QObject *requestedBy)
    : m_nodes(nodes),
      m_selection(selection),
      m_requestedBy(requestedBy)
{
    setExclusive(true);
}

void KisMoveBoundsCalculationJob::run()
{
    QRect handlesRect;

    Q_FOREACH (KisNodeSP node, m_nodes) {
        handlesRect |= KisLayerUtils::recursiveNodeExactBounds(node);
    }

    if (m_selection) {
        handlesRect &= m_selection->selectedExactRect();
    }

    emit sigCalcualtionFinished(handlesRect);
}

bool KisMoveBoundsCalculationJob::overrides(const KisSpontaneousJob *_otherJob)
{
    const KisMoveBoundsCalculationJob *otherJob =
        dynamic_cast<const KisMoveBoundsCalculationJob*>(_otherJob);

    return otherJob && otherJob->m_requestedBy == m_requestedBy;
}

int KisMoveBoundsCalculationJob::levelOfDetail() const
{
    return 0;
}
