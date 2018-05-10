/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qxcbconnection_xi2.h"

//#include "qxcbconnection.h"
//#include "qxcbkeyboard.h"
//#include "qxcbscreen.h"
//#include "qxcbwindow.h"
//#include "qtouchdevice.h"
//#include <qpa/qwindowsysteminterface.h>
#include <QDebug>
#include <QApplication>
#include <QDesktopWidget>

#ifdef XCB_USE_XINPUT2

#include <X11/extensions/XInput2.h>
#include <X11/extensions/XI2proto.h>

struct XInput2TouchDeviceData {
    XInput2TouchDeviceData()
        : xiDeviceInfo(0)
        , qtTouchDevice(0)
    {
    }
    XIDeviceInfo *xiDeviceInfo;
    QTouchDevice *qtTouchDevice;
    QHash<int, QWindowSystemInterface::TouchPoint> touchPoints;

    // Stuff that is relevant only for touchpads
    QHash<int, QPointF> pointPressedPosition; // in screen coordinates where each point was pressed
    QPointF firstPressedPosition;        // in screen coordinates where the first point was pressed
    QPointF firstPressedNormalPosition;  // device coordinates (0 to 1, 0 to 1) where the first point was pressed
    QSizeF size;                         // device size in mm
};

void QXcbConnection::initializeXInput2()
{
    // TODO Qt 6 (or perhaps earlier): remove these redundant env variables
    if (qEnvironmentVariableIsSet("KIS_QT_XCB_DEBUG_XINPUT"))
        const_cast<QLoggingCategory&>(lcQpaXInput()).setEnabled(QtDebugMsg, true);
    if (qEnvironmentVariableIsSet("KIS_QT_XCB_DEBUG_XINPUT_DEVICES"))
        const_cast<QLoggingCategory&>(lcQpaXInputDevices()).setEnabled(QtDebugMsg, true);
    Display *xDisplay = static_cast<Display *>(m_xlib_display);
    if (XQueryExtension(xDisplay, "XInputExtension", &m_xiOpCode, &m_xiEventBase, &m_xiErrorBase)) {
        int xiMajor = 2;
        m_xi2Minor = 2; // try 2.2 first, needed for TouchBegin/Update/End
        if (XIQueryVersion(xDisplay, &xiMajor, &m_xi2Minor) == BadRequest) {
            m_xi2Minor = 1; // for smooth scrolling 2.1 is enough
            if (XIQueryVersion(xDisplay, &xiMajor, &m_xi2Minor) == BadRequest) {
                m_xi2Minor = 0; // for tablet support 2.0 is enough
                m_xi2Enabled = XIQueryVersion(xDisplay, &xiMajor, &m_xi2Minor) != BadRequest;
            } else
                m_xi2Enabled = true;
        } else
            m_xi2Enabled = true;
        if (m_xi2Enabled) {
#ifdef XCB_USE_XINPUT22
            qCDebug(lcQpaXInputDevices, "XInput version %d.%d is available and Qt supports 2.2 or greater", xiMajor, m_xi2Minor);
#else
            qCDebug(lcQpaXInputDevices, "XInput version %d.%d is available and Qt supports 2.0", xiMajor, m_xi2Minor);
#endif
        }

        xi2SetupDevices();
    }
}

