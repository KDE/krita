/*
 *  Copyright (c) 2013 Digia Plc and/or its subsidiary(-ies).
 *  Copyright (c) 2013 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2013 Dmitry Kazakov <dimula73@gmail.com>
 *  Copyright (c) 2015 Michael Abrahams <miabraha@gmail.com>
 *  Copyright (c) 2015 The Qt Company Ltd.
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

#include <input/kis_tablet_event.h>
#include "kis_tablet_support_win.h"
// #include "kis_tablet_support.h"

#include <kis_debug.h>
#include <QApplication>
#include <QGuiApplication>
#include <QDesktopWidget>

#include <QScreen>
#include <QWidget>
#include <QLibrary>
#include <math.h>
#define Q_PI M_PI

#include <input/kis_extended_modifiers_mapper.h>
#include <input/kis_tablet_debugger.h>

// For "inline tool switches"
#include <KoToolManager.h>
#include <KoInputDevice.h>
#include "kis_screen_size_choice_dialog.h"

// NOTE: we stub out qwindowcontext.cpp::347 to disable Qt's own tablet support.

/**
 * A set of definitions to form a structure of a WinTab packet.  This
 * structure must be 100% the same as the structure of the packet
 * defined by Qt internally. It affects how pktdef.h is loaded.
 */

#define PACKETDATA  (PK_X | PK_Y | PK_BUTTONS | PK_NORMAL_PRESSURE | PK_TANGENT_PRESSURE \
                     | PK_ORIENTATION | PK_CURSOR | PK_Z)
#include "wintab.h"
#ifndef CSR_TYPE
#define CSR_TYPE 20 // Some old Wacom wintab.h may not provide this constant.
#endif
#include "pktdef.h"


namespace {
    enum {
        PacketMode = 0,
        TabletPacketQSize = 128,
        DeviceIdMask = 0xFF6, // device type mask && device color mask
        CursorTypeBitMask = 0x0F06 // bitmask to find the specific cursor type (see Wacom FAQ)
    };


    /* Items below are deprecated. */
    /**
     * A cached array for fetching packets from the WinTab queue
     */
    enum { QT_TABLET_NPACKETQSIZE = 128 };
    static PACKET globalPacketBuf[QT_TABLET_NPACKETQSIZE];  // our own tablet packet queue.

}

/**
 * Pointers to the API functions resolved at runtime.
 * Definitions at http://www.wacomeng.com/windows/docs/Wintab_v140.htm
 */
struct Q_DECL_HIDDEN KisWindowsWinTab32DLL
{
    bool init();

    typedef HCTX (API *PtrWTOpen)(HWND, LPLOGCONTEXT, BOOL);
    typedef BOOL (API *PtrWTClose)(HCTX);
    typedef UINT (API *PtrWTInfo)(UINT, UINT, LPVOID);
    typedef BOOL (API *PtrWTEnable)(HCTX, BOOL);
    typedef BOOL (API *PtrWTOverlap)(HCTX, BOOL);
    typedef int  (API *PtrWTPacketsGet)(HCTX, int, LPVOID);
    typedef BOOL (API *PtrWTGet)(HCTX, LPLOGCONTEXT);
    typedef int  (API *PtrWTQueueSizeGet)(HCTX);
    typedef BOOL (API *PtrWTQueueSizeSet)(HCTX, int);

    PtrWTOpen          wTOpen{0};
    PtrWTClose         wTClose{0};
    PtrWTInfo          wTInfo{0};
    PtrWTEnable        wTEnable{0};
    PtrWTOverlap       wTOverlap{0};
    PtrWTPacketsGet    wTPacketsGet{0};
    PtrWTGet           wTGet{0};
    PtrWTQueueSizeGet  wTQueueSizeGet{0};
    PtrWTQueueSizeSet  wTQueueSizeSet{0};
};



QDebug operator<<(QDebug d, const QWindowsTabletDeviceData &t)
{
    d << "TabletDevice id:" << t.uniqueId << " pressure: " << t.minPressure
      << ".." << t.maxPressure << " tan pressure: " << t.minTanPressure << ".."
      << t.maxTanPressure << " area:" << t.minX << t.minY << t.minZ
      << ".." << t.maxX << t.maxY << t.maxZ << " device " << t.currentDevice
      << " pointer " << t.currentPointerType;
    return d;
}

/**
 * Resolves the WINTAB api functions
 */
bool KisWindowsWinTab32DLL::init()
{
    if (wTInfo)
        return true;
    QLibrary library(QStringLiteral("wintab32"));
    if (!library.load())
        return false;
    wTOpen         = (PtrWTOpen)         library.resolve("WTOpenW");
    wTClose        = (PtrWTClose)        library.resolve("WTClose");
    wTInfo         = (PtrWTInfo)         library.resolve("WTInfoW");
    wTEnable       = (PtrWTEnable)       library.resolve("WTEnable");
    wTOverlap      = (PtrWTOverlap)      library.resolve("WTOverlap");
    wTPacketsGet   = (PtrWTPacketsGet)   library.resolve("WTPacketsGet");
    wTGet          = (PtrWTGet)          library.resolve("WTGetW");
    wTQueueSizeGet = (PtrWTQueueSizeGet) library.resolve("WTQueueSizeGet");
    wTQueueSizeSet = (PtrWTQueueSizeSet) library.resolve("WTQueueSizeSet");
    return wTOpen && wTClose && wTInfo && wTEnable && wTOverlap && wTPacketsGet && wTQueueSizeGet && wTQueueSizeSet;
}


static KisWindowsWinTab32DLL WINTAB_DLL;

class Q_DECL_HIDDEN QWindowsTabletSupport
{
 public:
    explicit QWindowsTabletSupport(HWND window, HCTX context)
        : m_window(window)
        , m_context(context)
    {
        AXIS orientation[3];
        // Some tablets don't support tilt, check if it is possible,
        if (WINTAB_DLL.wTInfo(WTI_DEVICES, DVC_ORIENTATION, &orientation))
            m_tiltSupport = orientation[0].axResolution && orientation[1].axResolution;
    };

