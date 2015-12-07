/*
 *  Copyright (c) 2013 Dmitry Kazakov <dimula73@gmail.com>
 *  Copyright (c) 2015 The Qt Company Ltd. http://www.qt.io/licensing/
 *  Copyright (c) 2015 Michael Abrahams <miabraha@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
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

#include "kis_xcb_connection.h"
#include "kis_config.h"
#include "kis_debug.h"
#include <input/kis_tablet_debugger.h>

#include <QX11Info>
#include <QApplication>
#include <QDesktopWidget>
#include <QScreen>
#include <QWindow>

// qtbase5-private-dev
// #include <qpa/qplatformscreen.h>
// #include <qpa/qwindowsysteminterface.h>

// Note: XInput 2.2 is required
#include <xcb/xcb.h>
#include <X11/extensions/XI2.h>
#include <X11/extensions/XInput.h>
#include <X11/extensions/XInput2.h>
#include <X11/extensions/XI2proto.h>

// Defined in kis_tablet_support_x11.cpp
extern bool kis_tabletChokeMouse;

KisXcbConnection::KisXcbConnection()
{
    // We will mix Xlib and XCB calls in this code.
    display = QX11Info::display();
    connection = QX11Info::connection();
    initializeAllAtoms();
    init_tablet();
}



KisXcbConnection::~KisXcbConnection()
{
    // From finalizeXInput2(). May be useful if we do touch support.
    // foreach (XInput2TouchDeviceData *dev, m_touchDevices) {
    //     if (dev->xiDeviceInfo)
    //         XIFreeDeviceInfo(dev->xiDeviceInfo);
    //     delete dev;
    // }
}


KisXcbAtom::Atom KisXcbConnection::kis_atom(xcb_atom_t xatom) const
{
    return static_cast<KisXcbAtom::Atom>(std::find(m_allAtoms, m_allAtoms + KisXcbAtom::NAtoms, xatom) - m_allAtoms);
}


void KisXcbConnection::initializeAllAtoms()
{
    const char *names[KisXcbAtom::NAtoms];
    const char *ptr = kis_xcb_atomnames;

    int i = 0;
    while (*ptr) {
        names[i++] = ptr;
        while (*ptr)
            ++ptr;
        ++ptr;
    }

    Q_ASSERT(i == KisXcbAtom::NPredefinedAtoms);
    Q_ASSERT(i == KisXcbAtom::NAtoms);

    xcb_intern_atom_cookie_t cookies[KisXcbAtom::NAtoms];

    for (i = 0; i < KisXcbAtom::NAtoms; ++i)
        cookies[i] = xcb_intern_atom(connection, false, strlen(names[i]), names[i]);

    for (i = 0; i < KisXcbAtom::NAtoms; ++i) {
        xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(connection, cookies[i], 0);
        m_allAtoms[i] = reply->atom;
        dbgInput << "Atom" << names[i] << "has value" << reply->atom;
        free(reply);
    }
}



void KisXcbConnection::init_tablet()
{
    // KisConfig cfg;
    // bool disableTouchOnCanvas = cfg.disableTouchOnCanvas();


    if (XQueryExtension(display, "XInputExtension",
                        &m_xiOpCode, &m_xiEventBase, &m_xiErrorBase)) {
        int xiMajor = 2;
        m_xi2Minor = 2; // try 2.2 first, needed for TouchBegin/Update/End
        if (XIQueryVersion(display, &xiMajor, &m_xi2Minor) == BadRequest) {
            errInput << "FATAL: XInput 2.2 not supported.";
        }
        xi2SetupDevices();
    }
}


void KisXcbConnection::xi2SetupDevices()
{
    int deviceCount = 0;
    XIDeviceInfo *devices = XIQueryDevice(display, XIAllDevices, &deviceCount);
    if (!devices)
        dbgInput << "QApplication: Failed to get list of devices";

    bool gotStylus, gotEraser;

    bool needCheckIfItIsReallyATablet;
    bool touchWacomTabletWorkaround;
    QTabletEvent::TabletDevice deviceType;
    m_tabletData.clear();


    for (int i = 0; i < deviceCount; ++i) {
        // Only non-master pointing devices are relevant here.
        if (devices[i].use != XISlavePointer)
            continue;
        dbgInput << "input device "<< devices[i].name;
        TabletData tabletData;
        // ScrollingDevice scrollingDevice;

        deviceType = QTabletEvent::NoDevice;
        gotStylus = false;
        gotEraser = false;
        needCheckIfItIsReallyATablet = false;
        touchWacomTabletWorkaround = false;  // XXX: Look into this

        // Inspect valuators for this device
        for (int c = 0; c < devices[i].num_classes; ++c) {
            switch (devices[i].classes[c]->type) {
            case XIValuatorClass: {
                XIValuatorClassInfo *vci = reinterpret_cast<XIValuatorClassInfo *>(devices[i].classes[c]);
                const int valuatorAtom = kis_atom(vci->label);
                dbgInput << "   has valuator" << atomName(vci->label) << \
                    "Atom number" << vci->label << "recognized?" << (valuatorAtom < KisXcbAtom::NAtoms);
                if (valuatorAtom < KisXcbAtom::NAtoms) {
                    TabletData::ValuatorClassInfo info;
                    info.minVal = vci->min;
                    info.maxVal = vci->max;
                    info.number = vci->number;
                    tabletData.valuatorInfo[valuatorAtom] = info;
                }
                break;
            }
            case XIButtonClass: {
                XIButtonClassInfo *bci = reinterpret_cast<XIButtonClassInfo *>(devices[i].classes[c]);

                /* From prior code
                DeviceButtonPress(dev, device_data.xinput_button_press,
                                  device_data.eventList[device_data.eventCount]);
                if (device_data.eventList[device_data.eventCount])
                    ++device_data.eventCount;
                DeviceButtonRelease(dev, device_data.xinput_button_release,
                                    device_data.eventList[device_data.eventCount]);
                if (device_data.eventList[device_data.eventCount])
                    ++device_data.eventCount;
                break;
                */


                // This doesn't seem useful
                /* if (bci->num_buttons >= 5) {
                    Atom label4 = bci->labels[3];
                    Atom label5 = bci->labels[4];
                    // Some drivers have no labels on the wheel buttons, some
                    // have no label on just one and some have no label on
                    // button 4 and the wrong one on button 5. So we just check
                    // that they are not labelled with unrelated buttons.
                    if ((!label4 ||
                         kis_atom(label4) == KisXcbAtom::ButtonWheelUp ||
                         kis_atom(label4) == KisXcbAtom::ButtonWheelDown) &&
                        (!label5 ||
                         kis_atom(label5) == KisXcbAtom::ButtonWheelUp ||
                         kis_atom(label5) == KisXcbAtom::ButtonWheelDown))
                        scrollingDevice.legacyOrientations |= Qt::Vertical;
                }
                if (bci->num_buttons >= 7) {
                    Atom label6 = bci->labels[5];
                    Atom label7 = bci->labels[6];
                    if ((!label6 || kis_atom(label6) == KisXcbAtom::ButtonHorizWheelLeft) &&
                        (!label7 || kis_atom(label7) == KisXcbAtom::ButtonHorizWheelRight))
                        scrollingDevice.legacyOrientations |= Qt::Horizontal;
                }
                */
                dbgInput << "   has" << bci->num_buttons << "buttons";
                break;
            }
            case XIKeyClass:
                dbgInput << "   posesses keys?";
                break;
            case XITouchClass:
                // will be handled in touchDeviceForId()
                // TODO: Look into this.
                break;
            default:
                dbgInput << "   has class" << devices[i].classes[c]->type;
                break;
            }
        }

        // If we have found the valuators which we expect a tablet to have, it
        // might be a tablet. No promises!
        bool isTablet = false;
        if (tabletData.valuatorInfo.contains(KisXcbAtom::AbsX) &&
                tabletData.valuatorInfo.contains(KisXcbAtom::AbsY) &&
                tabletData.valuatorInfo.contains(KisXcbAtom::AbsPressure))
            isTablet = true;

        // But we need to be careful not to take the touch and tablet-button devices as tablets.
        QByteArray name = QByteArray(devices[i].name).toLower();
        QString dbgType = QLatin1String("UNKNOWN");
        if (name.contains("eraser")) {
            isTablet = true;
            gotEraser = true;
            tabletData.pointerType = QTabletEvent::Eraser;
            dbgType = QLatin1String("eraser");
        } else if (name.contains("cursor")) {
            isTablet = true;
            tabletData.pointerType = QTabletEvent::Cursor;
            dbgType = QLatin1String("cursor");
        } else if ((name.contains("pen") || name.contains("stylus")) && isTablet) {
            tabletData.pointerType = QTabletEvent::Pen;
            gotStylus = true;
            dbgType = QLatin1String("pen");
        } else if (name.contains("wacom") && isTablet && !name.contains("touch")) {
            // combined device (evdev) rather than separate pen/eraser (wacom driver)
            tabletData.pointerType = QTabletEvent::Pen;
            gotStylus = true;
            dbgType = QLatin1String("pen");
            touchWacomTabletWorkaround = true;
        } else if (name.contains("aiptek") /* && device == KisXcbAtom::KEYBOARD */) {
            // some "Genius" tablets
            isTablet = true;
            tabletData.pointerType = QTabletEvent::Pen;
            dbgType = QLatin1String("pen");
            needCheckIfItIsReallyATablet = true;
            gotStylus = true;
        } else {
            isTablet = false;
        }

        if (isTablet) {
            tabletData.deviceId = devices[i].deviceid;
            m_tabletData.append(tabletData);
        } else {
            // Who cares!
            continue;
        }

        dbgInput << "###################################";
        dbgInput << "# Adding a tablet device:" << devices[i].name;
        dbgInput << "Device Type:" << KisTabletDebugger::tabletDeviceToString(deviceType);

        /*
        dbgInput << "# Axes limits data";
        dbgInput << "X:       " << device_data.minX << device_data.maxX;
        dbgInput << "Y:       " << device_data.minY << device_data.maxY;
        dbgInput << "Z:       " << device_data.minZ << device_data.maxZ;
        dbgInput << "Pressure:" << device_data.minPressure << device_data.maxPressure;
        dbgInput << "Rotation:" << device_data.minRotation << device_data.maxRotation;
        dbgInput << "T. Pres: " << device_data.minTanPressure << device_data.maxTanPressure;
        */

        // TODO: see if this is still necessary
        // device_data.savedAxesData.tryFetchAxesMapping(dev);

    } // Loop over devices
    XIFreeDeviceInfo(devices);
}

