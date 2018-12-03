
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

#include <QX11Info>
#include <QWidget>
#include <QPointer>
#include <QElapsedTimer>
#include <QGuiApplication>
#include <QApplication>

#include <X11/extensions/XI2proto.h>
#include <xcb/xproto.h>

Q_LOGGING_CATEGORY(lcQpaXInput, "qt.qpa.input")
Q_LOGGING_CATEGORY(lcQpaXInputDevices, "qt.qpa.input.devices")
Q_LOGGING_CATEGORY(lcQpaScreen, "qt.qpa.screen")

QXcbConnection::QXcbConnection(bool canGrabServer, const char *displayName)
    : m_connection(0)
    , m_canGrabServer(canGrabServer)
    , m_displayName(displayName ? QByteArray(displayName) : qgetenv("DISPLAY"))
    #ifdef XCB_USE_XLIB
    , m_xlib_display(0)
    #endif
{
    m_connection = QX11Info::connection();
    m_xlib_display = QX11Info::display();

    if (!m_connection || xcb_connection_has_error(m_connection)) {
        qFatal("QXcbConnection: Could not connect to display %s", m_displayName.constData());
    }

    initializeAllAtoms();

#if defined(XCB_USE_XINPUT2)
    initializeXInput2();
#endif
}

QXcbConnection::~QXcbConnection()
{
#if defined(XCB_USE_XINPUT2)
    finalizeXInput2();
#endif
}

QXcbAtom::Atom QXcbConnection::qatom(xcb_atom_t xatom) const
{
    return static_cast<QXcbAtom::Atom>(std::find(m_allAtoms, m_allAtoms + QXcbAtom::NAtoms, xatom) - m_allAtoms);
}

void *QXcbConnection::xlib_display() const
{
    return m_xlib_display;
}

