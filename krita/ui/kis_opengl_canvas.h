//Added by qt3to4:
#include <QMouseEvent>
#include <QTabletEvent>
#include <QDragEnterEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QEvent>
#include <QPaintEvent>
#include <QDropEvent>
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
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KIS_OPENGL_CANVAS_H_
#define KIS_OPENGL_CANVAS_H_

#include <config.h>
#include <config-krita.h>

#ifdef HAVE_OPENGL

#include <QWidget>
#include <qgl.h>

#include "kis_global.h"
#include "kis_canvas.h"

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#endif // Q_WS_X11

#define KisOpenGLCanvasFormat (QGL::DoubleBuffer|QGL::Rgba|QGL::DirectRendering|QGL::NoDepthBuffer)

class KisOpenGLCanvasWidget : public virtual QGLWidget, public virtual KisCanvasWidget {
public:
    KisOpenGLCanvasWidget(QWidget *parent, const char *name, QGLWidget *sharedContextWidget);
    ~KisOpenGLCanvasWidget();

    virtual KisCanvasWidgetPainter *createPainter();

#if defined(EXTENDED_X11_TABLET_SUPPORT)
    virtual void selectTabletDeviceEvents();
#endif

protected:
    virtual void paintEvent(QPaintEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void mouseDoubleClickEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void tabletEvent(QTabletEvent *event);
    virtual void enterEvent(QEvent *event );
    virtual void leaveEvent(QEvent *event);
    virtual void wheelEvent(QWheelEvent *event);
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void keyReleaseEvent(QKeyEvent *event);
    virtual void dragEnterEvent(QDragEnterEvent *event);
    virtual void dropEvent(QDropEvent *event);
#ifdef Q_WS_X11
    bool x11Event(XEvent *event);
#endif // Q_WS_X11
};
#endif // HAVE_OPENGL

#endif // KIS_OPENGL_CANVAS_H_

