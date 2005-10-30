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
 *

 The X11-specific event handling code is based upon code from
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

#include "kis_canvas.h"
#include "kis_cursor.h"
#include "kis_move_event.h"
#include "kis_button_press_event.h"
#include "kis_button_release_event.h"
#include "kis_double_click_event.h"
#include "kis_config.h"
#include "kis_qpaintdevice_canvas.h"
#include "kis_opengl_canvas.h"

#ifdef Q_WS_X11

#include <qdesktopwidget.h>
#include <qapplication.h>

#include <X11/keysym.h>

bool KisCanvasWidget::X11SupportInitialised = false;
long KisCanvasWidget::X11AltMask = 0;
long KisCanvasWidget::X11MetaMask = 0;

#if defined(EXTENDED_X11_TABLET_SUPPORT)

int KisCanvasWidget::X11DeviceMotionNotifyEvent = -1;
int KisCanvasWidget::X11DeviceButtonPressEvent = -1;
int KisCanvasWidget::X11DeviceButtonReleaseEvent = -1;

//X11XIDTabletDeviceMap KisCanvasWidget::X11TabletDeviceMap;
std::map<XID, KisCanvasWidget::X11TabletDevice> KisCanvasWidget::X11TabletDeviceMap;

#endif // EXTENDED_X11_TABLET_SUPPORT

#endif // Q_WS_X11


namespace {

    static Q_INT32 correctPressureScale( Q_INT32 inPressure )
    {
        KisConfig cfg;
        Q_INT32 correction = cfg.getPressureCorrection();

        Q_INT32 x1, y1, x2, y2;

        if ( correction == 0 ) {
            x1 = 20;
            y1 = 0;
            x2 = 80;
            y2 = 100;
        } else if ( correction < 50 ) {
            x1 = 20 - ( correction / 50 * 20 );
            y1 = 0;
            x2 = 80 + ( correction / 50 * 20 );
            y2 = 100;
        } else if ( correction == 50 ) {
            x1 = 0;
            y1 = 0;
            x2 = 100;
            y2 = 100;
        } else if ( correction > 50 && correction < 100 ){
            x1 = 0;
            y1 = correction / 50 * 20;
            x2 = 100;
            y2 = 100 - ( correction / 50 * 20 );
        } else {
            x1 = 0;
            y1 = 20;
            x2 = 100;
            y2 = 80;
        }

        return inPressure;
    }
}

KisCanvasWidget::KisCanvasWidget()
{
    m_enableMoveEventCompressionHint = false;
    m_lastPressure = 0;

#ifdef Q_WS_X11
    if (!X11SupportInitialised) {
        initX11Support();
    }

    m_lastRootX = -1;
    m_lastRootY = -1;
#endif
}

KisCanvasWidget::~KisCanvasWidget()
{
}

void KisCanvasWidget::widgetGotPaintEvent(QPaintEvent *e)
{
    emit sigGotPaintEvent(e);
}

void KisCanvasWidget::widgetGotMousePressEvent(QMouseEvent *e)
{
    KisButtonPressEvent ke(INPUT_DEVICE_MOUSE, e -> pos(), e -> globalPos(), PRESSURE_DEFAULT, 0, 0, e -> button(), e -> state());
    buttonPressEvent(&ke);
}

void KisCanvasWidget::widgetGotMouseReleaseEvent(QMouseEvent *e)
{
    KisButtonReleaseEvent ke(INPUT_DEVICE_MOUSE, e -> pos(), e -> globalPos(), PRESSURE_DEFAULT, 0, 0, e -> button(), e -> state());
    buttonReleaseEvent(&ke);
}

void KisCanvasWidget::widgetGotMouseDoubleClickEvent(QMouseEvent *e)
{
    KisDoubleClickEvent ke(INPUT_DEVICE_MOUSE, e -> pos(), e -> globalPos(), PRESSURE_DEFAULT, 0, 0, e -> button(), e -> state());
    doubleClickEvent(&ke);
}

void KisCanvasWidget::widgetGotMouseMoveEvent(QMouseEvent *e)
{
    KisMoveEvent ke(INPUT_DEVICE_MOUSE, e -> pos(), e -> globalPos(), PRESSURE_DEFAULT, 0, 0, e -> state());
    moveEvent(&ke);
}