    ~QWindowsTabletSupport();

    static QWindowsTabletSupport *create();
    void notifyActivate();


    bool translateTabletProximityEvent(WPARAM wParam, LPARAM lParam);
    bool translateTabletPacketEvent();
    QWindowsTabletDeviceData *currentTabletPointer{ 0 };
    bool hasTiltSupport() const { return m_tiltSupport; };


 private:
    /**
     * Handle a tablet device entering proximity or when switching tools.
     *
     * Calls tabletInit if \p uniqueId represents a new cursor.
     */
    void tabletUpdateCursor(const quint64 uniqueId, const UINT cursorType, const int newPointerType);

    /**
     * Initializes the QWindowsTabletDeviceData structure for cursor \p uniqueId
     * and stores it in the global cache.
     */
    QWindowsTabletDeviceData tabletInit(const quint64 uniqueId, const UINT cursorType) const;

    /**
     * Variables to handle the Wintab tablet context.
     */
    const HWND m_window;
    const HCTX m_context;
    int m_absoluteRange{20};
    bool m_tiltSupport{false};
    QPointF m_oldGlobalPosF;


    /**
     * The vector of available cursors, containing information about
     * each cursor, its resolution and capabilities.
     */
    QVector<QWindowsTabletDeviceData> m_devices;
    int m_currentDevice{-1};


    /**
     * The widget that got the latest tablet press.
     *
     * See QGuiApplicationPrivate::tabletPressTarget.
     *
     * It should be impossible that one widget would get tablet press and the
     * other tablet release. We keep track of the widget receiving the press
     * event to ensure that this is the case.
     */
    QWidget *targetWidget{0};


    /**
     * This is an inelegant solution to record pen / eraser switches.
     * On the Surface Pro 3 we are only notified of cursor changes at the last minute.
     * The recommended way to handle switches is WT_CSRCHANGE, but that doesn't work
     * unless we save packet ID information, and we cannot change the structure of the
     * PACKETDATA due to Qt restrictions.
     *
     * Furthermore, WT_CSRCHANGE only ever appears *after* we receive the packet.
     */
    UINT currentPkCursor{0};
    bool inlineSwitching{false};  //< Only enable this on SP3 or other devices with the same issue.

};

QWindowsTabletSupport *QTAB = 0;


// This is the WndProc for a single additional hidden window used to collect tablet events.
extern "C" LRESULT QT_WIN_CALLBACK
kisWindowsTabletSupportWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WT_PROXIMITY:
		if (QTAB->translateTabletProximityEvent(wParam, lParam))
			return 0;
        break;
    case WT_PACKET:
        if (QTAB->translateTabletPacketEvent())
            return 0;
        break;
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}


void QWindowsTabletSupport::notifyActivate()
{
    // Cooperate with other tablet applications, but when we get focus, I want to use the tablet.
    const bool result = WINTAB_DLL.wTEnable(m_context, true)
        && WINTAB_DLL.wTOverlap(m_context, true);
    dbgInput << __FUNCTION__ << result;
}


namespace {

    void sendProximityEvent(QWindowsTabletDeviceData tabletData, QEvent::Type type)
    {
        QPointF emptyPos;
        qreal zero = 0.0;
        QTabletEvent e(type, emptyPos, emptyPos, 0, tabletData.currentPointerType,
                       zero, 0, 0, zero, zero, 0, Qt::NoModifier,
                       tabletData.uniqueId, Qt::NoButton, (Qt::MouseButtons)0);
        qApp->sendEvent(qApp->activeWindow(), &e);
    }

    void printContext(const LOGCONTEXT &lc)
    {
        dbgInput << "# Getting current context data:";
        dbgInput << ppVar(lc.lcName);
        dbgInput << ppVar(lc.lcDevice);
        dbgInput << ppVar(lc.lcInOrgX);
        dbgInput << ppVar(lc.lcInOrgY);
        dbgInput << ppVar(lc.lcInExtX);
        dbgInput << ppVar(lc.lcInExtY);
        dbgInput << ppVar(lc.lcOutOrgX);
        dbgInput << ppVar(lc.lcOutOrgY);
        dbgInput << ppVar(lc.lcOutExtX);
        dbgInput << ppVar(lc.lcOutExtY);
        dbgInput << ppVar(lc.lcSysOrgX);
        dbgInput << ppVar(lc.lcSysOrgY);
        dbgInput << ppVar(lc.lcSysExtX);
        dbgInput << ppVar(lc.lcSysExtY);

        dbgInput << "Qt Desktop Geometry" << QApplication::desktop()->geometry();
    }


    static inline QTabletEvent::TabletDevice deviceType(const UINT cursorType)
    {
        if (((cursorType & 0x0006) == 0x0002) && ((cursorType & CursorTypeBitMask) != 0x0902))
            return QTabletEvent::Stylus;
        if (cursorType == 0x4020) // Surface Pro 2 tablet device
            return QTabletEvent::Stylus;
        switch (cursorType & CursorTypeBitMask) {
        case 0x0802:
            return QTabletEvent::Stylus;
        case 0x0902:
            return QTabletEvent::Airbrush;
        case 0x0004:
            return QTabletEvent::FourDMouse;
        case 0x0006:
            return QTabletEvent::Puck;
        case 0x0804:
            return QTabletEvent::RotationStylus;
        default:
            break;
        }
        return QTabletEvent::NoDevice;
    };


    static inline QTabletEvent::PointerType pointerType(unsigned pkCursor)
    {
        switch (pkCursor % 3) { // %3 for dual track
        case 0:
            return QTabletEvent::Cursor;
        case 1:
            return QTabletEvent::Pen;
        case 2:
            return QTabletEvent::Eraser;
        default:
            break;
        }
        return QTabletEvent::UnknownPointer;
    }

    static inline int indexOfDevice(const QVector<QWindowsTabletDeviceData> &devices, qint64 uniqueId)
    {
        for (int i = 0; i < devices.size(); ++i)
            if (devices.at(i).uniqueId == uniqueId)
                return i;
        return -1;
    }