void QXcbConnection::xi2SetupDevices()
{
#ifndef QT_NO_TABLETEVENT
    m_tabletData.clear();
#endif
    m_scrollingDevices.clear();

    if (!m_xi2Enabled)
        return;

    Display *xDisplay = static_cast<Display *>(m_xlib_display);
    int deviceCount = 0;
    XIDeviceInfo *devices = XIQueryDevice(xDisplay, XIAllDevices, &deviceCount);
    for (int i = 0; i < deviceCount; ++i) {
        // Only non-master pointing devices are relevant here.
        if (devices[i].use != XISlavePointer)
            continue;
        qCDebug(lcQpaXInputDevices) << "input device "<< devices[i].name;
#ifndef QT_NO_TABLETEVENT
        TabletData tabletData;
#endif
        ScrollingDevice scrollingDevice;
        for (int c = 0; c < devices[i].num_classes; ++c) {
            switch (devices[i].classes[c]->type) {
            case XIValuatorClass: {
                XIValuatorClassInfo *vci = reinterpret_cast<XIValuatorClassInfo *>(devices[i].classes[c]);
                const int valuatorAtom = qatom(vci->label);
                qCDebug(lcQpaXInputDevices) << "   has valuator" << atomName(vci->label) << "recognized?" << (valuatorAtom < QXcbAtom::NAtoms);
#ifndef QT_NO_TABLETEVENT
                if (valuatorAtom < QXcbAtom::NAtoms) {
                    TabletData::ValuatorClassInfo info;
                    info.minVal = vci->min;
                    info.maxVal = vci->max;
                    info.number = vci->number;
                    tabletData.valuatorInfo[valuatorAtom] = info;
                }
#endif // QT_NO_TABLETEVENT
                if (valuatorAtom == QXcbAtom::RelHorizScroll || valuatorAtom == QXcbAtom::RelHorizWheel)
                    scrollingDevice.lastScrollPosition.setX(vci->value);
                else if (valuatorAtom == QXcbAtom::RelVertScroll || valuatorAtom == QXcbAtom::RelVertWheel)
                    scrollingDevice.lastScrollPosition.setY(vci->value);
                break;
            }
#ifdef XCB_USE_XINPUT21
            case XIScrollClass: {
                XIScrollClassInfo *sci = reinterpret_cast<XIScrollClassInfo *>(devices[i].classes[c]);
                if (sci->scroll_type == XIScrollTypeVertical) {
                    scrollingDevice.orientations |= Qt::Vertical;
                    scrollingDevice.verticalIndex = sci->number;
                    scrollingDevice.verticalIncrement = sci->increment;
                }
                else if (sci->scroll_type == XIScrollTypeHorizontal) {
                    scrollingDevice.orientations |= Qt::Horizontal;
                    scrollingDevice.horizontalIndex = sci->number;
                    scrollingDevice.horizontalIncrement = sci->increment;
                }
                break;
            }
            case XIButtonClass: {
                XIButtonClassInfo *bci = reinterpret_cast<XIButtonClassInfo *>(devices[i].classes[c]);
                if (bci->num_buttons >= 5) {
                    Atom label4 = bci->labels[3];
                    Atom label5 = bci->labels[4];
                    // Some drivers have no labels on the wheel buttons, some have no label on just one and some have no label on
                    // button 4 and the wrong one on button 5. So we just check that they are not labelled with unrelated buttons.
                    if ((!label4 || qatom(label4) == QXcbAtom::ButtonWheelUp || qatom(label4) == QXcbAtom::ButtonWheelDown) &&
                            (!label5 || qatom(label5) == QXcbAtom::ButtonWheelUp || qatom(label5) == QXcbAtom::ButtonWheelDown))
                        scrollingDevice.legacyOrientations |= Qt::Vertical;
                }
                if (bci->num_buttons >= 7) {
                    Atom label6 = bci->labels[5];
                    Atom label7 = bci->labels[6];
                    if ((!label6 || qatom(label6) == QXcbAtom::ButtonHorizWheelLeft) && (!label7 || qatom(label7) == QXcbAtom::ButtonHorizWheelRight))
                        scrollingDevice.legacyOrientations |= Qt::Horizontal;
                }
                qCDebug(lcQpaXInputDevices, "   has %d buttons", bci->num_buttons);
                break;
            }
#endif
            case XIKeyClass:
                qCDebug(lcQpaXInputDevices) << "   it's a keyboard";
                break;
#ifdef XCB_USE_XINPUT22
            case XITouchClass:
                // will be handled in deviceForId()
                break;
#endif
            default:
                qCDebug(lcQpaXInputDevices) << "   has class" << devices[i].classes[c]->type;
                break;
            }
        }
        bool isTablet = false;
#ifndef QT_NO_TABLETEVENT
        // If we have found the valuators which we expect a tablet to have, it might be a tablet.
        if (tabletData.valuatorInfo.contains(QXcbAtom::AbsX) &&
                tabletData.valuatorInfo.contains(QXcbAtom::AbsY) &&
                tabletData.valuatorInfo.contains(QXcbAtom::AbsPressure))
            isTablet = true;

        // But we need to be careful not to take the touch and tablet-button devices as tablets.
        QByteArray name = QByteArray(devices[i].name).toLower();
        QString dbgType = QLatin1String("UNKNOWN");
        if (name.contains("eraser")) {
            isTablet = true;
            tabletData.pointerType = QTabletEvent::Eraser;
            dbgType = QLatin1String("eraser");
        } else if (name.contains("cursor")) {
            isTablet = true;
            tabletData.pointerType = QTabletEvent::Cursor;
            dbgType = QLatin1String("cursor");
        } else if ((name.contains("pen") || name.contains("stylus")) && isTablet) {
            tabletData.pointerType = QTabletEvent::Pen;
            dbgType = QLatin1String("pen");
        } else if (name.contains("wacom") && isTablet && !name.contains("touch")) {
            // combined device (evdev) rather than separate pen/eraser (wacom driver)
            tabletData.pointerType = QTabletEvent::Pen;
            dbgType = QLatin1String("pen");
        } else if (name.contains("aiptek") /* && device == QXcbAtom::KEYBOARD */) {
            // some "Genius" tablets
            isTablet = true;
            tabletData.pointerType = QTabletEvent::Pen;
            dbgType = QLatin1String("pen");
        } else if (name.contains("waltop") && name.contains("tablet")) {
            // other "Genius" tablets
            // WALTOP International Corp. Slim Tablet
            isTablet = true;
            tabletData.pointerType = QTabletEvent::Pen;
            dbgType = QLatin1String("pen");
        } else if (name.contains("uc-logic")) {
            isTablet = true;
            tabletData.pointerType = QTabletEvent::Pen;
            dbgType = QLatin1String("pen");
        } else if (name.contains("ugee")) {
                isTablet = true;
                tabletData.pointerType = QTabletEvent::Pen;
                dbgType = QLatin1String("pen");
        } else {
            isTablet = false;
        }

        if (isTablet) {
            tabletData.deviceId = devices[i].deviceid;
            m_tabletData.append(tabletData);
            qCDebug(lcQpaXInputDevices) << "   it's a tablet with pointer type" << dbgType;
        }
#endif // QT_NO_TABLETEVENT

#ifdef XCB_USE_XINPUT21
        if (scrollingDevice.orientations || scrollingDevice.legacyOrientations) {
            scrollingDevice.deviceId = devices[i].deviceid;
            // Only use legacy wheel button events when we don't have real scroll valuators.
            scrollingDevice.legacyOrientations &= ~scrollingDevice.orientations;
            m_scrollingDevices.insert(scrollingDevice.deviceId, scrollingDevice);
            qCDebug(lcQpaXInputDevices) << "   it's a scrolling device";
        }
#endif

        if (!isTablet) {
            // touchDeviceForId populates XInput2DeviceData the first time it is called
            // with a new deviceId. On subsequent calls it will return the cached object.
            XInput2TouchDeviceData *dev = touchDeviceForId(devices[i].deviceid);
            if (dev && lcQpaXInputDevices().isDebugEnabled()) {
                if (dev->qtTouchDevice->type() == QTouchDevice::TouchScreen)
                    qCDebug(lcQpaXInputDevices, "   it's a touchscreen with type %d capabilities 0x%X max touch points %d",
                            dev->qtTouchDevice->type(), (unsigned int)dev->qtTouchDevice->capabilities(),
                            dev->qtTouchDevice->maximumTouchPoints());
                else if (dev->qtTouchDevice->type() == QTouchDevice::TouchPad)
                    qCDebug(lcQpaXInputDevices, "   it's a touchpad with type %d capabilities 0x%X max touch points %d size %f x %f",
                            dev->qtTouchDevice->type(), (unsigned int)dev->qtTouchDevice->capabilities(),
                            dev->qtTouchDevice->maximumTouchPoints(),
                            dev->size.width(), dev->size.height());
            }
        }
    }
    XIFreeDeviceInfo(devices);
}

