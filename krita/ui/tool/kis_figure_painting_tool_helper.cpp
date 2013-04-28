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

#include "kis_figure_painting_tool_helper.h"

#include <KoCanvasResourceManager.h>

#include "kis_resources_snapshot.h"
#include "kis_distance_information.h"
#include "kis_image.h"
#include "kis_painter.h"


KisFigurePaintingToolHelper::KisFigurePaintingToolHelper(const QString &name,
                                                         KisImageWSP image,
                                                         KoCanvasResourceManager *resourceManager,
                                                         KisPainter::StrokeStyle strokeStyle,
                                                         KisPainter::FillStyle fillStyle)
{
    m_strokesFacade = image.data();

    m_resources =
        new KisResourcesSnapshot(image,
                                 image->postExecutionUndoAdapter(),
                                 resourceManager);

    m_resources->setStrokeStyle(strokeStyle);
    m_resources->setFillStyle(fillStyle);

    m_painterInfo =
        new PainterInfo(new KisPainter(),
                        new KisDistanceInformation());

    bool indirectPainting = m_resources->needsIndirectPainting();

    KisStrokeStrategy *stroke =
        new FreehandStrokeStrategy(indirectPainting, m_resources, m_painterInfo, name);

    m_strokeId = m_strokesFacade->startStroke(stroke);
}

KisFigurePaintingToolHelper::~KisFigurePaintingToolHelper()
{
    m_strokesFacade->endStroke(m_strokeId);
}

void KisFigurePaintingToolHelper::paintLine(const KisPaintInformation &pi0,
                                            const KisPaintInformation &pi1)
{
    m_strokesFacade->addJob(m_strokeId,
        new FreehandStrokeStrategy::Data(m_resources->currentNode(),
                                         m_painterInfo,
                                         pi0, pi1));
}

void KisFigurePaintingToolHelper::paintPolyline(const vQPointF &points)
{
    m_strokesFacade->addJob(m_strokeId,
        new FreehandStrokeStrategy::Data(m_resources->currentNode(),
                                         m_painterInfo,
                                         FreehandStrokeStrategy::Data::POLYLINE,
                                         points));
}

void KisFigurePaintingToolHelper::paintPolygon(const vQPointF &points)
{
    m_strokesFacade->addJob(m_strokeId,
        new FreehandStrokeStrategy::Data(m_resources->currentNode(),
                                         m_painterInfo,
                                         FreehandStrokeStrategy::Data::POLYGON,
                                         points));
}

void KisFigurePaintingToolHelper::paintRect(const QRectF &rect)
{
    m_strokesFacade->addJob(m_strokeId,
        new FreehandStrokeStrategy::Data(m_resources->currentNode(),
                                         m_painterInfo,
                                         FreehandStrokeStrategy::Data::RECT,
                                         rect));
}

void KisFigurePaintingToolHelper::paintEllipse(const QRectF &rect)
{
    m_strokesFacade->addJob(m_strokeId,
        new FreehandStrokeStrategy::Data(m_resources->currentNode(),
                                         m_painterInfo,
                                         FreehandStrokeStrategy::Data::ELLIPSE,
                                         rect));
}

void KisFigurePaintingToolHelper::paintPainterPath(const QPainterPath &path)
{
    m_strokesFacade->addJob(m_strokeId,
        new FreehandStrokeStrategy::Data(m_resources->currentNode(),
                                         m_painterInfo,
                                         FreehandStrokeStrategy::Data::PAINTER_PATH,
                                         path));
}

