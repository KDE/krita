/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
                                KisToolShapeUtils::FillStyle fillStyle,
                                QTransform fillTransform = QTransform());
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
                          KisToolShapeUtils::FillStyle fillStyle,
                          QTransform fillTransform);

private:
    KisStrokeId m_strokeId;
    KisResourcesSnapshotSP m_resources;
    KisStrokesFacade *m_strokesFacade;
};

#endif /* __KIS_FIGURE_PAINTING_TOOL_HELPER_H */
