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

#include <KoCanvasResourceProvider.h>

#include "kis_resources_snapshot.h"
#include <kis_distance_information.h>
#include "kis_image.h"
#include "kis_painter.h"
#include <strokes/KisFreehandStrokeInfo.h>
#include "KisAsyncronousStrokeUpdateHelper.h"


KisFigurePaintingToolHelper::KisFigurePaintingToolHelper(const KUndo2MagicString &name,
                                                         KisImageWSP image,
                                                         KisNodeSP currentNode,
                                                         KoCanvasResourceProvider *resourceManager,
                                                         KisPainter::StrokeStyle strokeStyle,
                                                         KisPainter::FillStyle fillStyle)
{
    m_strokesFacade = image.data();

    m_resources =
        new KisResourcesSnapshot(image,
                                 currentNode,
                                 resourceManager);

    m_resources->setStrokeStyle(strokeStyle);
    m_resources->setFillStyle(fillStyle);

    KisFreehandStrokeInfo *strokeInfo = new KisFreehandStrokeInfo();

    KisStrokeStrategy *stroke =
        new FreehandStrokeStrategy(m_resources, strokeInfo, name);

    m_strokeId = m_strokesFacade->startStroke(stroke);
}

KisFigurePaintingToolHelper::~KisFigurePaintingToolHelper()
{
    m_strokesFacade->addJob(m_strokeId,
        new KisAsyncronousStrokeUpdateHelper::UpdateData(true));
    m_strokesFacade->endStroke(m_strokeId);
}

void KisFigurePaintingToolHelper::paintLine(const KisPaintInformation &pi0,
                                            const KisPaintInformation &pi1)
{
    m_strokesFacade->addJob(m_strokeId,
        new FreehandStrokeStrategy::Data(0,
                                         pi0, pi1));
}

void KisFigurePaintingToolHelper::paintPolyline(const vQPointF &points)
{
    m_strokesFacade->addJob(m_strokeId,
        new FreehandStrokeStrategy::Data(0,
                                         FreehandStrokeStrategy::Data::POLYLINE,
                                         points));
}

void KisFigurePaintingToolHelper::paintPolygon(const vQPointF &points)
{
    m_strokesFacade->addJob(m_strokeId,
        new FreehandStrokeStrategy::Data(0,
                                         FreehandStrokeStrategy::Data::POLYGON,
                                         points));
}

void KisFigurePaintingToolHelper::paintRect(const QRectF &rect)
{
    m_strokesFacade->addJob(m_strokeId,
        new FreehandStrokeStrategy::Data(0,
                                         FreehandStrokeStrategy::Data::RECT,
                                         rect));
}

void KisFigurePaintingToolHelper::paintEllipse(const QRectF &rect)
{
    m_strokesFacade->addJob(m_strokeId,
        new FreehandStrokeStrategy::Data(0,
                                         FreehandStrokeStrategy::Data::ELLIPSE,
                                         rect));
}

void KisFigurePaintingToolHelper::paintPainterPath(const QPainterPath &path)
{
    m_strokesFacade->addJob(m_strokeId,
        new FreehandStrokeStrategy::Data(0,
                                         FreehandStrokeStrategy::Data::PAINTER_PATH,
                                         path));
}

void KisFigurePaintingToolHelper::setFGColorOverride(const KoColor &color)
{
    m_resources->setFGColorOverride(color);
}

void KisFigurePaintingToolHelper::setBGColorOverride(const KoColor &color)
{
    m_resources->setBGColorOverride(color);
}

void KisFigurePaintingToolHelper::setSelectionOverride(KisSelectionSP m_selection)
{
    m_resources->setSelectionOverride(m_selection);
}

void KisFigurePaintingToolHelper::setBrush(const KisPaintOpPresetSP &brush)
{
    m_resources->setBrush(brush);
}

void KisFigurePaintingToolHelper::paintPainterPathQPen(const QPainterPath path, const QPen &pen, const KoColor &color)
{
    m_strokesFacade->addJob(m_strokeId,
        new FreehandStrokeStrategy::Data(0,
                                         FreehandStrokeStrategy::Data::QPAINTER_PATH,
                                         path, pen, color));
}

void KisFigurePaintingToolHelper::paintPainterPathQPenFill(const QPainterPath path, const QPen &pen, const KoColor &color)
{
    m_strokesFacade->addJob(m_strokeId,
        new FreehandStrokeStrategy::Data(0,
                                         FreehandStrokeStrategy::Data::QPAINTER_PATH_FILL,
                                         path, pen, color));
}
