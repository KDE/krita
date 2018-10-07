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

#include "kis_debug.h"

#include <QTouchDevice>

#include <QPointF>
#include <QRectF>
#include <QVector2D>
#include <QTouchEvent>
#include <QWindow>
#include <QPointer>
#include <QLoggingCategory>
#include <xcb/xcb.h>

#define XCB_USE_XINPUT2 1
#define XCB_USE_XLIB 1

// This is needed to make Qt compile together with XKB. xkb.h is using a variable
// which is called 'explicit', this is a reserved keyword in c++
#ifndef QT_NO_XKB
#define explicit dont_use_cxx_explicit
//#include <xcb/xkb.h>
#undef explicit
#endif

#ifndef QT_NO_TABLETEVENT
#include <QTabletEvent>
#endif

#if XCB_USE_XINPUT2
#include <X11/extensions/XI2.h>
#include <X11/extensions/XI2proto.h>
#ifdef XIScrollClass
#define XCB_USE_XINPUT21    // XI 2.1 adds smooth scrolling support
#ifdef XI_TouchBeginMask
#define XCB_USE_XINPUT22    // XI 2.2 adds multi-point touch support
#endif
#endif
#endif // XCB_USE_XINPUT2

struct xcb_randr_get_output_info_reply_t;


struct XInput2TouchDeviceData;

Q_DECLARE_LOGGING_CATEGORY(lcQpaXInput)
Q_DECLARE_LOGGING_CATEGORY(lcQpaXInputDevices)
Q_DECLARE_LOGGING_CATEGORY(lcQpaScreen)

class QWindowSystemInterface
{
public:
    struct TouchPoint {
        TouchPoint() : id(0), pressure(0), state(Qt::TouchPointStationary), flags(0) { }
        int id;                 // for application use
        QPointF normalPosition; // touch device coordinates, (0 to 1, 0 to 1)
        QRectF area;            // the touched area, centered at position in screen coordinates
        qreal pressure;         // 0 to 1
        Qt::TouchPointState state; //Qt::TouchPoint{Pressed|Moved|Stationary|Released}
        QVector2D velocity;     // in screen coordinate system, pixels / seconds
        QTouchEvent::TouchPoint::InfoFlags flags;
        QVector<QPointF> rawPositions; // in screen coordinates
    };

    static void registerTouchDevice(QTouchDevice *device)
    {
        Q_UNUSED(device);
    }
    static void handleTouchEvent(QWindow *w, ulong timestamp, QTouchDevice *device,
                                 const QList<struct TouchPoint> &points, Qt::KeyboardModifiers mods = Qt::NoModifier)
    {
        Q_UNUSED(w);
        Q_UNUSED(timestamp);
        Q_UNUSED(device);
        Q_UNUSED(points);
        Q_UNUSED(mods);

        ENTER_FUNCTION();
    }
#if QT_VERSION >= 0x050700
    static void handleWheelEvent(QWindow *w, ulong timestamp, const QPointF & local, const QPointF & global, QPoint pixelDelta, QPoint angleDelta, Qt::KeyboardModifiers mods = Qt::NoModifier, Qt::ScrollPhase phase = Qt::NoScrollPhase, Qt::MouseEventSource source = Qt::MouseEventNotSynthesized);
#else
    static void handleWheelEvent(QWindow *w, ulong timestamp, const QPointF & local, const QPointF & global, QPoint pixelDelta, QPoint angleDelta, Qt::KeyboardModifiers mods = Qt::NoModifier, Qt::ScrollPhase phase = Qt::ScrollUpdate, Qt::MouseEventSource source = Qt::MouseEventNotSynthesized);
#endif

    static void handleTabletEnterProximityEvent(int device, int pointerType, qint64 uid);
    static void handleTabletLeaveProximityEvent(int device, int pointerType, qint64 uid);