    static QRect mapToNative(const QRect &qRect, int m_factor)
    {
        return QRect(qRect.x() * m_factor, qRect.y() * m_factor, qRect.width() * m_factor, qRect.height() * m_factor);
    }


    HWND createDummyWindow(const QString &className, const wchar_t *windowName, WNDPROC wndProc)
    {
        if (!wndProc)
            wndProc = DefWindowProc;

        WNDCLASSEX wc;
        wc.cbSize = sizeof(WNDCLASSEX);
        wc.style = 0;
        wc.lpfnWndProc = wndProc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = (HINSTANCE)GetModuleHandle(0);
        wc.hCursor = 0;
        wc.hbrBackground = GetSysColorBrush(COLOR_WINDOW);
        wc.hIcon = 0;
        wc.hIconSm = 0;
        wc.lpszMenuName = 0;
        wc.lpszClassName = (wchar_t*)className.utf16();
        ATOM atom = RegisterClassEx(&wc);
        if (!atom)
            qErrnoWarning("Registering tablet fake window class failed.");

        return CreateWindowEx(0, (wchar_t*)className.utf16(),
                              windowName, WS_OVERLAPPED,
                              CW_USEDEFAULT, CW_USEDEFAULT,
                              CW_USEDEFAULT, CW_USEDEFAULT,
                              HWND_MESSAGE, NULL, (HINSTANCE)GetModuleHandle(0), NULL);
    }

    static QPoint mousePosition()
    {
        POINT p;
        GetCursorPos(&p);
        return QPoint(p.x, p.y);
    }

    static inline QEvent::Type mouseEventType(QEvent::Type t)
    {
        return  (t == QEvent::TabletMove    ? QEvent::MouseMove :
                 t == QEvent::TabletPress   ? QEvent::MouseButtonPress :
                 t == QEvent::TabletRelease ? QEvent::MouseButtonRelease :
                 QEvent::None);
    }

    static inline bool isMouseEventType(QEvent::Type t)
    {
        return (t == QEvent::MouseMove ||
                t == QEvent::MouseButtonPress ||
                t == QEvent::MouseButtonRelease);
    }

    static inline int sign(int x)
    {
        return x >= 0 ? 1 : -1;
    }

    /**
     * Windows generates spoofed mouse events for certain rejected tablet
     * events. When those mouse events are unnecessary we catch them with this
     * event filter.
     */
    class EventEater : public QObject {
    public:
        EventEater(QObject *p) : QObject(p) {}

        bool eventFilter(QObject* object, QEvent* event ) {
            bool isMouseEvent = isMouseEventType(event->type());

            if (isMouseEvent && peckish) {
                peckish--;

                if (KisTabletDebugger::instance()->debugEnabled()) {
                    QString pre = QString("[BLOCKED]");
                    QMouseEvent *ev = static_cast<QMouseEvent*>(event);
                    dbgTablet << KisTabletDebugger::instance()->eventToString(*ev,pre);
                }
                return true;
            }
            // else if (isMouseEvent && hungry) {
            //     return true;
            // }
             return false;
        }

        void activate()   { peckish = 5;};
        void deactivate() { peckish = 0;};
        void chow() { peckish = 5; }; // XXX: perhaps this could be customizable

    private:
        bool hungry{false};   // Continue eating mouse strokes
        int  peckish{0};  // Eat a number of mouse events
    };

    EventEater *globalEventEater = 0;

}

inline QPointF QWindowsTabletDeviceData::scaleCoordinates(int coordX, int coordY, const QRect &targetArea) const
{
    const int targetX = targetArea.x();
    const int targetY = targetArea.y();
    const int targetWidth = targetArea.width();
    const int targetHeight = targetArea.height();

    const qreal x = sign(targetWidth) == sign(maxX) ?
        ((coordX - minX) * qAbs(targetWidth) / qAbs(qreal(maxX - minX))) + targetX :
        ((qAbs(maxX) - (coordX - minX)) * qAbs(targetWidth) / qAbs(qreal(maxX - minX))) + targetX;

    const qreal y = sign(targetHeight) == sign(maxY) ?
        ((coordY - minY) * qAbs(targetHeight) / qAbs(qreal(maxY - minY))) + targetY :
        ((qAbs(maxY) - (coordY - minY)) * qAbs(targetHeight) / qAbs(qreal(maxY - minY))) + targetY;

    return QPointF(x, y);
}


void QWindowsTabletSupport::tabletUpdateCursor(const quint64 uniqueId,
                                               const UINT cursorType,
                                               const int pkCursor)
{
    m_currentDevice = indexOfDevice(m_devices, uniqueId);
    if (m_currentDevice < 0) {
        m_currentDevice = m_devices.size();
        m_devices.push_back(tabletInit(uniqueId, cursorType));
    }
    m_devices[m_currentDevice].currentPointerType = pointerType(pkCursor);
    currentPkCursor = pkCursor;

    // Check tablet name to enable Surface Pro 3 workaround.
#ifdef UNICODE
    UINT nameLength = WINTAB_DLL.wTInfo(WTI_DEVICES, DVC_NAME, 0);
    TCHAR* dvcName = new TCHAR[nameLength + 1];
    WINTAB_DLL.wTInfo(WTI_DEVICES, DVC_NAME, dvcName);
    QString qDvcName = QString::fromWCharArray((const wchar_t*)dvcName);
    dbgInput << "DVC_NAME =" << qDvcName;
    if (qDvcName == QString::fromLatin1("N-trig DuoSense device")) {
        dbgInput << "Setting inline switching to true.";
        inlineSwitching = true;
    } else {
        inlineSwitching = false;
    }
    delete[] dvcName;
#endif

}