void QXcbConnection::finalizeXInput2()
{
    foreach (XInput2TouchDeviceData *dev, m_touchDevices) {
        if (dev->xiDeviceInfo)
            XIFreeDeviceInfo(dev->xiDeviceInfo);
        delete dev->qtTouchDevice;
        delete dev;
    }
}

void QXcbConnection::xi2Select(xcb_window_t window)
{
    if (!m_xi2Enabled)
        return;

    Display *xDisplay = static_cast<Display *>(m_xlib_display);
    unsigned int bitMask = 0;
    unsigned char *xiBitMask = reinterpret_cast<unsigned char *>(&bitMask);

#ifdef XCB_USE_XINPUT22
    if (isAtLeastXI22()) {
        bitMask |= XI_TouchBeginMask;
        bitMask |= XI_TouchUpdateMask;
        bitMask |= XI_TouchEndMask;
        bitMask |= XI_PropertyEventMask; // for tablets
        if (xi2MouseEvents()) {
            // We want both mouse and touch through XI2 if touch is supported (>= 2.2).
            // The plain xcb press and motion events will not be delivered after this.
            bitMask |= XI_ButtonPressMask;
            bitMask |= XI_ButtonReleaseMask;
            bitMask |= XI_MotionMask;
            qCDebug(lcQpaXInput, "XInput 2.2: Selecting press/release/motion events in addition to touch");
        }
        XIEventMask mask;
        mask.mask_len = sizeof(bitMask);
        mask.mask = xiBitMask;
        // When xi2MouseEvents() is true (the default), pointer emulation for touch and tablet
        // events will get disabled. This is preferable for touch, as Qt Quick handles touch events
        // directly while for others QtGui synthesizes mouse events, not so much for tablets. For
        // the latter we will synthesize the events ourselves.
        mask.deviceid = XIAllMasterDevices;
        Status result = XISelectEvents(xDisplay, window, &mask, 1);
        if (result != Success)
            qCDebug(lcQpaXInput, "XInput 2.2: failed to select pointer/touch events, window %x, result %d", window, result);
    }
#endif // XCB_USE_XINPUT22

    const bool pointerSelected = isAtLeastXI22() && xi2MouseEvents();
    QSet<int> tabletDevices;
#ifndef QT_NO_TABLETEVENT
    if (!m_tabletData.isEmpty()) {
        unsigned int tabletBitMask;
        unsigned char *xiTabletBitMask = reinterpret_cast<unsigned char *>(&tabletBitMask);
        QVector<XIEventMask> xiEventMask(m_tabletData.count());
        tabletBitMask = XI_PropertyEventMask;
        if (!pointerSelected)
            tabletBitMask |= XI_ButtonPressMask | XI_ButtonReleaseMask | XI_MotionMask;
        for (int i = 0; i < m_tabletData.count(); ++i) {
            int deviceId = m_tabletData.at(i).deviceId;
            tabletDevices.insert(deviceId);
            xiEventMask[i].deviceid = deviceId;
            xiEventMask[i].mask_len = sizeof(tabletBitMask);
            xiEventMask[i].mask = xiTabletBitMask;
        }
        XISelectEvents(xDisplay, window, xiEventMask.data(), m_tabletData.count());
    }
#endif // QT_NO_TABLETEVENT

#ifdef XCB_USE_XINPUT21
    // Enable each scroll device
    if (!m_scrollingDevices.isEmpty() && !pointerSelected) {
        // Only when XI2 mouse events are not enabled, otherwise motion and release are selected already.
        QVector<XIEventMask> xiEventMask(m_scrollingDevices.size());
        unsigned int scrollBitMask;
        unsigned char *xiScrollBitMask = reinterpret_cast<unsigned char *>(&scrollBitMask);

        scrollBitMask = XI_MotionMask;
        scrollBitMask |= XI_ButtonReleaseMask;
        int i=0;
        Q_FOREACH (const ScrollingDevice& scrollingDevice, m_scrollingDevices) {
            if (tabletDevices.contains(scrollingDevice.deviceId))
                continue; // All necessary events are already captured.
            xiEventMask[i].deviceid = scrollingDevice.deviceId;
            xiEventMask[i].mask_len = sizeof(scrollBitMask);
            xiEventMask[i].mask = xiScrollBitMask;
            i++;
        }
        XISelectEvents(xDisplay, window, xiEventMask.data(), i);
    }
#else
    Q_UNUSED(xiBitMask);
#endif

    {
        // Listen for hotplug events
        XIEventMask xiEventMask;
        bitMask = XI_HierarchyChangedMask;
        bitMask |= XI_DeviceChangedMask;
        xiEventMask.deviceid = XIAllDevices;
        xiEventMask.mask_len = sizeof(bitMask);
        xiEventMask.mask = xiBitMask;
        XISelectEvents(xDisplay, window, &xiEventMask, 1);
    }
}

