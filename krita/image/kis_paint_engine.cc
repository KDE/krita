/*
 *  copyright (c) 2006 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 *  02110-1301, USA.
 */
#include "kis_paint_engine.h"

#include <QPaintEngine>
#include <QPaintDevice>
#include <QPaintEngineState>
#include <QPolygon>
#include <QRect>
#include <QRectF>
#include <QLine>
#include <QLineF>
#include <QPainterPath>

#include "kis_paint_device.h"
#include "kis_painter.h"

class KisPaintEngine::KisPaintEnginePrivate {
public:
    KisPaintDevice * dev;
    QPaintEngineState state;
};

KisPaintEngine::KisPaintEngine()
{
    m_d = new KisPaintEnginePrivate;
}

KisPaintEngine::~KisPaintEngine()
{
    delete m_d;
}

bool KisPaintEngine::begin(QPaintDevice *pdev)
{
    KisPaintDevice * dev = dynamic_cast<KisPaintDevice*>( pdev );
    Q_ASSERT_X(dev, "KisPaintEngine::begin",
               "Can only work on KisPaintDevices, nothing else!");
    m_d->dev = dev;
    // XXX: Start transaction for undo?
    return true;
}

bool KisPaintEngine::end()
{
    // XXX: End transaction for undo?
    return true;
}


void KisPaintEngine::updateState(const QPaintEngineState &state)
{
    m_d->state = state;
}


void KisPaintEngine::drawRects(const QRect *rects, int rectCount)
{
    QPaintEngine::drawRects( rects, rectCount );
}

void KisPaintEngine::drawRects(const QRectF *rects, int rectCount)
{
    QPaintEngine::drawRects( rects, rectCount );
}


void KisPaintEngine::drawLines(const QLine *lines, int lineCount)
{
    QPaintEngine::drawLines( lines, lineCount );
}

void KisPaintEngine::drawLines(const QLineF *lines, int lineCount)
{
    QPaintEngine::drawLines( lines, lineCount );
}


void KisPaintEngine::drawEllipse(const QRectF &r)
{
    QPaintEngine::drawEllipse( r );
}

void KisPaintEngine::drawEllipse(const QRect &r)
{
    QPaintEngine::drawEllipse( r );
}


void KisPaintEngine::drawPath(const QPainterPath &path)
{
    // XXX: Implement!
    QPaintEngine::drawPath( path );
}


void KisPaintEngine::drawPoints(const QPointF *points, int pointCount)
{
    QPaintEngine::drawPoints( points, pointCount );
}

void KisPaintEngine::drawPoints(const QPoint *points, int pointCount)
{
    QPaintEngine::drawPoints( points, pointCount );
}


void KisPaintEngine::drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode)
{
    QPaintEngine::drawPolygon( points, pointCount, mode );
}

void KisPaintEngine::drawPolygon(const QPoint *points, int pointCount, PolygonDrawMode mode)
{
    QPaintEngine::drawPolygon( points, pointCount, mode );
}


void KisPaintEngine::drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr)
{
    drawImage( r, pm.toImage(), sr );
}

void KisPaintEngine::drawTextItem(const QPointF &p, const QTextItem &textItem)
{
    QPaintEngine::drawTextItem( p, textItem );
}

void KisPaintEngine::drawTiledPixmap(const QRectF &r, const QPixmap &pixmap, const QPointF &s)
{
    // XXX: Reimplement this, the default will convert the pixmap time
    // and again to a QImage
    QPaintEngine::drawTiledPixmap( r, pixmap, s );
}

void KisPaintEngine::drawImage(const QRectF &r, const QImage &pm, const QRectF &sr,
                               Qt::ImageConversionFlags flags)
{
    Q_UNUSED( flags );
    // XXX: How about sub-pixel bitBlting?
    QRect srcRect = sr.toRect();
    QRect dstRect = r.toRect();

    KisPainter p( m_d->dev );
    // XXX: Get the right porter-duff composite op from the state, for
    // now use OVER.

    p.bitBlt(dstRect.x(), dstRect.y(), m_d->dev->colorSpace()->compositeOp( COMPOSITE_OVER ),
             &pm, static_cast<quint8>( m_d->state.opacity() * 255 ),
             srcRect.x(), srcRect.y(), srcRect.width(), srcRect.height());
}