QWindowsTabletDeviceData QWindowsTabletSupport::tabletInit(const quint64 uniqueId, const UINT cursorType) const
{

    QWindowsTabletDeviceData result;
    result.uniqueId = uniqueId;
    /* browse WinTab's many info items to discover pressure handling. */
    AXIS axis;
    LOGCONTEXT lc;
    /* get the current context for its device variable. */
    WINTAB_DLL.wTGet(m_context, &lc);

    if (KisTabletDebugger::instance()->initializationDebugEnabled()) {
        printContext(lc);
    }

    /* get the size of the pressure axis. */
    WINTAB_DLL.wTInfo(WTI_DEVICES + lc.lcDevice, DVC_NPRESSURE, &axis);
    result.minPressure = int(axis.axMin);
    result.maxPressure = int(axis.axMax);

    WINTAB_DLL.wTInfo(WTI_DEVICES + lc.lcDevice, DVC_TPRESSURE, &axis);
    result.minTanPressure = int(axis.axMin);
    result.maxTanPressure = int(axis.axMax);

    LOGCONTEXT defaultLc;
    /* get default region */
    WINTAB_DLL.wTInfo(WTI_DEFCONTEXT, 0, &defaultLc);
    result.maxX = int(defaultLc.lcInExtX) - int(defaultLc.lcInOrgX);
    result.maxY = int(defaultLc.lcInExtY) - int(defaultLc.lcInOrgY);
    result.maxZ = int(defaultLc.lcInExtZ) - int(defaultLc.lcInOrgZ);
    result.currentDevice = deviceType(cursorType);

    return result;
};


QWindowsTabletSupport::~QWindowsTabletSupport()
{
    WINTAB_DLL.wTClose(m_context);
    DestroyWindow(m_window);
}

QWindowsTabletSupport *QWindowsTabletSupport::create()
{
    if (!WINTAB_DLL.init()) {
        qWarning() << "Failed to initialize Wintab";
        return 0;
    }

    const HWND window = createDummyWindow(QStringLiteral("TabletDummyWindow"),
                                          L"TabletDummyWindow",
                                          kisWindowsTabletSupportWndProc);

    LOGCONTEXT lcMine;
    // build our context from the default context
    WINTAB_DLL.wTInfo(WTI_DEFSYSCTX, 0, &lcMine);
    // Go for the raw coordinates, the tablet event will return good stuff
    lcMine.lcOptions |= CXO_MESSAGES | CXO_CSRMESSAGES;
    lcMine.lcPktData = lcMine.lcMoveMask = PACKETDATA;
    lcMine.lcPktMode = PacketMode;
    lcMine.lcOutOrgX = 0;
    lcMine.lcOutExtX = lcMine.lcInExtX;
    lcMine.lcOutOrgY = 0;
    lcMine.lcOutExtY = -lcMine.lcInExtY;
    const HCTX context = WINTAB_DLL.wTOpen(window, &lcMine, true);
    if (!context) {
        dbgInput << __FUNCTION__ << "Unable to open tablet.";
        DestroyWindow(window);
        return 0;
    }
    // Set the size of the Packet Queue to the correct size
    const int currentQueueSize = WINTAB_DLL.wTQueueSizeGet(context);
    if (currentQueueSize != TabletPacketQSize) {
        if (!WINTAB_DLL.wTQueueSizeSet(context, TabletPacketQSize)) {
            if (!WINTAB_DLL.wTQueueSizeSet(context, currentQueueSize))  {
                qWarning() << "Unable to set queue size on tablet. The tablet will not work.";
                WINTAB_DLL.wTClose(context);
                DestroyWindow(window);
                return 0;
            } // cannot restore old size
        } // cannot set
    } // mismatch
    dbgInput << "Opened tablet context " << context << " on window "
             <<  window << "changed packet queue size " << currentQueueSize
             << "->" <<  TabletPacketQSize;
    return new QWindowsTabletSupport(window, context);
}


bool QWindowsTabletSupport::translateTabletProximityEvent(WPARAM /* wParam */, LPARAM lParam)
{
    if (!LOWORD(lParam)) {
        // dbgInput << "leave proximity for device #" << m_currentDevice;
        sendProximityEvent(m_devices.at(m_currentDevice), QEvent::TabletLeaveProximity);
        // globalEventEater->deactivate();
        return true;
    }
    PACKET proximityBuffer[1]; // we are only interested in the first packet in this case
    const int totalPacks = WINTAB_DLL.wTPacketsGet(m_context, 1, proximityBuffer);
    if (!totalPacks)
        return false;
    UINT pkCursor = proximityBuffer[0].pkCursor;
    UINT physicalCursorId;
    WINTAB_DLL.wTInfo(WTI_CURSORS + pkCursor, CSR_PHYSID, &physicalCursorId);
    UINT cursorType;
    WINTAB_DLL.wTInfo(WTI_CURSORS + pkCursor, CSR_TYPE, &cursorType);
    const qint64 uniqueId = (qint64(cursorType & DeviceIdMask) << 32L) | qint64(physicalCursorId);
    // initializing and updating the cursor should be done in response to
    // WT_CSRCHANGE. We do it in WT_PROXIMITY because some wintab never send
    // the event WT_CSRCHANGE even if asked with CXO_CSRMESSAGES
    tabletUpdateCursor(uniqueId, cursorType, pkCursor);
    // dbgInput << "enter proximity for device #" << m_currentDevice << m_devices.at(m_currentDevice);
    sendProximityEvent(m_devices.at(m_currentDevice), QEvent::TabletEnterProximity);
    // globalEventEater->activate();
    return true;
}

