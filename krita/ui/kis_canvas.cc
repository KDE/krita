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

#include <qcursor.h>
//Added by qt3to4:
#include <QWheelEvent>
#include <QPaintEvent>
#include <QKeyEvent>
#include <QEvent>
#include <QDropEvent>
#include <QTabletEvent>
#include <QDragEnterEvent>
#include <QMouseEvent>

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

#ifdef Q_WS_X11

#include <qdesktopwidget.h>
#include <qapplication.h>
#include <QX11Info>

#include <X11/keysym.h>

bool KisCanvasWidget::X11SupportInitialised = false;
long KisCanvasWidget::X11AltMask = 0;
long KisCanvasWidget::X11MetaMask = 0;

#if defined(EXTENDED_X11_TABLET_SUPPORT)

int KisCanvasWidget::X11DeviceMotionNotifyEvent = -1;
int KisCanvasWidget::X11DeviceButtonPressEvent = -1;
int KisCanvasWidget::X11DeviceButtonReleaseEvent = -1;
int KisCanvasWidget::X11ProximityInEvent = -1;
int KisCanvasWidget::X11ProximityOutEvent = -1;

//X11XIDTabletDeviceMap KisCanvasWidget::X11TabletDeviceMap;
std::map<XID, KisCanvasWidget::X11TabletDevice> KisCanvasWidget::X11TabletDeviceMap;

#endif // EXTENDED_X11_TABLET_SUPPORT

#endif // Q_WS_X11

namespace {

