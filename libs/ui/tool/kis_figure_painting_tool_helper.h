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

#ifndef __KIS_FIGURE_PAINTING_TOOL_HELPER_H
#define __KIS_FIGURE_PAINTING_TOOL_HELPER_H

#include "kis_types.h"
#include "kritaui_export.h"
#include <brushengine/kis_paint_information.h>
#include "strokes/freehand_stroke.h"
#include "KisToolShapeUtils.h"

class KoCanvasResourceProvider;
class KisStrokesFacade;

class KRITAUI_EXPORT KisFigurePaintingToolHelper
{
public:
    KisFigurePaintingToolHelper(const KUndo2MagicString &name,
                                KisImageWSP image,
                                KisNodeSP currentNode,
                                KoCanvasResourceProvider *resourceManager,
                                KisToolShapeUtils::StrokeStyle strokeStyle,
                                KisToolShapeUtils::FillStyle fillStyle);
    ~KisFigurePaintingToolHelper();

    void paintLine(const KisPaintInformation &pi0,
                   const KisPaintInformation &pi1);
    void paintPolyline(const vQPointF &points);
    void paintPolygon(const vQPointF &points);
    void paintRect(const QRectF &rect);
    void paintEllipse(const QRectF &rect);
    void paintPainterPath(const QPainterPath &path);
    void setFGColorOverride(const KoColor &color);
    void setBGColorOverride(const KoColor &color);
    void setSelectionOverride(KisSelectionSP m_selection);
    void setBrush(const KisPaintOpPresetSP &brush);
    void paintPainterPathQPen(const QPainterPath, const QPen &pen, const KoColor &color);
    void paintPainterPathQPenFill(const QPainterPath, const QPen &pen, const KoColor &color);

private:
    void setupPaintStyles(KisResourcesSnapshotSP resources,
                          KisToolShapeUtils::StrokeStyle strokeStyle,
                          KisToolShapeUtils::FillStyle fillStyle);

private:
    KisStrokeId m_strokeId;
    KisResourcesSnapshotSP m_resources;
    KisStrokesFacade *m_strokesFacade;
};

#endif /* __KIS_FIGURE_PAINTING_TOOL_HELPER_H */
