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

#include "mixercanvas.h"

#include <kis_image.h>
#include <kis_paint_device.h>
#include <kis_painter.h>
#include <kis_painterly_overlay.h>
#include <kis_paint_layer.h>
#include <kis_paintop_registry.h>
#include <kis_canvas_resource_provider.h>
#include <kis_paintop_preset.h>

#include <KoCanvasResourceProvider.h>
#include <KoColorSpace.h>
#include <KoShapeManager.h>
#include <KoToolProxy.h>
#include <KoViewConverter.h>


#include <QImage>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QRectF>
#include <QRegion>
#include <QResizeEvent>
#include <QTabletEvent>
#include <QUndoCommand>

MixerCanvas::MixerCanvas(QWidget *parent)
    : QFrame(parent), KoCanvasBase(0), m_toolProxy(0), m_dirty(false)
{
    m_image = QImage(size(), QImage::Format_ARGB32);
    m_image.fill(0);
}

MixerCanvas::~MixerCanvas()
{
    if (m_toolProxy)
        delete m_toolProxy;
}

KisPaintLayer *MixerCanvas::layer()
{
    return m_layer.data();
}

KisPaintDevice *MixerCanvas::device()
{
    return m_layer->paintDevice().data();
}

KisPainterlyOverlay *MixerCanvas::overlay()
{
    return m_layer->paintDevice()->painterlyOverlay().data();
}

void MixerCanvas::setToolProxy(KoToolProxy *proxy)
{
    m_toolProxy = proxy;
}

void MixerCanvas::addCommand(QUndoCommand *command)
{
    delete command;
}

KoUnit MixerCanvas::unit() const
{
    Q_ASSERT(false);
    return KoUnit();
}

void MixerCanvas::setResources(KisCanvasResourceProvider *rp)
{
    KoCanvasResourceProvider *internal = resourceProvider();

    internal->setResource(KoCanvasResource::ForegroundColor, rp->fgColor());
    internal->setResource(KoCanvasResource::BackgroundColor, rp->bgColor());
    internal->setResource(KisCanvasResourceProvider::CurrentPattern, rp->currentPattern());
    internal->setResource(KisCanvasResourceProvider::CurrentGradient, rp->currentGradient());

    internal->setResource(KisCanvasResourceProvider::CurrentKritaNode, rp->currentNode().data());
    internal->setResource(KisCanvasResourceProvider::HdrExposure, rp->HDRExposure());
    QVariant v;
    v.setValue(rp->currentPreset());
    internal->setResource( KisCanvasResourceProvider::CurrentPaintOpPreset, v );

    checkCurrentPaintop();

    connect( rp->canvas()->resourceProvider(), SIGNAL(resourceChanged(int, const QVariant &)),
             this, SLOT(slotResourceChanged(int, const QVariant &)));

    // By now, both properties have been set
    connect( rp, SIGNAL(sigPaintOpPresetChanged( KisPaintOpPresetSP )),
             this, SLOT(checkCurrentPaintop()));
}

void MixerCanvas::checkCurrentPaintop()
{
#if 0
    KoCanvasResourceProvider *internal = resourceProvider();

    KisPainter painter(device());
    KisPaintOp *current = KisPaintOpRegistry::instance()->paintOp(
                          internal->resource(KisCanvasResourceProvider::CurrentPaintop).value<KoID>().id(),
                          settings, &painter, 0);
    painter.setPaintOp(current); // This is done just for the painter to own the paintop and destroy it

    if (current && !current->painterly())
        internal->setResource(KisCanvasResourceProvider::CurrentPaintop, KoID("paintcomplex", ""));
#endif
}

void MixerCanvas::checkCurrentLayer()
{
    KoCanvasResourceProvider *internal = resourceProvider();

    KisNodeSP current = internal->resource(KisCanvasResourceProvider::CurrentKritaNode).value<KisNodeSP>();
    if (current.data() != m_layer.data()) {
        QVariant v;
        v.setValue(KisNodeSP(m_layer.data()));
        internal->setResource(KisCanvasResourceProvider::CurrentKritaNode, v);
    }
}

void MixerCanvas::setLayer(const KoColorSpace *cs)
{
    Q_ASSERT(cs);
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
#if 0 // Apparently setting the paintop and settings is not atomic: it will be when I have fixed

    if (key != KisCanvasResourceProvider::CurrentPaintopSettings)
        resourceProvider()->setResource(key, value);
    else
        return;
    switch (key) {

      // the presets thing (boud)
        case KisCanvasResourceProvider::CurrentPaintop:
            checkCurrentPaintop();
            break;

        case KisCanvasResourceProvider::CurrentKritaNode:
            checkCurrentLayer();
            break;
    }
#endif
}

#include "mixercanvas.moc"
