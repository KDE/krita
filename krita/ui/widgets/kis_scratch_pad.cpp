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
#include <KoAbstractGradient.h>

#include <kis_color_picker_utils.h>
#include <kis_vec.h>
#include <kis_cursor.h>
#include <kis_paint_device.h>
#include <kis_gradient_painter.h>

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
}

KisScratchPad::~KisScratchPad() {

    delete m_painter;
}

void KisScratchPad::setPaintColor(const QColor& paintColor) {

    KoColor c(m_colorSpace);
    c.fromQColor(paintColor);
    m_paintColor = c;
}

void KisScratchPad::setPaintColor(const KoColor& paintColor) {

    m_paintColor = paintColor;
    m_paintColor.convertTo(m_colorSpace);
}

void KisScratchPad::setPreset(KisPaintOpPresetSP preset) {

    m_preset = preset;
}

void KisScratchPad::setBackgroundColor(const QColor& backgroundColor) {

    m_backgroundColor = backgroundColor;
}

void KisScratchPad::setBackgroundTile(const QImage& tile) {

    m_backgroundTile = tile;
}

void KisScratchPad::setColorSpace(const KoColorSpace *colorSpace) {

    m_colorSpace = colorSpace;
    m_paintDevice = new KisPaintDevice(colorSpace, "ScratchPad");
    m_compositeOp = m_colorSpace->compositeOp(COMPOSITE_OVER);
    clear();
}

void KisScratchPad::setDisplayProfile(const KoColorProfile *colorProfile) {

    m_displayProfile = colorProfile;
    QWidget::update();
}

