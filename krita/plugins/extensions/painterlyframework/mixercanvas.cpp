/* This file is part of the KDE project
 *
 * Copyright (C) 2007 Emanuele Tamponi <emanuele@valinor.it>
 * Copyright (C) 2009 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include "mixercanvas.h"

#include <QImage>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QRectF>
#include <QRegion>
#include <QResizeEvent>
#include <QTabletEvent>
#include <QUndoCommand>

#include <KoResourceManager.h>
#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoShapeManager.h>
#include <KoViewConverter.h>
#include <KoPointerEvent.h>

#include <kis_image.h>
#include <kis_paint_device.h>
#include <kis_painter.h>
#include <kis_paint_device.h>
#include <kis_paintop_registry.h>
#include <kis_canvas_resource_provider.h>
#include <kis_paintop_preset.h>
#include <kis_config.h>

#include "mixertool.h"

MixerCanvas::MixerCanvas(QWidget *parent)
        : QFrame(parent)
        , KoCanvasBase(0)
        , m_viewConverter(new KoViewConverter())
        , m_dirty(false)
{
    m_viewConverter->setZoom(1.0);

    m_image = QImage(size(), QImage::Format_ARGB32);
    m_image.fill(0);

    KisConfig config;
    const KoColorSpace* cs = KoColorSpaceRegistry::instance()->colorSpace(config.defaultPainterlyColorModelId(), config.defaultPainterlyColorDepthId(), "");
    if (!cs) {
        dbgPlugins << "Could not load painterly colorspace, falling back to rgb";
        cs = KoColorSpaceRegistry::instance()->rgb16();
    }
    m_paintDevice = new KisPaintDevice(cs, "Mixer");
    m_mixerTool = new MixerTool(this);

}

MixerCanvas::~MixerCanvas()
{
    delete m_mixerTool;
}

KisPaintDevice *MixerCanvas::device()
{
    return m_paintDevice;
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

const KoColorSpace* MixerCanvas::colorSpace()
{
    return m_paintDevice->colorSpace();
}

void MixerCanvas::mouseDoubleClickEvent(QMouseEvent *event)
{
    QPointF point = m_viewConverter->viewToDocument(event->posF());
    KoPointerEvent pointerEvent(event, point);
    m_mixerTool->mouseDoubleClickEvent(&pointerEvent);
}

void MixerCanvas::mouseMoveEvent(QMouseEvent *event)
{
    QPointF point = m_viewConverter->viewToDocument(event->posF());
    KoPointerEvent pointerEvent(event, point);
    m_mixerTool->mouseMoveEvent(&pointerEvent);
}

void MixerCanvas::mousePressEvent(QMouseEvent *event)
{
    QPointF point = m_viewConverter->viewToDocument(event->posF());
    KoPointerEvent pointerEvent(event, point);
    m_mixerTool->mousePressEvent(&pointerEvent);
}

void MixerCanvas::mouseReleaseEvent(QMouseEvent *event)
{
    QPointF point = m_viewConverter->viewToDocument(event->posF());
    KoPointerEvent pointerEvent(event, point);
    m_mixerTool->mouseReleaseEvent(&pointerEvent);
}

void MixerCanvas::tabletEvent(QTabletEvent *event)
{
    QPointF point = m_viewConverter->viewToDocument(event->pos());
    KoPointerEvent pointerEvent(event, point);

    KoPointerEvent ev(event, point);
    switch (event->type()) {
    case QEvent::TabletPress:
        m_mixerTool->mousePressEvent(&pointerEvent);
        break;
    case QEvent::TabletRelease:
        m_mixerTool->mouseReleaseEvent(&ev);
        break;
    case QEvent::TabletMove:
        m_mixerTool->mouseMoveEvent(&ev);
    default:
        ; // ignore the rest.
    }

}

void MixerCanvas::resizeEvent(QResizeEvent *event)
{
    if (event->size().width() > m_image.width() ||
            event->size().height() > m_image.height()) {
        QImage newImage(event->size(), QImage::Format_ARGB32);
        newImage.fill(0);

        QPainter p(&newImage);
        p.drawImage(m_image.rect(), m_image, m_image.rect());
        p.end();

        m_image = newImage;
    }

    QFrame::resizeEvent(event);
}

void MixerCanvas::paintEvent(QPaintEvent *event)
{
    if (m_dirty) {
        QRect r = event->rect();
        QRect imageRect = QRect(0, 0, r.width(), r.height());
        QPainter p(&m_image);
        p.drawImage(r, m_paintDevice->convertToQImage(0, r.x(), r.y(), r.width(), r.height()), imageRect);
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
    m_image.fill(0);
    update();
}

void MixerCanvas::slotResourceChanged(int key, const QVariant &value)
{
    Q_UNUSED(key);
    Q_UNUSED(value);
}

KoColor MixerCanvas::currentColorAt(QPoint pos)
{
    Q_UNUSED(pos);
    return KoColor();
}

#include "mixercanvas.moc"