void KisCanvasWidget::widgetGotTabletEvent(QTabletEvent *e)
{
    enumInputDevice device;

    switch (e -> device()) {
    default:
    case QTabletEvent::NoDevice:
    case QTabletEvent::Stylus:
        device = INPUT_DEVICE_STYLUS;
        break;
    case QTabletEvent::Puck:
        device = INPUT_DEVICE_PUCK;
        break;
    case QTabletEvent::Eraser:
        device = INPUT_DEVICE_ERASER;
        break;
    }

    double pressure = correctPressureScale( e -> pressure() ) / 255.0;

    if (e -> type() == QEvent::TabletPress) {
        KisButtonPressEvent ke(device, e -> pos(), e -> globalPos(), pressure, e -> xTilt(), e -> yTilt(), Qt::LeftButton, Qt::NoButton);
        translateTabletEvent(&ke);
    }
    else
    if (e -> type() == QEvent::TabletRelease) {
        KisButtonReleaseEvent ke(device, e -> pos(), e -> globalPos(), pressure, e -> xTilt(), e -> yTilt(), Qt::LeftButton, Qt::NoButton);
        translateTabletEvent(&ke);
    }
    else {
        KisMoveEvent ke(device, e -> pos(), e -> globalPos(), pressure, e -> xTilt(), e -> yTilt(), Qt::NoButton);
        translateTabletEvent(&ke);
#ifdef Q_WS_X11
        // Fix the problem that when you change from using a tablet device to the mouse,
        // the first mouse button event is not recognised. This is because we handle
        // X11 core mouse move events directly so Qt does not get to see them. This breaks
        // the tablet event accept/ignore mechanism, causing Qt to consume the first
        // mouse button event it sees, instead of a mouse move. 'Ignoring' tablet move events
        // stops Qt from stealing the next mouse button event. This does not affect the
        // tablet aware tools as they do not care about mouse moves while the tablet device is
        // drawing.
        e -> ignore();
#endif
    }
}

void KisCanvasWidget::widgetGotEnterEvent(QEvent *e)
{
    emit sigGotEnterEvent(e);
}

void KisCanvasWidget::widgetGotLeaveEvent(QEvent *e)
{
    emit sigGotLeaveEvent(e);
}

void KisCanvasWidget::widgetGotWheelEvent(QWheelEvent *e)
{
    emit sigGotMouseWheelEvent(e);
}

void KisCanvasWidget::widgetGotKeyPressEvent(QKeyEvent *e)
{
    emit sigGotKeyPressEvent(e);
}

void KisCanvasWidget::widgetGotKeyReleaseEvent(QKeyEvent *e)
{
    emit sigGotKeyReleaseEvent(e);
}

void KisCanvasWidget::widgetGotDragEnterEvent(QDragEnterEvent *e)
{
    emit sigGotDragEnterEvent(e);
}

void KisCanvasWidget::widgetGotDropEvent(QDropEvent *e)
{
    emit sigGotDropEvent(e);
}

void KisCanvasWidget::moveEvent(KisMoveEvent *e)
{
    emit sigGotMoveEvent(e);
}

void KisCanvasWidget::buttonPressEvent(KisButtonPressEvent *e)
{
    emit sigGotButtonPressEvent(e);
}

void KisCanvasWidget::buttonReleaseEvent(KisButtonReleaseEvent *e)
{
    emit sigGotButtonReleaseEvent(e);
}

void KisCanvasWidget::doubleClickEvent(KisDoubleClickEvent *e)
{
    emit sigGotDoubleClickEvent(e);
}

