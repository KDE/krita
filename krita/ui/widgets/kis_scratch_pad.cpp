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

#include <KoColorSpace.h>
#include <KoColorProfile.h>
#include <KoColorSpaceRegistry.h>

#include <kis_cursor.h>
#include <kis_paint_device.h>

KisScratchPad::KisScratchPad(QWidget *parent)
    : QWidget(parent)
    , m_colorSpace(0)
    , m_backgroundColor(Qt::white)
    , m_toolMode(HOVERING)
    , m_backgroundMode(SOLID_COLOR)
    , m_displayProfile(0)
{
    m_cursor = KisCursor::load("tool_freehand_cursor.png", 5, 5);
}

KisScratchPad::~KisScratchPad() {

}

void KisScratchPad::setPaintColor(const KoColor& foregroundColor) {

    m_foregroundColor = foregroundColor;
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
    QWidget::mouseMoveEvent(event);
}

void KisScratchPad::mousePressEvent ( QMouseEvent * event ) {

    qDebug() << "mousePressEvent();" << event->pos() << event->button();
    QWidget::mousePressEvent(event);
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

    qDebug() << "tabletEvent";
    QWidget::tabletEvent(event);
}

void KisScratchPad::wheelEvent ( QWheelEvent * event ) {

    qDebug() << "wheelEvent";
    QWidget::wheelEvent(event);
}
