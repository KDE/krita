/*
 *  Copyright (c) 2011 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_recording_adapter.h"

#include "kis_image.h"
#include "kis_node.h"
#include <brushengine/kis_paintop_preset.h>

#include "recorder/kis_action_recorder.h"
#include "recorder/kis_recorded_path_paint_action.h"
#include "recorder/kis_node_query_path.h"


KisRecordingAdapter::KisRecordingAdapter()
    : m_pathPaintAction(0)
{
}

KisRecordingAdapter::~KisRecordingAdapter()
{
}

void KisRecordingAdapter::startStroke(KisImageWSP image, KisResourcesSnapshotSP resources,
                                      const KisDistanceInitInfo &startDistInfo)
{
    Q_ASSERT(!m_pathPaintAction);
    Q_ASSERT(!m_image);

    m_image = image;
    m_pathPaintAction = new KisRecordedPathPaintAction(
        KisNodeQueryPath::absolutePath(resources->currentNode()), 0, startDistInfo);

    resources->setupPaintAction(m_pathPaintAction);
}

void KisRecordingAdapter::endStroke()
{
    Q_ASSERT(m_pathPaintAction);
    Q_ASSERT(m_image);

    m_image->actionRecorder()->addAction(*m_pathPaintAction);

    delete m_pathPaintAction;
    m_pathPaintAction = 0;
    m_image = 0;
}


void KisRecordingAdapter::addPoint(const KisPaintInformation &pi)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_pathPaintAction);
    m_pathPaintAction->addPoint(pi);
}

void KisRecordingAdapter::addLine(const KisPaintInformation &pi1,
                                  const KisPaintInformation &pi2)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_pathPaintAction);
    m_pathPaintAction->addLine(pi1, pi2);
}


void KisRecordingAdapter::addCurve(const KisPaintInformation &pi1,
                                   const QPointF &control1,
                                   const QPointF &control2,
                                   const KisPaintInformation &pi2)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_pathPaintAction);
    m_pathPaintAction->addCurve(pi1, control1, control2, pi2);
}