XInput2TouchDeviceData *QXcbConnection::touchDeviceForId(int id)
{
    XInput2TouchDeviceData *dev = 0;
    QHash<int, XInput2TouchDeviceData*>::const_iterator devIt = m_touchDevices.constFind(id);
    if (devIt != m_touchDevices.cend()) {
        dev = devIt.value();
    } else {
        int nrDevices = 0;
        QTouchDevice::Capabilities caps = 0;
        dev = new XInput2TouchDeviceData;
        dev->xiDeviceInfo = XIQueryDevice(static_cast<Display *>(m_xlib_display), id, &nrDevices);
        if (nrDevices <= 0) {
            delete dev;
            return 0;
        }
        int type = -1;
        int maxTouchPoints = 1;
        bool hasRelativeCoords = false;
        for (int i = 0; i < dev->xiDeviceInfo->num_classes; ++i) {
            XIAnyClassInfo *classinfo = dev->xiDeviceInfo->classes[i];
            switch (classinfo->type) {
#ifdef XCB_USE_XINPUT22
            case XITouchClass: {
                XITouchClassInfo *tci = reinterpret_cast<XITouchClassInfo *>(classinfo);
                maxTouchPoints = tci->num_touches;
                qCDebug(lcQpaXInputDevices, "   has touch class with mode %d", tci->mode);
                switch (tci->mode) {
                case XIDependentTouch:
                    type = QTouchDevice::TouchPad;
                    break;
                case XIDirectTouch:
                    type = QTouchDevice::TouchScreen;
                    break;
                }
                break;
            }
#endif // XCB_USE_XINPUT22
            case XIValuatorClass: {
                XIValuatorClassInfo *vci = reinterpret_cast<XIValuatorClassInfo *>(classinfo);
                if (vci->label == atom(QXcbAtom::AbsMTPositionX))
                    caps |= QTouchDevice::Position | QTouchDevice::NormalizedPosition;
                else if (vci->label == atom(QXcbAtom::AbsMTTouchMajor))
                    caps |= QTouchDevice::Area;
                else if (vci->label == atom(QXcbAtom::AbsMTPressure) || vci->label == atom(QXcbAtom::AbsPressure))
                    caps |= QTouchDevice::Pressure;
                else if (vci->label == atom(QXcbAtom::RelX)) {
                    hasRelativeCoords = true;
                    dev->size.setWidth((vci->max - vci->min) * 1000.0 / vci->resolution);
                } else if (vci->label == atom(QXcbAtom::RelY)) {
                    hasRelativeCoords = true;
                    dev->size.setHeight((vci->max - vci->min) * 1000.0 / vci->resolution);
                } else if (vci->label == atom(QXcbAtom::AbsX)) {
                    dev->size.setHeight((vci->max - vci->min) * 1000.0 / vci->resolution);
                } else if (vci->label == atom(QXcbAtom::AbsY)) {
                    dev->size.setWidth((vci->max - vci->min) * 1000.0 / vci->resolution);
                }
                break;
            }
            default:
                break;
            }
        }
        if (type < 0 && caps && hasRelativeCoords) {
            type = QTouchDevice::TouchPad;
            if (dev->size.width() < 10 || dev->size.height() < 10 ||
                    dev->size.width() > 10000 || dev->size.height() > 10000)
                dev->size = QSizeF(130, 110);
        }
        // WARNING: Krita edit
        //        if (!isAtLeastXI22() || type == QTouchDevice::TouchPad)
        //            caps |= QTouchDevice::MouseEmulation;

        if (type >= QTouchDevice::TouchScreen && type <= QTouchDevice::TouchPad) {
            dev->qtTouchDevice = new QTouchDevice;
            dev->qtTouchDevice->setName(QString::fromUtf8(dev->xiDeviceInfo->name));
            dev->qtTouchDevice->setType((QTouchDevice::DeviceType)type);
            dev->qtTouchDevice->setCapabilities(caps);
            dev->qtTouchDevice->setMaximumTouchPoints(maxTouchPoints);
            if (caps != 0)
                QWindowSystemInterface::registerTouchDevice(dev->qtTouchDevice);
            m_touchDevices[id] = dev;
        } else {
            XIFreeDeviceInfo(dev->xiDeviceInfo);
            delete dev;
            dev = 0;
        }
    }
    return dev;
}

#if defined(XCB_USE_XINPUT21) || !defined(QT_NO_TABLETEVENT)
static inline qreal fixed1616ToReal(FP1616 val)
{
    return qreal(val) / 0x10000;
}
#endif // defined(XCB_USE_XINPUT21) || !defined(QT_NO_TABLETEVENT)

bool QXcbConnection::xi2HandleEvent(xcb_ge_event_t *event)
{
    if (xi2PrepareXIGenericDeviceEvent(event, m_xiOpCode)) {
        xXIGenericDeviceEvent *xiEvent = reinterpret_cast<xXIGenericDeviceEvent *>(event);
        int sourceDeviceId = xiEvent->deviceid; // may be the master id
        xXIDeviceEvent *xiDeviceEvent = 0;
        QWindow *window = 0;

        switch (xiEvent->evtype) {
        case XI_ButtonPress:
        case XI_ButtonRelease:
        case XI_Motion:
#ifdef XCB_USE_XINPUT22
        case XI_TouchBegin:
        case XI_TouchUpdate:
        case XI_TouchEnd:
#endif
        {
            xiDeviceEvent = reinterpret_cast<xXIDeviceEvent *>(event);
            window = windowFromId(xiDeviceEvent->event);
            sourceDeviceId = xiDeviceEvent->sourceid; // use the actual device id instead of the master
            break;
        }
        case XI_HierarchyChanged:
            xi2HandleHierachyEvent(xiEvent);
            return false;
        case XI_DeviceChanged:
            xi2HandleDeviceChangedEvent(xiEvent);
            return false;
        default:
            break;
        }

#ifndef QT_NO_TABLETEVENT
        for (int i = 0; i < m_tabletData.count(); ++i) {
            if (m_tabletData.at(i).deviceId == sourceDeviceId) {
                if (xi2HandleTabletEvent(xiEvent, &m_tabletData[i], window)) {
                    return true;
                }
            }
        }
#endif // QT_NO_TABLETEVENT

#ifdef XCB_USE_XINPUT21
        QHash<int, ScrollingDevice>::iterator device = m_scrollingDevices.find(sourceDeviceId);
        if (device != m_scrollingDevices.end())
            xi2HandleScrollEvent(xiEvent, device.value());
        // Removed from Qt...
#endif // XCB_USE_XINPUT21

#ifdef XCB_USE_XINPUT22
        // Removed from Qt...
#endif // XCB_USE_XINPUT22
    }

    return false;
}

#ifdef XCB_USE_XINPUT22

