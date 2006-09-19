/*
 *  Copyright (c) 2005 Adrian Page <adrian@pagenet.plus.com>
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

#include <QMouseEvent>
#include <QTabletEvent>
#include <QDragEnterEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QEvent>
#include <QPaintEvent>
#include <QDropEvent>

#ifdef Q_WS_X11
#include <QX11Info>
#endif

#include "kis_canvas.h"
#include "kis_opengl_canvas.h"
#include "QPainter"

#ifdef HAVE_OPENGL
KisOpenGLCanvasWidget::KisOpenGLCanvasWidget(QWidget *parent, const char *name, QGLWidget *sharedContextWidget)
    : QGLWidget(KisOpenGLCanvasFormat, parent, sharedContextWidget)
{
    QGLWidget::setObjectName(name);
    setAttribute(Qt::WA_PaintOutsidePaintEvent);

    if (isSharing()) {
        kDebug(41001) << "Created QGLWidget with sharing\n";
    } else {
        kDebug(41001) << "Created QGLWidget with no sharing\n";
    }
}

KisOpenGLCanvasWidget::~KisOpenGLCanvasWidget()
{
}

void KisOpenGLCanvasWidget::paintEvent(QPaintEvent *e)
{
    QGLWidget::paintEvent(e);

    widgetGotPaintEvent(e);
}

void KisOpenGLCanvasWidget::mousePressEvent(QMouseEvent *e)
{
    widgetGotMousePressEvent(e);
}

void KisOpenGLCanvasWidget::mouseReleaseEvent(QMouseEvent *e)
{
    widgetGotMouseReleaseEvent(e);
}

void KisOpenGLCanvasWidget::mouseDoubleClickEvent(QMouseEvent *e)
{
    widgetGotMouseDoubleClickEvent(e);
}

void KisOpenGLCanvasWidget::mouseMoveEvent(QMouseEvent *e)
{
    widgetGotMouseMoveEvent(e);
}

void KisOpenGLCanvasWidget::tabletEvent(QTabletEvent *e)
{
    widgetGotTabletEvent(e);
}

void KisOpenGLCanvasWidget::enterEvent(QEvent * e)
{
    widgetGotEnterEvent(e);
}

void KisOpenGLCanvasWidget::leaveEvent(QEvent * e)
{
    widgetGotLeaveEvent(e);
}

void KisOpenGLCanvasWidget::wheelEvent(QWheelEvent *e)
{
    widgetGotWheelEvent(e);
}

void KisOpenGLCanvasWidget::keyPressEvent(QKeyEvent *e)
{
    widgetGotKeyPressEvent(e);
}

void KisOpenGLCanvasWidget::keyReleaseEvent(QKeyEvent *e)
{
    widgetGotKeyReleaseEvent(e);
}

void KisOpenGLCanvasWidget::dragEnterEvent(QDragEnterEvent *e)
{
    widgetGotDragEnterEvent(e);
}

void KisOpenGLCanvasWidget::dropEvent(QDropEvent *e)
{
    widgetGotDropEvent(e);
}

#ifdef Q_WS_X11

bool KisOpenGLCanvasWidget::x11Event(XEvent *event)
{
    return KisCanvasWidget::x11Event(event, QX11Info::display(), winId(), mapToGlobal(QPoint(0, 0)));
}

#endif // Q_WS_X11

#if defined(EXTENDED_X11_TABLET_SUPPORT)
void KisOpenGLCanvasWidget::selectTabletDeviceEvents()
{
    KisCanvasWidget::selectTabletDeviceEvents(this);
}
#endif

#endif // HAVE_OPENGL

