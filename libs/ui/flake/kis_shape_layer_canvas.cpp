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
#include <QMutexLocker>

#include <KoShapeManager.h>
#include <KoSelectedShapesProxySimple.h>
#include <KoViewConverter.h>
#include <KoColorSpace.h>

#include <kis_paint_device.h>
#include <kis_image.h>
#include <kis_layer.h>
#include <kis_painter.h>
#include <flake/kis_shape_layer.h>
#include <KoCompositeOpRegistry.h>
#include <KoSelection.h>
#include <KoUnit.h>
#include "kis_image_view_converter.h"

#include <kis_debug.h>

#include <QThread>
#include <QApplication>

//#define DEBUG_REPAINT

KisShapeLayerCanvas::KisShapeLayerCanvas(KisShapeLayer *parent, KisImageWSP image)
        : QObject()
        , KoCanvasBase(0)
        , m_isDestroying(false)
        , m_viewConverter(new KisImageViewConverter(image))
        , m_shapeManager(new KoShapeManager(this))
        , m_selectedShapesProxy(new KoSelectedShapesProxySimple(m_shapeManager.data()))
        , m_projection(0)
        , m_parentLayer(parent)
{
    m_shapeManager->selection()->setActiveLayer(parent);
    connect(this, SIGNAL(forwardRepaint()), SLOT(repaint()), Qt::QueuedConnection);
}

KisShapeLayerCanvas::~KisShapeLayerCanvas()
{
}

void KisShapeLayerCanvas::setImage(KisImageWSP image)
{
    m_viewConverter->setImage(image);
}

void KisShapeLayerCanvas::prepareForDestroying()
{
    m_isDestroying = true;
}

void KisShapeLayerCanvas::gridSize(QPointF *offset, QSizeF *spacing) const
{
    Q_ASSERT(false); // This should never be called as this canvas should have no tools.
    Q_UNUSED(offset);
    Q_UNUSED(spacing);
}

bool KisShapeLayerCanvas::snapToGrid() const
{
    Q_ASSERT(false); // This should never be called as this canvas should have no tools.
    return false;
}

void KisShapeLayerCanvas::addCommand(KUndo2Command *)
{
    Q_ASSERT(false); // This should never be called as this canvas should have no tools.
}

KoShapeManager *KisShapeLayerCanvas::shapeManager() const
{
    return m_shapeManager.data();
}

KoSelectedShapesProxy *KisShapeLayerCanvas::selectedShapesProxy() const
{
    return m_selectedShapesProxy.data();
}

#ifdef DEBUG_REPAINT
# include <stdlib.h>
#endif

void KisShapeLayerCanvas::updateCanvas(const QRectF& rc)
{
    dbgUI << "KisShapeLayerCanvas::updateCanvas()" << rc;
    //image is 0, if parentLayer is being deleted so don't update
    if (!m_parentLayer->image() || m_isDestroying) {
        return;
    }

    QRect r = m_viewConverter->documentToView(rc).toRect();
    r.adjust(-2, -2, 2, 2); // for antialias

    {
        QMutexLocker locker(&m_dirtyRegionMutex);
        m_dirtyRegion += r;
        qreal x, y;
        m_viewConverter->zoom(&x, &y);
    }

    emit forwardRepaint();
}

void KisShapeLayerCanvas::repaint()
{
    QRect r;

    {
        QMutexLocker locker(&m_dirtyRegionMutex);
        r = m_dirtyRegion.boundingRect();
        m_dirtyRegion = QRegion();
    }

    if (r.isEmpty()) return;

    r = r.intersected(m_parentLayer->image()->bounds());
    QImage image(r.width(), r.height(), QImage::Format_ARGB32);
    image.fill(0);
    QPainter p(&image);

    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);
    p.translate(-r.x(), -r.y());
    p.setClipRect(r);
#ifdef DEBUG_REPAINT
    QColor color = QColor(random() % 255, random() % 255, random() % 255);
    p.fillRect(r, color);
#endif

    m_shapeManager->paint(p, *m_viewConverter, false);
    p.end();

    KisPaintDeviceSP dev = new KisPaintDevice(m_projection->colorSpace());
    dev->convertFromQImage(image, 0);

    KisPainter::copyAreaOptimized(r.topLeft(), dev, m_projection, QRect(QPoint(), r.size()));

    m_parentLayer->setDirty(r);
}

KoToolProxy * KisShapeLayerCanvas::toolProxy() const
{
//     Q_ASSERT(false); // This should never be called as this canvas should have no tools.
    return 0;
}

KoViewConverter* KisShapeLayerCanvas::viewConverter() const
{
    return m_viewConverter.data();
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

void KisShapeLayerCanvas::forceRepaint()
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(qApp->thread() == QThread::currentThread());
    repaint();
}

