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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qwidget.h>
#ifdef HAVE_GL
#include <qgl.h>
#endif
#include <qpainter.h>

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

class KisCanvasWidget : public QObject {
    Q_OBJECT

public:
    KisCanvasWidget();
    virtual ~KisCanvasWidget();
    
    // When enabled, the canvas may throw away move events if the application
    // is unable to keep up with them, i.e. intermediate move events in the event
    // queue are skipped.
    void enableMoveEventCompressionHint(bool enableMoveCompression) { m_enableMoveEventCompressionHint = enableMoveCompression; }

    virtual KisCanvasWidgetPainter *createPainter() = 0;

#ifdef EXTENDED_X11_TABLET_SUPPORT
    static KisInputDevice findActiveInputDevice();
    virtual void selectTabletDeviceEvents() = 0;

    static void selectTabletDeviceEvents(QWidget *widget);
#endif

#ifdef Q_WS_X11
    static void initX11Support();
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
    void widgetGotPaintEvent(QPaintEvent *event);
    void widgetGotMousePressEvent(QMouseEvent *event);
    void widgetGotMouseReleaseEvent(QMouseEvent *event);
    void widgetGotMouseDoubleClickEvent(QMouseEvent *event);
    void widgetGotMouseMoveEvent(QMouseEvent *event);
    void widgetGotTabletEvent(QTabletEvent *event);
    void widgetGotEnterEvent(QEvent *event );
    void widgetGotLeaveEvent(QEvent *event);
    void widgetGotWheelEvent(QWheelEvent *event);
    void widgetGotKeyPressEvent(QKeyEvent *event);
    void widgetGotKeyReleaseEvent(QKeyEvent *event);
    void widgetGotDragEnterEvent(QDragEnterEvent *event);
    void widgetGotDropEvent(QDropEvent *event);
    void moveEvent(KisMoveEvent *event);
    void buttonPressEvent(KisButtonPressEvent *event);
    void buttonReleaseEvent(KisButtonReleaseEvent *event);
    void doubleClickEvent(KisDoubleClickEvent *event);
    void translateTabletEvent(KisEvent *event);

protected:

    bool m_enableMoveEventCompressionHint;
    double m_lastPressure;

#ifdef Q_WS_X11
    // On X11 systems, Qt throws away mouse move events if the application
    // is unable to keep up with them. We override this behaviour so that
    // we receive all move events, so that painting follows the mouse's motion
    // accurately.
    bool x11Event(XEvent *event, Display *x11Display, WId winId, QPoint widgetOriginPos);
    static Qt::ButtonState translateX11ButtonState(int state);
    static Qt::ButtonState translateX11Button(unsigned int button);

    static bool X11SupportInitialised;

    // Modifier masks for alt/meta - detected at run-time
    static long X11AltMask;
    static long X11MetaMask;

    int m_lastRootX;
    int m_lastRootY;

#ifdef EXTENDED_X11_TABLET_SUPPORT

public:
    class X11TabletDevice
    {
    public:
        X11TabletDevice();
        X11TabletDevice(const XDeviceInfo *deviceInfo);

        bool mightBeTabletDevice() const { return m_mightBeTabletDevice; }

        XID id() const { return m_deviceId; }
        XDevice *xDevice() const { return m_XDevice; }
        QString name() const { return m_name; }

        KisInputDevice inputDevice() const { return m_inputDevice; }
        void setInputDevice(KisInputDevice inputDevice) { m_inputDevice = inputDevice; }

        void setEnabled(bool enabled);
        bool enabled() const;

        Q_INT32 numAxes() const;

        void setXAxis(Q_INT32 axis);
        void setYAxis(Q_INT32 axis);
        void setPressureAxis(Q_INT32 axis);
        void setXTiltAxis(Q_INT32 axis);
        void setYTiltAxis(Q_INT32 axis);
        void setWheelAxis(Q_INT32 axis);
        void setToolIDAxis(Q_INT32 axis);
        void setSerialNumberAxis(Q_INT32 axis);

        static const Q_INT32 NoAxis = -1;
        static const Q_INT32 DefaultAxis = -2;

        Q_INT32 xAxis() const;
        Q_INT32 yAxis() const;
        Q_INT32 pressureAxis() const;
        Q_INT32 xTiltAxis() const;
        Q_INT32 yTiltAxis() const;
        Q_INT32 wheelAxis() const;
        Q_INT32 toolIDAxis() const;
        Q_INT32 serialNumberAxis() const;

