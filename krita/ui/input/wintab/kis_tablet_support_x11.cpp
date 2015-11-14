/*
 *  Copyright (c) 2013 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_tablet_support_x11.h"

#include <QDesktopWidget>
#include <QApplication>
#include <QWidget>
#include <QX11Info>

#include "kis_debug.h"
#include "kis_tablet_support.h"
#include "kis_xcb_connection.h"
#include "wacom-properties.h"
#include <input/kis_tablet_event.h>


// Note: XInput 2.2 is required
#include <xcb/xcb.h>
#include <X11/extensions/XI2.h>
#include <X11/extensions/XInput2.h>
#include <X11/extensions/XI2proto.h>

/**
 * This is an analog of a Qt's variable qt_tabletChokeMouse.  It is
 * intended to block Mouse events after any accepted Tablet event. In
 * Qt it is available on X11 only, so we won't extend this behavior on
 * Windows.
 */
bool kis_tabletChokeMouse = false;

/**
 * This variable is true when at least one of our tablet is a Evdev
 * one. In such a case we need to request the extension events from
 * the server manually
 */
bool kis_haveEvdevTablets = false;


KisXcbConnection *KIS_XCB = 0;

#define KIS_ATOM(x) KIS_XCB->m_allAtoms[KisXcbAtom::x]
#define KIS_X11 KIS_XCB

void KisTabletSupportXcb::init()
{
    // TODO: free this structure on exit
    dbgInput << "Creating xcb event handler.";
    KIS_XCB = new KisXcbConnection;
}

KisTabletSupportXcb::~KisTabletSupportXcb()
{
    delete KIS_XCB;
    KIS_XCB = 0;
}

void QTabletDeviceData::SavedAxesData::tryFetchAxesMapping(XDevice *dev)
{
    Atom propertyType;
    int propertyFormat;
    unsigned long nitems;
    unsigned long bytes_after;
    unsigned char *prop;

    int result = XGetDeviceProperty(KIS_X11->display, dev,
                                    KIS_ATOM(AxisLabels),
                                    0, 16, false,
                                    AnyPropertyType,
                                    &propertyType,
                                    &propertyFormat,
                                    &nitems,
                                    &bytes_after,
                                    &prop);

    if (result == Success && propertyType > 0) {

        //if (KisTabletDebugger::instance()->initializationDebugEnabled()) {
        dbgInput << "# Building tablet axes remapping table:";
        //}

        QVector<AxesIndexes> axesMap(nitems, Unused);

        for (unsigned int axisIndex = 0; axisIndex < nitems; axisIndex++) {
            Atom currentAxisName = ((Atom*)prop)[axisIndex];

            AxesIndexes mappedIndex =
                Unused;

            if (currentAxisName == KIS_ATOM(AbsX)) {
                mappedIndex = XCoord;
            } else if (currentAxisName == KIS_ATOM(AbsY)) {
                mappedIndex = YCoord;
            } else if (currentAxisName == KIS_ATOM(AbsPressure)) {
                mappedIndex = Pressure;
            } else if (currentAxisName == KIS_ATOM(AbsTiltX)) {
                mappedIndex = XTilt;
            } else if (currentAxisName == KIS_ATOM(AbsTiltY)) {
                mappedIndex = YTilt;
            }

            axesMap[axisIndex] = mappedIndex;

            //if (KisTabletDebugger::instance()->initializationDebugEnabled()) {
                dbgInput << XGetAtomName(KIS_X11->display, currentAxisName)
                         << axisIndex << "->" << mappedIndex;
            // }
        }

        this->setAxesMap(axesMap);
    }
}


// Not working - commented out
/*
void fetchWacomToolId(qint64 &serialId, qint64 &deviceId, QTabletDeviceData *tablet)
{
    XDevice *dev = static_cast<XDevice*>(tablet->device);
    Atom prop = None, type;
    int format;
    unsigned char* data;
    unsigned long nitems, bytes_after;

    prop = XInternAtom(KIS_X11->display, WACOM_PROP_SERIALIDS, True);

    if (!prop) {
        // property doesn't exist
        return;
    }

    XGetDeviceProperty(KIS_X11->display, dev, prop, 0, 1000, False, AnyPropertyType,
                       &type, &format, &nitems, &bytes_after, &data);

    if (nitems < 5 || format != 32) {
        // property offset doesn't exist
        return;
    }

    long *l = (long*)data;
    serialId = l[3]; // serial id of the stylus in proximity
    deviceId = l[4]; // device if of the stylus in proximity
}
*/

