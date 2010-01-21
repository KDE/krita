/*
 *  Copyright (c) 2009,2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include <KoViewConverter.h>

#include <kis_paint_action_type_option.h>
#include <kis_color_option.h>

#include "kis_experiment_paintop_settings.h"
#include "kis_experimentop_option.h"
#include "kis_experiment_shape_option.h"

bool KisExperimentPaintOpSettings::paintIncremental()
{
    return (enumPaintActionType)getInt("PaintOpAction", WASH) == BUILDUP;
}


QRectF KisExperimentPaintOpSettings::paintOutlineRect(const QPointF& pos, KisImageWSP image, OutlineMode _mode) const
{
    if (_mode != CURSOR_IS_OUTLINE) return QRectF();
    qreal width = getInt(EXPERIMENT_START_SIZE); /* scale();*/
    qreal height = getInt(EXPERIMENT_START_SIZE); /* scale();*/
    width += 10;
    height += 10;
    QRectF rc = QRectF(0, 0, width, height);
    return image->pixelToDocument(rc.translated(- QPoint(width * 0.5, height * 0.5))).translated(pos);
}

void KisExperimentPaintOpSettings::paintOutline(const QPointF& pos, KisImageWSP image, QPainter &painter, const KoViewConverter &converter, OutlineMode _mode) const
{
    if (_mode != CURSOR_IS_OUTLINE) return;
    qreal width = getInt(EXPERIMENT_START_SIZE); /* scale();*/
    qreal height = getInt(EXPERIMENT_START_SIZE); /* scale();*/
    painter.setPen(QColor(255,128,255));
    painter.setCompositionMode(QPainter::RasterOp_SourceXorDestination);
    painter.drawEllipse(converter.documentToView(image->pixelToDocument(QRectF(0, 0, width, height).translated(- QPoint(width * 0.5, height * 0.5))).translated(pos)));
}