    static qint32 correctPressureScale( qint32 inPressure )
    {
        KisConfig cfg;
        qint32 correction = cfg.getPressureCorrection();

        qint32 x1, y1, x2, y2;

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
    KisButtonPressEvent ke(KisInputDevice::mouse(), e->pos(), e->globalPos(), PRESSURE_DEFAULT, 0, 0, e->button(), e->buttons(), e->modifiers());
    buttonPressEvent(&ke);
}

void KisCanvasWidget::widgetGotMouseReleaseEvent(QMouseEvent *e)
{
    KisButtonReleaseEvent ke(KisInputDevice::mouse(), e->pos(), e->globalPos(), PRESSURE_DEFAULT, 0, 0, e->button(), e->buttons(), e->modifiers());
    buttonReleaseEvent(&ke);
}

void KisCanvasWidget::widgetGotMouseDoubleClickEvent(QMouseEvent *e)
{
    KisDoubleClickEvent ke(KisInputDevice::mouse(), e->pos(), e->globalPos(), PRESSURE_DEFAULT, 0, 0, e->button(), e->buttons(), e->modifiers());
    doubleClickEvent(&ke);
}

void KisCanvasWidget::widgetGotMouseMoveEvent(QMouseEvent *e)
{
    KisMoveEvent ke(KisInputDevice::mouse(), e->pos(), e->globalPos(), PRESSURE_DEFAULT, 0, 0, e->buttons(), e->modifiers());
    moveEvent(&ke);
}

void KisCanvasWidget::widgetGotTabletEvent(QTabletEvent *e)
{
    KisInputDevice device;

    switch (e->device()) {
    default:
    case QTabletEvent::NoDevice:
    case QTabletEvent::Stylus:
        device = KisInputDevice::stylus();
        break;
    case QTabletEvent::Puck:
        device = KisInputDevice::puck();
        break;
    case QTabletEvent::Eraser:
        device = KisInputDevice::eraser();
        break;
    }

    double pressure = correctPressureScale(static_cast<qint32>(e->pressure() * 255)) / 255.0;

    if (e->type() == QEvent::TabletPress) {
        KisButtonPressEvent ke(device, e->pos(), e->globalPos(), pressure, e->xTilt(), e->yTilt(), Qt::LeftButton, Qt::NoButton, e->modifiers());
        translateTabletEvent(&ke);
    }
    else
    if (e->type() == QEvent::TabletRelease) {
        KisButtonReleaseEvent ke(device, e->pos(), e->globalPos(), pressure, e->xTilt(), e->yTilt(), Qt::LeftButton, Qt::NoButton, e->modifiers());
        translateTabletEvent(&ke);
    }
    else {
        KisMoveEvent ke(device, e->pos(), e->globalPos(), pressure, e->xTilt(), e->yTilt(), Qt::NoButton, e->modifiers());
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
        e->ignore();
#endif
    }
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
    QWidget *widget = dynamic_cast<QWidget *>(this);
    Q_ASSERT(widget != 0);

    if (widget) {
        widget->setFocus();
    }

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
    if (QApplication::activeModalWidget() == 0) {

        bool checkThresholdOnly = false;

        if (e->type() == KisEvent::ButtonPressEvent || e->type() == KisEvent::ButtonReleaseEvent) {
            KisButtonEvent *b = static_cast<KisButtonEvent *>(e);

            if (b->button() == Qt::MidButton || b->button() == Qt::RightButton) {

                if (e->type() == KisEvent::ButtonPressEvent) {
                    buttonPressEvent(static_cast<KisButtonPressEvent *>(e));
                } else {
                    buttonReleaseEvent(static_cast<KisButtonReleaseEvent *>(e));
                }

                checkThresholdOnly = true;
            }
        }

        double previousPressure = m_lastPressure;

        // Store the last pressure before we generate a button press/release event as the event might cause
        // us to enter another event loop, and then the same pressure difference would generate another button event,
        // leading to endless recursion.
        m_lastPressure = e->pressure();

        // Use pressure threshold to detect 'left button' press/release
        if (e->pressure() >= PRESSURE_THRESHOLD && previousPressure < PRESSURE_THRESHOLD) {
            KisButtonPressEvent ke(e->device(), e->pos(), e->globalPos(), e->pressure(), e->xTilt(), e->yTilt(), Qt::LeftButton, e->buttons(), e->modifiers());
            buttonPressEvent(&ke);
        } else if (e->pressure() < PRESSURE_THRESHOLD && previousPressure >= PRESSURE_THRESHOLD) {
            KisButtonReleaseEvent ke(e->device(), e->pos(), e->globalPos(), e->pressure(), e->xTilt(), e->yTilt(), Qt::LeftButton, e->buttons(), e->modifiers());
            buttonReleaseEvent(&ke);
        } else {
            if (!checkThresholdOnly) {
                KisMoveEvent ke(e->device(), e->pos(), e->globalPos(), e->pressure(), e->xTilt(), e->yTilt(), e->buttons(), e->modifiers());
                moveEvent(&ke);
            }
        }
    }
}

#ifdef Q_WS_X11

void KisCanvasWidget::initX11Support()
{
    if (X11SupportInitialised)
    {
        return;
    }

    X11SupportInitialised = true;

    Display *x11Display = QX11Info::display();

    // Look at the modifier mapping and get the correct masks for alt/meta
    XModifierKeymap *map = XGetModifierMapping(x11Display);

    if (map) {
        int mapIndex = 0;

        for (int maskIndex = 0; maskIndex < 8; maskIndex++) {
            for (int i = 0; i < map->max_keypermod; i++) {
                if (map->modifiermap[mapIndex]) {

                    KeySym sym = XKeycodeToKeysym(x11Display, map->modifiermap[mapIndex], 0);

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
        XID lastStylusSeen = 0;
        XID lastEraserSeen = 0;
        bool foundStylus = false;
        bool foundEraser = false;

        for (int i = 0; i < numDevices; i++) {

            const XDeviceInfo *device = devices + i;
            X11TabletDevice tabletDevice(device);

            if (tabletDevice.mightBeTabletDevice()) {

                tabletDevice.readSettingsFromConfig();

                QString lowerCaseName = tabletDevice.name().toLower();

                // Find the devices that Qt will use as its stylus and eraser devices.
                if (!foundStylus || !foundEraser) {
                    if (lowerCaseName.startsWith("stylus") || lowerCaseName.startsWith("pen")) {
                        lastStylusSeen = device->id;
                        foundStylus = true;
                    }
                    else if (lowerCaseName.startsWith("eraser")) {
                        lastEraserSeen = device->id;
                        foundEraser = true;
                    }
                }

                X11TabletDeviceMap[device->id] = tabletDevice;

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
                if (tabletDevice.proximityInEvent() >= 0) {
                    X11ProximityInEvent = tabletDevice.proximityInEvent();
                }
                if (tabletDevice.proximityOutEvent() >= 0) {
                    X11ProximityOutEvent = tabletDevice.proximityOutEvent();
                }
            }
        }

        // Allocate input devices.
        for (X11XIDTabletDeviceMap::iterator it = X11TabletDeviceMap.begin(); it != X11TabletDeviceMap.end(); ++it) {

            X11TabletDevice& tabletDevice = (*it).second;

            if (foundStylus && tabletDevice.id() == lastStylusSeen) {
                tabletDevice.setInputDevice(KisInputDevice::stylus());
            } else if (foundEraser && tabletDevice.id() == lastEraserSeen) {
                tabletDevice.setInputDevice(KisInputDevice::eraser());
            } else {
                tabletDevice.setInputDevice(KisInputDevice::allocateInputDevice());
            }
        }

        XFreeDeviceList(const_cast<XDeviceInfo *>(devices));
    }
#endif // EXTENDED_X11_TABLET_SUPPORT
}

Qt::KeyboardModifiers KisCanvasWidget::translateX11KeyboardModifiers(int state)
{
    Qt::KeyboardModifiers modifiers = Qt::NoModifier;

    if (state & ShiftMask)
        modifiers |= Qt::ShiftModifier;
    if (state & ControlMask)
        modifiers |= Qt::ControlModifier;
    if (state & X11AltMask)
        modifiers |= Qt::AltModifier;
    if (state & X11MetaMask)
        modifiers |= Qt::MetaModifier;

    return modifiers;
}

Qt::MouseButtons KisCanvasWidget::translateX11MouseButtons(int state)
{
    Qt::MouseButtons buttons = Qt::NoButton;

    if (state & Button1Mask)
       buttons |= Qt::LeftButton;
    if (state & Button2Mask)
       buttons |= Qt::MidButton;
    if (state & Button3Mask)
       buttons |= Qt::RightButton;

    return buttons;
}

Qt::MouseButton KisCanvasWidget::translateX11Button(unsigned int X11Button)
{
    Qt::MouseButton qtButton;

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
    m_mightBeTabletDevice = false;
    m_inputDevice = KisInputDevice::unknown();
    m_enabled = false;
    m_xAxis = NoAxis;
    m_yAxis = NoAxis;
    m_pressureAxis = NoAxis;
    m_xTiltAxis = NoAxis;
    m_yTiltAxis = NoAxis;
    m_wheelAxis = NoAxis;
    m_toolIDAxis = NoAxis;
    m_serialNumberAxis = NoAxis;
    m_buttonPressEvent = -1;
    m_buttonReleaseEvent = -1;
    m_motionNotifyEvent = -1;
    m_proximityInEvent = -1;
    m_proximityOutEvent = -1;
}

KisCanvasWidget::X11TabletDevice::X11TabletDevice(const XDeviceInfo *deviceInfo)
{
    m_mightBeTabletDevice = false;
    m_inputDevice = KisInputDevice::unknown();
    m_enabled = false;
    m_xAxis = NoAxis;
    m_yAxis = NoAxis;
    m_pressureAxis = NoAxis;
    m_xTiltAxis = NoAxis;
    m_yTiltAxis = NoAxis;
    m_wheelAxis = NoAxis;
    m_toolIDAxis = NoAxis;
    m_serialNumberAxis = NoAxis;

    m_deviceId = deviceInfo->id;
    m_name = deviceInfo->name;

    // Get the ranges of the valuators
    XAnyClassPtr classInfo = const_cast<XAnyClassPtr>(deviceInfo->inputclassinfo);

    for (int i = 0; i < deviceInfo->num_classes; i++) {

        if (classInfo->c_class == ValuatorClass) {

            const XValuatorInfo *valuatorInfo = reinterpret_cast<const XValuatorInfo *>(classInfo);

            // Need at least x, y, and pressure.

            if (valuatorInfo->num_axes >= 3) {

                for (unsigned int axis = 0; axis < valuatorInfo->num_axes; axis++) {
                    m_axisInfo.append(valuatorInfo->axes[axis]);
                }

                m_mightBeTabletDevice = true;
            }
        }

        classInfo = reinterpret_cast<XAnyClassPtr>(reinterpret_cast<char *>(classInfo) + classInfo->length);
    }

    // Determine the event types it supports. We're only interested in
    // buttons and motion at the moment.
    m_buttonPressEvent = -1;
    m_buttonReleaseEvent = -1;
    m_motionNotifyEvent = -1;
    m_proximityInEvent = -1;
    m_proximityOutEvent = -1;

    m_XDevice = XOpenDevice(QX11Info::display(), m_deviceId);

    if (m_XDevice != NULL) {
        for (int i = 0; i < m_XDevice->num_classes; i++) {

            XEventClass eventClass;

            if (m_XDevice->classes[i].input_class == ButtonClass) {
                DeviceButtonPress(m_XDevice, m_buttonPressEvent, eventClass);
                m_eventClassList.append(eventClass);

                DeviceButtonRelease(m_XDevice, m_buttonReleaseEvent, eventClass);
                m_eventClassList.append(eventClass);
            }
            else
            if (m_XDevice->classes[i].input_class == ValuatorClass) {
                DeviceMotionNotify(m_XDevice, m_motionNotifyEvent, eventClass);
                m_eventClassList.append(eventClass);
            }
            else
            if (m_XDevice->classes[i].input_class == ProximityClass) {
                ProximityIn(m_XDevice, m_proximityInEvent, eventClass);
                m_eventClassList.append(eventClass);

                ProximityOut(m_XDevice, m_proximityOutEvent, eventClass);
                m_eventClassList.append(eventClass);
            }
        }

        // Note: We don't XCloseXDevice() since Qt will have already opened
        // it, and only one XCloseDevice() call closes it for all opens.
    }

    if (m_buttonPressEvent == -1 || m_buttonReleaseEvent == -1 || m_motionNotifyEvent == -1) {
        m_mightBeTabletDevice = false;
    }
}

bool KisCanvasWidget::X11TabletDevice::needsFindingActiveByProximity() const
{
    // Devices that Qt is aware of will generate QTabletEvents which the view
    // can use to determine if the device is active. We only need to check
    // using proximity for other devices which do not generate QTabletEvents.
    if (m_inputDevice == KisInputDevice::stylus() || m_inputDevice == KisInputDevice::eraser()
        || m_inputDevice == KisInputDevice::puck()) {
        return false;
    } else {
        return true;
    }
}

void KisCanvasWidget::X11TabletDevice::setEnabled(bool enabled)
{
    m_enabled = enabled;
}

bool KisCanvasWidget::X11TabletDevice::enabled() const
{
    return m_enabled;
}

qint32 KisCanvasWidget::X11TabletDevice::numAxes() const
{
    return m_axisInfo.count();
}

void KisCanvasWidget::X11TabletDevice::setXAxis(qint32 axis)
{
    m_xAxis = axis;
}

void KisCanvasWidget::X11TabletDevice::setYAxis(qint32 axis)
{
    m_yAxis = axis;
}

void KisCanvasWidget::X11TabletDevice::setPressureAxis(qint32 axis)
{
    m_pressureAxis = axis;
}

void KisCanvasWidget::X11TabletDevice::setXTiltAxis(qint32 axis)
{
    m_xTiltAxis = axis;
}

void KisCanvasWidget::X11TabletDevice::setYTiltAxis(qint32 axis)
{
    m_yTiltAxis = axis;
}

void KisCanvasWidget::X11TabletDevice::setWheelAxis(qint32 axis)
{
    m_wheelAxis = axis;
}

void KisCanvasWidget::X11TabletDevice::setToolIDAxis(qint32 axis)
{
    m_toolIDAxis = axis;
}

void KisCanvasWidget::X11TabletDevice::setSerialNumberAxis(qint32 axis)
{
    m_serialNumberAxis = axis;
}

qint32 KisCanvasWidget::X11TabletDevice::xAxis() const
{
    return m_xAxis;
}

qint32 KisCanvasWidget::X11TabletDevice::yAxis() const
{
    return m_yAxis;
}

qint32 KisCanvasWidget::X11TabletDevice::pressureAxis() const
{
    return m_pressureAxis;
}

qint32 KisCanvasWidget::X11TabletDevice::xTiltAxis() const
{
    return m_xTiltAxis;
}

qint32 KisCanvasWidget::X11TabletDevice::yTiltAxis() const
{
    return m_yTiltAxis;
}

qint32 KisCanvasWidget::X11TabletDevice::wheelAxis() const
{
    return m_wheelAxis;
}

qint32 KisCanvasWidget::X11TabletDevice::toolIDAxis() const
{
    return m_toolIDAxis;
}

qint32 KisCanvasWidget::X11TabletDevice::serialNumberAxis() const
{
    return m_serialNumberAxis;
}

void KisCanvasWidget::X11TabletDevice::readSettingsFromConfig()
{
    KisConfig cfg;

    m_enabled = cfg.tabletDeviceEnabled(m_name);

    m_xAxis = cfg.tabletDeviceAxis(m_name, "XAxis", DefaultAxis);
    m_yAxis = cfg.tabletDeviceAxis(m_name, "YAxis", DefaultAxis);
    m_pressureAxis = cfg.tabletDeviceAxis(m_name, "PressureAxis", DefaultAxis);
    m_xTiltAxis = cfg.tabletDeviceAxis(m_name, "XTiltAxis", DefaultAxis);
    m_yTiltAxis = cfg.tabletDeviceAxis(m_name, "YTiltAxis", DefaultAxis);
    m_wheelAxis = cfg.tabletDeviceAxis(m_name, "WheelAxis", DefaultAxis);
    m_toolIDAxis = cfg.tabletDeviceAxis(m_name, "ToolIDAxis", DefaultAxis);
    m_serialNumberAxis = cfg.tabletDeviceAxis(m_name, "SerialNumberAxis", DefaultAxis);

    if (!m_enabled && m_xAxis == DefaultAxis && m_yAxis == DefaultAxis && m_pressureAxis == DefaultAxis &&
         m_xTiltAxis == DefaultAxis && m_yTiltAxis == DefaultAxis && m_wheelAxis == DefaultAxis &&
         m_toolIDAxis == DefaultAxis && m_serialNumberAxis == DefaultAxis) {
        // This is the first time this device has been seen. Set up default values, assuming
        // it's a Wacom pad.
        m_xAxis = 0;
        m_yAxis = 1;
        m_pressureAxis = 2;

        if (m_axisInfo.count() >= 4) {
            m_xTiltAxis = 3;
        } else {
            m_xTiltAxis = NoAxis;
        }

        if (m_axisInfo.count() >= 5) {
            m_yTiltAxis = 4;
        } else {
            m_yTiltAxis = NoAxis;
        }

        if (m_axisInfo.count() >= 6) {
            m_wheelAxis = 5;
        } else {
            m_wheelAxis = NoAxis;
        }

        // Available since driver version 0.7.2.
        if (m_axisInfo.count() >= 7) {
            m_toolIDAxis = 6;
        } else {
            m_toolIDAxis = NoAxis;
        }

        if (m_axisInfo.count() >= 8) {
            m_serialNumberAxis = 7;
        } else {
            m_serialNumberAxis = NoAxis;
        }
    }
}

void KisCanvasWidget::X11TabletDevice::writeSettingsToConfig()
{
    KisConfig cfg;

    cfg.setTabletDeviceEnabled(m_name, m_enabled);

    cfg.setTabletDeviceAxis(m_name, "XAxis", m_xAxis);
    cfg.setTabletDeviceAxis(m_name, "YAxis", m_yAxis);
    cfg.setTabletDeviceAxis(m_name, "PressureAxis", m_pressureAxis);
    cfg.setTabletDeviceAxis(m_name, "XTiltAxis", m_xTiltAxis);
    cfg.setTabletDeviceAxis(m_name, "YTiltAxis", m_yTiltAxis);
    cfg.setTabletDeviceAxis(m_name, "WheelAxis", m_wheelAxis);
    cfg.setTabletDeviceAxis(m_name, "ToolIDAxis", m_toolIDAxis);
    cfg.setTabletDeviceAxis(m_name, "SerialNumberAxis", m_serialNumberAxis);
}

void KisCanvasWidget::X11TabletDevice::enableEvents(QWidget *widget) const
{
    if (!m_eventClassList.isEmpty()) {
        int result = XSelectExtensionEvent(QX11Info::display(), widget->handle(),
                                           const_cast<XEventClass*>(&m_eventClassList[0]),
                                           m_eventClassList.count());

        if (result != Success) {
            kDebug(41001) << "Failed to select extension events for " << m_name << endl;
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

KisCanvasWidget::X11TabletDevice::State::State(const KisPoint& pos, double pressure, const KisVector2D& tilt, double wheel,
                                               quint32 toolID, quint32 serialNumber)
    : m_pos(pos),
      m_pressure(pressure),
      m_tilt(tilt),
      m_wheel(wheel),
      m_toolID(toolID),
      m_serialNumber(serialNumber)
{
}

KisCanvasWidget::X11TabletDevice::State KisCanvasWidget::X11TabletDevice::translateAxisData(const int *axisData) const
{
    KisPoint pos(0, 0);

    if (m_xAxis != NoAxis && m_yAxis != NoAxis) {
        pos = KisPoint(translateAxisValue(axisData[m_xAxis], m_axisInfo[m_xAxis]),
                       translateAxisValue(axisData[m_yAxis], m_axisInfo[m_yAxis]));
    }

    double pressure = PRESSURE_DEFAULT;

    if (m_pressureAxis != NoAxis) {
        pressure = translateAxisValue(axisData[m_pressureAxis], m_axisInfo[m_pressureAxis]);
    }

    KisVector2D tilt = KisVector2D(0, 0);
    quint32 toolID = 0;
    quint32 serialNumber = 0;

    if (m_xTiltAxis != NoAxis) {
        // Latest wacom driver returns the tool id and serial number in
        // the upper 16 bits of the x and y tilts and wheel.
        int xTiltAxisValue = (qint16)(axisData[m_xTiltAxis] & 0xffff);
        toolID = ((quint32)axisData[m_xTiltAxis] >> 16) & 0xffff;

        tilt.setX(translateAxisValue(xTiltAxisValue, m_axisInfo[m_xTiltAxis]));
    }

    if (m_yTiltAxis != NoAxis) {
        int yTiltAxisValue = (qint16)(axisData[m_yTiltAxis] & 0xffff);
        serialNumber = (quint32)axisData[m_yTiltAxis] & 0xffff0000;

        tilt.setY(translateAxisValue(yTiltAxisValue, m_axisInfo[m_yTiltAxis]));
    }

    double wheel = 0;

    if (m_wheelAxis != NoAxis) {
        int wheelAxisValue = (qint16)(axisData[m_wheelAxis] & 0xffff);
        serialNumber |= ((quint32)axisData[m_wheelAxis] >> 16) & 0xffff;

        wheel = translateAxisValue(wheelAxisValue, m_axisInfo[m_wheelAxis]);
    }

    //QString ids;
    //ids.sprintf("Tool ID: %8x Serial Number: %8x", toolID, serialNumber);

    return State(pos, pressure, tilt, wheel, toolID, serialNumber);
}

KisCanvasWidget::X11XIDTabletDeviceMap& KisCanvasWidget::tabletDeviceMap()
{
    return X11TabletDeviceMap;
}

void KisCanvasWidget::selectTabletDeviceEvents(QWidget *widget)
{
    for (X11XIDTabletDeviceMap::const_iterator it = X11TabletDeviceMap.begin(); it != X11TabletDeviceMap.end(); ++it) {

        const X11TabletDevice& device = (*it).second;

        if (device.enabled()) {
            device.enableEvents(widget);
        }
    }
}

#endif // EXTENDED_X11_TABLET_SUPPORT

bool KisCanvasWidget::x11Event(XEvent *event, Display *x11Display, WId winId, QPoint widgetOriginPos)
{
    if (event->type == MotionNotify) {
        // Mouse move
        if (!m_enableMoveEventCompressionHint) {

            XMotionEvent motion = event->xmotion;
            QPoint globalPos(motion.x_root, motion.y_root);

            if (globalPos.x() != m_lastRootX || globalPos.y() != m_lastRootY) {

                Qt::KeyboardModifiers modifiers = translateX11KeyboardModifiers(motion.state);
                Qt::MouseButtons buttons = translateX11MouseButtons(motion.state);
                QPoint pos(motion.x, motion.y);
                QMouseEvent e(QEvent::MouseMove, pos, globalPos, Qt::NoButton, buttons, modifiers);

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
    if ((event->type == X11DeviceMotionNotifyEvent || event->type == X11DeviceButtonPressEvent || event->type == X11DeviceButtonReleaseEvent)
        && QApplication::activeModalWidget() == 0) {
        // Tablet event.
        int deviceId;
        const int *axisData;
        Qt::MouseButton button;
        Qt::KeyboardModifiers modifiers;
        Qt::MouseButtons buttons;

        if (event->type == X11DeviceMotionNotifyEvent) {
            // Tablet move
            const XDeviceMotionEvent *motion = reinterpret_cast<const XDeviceMotionEvent *>(event);
            XEvent mouseEvent;

            // Look for an accompanying core event.
            if (XCheckTypedWindowEvent(x11Display, winId, MotionNotify, &mouseEvent)) {
                if (motion->time == mouseEvent.xmotion.time) {
                    // Do nothing
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
                    }
                }
            }

            deviceId = motion->deviceid;
            axisData = motion->axis_data;
            button = Qt::NoButton;
            buttons = translateX11MouseButtons(motion->state);
            modifiers = translateX11KeyboardModifiers(motion->state);
        }
        else
        if (event->type == X11DeviceButtonPressEvent) {
            // Tablet button press
            const XDeviceButtonPressedEvent *buttonPressed = reinterpret_cast<const XDeviceButtonPressedEvent *>(event);
            deviceId = buttonPressed->deviceid;
            axisData = buttonPressed->axis_data;
            button = translateX11Button(buttonPressed->button);
            buttons = translateX11MouseButtons(buttonPressed->state);
            modifiers = translateX11KeyboardModifiers(buttonPressed->state);

            if (QApplication::activePopupWidget() == 0) {
                XEvent mouseEvent;

                // Look for and swallow an accompanying core event, but only if there's
                // no active popup, as that needs to see it.
                if (XCheckTypedWindowEvent(x11Display, winId, ButtonPress, &mouseEvent)) {
                    if (buttonPressed->time == mouseEvent.xbutton.time) {
                        // Do nothing
                    }
                    else {
                        XPutBackEvent(x11Display, &mouseEvent);
                    }
                }
            }
        }
        else {
            // Tablet button release
            const XDeviceButtonReleasedEvent *buttonReleased = reinterpret_cast<const XDeviceButtonReleasedEvent *>(event);
            deviceId = buttonReleased->deviceid;
            axisData = buttonReleased->axis_data;
            button = translateX11Button(buttonReleased->button);
            buttons = translateX11MouseButtons(buttonReleased->state);
            modifiers = translateX11KeyboardModifiers(buttonReleased->state);

            if (QApplication::activePopupWidget() == 0) {
                XEvent mouseEvent;

                // Look for and swallow an accompanying core event, but only if there's
                // no active popup, as that needs to see it.
                if (XCheckTypedWindowEvent(x11Display, winId, ButtonRelease, &mouseEvent)) {
                    if (buttonReleased->time == mouseEvent.xbutton.time) {
                        // Do nothing
                    }
                    else {
                        XPutBackEvent(x11Display, &mouseEvent);
                    }
                }
            }
        }

        X11XIDTabletDeviceMap::const_iterator it = X11TabletDeviceMap.find(deviceId);

        if (it != X11TabletDeviceMap.end()) {

            const X11TabletDevice& tabletDevice = (*it).second;

            if (tabletDevice.enabled()) {
                X11TabletDevice::State deviceState = tabletDevice.translateAxisData(axisData);

                // Map normalised position coordinates to screen coordinates
                QDesktopWidget *desktop = QApplication::desktop();
                KisPoint globalPos(deviceState.pos().x() * desktop->width(), deviceState.pos().y() * desktop->height());
                // Convert screen coordinates to widget coordinates
                KisPoint pos = globalPos - KoPoint( widgetOriginPos );

                // Map tilt to -60 - +60 degrees
                KisVector2D tilt(deviceState.tilt().x() * 60, deviceState.tilt().y() * 60);

                if (event->type == X11DeviceMotionNotifyEvent) {
                    KisMoveEvent e(tabletDevice.inputDevice(), pos, globalPos, deviceState.pressure(), tilt.x(), tilt.y(), buttons, modifiers);
                    translateTabletEvent(&e);
                }
                else
                if (event->type == X11DeviceButtonPressEvent) {
                    KisButtonPressEvent e(tabletDevice.inputDevice(), pos, globalPos, deviceState.pressure(), tilt.x(), tilt.y(), 
                                          button, buttons, modifiers);
                    translateTabletEvent(&e);
                }
                else {
                    KisButtonReleaseEvent e(tabletDevice.inputDevice(), pos, globalPos, deviceState.pressure(), tilt.x(), tilt.y(), 
                                            button, buttons, modifiers);
                    translateTabletEvent(&e);
                }
            }

            // Consume the event even if the device is disabled otherwise Qt will
            // process it and send a QTabletEvent.
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

#if defined(EXTENDED_X11_TABLET_SUPPORT)

KisInputDevice KisCanvasWidget::findActiveInputDevice()
{
    X11XIDTabletDeviceMap::const_iterator it;

    for (it = X11TabletDeviceMap.begin(); it != X11TabletDeviceMap.end(); ++it) {
        const X11TabletDevice& tabletDevice = (*it).second;

        if (!tabletDevice.needsFindingActiveByProximity()) {
            continue;
        }

        XDeviceState *deviceState = XQueryDeviceState(QX11Info::display(),
                                                      tabletDevice.xDevice());

        // If your the laptop sleeps, and you remove the mouse from the usb
        // port, then on wake-up Krita can crash because the above call will
        // return 0.
        if (!deviceState) continue;

        const XInputClass *inputClass = deviceState->data;
        bool deviceIsInProximity = false;

        for (int i = 0; i < deviceState->num_classes; i++) {

            if (inputClass->c_class == ValuatorClass) {

                const XValuatorState *valuatorState = reinterpret_cast<const XValuatorState *>(inputClass);

                if ((valuatorState->mode & ProximityState) == InProximity) {
                    deviceIsInProximity = true;
                    break;
                }
            }

            inputClass = reinterpret_cast<const XInputClass *>(reinterpret_cast<const char *>(inputClass) + inputClass->length);
        }

        XFreeDeviceState(deviceState);

        if (deviceIsInProximity && tabletDevice.enabled()) {
            return tabletDevice.inputDevice();
        }
    }

    return KisInputDevice::mouse();
}

#endif // EXTENDED_X11_TABLET_SUPPORT


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
    {
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