    static void handleTabletEvent(QWindow *w, const QPointF &local, const QPointF &global,
                                  int device, int pointerType, Qt::MouseButtons buttons, qreal pressure, int xTilt, int yTilt,
                                  qreal tangentialPressure, qreal rotation, int z, qint64 uid,
                                  Qt::KeyboardModifiers modifiers);
};

namespace QXcbAtom {
    enum Atom {
        // window-manager <-> client protocols
        WM_PROTOCOLS,
        WM_DELETE_WINDOW,
        WM_TAKE_FOCUS,
        _NET_WM_PING,
        _NET_WM_CONTEXT_HELP,
        _NET_WM_SYNC_REQUEST,
        _NET_WM_SYNC_REQUEST_COUNTER,
        MANAGER, // System tray notification
        _NET_SYSTEM_TRAY_OPCODE, // System tray operation

        // ICCCM window state
        WM_STATE,
        WM_CHANGE_STATE,
        WM_CLASS,
        WM_NAME,

        // Session management
        WM_CLIENT_LEADER,
        WM_WINDOW_ROLE,
        SM_CLIENT_ID,

        // Clipboard
        CLIPBOARD,
        INCR,
        TARGETS,
        MULTIPLE,
        TIMESTAMP,
        SAVE_TARGETS,
        CLIP_TEMPORARY,
        _QT_SELECTION,
        _QT_CLIPBOARD_SENTINEL,
        _QT_SELECTION_SENTINEL,
        CLIPBOARD_MANAGER,

        RESOURCE_MANAGER,

        _XSETROOT_ID,

        _QT_SCROLL_DONE,
        _QT_INPUT_ENCODING,

        // Qt/XCB specific
        _QT_CLOSE_CONNECTION,

        _MOTIF_WM_HINTS,

        DTWM_IS_RUNNING,
        ENLIGHTENMENT_DESKTOP,
        _DT_SAVE_MODE,
        _SGI_DESKS_MANAGER,

        // EWMH (aka NETWM)
        _NET_SUPPORTED,
        _NET_VIRTUAL_ROOTS,
        _NET_WORKAREA,

        _NET_MOVERESIZE_WINDOW,
        _NET_WM_MOVERESIZE,

        _NET_WM_NAME,
        _NET_WM_ICON_NAME,
        _NET_WM_ICON,

        _NET_WM_PID,

        _NET_WM_WINDOW_OPACITY,

        _NET_WM_STATE,
        _NET_WM_STATE_ABOVE,
        _NET_WM_STATE_BELOW,
        _NET_WM_STATE_FULLSCREEN,
        _NET_WM_STATE_MAXIMIZED_HORZ,
        _NET_WM_STATE_MAXIMIZED_VERT,
        _NET_WM_STATE_MODAL,
        _NET_WM_STATE_STAYS_ON_TOP,
        _NET_WM_STATE_DEMANDS_ATTENTION,

        _NET_WM_USER_TIME,
        _NET_WM_USER_TIME_WINDOW,
        _NET_WM_FULL_PLACEMENT,

        _NET_WM_WINDOW_TYPE,
        _NET_WM_WINDOW_TYPE_DESKTOP,
        _NET_WM_WINDOW_TYPE_DOCK,
        _NET_WM_WINDOW_TYPE_TOOLBAR,
        _NET_WM_WINDOW_TYPE_MENU,
        _NET_WM_WINDOW_TYPE_UTILITY,
        _NET_WM_WINDOW_TYPE_SPLASH,
        _NET_WM_WINDOW_TYPE_DIALOG,
        _NET_WM_WINDOW_TYPE_DROPDOWN_MENU,
        _NET_WM_WINDOW_TYPE_POPUP_MENU,
        _NET_WM_WINDOW_TYPE_TOOLTIP,
        _NET_WM_WINDOW_TYPE_NOTIFICATION,
        _NET_WM_WINDOW_TYPE_COMBO,
        _NET_WM_WINDOW_TYPE_DND,
        _NET_WM_WINDOW_TYPE_NORMAL,
        _KDE_NET_WM_WINDOW_TYPE_OVERRIDE,