void KisCanvasWidget::translateTabletEvent(KisEvent *e)
{
    bool checkThresholdOnly = false;

    if (e -> type() == KisEvent::ButtonPressEvent || e -> type() == KisEvent::ButtonReleaseEvent) {
        KisButtonEvent *b = static_cast<KisButtonEvent *>(e);

        if (b -> button() == Qt::MidButton || b -> button() == Qt::RightButton) {

            if (e -> type() == KisEvent::ButtonPressEvent) {
                buttonPressEvent(static_cast<KisButtonPressEvent *>(e));
            } else {
                buttonReleaseEvent(static_cast<KisButtonReleaseEvent *>(e));
            }

            checkThresholdOnly = true;
        }
    }

    // Use pressure threshold to detect 'left button' press/release
    if (e -> pressure() >= PRESSURE_THRESHOLD && m_lastPressure < PRESSURE_THRESHOLD) {
        KisButtonPressEvent ke(e -> device(), e -> pos(), e -> globalPos(), e -> pressure(), e -> xTilt(), e -> yTilt(), Qt::LeftButton, e -> state());
        buttonPressEvent(&ke);
    } else if (e -> pressure() < PRESSURE_THRESHOLD && m_lastPressure >= PRESSURE_THRESHOLD) {
        KisButtonReleaseEvent ke(e -> device(), e -> pos(), e -> globalPos(), e -> pressure(), e -> xTilt(), e -> yTilt(), Qt::LeftButton, e -> state());
        buttonReleaseEvent(&ke);
    } else {
        if (!checkThresholdOnly) {
            KisMoveEvent ke(e -> device(), e -> pos(), e -> globalPos(), e -> pressure(), e -> xTilt(), e -> yTilt(), e -> state());
            moveEvent(&ke);
        }
    }

    m_lastPressure = e -> pressure();
}

#ifdef Q_WS_X11

void KisCanvasWidget::initX11Support()
{
    Q_ASSERT(!X11SupportInitialised);
    X11SupportInitialised = true;

    Display *x11Display = QApplication::desktop() -> x11Display();

    // Look at the modifier mapping and get the correct masks for alt/meta
    XModifierKeymap *map = XGetModifierMapping(x11Display);

    if (map) {
        int mapIndex = 0;

        for (int maskIndex = 0; maskIndex < 8; maskIndex++) {
            for (int i = 0; i < map -> max_keypermod; i++) {
                if (map -> modifiermap[mapIndex]) {

                    KeySym sym = XKeycodeToKeysym(x11Display, map -> modifiermap[mapIndex], 0);

                    if (X11AltMask == 0 && (sym == XK_Alt_L || sym == XK_Alt_R)) {
                        X11AltMask = 1 << maskIndex;
                    }
                    if (X11MetaMask == 0 && (sym == XK_Meta_L || sym == XK_Meta_R)) {
                        X11MetaMask = 1 << maskIndex;
                    }
                }

                mapIndex++;
            }
        }

        XFreeModifiermap(map);
    }
    else {
        // Assume defaults
        X11AltMask = Mod1Mask;
        X11MetaMask = Mod4Mask;
    }

#if defined(EXTENDED_X11_TABLET_SUPPORT)

    int numDevices = 0;
    const XDeviceInfo *devices = XListInputDevices(x11Display, &numDevices);

    if (devices != NULL) {
        for (int i = 0; i < numDevices; i++) {

            const XDeviceInfo *device = devices + i;
            X11TabletDevice tabletDevice(device);

            if (tabletDevice.device() != INPUT_DEVICE_UNKNOWN) {
                X11TabletDeviceMap[device -> id] = tabletDevice;

                // Event types are device-independent. Store any
                // the device supports.
                if (tabletDevice.buttonPressEvent() >= 0) {
                    X11DeviceButtonPressEvent = tabletDevice.buttonPressEvent();
                }
                if (tabletDevice.buttonReleaseEvent() >= 0) {
                    X11DeviceButtonReleaseEvent = tabletDevice.buttonReleaseEvent();
                }
                if (tabletDevice.motionNotifyEvent() >= 0) {
                    X11DeviceMotionNotifyEvent = tabletDevice.motionNotifyEvent();
                }
            }
        }

        XFreeDeviceList(const_cast<XDeviceInfo *>(devices));
    }
#endif // EXTENDED_X11_TABLET_SUPPORT
}

Qt::ButtonState KisCanvasWidget::translateX11ButtonState(int state)
{
    int buttonState = 0;

    if (state & Button1Mask)
        buttonState |= Qt::LeftButton;
    if (state & Button2Mask)
        buttonState |= Qt::MidButton;
    if (state & Button3Mask)
        buttonState |= Qt::RightButton;
    if (state & ShiftMask)
        buttonState |= Qt::ShiftButton;
    if (state & ControlMask)
        buttonState |= Qt::ControlButton;
    if (state & X11AltMask)
        buttonState |= Qt::AltButton;
    if (state & X11MetaMask)
        buttonState |= Qt::MetaButton;

    return static_cast<Qt::ButtonState>(buttonState);
}