bool QWindowsTabletSupport::translateTabletPacketEvent()
{
    static PACKET localPacketBuf[TabletPacketQSize];  // our own tablet packet queue.
    const int packetCount = WINTAB_DLL.wTPacketsGet(m_context, TabletPacketQSize, &localPacketBuf);
    if (!packetCount || m_currentDevice < 0)
        return false;
    const qreal dpr = qApp->primaryScreen()->devicePixelRatio();

    QWindowsTabletDeviceData tabletData = m_devices.at(m_currentDevice);
    int currentDevice  = tabletData.currentDevice;
    int currentPointerType = tabletData.currentPointerType;

    static Qt::MouseButtons buttons = Qt::NoButton, btnOld, btnChange;

    // The tablet can be used in 2 different modes, depending on its settings:
    // 1) Absolute (pen) mode:
    //    The coordinates are scaled to the virtual desktop (by default). The user
    //    can also choose to scale to the monitor or a region of the screen.
    //    When entering proximity, the tablet driver snaps the mouse pointer to the
    //    tablet position scaled to that area and keeps it in sync.
    // 2) Relative (mouse) mode:
    //    The pen follows the mouse. The constant 'absoluteRange' specifies the
    //    manhattanLength difference for detecting if a tablet input device is in this mode,
    //    in which case we snap the position to the mouse position.
    // It seems there is no way to find out the mode programmatically, the LOGCONTEXT orgX/Y/Ext
    // area is always the virtual desktop.
    const QRect virtualDesktopArea = mapToNative(qApp->primaryScreen()->virtualGeometry(), dpr);

    Qt::KeyboardModifiers keyboardModifiers = QApplication::queryKeyboardModifiers();

    for (int i = 0; i < packetCount; ++i) {
        const PACKET &packet = localPacketBuf[i];

        // Identify press/release from button state changes and translate
        // into Qt events. (see QGuiApplicationPrivate::processTabletEvent)
        buttons = static_cast<Qt::MouseButtons>(localPacketBuf[i].pkButtons);
        bool anyButtonsStillPressed = buttons;
        QEvent::Type type = QEvent::TabletMove;
        if (buttons > btnOld) {
            type = QEvent::TabletPress;
        } else if (buttons < btnOld)  {
            type = QEvent::TabletRelease;
            if (!anyButtonsStillPressed) {
                globalEventEater->deactivate();
            }
        }


        // Pick out the individual button press
        btnChange = btnOld ^ buttons;
        btnOld = buttons;
        Qt::MouseButton button = Qt::NoButton;
        for (int check = Qt::LeftButton; check <= int(Qt::MaxMouseButton); check = check << 1) {
            if (check & btnChange) {
                button = Qt::MouseButton(check);
                break;
            }
        }

        // Older method
        // globalButtonsConverter->convert(btnOld, btnNew, &button, &buttons);


        const int z = currentDevice == QTabletEvent::FourDMouse ? int(packet.pkZ) : 0;

        // This code is to delay the tablet data one cycle to sync with the mouse location.
        QPointF globalPosF = m_oldGlobalPosF;
        m_oldGlobalPosF = tabletData.scaleCoordinates(packet.pkX, packet.pkY, virtualDesktopArea);

        QPoint globalPos = globalPosF.toPoint();

        // Get Mouse Position and compare to tablet info
        QPoint mouseLocation = mousePosition();

        // Positions should be almost the same if we are in absolute
        // mode. If they are not, use the mouse location.
        if ((mouseLocation - globalPos).manhattanLength() > m_absoluteRange) {
            globalPos = mouseLocation;
            globalPosF = globalPos;
        }

        // Older window-targeting code
        // QWindow *target = QGuiApplicationPrivate::tabletPressTarget; // Pass to window that grabbed it.
        //if (!target)
        //	target = QWindowsScreen::windowAt(globalPos, CWP_SKIPINVISIBLE | CWP_SKIPTRANSPARENT);
        //if (!target)
        //	continue;

        QWidget *w = targetWidget;
        if (!w) w  = QApplication::activePopupWidget();
        if (!w) w  = QApplication::activeModalWidget();
        if (!w) w  = qApp->widgetAt(globalPos);
        if (!w) w  = qApp->activeWindow();

        if (type == QEvent::TabletPress) {
            targetWidget = w;
        }
        else if (type == QEvent::TabletRelease && targetWidget) {
            targetWidget = 0;
        }
        const QPoint localPos = w->mapFromGlobal(globalPos);


        const qreal pressureNew = packet.pkButtons &&
            (currentPointerType == QTabletEvent::Pen || currentPointerType == QTabletEvent::Eraser) ?
            tabletData.scalePressure(packet.pkNormalPressure) :
            qreal(0);
        const qreal tangentialPressure = currentDevice == QTabletEvent::Airbrush ?
            tabletData.scaleTangentialPressure(packet.pkTangentPressure) :
            qreal(0);

        int tiltX = 0;
        int tiltY = 0;
        qreal rotation = 0;
        if (m_tiltSupport) {
            // Convert from azimuth and altitude to x tilt and y tilt. What
            // follows is the optimized version. Here are the equations used:
            // X = sin(azimuth) * cos(altitude)
            // Y = cos(azimuth) * cos(altitude)
            // Z = sin(altitude)
            // X Tilt = arctan(X / Z)
            // Y Tilt = arctan(Y / Z)
            const double radAzim = (packet.pkOrientation.orAzimuth / 10.0) * (M_PI / 180);
            const double tanAlt = std::tan((std::abs(packet.pkOrientation.orAltitude / 10.0)) * (M_PI / 180));

            const double degX = std::atan(std::sin(radAzim) / tanAlt);
            const double degY = std::atan(std::cos(radAzim) / tanAlt);
            tiltX = int(degX * (180 / M_PI));
            tiltY = int(-degY * (180 / M_PI));
            rotation = 360.0 - (packet.pkOrientation.orTwist / 10.0);
            if (rotation > 180.0)
                rotation -= 360.0;
        }

        // This is adds *a lot* of noise to the output log
        if (false) {
            dbgInput
                << "Packet #" << (i+1) << '/' << packetCount << "button:" << packet.pkButtons
                << globalPosF << z << "to:" << w << localPos << "(packet" << packet.pkX
                << packet.pkY << ") dev:" << currentDevice << "pointer:"
                << currentPointerType << "P:" << pressureNew << "tilt:" << tiltX << ','
                << tiltY << "tanP:" << tangentialPressure << "rotation:" << rotation;
        }

        const QPointF localPosDip = QPointF(localPos / dpr);
        const QPointF globalPosDip = globalPosF / dpr;



        // Reusable function - closures are your friend!
        auto trySendTabletEvent = [&](QTabletEvent::Type t){
            // First, try sending a tablet event.
            QTabletEvent e(t, localPosDip, globalPosDip, currentDevice, currentPointerType,
                           pressureNew, tiltX, tiltY, tangentialPressure, rotation, z,
                           keyboardModifiers, tabletData.uniqueId, button, buttons);
            qApp->sendEvent(w, &e);

            bool accepted = e.isAccepted();

            // If it's rejected, flush the eventEater send a synthetic mouse event.
            if (accepted) {
                globalEventEater->activate();
            }
            else {
                globalEventEater->deactivate();
                QMouseEvent m(mouseEventType(t), localPosDip, button, buttons, keyboardModifiers);
                qApp->sendEvent(w, &e);
                accepted = m.isAccepted();
            }
            return accepted;
        };

        /**
         * Workaround to deal with "inline" tool switches.
         * These are caused by the eraser trigger button on the Surface Pro 3.
         * We shoot out a tabletUpdateCursor request and a switchInputDevice request.
         */
        if (inlineSwitching && (packet.pkCursor != currentPkCursor)) {

            // Send tablet release event.
            trySendTabletEvent(QTabletEvent::TabletRelease);

            // Read the new cursor info.
            UINT pkCursor = packet.pkCursor;
            UINT physicalCursorId;
            WINTAB_DLL.wTInfo(WTI_CURSORS + pkCursor, CSR_PHYSID, &physicalCursorId);
            UINT cursorType;
            WINTAB_DLL.wTInfo(WTI_CURSORS + pkCursor, CSR_TYPE, &cursorType);
            const qint64 uniqueId = (qint64(cursorType & DeviceIdMask) << 32L) | qint64(physicalCursorId);
            tabletUpdateCursor(uniqueId, cursorType, pkCursor);

            // Update the local loop variables.
            tabletData = m_devices.at(m_currentDevice);
            currentDevice  = deviceType(tabletData.currentDevice);
            currentPointerType = pointerType(pkCursor);

            // Hack. Don't do this right now.
            // dbgInput << "Cursor updated. Requesting input device switch.";
            // KoInputDevice id((QTabletEvent::TabletDevice)currentDevice,
            // 	 (QTabletEvent::PointerType)currentPointerType,
            // 	 uniqueId);
            // KoToolManager::instance()->switchInputDeviceRequested(id);

            trySendTabletEvent(QTabletEvent::TabletPress);
        }
        // Workaround ends here.


        trySendTabletEvent(type);

    } // Loop over packets
    return true;
}




