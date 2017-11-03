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
#include <KisStrokeSpeedMonitor.h>

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
#ifdef Q_OS_OSX
    QPixmap pixmap(320, 128);
    pixmap.fill(Qt::transparent);
    {
        QPainter painter(&pixmap);
        draw(painter);
    }
    gc.drawPixmap(0, 0, pixmap);
#else
    draw(gc);
#endif
}



void KisFpsDecoration::draw(QPainter& gc)
{
    QStringList lines;

    if (KisOpenglCanvasDebugger::instance()->showFpsOnCanvas()) {
        const qreal value = KisOpenglCanvasDebugger::instance()->accumulatedFps();
        lines << QString("Canvas FPS: %1").arg(QString::number(value, 'f', 1));
    }

    KisStrokeSpeedMonitor *monitor = KisStrokeSpeedMonitor::instance();

    if (monitor->haveStrokeSpeedMeasurement()) {
        lines << QString("Last cursor/brush speed (px/ms): %1/%2%3")
                     .arg(monitor->lastCursorSpeed(), 0, 'f', 1)
                     .arg(monitor->lastRenderingSpeed(), 0, 'f', 1)
                     .arg(monitor->lastStrokeSaturated() ? " (!)" : "");
        lines << QString("Last brush framerate: %1 fps")
                     .arg(monitor->lastFps(), 0, 'f', 1);

        lines << QString("Average cursor/brush speed (px/ms): %1/%2")
                     .arg(monitor->avgCursorSpeed(), 0, 'f', 1)
                     .arg(monitor->avgRenderingSpeed(), 0, 'f', 1);
        lines << QString("Average brush framerate: %1 fps")
                     .arg(monitor->avgFps(), 0, 'f', 1);
    }


    QPoint startPoint(20,30);
    const int lineSpacing = QFontMetrics(gc.font()).lineSpacing();


    gc.save();

    Q_FOREACH (const QString &line, lines) {
        gc.setPen(QPen(Qt::white));
        gc.drawText(startPoint + QPoint(1,1), line);
        gc.setPen(QPen(Qt::black));
        gc.drawText(startPoint, line);

        startPoint += QPoint(0, lineSpacing);
    }

    gc.restore();
}
