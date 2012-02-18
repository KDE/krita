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
#include "krita_export.h"
#include "kis_paint_information.h"
#include "strokes/freehand_stroke.h"


class KoCanvasResourceManager;
class KisStrokesFacade;

class KRITAUI_EXPORT KisFigurePaintingToolHelper
{
protected:
    typedef FreehandStrokeStrategy::PainterInfo PainterInfo;

public:
    KisFigurePaintingToolHelper(const QString &name,
                                KisImageWSP image,
                                KoCanvasResourceManager *resourceManager,
                                KisPainter::StrokeStyle strokeStyle,
                                KisPainter::FillStyle fillStyle);
    ~KisFigurePaintingToolHelper();

    void paintLine(const KisPaintInformation &pi0,
                   const KisPaintInformation &pi1);
    void paintPolyline(const vQPointF &points);
    void paintPolygon(const vQPointF &points);
    void paintRect(const QRectF &rect);
    void paintEllipse(const QRectF &rect);
    void paintPainterPath(const QPainterPath &path);

private:
    KisStrokeId m_strokeId;
    KisResourcesSnapshotSP m_resources;
    PainterInfo *m_painterInfo;
    KisStrokesFacade *m_strokesFacade;
};

#endif /* __KIS_FIGURE_PAINTING_TOOL_HELPER_H */
