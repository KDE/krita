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
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KIS_CANVAS_H_
#define KIS_CANVAS_H_

#include <config.h>
#include <config-krita.h>

#include <QWidget>
#ifdef HAVE_OPENGL
#include <qgl.h>
#endif
#include <QPainter>
//Added by qt3to4:
#include <Q3ValueVector>
#include <QWheelEvent>
#include <QPaintEvent>
#include <QKeyEvent>
#include <QEvent>
#include <QDropEvent>
#include <QTabletEvent>
#include <QDragEnterEvent>
#include <QMouseEvent>

#include "kis_canvas_widget.h"
#include "kis_global.h"
#include "kis_point.h"
#include "kis_vec.h"
#include "kis_input_device.h"

#ifdef Q_WS_X11

// Irix has a different (and better) XInput tablet driver to
// the XFree/xorg driver. Qt needs a separate code path for that
// and so would we.
#if defined(HAVE_XINPUTEXT) && !defined(Q_OS_IRIX)
#define EXTENDED_X11_TABLET_SUPPORT
#endif

#include <map>
#include <X11/Xlib.h>

#if defined(EXTENDED_X11_TABLET_SUPPORT)
#include <X11/extensions/XInput.h>
#endif

#endif // Q_WS_X11

class KisEvent;
class KisMoveEvent;
class KisButtonPressEvent;
class KisButtonReleaseEvent;
class KisDoubleClickEvent;
class KisCanvasWidgetPainter;

/**
 * KisCanvas is is responsible for the interaction with the user via mouse
 * and keyboard. There is one per view. KisCanvas delegates the drawing
 * of the image to KisCanvasWidget and filters the events caught by the
 * KisCanvasWidget and where necessary
 */
class KRITAUI_EXPORT KisCanvas : public QObject {
    Q_OBJECT

public:
    KisCanvas(QWidget *parent, const char *name);
    virtual ~KisCanvas();

    // When enabled, the canvas may throw away move events if the application
    // is unable to keep up with them, i.e. intermediate move events in the event
    // queue are skipped.
    void enableMoveEventCompressionHint(bool enableMoveCompression);

    bool isOpenGLCanvas() const;

    /**
     * Returns true if the cursor is over the canvas.
     */
    bool cursorIsOverCanvas() const;

    /**
     * Handle the given event (which must be a key event) as if the canvas
     * had received it directly.
     */
    void handleKeyEvent(QEvent *e);

    int width() const;
    int height() const;

    void update();
    void update(const QRect& r);
    void update(int x, int y, int width, int height);
    void repaint();
    void repaint(int x, int y, int width, int height);
    void repaint(const QRect& r);
    void repaint(const QRegion& r);

    void updateGeometry();

    QWidget * canvasWidget();

#if defined(EXTENDED_X11_TABLET_SUPPORT)
    void selectTabletDeviceEvents();
#endif

signals:
    void sigGotPaintEvent(QPaintEvent*);
    void sigGotEnterEvent(QEvent*);
    void sigGotLeaveEvent(QEvent*);
    void sigGotMouseWheelEvent(QWheelEvent*);
    void sigGotKeyPressEvent(QKeyEvent*);
    void sigGotKeyReleaseEvent(QKeyEvent*);
    void sigGotDragEnterEvent(QDragEnterEvent*);
    void sigGotDropEvent(QDropEvent*);
    void sigGotMoveEvent(KisMoveEvent *);
    void sigGotButtonPressEvent(KisButtonPressEvent *);
    void sigGotButtonReleaseEvent(KisButtonReleaseEvent *);
    void sigGotDoubleClickEvent(KisDoubleClickEvent *);

protected:
    // Allow KisView to render on the widget directly, but everything else
    // has restricted access.
    friend class KisView;
    friend class KisCanvasControllerImpl;
    friend class QPainter;

    // One of these will be valid, the other null. In Qt3, using a QPainter on
    // a QGLWidget is not reliable.
    QWidget *QPaintDeviceWidget() const;
#ifdef HAVE_OPENGL
    QGLWidget *OpenGLWidget() const;
#endif
    void createQPaintDeviceCanvas();
#ifdef HAVE_OPENGL
    void createOpenGLCanvas(QGLWidget *sharedContextWidget);
#endif
    void show();
    void hide();
    void setGeometry(int x, int y, int width, int height);

    void setUpdatesEnabled(bool updatesEnabled);
    bool updatesEnabled() const;

    void setFocusPolicy(Qt::FocusPolicy focusPolicy);

    QCursor cursor() const;
    void setCursor(const QCursor& cursor);

protected:
#ifdef HAVE_OPENGL
    void createCanvasWidget(bool useOpenGL, QGLWidget *sharedContextWidget = 0);
#else
    void createCanvasWidget(bool useOpenGL);
#endif
    QWidget *m_parent;
    QString m_name;
    KisCanvasWidget *m_canvasWidget;
    bool m_enableMoveEventCompressionHint;
    bool m_useOpenGL;
};

#endif // KIS_CANVAS_H_

