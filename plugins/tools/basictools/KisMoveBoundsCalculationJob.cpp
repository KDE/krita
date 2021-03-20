/*
 *  SPDX-FileCopyrightText: 2019 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
        handlesRect |= KisLayerUtils::recursiveTightNodeVisibleBounds(node);
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

QString KisMoveBoundsCalculationJob::debugName() const
{
    QString result;
    QDebug dbg(&result);
    dbg << "KisMoveBoundsCalculationJob" << ppVar(m_requestedBy) << m_nodes;
    return result;
}
