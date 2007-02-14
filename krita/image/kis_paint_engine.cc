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

#include <QtCore>
#include <QtGui>

#include "kis_paint_device.h"
#include "kis_painter.h"
#include "kis_brush.h"
#include "kis_paintop.h"
#include "kis_iterators_pixel.h"
#include "KoCompositeOp.h"

class KisInternalPaintOp : public KisPaintOp {

    typedef KisPaintOp super;

public:
    KisInternalPaintOp(KisPainter * painter);
    ~KisInternalPaintOp();

    void paintAt(const QPointF &pos, const KisPaintInformation& info);

};

KisInternalPaintOp::KisInternalPaintOp(KisPainter * painter) : super (painter)
{
}

KisInternalPaintOp::~KisInternalPaintOp()
{
}

void KisInternalPaintOp::paintAt(const QPointF &pos, const KisPaintInformation& info)
{
    if (!m_painter) return;
    KisPaintDeviceSP device = m_painter->device();
    if (!device) return;
    KisBrush * brush = m_painter->brush();
    if (!brush) return;
    if (! brush->canPaintFor(info) )
        return;

    QPointF hotSpot = brush->hotSpot(info);
    QPointF pt = pos - hotSpot;

    qint32 x = qRound(pt.x());
    qint32 y = qRound(pt.y());

    KisPaintDeviceSP dab = KisPaintDeviceSP(0);
    if (brush->brushType() == IMAGE ||
        brush->brushType() == PIPE_IMAGE) {
        dab = brush->image(device->colorSpace(), info);
    }
    else {
        // Compute mask without sub-pixel positioning
        KisQImagemaskSP mask = brush->mask(info);
        dab = computeDab(mask);
    }

    m_painter->setPressure(info.pressure);
    QRect dabRect = QRect(0, 0, brush->maskWidth(info), brush->maskHeight(info));
    QRect dstRect = QRect(x, y, dabRect.width(), dabRect.height());

    KisImageSP image = device->image();

    if (image != 0) {
        dstRect &= image->bounds();
    }

    if (dstRect.isNull() || dstRect.isEmpty() || !dstRect.isValid()) return;

    KoColorSpace * cs = dab->colorSpace();

    // Set all alpha > opaque/2 to opaque, the rest to transparent.
    // XXX: Using 4/10 as the 1x1 circle brush paints nothing with 0.5.

    KisRectIteratorPixel pixelIt = dab->createRectIterator(dabRect.x(), dabRect.y(), dabRect.width(), dabRect.height());

    while (!pixelIt.isDone()) {
        quint8 alpha = cs->alpha(pixelIt.rawData());

        if (alpha < (4 * OPACITY_OPAQUE) / 10) {
            cs->setAlpha(pixelIt.rawData(), OPACITY_TRANSPARENT, 1);
        } else {
            cs->setAlpha(pixelIt.rawData(), OPACITY_OPAQUE, 1);
        }

        ++pixelIt;
    }

    qint32 sx = dstRect.x() - x;
    qint32 sy = dstRect.y() - y;
    qint32 sw = dstRect.width();
    qint32 sh = dstRect.height();

    if (m_source->hasSelection()) {
        m_painter->bltSelection(dstRect.x(), dstRect.y(), m_painter->compositeOp(), dab,
                                m_source->selection(), m_painter->opacity(), sx, sy, sw, sh);
    }
    else {
        m_painter->bitBlt(dstRect.x(), dstRect.y(), m_painter->compositeOp(), dab, m_painter->opacity(), sx, sy, sw, sh);
    }
}



class KisPaintEngine::KisPaintEnginePrivate {
public:
    KisPaintDevice * dev;
    KisPainter * p;
    KisBrush * brush;
    QMatrix matrix;
    qreal opacity;
    QPainter::RenderHints hints;
    QPointF brushOrigin;
};



KisPaintEngine::KisPaintEngine()
{
    d = new KisPaintEnginePrivate;
    d->p = 0;
    d->brush = 0;

    // Set capabilities
    gccaps = AllFeatures;
}

KisPaintEngine::~KisPaintEngine()
{
    delete d->brush;
    delete d->p;
    delete d;
}