bool QXcbConnection::xi2SetMouseGrabEnabled(xcb_window_t w, bool grab)
{
    if (grab && !canGrab())
        return false;

    int num_devices = 0;
    Display *xDisplay = static_cast<Display *>(xlib_display());
    XIDeviceInfo *info = XIQueryDevice(xDisplay, XIAllMasterDevices, &num_devices);
    if (!info)
        return false;

    XIEventMask evmask;
    unsigned char mask[XIMaskLen(XI_LASTEVENT)];
    evmask.mask = mask;
    evmask.mask_len = sizeof(mask);
    memset(mask, 0, sizeof(mask));
    evmask.deviceid = XIAllMasterDevices;

    XISetMask(mask, XI_ButtonPress);
    XISetMask(mask, XI_ButtonRelease);
    XISetMask(mask, XI_Motion);
    XISetMask(mask, XI_TouchBegin);
    XISetMask(mask, XI_TouchUpdate);
    XISetMask(mask, XI_TouchEnd);

    bool grabbed = true;
    for (int i = 0; i < num_devices; i++) {
        int id = info[i].deviceid, n = 0;
        XIDeviceInfo *deviceInfo = XIQueryDevice(xDisplay, id, &n);
        if (deviceInfo) {
            const bool grabbable = deviceInfo->use != XIMasterKeyboard;
            XIFreeDeviceInfo(deviceInfo);
            if (!grabbable)
                continue;
        }
        if (!grab) {
            Status result = XIUngrabDevice(xDisplay, id, CurrentTime);
            if (result != Success) {
                grabbed = false;
                qCDebug(lcQpaXInput, "XInput 2.2: failed to ungrab events for device %d (result %d)", id, result);
            }
        } else {
            Status result = XIGrabDevice(xDisplay, id, w, CurrentTime, None, XIGrabModeAsync,
                                         XIGrabModeAsync, False, &evmask);
            if (result != Success) {
                grabbed = false;
                qCDebug(lcQpaXInput, "XInput 2.2: failed to grab events for device %d on window %x (result %d)", id, w, result);
            }
        }
    }

    XIFreeDeviceInfo(info);

    m_xiGrab = grabbed;

    return grabbed;
}
#endif // XCB_USE_XINPUT22

void QXcbConnection::xi2HandleHierachyEvent(void *event)
{
    xXIHierarchyEvent *xiEvent = reinterpret_cast<xXIHierarchyEvent *>(event);
    // We only care about hotplugged devices
    if (!(xiEvent->flags & (XISlaveRemoved | XISlaveAdded)))
        return;
    xi2SetupDevices();
    // Reselect events for all event-listening windows.
    Q_FOREACH (xcb_window_t window, m_windowMapper.keys()) {
        xi2Select(window);
    }
}

void QXcbConnection::xi2HandleDeviceChangedEvent(void *event)
{
    xXIDeviceChangedEvent *xiEvent = reinterpret_cast<xXIDeviceChangedEvent *>(event);

    // ### If a slave device changes (XIDeviceChange), we should probably run setup on it again.
    if (xiEvent->reason != XISlaveSwitch)
        return;

#ifdef XCB_USE_XINPUT21
    // This code handles broken scrolling device drivers that reset absolute positions
    // when they are made active. Whenever a new slave device is made active the
    // primary pointer sends a DeviceChanged event with XISlaveSwitch, and the new
    // active slave in sourceid.

    QHash<int, ScrollingDevice>::iterator device = m_scrollingDevices.find(xiEvent->sourceid);
    if (device == m_scrollingDevices.end())
        return;

    int nrDevices = 0;
    XIDeviceInfo* xiDeviceInfo = XIQueryDevice(static_cast<Display *>(m_xlib_display), xiEvent->sourceid, &nrDevices);
    if (nrDevices <= 0) {
        qCDebug(lcQpaXInputDevices, "scrolling device %d no longer present", xiEvent->sourceid);
        return;
    }
    updateScrollingDevice(*device, xiDeviceInfo->num_classes, xiDeviceInfo->classes);
    XIFreeDeviceInfo(xiDeviceInfo);
#endif
}

void QXcbConnection::updateScrollingDevice(ScrollingDevice &scrollingDevice, int num_classes, void *classInfo)
{
#ifdef XCB_USE_XINPUT21
    XIAnyClassInfo **classes = reinterpret_cast<XIAnyClassInfo**>(classInfo);
    QPointF lastScrollPosition;
    if (lcQpaXInput().isDebugEnabled())
        lastScrollPosition = scrollingDevice.lastScrollPosition;
    for (int c = 0; c < num_classes; ++c) {
        if (classes[c]->type == XIValuatorClass) {
            XIValuatorClassInfo *vci = reinterpret_cast<XIValuatorClassInfo *>(classes[c]);
            const int valuatorAtom = qatom(vci->label);
            if (valuatorAtom == QXcbAtom::RelHorizScroll || valuatorAtom == QXcbAtom::RelHorizWheel)
                scrollingDevice.lastScrollPosition.setX(vci->value);
            else if (valuatorAtom == QXcbAtom::RelVertScroll || valuatorAtom == QXcbAtom::RelVertWheel)
                scrollingDevice.lastScrollPosition.setY(vci->value);
        }
    }
    if (lcQpaXInput().isDebugEnabled() && lastScrollPosition != scrollingDevice.lastScrollPosition)
        qCDebug(lcQpaXInput, "scrolling device %d moved from (%f, %f) to (%f, %f)", scrollingDevice.deviceId,
                lastScrollPosition.x(), lastScrollPosition.y(),
                scrollingDevice.lastScrollPosition.x(),
                scrollingDevice.lastScrollPosition.y());
#else
    Q_UNUSED(scrollingDevice);
    Q_UNUSED(num_classes);
    Q_UNUSED(classInfo);
#endif
}

