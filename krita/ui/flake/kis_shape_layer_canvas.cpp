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


#include <QPainter>

#include <KoShapeManager.h>
#include <KoViewConverter.h>
#include <KoColorSpace.h>

#include <kis_paint_device.h>
#include <kis_image.h>
#include <kis_layer.h>
#include <kis_painter.h>
#include <flake/kis_shape_layer.h>
#include <KoCompositeOp.h>
#include <KoSelection.h>

#include <kis_debug.h>

//#define DEBUG_REPAINT

KisShapeLayerCanvas::KisShapeLayerCanvas(KisShapeLayer *parent, KoViewConverter * viewConverter)
        : QObject(parent)
        , KoCanvasBase(0)
        , m_viewConverter(viewConverter)
        , m_shapeManager(new KoShapeManager(this))
        , m_projection(0)
        , m_parentLayer(parent)
        , m_repaintTriggered(false)
        , m_antialias(false)
{
    m_shapeManager->selection()->setActiveLayer(parent);
}

KisShapeLayerCanvas::~KisShapeLayerCanvas()
{
}

void KisShapeLayerCanvas::gridSize(qreal *horizontal, qreal *vertical) const
{
    Q_ASSERT(false); // This should never be called as this canvas should have no tools.
    Q_UNUSED(horizontal);
    Q_UNUSED(vertical);
}

bool KisShapeLayerCanvas::snapToGrid() const
{
    Q_ASSERT(false); // This should never be called as this canvas should have no tools.
    return false;
}

void KisShapeLayerCanvas::addCommand(QUndoCommand *)
{
    Q_ASSERT(false); // This should never be called as this canvas should have no tools.
}

KoShapeManager *KisShapeLayerCanvas::shapeManager() const
{
    return m_shapeManager;
}

#ifdef DEBUG_REPAINT
# include <stdlib.h>
#endif

void KisShapeLayerCanvas::updateCanvas(const QRectF& rc)
{
    dbgImage << "KisShapeLayerCanvas::updateCanvas()" << rc;

    QRect r = m_viewConverter->documentToView(rc).toRect();
    r.adjust(-2, -2, 2, 2); // for antialias
    m_dirty += r;
    if (! m_repaintTriggered) {
        qreal x, y;
        m_viewConverter->zoom(&x, &y);
        m_antialias = x < 3 || y < 3;

        QTimer::singleShot(0, this, SLOT(repaint()));
        m_repaintTriggered = true;
    }
}

void KisShapeLayerCanvas::repaint()
{
    QRect r = m_dirty.boundingRect();
    QImage image(r.width(), r.height(), QImage::Format_ARGB32);
    image.fill(0);
    QPainter p(&image);

    p.setRenderHint(QPainter::Antialiasing, m_antialias);
    p.setRenderHint(QPainter::TextAntialiasing, m_antialias);
    p.translate(-r.x(), -r.y());
    p.setClipRect(r);
#ifdef DEBUG_REPAINT
    QColor color = QColor(random() % 255, random() % 255, random() % 255);
    p.fillRect(r, color);
#endif

    m_shapeManager->paint(p, *m_viewConverter, false);
    p.end();

    KisPaintDeviceSP dev = new KisPaintDevice(m_projection->colorSpace());
    dev->convertFromQImage(image, "");
    KisPainter kp(m_projection.data());
    kp.setCompositeOp(m_projection->colorSpace()->compositeOp(COMPOSITE_COPY));
    kp.bitBlt(r.x(), r.y(), dev, 0, 0, r.width(), r.height());
    kp.end();
    m_parentLayer->setDirty(r);
    m_dirty = QRegion();
    m_repaintTriggered = false;
}

KoToolProxy * KisShapeLayerCanvas::toolProxy() const
{
//     Q_ASSERT(false); // This should never be called as this canvas should have no tools.
    return 0;
}

const KoViewConverter *KisShapeLayerCanvas::viewConverter() const
{
    return m_viewConverter;
}

QWidget* KisShapeLayerCanvas::canvasWidget()
{
    return 0;
}

const QWidget* KisShapeLayerCanvas::canvasWidget() const
{
    return 0;
}

KoUnit KisShapeLayerCanvas::unit() const
{
    Q_ASSERT(false); // This should never be called as this canvas should have no tools.
    return KoUnit(KoUnit::Point);
}

#include "kis_shape_layer_canvas.moc"
