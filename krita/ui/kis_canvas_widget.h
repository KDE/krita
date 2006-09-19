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

#ifndef KIS_CANVAS_WIDGET_H_
#define KIS_CANVAS_WIDGET_H_

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

#include "kis_global.h"
#include "kis_point.h"
#include "kis_vec.h"
#include "kis_canvas.h"
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

class KisCanvasWidget : public QObject {
    Q_OBJECT

public:
    KisCanvasWidget();
    virtual ~KisCanvasWidget();

    // When enabled, the canvas may throw away move events if the application
    // is unable to keep up with them, i.e. intermediate move events in the event
    // queue are skipped.
    void enableMoveEventCompressionHint(bool enableMoveCompression) { m_enableMoveEventCompressionHint = enableMoveCompression; }

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
    void sigGotMouseWheelEvent(QWheelEvent*);
    void sigGotKeyPressEvent(QKeyEvent*);
    void sigGotKeyReleaseEvent(QKeyEvent*);
    void sigGotDragEnterEvent(QDragEnterEvent*);
    void sigGotDropEvent(QDropEvent*);
    void sigGotMoveEvent(KisMoveEvent *);
    void sigGotButtonPressEvent(KisButtonPressEvent *);
    void sigGotButtonReleaseEvent(KisButtonReleaseEvent *);
    void sigGotDoubleClickEvent(KisDoubleClickEvent *);
    void sigGotEnterEvent(QEvent *);
    void sigGotLeaveEvent(QEvent *);

protected:
    void widgetGotPaintEvent(QPaintEvent *event);
    void widgetGotMousePressEvent(QMouseEvent *event);
    void widgetGotMouseReleaseEvent(QMouseEvent *event);
    void widgetGotMouseDoubleClickEvent(QMouseEvent *event);
    void widgetGotMouseMoveEvent(QMouseEvent *event);
    void widgetGotTabletEvent(QTabletEvent *event);
    void widgetGotWheelEvent(QWheelEvent *event);
    void widgetGotKeyPressEvent(QKeyEvent *event);
    void widgetGotKeyReleaseEvent(QKeyEvent *event);
    void widgetGotDragEnterEvent(QDragEnterEvent *event);
    void widgetGotDropEvent(QDropEvent *event);
    void widgetGotEnterEvent(QEvent * event);
    void widgetGotLeaveEvent(QEvent * event);
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
    static Qt::KeyboardModifiers translateX11KeyboardModifiers(int state);
    static Qt::MouseButtons translateX11MouseButtons(int state);
    static Qt::MouseButton translateX11Button(unsigned int button);

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
        bool needsFindingActiveByProximity() const;

        XID id() const { return m_deviceId; }
        XDevice *xDevice() const { return m_XDevice; }
        QString name() const { return m_name; }

        KisInputDevice inputDevice() const { return m_inputDevice; }
        void setInputDevice(KisInputDevice inputDevice) { m_inputDevice = inputDevice; }

        void setEnabled(bool enabled);
        bool enabled() const;

        qint32 numAxes() const;

        void setXAxis(qint32 axis);
        void setYAxis(qint32 axis);
        void setPressureAxis(qint32 axis);
        void setXTiltAxis(qint32 axis);
        void setYTiltAxis(qint32 axis);
        void setWheelAxis(qint32 axis);
        void setToolIDAxis(qint32 axis);
        void setSerialNumberAxis(qint32 axis);

        static const qint32 NoAxis = -1;
        static const qint32 DefaultAxis = -2;

        qint32 xAxis() const;
        qint32 yAxis() const;
        qint32 pressureAxis() const;
        qint32 xTiltAxis() const;
        qint32 yTiltAxis() const;
        qint32 wheelAxis() const;
        qint32 toolIDAxis() const;
        qint32 serialNumberAxis() const;

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
                  quint32 toolID, quint32 serialNumber);

            // Position, pressure and wheel are normalised to 0 - 1
            KisPoint pos() const { return m_pos; }
            double pressure() const { return m_pressure; }
            // Tilt is normalised to -1->+1
            KisVector2D tilt() const { return m_tilt; }
            double wheel() const { return m_wheel; }
            // Wacom tool id and serial number of device.
            quint32 toolID() const { return m_toolID; }
            quint32 serialNumber() const { return m_serialNumber; }

        private:
            KisPoint m_pos;
            double m_pressure;
            KisVector2D m_tilt;
            double m_wheel;
            quint32 m_toolID;
            quint32 m_serialNumber;
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

        qint32 m_xAxis;
        qint32 m_yAxis;
        qint32 m_pressureAxis;
        qint32 m_xTiltAxis;
        qint32 m_yTiltAxis;
        qint32 m_wheelAxis;
        qint32 m_toolIDAxis;
        qint32 m_serialNumberAxis;

        Q3ValueVector<XAxisInfo> m_axisInfo;

        int m_motionNotifyEvent;
        int m_buttonPressEvent;
        int m_buttonReleaseEvent;
        int m_proximityInEvent;
        int m_proximityOutEvent;

        Q3ValueVector<XEventClass> m_eventClassList;
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

#endif // KIS_CANVAS_WIDGET_H_

