/* This file is part of the KDE project
   Made by Emanuele Tamponi (emanuele@valinor.it)
   Copyright (C) 2007 Emanuele Tamponi

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include <QFrame>

#include <KDebug>

#include <KoCanvasBase.h>
#include <KoCanvasResourceProvider.h>
#include <KoColor.h>
#include <KoColorSpace.h>
#include <KoColorProfile.h>
#include <KoID.h>
#include <KoPointerEvent.h>
#include <KoShapeManager.h>
#include <KoToolProxy.h>

#include "kis_paint_device.h"
#include "kis_painterly_overlay.h"
#include "kis_paint_layer.h"
#include "kis_painter.h"
#include "kis_paintop.h"
#include "kis_paintop_registry.h"
#include "kis_resource_provider.h"

#include "colorspot.h"
#include "mixertool.h"

#include "mixercanvas.h"

MixerCanvas::MixerCanvas(QWidget *parent)
    : QFrame(parent), KoCanvasBase(0), m_toolProxy(0)
{
    m_dirty = false;
    m_image = QImage(size(), QImage::Format_ARGB32);
    m_image.fill(0);
}

MixerCanvas::~MixerCanvas()
{
    if (m_toolProxy)
        delete m_toolProxy;
}

void MixerCanvas::setResources(KoCanvasResourceProvider *rp)
{
    KoCanvasResourceProvider *internal = resourceProvider();

    internal->setResource(KoCanvasResource::ForegroundColor,
                          rp->resource(KoCanvasResource::ForegroundColor));
    internal->setResource(KoCanvasResource::BackgroundColor,
                          rp->resource(KoCanvasResource::BackgroundColor));
    internal->setResource(KisResourceProvider::CurrentBrush,
                          rp->resource(KisResourceProvider::CurrentBrush));
    internal->setResource(KisResourceProvider::CurrentPattern,
                          rp->resource(KisResourceProvider::CurrentPattern));
    internal->setResource(KisResourceProvider::CurrentGradient,
                          rp->resource(KisResourceProvider::CurrentGradient));
    internal->setResource(KisResourceProvider::CurrentComplexColor,
                          rp->resource(KisResourceProvider::CurrentComplexColor));

    internal->setResource(KisResourceProvider::CurrentKritaLayer,
                          rp->resource(KisResourceProvider::CurrentKritaLayer));
    internal->setResource(KisResourceProvider::HdrExposure,
                          rp->resource(KisResourceProvider::HdrExposure));
    internal->setResource(KisResourceProvider::CurrentPaintop,
                          rp->resource(KisResourceProvider::CurrentPaintop));

    initPaintopSettings();
    checkCurrentPaintop();

    connect(rp, SIGNAL(resourceChanged(int, const QVariant &)), this, SLOT(slotResourceChanged(int, const QVariant &)));
}

void MixerCanvas::initPaintopSettings()
{
    // TODO Write here when the paintop will have the settings...
}

void MixerCanvas::checkCurrentPaintop()
{
    KoCanvasResourceProvider *internal = resourceProvider();

    KisPainter painter(device());
    KisPaintOp *current = KisPaintOpRegistry::instance()->paintOp(
                          internal->resource(KisResourceProvider::CurrentPaintop).value<KoID>().id(),
                          static_cast<KisPaintOpSettings*>(
                          internal->resource(KisResourceProvider::CurrentPaintopSettings).value<void *>()),
                          &painter, 0);
    painter.setPaintOp(current);

    if (!current->painterly())
        internal->setResource(KisResourceProvider::CurrentPaintop, KoID("paintcomplex", ""));
}

void MixerCanvas::checkCurrentLayer()
{
    KoCanvasResourceProvider *internal = resourceProvider();
    KisLayerSP current = internal->resource(KisResourceProvider::CurrentKritaLayer).value<KisLayerSP>();
    if (current.data() != m_layer.data()) {
        QVariant v;
        v.setValue(KisLayerSP(m_layer.data()));
        internal->setResource(KisResourceProvider::CurrentKritaLayer, v);
    }
}

void MixerCanvas::setLayer(KoColorSpace *cs)
{
    m_layer = new KisPaintLayer(0, "Artistic Mixer Layer", 255, cs);
    m_layer->paintDevice()->createPainterlyOverlay();
}

void MixerCanvas::mouseDoubleClickEvent(QMouseEvent */*event*/)
{
//     m_toolProxy->mouseDoubleClickEvent(event, event->pos());
}

void MixerCanvas::mouseMoveEvent(QMouseEvent *event)
{
    m_toolProxy->mouseMoveEvent(event, event->pos());
}

void MixerCanvas::mousePressEvent(QMouseEvent *event)
{
    m_toolProxy->mousePressEvent(event, event->pos());
}

void MixerCanvas::mouseReleaseEvent(QMouseEvent *event)
{
    m_toolProxy->mouseReleaseEvent(event, event->pos());
}

void MixerCanvas::tabletEvent(QTabletEvent *event)
{
    m_toolProxy->tabletEvent(event, event->pos());
}

void MixerCanvas::resizeEvent(QResizeEvent *event)
{
    if (event->size().width() > m_image.width() ||
        event->size().height() > m_image.height()) {
        QImage newImg(event->size(), QImage::Format_ARGB32);
        newImg.fill(0);

        QPainter p(&newImg);
        p.drawImage(m_image.rect(), m_image, m_image.rect());
        p.end();

        m_image = newImg;
    }

    QFrame::resizeEvent(event);
}

void MixerCanvas::paintEvent(QPaintEvent *event)
{
    if (m_dirty) {
        QRect r = event->rect();
        QRect imgRect = QRect(0, 0, r.width(), r.height());
        QPainter p(&m_image);
        p.drawImage(r, m_layer->paintDevice()->convertToQImage(0, r.x(), r.y(), r.width(), r.height()), imgRect);
        p.end();

        m_dirty = false;
    }

    QPainter p(this);
    p.drawImage(m_image.rect(), m_image, m_image.rect());
    p.end();

    QFrame::paintEvent(event);
}

void MixerCanvas::updateCanvas(const QRegion& region)
{
    m_dirty = true;
    update(region.boundingRect());
}

void MixerCanvas::slotClear()
{
    device()->clear();
    overlay()->clear();
    m_image.fill(0);
    update();
}

void MixerCanvas::slotResourceChanged(int key, const QVariant &value)
{
    if (key != KisResourceProvider::CurrentPaintopSettings)
        resourceProvider()->setResource(key, value);
    else
        return;

    switch (key) {
        case KisResourceProvider::CurrentPaintop:
            checkCurrentPaintop();
            break;
        case KisResourceProvider::CurrentKritaLayer:
            checkCurrentLayer();
            break;
    }
}


#include "mixercanvas.moc"