QTabletEvent::TabletDevice parseWacomDeviceId(quint32 deviceId)
{
    enum {
        CSR_TYPE_SPECIFIC_MASK = 0x0F06,
        CSR_TYPE_SPECIFIC_STYLUS_BITS = 0x0802,
        CSR_TYPE_SPECIFIC_AIRBRUSH_BITS = 0x0902,
        CSR_TYPE_SPECIFIC_4DMOUSE_BITS = 0x0004,
        CSR_TYPE_SPECIFIC_LENSCURSOR_BITS = 0x0006,
        CSR_TYPE_SPECIFIC_ROTATIONSTYLUS_BITS = 0x0804
    };

    QTabletEvent::TabletDevice device;

    if ((((deviceId & 0x0006) == 0x0002) &&
         ((deviceId & CSR_TYPE_SPECIFIC_MASK) != CSR_TYPE_SPECIFIC_AIRBRUSH_BITS)) || // Bamboo
        deviceId == 0x4020) { // Surface Pro 2 tablet device

        device = QTabletEvent::Stylus;
    } else {
        switch (deviceId & CSR_TYPE_SPECIFIC_MASK) {
        case CSR_TYPE_SPECIFIC_STYLUS_BITS:
            device = QTabletEvent::Stylus;
            break;
        case CSR_TYPE_SPECIFIC_AIRBRUSH_BITS:
            device = QTabletEvent::Airbrush;
            break;
        case CSR_TYPE_SPECIFIC_4DMOUSE_BITS:
            device = QTabletEvent::FourDMouse;
            break;
        case CSR_TYPE_SPECIFIC_LENSCURSOR_BITS:
            device = QTabletEvent::Puck;
            break;
        case CSR_TYPE_SPECIFIC_ROTATIONSTYLUS_BITS:
            device = QTabletEvent::RotationStylus;
            break;
        default:
            if (deviceId != 0) {
                warnKrita << "Unrecognized stylus device found! Falling back to usual \'Stylus\' definition";
                warnKrita << ppVar(deviceId);
                warnKrita << ppVar((deviceId & CSR_TYPE_SPECIFIC_MASK));
            }

            device = QTabletEvent::Stylus;
        }
    }

    return device;
}