bool KisPaintEngine::begin(QPaintDevice *pdev)
{
    kDebug(41001) << "KisPaintEngine::begin\n";

    KisPaintDevice * dev = dynamic_cast<KisPaintDevice*>( pdev );
    Q_ASSERT_X(dev, "KisPaintEngine::begin",
               "Can only work on KisPaintDevices, nothing else!");
    d->dev = dev;

    initPainter();

    // XXX: Start transaction for undo?
    return true;
}

void KisPaintEngine::initPainter()
{
    KisInternalPaintOp * po;
    QBrush defaultPen(Qt::white);
    QImage img(1, 1, QImage::Format_ARGB32 );
    QPainter p(&img);

    if (d->p)
        delete d->p;
    if (d->brush)
        delete d->brush;

    p.setBrush(defaultPen);
    p.drawRect(0, 0, 1, 1);
    defaultPen.setColor(Qt::black);
    p.setBrush(defaultPen);
    p.drawEllipse(0, 0, 1, 1);
    p.end();

    d->brush = new KisBrush(img);
    d->p = new KisPainter(d->dev);
    po = new KisInternalPaintOp(d->p);
    d->p->setBrush(d->brush);
    d->p->setPaintOp(po);
    d->p->setFillStyle(KisPainter::FillStyleForegroundColor);
}

bool KisPaintEngine::end()
{
    kDebug(41001) << "KisPaintEngine::end\n";
    // XXX: End transaction for undo?
    return true;
}


void KisPaintEngine::updatePen (const QPen& newPen)
{
    kDebug(41001) << "Inside KisPaintEngine::updatePen(): " << ceil(newPen.widthF()) << endl;
    double width = (newPen.width() < 1) ? 1 : newPen.width();
    QImage img((int)width, (int)width, QImage::Format_ARGB32 );
    QPainter p(&img);
    p.setPen(newPen);
    p.drawPoint(QPointF(width/2.0,width/2.0));
    p.end();

    if (d->brush)
        delete d->brush;
    d->brush = new KisBrush(img);

    d->p->setBrush(d->brush);
}

void KisPaintEngine::updateBrush (const QBrush& newBrush, const QPointF& newOrigin)
{
    kDebug(41001) << "Inside KisPaintEngine::updateBrush(): " << newBrush.color() << endl;
    d->p->setPaintColor(KoColor(newBrush.color(), d->dev->colorSpace()));
}


void KisPaintEngine::updateState(const QPaintEngineState &state)
{
    kDebug(41001) << "Inside KisPaintEngine::updateState()" << endl;
    QPaintEngine::DirtyFlags flags = state.state();

    if (flags & DirtyOpacity) {
        d->opacity = state.opacity();
        if (d->opacity > 1)
            d->opacity = 1;
        if (d->opacity < 0)
            d->opacity = 0;
        // Force update pen/brush as to get proper alpha colors propagated
        flags |= DirtyPen;
        flags |= DirtyBrush;
    }

    if (flags & DirtyTransform) d->matrix = state.matrix();
    if (flags & DirtyPen) updatePen(state.pen());
    if (flags & (DirtyBrush | DirtyBrushOrigin)) updateBrush(state.brush(), state.brushOrigin());
//     if (flags & DirtyFont) d->font = state.font();

    if (state.state() & DirtyClipEnabled) {
        // TODO: What is this intended to do? DirtyClipEnabled
    }

    if (flags & DirtyClipPath) {
        // TODO: What is this intended to do? DirtyClipPath
    }
    if (flags & DirtyHints) d->hints = state.renderHints();
}

void KisPaintEngine::drawRects(const QRect *rects, int rectCount)
{
    kDebug(41001) << "KisPaintEngine::drawRects " << rectCount << endl;
    QPaintEngine::drawRects( rects, rectCount );
}

void KisPaintEngine::drawRects(const QRectF *rects, int rectCount)
{
    kDebug(41001) << "KisPaintEngine::drawRects " << rectCount << endl;
    QPaintEngine::drawRects( rects, rectCount );
}


void KisPaintEngine::drawLines(const QLine *lines, int lineCount)
{
    kDebug(41001) << "KisPaintEngine::drawLines " << lineCount << endl;
    QPaintEngine::drawLines( lines, lineCount );
}

void KisPaintEngine::drawLines(const QLineF *lines, int lineCount)
{
    kDebug(41001) << "KisPaintEngine::drawLines " << lineCount << endl;
    QPaintEngine::drawLines( lines, lineCount );
}


