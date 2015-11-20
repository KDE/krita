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
    m_keyboard = new QXcbKeyboard(this);
}

QXcbConnection::~QXcbConnection()
{
#if defined(XCB_USE_XINPUT2)
    finalizeXInput2();
#endif

    delete m_keyboard;
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

void QXcbConnection::addWindowEventListener(xcb_window_t id, QXcbWindowEventListener *eventListener)
{
    m_mapper.insert(id, eventListener);
}

void QXcbConnection::removeWindowEventListener(xcb_window_t id)
{
    m_mapper.remove(id);
}

QXcbWindowEventListener *QXcbConnection::windowEventListenerFromId(xcb_window_t id)
{
    return m_mapper.value(id, 0);
}

QXcbWindow *QXcbConnection::platformWindowFromId(xcb_window_t id)
{
    QXcbWindowEventListener *listener = m_mapper.value(id, 0);
    if (listener)
        return listener->toWindow();
    return 0;
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

QPointF QXcbScreen::mapFromNative(const QPointF &pos) const
{
    return pos;
}

QRect QXcbScreen::geometry() const
{
    return QRect(0,0,1000,1000);
}

QSizeF QXcbScreen::physicalSize() const
{
    return QSizeF(500,300);
}


QWindow* QXcbWindow::window() const
{
    return 0;
}

QXcbScreen *QXcbWindow::xcbScreen() const
{
    return 0;
}

qreal QXcbWindow::devicePixelRatio() const
{
    return 1.0;
}










Qt::KeyboardModifiers QXcbKeyboard::translateModifiers(int s) const
{
    Q_UNUSED(s);

    Qt::KeyboardModifiers ret = 0;

    // TODO: find modifiers somewhere!

/*    if (s & XCB_MOD_MASK_SHIFT)
        ret |= Qt::ShiftModifier;
    if (s & XCB_MOD_MASK_CONTROL)
        ret |= Qt::ControlModifier;
    if (s & rmod_masks.alt)
        ret |= Qt::AltModifier;
    if (s & rmod_masks.meta)
        ret |= Qt::MetaModifier;
    if (s & rmod_masks.altgr)
        ret |= Qt::GroupSwitchModifier;

*/

    return ret;

}