Qt::MouseButton QXcbConnection::xiToQtMouseButton(uint32_t b)
{
    switch (b) {
    case 1: return Qt::LeftButton;
    case 2: return Qt::MiddleButton;
    case 3: return Qt::RightButton;
        // 4-7 are for scrolling
    default: break;
    }
    if (b >= 8 && b <= Qt::MaxMouseButton)
        return static_cast<Qt::MouseButton>(Qt::BackButton << (b - 8));
    return Qt::NoButton;
}

Qt::MouseButtons QXcbConnection::xiToQtMouseButtons(xXIDeviceEvent *xiDeviceEvent)
{
    /**
     * WARNING: we haven't tested this method on different tablets. For basic tablets
     *          without any button remapping it seems to work. For more complex configurations
     *          I just don't know. Right now it is only safe to check if buttons is
     *          equal to Qt::NoButton!
     */


    int len = xiDeviceEvent->buttons_len;
    const uint32_t *buttons = reinterpret_cast<const quint32*>(&xiDeviceEvent[1]);

    Qt::MouseButtons qtbuttons = Qt::NoButton;

    const int numBits = len * 32;

    for (int i = 1; i < numBits; i++) {
        const int index = (i) / 32;
        const int offset = (i) % 32;

        const bool isActive = buttons[index] & (1 << offset);
        if (isActive) {
            qtbuttons |= xiToQtMouseButton(i);
        }
    }

    return qtbuttons;
}

static QTabletEvent::TabletDevice toolIdToTabletDevice(quint32 toolId) {
    // keep in sync with wacom_intuos_inout() in Linux kernel driver wacom_wac.c
    switch (toolId) {
    case 0xd12:
    case 0x912:
    case 0x112:
    case 0x913: /* Intuos3 Airbrush */
    case 0x91b: /* Intuos3 Airbrush Eraser */
    case 0x902: /* Intuos4/5 13HD/24HD Airbrush */
    case 0x90a: /* Intuos4/5 13HD/24HD Airbrush Eraser */
    case 0x100902: /* Intuos4/5 13HD/24HD Airbrush */
    case 0x10090a: /* Intuos4/5 13HD/24HD Airbrush Eraser */
        return QTabletEvent::Airbrush;
    case 0x007: /* Mouse 4D and 2D */
    case 0x09c:
    case 0x094:
        return QTabletEvent::FourDMouse;
    case 0x017: /* Intuos3 2D Mouse */
    case 0x806: /* Intuos4 Mouse */
    case 0x096: /* Lens cursor */
    case 0x097: /* Intuos3 Lens cursor */
    case 0x006: /* Intuos4 Lens cursor */
        return QTabletEvent::Puck;
    case 0x885:    /* Intuos3 Art Pen (Marker Pen) */
    case 0x100804: /* Intuos4/5 13HD/24HD Art Pen */
    case 0x10080c: /* Intuos4/5 13HD/24HD Art Pen Eraser */
        return QTabletEvent::RotationStylus;
    case 0:
        return QTabletEvent::NoDevice;
    }
    return QTabletEvent::Stylus;  // Safe default assumption if nonzero
}

