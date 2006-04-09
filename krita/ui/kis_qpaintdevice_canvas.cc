/*
 *  Copyright (c) 1999 Matthias Elter  <me@kde.org>
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.g
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_canvas.h"
#include "kis_canvas_painter.h"
#include "kis_qpaintdevice_canvas.h"
#include "kis_qpaintdevice_canvas_painter.h"
//Added by qt3to4:
#include <QMouseEvent>
#include <QTabletEvent>
#include <QDragEnterEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QEvent>
#include <QPaintEvent>
#include <QDropEvent>

KisQPaintDeviceCanvasWidget::KisQPaintDeviceCanvasWidget(QWidget *parent, const char *name)
    : QWidget(parent, name)
{
}

KisQPaintDeviceCanvasWidget::~KisQPaintDeviceCanvasWidget()
{
}

void KisQPaintDeviceCanvasWidget::paintEvent(QPaintEvent *e)
{
    widgetGotPaintEvent(e);
}

void KisQPaintDeviceCanvasWidget::mousePressEvent(QMouseEvent *e)
{
    widgetGotMousePressEvent(e);
}

void KisQPaintDeviceCanvasWidget::mouseReleaseEvent(QMouseEvent *e)
{
    widgetGotMouseReleaseEvent(e);
}

void KisQPaintDeviceCanvasWidget::mouseDoubleClickEvent(QMouseEvent *e)
{
    widgetGotMouseDoubleClickEvent(e);
}

void KisQPaintDeviceCanvasWidget::mouseMoveEvent(QMouseEvent *e)
{
    widgetGotMouseMoveEvent(e);
}

void KisQPaintDeviceCanvasWidget::tabletEvent(QTabletEvent *e)
{
    widgetGotTabletEvent(e);
}

void KisQPaintDeviceCanvasWidget::enterEvent(QEvent *e)
{
    //widgetGotEnterEvent(e);
}

void KisQPaintDeviceCanvasWidget::leaveEvent(QEvent *e)
{
    //widgetGotLeaveEvent(e);
}

void KisQPaintDeviceCanvasWidget::wheelEvent(QWheelEvent *e)
{
    widgetGotWheelEvent(e);
}

void KisQPaintDeviceCanvasWidget::keyPressEvent(QKeyEvent *e)
{
    widgetGotKeyPressEvent(e);
}

void KisQPaintDeviceCanvasWidget::keyReleaseEvent(QKeyEvent *e)
{
    widgetGotKeyReleaseEvent(e);
}

void KisQPaintDeviceCanvasWidget::dragEnterEvent(QDragEnterEvent *e)
{
    widgetGotDragEnterEvent(e);
}

void KisQPaintDeviceCanvasWidget::dropEvent(QDropEvent *e)
{
    widgetGotDropEvent(e);
}

#ifdef Q_WS_X11

bool KisQPaintDeviceCanvasWidget::x11Event(XEvent *event)
{
    return KisCanvasWidget::x11Event(event, x11Display(), winId(), mapToGlobal(QPoint(0, 0)));
}

#endif // Q_WS_X11

KisCanvasWidgetPainter *KisQPaintDeviceCanvasWidget::createPainter()
{
    return new KisQPaintDeviceCanvasPainter(this);
}

#if defined(EXTENDED_X11_TABLET_SUPPORT)
void KisQPaintDeviceCanvasWidget::selectTabletDeviceEvents()
{
    KisCanvasWidget::selectTabletDeviceEvents(this);
}
#endif