        void readSettingsFromConfig();
        void writeSettingsToConfig();

        // These return -1 if the device does not support the event
        int buttonPressEvent() const { return m_buttonPressEvent; }
        int buttonReleaseEvent() const { return m_buttonReleaseEvent; }
        int motionNotifyEvent() const { return m_motionNotifyEvent; }
        int proximityInEvent() const { return m_proximityInEvent; }
        int proximityOutEvent() const { return m_proximityOutEvent; }

        void enableEvents(QWidget *widget) const;

        class State
        {
        public:
            State() {}
            State(const KisPoint& pos, double pressure, const KisVector2D& tilt, double wheel,
                  Q_UINT32 toolID, Q_UINT32 serialNumber);

            // Position, pressure and wheel are normalised to 0 - 1
            KisPoint pos() const { return m_pos; }
            double pressure() const { return m_pressure; }
            // Tilt is normalised to -1->+1
            KisVector2D tilt() const { return m_tilt; }
            double wheel() const { return m_wheel; }
            // Wacom tool id and serial number of device.
            Q_UINT32 toolID() const { return m_toolID; }
            Q_UINT32 serialNumber() const { return m_serialNumber; }

        private:
            KisPoint m_pos;
            double m_pressure;
            KisVector2D m_tilt;
            double m_wheel;
            Q_UINT32 m_toolID;
            Q_UINT32 m_serialNumber;
        };

        State translateAxisData(const int *axisData) const;

    private:
        double translateAxisValue(int value, const XAxisInfo& axisInfo) const;

        XID m_deviceId;
        XDevice *m_XDevice;

        QString m_name;

        bool m_mightBeTabletDevice;
        KisInputDevice m_inputDevice;

        bool m_enabled;

        Q_INT32 m_xAxis;
        Q_INT32 m_yAxis;
        Q_INT32 m_pressureAxis;
        Q_INT32 m_xTiltAxis;
        Q_INT32 m_yTiltAxis;
        Q_INT32 m_wheelAxis;
        Q_INT32 m_toolIDAxis;
        Q_INT32 m_serialNumberAxis;

        QValueVector<XAxisInfo> m_axisInfo;

        int m_motionNotifyEvent;
        int m_buttonPressEvent;
        int m_buttonReleaseEvent;
        int m_proximityInEvent;
        int m_proximityOutEvent;

        QValueVector<XEventClass> m_eventClassList;
    };

    typedef std::map<XID, X11TabletDevice> X11XIDTabletDeviceMap;
    static X11XIDTabletDeviceMap& tabletDeviceMap();

protected:
    static int X11DeviceMotionNotifyEvent;
    static int X11DeviceButtonPressEvent;
    static int X11DeviceButtonReleaseEvent;
    static int X11ProximityInEvent;
    static int X11ProximityOutEvent;

    static X11XIDTabletDeviceMap X11TabletDeviceMap;

#endif // EXTENDED_X11_TABLET_SUPPORT

#endif // Q_WS_X11
};

class KisCanvas : public QObject {
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
    void repaint(bool erase);
    void repaint(int x, int y, int width, int height, bool erase = true);
    void repaint(const QRect& r, bool erase = true);
    void repaint(const QRegion& r, bool erase = true);

    void updateGeometry();

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
    friend class KisCanvasPainter;

    // One of these will be valid, the other null. In Qt3, using a QPainter on
    // a QGLWidget is not reliable.
    QWidget *QPaintDeviceWidget() const;
#ifdef HAVE_GL
    QGLWidget *OpenGLWidget() const;
#endif
    void createQPaintDeviceCanvas();
#ifdef HAVE_GL
    void createOpenGLCanvas(QGLWidget *sharedContextWidget);
#endif
    void show();
    void hide();
    void setGeometry(int x, int y, int width, int height);

    void setUpdatesEnabled(bool updatesEnabled);
    bool isUpdatesEnabled() const;

    void setFocusPolicy(QWidget::FocusPolicy focusPolicy);

    const QCursor& cursor() const;
    void setCursor(const QCursor& cursor);

    KisCanvasWidgetPainter *createPainter();
    KisCanvasWidget *canvasWidget() const;

protected:
#ifdef HAVE_GL
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