Qt::ButtonState KisCanvasWidget::translateX11Button(unsigned int X11Button)
{
    Qt::ButtonState qtButton;

    switch (X11Button) {
    case Button1:
        qtButton = Qt::LeftButton;
        break;
    case Button2:
        qtButton = Qt::MidButton;
        break;
    case Button3:
        qtButton = Qt::RightButton;
        break;
    default:
        qtButton = Qt::NoButton;
    }

    return qtButton;
}

#if defined(EXTENDED_X11_TABLET_SUPPORT)

KisCanvasWidget::X11TabletDevice::X11TabletDevice()
{
    m_device = INPUT_DEVICE_UNKNOWN;
    m_buttonPressEvent = -1;
    m_buttonReleaseEvent = -1;
    m_motionNotifyEvent = -1;
}

KisCanvasWidget::X11TabletDevice::X11TabletDevice(const XDeviceInfo *deviceInfo)
{
    m_device = INPUT_DEVICE_UNKNOWN;
    m_deviceId = deviceInfo -> id;

    QString deviceName = deviceInfo -> name;
    deviceName = deviceName.lower();

    const char *stylusName = "stylus";
    const char *eraserName = "eraser";
    const char *penName = "pen";
    //const char *cursorName = "cursor";

    if (deviceName.startsWith(stylusName) || deviceName.startsWith(penName)) {
        m_device = INPUT_DEVICE_STYLUS;
    }
    else
    if (deviceName.startsWith(eraserName)) {
        m_device = INPUT_DEVICE_ERASER;
    }

    if (m_device != INPUT_DEVICE_UNKNOWN) {

        // Get the ranges of the valuators
        XAnyClassPtr classInfo = const_cast<XAnyClassPtr>(deviceInfo -> inputclassinfo);

        for (int i = 0; i < deviceInfo -> num_classes; i++) {

            if (classInfo -> c_class == ValuatorClass) {

                const XValuatorInfo *valuatorInfo = reinterpret_cast<const XValuatorInfo *>(classInfo);

                if (valuatorInfo -> num_axes >= 5) {

                    m_xInfo = valuatorInfo -> axes[0];
                    m_yInfo = valuatorInfo -> axes[1];
                    m_pressureInfo = valuatorInfo -> axes[2];
                    m_tiltXInfo = valuatorInfo -> axes[3];
                    m_tiltYInfo = valuatorInfo -> axes[4];
                }
            }

            classInfo = reinterpret_cast<XAnyClassPtr>(reinterpret_cast<char *>(classInfo) + classInfo -> length);
        }

        // Determine the event types it supports. We're only interested in
        // buttons and motion at the moment.
        m_buttonPressEvent = -1;
        m_buttonReleaseEvent = -1;
        m_motionNotifyEvent = -1;

        XDevice *device = XOpenDevice(QApplication::desktop() -> x11Display(), m_deviceId);

        if (device != NULL) {
            for (int i = 0; i < device -> num_classes; i++) {

                XEventClass eventClass;

                if (device -> classes[i].input_class == ButtonClass) {
                    DeviceButtonPress(device, m_buttonPressEvent, eventClass);
                    DeviceButtonRelease(device, m_buttonReleaseEvent, eventClass);
                }
                else
                if (device -> classes[i].input_class == ValuatorClass) {
                    DeviceMotionNotify(device, m_motionNotifyEvent, eventClass);
                }
            }

            // Note: We don't XCloseDevice() since Qt will have already opened
            // it, and only one XCloseDevice() call closes it for all opens.
        }
    }
}

double KisCanvasWidget::X11TabletDevice::translateAxisValue(int value, const XAxisInfo& axisInfo) const
{
    int axisRange = axisInfo.max_value - axisInfo.min_value;
    double translatedValue = 0;

    if (axisRange != 0) {
        translatedValue = (static_cast<double>(value) - axisInfo.min_value) / axisRange;
        if (axisInfo.min_value < 0) {
            translatedValue -= 0.5;
        }
    }

    return translatedValue;
}

