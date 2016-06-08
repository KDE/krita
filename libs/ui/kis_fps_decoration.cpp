/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_fps_decoration.h"

#include <QPainter>
#include "kis_canvas2.h"
#include "kis_coordinates_converter.h"
#include "opengl/kis_opengl_canvas_debugger.h"

const QString KisFpsDecoration::idTag = "fps_decoration";

KisFpsDecoration::KisFpsDecoration(QPointer<KisView> view)
    : KisCanvasDecoration(idTag, view)
{
    setVisible(true);
}

KisFpsDecoration::~KisFpsDecoration()
{
}

void KisFpsDecoration::drawDecoration(QPainter& gc, const QRectF& /*updateRect*/, const KisCoordinatesConverter */*converter*/, KisCanvas2* /*canvas*/)
{
    const qreal value = KisOpenglCanvasDebugger::instance()->accumulatedFps();
    const QString text = QString("FPS: %1").arg(value);

    gc.save();
    gc.setPen(QPen(Qt::white));
    gc.drawText(QPoint(21,31), text);
    gc.setPen(QPen(Qt::black));
    gc.drawText(QPoint(20,30), text);
    gc.restore();
}