        _KDE_NET_WM_FRAME_STRUT,
        _NET_FRAME_EXTENTS,

        _NET_STARTUP_INFO,
        _NET_STARTUP_INFO_BEGIN,

        _NET_SUPPORTING_WM_CHECK,

        _NET_WM_CM_S0,

        _NET_SYSTEM_TRAY_VISUAL,

        _NET_ACTIVE_WINDOW,

        // Property formats
        TEXT,
        UTF8_STRING,
        CARDINAL,

        // Xdnd
        XdndEnter,
        XdndPosition,
        XdndStatus,
        XdndLeave,
        XdndDrop,
        XdndFinished,
        XdndTypelist,
        XdndActionList,

        XdndSelection,

        XdndAware,
        XdndProxy,

        XdndActionCopy,
        XdndActionLink,
        XdndActionMove,
        XdndActionPrivate,

        // Motif DND
        _MOTIF_DRAG_AND_DROP_MESSAGE,
        _MOTIF_DRAG_INITIATOR_INFO,
        _MOTIF_DRAG_RECEIVER_INFO,
        _MOTIF_DRAG_WINDOW,
        _MOTIF_DRAG_TARGETS,

        XmTRANSFER_SUCCESS,
        XmTRANSFER_FAILURE,

        // Xkb
        _XKB_RULES_NAMES,

        // XEMBED
        _XEMBED,
        _XEMBED_INFO,

        // XInput2
        ButtonLeft,
        ButtonMiddle,
        ButtonRight,
        ButtonWheelUp,
        ButtonWheelDown,
        ButtonHorizWheelLeft,
        ButtonHorizWheelRight,
        AbsMTPositionX,
        AbsMTPositionY,
        AbsMTTouchMajor,
        AbsMTTouchMinor,
        AbsMTPressure,
        AbsMTTrackingID,
        MaxContacts,
        RelX,
        RelY,
        // XInput2 tablet
        AbsX,
        AbsY,
        AbsPressure,
        AbsTiltX,
        AbsTiltY,
        AbsWheel,
        AbsDistance,
        WacomSerialIDs,
        INTEGER,
        RelHorizWheel,
        RelVertWheel,
        RelHorizScroll,
        RelVertScroll,

        _XSETTINGS_SETTINGS,

        _COMPIZ_DECOR_PENDING,
        _COMPIZ_DECOR_REQUEST,
        _COMPIZ_DECOR_DELETE_PIXMAP,

        NPredefinedAtoms,

        _QT_SETTINGS_TIMESTAMP = NPredefinedAtoms,
        NAtoms
    };
}

class QXcbConnection
{
public:
    struct ScrollingDevice {
        ScrollingDevice() : deviceId(0), verticalIndex(0), horizontalIndex(0), orientations(0), legacyOrientations(0) { }
        int deviceId;
        int verticalIndex, horizontalIndex;
        double verticalIncrement, horizontalIncrement;
        Qt::Orientations orientations;
        Qt::Orientations legacyOrientations;
        QPointF lastScrollPosition;
    };

    struct TabletData {
        TabletData() : deviceId(0), pointerType(QTabletEvent::UnknownPointer),
            tool(QTabletEvent::Stylus), buttons(0), serialId(0), inProximity(false) { }
        int deviceId;
        QTabletEvent::PointerType pointerType;
        QTabletEvent::TabletDevice tool;
        Qt::MouseButtons buttons;
        qint64 serialId;
        bool inProximity;
        struct ValuatorClassInfo {
        ValuatorClassInfo() : minVal(0.), maxVal(0.), curVal(0.) { }
            double minVal;
            double maxVal;
            double curVal;
            int number;
        };
        QHash<int, ValuatorClassInfo> valuatorInfo;
    };

public:
    QXcbConnection(bool canGrabServer, const char *displayName);
    ~QXcbConnection();
    
#ifdef XCB_USE_XINPUT21
    bool isAtLeastXI21() const { return m_xi2Enabled && m_xi2Minor >= 1; }
#else
    bool isAtLeastXI21() const { return false; }
#endif
#ifdef XCB_USE_XINPUT22
    bool isAtLeastXI22() const { return m_xi2Enabled && m_xi2Minor >= 2; }
#else
    bool isAtLeastXI22() const { return false; }
#endif