KisCanvasWidget::X11TabletDevice::State::State(const KisPoint& pos, double pressure, const KisVector2D& tilt)
    : m_pos(pos), m_pressure(pressure), m_tilt(tilt)
{
}

KisCanvasWidget::X11TabletDevice::State KisCanvasWidget::X11TabletDevice::translateAxisData(const int *axisData) const
{
    KisPoint pos = KisPoint(translateAxisValue(axisData[0], m_xInfo), translateAxisValue(axisData[1], m_yInfo));
    double pressure = translateAxisValue(axisData[2], m_pressureInfo);
    KisVector2D tilt = KisVector2D(translateAxisValue(axisData[3], m_tiltXInfo), translateAxisValue(axisData[4], m_tiltYInfo));
    return State(pos, pressure, tilt);
}

#endif // EXTENDED_X11_TABLET_SUPPORT

bool KisCanvasWidget::x11Event(XEvent *event, Display *x11Display, WId winId, QPoint widgetOriginPos)
{
    if (event -> type == MotionNotify) {
        // Mouse move
        if (!m_enableMoveEventCompressionHint) {

            XMotionEvent motion = event -> xmotion;
            QPoint globalPos(motion.x_root, motion.y_root);

            if (globalPos.x() != m_lastRootX || globalPos.y() != m_lastRootY) {

                int state = translateX11ButtonState(motion.state);
                QPoint pos(motion.x, motion.y);
                QMouseEvent e(QEvent::MouseMove, pos, globalPos, Qt::NoButton, state);

                widgetGotMouseMoveEvent(&e);
            }

            m_lastRootX = globalPos.x();
            m_lastRootY = globalPos.y();

            return true;
        }
        else {
            return false;
        }
    }
    else
#if defined(EXTENDED_X11_TABLET_SUPPORT)
    if (event -> type == X11DeviceMotionNotifyEvent || event -> type == X11DeviceButtonPressEvent || event -> type == X11DeviceButtonReleaseEvent) {
        // Tablet event
        int deviceId;
        const int *axisData;
        Qt::ButtonState button;
        Qt::ButtonState buttonState;

        if (event -> type == X11DeviceMotionNotifyEvent) {
            // Tablet move
            const XDeviceMotionEvent *motion = reinterpret_cast<const XDeviceMotionEvent *>(event);
            XEvent mouseEvent;

            // Look for an accompanying core event.
            if (XCheckTypedWindowEvent(x11Display, winId, MotionNotify, &mouseEvent)) {
                if (motion -> time == mouseEvent.xmotion.time) {
                    // Do nothing
                    //  kdDebug() << "Consumed core event" << endl;
                } else {
                    XPutBackEvent(x11Display, &mouseEvent);
                }
            }

            if (m_enableMoveEventCompressionHint) {
                while (true) {
                    // Look for another motion notify in the queue and skip
                    // to that if found.
                    if (!XCheckTypedWindowEvent(x11Display, winId, X11DeviceMotionNotifyEvent, &mouseEvent)) {
                        break;
                    }

                    motion = reinterpret_cast<const XDeviceMotionEvent *>(&mouseEvent);

                    XEvent coreMotionEvent;

                    // Look for an accompanying core event.
                    if (!XCheckTypedWindowEvent(x11Display, winId, MotionNotify, &coreMotionEvent)) {
                        // Do nothing
                        // kdDebug() << "Didn't find an expected core move event" << endl;
                    }
                }
            }

            deviceId = motion -> deviceid;
            axisData = motion -> axis_data;
            button = Qt::NoButton;
            buttonState = translateX11ButtonState(motion -> state);
        }
        else
        if (event -> type == X11DeviceButtonPressEvent) {
            // Tablet button press
            const XDeviceButtonPressedEvent *buttonPressed = reinterpret_cast<const XDeviceButtonPressedEvent *>(event);
            deviceId = buttonPressed -> deviceid;
            axisData = buttonPressed -> axis_data;
            button = translateX11Button(buttonPressed -> button);
            buttonState = translateX11ButtonState(buttonPressed -> state);

            XEvent mouseEvent;

            // Look for an accompanying core event.
            if (XCheckTypedWindowEvent(x11Display, winId, ButtonPress, &mouseEvent)) {
                if (buttonPressed -> time == mouseEvent.xbutton.time) {
                    // Do nothing
                    // kdDebug() << "Consumed core event" << endl;
                }
                else {
                    XPutBackEvent(x11Display, &mouseEvent);
                }
            }
        }
        else {
            // Tablet button release
            const XDeviceButtonReleasedEvent *buttonReleased = reinterpret_cast<const XDeviceButtonReleasedEvent *>(event);
            deviceId = buttonReleased -> deviceid;
            axisData = buttonReleased -> axis_data;
            button = translateX11Button(buttonReleased -> button);
            buttonState = translateX11ButtonState(buttonReleased -> state);

            XEvent mouseEvent;

            // Look for an accompanying core event.
            if (XCheckTypedWindowEvent(x11Display, winId, ButtonRelease, &mouseEvent)) {
                if (buttonReleased -> time == mouseEvent.xbutton.time) {
                    // Do nothing
                    // kdDebug() << "Consumed core event" << endl;
                }
                else {
                    XPutBackEvent(x11Display, &mouseEvent);
                }
            }
        }

        X11XIDTabletDeviceMap::const_iterator it = X11TabletDeviceMap.find(deviceId);

        if (it != X11TabletDeviceMap.end()) {

            const X11TabletDevice& tabletDevice = (*it).second;
            X11TabletDevice::State deviceState = tabletDevice.translateAxisData(axisData);

            // Map normalised position coordinates to screen coordinates
            QDesktopWidget *desktop = QApplication::desktop();
            KisPoint globalPos(deviceState.pos().x() * desktop -> width(), deviceState.pos().y() * desktop -> height());
            // Convert screen coordinates to widget coordinates
            KisPoint pos = globalPos - widgetOriginPos;

            // Map tilt to -60 - +60 degrees
            KisVector2D tilt(deviceState.tilt().x() * 60, deviceState.tilt().y() * 60);

            if (event -> type == X11DeviceMotionNotifyEvent) {
                KisMoveEvent e(tabletDevice.device(), pos, globalPos, deviceState.pressure(), tilt.x(), tilt.y(), buttonState);
                translateTabletEvent(&e);
            }
            else
            if (event -> type == X11DeviceButtonPressEvent) {
                KisButtonPressEvent e(tabletDevice.device(), pos, globalPos, deviceState.pressure(), tilt.x(), tilt.y(), button, buttonState);
                translateTabletEvent(&e);
            }
            else {
                KisButtonReleaseEvent e(tabletDevice.device(), pos, globalPos, deviceState.pressure(), tilt.x(), tilt.y(), button, buttonState);
                translateTabletEvent(&e);
            }

            return true;
        }
        else {
            return false;
        }
    }
    else
#endif // EXTENDED_X11_TABLET_SUPPORT
    {
        return false;
    }
}

