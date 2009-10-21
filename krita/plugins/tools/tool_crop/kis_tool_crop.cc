/*
 *  kis_tool_crop.cc -- part of Krita
 *
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2005 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *  Copyright (C) 2007 Adrian Page <adrian@pagenet.plus.com>
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

#include "kis_tool_crop.h"


#include <QCheckBox>
#include <QComboBox>
#include <QObject>
#include <QPainter>
#include <QPen>
#include <QPushButton>
#include <QRect>
#include <QVector>

#include <kis_debug.h>
#include <klocale.h>
#include <knuminput.h>

#include <KoCanvasBase.h>
#include <kis_global.h>
#include <kis_painter.h>
#include <kis_cursor.h>
#include <kis_image.h>
#include <kis_undo_adapter.h>
#include <KoPointerEvent.h>
#include <kis_selected_transaction.h>
#include <kis_selection.h>
#include <kis_layer.h>
#include <kis_crop_visitor.h>

KisToolCrop::KisToolCrop(KoCanvasBase * canvas)
        : KisTool(canvas, KisCursor::load("tool_crop_cursor.png", 6, 6))
{
    setObjectName("tool_crop");
    m_selecting = false;
    m_rectCrop = QRect(0, 0, 0, 0);
    m_handleSize = 13;
    m_haveCropSelection = false;
    m_optWidget = 0;
}

KisToolCrop::~KisToolCrop()
{
}

void KisToolCrop::activate(bool tmp)
{
    KisTool::activate(tmp);

    KisSelectionSP sel = currentSelection();
    if (sel) {
        sel->updateProjection();
        m_rectCrop = sel->selectedExactRect();
        validateSelection();
        updateCanvasPixelRect(image()->bounds());
    }
}

void KisToolCrop::deactivate()
{
    m_selecting = false;
    m_haveCropSelection = false;
    m_rectCrop = QRect(0, 0, 0, 0);
    updateWidgetValues();
    updateCanvasPixelRect(image()->bounds());
}

void KisToolCrop::paint(QPainter &painter, const KoViewConverter &converter)
{
    Q_UNUSED(converter);
    paintOutlineWithHandles(painter);
}

void KisToolCrop::mousePressEvent(KoPointerEvent *e)
{
    if (m_canvas) {

        if (!currentNode())
            return;

        if (currentImage() && currentNode()->paintDevice() && e->button() == Qt::LeftButton) {

            QPoint pos = convertToIntPixelCoord(e);

            pos.setX(qBound(0, pos.x(), image()->width() - 1));
            pos.setY(qBound(0, pos.y(), image()->height() - 1));

            m_selecting = true;

            if (!m_haveCropSelection) { //if the selection is not set
                m_rectCrop = QRect(pos.x(), pos.y(), 1, 1);
                updateCanvasPixelRect(image()->bounds());
            } else {
                m_mouseOnHandleType = mouseOnHandle(pixelToView(convertToPixelCoord(e)));
                m_dragStart = pos;
            }
        }
    }
}

void KisToolCrop::mouseMoveEvent(KoPointerEvent *e)
{
    if (m_canvas && currentImage()) {
        if (m_selecting) { //if the user selects
            QPoint pos = convertToIntPixelCoord(e);

            if (!m_haveCropSelection) { //if the cropSelection is not yet set
                QRectF updateRect = boundingRect();

                m_rectCrop.setBottomRight(convertToIntPixelCoord(e));

                m_rectCrop.setRight(qBound(0, m_rectCrop.right(), image()->width() - 1));
                m_rectCrop.setBottom(qBound(0, m_rectCrop.bottom(), image()->height() - 1));

                updateRect |= boundingRect();
                updateCanvasViewRect(updateRect);
            } else { //if the crop selection is set
                QPoint dragStop = convertToIntPixelCoord(e);
                QPoint drag = dragStop - m_dragStart;

                if (m_mouseOnHandleType != None && m_dragStart != dragStop) {

                    qint32 imageWidth = currentImage()->width();
                    qint32 imageHeight = currentImage()->height();

                    QRectF updateRect = boundingRect();

                    if (m_mouseOnHandleType == Inside) {
                        m_rectCrop.translate(drag);

                        if (m_rectCrop.left() < 0) {
                            m_rectCrop.moveLeft(0);
                        }
                        if (m_rectCrop.right() > imageWidth - 1) {
                            m_rectCrop.moveRight(imageWidth - 1);
                        }
                        if (m_rectCrop.top() < 0) {
                            m_rectCrop.moveTop(0);
                        }
                        if (m_rectCrop.bottom() > imageHeight - 1) {
                            m_rectCrop.moveBottom(imageHeight - 1);
                        }
                    } else if (m_optWidget->boolRatio->isChecked()) {
                        if (! m_optWidget->boolWidth->isChecked() && !m_optWidget->boolHeight->isChecked()) {
                            switch (m_mouseOnHandleType) {
                            case(UpperLeft): {
                                qint32 dep = (drag.x() + drag.y()) / 2;
                                m_rectCrop.setTop(m_rectCrop.top() + dep);
                                m_rectCrop.setLeft((int)(m_rectCrop.right() - m_optWidget->doubleRatio->value() * m_rectCrop.height()));
                            }
                            break;
                            case(LowerRight): {
                                qint32 dep = (drag.x() + drag.y()) / 2;
                                m_rectCrop.setBottom(m_rectCrop.bottom() + dep);
                                m_rectCrop.setWidth((int)(m_optWidget->doubleRatio->value() * m_rectCrop.height()));
                                break;
                            }
                            case(UpperRight): {
                                qint32 dep = (drag.x() - drag.y()) / 2;
                                m_rectCrop.setTop(m_rectCrop.top() - dep);
                                m_rectCrop.setWidth((int)(m_optWidget->doubleRatio->value() * m_rectCrop.height()));
                                break;
                            }
                            case(LowerLeft): {
                                qint32 dep = (drag.x() - drag.y()) / 2;
                                m_rectCrop.setBottom(m_rectCrop.bottom() - dep);
                                m_rectCrop.setLeft((int)(m_rectCrop.right() - m_optWidget->doubleRatio->value() * m_rectCrop.height()));
                                break;
                            }
                            case(Upper):
                                m_rectCrop.setTop(pos.y());
                                m_rectCrop.setWidth((int)(m_rectCrop.height() * m_optWidget->doubleRatio->value()));
                                break;
                            case(Lower):
                                m_rectCrop.setBottom(pos.y());
                                m_rectCrop.setWidth((int)(m_rectCrop.height() * m_optWidget->doubleRatio->value()));
                                break;
                            case(Left):
                                m_rectCrop.setLeft(pos.x());
                                m_rectCrop.setHeight((int)(m_rectCrop.width() / m_optWidget->doubleRatio->value()));
                                break;
                            case(Right):
                                m_rectCrop.setRight(pos.x());
                                m_rectCrop.setHeight((int)(m_rectCrop.width() / m_optWidget->doubleRatio->value()));
                                break;
                            case(Inside):  // never happen
                                break;
                            }
                        }
                    } else {
                        if (m_optWidget->boolWidth->isChecked()) {
                            m_rectCrop.setWidth(m_optWidget->intWidth->value());
                        } else {
                            switch (m_mouseOnHandleType) {
                            case(LowerLeft):
                            case(Left):
                            case(UpperLeft):
                                m_rectCrop.setLeft(pos.x());
                                break;
                            case(Right):
                            case(UpperRight):
                            case(LowerRight):
                                m_rectCrop.setRight(pos.x());
                                break;
                            default:
                                break;
                            }
                        }
                        if (m_optWidget->boolHeight->isChecked()) {
                            m_rectCrop.setHeight(m_optWidget->intHeight->value());
                        } else {
                            switch (m_mouseOnHandleType) {
                            case(UpperLeft):
                            case(Upper):
                            case(UpperRight):
                                m_rectCrop.setTop(pos.y());
                                break;
                            case(LowerRight):
                            case(LowerLeft):
                            case(Lower):
                                m_rectCrop.setBottom(pos.y());
                                break;
                            default:
                                break;
                            }
                        }
                    }

                    m_rectCrop.setLeft(qBound(0, m_rectCrop.left(), image()->width() - 1));
                    m_rectCrop.setRight(qBound(0, m_rectCrop.right(), image()->width() - 1));
                    m_rectCrop.setTop(qBound(0, m_rectCrop.top(), image()->height() - 1));
                    m_rectCrop.setBottom(qBound(0, m_rectCrop.bottom(), image()->height() - 1));

                    m_dragStart = dragStop;

                    updateRect |= boundingRect();
                    updateCanvasViewRect(updateRect);
                }
            }
            updateWidgetValues();
        } else { //if we are not selecting
            if (m_haveCropSelection) {  //if the crop selection is set
                QPointF pos = convertToPixelCoord(e);
                qint32 type = mouseOnHandle(pixelToView(pos));
                //set resize cursor if we are on one of the handles
                setMoveResizeCursor(type);
            }
        }
    }
}

void KisToolCrop::updateWidgetValues(bool updateratio)
{
    QRect r = m_rectCrop.normalized();
    if (!m_optWidget) createOptionWidget();

    setOptionWidgetX(r.x());
    setOptionWidgetY(r.y());
    setOptionWidgetWidth(r.width());
    setOptionWidgetHeight(r.height());
    if (updateratio && !m_optWidget->boolRatio->isChecked())
        setOptionWidgetRatio((double)r.width() / (double)r.height());
}

void KisToolCrop::mouseReleaseEvent(KoPointerEvent *e)
{
    if (m_canvas && currentImage() && m_selecting && e->button() == Qt::LeftButton) {

        m_selecting = false;
        m_haveCropSelection = true;
        m_rectCrop = m_rectCrop.normalized();

        QRectF updateRect = boundingRect();

        validateSelection();

        updateRect |= boundingRect();
        updateCanvasViewRect(updateRect);
    }
}

void KisToolCrop::mouseDoubleClickEvent(KoPointerEvent *)
{
    if (m_haveCropSelection) crop();
}

void KisToolCrop::validateSelection(bool updateratio)
{
    if (m_canvas && image()) {
        m_rectCrop.setLeft(qBound(0, m_rectCrop.left(), image()->width() - 1));
        m_rectCrop.setRight(qBound(0, m_rectCrop.right(), image()->width() - 1));
        m_rectCrop.setTop(qBound(0, m_rectCrop.top(), image()->height() - 1));
        m_rectCrop.setBottom(qBound(0, m_rectCrop.bottom(), image()->height() - 1));

        updateWidgetValues(updateratio);
    }
}

#define FADE_AREA_OUTSIDE_CROP 1

#if FADE_AREA_OUTSIDE_CROP
#define BORDER_LINE_WIDTH 0
#define HALF_BORDER_LINE_WIDTH 0
#else
#define BORDER_LINE_WIDTH 1
#define HALF_BORDER_LINE_WIDTH (BORDER_LINE_WIDTH / 2.0)
#endif

#define HANDLE_BORDER_LINE_WIDTH 1

QRectF KisToolCrop::borderLineRect()
{
    QRectF borderRect = pixelToView(m_rectCrop.normalized());

    // Draw the border line right next to the crop rectangle perimeter.
    borderRect.adjust(-HALF_BORDER_LINE_WIDTH, -HALF_BORDER_LINE_WIDTH, HALF_BORDER_LINE_WIDTH, HALF_BORDER_LINE_WIDTH);

    return borderRect;
}

#define OUTSIDE_CROP_ALPHA 200

void KisToolCrop::paintOutlineWithHandles(QPainter& gc)
{
    if (m_canvas && (m_selecting || m_haveCropSelection)) {
        gc.save();

#if FADE_AREA_OUTSIDE_CROP

        QRectF wholeImageRect = pixelToView(image()->bounds());
        QRectF borderRect = borderLineRect();

        QPainterPath path;

        path.addRect(wholeImageRect);
        path.addRect(borderRect);
        gc.setPen(Qt::NoPen);
        gc.setBrush(QColor(0, 0, 0, OUTSIDE_CROP_ALPHA));
        gc.drawPath(path);

        // Handles
        QPen pen(Qt::SolidLine);
        pen.setWidth(HANDLE_BORDER_LINE_WIDTH);
        pen.setColor(Qt::black);
        gc.setPen(pen);
        gc.setBrush(Qt::yellow);
        gc.drawPath(handlesPath());
#else
        QPen pen(Qt::SolidLine);
        pen.setWidth(BORDER_LINE_WIDTH);
        gc.setPen(pen);

        QRectF borderRect = borderLineRect();

        //  Border
        gc.setBrush(Qt::NoBrush);
        gc.drawRect(borderRect);

        // Handles
        gc.setBrush(Qt::black);
        gc.drawPath(handlesPath());

        //draw guides
        gc.drawLine(borderRect.bottomLeft(), QPointF(-m_canvas->canvasWidget()->width(), borderRect.bottom()));
        gc.drawLine(borderRect.bottomLeft(), QPointF(borderRect.left(), m_canvas->canvasWidget()->height()));
        gc.drawLine(borderRect.topRight(), QPointF(m_canvas->canvasWidget()->width(), borderRect.top()));
        gc.drawLine(borderRect.topRight(), QPointF(borderRect.right(), -m_canvas->canvasWidget()->height()));
#endif
        gc.restore();
    }
}

void KisToolCrop::crop()
{
    // XXX: Should cropping be part of KisImage/KisPaintDevice's API?

    m_haveCropSelection = false;
    useCursor(cursor());

    if (!currentImage())
        return;

    QRect cropRect = m_rectCrop.normalized();

    // The visitor adds the undo steps to the macro
    if (m_optWidget->cmbType->currentIndex() == 0) {
        // The layer(s) under the current layer will take care of adding
        // undo information to the Crop macro.
        if (currentImage()->undo())
            currentImage()->undoAdapter()->beginMacro(i18n("Crop"));

        KisCropVisitor v(cropRect, false);
        KisNodeSP node = currentNode();
        node->accept(v);

        if (currentImage()->undo())
            currentImage()->undoAdapter()->endMacro();

    } else {
        // Resize creates the undo macro itself
        currentImage()->resize(cropRect, true);
    }

    m_rectCrop = QRect(0, 0, 0, 0);

    updateWidgetValues();
}

void KisToolCrop::setCropX(int x)
{
    QRectF updateRect;

    if (!m_haveCropSelection) {
        m_haveCropSelection = true;
        m_rectCrop = QRect(x, 0, 1, 1);
        updateRect = pixelToView(image()->bounds());
    } else {
        updateRect = boundingRect();
        m_rectCrop.moveLeft(x);
    }

    validateSelection();

    updateRect |= boundingRect();
    updateCanvasViewRect(updateRect);
}

void KisToolCrop::setCropY(int y)
{
    QRectF updateRect;

    if (!m_haveCropSelection) {
        m_haveCropSelection = true;
        m_rectCrop = QRect(0, y, 1, 1);
        updateRect = pixelToView(image()->bounds());
    } else {
        updateRect = boundingRect();
        m_rectCrop.moveTop(y);
    }

    validateSelection();

    updateRect |= boundingRect();
    updateCanvasViewRect(updateRect);
}

void KisToolCrop::setCropWidth(int w)
{
    QRectF updateRect;

    if (!m_haveCropSelection) {
        m_haveCropSelection = true;
        m_rectCrop = QRect(0, 0, w, 1);
        updateRect = pixelToView(image()->bounds());
    } else {
        updateRect = boundingRect();
        m_rectCrop.setWidth(w);
    }

    if (m_optWidget->boolRatio->isChecked()) {
        m_rectCrop.setHeight((int)(w / m_optWidget->doubleRatio->value()));
    } else {
        setOptionWidgetRatio((double)m_rectCrop.width() / (double)m_rectCrop.height());
    }

    validateSelection();

    updateRect |= boundingRect();
    updateCanvasViewRect(updateRect);
}

void KisToolCrop::setCropHeight(int h)
{
    QRectF updateRect;

    if (!m_haveCropSelection) {
        m_haveCropSelection = true;
        m_rectCrop = QRect(0, 0, 1, h);
        updateRect = pixelToView(image()->bounds());
    } else {
        updateRect = boundingRect();
        m_rectCrop.setHeight(h);
    }

    if (m_optWidget->boolRatio->isChecked()) {
        m_rectCrop.setWidth((int)(h * m_optWidget->doubleRatio->value()));
    } else {
        setOptionWidgetRatio((double)m_rectCrop.width() / (double)m_rectCrop.height());
    }

    validateSelection();

    updateRect |= boundingRect();
    updateCanvasViewRect(updateRect);
}

void KisToolCrop::setRatio(double)
{
    if (!(m_optWidget->boolWidth->isChecked() && m_optWidget->boolHeight->isChecked())) {
        QRectF updateRect;

        if (!m_haveCropSelection) {
            m_haveCropSelection = true;
            m_rectCrop = QRect(0, 0, 1, 1);
            updateRect = pixelToView(image()->bounds());
        } else {
            updateRect = boundingRect();
        }
        if (m_optWidget->boolWidth->isChecked()) {
            m_rectCrop.setHeight((int)(m_rectCrop.width() / m_optWidget->doubleRatio->value()));
            setOptionWidgetHeight(m_rectCrop.height());
        } else if (m_optWidget->boolHeight->isChecked()) {
            m_rectCrop.setWidth((int)(m_rectCrop.height() * m_optWidget->doubleRatio->value()));
            setOptionWidgetWidth(m_rectCrop.width());
        } else {
            int newwidth = (int)(m_optWidget->doubleRatio->value() * m_rectCrop.height());
            newwidth = (newwidth + m_rectCrop.width()) / 2;
            m_rectCrop.setWidth(newwidth);
            setOptionWidgetWidth(newwidth);
            m_rectCrop.setHeight((int)(newwidth / m_optWidget->doubleRatio->value()));
            setOptionWidgetHeight(m_rectCrop.height());
        }
        validateSelection(false);
        updateRect |= boundingRect();
        updateCanvasViewRect(updateRect);
    }
}

void KisToolCrop::setOptionWidgetX(qint32 x)
{
    // Disable signals otherwise we get the valueChanged signal, which we don't want
    // to go through the logic for setting values that way.
    m_optWidget->intX->blockSignals(true);
    m_optWidget->intX->setValue(x);
    m_optWidget->intX->blockSignals(false);
}

void KisToolCrop::setOptionWidgetY(qint32 y)
{
    m_optWidget->intY->blockSignals(true);
    m_optWidget->intY->setValue(y);
    m_optWidget->intY->blockSignals(false);
}

void KisToolCrop::setOptionWidgetWidth(qint32 x)
{
    m_optWidget->intWidth->blockSignals(true);
    m_optWidget->intWidth->setValue(x);
    m_optWidget->intWidth->blockSignals(false);
}

void KisToolCrop::setOptionWidgetHeight(qint32 y)
{
    m_optWidget->intHeight->blockSignals(true);
    m_optWidget->intHeight->setValue(y);
    m_optWidget->intHeight->blockSignals(false);
}

void KisToolCrop::setOptionWidgetRatio(double ratio)
{
    m_optWidget->doubleRatio->blockSignals(true);
    m_optWidget->doubleRatio->setValue(ratio);
    m_optWidget->doubleRatio->blockSignals(false);
}


QWidget* KisToolCrop::createOptionWidget()
{
    m_optWidget = new WdgToolCrop(0);

    Q_CHECK_PTR(m_optWidget);
    m_optWidget->setObjectName(toolId() + " option widget");

    connect(m_optWidget->bnCrop, SIGNAL(clicked()), this, SLOT(crop()));

    connect(m_optWidget->intX, SIGNAL(valueChanged(int)), this, SLOT(setCropX(int)));
    connect(m_optWidget->intY, SIGNAL(valueChanged(int)), this, SLOT(setCropY(int)));
    connect(m_optWidget->intWidth, SIGNAL(valueChanged(int)), this, SLOT(setCropWidth(int)));
    connect(m_optWidget->intHeight, SIGNAL(valueChanged(int)), this, SLOT(setCropHeight(int)));
    connect(m_optWidget->doubleRatio, SIGNAL(valueChanged(double)), this, SLOT(setRatio(double)));

    m_optWidget->setFixedHeight(m_optWidget->sizeHint().height());

    return m_optWidget;
}

QWidget* KisToolCrop::optionWidget()
{
    return m_optWidget;
}

QRectF KisToolCrop::lowerRightHandleRect(QRectF cropBorderRect)
{
    return QRectF(cropBorderRect.right() - m_handleSize / 2.0, cropBorderRect.bottom() - m_handleSize / 2.0, m_handleSize, m_handleSize);
}

QRectF KisToolCrop::upperRightHandleRect(QRectF cropBorderRect)
{
    return QRectF(cropBorderRect.right() - m_handleSize / 2.0 , cropBorderRect.top() - m_handleSize / 2.0, m_handleSize, m_handleSize);
}

QRectF KisToolCrop::lowerLeftHandleRect(QRectF cropBorderRect)
{
    return QRectF(cropBorderRect.left() - m_handleSize / 2.0 , cropBorderRect.bottom() - m_handleSize / 2.0, m_handleSize, m_handleSize);
}

QRectF KisToolCrop::upperLeftHandleRect(QRectF cropBorderRect)
{
    return QRectF(cropBorderRect.left() - m_handleSize / 2.0, cropBorderRect.top() - m_handleSize / 2.0, m_handleSize, m_handleSize);
}

QRectF KisToolCrop::lowerHandleRect(QRectF cropBorderRect)
{
    return QRectF(cropBorderRect.left() + (cropBorderRect.width() - m_handleSize) / 2.0 , cropBorderRect.bottom() - m_handleSize / 2.0, m_handleSize, m_handleSize);
}

QRectF KisToolCrop::rightHandleRect(QRectF cropBorderRect)
{
    return QRectF(cropBorderRect.right() - m_handleSize / 2.0 , cropBorderRect.top() + (cropBorderRect.height() - m_handleSize) / 2.0, m_handleSize, m_handleSize);
}

QRectF KisToolCrop::upperHandleRect(QRectF cropBorderRect)
{
    return QRectF(cropBorderRect.left() + (cropBorderRect.width() - m_handleSize) / 2.0 , cropBorderRect.top() - m_handleSize / 2.0, m_handleSize, m_handleSize);
}

QRectF KisToolCrop::leftHandleRect(QRectF cropBorderRect)
{
    return QRectF(cropBorderRect.left() - m_handleSize / 2.0, cropBorderRect.top() + (cropBorderRect.height() - m_handleSize) / 2.0, m_handleSize, m_handleSize);
}

QPainterPath KisToolCrop::handlesPath()
{
    QRectF cropBorderRect = borderLineRect();
    QPainterPath path;

    path.addRect(upperLeftHandleRect(cropBorderRect));
    path.addRect(upperRightHandleRect(cropBorderRect));
    path.addRect(lowerLeftHandleRect(cropBorderRect));
    path.addRect(lowerRightHandleRect(cropBorderRect));
    path.addRect(upperHandleRect(cropBorderRect));
    path.addRect(lowerHandleRect(cropBorderRect));
    path.addRect(leftHandleRect(cropBorderRect));
    path.addRect(rightHandleRect(cropBorderRect));

    return path;
}

qint32 KisToolCrop::mouseOnHandle(QPointF currentViewPoint)
{
    QRectF borderRect = borderLineRect();
    qint32 handleType = None;

    if (upperLeftHandleRect(borderRect).contains(currentViewPoint)) {
        handleType = UpperLeft;
    } else if (lowerLeftHandleRect(borderRect).contains(currentViewPoint)) {
        handleType = LowerLeft;
    } else if (upperRightHandleRect(borderRect).contains(currentViewPoint)) {
        handleType = UpperRight;
    } else if (lowerRightHandleRect(borderRect).contains(currentViewPoint)) {
        handleType = LowerRight;
    } else if (upperHandleRect(borderRect).contains(currentViewPoint)) {
        handleType = Upper;
    } else if (lowerHandleRect(borderRect).contains(currentViewPoint)) {
        handleType = Lower;
    } else if (leftHandleRect(borderRect).contains(currentViewPoint)) {
        handleType = Left;
    } else if (rightHandleRect(borderRect).contains(currentViewPoint)) {
        handleType = Right;
    } else if (borderRect.contains(currentViewPoint)) {
        handleType = Inside;
    }

    return handleType;
}

void KisToolCrop::setMoveResizeCursor(qint32 handle)
{
    QCursor cursor;

    switch (handle) {
    case(UpperLeft):
    case(LowerRight):
        cursor = KisCursor::sizeFDiagCursor();
        break;
    case(LowerLeft):
    case(UpperRight):
        cursor = KisCursor::sizeBDiagCursor();
        break;
    case(Upper):
    case(Lower):
        cursor = KisCursor::sizeVerCursor();
        break;
    case(Left):
    case(Right):
        cursor = KisCursor::sizeHorCursor();
        break;
    case(Inside):
        cursor = KisCursor::sizeAllCursor();
        break;
    default:
        cursor = KisCursor::arrowCursor();
        break;
    }
    useCursor(cursor);
}

QRectF KisToolCrop::boundingRect()
{
    QRectF rect = handlesPath().boundingRect();
    rect.adjust(-HANDLE_BORDER_LINE_WIDTH, -HANDLE_BORDER_LINE_WIDTH, HANDLE_BORDER_LINE_WIDTH, HANDLE_BORDER_LINE_WIDTH);
    return rect;
}

#include "kis_tool_crop.moc"