namespace {
    Qt::MouseButton xiToQtMouseButton(uint32_t b)
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


    Qt::MouseButtons translateMouseButtons(int s)
    {
        Qt::MouseButtons ret = 0;
        if (s & XCB_BUTTON_MASK_1)
            ret |= Qt::LeftButton;
        if (s & XCB_BUTTON_MASK_2)
            ret |= Qt::MidButton;
        if (s & XCB_BUTTON_MASK_3)
            ret |= Qt::RightButton;
        return ret;
    }

    QTabletEvent::TabletDevice toolIdToTabletDevice(quint32 toolId) {
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


    int xi2ValuatorOffset(unsigned char *maskPtr, int maskLen, int number)
    {
        int offset = 0;
        for (int i = 0; i < maskLen; i++) {
            if (number < 8) {
                if ((maskPtr[i] & (1 << number)) == 0)
                    return -1;
            }
            for (int j = 0; j < 8; j++) {
                if (j == number)
                    return offset;
                if (maskPtr[i] & (1 << j))
                    offset++;
            }
            number -= 8;
        }
        return -1;
    }


    bool xi2GetValuatorValueIfSet(void *event, int valuatorNum, double *value)
    {
        xXIDeviceEvent *xideviceevent = static_cast<xXIDeviceEvent *>(event);
        unsigned char *buttonsMaskAddr = (unsigned char*)&xideviceevent[1];
        unsigned char *valuatorsMaskAddr = buttonsMaskAddr + xideviceevent->buttons_len * 4;
        FP3232 *valuatorsValuesAddr = (FP3232*)(valuatorsMaskAddr + xideviceevent->valuators_len * 4);

        int valuatorOffset = xi2ValuatorOffset(valuatorsMaskAddr, xideviceevent->valuators_len, valuatorNum);
        if (valuatorOffset < 0)
            return false;

        *value = valuatorsValuesAddr[valuatorOffset].integral;
        *value += ((double)valuatorsValuesAddr[valuatorOffset].frac / (1 << 16) / (1 << 16));
        return true;
    }

    void sendProximityEvent(KisXcbConnection::TabletData &tabletData, QEvent::Type type)
    {
        QPointF emptyPos;
        qreal zero = 0.0;
        QTabletEvent e(type, emptyPos, emptyPos, tabletData.tool, tabletData.pointerType,
                       zero, 0, 0, zero, zero, 0, (Qt::KeyboardModifiers)0,
                       tabletData.serialId, (Qt::MouseButton)0, tabletData.buttons);
        qApp->sendEvent(qApp->activeWindow(), &e);
    }

/**
 * <MEMORY LAYOUT NONSENSE>
 */
    // Starting from the xcb version 1.9.3 struct xcb_ge_event_t has changed:
    // - "pad0" became "extension"
    // - "pad1" and "pad" became "pad0"
    // New and old version of this struct share the following fields:
    // NOTE: API might change again in the next release of xcb in which case this comment will
    // need to be updated to reflect the reality.
    typedef struct qt_xcb_ge_event_t {
        uint8_t  response_type;
        uint8_t  extension;
        uint16_t sequence;
        uint32_t length;
        uint16_t event_type;
    } qt_xcb_ge_event_t;

    bool xi2PrepareXIGenericDeviceEvent(xcb_ge_event_t *ev, int opCode)
    {
        qt_xcb_ge_event_t *event = (qt_xcb_ge_event_t *)ev;
        // xGenericEvent has "extension" on the second byte, the same is true for xcb_ge_event_t starting from
        // the xcb version 1.9.3, prior to that it was called "pad0".
        if (event->extension == opCode) {
            // xcb event structs contain stuff that wasn't on the wire, the full_sequence field
            // adds an extra 4 bytes and generic events cookie data is on the wire right after the standard 32 bytes.
            // Move this data back to have the same layout in memory as it was on the wire
            // and allow casting, overwriting the full_sequence field.
            memmove((char*) event + 32, (char*) event + 36, event->length * 4);
            return true;
        }
        return false;
    }
/**
 * </MEMORY LAYOUT NONSENSE>
 */

}

bool KisXcbConnection::xi2HandleTabletEvent(void *event, TabletData &tabletData)
{
    bool handled = true;
    xXIGenericDeviceEvent *xiEvent = static_cast<xXIGenericDeviceEvent *>(event);
    xXIDeviceEvent *xiDeviceEvent = reinterpret_cast<xXIDeviceEvent *>(xiEvent);

    switch (xiEvent->evtype) {
    case XI_ButtonPress: {
        Qt::MouseButton b = xiToQtMouseButton(xiDeviceEvent->detail);
        tabletData.buttons |= b;
        return xi2ReportTabletEvent(tabletData, xiEvent, QEvent::TabletPress);
        break;
    }
    case XI_ButtonRelease: {
        Qt::MouseButton b = xiToQtMouseButton(xiDeviceEvent->detail);
        tabletData.buttons ^= b;
        return xi2ReportTabletEvent(tabletData, xiEvent, QEvent::TabletRelease);
        break;
    }
    case XI_Motion:
        // Report TabletMove only when the stylus is touching the tablet or any button is pressed.
        // TODO: report proximity (hover) motion (no suitable Qt event exists yet).
        // if (tabletData.buttons != Qt::NoButton)
        return xi2ReportTabletEvent(tabletData, xiEvent, QEvent::TabletMove);
        break;
    case XI_PropertyEvent: {
        // This is the wacom driver's way of reporting tool proximity.
        // The evdev driver doesn't do it this way.
        // Note: this was inline in Qt source but split up here, because it is gross.
        xXIPropertyEvent *ev = reinterpret_cast<xXIPropertyEvent *>(event);
        if (ev->what == XIPropertyModified) {
            if (ev->property == kis_atom(KisXcbAtom::WacomSerialIDs)) {
                handled = handleWacomProximityEvent(tabletData, ev);
            }
        }
        break;
    }
    default:
        handled = false;
        break;
    }

    // Synthesize mouse events since otherwise there are no mouse events from
    // the pen on the XI 2.2+ path.
    // TODO: not implemented.
    // if (xi2MouseEvents() && eventListener)
    //     eventListener->handleXIMouseEvent(reinterpret_cast<xcb_ge_event_t *>(event));


    return handled;
}


bool KisXcbConnection::handleWacomProximityEvent(TabletData &tabletData, void *event)
{
    xXIPropertyEvent *ev = reinterpret_cast<xXIPropertyEvent *>(event);
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
    bool handled = false;
    if (XIGetProperty(display, tabletData.deviceId, ev->property, 0, 100,
                      0, AnyPropertyType, &propType, &propFormat,
                      &numItems, &bytesAfter, &data) == Success) {
        if (propType == atom(KisXcbAtom::INTEGER) && propFormat == 32 && numItems == _WACSER_COUNT) {
            handled = true;

            quint32 *ptr = reinterpret_cast<quint32 *>(data);
            quint32 tool = ptr[_WACSER_TOOL_ID];
            // Workaround for http://sourceforge.net/p/linuxwacom/bugs/246/
            // e.g. on Thinkpad Helix, tool ID will be 0 and serial will be 1
            if (!tool && ptr[_WACSER_TOOL_SERIAL])
                tool = ptr[_WACSER_TOOL_SERIAL];

            // The property change event informs us which tool is in proximity or which one left proximity.
            if (tool) {
                tabletData.inProximity = true;
                tabletData.tool = toolIdToTabletDevice(tool);
                tabletData.serialId = qint64(ptr[_WACSER_USB_ID]) << 32 | qint64(ptr[_WACSER_TOOL_SERIAL]);
                sendProximityEvent(tabletData, QEvent::TabletEnterProximity);
            } else {
                tabletData.inProximity = false;
                tabletData.tool = toolIdToTabletDevice(ptr[_WACSER_LAST_TOOL_ID]);
                // Workaround for http://sourceforge.net/p/linuxwacom/bugs/246/
                // e.g. on Thinkpad Helix, tool ID will be 0 and serial will be 1
                if (!tabletData.tool)
                    tabletData.tool = toolIdToTabletDevice(ptr[_WACSER_LAST_TOOL_SERIAL]);
                tabletData.serialId = qint64(ptr[_WACSER_USB_ID]) << 32 | qint64(ptr[_WACSER_LAST_TOOL_SERIAL]);
                sendProximityEvent(tabletData, QEvent::TabletLeaveProximity);
                tabletData.widgetToGetPress = 0;
            }

            // Qt had more informative debug output
            dbgInput << QString("XI2 proximity change on tablet %d (USB %x):"\
                                "last tool: %x id %x current tool: %x id %x TabletDevice %d")\
                .arg(tabletData.deviceId).arg(ptr[_WACSER_USB_ID]).arg(ptr[_WACSER_LAST_TOOL_SERIAL]) \
                .arg(ptr[_WACSER_LAST_TOOL_ID]).arg(ptr[_WACSER_TOOL_SERIAL]).arg(ptr[_WACSER_TOOL_ID])\
                .arg(tabletData.tool);
        }
        XFree(data);
    }
    return handled;
}

bool KisXcbConnection::xi2ReportTabletEvent(TabletData &tabletData, void *event, QEvent::Type type)
{
    xXIDeviceEvent *ev = reinterpret_cast<xXIDeviceEvent*>(event);

    // Assumption: dpr is the same for all screens and never changes. (Should hold for X11)
    // static qreal dpr = QPlatformScreen::platformScreenForWindow(qApp->allWindows().at(0))->devicePixelRatio();
    static qreal dpr = qApp->screens().at(0)->devicePixelRatio();

    double pressure = 0, rotation = 0, tangentialPressure = 0;
    int xTilt = 0, yTilt = 0;
    Qt::KeyboardModifiers keyState = QApplication::queryKeyboardModifiers();

    // Note: there may be an issue related to scaling by dpr
    QPointF global(0.0, 0.0);
    QRect screenArea = qApp->desktop()->rect();
    // auto screens = qApp->screens();
    // QRegion screenArea(screens.at(0)->geometry());
    // for (int i = 1; i < screens.count(); ++i)
    //     screenArea += screens.at(i)->geometry();

    for (QHash<int, TabletData::ValuatorClassInfo>::iterator it = tabletData.valuatorInfo.begin(),
            ite = tabletData.valuatorInfo.end(); it != ite; ++it) {
        int valuator = it.key();
        TabletData::ValuatorClassInfo &classInfo(it.value());
        // Using raw valuator data
        xi2GetValuatorValueIfSet(event, classInfo.number, &classInfo.curVal);
        double normalizedValue = (classInfo.curVal - classInfo.minVal) / (classInfo.maxVal - classInfo.minVal);
        switch (valuator) {
        case KisXcbAtom::AbsX:
            global.rx() = screenArea.width() * normalizedValue;
            break;
        case KisXcbAtom::AbsY:
            global.ry() = screenArea.height() * normalizedValue;
            break;
        case KisXcbAtom::AbsPressure:
            pressure = normalizedValue;
            break;
        case KisXcbAtom::AbsTiltX:
            xTilt = classInfo.curVal;
            break;
        case KisXcbAtom::AbsTiltY:
            yTilt = classInfo.curVal;
            break;
        case KisXcbAtom::AbsWheel:
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

    // Global valuator method
    QPointF local = global - QPointF((ev->root_x >> 16) - (ev->event_x >> 16),
                                     (ev->root_y >> 16) - (ev->event_y >> 16));


    // Find target widget. Start by finding top level window.
    QWidget *w = tabletData.widgetToGetPress;
    if (!w) w = QApplication::activePopupWidget();
    if (!w) w = QApplication::activeModalWidget();
    if (!w) w = QWidget::find((WId)ev->event);
    if (!w) w = qApp->activeWindow();
    if (!w) return true;

    // Now find child widget if appropriate.
    QWidget *child = w->childAt(local.toPoint());
    QPointF widgetLocal;
    if (child) {
        w = child;
        widgetLocal = w->mapFromGlobal(global.toPoint());
    } else {
        // TODO: straighten out local/global transformations
        widgetLocal = global - w->mapToGlobal(QPoint());
    }


    Qt::MouseButton qtbutton = Qt::NoButton;
    if (type != QEvent::TabletMove) {
        qtbutton = xiToQtMouseButton(ev->detail);
    }

    if (type == QEvent::TabletRelease) {
        dbgInput << "Releasing target widget" << w;
        tabletData.widgetToGetPress = 0;
    }


    QTabletEvent e(type, widgetLocal, global, tabletData.tool, tabletData.pointerType,
                   pressure, xTilt, yTilt, tangentialPressure, rotation, 0,
                   keyState, tabletData.serialId, qtbutton, tabletData.buttons);
    e.ignore();
    qApp->sendEvent(w, &e);
    dbgInput << "Custom QTabletEvent" << type << "sent to widget:" << w << " Accepted?" << e.isAccepted();
    dbgInput << KisTabletDebugger::instance()->eventToString(e, "");
    // return e.isAccepted();
    if (e.isAccepted()) {
        // Lock onto target widget
        if (type == QEvent::TabletPress) {
            dbgInput << "Targeting new widget" << w;
            tabletData.widgetToGetPress = w;
        }
    } else {
        QWindow *window = QWindow::fromWinId(ev->event);
        // QWindowSystemInterface::handleTabletEvent(window, local, global,
        //                                           tabletData.tool, tabletData.pointerType,
        //                                           tabletData.buttons, pressure,
        //                                           xTilt, yTilt, tangentialPressure,
        //                                           rotation, 0, tabletData.serialId);
        return false;
    }


    return true;
}



bool KisXcbConnection::xi2HandleEvent(xcb_ge_event_t *event)
{

    if (xi2PrepareXIGenericDeviceEvent(event, m_xiOpCode)) {
        xXIGenericDeviceEvent *xiEvent = reinterpret_cast<xXIGenericDeviceEvent *>(event);
        int sourceDeviceId = xiEvent->deviceid; // may be the master id
        xXIDeviceEvent *xiDeviceEvent = 0;

        switch (xiEvent->evtype) {
        case XI_ButtonPress:
        case XI_ButtonRelease:
        case XI_Motion:
        case XI_TouchBegin:
        case XI_TouchUpdate:
        case XI_TouchEnd:
        {
            xiDeviceEvent = reinterpret_cast<xXIDeviceEvent *>(event);
            sourceDeviceId = xiDeviceEvent->sourceid; // use the actual device id instead of the master
            break;
        }
        default:
            break;
        }

        for (int i = 0; i < m_tabletData.count(); ++i) {
            if (m_tabletData.at(i).deviceId == sourceDeviceId) {
                if (xi2HandleTabletEvent(xiEvent, m_tabletData[i])) {
                    // dbgInput << "Slurped an event. Don't use this one, Qt!";
                    return true;
                }
            }
        }

        // Handle non-tablet XInput2.2 events.
        if (xiDeviceEvent) {
            switch (xiDeviceEvent->evtype) {
            case XI_ButtonPress:
            case XI_ButtonRelease:
            case XI_Motion:
                // We don't really care about processing stuff here, we just want to block processing
                if (kis_tabletChokeMouse) {
                    kis_tabletChokeMouse = false;
                    return true;
                }
                break;

            // XXX: Uncomment when restoring touch events.
            // case XI_TouchBegin:
            // case XI_TouchUpdate:
            // case XI_TouchEnd:
            //     qCDebug(lcQpaXInput, "XI2 touch event type %d seq %d detail %d pos %6.1f, %6.1f root pos %6.1f, %6.1f on window %x",event->event_type, xiDeviceEvent->sequenceNumber, xiDeviceEvent->detail,fixed1616ToReal(xiDeviceEvent->event_x), fixed1616ToReal(xiDeviceEvent->event_y),fixed1616ToReal(xiDeviceEvent->root_x), fixed1616ToReal(xiDeviceEvent->root_y),xiDeviceEvent->event);
            //     if (QXcbWindow *platformWindow = platformWindowFromId(xiDeviceEvent->event))
            //         xi2ProcessTouch(xiDeviceEvent, platformWindow);
            //     break;

            }
        }
    }
    return false;
};


// static inline int fixed1616ToInt(FP1616 val)
// {
//     return int((qreal(val >> 16)) + (val & 0xFFFF) / (qreal)0xFFFF);
// }

// // With XI 2.2+ press/release/motion comes here instead of the above handlers.
// void KisXcbConnection::handleXIMouseEvent(xcb_ge_event_t *event)
// {
//     xXIDeviceEvent *ev = reinterpret_cast<xXIDeviceEvent *>(event);
//     const Qt::KeyboardModifiers modifiers = connection->keyboard()->translateModifiers(ev->mods.effective_mods);
//     const int event_x = fixed1616ToInt(ev->event_x);
//     const int event_y = fixed1616ToInt(ev->event_y);
//     const int root_x = fixed1616ToInt(ev->root_x);
//     const int root_y = fixed1616ToInt(ev->root_y);

//     connection->keyboard()->updateXKBStateFromXI(&ev->mods, &ev->group);

//     const Qt::MouseButton button = conn->xiToQtMouseButton(ev->detail);

//     if (ev->buttons_len > 0) {
//         unsigned char *buttonMask = (unsigned char *) &ev[1];
//         for (int i = 1; i <= 15; ++i)
//             conn->setButton(conn->translateMouseButton(i), XIMaskIsSet(buttonMask, i));
//     }

//     switch (ev->evtype) {
//     case XI_ButtonPress:
//         dbgInput << "XI2 mouse press, " << ppvar(button) << ppVar(ev->time);
//         conn->setButton(button, true);
//         handleButtonPressEvent(event_x, event_y, root_x, root_y, ev->detail, modifiers, ev->time);
//         break;
//     case XI_ButtonRelease:
//         dbgInput << "XI2 mouse release, " << ppvar(button) << ppVar(ev->time);
//         conn->setButton(button, false);
//         handleButtonReleaseEvent(event_x, event_y, root_x, root_y, ev->detail, modifiers, ev->time);
//         break;
//     case XI_Motion:
//         dbgInput << "XI2 mouse motion, " << ppvar(button) << ppVar(ev->time);
//         handleMotionNotifyEvent(event_x, event_y, root_x, root_y, modifiers, ev->time);
//         break;
//     default:
//         qWarning() << "Unrecognized XI2 mouse event" << ev->evtype;
//         break;
//     }
// }


QByteArray KisXcbConnection::atomName(xcb_atom_t atom)
{
    if (!atom)
        return QByteArray();

    xcb_generic_error_t *error = 0;
    xcb_get_atom_name_cookie_t cookie = xcb_get_atom_name(connection, atom);
    xcb_get_atom_name_reply_t *reply = xcb_get_atom_name_reply(connection, cookie, &error);
    if (error) {
        qWarning() << "QXcbConnection::atomName: bad Atom" << atom;
        free(error);
    }
    if (reply) {
        QByteArray result(xcb_get_atom_name_name(reply), xcb_get_atom_name_name_length(reply));
        free(reply);
        return result;
    }
    return QByteArray();
}