#endif // Q_WS_X11

/*************************************************************************/

#define QPAINTDEVICE_CANVAS_WIDGET false
#define OPENGL_CANVAS_WIDGET true

KisCanvas::KisCanvas(QWidget *parent, const char *name)
{
    m_parent = parent;
    m_name = name;
    m_enableMoveEventCompressionHint = false;
    m_canvasWidget = 0;
    m_useOpenGL = false;
#ifdef HAVE_GL
    createCanvasWidget(QPAINTDEVICE_CANVAS_WIDGET);
#endif
}

KisCanvas::~KisCanvas()
{
    delete m_canvasWidget;
}

#ifdef HAVE_GL
void KisCanvas::createCanvasWidget(bool useOpenGL, QGLWidget *sharedContextWidget)
{
    delete m_canvasWidget;

    if (useOpenGL && !QGLFormat::hasOpenGL()) {
        kdDebug() << "Tried to create OpenGL widget when system doesn't have OpenGL\n";
        useOpenGL = false;
    }

    if (useOpenGL) {
        m_canvasWidget = new KisOpenGLCanvasWidget(m_parent, m_name.latin1(), sharedContextWidget);
    } else {
        m_canvasWidget = new KisQPaintDeviceCanvasWidget(m_parent, m_name.latin1());
    }

    m_useOpenGL = useOpenGL;

    Q_CHECK_PTR(m_canvasWidget);
    QWidget *widget = dynamic_cast<QWidget *>(m_canvasWidget);

    widget -> setBackgroundMode(QWidget::NoBackground);
    widget -> setMouseTracking(true);
    widget -> setAcceptDrops(true);
    m_canvasWidget -> enableMoveEventCompressionHint(m_enableMoveEventCompressionHint);

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
}
#endif