void KisPaintEngine::drawEllipse(const QRectF &r)
{
    kDebug(41001) << "KisPaintEngine::drawEllipse in rect " << r << endl;
    QPaintEngine::drawEllipse( r );
}

void KisPaintEngine::drawEllipse(const QRect &r)
{
    kDebug(41001) << "KisPaintEngine::drawEllipse in rect " << r << endl;
    QPaintEngine::drawEllipse( r );
}


void KisPaintEngine::drawPath(const QPainterPath &path)
{
    kDebug(41001) << "KisPaintEngine::drawPath with bounding rect " << d->matrix.mapRect(path.boundingRect()) << endl;
    // Simple-minded implementation

    QList<QPolygonF> polys = path.toFillPolygons(d->matrix);

    d->p->begin(d->dev);

//     QPolygonF poly;
//     foreach (poly, polys)
//         d->p->paintPolygon(poly);

    d->p->paintPolygon(path.toFillPolygon(d->matrix));

    d->p->end();
}


void KisPaintEngine::drawPoints(const QPointF *points, int pointCount)
{
    kDebug(41001) << "KisPaintEngine::drawPoints " << pointCount << endl;
    QPaintEngine::drawPoints( points, pointCount );
}

void KisPaintEngine::drawPoints(const QPoint *points, int pointCount)
{
    kDebug(41001) << "KisPaintEngine::drawPoints: " << pointCount << endl;
    QPaintEngine::drawPoints( points, pointCount );
}


void KisPaintEngine::drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode)
{
    kDebug(41001) << "KisPaintEngine::drawPolygon: " << pointCount << endl;
    QPaintEngine::drawPolygon( points, pointCount, mode );
}

void KisPaintEngine::drawPolygon(const QPoint *points, int pointCount, PolygonDrawMode mode)
{
    kDebug(41001) << "KisPaintEngine::drawPolygon: " << pointCount << endl;
    QPaintEngine::drawPolygon( points, pointCount, mode );
}


void KisPaintEngine::drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr)
{
    kDebug(41001) << "KisPaintEngine::drawPixmap r: " << r << ", sr: " << sr << endl;
    drawImage( r, pm.toImage(), sr );
}

void KisPaintEngine::drawTextItem(const QPointF &p, const QTextItem &textItem)
{
    kDebug(41001) << "KisPaintEngine::drawTextItem p: " << p << endl;
    QPaintEngine::drawTextItem( p, textItem );
}

void KisPaintEngine::drawTiledPixmap(const QRectF &r, const QPixmap &pixmap, const QPointF &s)
{
    kDebug(41001) << "KisPaintEngine::drawTiledPixmap r:" << r << ", s: " << s << endl;
    // XXX: Reimplement this, the default will convert the pixmap time
    // and again to a QImage
    QPaintEngine::drawTiledPixmap( r, pixmap, s );
}

void KisPaintEngine::drawImage(const QRectF &r, const QImage &pm, const QRectF &sr,
                               Qt::ImageConversionFlags flags)
{
    kDebug(41001) << "KisPaintEngine::drawImage r: " << r << ", sr: " << sr << endl;
    Q_UNUSED( flags );
    // XXX: How about sub-pixel bitBlting?
    QRect srcRect = sr.toRect();
    QRect dstRect = r.toRect();

    KisPainter p( d->dev );
    // XXX: Get the right porter-duff composite op from the state, for
    // now use OVER.


#if 0
    p.bitBlt(dstRect.x(), dstRect.y(), d->dev->colorSpace()->compositeOp( COMPOSITE_OVER ),
             &pm, static_cast<quint8>( d->state.opacity() * 255 ),
             srcRect.x(), srcRect.y(), srcRect.width(), srcRect.height());
#else
    KisPaintDeviceSP dev = new KisPaintDevice( d->dev->colorSpace() );
    dev->convertFromQImage(pm, "");
    p.bitBlt(dstRect.x(), dstRect.y(), d->dev->colorSpace()->compositeOp( COMPOSITE_OVER ),
             dev, OPACITY_OPAQUE,
             srcRect.x(), srcRect.y(), srcRect.width(), srcRect.height());


#endif

}
