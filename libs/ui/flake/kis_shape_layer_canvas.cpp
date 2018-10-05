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

#include <kis_spontaneous_job.h>
#include "kis_global.h"

//#define DEBUG_REPAINT

KisShapeLayerCanvasBase::KisShapeLayerCanvasBase(KisShapeLayer *parent, KisImageWSP image)
    : KoCanvasBase(0)
    , m_viewConverter(new KisImageViewConverter(image))
    , m_shapeManager(new KoShapeManager(this))
    , m_selectedShapesProxy(new KoSelectedShapesProxySimple(m_shapeManager.data()))
{
    m_shapeManager->selection()->setActiveLayer(parent);
}

KoShapeManager *KisShapeLayerCanvasBase::shapeManager() const
{
    return m_shapeManager.data();
}

KoSelectedShapesProxy *KisShapeLayerCanvasBase::selectedShapesProxy() const
{
    return m_selectedShapesProxy.data();
}

KoViewConverter* KisShapeLayerCanvasBase::viewConverter() const
{
    return m_viewConverter.data();
}

void KisShapeLayerCanvasBase::gridSize(QPointF *offset, QSizeF *spacing) const
{
    KIS_SAFE_ASSERT_RECOVER_NOOP(false); // This should never be called as this canvas should have no tools.
    Q_UNUSED(offset);
    Q_UNUSED(spacing);
}

bool KisShapeLayerCanvasBase::snapToGrid() const
{
    KIS_SAFE_ASSERT_RECOVER_NOOP(false); // This should never be called as this canvas should have no tools.
    return false;
}

void KisShapeLayerCanvasBase::addCommand(KUndo2Command *)
{
    KIS_SAFE_ASSERT_RECOVER_NOOP(false); // This should never be called as this canvas should have no tools.
}


KoToolProxy * KisShapeLayerCanvasBase::toolProxy() const
{
//     KIS_SAFE_ASSERT_RECOVER_NOOP(false); // This should never be called as this canvas should have no tools.
    return 0;
}

QWidget* KisShapeLayerCanvasBase::canvasWidget()
{
    return 0;
}

const QWidget* KisShapeLayerCanvasBase::canvasWidget() const
{
    return 0;
}

KoUnit KisShapeLayerCanvasBase::unit() const
{
    KIS_SAFE_ASSERT_RECOVER_NOOP(false); // This should never be called as this canvas should have no tools.
    return KoUnit(KoUnit::Point);
}

void KisShapeLayerCanvasBase::prepareForDestroying()
{
    m_isDestroying = true;
}

bool KisShapeLayerCanvasBase::hasChangedWhileBeingInvisible()
{
    return m_hasChangedWhileBeingInvisible;
}


KisShapeLayerCanvas::KisShapeLayerCanvas(KisShapeLayer *parent, KisImageWSP image)
        : KisShapeLayerCanvasBase(parent, image)
        , m_projection(0)
        , m_parentLayer(parent)
        , m_asyncUpdateSignalCompressor(100, KisSignalCompressor::FIRST_INACTIVE)
{
    /**
     * The layour should also add itself to its own shape manager, so that the canvas
     * would track its changes/transformations
     */
    m_shapeManager->addShape(parent, KoShapeManager::AddWithoutRepaint);
    m_shapeManager->selection()->setActiveLayer(parent);

    connect(this, SIGNAL(forwardRepaint()), SLOT(repaint()), Qt::QueuedConnection);
    connect(&m_asyncUpdateSignalCompressor, SIGNAL(timeout()), SLOT(slotStartAsyncRepaint()));

    setImage(image);
}

KisShapeLayerCanvas::~KisShapeLayerCanvas()
{
    m_shapeManager->remove(m_parentLayer);
}

void KisShapeLayerCanvas::setImage(KisImageWSP image)
{
    if (m_image) {
        disconnect(m_image, 0, this, 0);
    }

    m_viewConverter->setImage(image);
    m_image = image;

    if (image) {
        connect(m_image, SIGNAL(sigSizeChanged(QPointF,QPointF)), SLOT(slotImageSizeChanged()));
        m_cachedImageRect = m_image->bounds();
    }
}


#ifdef DEBUG_REPAINT
# include <stdlib.h>
#endif


class KisRepaintShapeLayerLayerJob : public KisSpontaneousJob
{
public:
    KisRepaintShapeLayerLayerJob(KisShapeLayerSP layer, KisShapeLayerCanvas *canvas)
        : m_layer(layer),
          m_canvas(canvas)
    {
    }