bool old_translateXinputEvent(const XEvent *ev, QTabletDeviceData *tablet, QWidget *defaultWidget)
{
    Q_ASSERT(defaultWidget);
    Q_ASSERT(tablet != 0);

    QWidget *w = defaultWidget;
    QPoint global,
        curr;
    QPointF hiRes;
    qreal pressure = 0;
    int xTilt = 0,
        yTilt = 0,
        z = 0;
    qreal tangentialPressure = 0;
    qreal rotation = 0;
    int deviceType = QTabletEvent::NoDevice;
    int pointerType = QTabletEvent::UnknownPointer;
    const XDeviceMotionEvent *motion = 0;
    XDeviceButtonEvent *button = 0;
    KisTabletEvent::ExtraEventType t = KisTabletEvent::TabletMoveEx;
    Qt::KeyboardModifiers modifiers = 0;

    modifiers = QApplication::queryKeyboardModifiers();

    XID device_id = 0;

    if (ev->type == tablet->xinput_motion) {
        motion = reinterpret_cast<const XDeviceMotionEvent*>(ev);
        t = KisTabletEvent::TabletMoveEx;
        global = QPoint(motion->x_root, motion->y_root);
        curr = QPoint(motion->x, motion->y);
        device_id = motion->deviceid;
    } else if (ev->type == tablet->xinput_button_press || ev->type == tablet->xinput_button_release) {
        if (ev->type == tablet->xinput_button_press) {
            t = KisTabletEvent::TabletPressEx;
        } else {
            t = KisTabletEvent::TabletReleaseEx;
        }
        button = (XDeviceButtonEvent*)ev;
        global = QPoint(button->x_root, button->y_root);
        curr = QPoint(button->x, button->y);
        device_id = button->deviceid;
    } else {
        qFatal("Unknown event type! Probably, 'proximity', "
               "but we don't handle it here, so this is a bug");
    }

    qint64 wacomSerialId = 0;
    qint64 wacomDeviceId = 0;
    // We've been passed in data for a tablet device that handles this type
    // of event, but it isn't necessarily the tablet device that originated
    // the event.  Use the device id to find the originating device if we
    // have it.
    QTabletDeviceDataList *tablet_list = qt_tablet_devices();
    for (int i = 0; i < tablet_list->size(); ++i) {
        QTabletDeviceData &tab = tablet_list->operator[](i);
        if (device_id == static_cast<XDevice *>(tab.device)->device_id) {
            // Replace the tablet passed in with this one.
            tablet = &tab;
            deviceType = tab.deviceType;
            if (tab.deviceType == QTabletEvent::XFreeEraser) {
                deviceType = QTabletEvent::Stylus;
                pointerType = QTabletEvent::Eraser;
            } else if (tab.deviceType == QTabletEvent::Stylus) {
                pointerType = QTabletEvent::Pen;
            }
            break;
        }
    }

    /**
     * Touch events from Wacom tablets should not be sent as real
     * tablet events
     */
    if (tablet->isTouchWacomTablet) return false;

    // fetchWacomToolId(wacomSerialId, wacomDeviceId, tablet);

    if (wacomDeviceId && deviceType == QTabletEvent::Stylus) {
        deviceType = parseWacomDeviceId(wacomDeviceId);
    }

    QRect screenArea = qApp->desktop()->rect();

    /**
     * Some 'nice' tablet drivers (evdev) do not return the value
     * of all the axes each time. Instead they tell about the
     * recenty changed axes only, so we have to keep the state of
     * all the axes internally and update the relevant part only.
     */
    bool hasSaneData = false;
    if (motion) {
        hasSaneData =
            tablet->savedAxesData.updateAxesData(motion->first_axis,
                                                 motion->axes_count,
                                                 motion->axis_data);
    } else if (button) {
        hasSaneData =
            tablet->savedAxesData.updateAxesData(button->first_axis,
                                                 button->axes_count,
                                                 button->axis_data);
    }

    if (!hasSaneData) return false;

    hiRes = tablet->savedAxesData.position(tablet, screenArea);
    pressure = tablet->savedAxesData.pressure();
    xTilt = tablet->savedAxesData.xTilt();
    yTilt = tablet->savedAxesData.yTilt();


    if (deviceType == QTabletEvent::Airbrush) {
        tangentialPressure =
            std::fmod(qreal(tablet->savedAxesData.rotation() - tablet->minRotation) /
                      (tablet->maxRotation - tablet->minRotation) , 2.0);
    } else {
        rotation =
            std::fmod(qreal(tablet->savedAxesData.rotation() - tablet->minRotation) /
                      (tablet->maxRotation - tablet->minRotation) + 0.5, 1.0) * 360.0;
    }

    if (tablet->widgetToGetPress) {
        w = tablet->widgetToGetPress;
    } else {
        QWidget *child = w->childAt(curr);
        if (child)
            w = child;
    }
    curr = w->mapFromGlobal(global);

    if (t == KisTabletEvent::TabletPressEx) {
        tablet->widgetToGetPress = w;
    } else if (t == KisTabletEvent::TabletReleaseEx && tablet->widgetToGetPress) {
        w = tablet->widgetToGetPress;
        curr = w->mapFromGlobal(global);
        tablet->widgetToGetPress = 0;
    }

    Qt::MouseButton qtbutton = Qt::NoButton;
    Qt::MouseButtons qtbuttons;

    if (motion) {
        // qtbuttons = translateMouseButtons(motion->state);
    } else if (button) {
        // qtbuttons = translateMouseButtons(button->state);
        // qtbutton = translateMouseButton(button->button);
    }

    KisTabletEvent e(t, curr, global, hiRes,
                     deviceType, pointerType,
                     qreal(pressure / qreal(tablet->maxPressure - tablet->minPressure)),
                     xTilt, yTilt, tangentialPressure, rotation, z, modifiers, wacomSerialId,
                     qtbutton, qtbuttons);

    e.ignore();
    QApplication::sendEvent(w, &e);
    kis_tabletChokeMouse = true;

    return e.isAccepted();
}


