/* This file is part of the KDE project
 * Copyright 2010 (C) Boudewijn Rempt <boud@valdyas.org> *
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
#include "kis_scratch_pad.h"

#include <QRect>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QTabletEvent>

#include <KoColorSpace.h>
#include <KoColorProfile.h>
#include <KoColorSpaceRegistry.h>

#include <kis_vec.h>
#include <kis_cursor.h>
#include <kis_paint_device.h>

KisScratchPad::KisScratchPad(QWidget *parent)
    : QWidget(parent)
    , m_colorSpace(0)
    , m_backgroundColor(Qt::white)
    , m_toolMode(HOVERING)
    , m_backgroundMode(SOLID_COLOR)
    , m_displayProfile(0)
    , m_painter(0)
    , m_compositeOp(0)
{
    m_cursor = KisCursor::load("tool_freehand_cursor.png", 5, 5);
    setCursor(m_cursor);
}

KisScratchPad::~KisScratchPad() {

    delete m_painter;
}

void KisScratchPad::setPaintColor(const QColor& paintColor) {

    KoColor c(m_colorSpace);
    c.fromQColor(paintColor);
    m_paintColor = c;
}

void KisScratchPad::setPaintColor(KoColor& paintColor) {

    paintColor.convertTo(m_colorSpace);
    m_paintColor = paintColor;
}

void KisScratchPad::setPreset(KisPaintOpPresetSP preset) {

    qDebug() << "setPreset" << preset->name();
    m_preset = preset;
}

void KisScratchPad::setBackgroundColor(const QColor& backgroundColor) {

    m_backgroundColor = backgroundColor;
}

void KisScratchPad::setBackgroundTile(const QImage& tile) {

    m_backgroundTile = tile;
}

void KisScratchPad::setColorSpace(const KoColorSpace *colorSpace) {

    qDebug() << "setColorSpace();" << colorSpace->name();

    m_colorSpace = colorSpace;
    m_paintDevice = new KisPaintDevice(colorSpace, "ScratchPad");
    m_compositeOp = m_colorSpace->compositeOp(COMPOSITE_OVER);
    clear();
}

void KisScratchPad::setDisplayProfile(const KoColorProfile *colorProfile) {

    m_displayProfile = colorProfile;
    update();
}

void KisScratchPad::clear() {

    qDebug() << "clear";

    if (m_paintDevice) {
        m_paintDevice->clear();
        switch(m_backgroundMode) {
        case TILED:
        case STRETCHED:
        case CENTERED:
        case GRADIENT:
        case SOLID_COLOR:
        default:
            KoColor c(m_paintDevice->colorSpace());
            c.fromQColor(m_backgroundColor);
            m_paintDevice->setDefaultPixel(c.data());
        }
    }
    update();
}

void KisScratchPad::contextMenuEvent ( QContextMenuEvent * event ) {

    qDebug() << "contextMenuEvent()";
    QWidget::contextMenuEvent(event);
}

void KisScratchPad::keyPressEvent ( QKeyEvent * event ) {

    qDebug() << "keyPressEvent();" << event->key();
    QWidget::keyPressEvent(event);
}

void KisScratchPad::keyReleaseEvent ( QKeyEvent * event ) {

    qDebug() << "keyReleaseEvent();" << event->key();
    QWidget::keyReleaseEvent(event);
}

void KisScratchPad::mouseDoubleClickEvent ( QMouseEvent * event ) {

    qDebug() << "mouseDoubleClickEvent();" << event->pos() << event->button();
    QWidget::mouseDoubleClickEvent(event);
}

void KisScratchPad::mouseMoveEvent ( QMouseEvent * event ) {

    qDebug() << "mouseMoveEvent();" << event->pos() << event->button();
    if (!m_paintDevice) return;

    m_currentMousePosition = event->pos();
    if (event->button() == Qt::LeftButton) {
        paint(event);
    }
    else if (event->button() == Qt::MidButton) {
        // start panning
        m_toolMode = PANNING;
    }
    else if (event->button() == Qt::RightButton) {
        // start picking
        m_toolMode = PICKING;
    }
}

void KisScratchPad::mousePressEvent ( QMouseEvent * event ) {

    qDebug() << "mousePressEvent();" << event->pos() << event->button();
    if (!m_paintDevice) return;

    m_currentMousePosition = event->pos();
    if (event->button() == Qt::LeftButton) {
        initPainting(event);
    }
    else if (event->button() == Qt::MidButton) {
        // start panning
        m_toolMode = PANNING;
    }
    else if (event->button() == Qt::RightButton) {
        // start picking
        m_toolMode = PICKING;
    }


}

void KisScratchPad::mouseReleaseEvent ( QMouseEvent * event ) {

    qDebug() << "mouseReleaseEvent();" << event->pos() << event->button();
    QWidget::mouseReleaseEvent(event);
}

void KisScratchPad::paintEvent ( QPaintEvent * event ) {

    qDebug() << "paintEvent()" << event->rect();
    if (m_colorSpace == 0 || m_paintDevice == 0) {
        return;
    }
    QRect rc = event->rect();
    QPainter gc(this);
    gc.drawImage(rc, m_paintDevice->convertToQImage(m_displayProfile,
                                                    rc.x() + m_offset.x(),
                                                    rc.y() + m_offset.y(),
                                                    rc.width(),
                                                    rc.height()));
    gc.end();
}

void KisScratchPad::resizeEvent ( QResizeEvent * event ) {

    qDebug() << "resizeEvent";

    QWidget::resizeEvent(event);
}

void KisScratchPad::tabletEvent ( QTabletEvent * event ) {

    if (event->type() == QEvent::TabletPress) {
        initPainting(event);
    }
    else if (event->type() == QEvent::TabletMove) {
        paint(event);
    }
    else {
        endPaint(event);
    }

}

void KisScratchPad::wheelEvent ( QWheelEvent * event ) {

    qDebug() << "wheelEvent";
    QWidget::wheelEvent(event);
}

void KisScratchPad::initPainting(QEvent* event) {

    qDebug() << "start painting";
    m_toolMode = PAINTING;
    m_dragDist = 0;
    if (m_painter) delete m_painter;
    m_painter = new KisPainter(m_paintDevice);
    m_painter->setCompositeOp(m_compositeOp);
    m_painter->setPaintColor(m_paintColor);
    m_painter->setPaintOpPreset(m_preset, 0);
    if (QTabletEvent* tabletEvent = dynamic_cast<QTabletEvent*>(event)) {
        const QPointF pos = tabletEvent->hiResGlobalPos() - mapToGlobal(QPoint(0, 0));
        m_previousPaintInformation = KisPaintInformation(pos,
                                                         tabletEvent->pressure(),
                                                         tabletEvent->xTilt(),
                                                         tabletEvent->yTilt(),
                                                         KisVector2D::Zero(),
                                                         tabletEvent->rotation(),
                                                         tabletEvent->tangentialPressure());
    }
    else if (QMouseEvent* mouseEvent = dynamic_cast<QMouseEvent*>(event)) {
        m_previousPaintInformation = KisPaintInformation(mouseEvent->posF());
    }
    m_painter->paintAt(m_previousPaintInformation);
    update(m_painter->dirtyRegion());
}

void KisScratchPad::paint(QEvent* event) {

    KisPaintInformation info;
    if (QTabletEvent* tabletEvent = dynamic_cast<QTabletEvent*>(event)) {
        const QPointF pos = tabletEvent->hiResGlobalPos() - mapToGlobal(QPoint(0, 0));
        QPointF dragVec = pos - m_previousPaintInformation.pos();
        info = KisPaintInformation(pos,
                                   tabletEvent->pressure(),
                                   tabletEvent->xTilt(),
                                   tabletEvent->yTilt(),
                                   toKisVector2D(dragVec),
                                   tabletEvent->rotation(),
                                   tabletEvent->tangentialPressure());

    }
    else if (QMouseEvent* mouseEvent = dynamic_cast<QMouseEvent*>(event)) {
        info = KisPaintInformation(mouseEvent->posF());
    }

    m_painter->paintLine(m_previousPaintInformation, info);
    m_previousPaintInformation = info;
    update(m_painter->dirtyRegion());

}

void KisScratchPad::endPaint(QEvent *event) {

    qDebug() << "endPaint";
}