/**
 * Logic to handle a tablet device entering proximity or when switching tools.
 *
 * Made obsolete by tabletInit().
 */

// static void tabletChangeCursor(QWindowsTabletDeviceData &tdd, const UINT newCursor)
// {
//     tdd.currentPointerType = pointerType(newCursor);
//     dbgInput << "WinTab: updating cursor type" << ppVar(newCursor);
//     currentCursor = newCursor;
// }

// static void tabletUpdateCursor(const UINT newCursor)
// {
    /*    UINT csr_physid;
          ptrWTInfo(WTI_CURSORS + newCursor, CSR_PHYSID, &csr_physid);
          UINT csr_type;
          ptrWTInfo(WTI_CURSORS + newCursor, CSR_TYPE, &csr_type);
          const UINT deviceIdMask = 0xFF6; // device type mask && device color mask
          quint64 uniqueId = (csr_type & deviceIdMask);
          uniqueId = (uniqueId << 32) | csr_physid;

          const QTabletCursorInfo *const globalCursorInfo = tCursorInfo();
          bool isInit = !globalCursorInfo->contains(uniqueId);
          if (isInit) {
          tabletInit(uniqueId, csr_type, global_context);
          }

          // Check tablet name to enable Surface Pro 3 workaround.
          #ifdef UNICODE
          UINT nameLength = ptrWTInfo(WTI_DEVICES, DVC_NAME, 0);
          TCHAR* dvcName = new TCHAR[nameLength + 1];
          ptrWTInfo(WTI_DEVICES, DVC_NAME, dvcName);
          QString qDvcName = QString::fromWCharArray((const wchar_t*)dvcName);
          delete dvcName;
          dbgInput << "DVC_NAME =" << qDvcName;
          if (qDvcName == QString::fromLatin1("N-trig DuoSense device")) {
          inlineSwitching = true;
          } else {
          inlineSwitching = false;
          }
          #endif

          currentTabletPointer = globalCursorInfo->value(uniqueId);
          tabletChangeCursor(currentTabletPointer, newCursor);

          BYTE logicalButtons[32];
          memset(logicalButtons, 0, 32);
          ptrWTInfo(WTI_CURSORS + newCursor, CSR_SYSBTNMAP, &logicalButtons);
          currentTabletPointer.buttonsMap[0x1] = logicalButtons[0];
          currentTabletPointer.buttonsMap[0x2] = logicalButtons[1];
          currentTabletPointer.buttonsMap[0x4] = logicalButtons[2];


          if (isInit && KisTabletDebugger::instance()->initializationDebugEnabled()) {
          dbgInput << "--------------------------";
          dbgInput << "--- Tablet buttons map ---";
          for (int i = 0; i < 16; i++) {
          dbgInput << "( 1 <<" << 2*i << ")" << "->" << logicalButtons[2*i]
          << "( 1 <<" << 2*i+1 << ")" << "->" << logicalButtons[2*i+1];
          }
          dbgInput << "--------------------------";
          }
    */
// }




/**
 * This is a default implementation of a class for converting the
 * WinTab value of the buttons pressed to the Qt buttons. This class
 * may be substituted from the UI.
 */