void KisCanvas::createQPaintDeviceCanvas()
{
#ifdef HAVE_GL
	createCanvasWidget(QPAINTDEVICE_CANVAS_WIDGET);
#endif
}

#ifdef HAVE_GL
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
        m_canvasWidget -> enableMoveEventCompressionHint(enableMoveCompression);
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

#ifdef HAVE_GL
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
    return m_canvasWidget -> createPainter();
}

KisCanvasWidget *KisCanvas::canvasWidget() const
{
    return m_canvasWidget;
}

void KisCanvas::setGeometry(int x, int y, int width, int height)
{
    Q_ASSERT(m_canvasWidget);
    dynamic_cast<QWidget *>(m_canvasWidget) -> setGeometry(x, y, width, height);
}

void KisCanvas::show()
{
    Q_ASSERT(m_canvasWidget);
    dynamic_cast<QWidget *>(m_canvasWidget) -> show();
}

int KisCanvas::width() const
{
    Q_ASSERT(m_canvasWidget);
    return dynamic_cast<QWidget *>(m_canvasWidget) -> width();
}

int KisCanvas::height() const
{
    Q_ASSERT(m_canvasWidget);
    return dynamic_cast<QWidget *>(m_canvasWidget) -> height();
}

void KisCanvas::update()
{
    Q_ASSERT(m_canvasWidget);
    dynamic_cast<QWidget *>(m_canvasWidget) -> update();
}

void KisCanvas::update(const QRect& r)
{
    Q_ASSERT(m_canvasWidget);
    dynamic_cast<QWidget *>(m_canvasWidget) -> update(r);
}

void KisCanvas::update(int x, int y, int width, int height)
{
    Q_ASSERT(m_canvasWidget);
    dynamic_cast<QWidget *>(m_canvasWidget) -> update(x, y, width, height);
}

void KisCanvas::repaint()
{
    Q_ASSERT(m_canvasWidget);
    dynamic_cast<QWidget *>(m_canvasWidget) -> repaint();
}

void KisCanvas::repaint(bool erase)
{
    Q_ASSERT(m_canvasWidget);
    dynamic_cast<QWidget *>(m_canvasWidget) -> repaint(erase);
}

void KisCanvas::repaint(int x, int y, int width, int height, bool erase)
{
    Q_ASSERT(m_canvasWidget);
    dynamic_cast<QWidget *>(m_canvasWidget) -> repaint(x, y, width, height, erase);
}

void KisCanvas::repaint(const QRect& r, bool erase)
{
    Q_ASSERT(m_canvasWidget);
    dynamic_cast<QWidget *>(m_canvasWidget) -> repaint(r, erase);
}

void KisCanvas::repaint(const QRegion& r, bool erase)
{
    Q_ASSERT(m_canvasWidget);
    dynamic_cast<QWidget *>(m_canvasWidget) -> repaint(r, erase);
}

bool KisCanvas::isUpdatesEnabled() const
{
    Q_ASSERT(m_canvasWidget);
    return dynamic_cast<QWidget *>(m_canvasWidget) -> isUpdatesEnabled();
}

void KisCanvas::setUpdatesEnabled(bool updatesEnabled)
{
    Q_ASSERT(m_canvasWidget);
    dynamic_cast<QWidget *>(m_canvasWidget) -> setUpdatesEnabled(updatesEnabled);
}

void KisCanvas::setFocusPolicy(QWidget::FocusPolicy focusPolicy)
{
    Q_ASSERT(m_canvasWidget);
    dynamic_cast<QWidget *>(m_canvasWidget) -> setFocusPolicy(focusPolicy);
}

const QCursor& KisCanvas::cursor() const
{
    Q_ASSERT(m_canvasWidget);
    return dynamic_cast<QWidget *>(m_canvasWidget) -> cursor();
}

void KisCanvas::setCursor(const QCursor& cursor)
{
    Q_ASSERT(m_canvasWidget);
    dynamic_cast<QWidget *>(m_canvasWidget) -> setCursor(cursor);
}

#include "kis_canvas.moc"