#ifndef QT_NO_TABLETEVENT
bool QXcbConnection::xi2HandleTabletEvent(void *event, TabletData *tabletData, QWindow *window)
{
    bool handled = true;
    Display *xDisplay = static_cast<Display *>(m_xlib_display);
    xXIGenericDeviceEvent *xiEvent = static_cast<xXIGenericDeviceEvent *>(event);
    xXIDeviceEvent *xiDeviceEvent = reinterpret_cast<xXIDeviceEvent *>(xiEvent);


    /**
     * On some systems we can lose tablet button release event if the tablet
     * was "closing a popup window by clicking somewhere outside the app
     * window". It means that we get a tablet press event, but get absolutely
     * no tablet release event. That can cause quite a lot of troubles, so here we
     * check if reported button state is consistent and if not, just reset it.
     *
     * WARNING: We haven't tested xiToQtMouseButtons() functions on all the
     *          tablet models and configurations, so at the moment we rely only
     *          on its Qt::NoButton state. If people test it on custom tablet
     *          button configurations, we can just stop tracking
     *          tabletData->buttons and use this mapping instead.
     */

    if (xiEvent->evtype == XI_Motion ||
            xiEvent->evtype == XI_ButtonPress ||
            xiEvent->evtype == XI_ButtonRelease) {

        Qt::MouseButtons expectedButtons = xiToQtMouseButtons(xiDeviceEvent);

        if (expectedButtons != tabletData->buttons) {
            if (expectedButtons == Qt::NoButton) {
                tabletData->buttons = expectedButtons;
            } else {
                qWarning() << "===";
                qWarning() << "WARNING: Tracked tablet buttons are not consistent!";
                qWarning() << "        "  << ppVar(tabletData->buttons);
                qWarning() << "        "  << ppVar(expectedButtons);
            }
        }
    }

    switch (xiEvent->evtype) {
    case XI_ButtonPress: {
        Qt::MouseButton b = xiToQtMouseButton(xiDeviceEvent->detail);
        tabletData->buttons |= b;
        xi2ReportTabletEvent(*tabletData, xiEvent);
        break;
    }
    case XI_ButtonRelease: {
        Qt::MouseButton b = xiToQtMouseButton(xiDeviceEvent->detail);
        tabletData->buttons ^= b;
        xi2ReportTabletEvent(*tabletData, xiEvent);
        break;
    }
    case XI_Motion:
        xi2ReportTabletEvent(*tabletData, xiEvent);
        break;
    case XI_PropertyEvent: {
        // This is the wacom driver's way of reporting tool proximity.
        // The evdev driver doesn't do it this way.
        xXIPropertyEvent *ev = reinterpret_cast<xXIPropertyEvent *>(event);
        if (ev->what == XIPropertyModified) {
            if (ev->property == atom(QXcbAtom::WacomSerialIDs)) {
                enum WacomSerialIndex {
                    _WACSER_USB_ID = 0,
                    _WACSER_LAST_TOOL_SERIAL,
                    _WACSER_LAST_TOOL_ID,
                    _WACSER_TOOL_SERIAL,
                    _WACSER_TOOL_ID,
                    _WACSER_COUNT
                };
                Atom propType;
                int propFormat;
                unsigned long numItems, bytesAfter;
                unsigned char *data;
                if (XIGetProperty(xDisplay, tabletData->deviceId, ev->property, 0, 100,
                                  0, AnyPropertyType, &propType, &propFormat,
                                  &numItems, &bytesAfter, &data) == Success) {
                    if (propType == atom(QXcbAtom::INTEGER) && propFormat == 32 && numItems == _WACSER_COUNT) {
                        quint32 *ptr = reinterpret_cast<quint32 *>(data);
                        quint32 tool = ptr[_WACSER_TOOL_ID];
                        // Workaround for http://sourceforge.net/p/linuxwacom/bugs/246/
                        // e.g. on Thinkpad Helix, tool ID will be 0 and serial will be 1
                        if (!tool && ptr[_WACSER_TOOL_SERIAL])
                            tool = ptr[_WACSER_TOOL_SERIAL];

                        // The property change event informs us which tool is in proximity or which one left proximity.
                        if (tool) {
                            tabletData->inProximity = true;
                            tabletData->tool = toolIdToTabletDevice(tool);
                            tabletData->serialId = qint64(ptr[_WACSER_USB_ID]) << 32 | qint64(ptr[_WACSER_TOOL_SERIAL]);
                            QWindowSystemInterface::handleTabletEnterProximityEvent(tabletData->tool,
                                                                                    tabletData->pointerType,
                                                                                    tabletData->serialId);
                        } else {
                            tabletData->inProximity = false;
                            tabletData->tool = toolIdToTabletDevice(ptr[_WACSER_LAST_TOOL_ID]);
                            // Workaround for http://sourceforge.net/p/linuxwacom/bugs/246/
                            // e.g. on Thinkpad Helix, tool ID will be 0 and serial will be 1
                            if (!tabletData->tool)
                                tabletData->tool = toolIdToTabletDevice(ptr[_WACSER_LAST_TOOL_SERIAL]);
                            tabletData->serialId = qint64(ptr[_WACSER_USB_ID]) << 32 | qint64(ptr[_WACSER_LAST_TOOL_SERIAL]);
                            QWindowSystemInterface::handleTabletLeaveProximityEvent(tabletData->tool,
                                                                                    tabletData->pointerType,
                                                                                    tabletData->serialId);
                        }
                        // TODO maybe have a hash of tabletData->deviceId to device data so we can
                        // look up the tablet name here, and distinguish multiple tablets
                        qCDebug(lcQpaXInput, "XI2 proximity change on tablet %d (USB %x): last tool: %x id %x current tool: %x id %x TabletDevice %d",
                                tabletData->deviceId, ptr[_WACSER_USB_ID], ptr[_WACSER_LAST_TOOL_SERIAL], ptr[_WACSER_LAST_TOOL_ID],
                                ptr[_WACSER_TOOL_SERIAL], ptr[_WACSER_TOOL_ID], tabletData->tool);
                    }
                    XFree(data);
                }
            }
        }
        break;
    }
    default:
        handled = false;
        break;
    }

#ifdef XCB_USE_XINPUT22
    // Synthesize mouse events since otherwise there are no mouse events from
    // the pen on the XI 2.2+ path.
    if (xi2MouseEvents() && window) {
        // DK: I have no idea why this line was introduced in Qt5.5!
        //eventListener->handleXIMouseEvent(reinterpret_cast<xcb_ge_event_t *>(event));
    }
#else
    Q_UNUSED(eventListener);
#endif

    return handled;
}

inline qreal scaleOneValuator(qreal normValue, qreal screenMin, qreal screenSize)
{
    return screenMin + normValue * screenSize;
}

void QXcbConnection::xi2ReportTabletEvent(TabletData &tabletData, void *event)
{
    xXIDeviceEvent *ev = reinterpret_cast<xXIDeviceEvent *>(event);
    QWindow *window = windowFromId(ev->event);
    if (!window) return;

    const double scale = 65536.0;
    QPointF local(ev->event_x / scale, ev->event_y / scale);
    QPointF global(ev->root_x / scale, ev->root_y / scale);
    double pressure = 0, rotation = 0, tangentialPressure = 0;
    int xTilt = 0, yTilt = 0;

    QRect screenArea = qApp->desktop()->rect();

    for (QHash<int, TabletData::ValuatorClassInfo>::iterator it = tabletData.valuatorInfo.begin(),
         ite = tabletData.valuatorInfo.end(); it != ite; ++it) {
        int valuator = it.key();
        TabletData::ValuatorClassInfo &classInfo(it.value());
        xi2GetValuatorValueIfSet(event, classInfo.number, &classInfo.curVal);
        double normalizedValue = (classInfo.curVal - classInfo.minVal) / (classInfo.maxVal - classInfo.minVal);
        switch (valuator) {
        case QXcbAtom::AbsX: {
            const qreal value = scaleOneValuator(normalizedValue, screenArea.x(), screenArea.width());
            const qreal offset = local.x() - global.x();
            global.rx() = value;
            local.rx() = value + offset;
            break;
        }
        case QXcbAtom::AbsY: {
            qreal value = scaleOneValuator(normalizedValue, screenArea.y(), screenArea.height());
            qreal offset = local.y() - global.y();
            global.ry() = value;
            local.ry() = value + offset;
            break;
        }
        case QXcbAtom::AbsPressure:
            pressure = normalizedValue;
            break;
        case QXcbAtom::AbsTiltX:
            xTilt = classInfo.curVal;
            break;
        case QXcbAtom::AbsTiltY:
            yTilt = classInfo.curVal;
            break;
        case QXcbAtom::AbsWheel:
            switch (tabletData.tool) {
            case QTabletEvent::Airbrush:
                tangentialPressure = normalizedValue * 2.0 - 1.0; // Convert 0..1 range to -1..+1 range
                break;
            case QTabletEvent::RotationStylus:
                rotation = normalizedValue * 360.0 - 180.0; // Convert 0..1 range to -180..+180 degrees
                break;
            default:    // Other types of styli do not use this valuator
                break;
            }
            break;
        default:
            break;
        }
    }

    if (Q_UNLIKELY(lcQpaXInput().isDebugEnabled()))
        qCDebug(lcQpaXInput, "XI2 event on tablet %d with tool %d type %d seq %d detail %d pos %6.1f, %6.1f root pos %6.1f, %6.1f buttons 0x%x pressure %4.2lf tilt %d, %d rotation %6.2lf",
                tabletData.deviceId, tabletData.tool, ev->evtype, ev->sequenceNumber, ev->detail,
                fixed1616ToReal(ev->event_x), fixed1616ToReal(ev->event_y),
                fixed1616ToReal(ev->root_x), fixed1616ToReal(ev->root_y),
                (int)tabletData.buttons, pressure, xTilt, yTilt, rotation);

    Qt::KeyboardModifiers modifiers = QApplication::queryKeyboardModifiers();

    QWindowSystemInterface::handleTabletEvent(window, local, global,
                                              tabletData.tool, tabletData.pointerType,
                                              tabletData.buttons, pressure,
                                              xTilt, yTilt, tangentialPressure,
                                              rotation, 0, tabletData.serialId,
                                              modifiers);
}