    void initializeXInput2();
    void xi2SetupDevices();
    void finalizeXInput2();

    void xi2Select(xcb_window_t window);
    XInput2TouchDeviceData *touchDeviceForId(int id);

    bool xi2HandleEvent(xcb_ge_event_t *event);
    bool xi2SetMouseGrabEnabled(xcb_window_t w, bool grab);

    static bool xi2PrepareXIGenericDeviceEvent(xcb_ge_event_t *event, int opCode);  // FIXME: to be copied
    static bool xi2GetValuatorValueIfSet(void *event, int valuatorNum, double *value);  // FIXME: to be copied

    void xi2HandleHierachyEvent(void *event);
    void xi2HandleDeviceChangedEvent(void *event);
    void updateScrollingDevice(ScrollingDevice &scrollingDevice, int num_classes, void *classInfo);

    void handleEnterEvent(const xcb_enter_notify_event_t *);
    void xi2HandleScrollEvent(void *event, ScrollingDevice &scrollingDevice);
    Qt::MouseButton xiToQtMouseButton(uint32_t b);
    Qt::MouseButtons xiToQtMouseButtons(xXIDeviceEvent *xiDeviceEvent);

    bool xi2HandleTabletEvent(void *event, TabletData *tabletData, QWindow *window);
    void xi2ReportTabletEvent(TabletData &tabletData, void *event);

    inline xcb_atom_t atom(QXcbAtom::Atom atom) const { return m_allAtoms[atom]; }
    QXcbAtom::Atom qatom(xcb_atom_t xatom) const;
    QByteArray atomName(xcb_atom_t atom);
    void initializeAllAtoms();

    bool xi2MouseEvents() const;

    QWindow *windowFromId(xcb_window_t id);

    bool canGrab() const { return m_canGrabServer; }
    void *xlib_display() const;

    xcb_connection_t *xcb_connection() const { return m_connection; }

    void notifyEnterEvent(xcb_enter_notify_event_t *event);

    void addWindowFromXi2Id(xcb_window_t id);

private:
    xcb_connection_t *m_connection;
    bool m_canGrabServer;
    QByteArray m_displayName;
    void *m_xlib_display;

    xcb_atom_t m_allAtoms[QXcbAtom::NAtoms];

    bool m_xi2Enabled;
    int m_xi2Minor;

    int m_xiOpCode, m_xiEventBase, m_xiErrorBase;
    QVector<TabletData> m_tabletData;

    QHash<xcb_window_t, QPointer<QWindow>> m_windowMapper;

    QHash<int, XInput2TouchDeviceData*> m_touchDevices;
    bool m_xiGrab;


    QHash<int, ScrollingDevice> m_scrollingDevices;
};


#ifdef Q_XCB_DEBUG
template <typename cookie_t>
cookie_t q_xcb_call_template(const cookie_t &cookie, QXcbConnection *connection, const char *file, int line)
{
    connection->log(file, line, cookie.sequence);
    return cookie;
}
#define Q_XCB_CALL(x) q_xcb_call_template(x, connection(), __FILE__, __LINE__)
#define Q_XCB_CALL2(x, connection) q_xcb_call_template(x, connection, __FILE__, __LINE__)
#define Q_XCB_NOOP(c) q_xcb_call_template(xcb_no_operation(c->xcb_connection()), c, __FILE__, __LINE__);
#else
#define Q_XCB_CALL(x) x
#define Q_XCB_CALL2(x, connection) x
#define Q_XCB_NOOP(c) (void)c;
#endif