void KisScratchPad::clear() {

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

void KisScratchPad::fillGradient(KoAbstractGradient* gradient)
{
    KisGradientPainter painter(m_paintDevice);
    painter.setGradient(gradient);
    painter.paintGradient(QPointF(0,0), QPointF(width(), height()),
                          KisGradientPainter::GradientShapeLinear, KisGradientPainter::GradientRepeatNone,
                          0.2, false,
                          0, 0, width(), height());
    update();                         
}

void KisScratchPad::fillSolid(const KoColor& color)
{
    m_paintDevice->fill(0, 0, width(), height(), color.data());
    update();
}

void KisScratchPad::contextMenuEvent ( QContextMenuEvent * event ) {

    QWidget::contextMenuEvent(event);
}

void KisScratchPad::keyPressEvent ( QKeyEvent * event ) {

    QWidget::keyPressEvent(event);
}

void KisScratchPad::keyReleaseEvent ( QKeyEvent * event ) {

    QWidget::keyReleaseEvent(event);
}

void KisScratchPad::mouseDoubleClickEvent ( QMouseEvent * event ) {

    QWidget::mouseDoubleClickEvent(event);
}

void KisScratchPad::mouseMoveEvent ( QMouseEvent * event ) {

    if (!m_paintDevice) return;

    m_currentMousePosition = event->pos();

    switch (m_toolMode) {
    case PAINTING:
        paint(event);
        break;
    case PANNING:
        pan(event);
        break;
    case PICKING:
        pick(event);
        break;
    case HOVERING:
    default:
        event->ignore();
    }
}

void KisScratchPad::mousePressEvent ( QMouseEvent * event ) {

    if (!m_paintDevice) return;

    m_currentMousePosition = event->pos();
    if (event->button() == Qt::LeftButton) {
        initPainting(event);
        event->accept();
        return;
    }
    else if (event->button() == Qt::MidButton) {
        // start panning
        m_toolMode = PANNING;
        initPan(event);
        event->accept();
        return;
    }
    else if (event->button() == Qt::RightButton) {
        // start picking
        m_toolMode = PICKING;
    }


}

void KisScratchPad::mouseReleaseEvent ( QMouseEvent * event ) {

    if (m_toolMode == PAINTING) {
        endPaint(event);
    }
    else if (m_toolMode == PANNING) {
        endPan(event);
    }
    QWidget::mouseReleaseEvent(event);
}

void KisScratchPad::paintEvent ( QPaintEvent * event ) {

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

    QWidget::resizeEvent(event);
}

void KisScratchPad::tabletEvent ( QTabletEvent * event ) {

    if (!m_paintDevice) return;

    if (event->type() == QEvent::TabletPress) {
        initPainting(event);
    }
    else if (event->type() == QEvent::TabletMove && m_toolMode == PAINTING) {
        paint(event);
    }
    else if (event->type() == QEvent::TabletRelease && m_toolMode == PAINTING) {
        endPaint(event);
    }

}

void KisScratchPad::wheelEvent ( QWheelEvent * event ) {

    QWidget::wheelEvent(event);
}

void KisScratchPad::initPainting(QEvent* event) {

    m_toolMode = PAINTING;
    m_dragDist = 0;
    if (m_painter) delete m_painter;
    m_painter = new KisPainter(m_paintDevice);
    m_painter->setCompositeOp(m_compositeOp);
    m_painter->setPaintColor(m_paintColor);
    m_painter->setPaintOpPreset(m_preset, 0);
    QPointF pos;
    if (QTabletEvent* tabletEvent = dynamic_cast<QTabletEvent*>(event)) {
        pos = tabletEvent->hiResGlobalPos() - mapToGlobal(QPoint(0, 0));
        m_previousPaintInformation = KisPaintInformation(QPointF(pos.x() + m_offset.x(), pos.y() + m_offset.y()),
                                                         tabletEvent->pressure(),
                                                         tabletEvent->xTilt(),
                                                         tabletEvent->yTilt(),
                                                         KisVector2D::Zero(),
                                                         tabletEvent->rotation(),
                                                         tabletEvent->tangentialPressure());
    }
    else if (QMouseEvent* mouseEvent = dynamic_cast<QMouseEvent*>(event)) {
        pos = mouseEvent->pos();
        m_previousPaintInformation = KisPaintInformation(pos + m_offset);
    }
    m_painter->paintAt(m_previousPaintInformation);
    QRect rc = m_painter->dirtyRegion().boundingRect();

    update(pos.x() - rc.width(), pos.y() - rc.height(), rc.width() * 2, rc.height() *2);
}

void KisScratchPad::paint(QEvent* event) {

    if (!m_painter) {
        return;
    }

    KisPaintInformation info;
    QPointF pos;
    if (QTabletEvent* tabletEvent = dynamic_cast<QTabletEvent*>(event)) {
        pos = tabletEvent->hiResGlobalPos() - mapToGlobal(QPoint(0, 0));
        QPointF dragVec = pos - m_previousPaintInformation.pos();
        info = KisPaintInformation(QPointF(pos.x() + m_offset.x(), pos.y() + m_offset.y()),
                                   tabletEvent->pressure(),
                                   tabletEvent->xTilt(),
                                   tabletEvent->yTilt(),
                                   toKisVector2D(dragVec),
                                   tabletEvent->rotation(),
                                   tabletEvent->tangentialPressure());

    }
    else if (QMouseEvent* mouseEvent = dynamic_cast<QMouseEvent*>(event)) {
        pos = mouseEvent->pos();
        info = KisPaintInformation(pos + m_offset);
    }

    m_painter->paintLine(m_previousPaintInformation, info);
    m_previousPaintInformation = info;
    QRect rc = m_painter->dirtyRegion().boundingRect();

    update(pos.x() - rc.width(), pos.y() - rc.height(), rc.width() * 2, rc.height() *2);
}

void KisScratchPad::endPaint(QEvent *event) {

    Q_UNUSED(event);
    m_toolMode = HOVERING;

    QRect rc = m_painter->dirtyRegion().boundingRect();
    update(rc.translated(m_currentMousePosition));

    delete m_painter;
    m_painter = 0;
}

void KisScratchPad::pick(QMouseEvent* event) {

    emit colorSelected(KisToolUtils::pick(m_paintDevice, event->pos()));
}

void KisScratchPad::initPan(QMouseEvent* event) {

    m_toolMode = PANNING;
    m_lastPosition = event->pos();
    setCursor(QCursor(Qt::ClosedHandCursor));
    event->accept();
}

void KisScratchPad::pan(QMouseEvent* event) {

    QPoint actualPosition = event->pos();
    QPoint distance = m_lastPosition - actualPosition;

    m_offset += distance;
    QWidget::update();

    m_lastPosition = actualPosition;
    event->accept();
}

void KisScratchPad::endPan(QMouseEvent* event) {

    m_toolMode = HOVERING;
    setCursor(m_cursor);
    event->ignore();
}
