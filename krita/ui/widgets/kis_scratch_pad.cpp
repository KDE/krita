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

#include <kis_config.h>
#include <kis_color_picker_utils.h>
#include <kis_vec.h>
#include <kis_cursor.h>
#include <kis_paint_device.h>
#include <kis_gradient_painter.h>
#include <kis_paintop_settings.h>
#include <kis_default_bounds.h>
#include <kis_dumb_undo_adapter.h>
#include <kis_canvas_resource_provider.h>

class KisScratchPadDefaultBounds : public KisDefaultBounds
{
public:
    QRect bounds() const
    {
        return QRect(0,0,0,0);
    }
};


KisScratchPad::KisScratchPad(QWidget *parent)
    : QWidget(parent)
    , m_colorSpace(0)
    , m_backgroundColor(Qt::white, KoColorSpaceRegistry::instance()->rgb8())
    , m_canvasColor(Qt::white)
    , m_toolMode(HOVERING)
    , m_paintDevice(0)
    , m_paintLayer(0)
    , m_backgroundMode(SOLID_COLOR)
    , m_displayProfile(0)
    , m_painter(0)
    , m_compositeOp(0)
    , m_scale(1.0)
    , m_opacity(OPACITY_OPAQUE_U8)
    , m_resourceProvider(0)
{
    setAutoFillBackground(false);

    m_cursor = KisCursor::load("tool_freehand_cursor.png", 5, 5);

    KisConfig cfg;
    int checkSize = cfg.checkSize();
    QImage tile(checkSize * 2, checkSize * 2, QImage::Format_RGB32);
    QPainter pt(&tile);
    pt.fillRect(tile.rect(), Qt::white);
    pt.fillRect(0, 0, checkSize, checkSize, cfg.checkersColor());
    pt.fillRect(checkSize, checkSize, checkSize, checkSize, cfg.checkersColor());
    pt.end();
    m_checkBrush = QBrush(tile);
}

KisScratchPad::~KisScratchPad() {
    delete m_painter;
}


void KisScratchPad::setCutoutOverlay(const QRect& rc)  {

    m_cutoutOverlay = rc;
}

QImage KisScratchPad::cutoutOverlay() const {

    QRect rc = m_cutoutOverlay.translated(m_offset);
    QImage img = m_paintDevice->convertToQImage(0, rc.x(), rc.y(), rc.width(), rc.height());
    return img;
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

void KisScratchPad::setBackgroundColor(const KoColor& backgroundColor) {

    m_backgroundColor = backgroundColor;
}

void KisScratchPad::setCanvasColor(const QColor& canvasColor)
{
    m_canvasColor = canvasColor;
    clear();
}

void KisScratchPad::setBackgroundTile(const QImage& tile) {

    m_backgroundTile = tile;
}

void KisScratchPad::setColorSpace(const KoColorSpace *colorSpace) {

    m_colorSpace = colorSpace;
    m_paintDevice = new KisPaintDevice(colorSpace, "scratchpad");
    m_paintLayer = new KisPaintLayer(0, "ScratchPad", OPACITY_OPAQUE_U8, m_paintDevice);
    m_paintDevice->setDefaultBounds(new KisScratchPadDefaultBounds());

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
            c.fromQColor(m_canvasColor);
            m_paintDevice->setDefaultPixel(c.data());
            m_paintLayer->updateProjection(rect().translated(m_offset));
        }
    }
    update();
}

void KisScratchPad::fillGradient(KoAbstractGradient* gradient)
{
    if (!m_paintDevice) return;
    KisGradientPainter painter(m_paintDevice);
    painter.setGradient(gradient);
    painter.paintGradient(QPointF(0,0)+m_offset, QPointF(width(), height())+m_offset,
                          KisGradientPainter::GradientShapeLinear, KisGradientPainter::GradientRepeatNone,
                          0.2, false,
                          m_offset.x(), m_offset.y(), width(), height());

    m_paintLayer->updateProjection(rect().translated(m_offset));
    update();
}

void KisScratchPad::fillSolid(const KoColor& color)
{
    if (!m_paintDevice) return;
    KoColor csColor(color, m_paintDevice->colorSpace());
    m_paintDevice->fill(m_offset.x(), m_offset.y(), width(), height(), csColor.data());
    m_paintDevice->setDirty(QRect(0, 0, width(), height()).translated(m_offset));
    m_paintLayer->updateProjection(rect().translated(m_offset));
    update();
}

void KisScratchPad::setPresetImage(const QImage& image)
{
    KisPaintDeviceSP device = new KisPaintDevice(m_paintDevice->colorSpace());
    device->convertFromQImage(image, "");
    KisPainter painter(m_paintDevice);
    painter.bitBlt(m_cutoutOverlay.x(), m_cutoutOverlay.y(), device, 0, 0, m_cutoutOverlay.width(), m_cutoutOverlay.height());
    update();
}