void evdevEventsActivationWorkaround(WId window)
{
    /**
     * Evdev devices send us events *only* in case we requested
     * them for every window which desires to get them, so just
     * do it as it wants
     */
    static QSet<WId> registeredWindows;
    if (registeredWindows.contains(window)) return;

    registeredWindows.insert(window);

    QTabletDeviceDataList *tablets = qt_tablet_devices();
    for (int i = 0; i < tablets->size(); ++i) {
        QTabletDeviceData &tab = tablets->operator [](i);

        XSelectExtensionEvent(KIS_X11->display,
                              window,
                              tab.eventList,
                              tab.eventCount);
    }
}

bool KisTabletSupportXcb::nativeEventFilter(const QByteArray &/*eventType*/, void *message, long * /*unused_on_X11*/)
{
    xcb_generic_event_t *event = static_cast<xcb_generic_event_t*>(message);
    uint response_type = event->response_type & ~0x80; // hmm

    // This only works when XI2.2 is not in use.
    // Otherwise all XI events are sent as XCB_GE_GENERIC.
    if (kis_tabletChokeMouse &&
        (response_type == XCB_BUTTON_RELEASE ||
         response_type == XCB_BUTTON_PRESS ||
         response_type == XCB_MOTION_NOTIFY)) {

        kis_tabletChokeMouse = false;

        // Mhom-mhom...
        return true;
    }

    // if (kis_haveEvdevTablets && response_type == EnterNotify) {
    //     evdevEventsActivationWorkaround((WId)event->xany.window);
    // }

    if (response_type == XCB_GE_GENERIC) {
        bool handled = KIS_XCB->xi2HandleEvent(reinterpret_cast<xcb_ge_event_t *>(message));
        kis_tabletChokeMouse = handled;
        return handled;
    }
    return false;
}



bool old_handleNativeEvent(XEvent* event)
{

    // Old event handler code
    QTabletDeviceDataList *tablets = qt_tablet_devices();
    for (int i = 0; i < tablets->size(); ++i) {
        QTabletDeviceData &tab = tablets->operator [](i);
        if (event->type == tab.xinput_motion
            || event->type == tab.xinput_button_release
            || event->type == tab.xinput_button_press) {

            QWidget *widget = QApplication::activePopupWidget();

            if (!widget) {
                widget = QApplication::activeModalWidget();
            }

            if (!widget) {
                widget = QWidget::find((WId)event->xany.window);
            }

            bool retval = widget ? old_translateXinputEvent(event, &tab, widget) : false;

            if (retval) {
                /**
                 * If the tablet event is accepted, no mouse event
                 * should arrive. Otherwise, the popup widgets (at
                 * least) will not work correctly
                 */
                kis_tabletChokeMouse = true;
            }

            return retval;
        } else if (event->type == tab.xinput_proximity_in ||
                   event->type == tab.xinput_proximity_out) {

            const XProximityNotifyEvent *proximity =
                reinterpret_cast<const XProximityNotifyEvent*>(event);
            XID device_id = proximity->deviceid;

            QTabletDeviceDataList *tablet_list = qt_tablet_devices();
            for (int i = 0; i < tablet_list->size(); ++i) {
                QTabletDeviceData &tab = tablet_list->operator[](i);
                if (device_id == static_cast<XDevice *>(tab.device)->device_id &&
                    tab.isTouchWacomTablet) {

                    QWidget *widget = QApplication::activePopupWidget();

                    if (!widget) {
                        widget = QApplication::activeModalWidget();
                    }

                    if (!widget) {
                        widget = QWidget::find((WId)event->xany.window);
                    }

                    if (widget) {
                        QPoint curr(proximity->x, proximity->y);
                        QWidget *child = widget->childAt(curr);

                        if (child) {
                            widget = child;
                        }

                        QEvent::Type type = (QEvent::Type)
                            (event->type == tab.xinput_proximity_in ?
                             KisTabletEvent::TouchProximityInEx :
                             KisTabletEvent::TouchProximityOutEx);

                        QEvent e(type);
                        e.ignore();
                        QApplication::sendEvent(widget, &e);
                    }

                    return true;
                }
            }
        }
    }

    return false;
}
