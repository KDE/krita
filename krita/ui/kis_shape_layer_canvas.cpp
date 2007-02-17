/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
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

#include "kis_shape_layer_canvas.h"

#include <KoShapeManager.h>
#include <KoViewConverter.h>

#include <kis_paint_device.h>
#include <kis_painter.h>
#include <KoCompositeOp.h>

KisShapeLayerCanvas::KisShapeLayerCanvas(KoViewConverter * viewConverter)
    : KoCanvasBase( 0 )
    , m_viewConverter( viewConverter )
    , m_shapeManager( new KoShapeManager( this ) )
    , m_projection( 0 )
{
}

KisShapeLayerCanvas::~KisShapeLayerCanvas()
{
}

void KisShapeLayerCanvas::gridSize(double *horizontal, double *vertical) const
{
    // XXX: implement grids & snapping to grids for shapes in Krita
    Q_UNUSED( horizontal );
    Q_UNUSED( vertical );
}

bool KisShapeLayerCanvas::snapToGrid() const
{
    // XXX: implement snapping to grids for shapes in a layer
    return false;
}

void KisShapeLayerCanvas::addCommand(QUndoCommand *command)
{
    // XXX: implement this one way or another! (via the image?)
    kDebug(41001) << "KisShapeLayerCanvas::addCommand()" << endl;
}

KoShapeManager *KisShapeLayerCanvas::shapeManager() const
{
    return m_shapeManager;
}

void KisShapeLayerCanvas::updateCanvas(const QRectF& rc)
{
    // XXX: This is the interesting bit! Render the shapes in the
    // shape manager onto the projection.
    kDebug(41001) << "KisShapeLayerCanvas::updateCanvas()" << endl;


    // XXX: Convert rc to krita image pixels

    // Create the image as big as the pixels size calculated above

    QImage img(1024, 768, QImage::Format_ARGB32);

    QPainter p;

    p.begin(&img);

    // Translate the painter so that rc.x(),rc.y() == 0,0, to fit with the qimage we created

    p.setClipRect(rc.toRect());

    m_shapeManager->paint(p, *m_viewConverter, false);
    p.end();

    KisPainter kp(m_projection.data());

    // Use the image pixels rect we calculated above here
    QRect r = rc.toRect();
    kp.bitBlt(r.x(), r.y(), m_projection->colorSpace()->compositeOp( COMPOSITE_OVER ), &img,
              OPACITY_OPAQUE, r.x(), r.y(), r.width(), r.height());
    kp.end();
}

KoToolProxy * KisShapeLayerCanvas::toolProxy()
{
    return 0;
}

KoViewConverter *KisShapeLayerCanvas::viewConverter()
{
    return m_viewConverter;
}

QWidget* KisShapeLayerCanvas::canvasWidget()
{
    return 0;
}

KoUnit KisShapeLayerCanvas::unit()
{
    // XXX: Is this right?
    return KoUnit(KoUnit::Point);
}
