/*
 *  kis_tool_freehand_p.cpp - part of Krita
 *
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2007-2008 Cyrille Berger <cberger@cberger.net>
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

#include "kis_tool_freehand_p.h"
#include "kis_tool_freehand.h"

#include <kis_painter.h>
#include <kis_paint_information.h>
#include <kis_paint_device.h>

FreehandPaintJob::FreehandPaintJob(KisToolFreehand* toolFreeHand,
                                   KisPainter* painter,
                                   const KisPaintInformation & pi1,
                                   const KisPaintInformation & pi2,
                                   const FreehandPaintJob* previousPaintJob) :
        m_toolFreeHand(toolFreeHand),
        m_painter(painter),
        m_dragDist(-1.0),
        m_pi1(pi1),
        m_pi2(pi2),
        m_previousPaintJob(previousPaintJob)
{
    setAutoDelete(false);
}


FreehandPaintJob::~FreehandPaintJob()
{
}

FreehandPaintAtJob::FreehandPaintAtJob(KisToolFreehand* toolFreeHand,
                                       KisPainter* painter,
                                       const KisPaintInformation & pi,
                                       const FreehandPaintJob* previousPaintJob)
        : FreehandPaintJob(toolFreeHand, painter, pi, pi, previousPaintJob)
{
}

FreehandPaintAtJob::~FreehandPaintAtJob()
{
}

void FreehandPaintAtJob::run()
{
    m_dragDist = 0.0;
    m_painter->paintAt(KisPaintInformation(m_pi1));
    m_toolFreeHand->setDirty(m_painter->dirtyRegion());
}



FreehandPaintLineJob::FreehandPaintLineJob(KisToolFreehand* toolFreeHand,
        KisPainter* painter,
        const KisPaintInformation & pi1,
        const KisPaintInformation & pi2,
        const FreehandPaintJob* previousPaintJob)
        : FreehandPaintJob(toolFreeHand, painter, pi1, pi2, previousPaintJob)
{
}

FreehandPaintLineJob::~FreehandPaintLineJob()
{
}

void FreehandPaintLineJob::run()
{
    m_dragDist = (m_previousPaintJob) ? m_dragDist = m_previousPaintJob->dragDist() : 0.0;
    m_dragDist = m_painter->paintLine(m_pi1, m_pi2, m_dragDist);
    m_toolFreeHand->setDirty(m_painter->dirtyRegion());
}

FreehandPaintBezierJob::FreehandPaintBezierJob(KisToolFreehand* toolFreeHand,
        KisPainter* painter,
        const KisPaintInformation & pi1,
        const QPointF& control1,
        const QPointF& control2,
        const KisPaintInformation & pi2,
        const FreehandPaintJob* previousPaintJob)
        : FreehandPaintJob(toolFreeHand, painter, pi1, pi2, previousPaintJob)
        , m_control1(control1)
        , m_control2(control2)
{
}

FreehandPaintBezierJob::~FreehandPaintBezierJob()
{
}

void FreehandPaintBezierJob::run()
{
    m_dragDist = (m_previousPaintJob) ? m_dragDist = m_previousPaintJob->dragDist() : 0.0;
    m_dragDist = m_painter->paintBezierCurve(m_pi1, m_control1, m_control2, m_pi2, m_dragDist);
    m_toolFreeHand->setDirty(m_painter->dirtyRegion());
}