    bool overrides(const KisSpontaneousJob *_otherJob) override {
        const KisRepaintShapeLayerLayerJob *otherJob =
            dynamic_cast<const KisRepaintShapeLayerLayerJob*>(_otherJob);

        return otherJob && otherJob->m_canvas == m_canvas;
    }

    void run() override {
        m_canvas->repaint();
    }

    int levelOfDetail() const override {
        return 0;
    }

private:

    // we store a pointer to the layer just
    // to keep the lifetime of the canvas!
    KisShapeLayerSP m_layer;

    KisShapeLayerCanvas *m_canvas;
};


void KisShapeLayerCanvas::updateCanvas(const QVector<QRectF> &region)
{
    if (!m_parentLayer->image() || m_isDestroying) {
        return;
    }

    {
        QMutexLocker locker(&m_dirtyRegionMutex);
        Q_FOREACH (const QRectF &rc, region) {
            // grow for antialiasing
            const QRect imageRect = kisGrowRect(m_viewConverter->documentToView(rc).toAlignedRect(), 2);
            m_dirtyRegion += imageRect;
        }
    }

    /**
     * HACK ALERT!
     *
     * The shapes may be accessed from both, GUI and worker threads! And we have no real
     * guard against this until the vector tools will be ported to the strokes framework.
     *
     * Here we just avoid the most obvious conflict of threads:
     *
     * 1) If the layer if modified by a non-gui (worker) thread, use a spontaneous jobs
     *    to rerender the canvas. The job will be executed (almost) exclusively and it is
     *    the responsibility of the worker thread to add a barrier to wait until this job is
     *    completed, and not try to access the shapes concurrently.
     *
     * 2) If the layer is modified by a gui thread, it means that we are being accessed by
     *    a legacy vector tool. It this case just emit a queued signal to make sure the updates
     *    are compressed a little bit (TODO: add a compressor?)
     */

    if (qApp->thread() == QThread::currentThread()) {
        emit forwardRepaint();
    } else {
        m_asyncUpdateSignalCompressor.start();
        m_hasUpdateInCompressor = true;
    }
}


void KisShapeLayerCanvas::updateCanvas(const QRectF& rc)
{
    updateCanvas(QVector<QRectF>({rc}));
}

void KisShapeLayerCanvas::slotStartAsyncRepaint()
{
    m_hasUpdateInCompressor = false;
    m_image->addSpontaneousJob(new KisRepaintShapeLayerLayerJob(m_parentLayer, this));
}

void KisShapeLayerCanvas::slotImageSizeChanged()
{
    QRegion dirtyCacheRegion;
    dirtyCacheRegion += m_image->bounds();
    dirtyCacheRegion += m_cachedImageRect;
    dirtyCacheRegion -= m_image->bounds() & m_cachedImageRect;

    QVector<QRectF> dirtyRects;
    Q_FOREACH (const QRect &rc, dirtyCacheRegion.rects()) {
        dirtyRects.append(m_viewConverter->viewToDocument(rc));
    }
    updateCanvas(dirtyRects);

    m_cachedImageRect = m_image->bounds();
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

    // Crop the update rect by the image bounds. We keep the cache consistent
    // by tracking the size of the image in slotImageSizeChanged()
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

    m_hasChangedWhileBeingInvisible |= !m_parentLayer->visible(true);
}

void KisShapeLayerCanvas::forceRepaint()
{
    /**
     * WARNING! Although forceRepaint() may be called from different threads, it is
     * not entirely safe. If the user plays with shapes at the same time (vector tools are
     * not ported to strokes yet), the shapes my be accessed from two different places at
     * the same time, which will cause a crash.
     *
     * The only real solution to this is to port vector tools to strokes framework.
     */

    if (m_hasUpdateInCompressor) {
        m_asyncUpdateSignalCompressor.stop();
        slotStartAsyncRepaint();
    }
}

void KisShapeLayerCanvas::resetCache()
{
    m_projection->clear();

    QList<KoShape*> shapes = m_shapeManager->shapes();
    Q_FOREACH (const KoShape* shape, shapes) {
        shape->update();
    }
}

void KisShapeLayerCanvas::rerenderAfterBeingInvisible()
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_parentLayer->visible(true))

    m_hasChangedWhileBeingInvisible = false;
    resetCache();
}

