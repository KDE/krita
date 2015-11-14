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

// Note: XInput 2.2 is required
#include <xcb/xcb.h>
#include <QTabletEvent>
#include <QX11Info>


namespace KisXcbAtom {
    /* Warning: if you modify this list, modify the names of atoms in kis_x11_atomnames as well! */
    enum Atom {
        XWacomStylus,
        XWacomCursor,
        XWacomEraser,

        XTabletStylus,
        XTabletEraser,

        XInputTablet,

        XInputKeyboard,

        AxisLabels,
        ATOM,

        AbsX,
        AbsY,
        AbsPressure,
        AbsTiltX,
        AbsTiltY,
        AbsWheel,
        AbsDistance,


        WacomSerialIDs,
        INTEGER,

        WacomTouch,

        AiptekStylus,

        NPredefinedAtoms,
        NAtoms = NPredefinedAtoms
    };
}


/* Warning: if you modify this string, modify the list of atoms in KisX11Data as well! */
static const char kis_xcb_atomnames[] = {
    // Wacom old. (before version 0.10)
    "Wacom Stylus\0"
    "Wacom Cursor\0"
    "Wacom Eraser\0"

    // Tablet
    "STYLUS\0"
    "ERASER\0"

    // XInput tablet device
    "TABLET\0"

    // Really "nice" Aiptek devices reporting they are a keyboard
    "KEYBOARD\0"

    // Evdev property that report the assignment of axes
    "Axis Labels\0"
    "ATOM\0"

    // Types of axes reported by evdev
    "Abs X\0"
    "Abs Y\0"
    "Abs Pressure\0"
    "Abs Tilt X\0"
    "Abs Tilt Y\0"
    "Abs Wheel\0"
    "Abs Distance\0"

    // Wacom driver oddity
    "Wacom Serial IDs\0"
    "INTEGER\0"

    // Touch capabilities reported by Wacom Intuos tablets
    "TOUCH\0"

    // Aiptek drivers (e.g. Hyperpen 12000U) reports non-standard type string
    "Stylus\0"

};

class QWidget;

// from QXcbConnection
struct KisXcbConnection
{

    KisXcbConnection();
    ~KisXcbConnection();

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
        QWidget* widgetToGetPress{0};
    };

    void init_tablet();
    void xi2SetupDevices();
    bool xi2HandleEvent(xcb_ge_event_t *event);
    void initializeAllAtoms();
    bool xi2HandleTabletEvent(void *event, TabletData &tabletData);
    bool xi2ReportTabletEvent(TabletData &tabletData, void *event, QEvent::Type type);
    // void handleXIMouseEvent(xcb_ge_event_t *event);  /* not implemented */
    bool handleWacomProximityEvent(TabletData &tabletData, void * event);
    QByteArray atomName(xcb_atom_t atom);

    KisXcbAtom::Atom kis_atom(xcb_atom_t xatom) const;
    inline xcb_atom_t atom(KisXcbAtom::Atom atom) const { return m_allAtoms[atom]; }

    Display *display;
    xcb_connection_t *connection;
    xcb_atom_t m_allAtoms[KisXcbAtom::NAtoms];

    int m_xiOpCode;

    // XXX: these seem unused
    bool use_xinput;
    int m_xi2Minor;
    int m_xi2Major;
    int m_xiEventBase;
    int m_xiErrorBase;



    QVector<TabletData> m_tabletData;


    // QList<QScreen *> m_screens;
    // QRect nativeGeometry() const {
    //       // TODO: Perform the inverse of this:
    //       const int dpr = int(devicePixelRatio()); // we may override m_devicePixelRatio
    //       m_geometry = QRect(xGeometry.topLeft(), xGeometry.size()/dpr);
    //       m_nativeGeometry = QRect(xGeometry.topLeft(), xGeometry.size());
    //       m_availableGeometry = QRect(mapFromNative(xAvailableGeometry.topLeft()), xAvailableGeometry.size()/dpr);
    // }

};