struct DefaultButtonsConverter : public KisTabletSupportWin::ButtonsConverter
{
    void convert(DWORD btnOld, DWORD btnNew,
                 Qt::MouseButton *button,
                 Qt::MouseButtons *buttons) {

        int pressedButtonValue = btnNew ^ btnOld;

        *button = buttonValueToEnum(pressedButtonValue);

        *buttons = Qt::NoButton;
        for (int i = 0; i < 3; i++) {
            int btn = 0x1 << i;

            if (btn & btnNew) {
                Qt::MouseButton convertedButton =
                    buttonValueToEnum(btn);

                *buttons |= convertedButton;

                /**
                 * If a button that is present in hardware input is
                 * mapped to a Qt::NoButton, it means that it is going
                 * to be eaten by the driver, for example by its
                 * "Pan/Scroll" feature. Therefore we shouldn't handle
                 * any of the events associated to it. So just return
                 * Qt::NoButton here.
                 */
                if (convertedButton == Qt::NoButton) {
                    *button = Qt::NoButton;
                    *buttons = Qt::NoButton;
                    break;
                }
            }
        }
    }

private:
    Qt::MouseButton buttonValueToEnum(DWORD button) {
        const int leftButtonValue = 0x1;
        const int middleButtonValue = 0x2;
        const int rightButtonValue = 0x4;
        const int doubleClickButtonValue = 0x7;

        //button = QTAB->currentTabletPointer.buttonsMap.value(button);
        button = Qt::NoButton;

        return button == leftButtonValue ? Qt::LeftButton :
            button == rightButtonValue ? Qt::RightButton :
            button == doubleClickButtonValue ? Qt::MiddleButton :
            button == middleButtonValue ? Qt::MiddleButton :
            button ? Qt::LeftButton /* fallback item */ :
            Qt::NoButton;
    }
};

static KisTabletSupportWin::ButtonsConverter *globalButtonsConverter =
    new DefaultButtonsConverter();



/**
 * Obsolete
 */
static bool dialogOpen = false;  //< KisTabletSupportWin is not a Q_OBJECT and can't accept dialog signals

bool translateTabletEvent(const MSG &msg, PACKET *localPacketBuf, int numPackets)
{
    return 0;
    /*
    Q_UNUSED(msg);
    POINT ptNew;
    static DWORD btnNew, btnOld, btnChange;
    qreal prsNew;
    ORIENTATION ort;
    int i,
        tiltX,
        tiltY;
    bool sendEvent = false;
    KisTabletEvent::ExtraEventType t;
    int z = 0;
    qreal rotation = 0.0;
    qreal tangentialPressure;
    auto currentTabletPointer = QTAB->currentTabletPointer;

    Qt::KeyboardModifiers modifiers = QApplication::queryKeyboardModifiers();

    for (i = 0; i < numPackets; i++) {
        btnOld = btnNew;
        btnNew = localPacketBuf[i].pkButtons;
        btnChange = btnOld ^ btnNew;

        bool buttonPressed = btnChange && btnNew > btnOld;
        bool buttonReleased = btnChange && btnNew < btnOld;
        bool anyButtonsStillPressed = btnNew;

        ptNew.x = UINT(localPacketBuf[i].pkX);
        ptNew.y = UINT(localPacketBuf[i].pkY);
        z = UINT(localPacketBuf[i].pkZ);

        prsNew = 0.0;

        // QPointF hiResGlobal = QTAB->currentTabletPointer.scaleCoord(ptNew.x, ptNew.y,
        //                                                     currentTabletPointer.sysOrgX, currentTabletPointer.sysExtX,
        //                                                     currentTabletPointer.sysOrgY, currentTabletPointer.sysExtY);
        QPointF hiResGlobal(0, 0);

        if (KisTabletDebugger::instance()->debugRawTabletValues()) {
            dbgInput << "WinTab (RC):"
                // << "Dsk:"  << QRect(currentTabletPointer->sysOrgX, currentTabletPointer->sysOrgY, currentTabletPointer->sysExtX,  currentTabletPointer->sysExtY)
                     << "Raw:" << ptNew.x << ptNew.y
                     << "Scaled:" << hiResGlobal;

            dbgInput << "WinTab (BN):"
                     << "old:" << btnOld
                     << "new:" << btnNew
                     << "diff:" << (btnOld ^ btnNew)
                     << (buttonPressed ? "P" : buttonReleased ? "R" : ".");
        }


        Qt::MouseButton button = Qt::NoButton;
        Qt::MouseButtons buttons;

        globalButtonsConverter->convert(btnOld, btnNew, &button, &buttons);

        t = KisTabletEvent::TabletMoveEx;
        if (buttonPressed && button != Qt::NoButton) {
            t = KisTabletEvent::TabletPressEx;
        } else if (buttonReleased && button != Qt::NoButton) {
            t = KisTabletEvent::TabletReleaseEx;
        }

        if (anyButtonsStillPressed) {
            if (currentTabletPointer->currentPointerType == QTabletEvent::Pen || currentTabletPointer->currentPointerType == QTabletEvent::Eraser)
                prsNew = localPacketBuf[i].pkNormalPressure
                    / qreal(currentTabletPointer->maxPressure
                            - currentTabletPointer->minPressure);
            else
                prsNew = 0;
        }

        QPoint globalPos(qRound(hiResGlobal.x()), qRound(hiResGlobal.y()));

        // make sure the tablet event get's sent to the proper widget...
        QWidget *w = 0;

        //Find the appropriate window in an order of preference

        if (!w) w = qApp->widgetAt(globalPos);
        if (!w) w = qApp->activeWindow();

        QWidget *parentOverride = 0;

        if (!parentOverride) parentOverride = qApp->activePopupWidget();
        if (!parentOverride) parentOverride = qApp->activeModalWidget();

        if (!w || (parentOverride && !parentOverride->isAncestorOf(w))) {
            w = parentOverride;
        }

        // Stubbed
        QWidget *kis_tablet_pressed = 0;

        if (kis_tablet_pressed) {
            w = kis_tablet_pressed;
        }

        if (t == KisTabletEvent::TabletPressEx && !kis_tablet_pressed) {
            kis_tablet_pressed = w;
        }

        if (!anyButtonsStillPressed) {
            kis_tablet_pressed = 0;
        }

        QPoint localPos = w->mapFromGlobal(globalPos);
        if (currentTabletPointer->currentDevice == QTabletEvent::Airbrush) {
            tangentialPressure = localPacketBuf[i].pkTangentPressure
                / qreal(currentTabletPointer->maxTanPressure
                        - currentTabletPointer->minTanPressure);
        } else {
            tangentialPressure = 0.0;
        }

        if (!QTAB->hasTiltSupport()) {
            tiltX = tiltY = 0;
            rotation = 0.0;
        } else {
            ort = localPacketBuf[i].pkOrientation;
            // convert from azimuth and altitude to x tilt and y tilt
            // what follows is the optimized version.  Here are the equations
            // I used to get to this point (in case things change :)
            // X = sin(azimuth) * cos(altitude)
            // Y = cos(azimuth) * cos(altitude)
            // Z = sin(altitude)
            // X Tilt = arctan(X / Z)
            // Y Tilt = arctan(Y / Z)
            double radAzim = (ort.orAzimuth / 10) * (Q_PI / 180);
            //double radAlt = abs(ort.orAltitude / 10) * (Q_PI / 180);
            double tanAlt = tan((abs(ort.orAltitude / 10)) * (Q_PI / 180));

            double degX = atan(sin(radAzim) / tanAlt);
            double degY = atan(cos(radAzim) / tanAlt);
            tiltX = int(degX * (180 / Q_PI));
            tiltY = int(-degY * (180 / Q_PI));

            // Rotation is measured in degrees. Axis inverted to fit
            // the coordinate system of the Linux driver.
            rotation = (360 - 1) - ort.orTwist / 10;
        }

        if (KisTabletDebugger::instance()->debugRawTabletValues()) {
            ort = localPacketBuf[i].pkOrientation;

            dbgInput << "WinTab (RS):"
                     << "NP:" << localPacketBuf[i].pkNormalPressure
                     << "TP:" << localPacketBuf[i].pkTangentPressure
                     << "Az:" << ort.orAzimuth
                     << "Alt:" << ort.orAltitude
                     << "Twist:" << ort.orTwist;
        }

        // Workaround to deal with "inline" tool switches.
        // These are caused by the eraser trigger button on the Surface Pro 3.
        // We shoot out a tabletUpdateCursor request and a switchInputDevice request.

        if (inlineSwitching && !dialogOpen && (localPacketBuf[i].pkCursor != currentCursor)) {

            tabletUpdateCursor(localPacketBuf[i].pkCursor);
            KoInputDevice id(QTabletEvent::TabletDevice(currentTabletPointer->currentDevice),
                             QTabletEvent::PointerType(currentTabletPointer->currentPointerType),
                             currentTabletPointer->uniqueId);
            KoToolManager::instance()->switchInputDeviceRequested(id);
        }

        KisTabletEvent e(t, localPos, globalPos, hiResGlobal, currentTabletPointer->currentDevice,
                         currentTabletPointer->currentPointerType, prsNew, tiltX, tiltY,
                         tangentialPressure, rotation, z, modifiers, currentTabletPointer->uniqueId,
                         button, buttons);

        if (button == Qt::NoButton &&
            (t == KisTabletEvent::TabletPressEx ||
             t == KisTabletEvent::TabletReleaseEx)) {


            // Eat events which do not correspond to any mouse
            // button. This can happen when the user assinged a stylus
            // key to e.g. some keyboard key

            e.accept();
        } else {
            e.ignore();
            sendEvent = qApp->sendEvent(w, &e);
        }

        if (e.isAccepted()) {
            globalEventEater->chow();
        } else {
            QTabletEvent t = e.toQTabletEvent();
            qApp->sendEvent(w,  &t);
        }
    }
    return sendEvent;
    */
}