QByteArray QXcbConnection::atomName(xcb_atom_t atom)
{
    if (!atom)
        return QByteArray();

    xcb_generic_error_t *error = 0;
    xcb_get_atom_name_cookie_t cookie = Q_XCB_CALL(xcb_get_atom_name(xcb_connection(), atom));
    xcb_get_atom_name_reply_t *reply = xcb_get_atom_name_reply(xcb_connection(), cookie, &error);
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

static const char * xcb_atomnames = {
    // window-manager <-> client protocols
    "WM_PROTOCOLS\0"
    "WM_DELETE_WINDOW\0"
    "WM_TAKE_FOCUS\0"
    "_NET_WM_PING\0"
    "_NET_WM_CONTEXT_HELP\0"
    "_NET_WM_SYNC_REQUEST\0"
    "_NET_WM_SYNC_REQUEST_COUNTER\0"
    "MANAGER\0"
    "_NET_SYSTEM_TRAY_OPCODE\0"

    // ICCCM window state
    "WM_STATE\0"
    "WM_CHANGE_STATE\0"
    "WM_CLASS\0"
    "WM_NAME\0"

    // Session management
    "WM_CLIENT_LEADER\0"
    "WM_WINDOW_ROLE\0"
    "SM_CLIENT_ID\0"

    // Clipboard
    "CLIPBOARD\0"
    "INCR\0"
    "TARGETS\0"
    "MULTIPLE\0"
    "TIMESTAMP\0"
    "SAVE_TARGETS\0"
    "CLIP_TEMPORARY\0"
    "_QT_SELECTION\0"
    "_QT_CLIPBOARD_SENTINEL\0"
    "_QT_SELECTION_SENTINEL\0"
    "CLIPBOARD_MANAGER\0"

    "RESOURCE_MANAGER\0"

    "_XSETROOT_ID\0"

    "_QT_SCROLL_DONE\0"
    "_QT_INPUT_ENCODING\0"

    "_QT_CLOSE_CONNECTION\0"

    "_MOTIF_WM_HINTS\0"

    "DTWM_IS_RUNNING\0"
    "ENLIGHTENMENT_DESKTOP\0"
    "_DT_SAVE_MODE\0"
    "_SGI_DESKS_MANAGER\0"

    // EWMH (aka NETWM)
    "_NET_SUPPORTED\0"
    "_NET_VIRTUAL_ROOTS\0"
    "_NET_WORKAREA\0"

    "_NET_MOVERESIZE_WINDOW\0"
    "_NET_WM_MOVERESIZE\0"

    "_NET_WM_NAME\0"
    "_NET_WM_ICON_NAME\0"
    "_NET_WM_ICON\0"

    "_NET_WM_PID\0"

    "_NET_WM_WINDOW_OPACITY\0"

    "_NET_WM_STATE\0"
    "_NET_WM_STATE_ABOVE\0"
    "_NET_WM_STATE_BELOW\0"
    "_NET_WM_STATE_FULLSCREEN\0"
    "_NET_WM_STATE_MAXIMIZED_HORZ\0"
    "_NET_WM_STATE_MAXIMIZED_VERT\0"
    "_NET_WM_STATE_MODAL\0"
    "_NET_WM_STATE_STAYS_ON_TOP\0"
    "_NET_WM_STATE_DEMANDS_ATTENTION\0"

    "_NET_WM_USER_TIME\0"
    "_NET_WM_USER_TIME_WINDOW\0"
    "_NET_WM_FULL_PLACEMENT\0"

    "_NET_WM_WINDOW_TYPE\0"
    "_NET_WM_WINDOW_TYPE_DESKTOP\0"
    "_NET_WM_WINDOW_TYPE_DOCK\0"
    "_NET_WM_WINDOW_TYPE_TOOLBAR\0"
    "_NET_WM_WINDOW_TYPE_MENU\0"
    "_NET_WM_WINDOW_TYPE_UTILITY\0"
    "_NET_WM_WINDOW_TYPE_SPLASH\0"
    "_NET_WM_WINDOW_TYPE_DIALOG\0"
    "_NET_WM_WINDOW_TYPE_DROPDOWN_MENU\0"
    "_NET_WM_WINDOW_TYPE_POPUP_MENU\0"
    "_NET_WM_WINDOW_TYPE_TOOLTIP\0"
    "_NET_WM_WINDOW_TYPE_NOTIFICATION\0"
    "_NET_WM_WINDOW_TYPE_COMBO\0"
    "_NET_WM_WINDOW_TYPE_DND\0"
    "_NET_WM_WINDOW_TYPE_NORMAL\0"
    "_KDE_NET_WM_WINDOW_TYPE_OVERRIDE\0"

    "_KDE_NET_WM_FRAME_STRUT\0"
    "_NET_FRAME_EXTENTS\0"

    "_NET_STARTUP_INFO\0"
    "_NET_STARTUP_INFO_BEGIN\0"

    "_NET_SUPPORTING_WM_CHECK\0"

    "_NET_WM_CM_S0\0"

    "_NET_SYSTEM_TRAY_VISUAL\0"

    "_NET_ACTIVE_WINDOW\0"

    // Property formats
    "TEXT\0"
    "UTF8_STRING\0"
    "CARDINAL\0"

    // xdnd
    "XdndEnter\0"
    "XdndPosition\0"
    "XdndStatus\0"
    "XdndLeave\0"
    "XdndDrop\0"
    "XdndFinished\0"
    "XdndTypeList\0"
    "XdndActionList\0"

    "XdndSelection\0"

    "XdndAware\0"
    "XdndProxy\0"

    "XdndActionCopy\0"
    "XdndActionLink\0"
    "XdndActionMove\0"
    "XdndActionPrivate\0"

    // Motif DND
    "_MOTIF_DRAG_AND_DROP_MESSAGE\0"
    "_MOTIF_DRAG_INITIATOR_INFO\0"
    "_MOTIF_DRAG_RECEIVER_INFO\0"
    "_MOTIF_DRAG_WINDOW\0"
    "_MOTIF_DRAG_TARGETS\0"

    "XmTRANSFER_SUCCESS\0"
    "XmTRANSFER_FAILURE\0"

    // Xkb
    "_XKB_RULES_NAMES\0"

    // XEMBED
    "_XEMBED\0"
    "_XEMBED_INFO\0"

    // XInput2
    "Button Left\0"
    "Button Middle\0"
    "Button Right\0"
    "Button Wheel Up\0"
    "Button Wheel Down\0"
    "Button Horiz Wheel Left\0"
    "Button Horiz Wheel Right\0"
    "Abs MT Position X\0"
    "Abs MT Position Y\0"
    "Abs MT Touch Major\0"
    "Abs MT Touch Minor\0"
    "Abs MT Pressure\0"
    "Abs MT Tracking ID\0"
    "Max Contacts\0"
    "Rel X\0"
    "Rel Y\0"
    // XInput2 tablet
    "Abs X\0"
    "Abs Y\0"
    "Abs Pressure\0"
    "Abs Tilt X\0"
    "Abs Tilt Y\0"
    "Abs Wheel\0"
    "Abs Distance\0"
    "Wacom Serial IDs\0"
    "INTEGER\0"
    "Rel Horiz Wheel\0"
    "Rel Vert Wheel\0"
    "Rel Horiz Scroll\0"
    "Rel Vert Scroll\0"
    "_XSETTINGS_SETTINGS\0"
    "_COMPIZ_DECOR_PENDING\0"
    "_COMPIZ_DECOR_REQUEST\0"
    "_COMPIZ_DECOR_DELETE_PIXMAP\0" // \0\0 terminates loop.
};

void QXcbConnection::initializeAllAtoms() {
    const char *names[QXcbAtom::NAtoms];
    const char *ptr = xcb_atomnames;

    int i = 0;
    while (*ptr) {
        names[i++] = ptr;
        while (*ptr)
            ++ptr;
        ++ptr;
    }

    Q_ASSERT(i == QXcbAtom::NPredefinedAtoms);

    QByteArray settings_atom_name("_QT_SETTINGS_TIMESTAMP_");
    settings_atom_name += m_displayName;
    names[i++] = settings_atom_name;

    xcb_intern_atom_cookie_t cookies[QXcbAtom::NAtoms];

    Q_ASSERT(i == QXcbAtom::NAtoms);
    for (i = 0; i < QXcbAtom::NAtoms; ++i)
        cookies[i] = xcb_intern_atom(xcb_connection(), false, strlen(names[i]), names[i]);

    for (i = 0; i < QXcbAtom::NAtoms; ++i) {
        xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(xcb_connection(), cookies[i], 0);
        m_allAtoms[i] = reply->atom;
        free(reply);
    }
}

bool QXcbConnection::xi2MouseEvents() const
{
    static bool mouseViaXI2 = !qEnvironmentVariableIsSet("QT_XCB_NO_XI2_MOUSE");
    return mouseViaXI2;
}

void QXcbConnection::notifyEnterEvent(xcb_enter_notify_event_t *event)
{
    xcb_window_t window;

    // first cleaning up deleted windows: assuming 0 is not a valid window id

    while ((window = m_windowMapper.key(0,0)) != 0) {
        m_windowMapper.remove(window);
    }

    addWindowFromXi2Id(event->event);
}

void QXcbConnection::addWindowFromXi2Id(xcb_window_t id)
{
    if (!m_windowMapper.contains(id)) {
        QWidget *widget = QWidget::find(id);
        if (widget) {
            QWindow *windowHandle = widget->windowHandle();
            m_windowMapper.insert(id, windowHandle);

            // we should also cold-init the window events mask
            // (Qt's own initialization got broken in Qt 5.10)
            xi2Select(id);
        }
    }
}

QWindow* QXcbConnection::windowFromId(xcb_window_t id)
{
    QWindow *window = m_windowMapper.value(id, 0);

    // Try to fetch the window Id lazily. It is needed when the cursor gets under
    // a popup window or a popup dialog, which doesn't produce any enter event on
    // some systems
    if (!window) {
        addWindowFromXi2Id(id);
        window = m_windowMapper.value(id, 0);
    }

    return window;
}

static int xi2ValuatorOffset(unsigned char *maskPtr, int maskLen, int number)
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

bool QXcbConnection::xi2GetValuatorValueIfSet(void *event, int valuatorNum, double *value)
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

bool QXcbConnection::xi2PrepareXIGenericDeviceEvent(xcb_ge_event_t *ev, int opCode)
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

class Q_GUI_EXPORT QWindowSystemInterfacePrivate {
public:
    enum EventType {
        UserInputEvent = 0x100,
        Close = UserInputEvent | 0x01,
        GeometryChange = 0x02,
        Enter = UserInputEvent | 0x03,
        Leave = UserInputEvent | 0x04,
        ActivatedWindow = 0x05,
        WindowStateChanged = 0x06,
        Mouse = UserInputEvent | 0x07,
        FrameStrutMouse = UserInputEvent | 0x08,
        Wheel = UserInputEvent | 0x09,
        Key = UserInputEvent | 0x0a,
        Touch = UserInputEvent | 0x0b,
        ScreenOrientation = 0x0c,
        ScreenGeometry = 0x0d,
        ScreenAvailableGeometry = 0x0e,
        ScreenLogicalDotsPerInch = 0x0f,
        ScreenRefreshRate = 0x10,
        ThemeChange = 0x11,
        Expose_KRITA_XXX = 0x12,
        FileOpen = UserInputEvent | 0x13,
        Tablet = UserInputEvent | 0x14,
        TabletEnterProximity = UserInputEvent | 0x15,
        TabletLeaveProximity = UserInputEvent | 0x16,
        PlatformPanel = UserInputEvent | 0x17,
        ContextMenu = UserInputEvent | 0x18,
        EnterWhatsThisMode = UserInputEvent | 0x19,
#ifndef QT_NO_GESTURES
        Gesture = UserInputEvent | 0x1a,
#endif
        ApplicationStateChanged = 0x19,
        FlushEvents = 0x20,
        WindowScreenChanged = 0x21
    };

    class WindowSystemEvent {
    public:
        enum {
            Synthetic = 0x1,
            NullWindow = 0x2
        };

        explicit WindowSystemEvent(EventType t)
            : type(t), flags(0) { }
        virtual ~WindowSystemEvent() { }

        bool synthetic() const  { return flags & Synthetic; }
        bool nullWindow() const { return flags & NullWindow; }

        EventType type;
        int flags;
    };

    class UserEvent : public WindowSystemEvent {
    public:
        UserEvent(QWindow * w, ulong time, EventType t)
            : WindowSystemEvent(t), window(w), timestamp(time)
        {
            if (!w)
                flags |= NullWindow;
        }
        QPointer<QWindow> window;
        unsigned long timestamp;
    };

    class InputEvent: public UserEvent {
    public:
        InputEvent(QWindow * w, ulong time, EventType t, Qt::KeyboardModifiers mods)
            : UserEvent(w, time, t), modifiers(mods) {}
        Qt::KeyboardModifiers modifiers;
    };

    class TabletEvent : public InputEvent {
    public:
        static void handleTabletEvent(QWindow *w, const QPointF &local, const QPointF &global,
                                      int device, int pointerType, Qt::MouseButtons buttons, qreal pressure, int xTilt, int yTilt,
                                      qreal tangentialPressure, qreal rotation, int z, qint64 uid,
                                      Qt::KeyboardModifiers modifiers = Qt::NoModifier);

        TabletEvent(QWindow *w, ulong time, const QPointF &local, const QPointF &global,
                    int device, int pointerType, Qt::MouseButtons b, qreal pressure, int xTilt, int yTilt, qreal tpressure,
                    qreal rotation, int z, qint64 uid, Qt::KeyboardModifiers mods)
            : InputEvent(w, time, Tablet, mods),
              buttons(b), local(local), global(global), device(device), pointerType(pointerType),
              pressure(pressure), xTilt(xTilt), yTilt(yTilt), tangentialPressure(tpressure),
              rotation(rotation), z(z), uid(uid) { }
        Qt::MouseButtons buttons;
        QPointF local;
        QPointF global;
        int device;
        int pointerType;
        qreal pressure;
        int xTilt;
        int yTilt;
        qreal tangentialPressure;
        qreal rotation;
        int z;
        qint64 uid;
    };

    class WheelEvent : public InputEvent {
    public:
#if QT_VERSION >= 0x050700
        WheelEvent(QWindow *w, ulong time, const QPointF & local, const QPointF & global, QPoint pixelD, QPoint angleD, int qt4D, Qt::Orientation qt4O,
                   Qt::KeyboardModifiers mods, Qt::ScrollPhase phase = Qt::NoScrollPhase, Qt::MouseEventSource src = Qt::MouseEventNotSynthesized)
#else
        WheelEvent(QWindow *w, ulong time, const QPointF & local, const QPointF & global, QPoint pixelD, QPoint angleD, int qt4D, Qt::Orientation qt4O,
                   Qt::KeyboardModifiers mods, Qt::ScrollPhase phase = Qt::ScrollUpdate, Qt::MouseEventSource src = Qt::MouseEventNotSynthesized)
#endif
            : InputEvent(w, time, Wheel, mods), pixelDelta(pixelD), angleDelta(angleD), qt4Delta(qt4D), qt4Orientation(qt4O), localPos(local), globalPos(global), phase(phase), source(src) { }
        QPoint pixelDelta;
        QPoint angleDelta;
        int qt4Delta;
        Qt::Orientation qt4Orientation;
        QPointF localPos;
        QPointF globalPos;
        Qt::ScrollPhase phase;
        Qt::MouseEventSource source;
    };
};

void processTabletEvent(QWindowSystemInterfacePrivate::TabletEvent *e);

static QElapsedTimer g_eventTimer;
struct EventTimerStaticInitializer
{
    EventTimerStaticInitializer() {
        g_eventTimer.start();
    }
};
EventTimerStaticInitializer __timerStaticInitializer;

Qt::MouseButtons tabletState = Qt::NoButton;
QPointer<QWidget> tabletPressWidget = 0;

void QWindowSystemInterface::handleTabletEvent(QWindow *w, const QPointF &local, const QPointF &global,
                                               int device, int pointerType, Qt::MouseButtons buttons, qreal pressure, int xTilt, int yTilt,
                                               qreal tangentialPressure, qreal rotation, int z, qint64 uid,
                                               Qt::KeyboardModifiers modifiers)
{
    qint64 timestamp = g_eventTimer.msecsSinceReference() + g_eventTimer.elapsed();

    QWindowSystemInterfacePrivate::TabletEvent *e =
            new QWindowSystemInterfacePrivate::TabletEvent(w, timestamp, local, global, device, pointerType, buttons, pressure,
                                                           xTilt, yTilt, tangentialPressure, rotation, z, uid, modifiers);

    processTabletEvent(e);
}

void processTabletEvent(QWindowSystemInterfacePrivate::TabletEvent *e)
{
#ifndef QT_NO_TABLETEVENT
    QEvent::Type type = QEvent::TabletMove;
    if (e->buttons != tabletState)
        type = (e->buttons > tabletState) ? QEvent::TabletPress : QEvent::TabletRelease;

    bool localValid = true;

    // It can happen that we got no tablet release event. Just catch it here
    // and clean up the state.
    if (type == QEvent::TabletMove && e->buttons == Qt::NoButton) {
        tabletPressWidget = 0;
    }

    QWidget *targetWidget = 0;

    if (tabletPressWidget) {
        targetWidget = tabletPressWidget;
        localValid = false;
    } else if (e->window) {
        /**
         * Here we use a weird way of converting QWindow into a
         * QWidget.  The problem is that the Qt itself does it by just
         * converting QWindow into QWidgetWindow. But the latter one
         * is private, so we cannot use it.
         *
         * We also cannot use QApplication::widegtAt(). We *MUST NOT*!
         * There is some but in XCB: if we call
         * QApplication::topLevelAt() during the event processing, the
         * Enter/Leave events stop arriving. Or, more precisely, they
         * start to errive at random points in time. Which makes
         * KisShortcutMatcher go crazy of course.
         *
         * So instead of just fetching the toplevel window we decrypt
         * the pointer using WinId mapping.
         */

        targetWidget = QWidget::find(e->window->winId());

        if (targetWidget) {
            QWidget *childWidget = targetWidget->childAt(e->local.toPoint());
            if (childWidget) {
                targetWidget = childWidget;
                localValid = false;
            }
        }
    }

    if (!targetWidget) {
        targetWidget = QApplication::widgetAt(e->global.toPoint());
        localValid = false;

        if (!targetWidget) return;
    }

    if (type == QEvent::TabletPress) {
        tabletPressWidget = targetWidget;
    } else if (type == QEvent::TabletRelease) {
        tabletPressWidget = 0;
    }

    QPointF local = e->local;
    if (!localValid) {
        QPointF delta = e->global - e->global.toPoint();
        local = targetWidget->mapFromGlobal(e->global.toPoint()) + delta;
    }
    Qt::MouseButtons stateChange = e->buttons ^ tabletState;
    Qt::MouseButton button = Qt::NoButton;
    for (int check = Qt::LeftButton; check <= int(Qt::MaxMouseButton); check = check << 1) {
        if (check & stateChange) {
            button = Qt::MouseButton(check);
            break;
        }
    }
    QTabletEvent ev(type, local, e->global,
                    e->device, e->pointerType, e->pressure, e->xTilt, e->yTilt,
                    e->tangentialPressure, e->rotation, e->z,
                    e->modifiers, e->uid, button, e->buttons);
    ev.setTimestamp(e->timestamp);

    QGuiApplication::sendEvent(targetWidget, &ev);

    tabletState = e->buttons;
#else
    Q_UNUSED(e)
#endif
}

void QWindowSystemInterface::handleTabletEnterProximityEvent(int device, int pointerType, qint64 uid)
{
    qint64 timestamp = g_eventTimer.msecsSinceReference() + g_eventTimer.elapsed();

    QTabletEvent ev(QEvent::TabletEnterProximity, QPointF(), QPointF(),
                    device, pointerType, 0, 0, 0,
                    0, 0, 0,
                    Qt::NoModifier, uid, Qt::NoButton, tabletState);
    ev.setTimestamp(timestamp);
    QGuiApplication::sendEvent(qGuiApp, &ev);
}

void QWindowSystemInterface::handleTabletLeaveProximityEvent(int device, int pointerType, qint64 uid)
{
    qint64 timestamp = g_eventTimer.msecsSinceReference() + g_eventTimer.elapsed();

    QTabletEvent ev(QEvent::TabletLeaveProximity, QPointF(), QPointF(),
                    device, pointerType, 0, 0, 0,
                    0, 0, 0,
                    Qt::NoModifier, uid, Qt::NoButton, tabletState);
    ev.setTimestamp(timestamp);
    QGuiApplication::sendEvent(qGuiApp, &ev);
}

void processWheelEvent(QWindowSystemInterfacePrivate::WheelEvent *e);

void QWindowSystemInterface::handleWheelEvent(QWindow *tlw, ulong timestamp, const QPointF & local, const QPointF & global, QPoint pixelDelta, QPoint angleDelta, Qt::KeyboardModifiers mods, Qt::ScrollPhase phase, Qt::MouseEventSource source)
{
    // Qt 4 sends two separate wheel events for horizontal and vertical
    // deltas. For Qt 5 we want to send the deltas in one event, but at the
    // same time preserve source and behavior compatibility with Qt 4.
    //
    // In addition high-resolution pixel-based deltas are also supported.
    // Platforms that does not support these may pass a null point here.
    // Angle deltas must always be sent in addition to pixel deltas.
    QScopedPointer<QWindowSystemInterfacePrivate::WheelEvent> e;

    // Pass Qt::ScrollBegin and Qt::ScrollEnd through
    // even if the wheel delta is null.
    if (angleDelta.isNull() && phase == Qt::ScrollUpdate)
        return;

    // Simple case: vertical deltas only:
    if (angleDelta.y() != 0 && angleDelta.x() == 0) {
        e.reset(new QWindowSystemInterfacePrivate::WheelEvent(tlw, timestamp, local, global, pixelDelta, angleDelta, angleDelta.y(), Qt::Vertical, mods, phase, source));
        processWheelEvent(e.data());
        return;
    }

    // Simple case: horizontal deltas only:
    if (angleDelta.y() == 0 && angleDelta.x() != 0) {
        e.reset(new QWindowSystemInterfacePrivate::WheelEvent(tlw, timestamp, local, global, pixelDelta, angleDelta, angleDelta.x(), Qt::Horizontal, mods, phase, source));
        processWheelEvent(e.data());
        return;
    }

    // Both horizontal and vertical deltas: Send two wheel events.
    // The first event contains the Qt 5 pixel and angle delta as points,
    // and in addition the Qt 4 compatibility vertical angle delta.
    e.reset(new QWindowSystemInterfacePrivate::WheelEvent(tlw, timestamp, local, global, pixelDelta, angleDelta, angleDelta.y(), Qt::Vertical, mods, phase, source));
    processWheelEvent(e.data());

    // The second event contains null pixel and angle points and the
    // Qt 4 compatibility horizontal angle delta.
    e.reset(new QWindowSystemInterfacePrivate::WheelEvent(tlw, timestamp, local, global, QPoint(), QPoint(), angleDelta.x(), Qt::Horizontal, mods, phase, source));
    processWheelEvent(e.data());
}

void processWheelEvent(QWindowSystemInterfacePrivate::WheelEvent *e)
{
#ifndef QT_NO_WHEELEVENT
    QWindow *window = e->window.data();
    QPointF globalPoint = e->globalPos;
    QPointF localPoint = e->localPos;

    if (e->nullWindow()) {
        window = QGuiApplication::topLevelAt(globalPoint.toPoint());
        if (window) {
            QPointF delta = globalPoint - globalPoint.toPoint();
            localPoint = window->mapFromGlobal(globalPoint.toPoint()) + delta;
        }
    }

    if (!window)
        return;

    // Cut off in Krita...
    //
    // QGuiApplicationPrivate::lastCursorPosition = globalPoint;
    // modifier_buttons = e->modifiers;

    //if (window->d_func()->blockedByModalWindow) {
    if (QGuiApplication::modalWindow() &&
            QGuiApplication::modalWindow() != window &&
            QGuiApplication::modalWindow() != window->transientParent()) {

        // a modal window is blocking this window, don't allow wheel events through
        return;
    }

#if QT_VERSION >= 0x050500
    QWheelEvent ev(localPoint, globalPoint, e->pixelDelta, e->angleDelta, e->qt4Delta, e->qt4Orientation, QGuiApplication::mouseButtons(), e->modifiers, e->phase, e->source);
#else
    QWheelEvent ev(localPoint, globalPoint, e->pixelDelta, e->angleDelta, e->qt4Delta, e->qt4Orientation, QGuiApplication::mouseButtons(), e->modifiers, e->phase);
#endif
    ev.setTimestamp(e->timestamp);
    QGuiApplication::sendEvent(window, &ev);
#endif /* ifndef QT_NO_WHEELEVENT */
}