void KisScratchPad::setCanvasResourceProvider(KisCanvasResourceProvider* resourceProvider)
{
    m_resourceProvider = resourceProvider;
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
    gc.fillRect(rc, m_checkBrush);
    KisPaintDeviceSP projection = m_paintLayer->projection();
    gc.drawImage(rc, projection->convertToQImage(m_displayProfile,
                                                    rc.x() + m_offset.x(),
                                                    rc.y() + m_offset.y(),
                                                    rc.width(),
                                                    rc.height()));
    QBrush brush(Qt::lightGray);
    QPen pen(brush, 1, Qt::DotLine);
    gc.setPen(pen);
    if (m_cutoutOverlay.isValid()) {
        gc.drawRect(m_cutoutOverlay);
    }
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
    Q_ASSERT(m_resourceProvider);
    if (currentPaintOpPreset() && currentPaintOpPreset()->settings()) {
        m_paintIncremental = currentPaintOpPreset()->settings()->paintIncremental();
        /// todo: create a KoPointerEvent and use it here
        /// will be done probably, when when using a common class with kistoolfreehand
//        currentPaintOpPreset()->settings()->mousePressEvent(e);
//        if (e->isAccepted()) {
//            return;
//        }
    }
    m_toolMode = PAINTING;
    m_dragDist = 0;

    KisPaintDeviceSP paintDevice = currentNode()->paintDevice();
    KisPaintDeviceSP targetDevice;

    if (!m_compositeOp)
        m_compositeOp = paintDevice->colorSpace()->compositeOp(COMPOSITE_OVER);


    if (!m_paintIncremental) {
        KisIndirectPaintingSupport* indirect =
            dynamic_cast<KisIndirectPaintingSupport*>(currentNode().data());

        if (indirect) {
            targetDevice = new KisPaintDevice(currentNode().data(), paintDevice->colorSpace());
//            targetDevice->setDefaultBounds(KisDefaultBounds(QRect(0,0,1,1)));
            indirect->setTemporaryTarget(targetDevice);
            indirect->setTemporaryCompositeOp(m_compositeOp);
            indirect->setTemporaryOpacity(m_opacity);
        }
        else {
            m_paintIncremental = true;
        }
    }

    if (!targetDevice)
        targetDevice = paintDevice;

    delete m_painter;

    m_painter = new KisPainter(targetDevice);
    m_painter->beginTransaction("scratchpad stroke");

    if (m_paintIncremental) {
        m_painter->setCompositeOp(m_compositeOp);
        m_painter->setOpacity(m_opacity);
    } else {
        m_painter->setCompositeOp(paintDevice->colorSpace()->compositeOp(COMPOSITE_ALPHA_DARKEN));
        m_painter->setOpacity(OPACITY_OPAQUE_U8);
    }

    m_painter->setPaintColor(m_paintColor);
    m_painter->setBackgroundColor(m_backgroundColor);
    m_painter->setGradient(m_resourceProvider->currentGradient());
    m_painter->setPattern(m_resourceProvider->currentPattern());
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
    m_distanceInformation.spacing = m_painter->paintAt(m_previousPaintInformation);
    m_distanceInformation.distance = 0.0;


    QRect bounds;
    foreach(const QRect &rc, m_painter->takeDirtyRegion()) {
        bounds |= rc;
    }

    update(pos.x() - bounds.width(), pos.y() - bounds.height(), bounds.width() * 2, bounds.height() *2);
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

    m_distanceInformation = m_painter->paintLine(m_previousPaintInformation, info, m_distanceInformation);
    m_previousPaintInformation = info;

    QRect bounds;
    foreach(const QRect &rc, m_painter->takeDirtyRegion()) {
        bounds |= rc;
        m_incrementalDirtyRegion += rc;
    }

    m_paintLayer->updateProjection(bounds);
    update(pos.x() - bounds.width(), pos.y() - bounds.height(), bounds.width() * 2, bounds.height() *2);
}

void KisScratchPad::endPaint(QEvent *event) {

    Q_UNUSED(event);
    m_toolMode = HOVERING;

    if (m_painter) {
        // If painting in mouse release, make sure painter
        // is destructed or end()ed

        // XXX: For now, only layers can be painted on in non-incremental mode
        KisLayerSP layer = dynamic_cast<KisLayer*>(currentNode().data());

        if (layer && !m_paintIncremental) {
            m_painter->deleteTransaction();

            KisIndirectPaintingSupport *indirect =
                dynamic_cast<KisIndirectPaintingSupport*>(layer.data());
            Q_ASSERT(indirect);

            indirect->mergeToLayer(layer, m_incrementalDirtyRegion, QString("scratchpaint"));
            m_incrementalDirtyRegion = QRegion();
        } else {
            KisDumbUndoAdapter dua;
            m_painter->endTransaction(&dua);
        }
    }


    QRect bounds;
    foreach(const QRect &rc, m_painter->takeDirtyRegion()) {
        bounds |= rc;
    }

    update(bounds.translated(m_currentMousePosition));

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