void QXcbConnection::xi2HandleScrollEvent(void *event, ScrollingDevice &scrollingDevice)
{
#ifdef XCB_USE_XINPUT21
    xXIGenericDeviceEvent *xiEvent = reinterpret_cast<xXIGenericDeviceEvent *>(event);

    if (xiEvent->evtype == XI_Motion && scrollingDevice.orientations) {
        xXIDeviceEvent* xiDeviceEvent = reinterpret_cast<xXIDeviceEvent *>(event);
        if (QWindow *platformWindow = windowFromId(xiDeviceEvent->event)) {
            QPoint rawDelta;
            QPoint angleDelta;
            double value;
            if (scrollingDevice.orientations & Qt::Vertical) {
                if (xi2GetValuatorValueIfSet(xiDeviceEvent, scrollingDevice.verticalIndex, &value)) {
                    double delta = scrollingDevice.lastScrollPosition.y() - value;
                    scrollingDevice.lastScrollPosition.setY(value);
                    angleDelta.setY((delta / scrollingDevice.verticalIncrement) * 120);
                    // We do not set "pixel" delta if it is only measured in ticks.
                    if (scrollingDevice.verticalIncrement > 1)
                        rawDelta.setY(delta);
                }
            }
            if (scrollingDevice.orientations & Qt::Horizontal) {
                if (xi2GetValuatorValueIfSet(xiDeviceEvent, scrollingDevice.horizontalIndex, &value)) {
                    double delta = scrollingDevice.lastScrollPosition.x() - value;
                    scrollingDevice.lastScrollPosition.setX(value);
                    angleDelta.setX((delta / scrollingDevice.horizontalIncrement) * 120);
                    // We do not set "pixel" delta if it is only measured in ticks.
                    if (scrollingDevice.horizontalIncrement > 1)
                        rawDelta.setX(delta);
                }
            }
            if (!angleDelta.isNull()) {
                const int dpr = int(platformWindow->devicePixelRatio());
                QPoint local(fixed1616ToReal(xiDeviceEvent->event_x)/dpr, fixed1616ToReal(xiDeviceEvent->event_y)/dpr);
                QPoint global(fixed1616ToReal(xiDeviceEvent->root_x)/dpr, fixed1616ToReal(xiDeviceEvent->root_y)/dpr);
                Qt::KeyboardModifiers modifiers = QApplication::queryKeyboardModifiers();
                if (modifiers & Qt::AltModifier) {
                    std::swap(angleDelta.rx(), angleDelta.ry());
                    std::swap(rawDelta.rx(), rawDelta.ry());
                }
                QWindowSystemInterface::handleWheelEvent(platformWindow, xiEvent->time, local, global, rawDelta, angleDelta, modifiers);
            }
        }
    } else if (xiEvent->evtype == XI_ButtonRelease && scrollingDevice.legacyOrientations) {
        xXIDeviceEvent* xiDeviceEvent = reinterpret_cast<xXIDeviceEvent *>(event);
        if (QWindow *platformWindow = windowFromId(xiDeviceEvent->event)) {
            QPoint angleDelta;
            if (scrollingDevice.legacyOrientations & Qt::Vertical) {
                if (xiDeviceEvent->detail == 4)
                    angleDelta.setY(120);
                else if (xiDeviceEvent->detail == 5)
                    angleDelta.setY(-120);
            }
            if (scrollingDevice.legacyOrientations & Qt::Horizontal) {
                if (xiDeviceEvent->detail == 6)
                    angleDelta.setX(120);
                else if (xiDeviceEvent->detail == 7)
                    angleDelta.setX(-120);
            }
            if (!angleDelta.isNull()) {
                const int dpr = int(platformWindow->devicePixelRatio());
                QPoint local(fixed1616ToReal(xiDeviceEvent->event_x)/dpr, fixed1616ToReal(xiDeviceEvent->event_y)/dpr);
                QPoint global(fixed1616ToReal(xiDeviceEvent->root_x)/dpr, fixed1616ToReal(xiDeviceEvent->root_y)/dpr);
                Qt::KeyboardModifiers modifiers = QApplication::queryKeyboardModifiers();
                if (modifiers & Qt::AltModifier)
                    std::swap(angleDelta.rx(), angleDelta.ry());
                QWindowSystemInterface::handleWheelEvent(platformWindow, xiEvent->time, local, global, QPoint(), angleDelta, modifiers);
            }
        }
    }
#else
    Q_UNUSED(event);
    Q_UNUSED(scrollingDevice);
#endif // XCB_USE_XINPUT21
}

#endif // QT_NO_TABLETEVENT

#endif // XCB_USE_XINPUT2