void KisTabletSupportWin::init()
{
    globalEventEater = new EventEater(qApp);
    QTAB = QWindowsTabletSupport::create();
    qApp->installEventFilter(globalEventEater);
}

void KisTabletSupportWin::setButtonsConverter(ButtonsConverter *buttonsConverter)
{
    globalButtonsConverter = buttonsConverter;
}

bool KisTabletSupportWin::nativeEventFilter(const QByteArray &/*eventType*/, void *message, long *result)
{
  /*
	MSG *msg = static_cast<MSG*>(message);
    Q_UNUSED(result);

    static bool mouseEnteredFlag = false;

    switch(msg->message){
    case WT_CTXOPEN:
        global_context = reinterpret_cast<HCTX>(msg->wParam);
        break;
    case WT_CTXCLOSE:
        global_context = 0;
        break;
    case WM_ACTIVATE: {

        // Workaround for a focus bug by Qt.
        // Looks like modal windows do not grab focus on Windows. The
        // parent widget will still be regarded as a focusWidget()
        // although it gets no events. So notify the pure parent that
        // he is not in focus anymore.

        QWidget *modalWidget = QApplication::activeModalWidget();
        if (modalWidget) {
            QWidget *focusWidget = QApplication::focusWidget();
            if (focusWidget) {
                bool active = msg->wParam == WA_ACTIVE || msg->wParam == WA_CLICKACTIVE;
                QFocusEvent focusEvent(active ? QEvent::FocusIn : QEvent::FocusOut);
                QApplication::sendEvent(focusWidget, &focusEvent);
            }
        }
        break;
    }
    case WM_MOUSELEAVE:
        mouseEnteredFlag = false;
        break;
    case WM_MOUSEMOVE:
        if (global_context && !mouseEnteredFlag) {
            WINTAB_DLL.wTOverlap(global_context, true);
            mouseEnteredFlag = true;
        }
        break;

    case WT_PROXIMITY: {
        PACKET proximityBuffer[1]; // we are only interested in the first packet in this case
        const int totalPacks = WINTAB_DLL.wTPacketsGet(global_context, 1, proximityBuffer);
        if (totalPacks == 1) {
            // tabletUpdateCursor(proximityBuffer[0].pkCursor);
        }
        break;
    }
    case WT_PACKET: {
        Q_ASSERT(global_context);
        int nPackets;
        if ((nPackets = WINTAB_DLL.wTPacketsGet(global_context, QT_TABLET_NPACKETQSIZE, &globalPacketBuf))) {
            return translateTabletEvent(*msg, globalPacketBuf, nPackets);
        }

        break;
    }
    }
	*/

    return false;

}
