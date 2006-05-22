/*
 *  Copyright (c) 1999 Matthias Elter  <me@kde.org>
 *  Copyright (c) 2004-2006 Adrian Page <adrian@pagenet.plus.com>
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
 *

 Some of the X11-specific event handling code is based upon code from
 src/kernel/qapplication_x11.cpp from the Qt GUI Toolkit and is subject
 to the following license and copyright:

 ****************************************************************************
**
**
** Implementation of X11 startup routines and event handling
**
** Created : 931029
**
** Copyright (C) 1992-2003 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Unix/X11 may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include <QCursor>
#include <QWheelEvent>
#include <QPaintEvent>
#include <QKeyEvent>
#include <QEvent>
#include <QDropEvent>
#include <QTabletEvent>
#include <QDragEnterEvent>
#include <QMouseEvent>
#include <QApplication>

#include "kis_canvas.h"
#include "kis_cursor.h"
#include "kis_move_event.h"
#include "kis_button_press_event.h"
#include "kis_button_release_event.h"
#include "kis_double_click_event.h"
#include "kis_config.h"
#include "kis_qpaintdevice_canvas.h"
#include "kis_opengl_canvas.h"
#include "kis_config.h"
#include "kis_input_device.h"
#include "fixx11h.h"
#include "kis_canvas_widget.h"

#define QPAINTDEVICE_CANVAS_WIDGET false
#define OPENGL_CANVAS_WIDGET true

KisCanvas::KisCanvas(QWidget *parent, const char *name)
{
    m_parent = parent;
    m_name = name;
    m_enableMoveEventCompressionHint = false;
    m_canvasWidget = 0;
    m_useOpenGL = false;
    createCanvasWidget(QPAINTDEVICE_CANVAS_WIDGET);
}

KisCanvas::~KisCanvas()
{
    delete m_canvasWidget;
}

#ifdef HAVE_OPENGL
void KisCanvas::createCanvasWidget(bool useOpenGL, QGLWidget *sharedContextWidget)
#else
void KisCanvas::createCanvasWidget(bool useOpenGL)
#endif
{
    delete m_canvasWidget;

#ifndef HAVE_OPENGL
    useOpenGL = false;
#else
    if (useOpenGL && !QGLFormat::hasOpenGL()) {
        kDebug(41001) << "Tried to create OpenGL widget when system doesn't have OpenGL\n";
        useOpenGL = false;
    }

    if (useOpenGL) {
        m_canvasWidget = new KisOpenGLCanvasWidget(m_parent, m_name.toLatin1(), sharedContextWidget);
    } else
#endif
    {Qt::WA_PaintOutsidePaintEvent
        m_canvasWidget = new KisQPaintDeviceCanvasWidget(m_parent, m_name.toLatin1());
    }

    m_useOpenGL = useOpenGL;

    Q_CHECK_PTR(m_canvasWidget);
    QWidget *widget = dynamic_cast<QWidget *>(m_canvasWidget);

    widget->setAutoFillBackground(false);
    widget->setAttribute(Qt::WA_OpaquePaintEvent);
    widget->setMouseTracking(true);
    widget->setAcceptDrops(true);
    m_canvasWidget->enableMoveEventCompressionHint(m_enableMoveEventCompressionHint);

#if defined(EXTENDED_X11_TABLET_SUPPORT)
    selectTabletDeviceEvents();
#endif

    connect(m_canvasWidget, SIGNAL(sigGotPaintEvent(QPaintEvent *)), SIGNAL(sigGotPaintEvent(QPaintEvent *)));
    connect(m_canvasWidget, SIGNAL(sigGotEnterEvent(QEvent*)), SIGNAL(sigGotEnterEvent(QEvent*)));
    connect(m_canvasWidget, SIGNAL(sigGotLeaveEvent(QEvent*)), SIGNAL(sigGotLeaveEvent(QEvent*)));
    connect(m_canvasWidget, SIGNAL(sigGotMouseWheelEvent(QWheelEvent*)), SIGNAL(sigGotMouseWheelEvent(QWheelEvent*)));
    connect(m_canvasWidget, SIGNAL(sigGotKeyPressEvent(QKeyEvent*)), SIGNAL(sigGotKeyPressEvent(QKeyEvent*)));
    connect(m_canvasWidget, SIGNAL(sigGotKeyReleaseEvent(QKeyEvent*)), SIGNAL(sigGotKeyReleaseEvent(QKeyEvent*)));
    connect(m_canvasWidget, SIGNAL(sigGotDragEnterEvent(QDragEnterEvent*)), SIGNAL(sigGotDragEnterEvent(QDragEnterEvent*)));
    connect(m_canvasWidget, SIGNAL(sigGotDropEvent(QDropEvent*)), SIGNAL(sigGotDropEvent(QDropEvent*)));
    connect(m_canvasWidget, SIGNAL(sigGotMoveEvent(KisMoveEvent *)), SIGNAL(sigGotMoveEvent(KisMoveEvent *)));
    connect(m_canvasWidget, SIGNAL(sigGotButtonPressEvent(KisButtonPressEvent *)), SIGNAL(sigGotButtonPressEvent(KisButtonPressEvent *)));
    connect(m_canvasWidget, SIGNAL(sigGotButtonReleaseEvent(KisButtonReleaseEvent *)), SIGNAL(sigGotButtonReleaseEvent(KisButtonReleaseEvent *)));
    connect(m_canvasWidget, SIGNAL(sigGotDoubleClickEvent(KisDoubleClickEvent *)), SIGNAL(sigGotDoubleClickEvent(KisDoubleClickEvent *)));

    m_canvasWidget
}

void KisCanvas::createQPaintDeviceCanvas()
{
	createCanvasWidget(QPAINTDEVICE_CANVAS_WIDGET);
}

#ifdef HAVE_OPENGL
void KisCanvas::createOpenGLCanvas(QGLWidget *sharedContextWidget)
{
    createCanvasWidget(OPENGL_CANVAS_WIDGET, sharedContextWidget);
}
#endif

bool KisCanvas::isOpenGLCanvas() const
{
    return m_useOpenGL;
}

void KisCanvas::enableMoveEventCompressionHint(bool enableMoveCompression)
{
    m_enableMoveEventCompressionHint = enableMoveCompression;
    if (m_canvasWidget != 0) {
        m_canvasWidget->enableMoveEventCompressionHint(enableMoveCompression);
    }
}

QWidget *KisCanvas::QPaintDeviceWidget() const
{
    if (m_useOpenGL) {
        return 0;
    } else {
        return dynamic_cast<QWidget *>(m_canvasWidget);
    }
}

#ifdef HAVE_OPENGL
QGLWidget *KisCanvas::OpenGLWidget() const
{
    if (m_useOpenGL) {
        return dynamic_cast<QGLWidget *>(m_canvasWidget);
    } else {
        return 0;
    }
}
#endif

KisCanvasWidgetPainter *KisCanvas::createPainter()
{
    Q_ASSERT(m_canvasWidget != 0);
    return m_canvasWidget->createPainter();
}

KisCanvasWidget *KisCanvas::canvasWidget() const
{
    return m_canvasWidget;
}

void KisCanvas::setGeometry(int x, int y, int width, int height)
{
    Q_ASSERT(m_canvasWidget);
    dynamic_cast<QWidget *>(m_canvasWidget)->setGeometry(x, y, width, height);
}

void KisCanvas::show()
{
    Q_ASSERT(m_canvasWidget);
    dynamic_cast<QWidget *>(m_canvasWidget)->show();
}

void KisCanvas::hide()
{
    Q_ASSERT(m_canvasWidget);
    dynamic_cast<QWidget *>(m_canvasWidget)->hide();
}

int KisCanvas::width() const
{
    Q_ASSERT(m_canvasWidget);
    return dynamic_cast<QWidget *>(m_canvasWidget)->width();
}

int KisCanvas::height() const
{
    Q_ASSERT(m_canvasWidget);
    return dynamic_cast<QWidget *>(m_canvasWidget)->height();
}

void KisCanvas::update()
{
    Q_ASSERT(m_canvasWidget);
    dynamic_cast<QWidget *>(m_canvasWidget)->update();
}

void KisCanvas::update(const QRect& r)
{
    Q_ASSERT(m_canvasWidget);
    dynamic_cast<QWidget *>(m_canvasWidget)->update(r);
}

void KisCanvas::update(int x, int y, int width, int height)
{
    Q_ASSERT(m_canvasWidget);
    dynamic_cast<QWidget *>(m_canvasWidget)->update(x, y, width, height);
}

void KisCanvas::repaint()
{
    Q_ASSERT(m_canvasWidget);
    dynamic_cast<QWidget *>(m_canvasWidget)->repaint();
}

void KisCanvas::repaint(int x, int y, int width, int height)
{
    Q_ASSERT(m_canvasWidget);
    dynamic_cast<QWidget *>(m_canvasWidget)->repaint(x, y, width, height);
}

void KisCanvas::repaint(const QRect& r)
{
    Q_ASSERT(m_canvasWidget);
    dynamic_cast<QWidget *>(m_canvasWidget)->repaint(r);
}

void KisCanvas::repaint(const QRegion& r)
{
    Q_ASSERT(m_canvasWidget);
    dynamic_cast<QWidget *>(m_canvasWidget)->repaint(r);
}

bool KisCanvas::updatesEnabled() const
{
    Q_ASSERT(m_canvasWidget);
    return dynamic_cast<QWidget *>(m_canvasWidget)->updatesEnabled();
}

void KisCanvas::setUpdatesEnabled(bool updatesEnabled)
{
    Q_ASSERT(m_canvasWidget);
    dynamic_cast<QWidget *>(m_canvasWidget)->setUpdatesEnabled(updatesEnabled);
}

void KisCanvas::updateGeometry()
{
    Q_ASSERT(m_canvasWidget);
    dynamic_cast<QWidget *>(m_canvasWidget)->updateGeometry();
}

void KisCanvas::setFocusPolicy(Qt::FocusPolicy focusPolicy)
{
    Q_ASSERT(m_canvasWidget);
    dynamic_cast<QWidget *>(m_canvasWidget)->setFocusPolicy(focusPolicy);
}

QCursor KisCanvas::cursor() const
{
    Q_ASSERT(m_canvasWidget);
    return dynamic_cast<QWidget *>(m_canvasWidget)->cursor();
}

void KisCanvas::setCursor(const QCursor& cursor)
{
    Q_ASSERT(m_canvasWidget);
    dynamic_cast<QWidget *>(m_canvasWidget)->setCursor(cursor);
}

#if defined(EXTENDED_X11_TABLET_SUPPORT)
void KisCanvas::selectTabletDeviceEvents()
{
    Q_ASSERT(m_canvasWidget);
    m_canvasWidget->selectTabletDeviceEvents();
}
#endif

bool KisCanvas::cursorIsOverCanvas() const
{
    if (QApplication::activePopupWidget() != 0) {
        return false;
    }
    if (QApplication::activeModalWidget() != 0) {
        return false;
    }

    QWidget *canvasWidget = dynamic_cast<QWidget *>(m_canvasWidget);
    Q_ASSERT(canvasWidget != 0);

    if (canvasWidget) {
        if (QApplication::widgetAt(QCursor::pos()) == canvasWidget) {
            return true;
        }
    }
    return false;
}

void KisCanvas::handleKeyEvent(QEvent *e)
{
    QKeyEvent *ke = dynamic_cast<QKeyEvent *>(e);

    Q_ASSERT(ke != 0);

    if (ke) {
        QWidget *canvasWidget = dynamic_cast<QWidget *>(m_canvasWidget);
        Q_ASSERT(canvasWidget != 0);

        if (canvasWidget) {
            canvasWidget->setFocus();

            if (e->type() == QEvent::KeyPress) {
                emit sigGotKeyPressEvent(ke);
            } else {
                emit sigGotKeyReleaseEvent(ke);
            }
        }
    }
}

#include "kis_canvas.moc"

