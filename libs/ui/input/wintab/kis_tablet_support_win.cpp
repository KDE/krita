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


#include "kis_tablet_support_win_p.h"

#include "kis_tablet_support_win.h"

#include <kis_debug.h>
#include <QApplication>
#include <QGuiApplication>
#include <QDesktopWidget>

// #include <qpa/qwindowsysteminterface.h>
// #include <qpa/qplatformscreen.h>
// #include <private/qguiapplication_p.h>

#include <QPointer>
#include <QScreen>
#include <QWidget>
#include <QLibrary>
#include <cmath>
#define Q_PI M_PI

#include <input/kis_extended_modifiers_mapper.h>
#include <input/kis_tablet_debugger.h>

// For "inline tool switches"
#include <KoToolManager.h>
#include <KoInputDevice.h>
#include "kis_screen_size_choice_dialog.h"


// NOTE: we stub out qwindowcontext.cpp::347 to disable Qt's own tablet support.

// Note: The definition of the PACKET structure in pktdef.h depends on this define.
#define PACKETDATA (PK_X | PK_Y | PK_BUTTONS | PK_TIME | PK_NORMAL_PRESSURE | PK_TANGENT_PRESSURE | PK_ORIENTATION | PK_CURSOR | PK_Z)
#include "pktdef.h"


QT_BEGIN_NAMESPACE

enum {
    PacketMode = 0,
    TabletPacketQSize = 128,
    DeviceIdMask = 0xFF6, // device type mask && device color mask
    CursorTypeBitMask = 0x0F06 // bitmask to find the specific cursor type (see Wacom FAQ)
};

/*
 *
 * Krita extensions begin here
 *
 *
 */

QWindowsTabletSupport *QTAB = 0;
static QPointer<QWidget> targetWindow = 0; //< Window receiving last tablet event
static QPointer<QWidget> qt_tablet_target = 0; //< Widget receiving last tablet event
static bool dialogOpen = false;  //< KisTabletSupportWin is not a Q_OBJECT and can't accept dialog signals

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


void printContext(const LOGCONTEXT &lc)
{
    dbgTablet << "# Getting current context data:";
    dbgTablet << ppVar(lc.lcName);
    dbgTablet << ppVar(lc.lcDevice);
    dbgTablet << ppVar(lc.lcInOrgX);
    dbgTablet << ppVar(lc.lcInOrgY);
    dbgTablet << ppVar(lc.lcInExtX);
    dbgTablet << ppVar(lc.lcInExtY);
    dbgTablet << ppVar(lc.lcOutOrgX);
    dbgTablet << ppVar(lc.lcOutOrgY);
    dbgTablet << ppVar(lc.lcOutExtX);
    dbgTablet << ppVar(lc.lcOutExtY);
    dbgTablet << ppVar(lc.lcSysOrgX);
    dbgTablet << ppVar(lc.lcSysOrgY);
    dbgTablet << ppVar(lc.lcSysExtX);
    dbgTablet << ppVar(lc.lcSysExtY);

    dbgTablet << "Qt Desktop Geometry" << QApplication::desktop()->geometry();
}


static QRect mapToNative(const QRect &qRect, int m_factor)
{
    return QRect(qRect.x() * m_factor, qRect.y() * m_factor, qRect.width() * m_factor, qRect.height() * m_factor);
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

static QPoint mousePosition()
{
    POINT p;
    GetCursorPos(&p);
    return QPoint(p.x, p.y);
}

QWindowsWinTab32DLL QWindowsTabletSupport::m_winTab32DLL;

bool KisTabletSupportWin::init()
{
    if (!QWindowsTabletSupport::m_winTab32DLL.init()) {
        qWarning() << "Failed to initialize Wintab";
        return false;
    }

    QTAB = QWindowsTabletSupport::create();

    // Refresh tablet context after tablet rotated, screen added, etc.
    QObject::connect(qApp->primaryScreen(), &QScreen::geometryChanged,
                     [=](const QRect & geometry){
                         delete QTAB;
                         QTAB = QWindowsTabletSupport::create();
                     });
    return true;
}


// Derived from qwidgetwindow.
//
// The work done by processTabletEvent from qguiapplicationprivate is divided
// between here and translateTabletPacketEvent.
static void handleTabletEvent(QWidget *windowWidget, const QPointF &local, const QPointF &global,
                              int device, int pointerType, Qt::MouseButton button, Qt::MouseButtons buttons,
                              qreal pressure,int xTilt, int yTilt, qreal tangentialPressure, qreal rotation,
                              int z, qint64 uniqueId, Qt::KeyboardModifiers modifiers, QEvent::Type type, LONG time)
{

    // Lock in target window
    if (type == QEvent::TabletPress) {
        targetWindow = windowWidget;
        dbgInput << "Locking target window" << targetWindow;
    } else if ((type == QEvent::TabletRelease || buttons == Qt::NoButton) && (targetWindow != 0)) {
        dbgInput << "Releasing target window" << targetWindow;
        targetWindow = 0;
    }
    if (!windowWidget) // Should never happen
        return;


    // We do this instead of constructing the event e beforehand
    const QPoint localPos = local.toPoint();
    const QPoint globalPos = global.toPoint();

    if (type == QEvent::TabletPress) {
        QWidget *widget = windowWidget->childAt(localPos);
        if (!widget)
            widget = windowWidget;
        qt_tablet_target = widget;
    }

    QWidget *finalDestination = qt_tablet_target;
    if (!finalDestination) {
        finalDestination = windowWidget->childAt(localPos);
    }


    if ((type == QEvent::TabletRelease || buttons == Qt::NoButton) && (qt_tablet_target != 0)) {
        dbgInput << "releasing tablet target" << qt_tablet_target;
        qt_tablet_target = 0;
    }

    if (finalDestination) {
        // The event was specified relative to windowWidget, so we remap it
        QPointF delta = global - globalPos;
        QPointF mapped = finalDestination->mapFromGlobal(global.toPoint()) + delta;
        QTabletEvent ev(type, mapped, global, device, pointerType, pressure, xTilt, yTilt,
                        tangentialPressure, rotation, z, modifiers, uniqueId, button, buttons);
        ev.setTimestamp(time);
        QGuiApplication::sendEvent(finalDestination, &ev);


        if (ev.isAccepted()) {
            // dbgTablet << "Tablet event" << type << "accepted" << "by target widget" << finalDestination;
        }
        else {
            // Turn off eventEater send a synthetic mouse event.
            // dbgTablet << "Tablet event" << type << "rejected; sending mouse event to" << finalDestination;
            qt_tablet_target = 0;

            // We shouldn't ever get a widget accepting a tablet event from this
            // call, so we won't worry about any interactions with our own
            // widget-locking code.
            // QWindow *target = platformScreen->topLevelAt(globalPos);
            // if (!target) return;
            // QPointF windowLocal = global - QPointF(target->mapFromGlobal(QPoint())) + delta;
            // QWindowSystemInterface::handleTabletEvent(target, ev.timestamp(), windowLocal,
            //                                           global, device, pointerType,
            //                                           buttons, pressure, xTilt, yTilt,
            //                                           tangentialPressure, rotation, z,
            //                                           uniqueId, modifiers);

        }
    } else {
        qt_tablet_target = 0;
        targetWindow = 0;
    }
}

/**
 * This is a default implementation of a class for converting the
 * WinTab value of the buttons pressed to the Qt buttons. This class
 * may be substituted from the UI.
 */
struct DefaultButtonsConverter
{
    void convert(DWORD btnOld, DWORD btnNew,
                 Qt::MouseButton *button,
                 Qt::MouseButtons *buttons,
                 const QWindowsTabletDeviceData &tdd) {

        int pressedButtonValue = btnNew ^ btnOld;

        *button = buttonValueToEnum(pressedButtonValue, tdd);

        *buttons = Qt::NoButton;
        for (int i = 0; i < 3; i++) {
            int btn = 0x1 << i;

            if (btn & btnNew) {
                Qt::MouseButton convertedButton =
                    buttonValueToEnum(btn, tdd);

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

                    /**
                     * Sometimes the driver-handled shortcuts are just
                     * keyboard modifiers, so ideally we should handle
                     * them as well. The problem is that we cannot
                     * know if the shortcut was a pan/zoom action or a
                     * shortcut. So here we use a "hackish" approach.
                     * We just check if any modifier has been pressed
                     * and, if so, pass the button to Krita. Of
                     * course, if the driver uses some really complex
                     * shortcuts like "Shift + stylus btn" to generate
                     * some recorded shortcut, it will not work. But I
                     * guess it will be ok for the most of the
                     * usecases.
                     *
                     * WARNING: this hack will *not* work if you bind
                     *          any non-modifier key to the stylus
                     *          button, e.g. Space.
                     */

                    const Qt::KeyboardModifiers keyboardModifiers = QApplication::queryKeyboardModifiers();

                    if (KisTabletDebugger::instance()->shouldEatDriverShortcuts() ||
                        keyboardModifiers == Qt::NoModifier) {

                        *button = Qt::NoButton;
                        *buttons = Qt::NoButton;
                        break;
                    }
                }
            }
        }
    }

private:
    Qt::MouseButton buttonValueToEnum(DWORD button,
                                      const QWindowsTabletDeviceData &tdd) {
        const int leftButtonValue = 0x1;
        const int middleButtonValue = 0x2;
        const int rightButtonValue = 0x4;
        const int doubleClickButtonValue = 0x7;

        button = tdd.buttonsMap.value(button);

        return button == leftButtonValue ? Qt::LeftButton :
            button == rightButtonValue ? Qt::RightButton :
            button == doubleClickButtonValue ? Qt::MiddleButton :
            button == middleButtonValue ? Qt::MiddleButton :
            button ? Qt::LeftButton /* fallback item */ :
            Qt::NoButton;
    }
};


static DefaultButtonsConverter *globalButtonsConverter =
    new DefaultButtonsConverter();

/*
 *
 * Krita extensions end here
 *
 *
 */

// This is the WndProc for a single additional hidden window used to collect tablet events.
extern "C" LRESULT QT_WIN_CALLBACK qWindowsTabletSupportWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
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


// Scale tablet coordinates to screen coordinates.

static inline int sign(int x)
{
    return x >= 0 ? 1 : -1;
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


/*!
  \class QWindowsWinTab32DLL QWindowsTabletSupport
  \brief Functions from wintabl32.dll shipped with WACOM tablets used by QWindowsTabletSupport.

  \internal
  \ingroup qt-lighthouse-win
*/
bool QWindowsWinTab32DLL::init()
{
    if (wTInfo)
        return true;
    QLibrary library(QStringLiteral("wintab32"));
    if (!library.load()) {
        qWarning() << QString("Could not load wintab32 dll: %1").arg(library.errorString());
        return false;
    }
    wTOpen         = (PtrWTOpen)         library.resolve("WTOpenW");
    wTClose        = (PtrWTClose)        library.resolve("WTClose");
    wTInfo         = (PtrWTInfo)         library.resolve("WTInfoW");
    wTEnable       = (PtrWTEnable)       library.resolve("WTEnable");
    wTOverlap      = (PtrWTOverlap)      library.resolve("WTOverlap");
    wTPacketsGet   = (PtrWTPacketsGet)   library.resolve("WTPacketsGet");
    wTGet          = (PtrWTGet)          library.resolve("WTGetW");
    wTQueueSizeGet = (PtrWTQueueSizeGet) library.resolve("WTQueueSizeGet");
    wTQueueSizeSet = (PtrWTQueueSizeSet) library.resolve("WTQueueSizeSet");

    if (wTOpen && wTClose && wTInfo && wTEnable && wTOverlap && wTPacketsGet && wTQueueSizeGet && wTQueueSizeSet) {
        return true;
    }
    qWarning() << "Could not resolve the following symbols:\n"
               << "\t wTOpen" << wTOpen << "\n"
               << "\t wtClose" << wTClose << "\n"
               << "\t wtInfo" << wTInfo << "\n"
               << "\t wTEnable" << wTEnable << "\n"
               << "\t wTOverlap" << wTOverlap << "\n"
               << "\t wTPacketsGet" << wTPacketsGet << "\n"
               << "\t wTQueueSizeGet" << wTQueueSizeGet << "\n"
               << "\t wTQueueSizeSet" << wTQueueSizeSet << "\n";
    return false;
}



/*!
  \class QWindowsTabletSupport
  \brief Tablet support for Windows.

  Support for WACOM tablets.

  \sa http://www.wacomeng.com/windows/docs/Wintab_v140.htm

  \internal
  \since 5.2
  \ingroup qt-lighthouse-win
*/

QWindowsTabletSupport::QWindowsTabletSupport(HWND window, HCTX context)
    : m_window(window)
    , m_context(context)
    , m_absoluteRange(20)
    , m_tiltSupport(false)
    , m_currentDevice(-1)
{
    AXIS orientation[3];
    // Some tablets don't support tilt, check if it is possible,
    if (QWindowsTabletSupport::m_winTab32DLL.wTInfo(WTI_DEVICES, DVC_ORIENTATION, &orientation))
        m_tiltSupport = orientation[0].axResolution && orientation[1].axResolution;
}

QWindowsTabletSupport::~QWindowsTabletSupport()
{
    QWindowsTabletSupport::m_winTab32DLL.wTClose(m_context);
    DestroyWindow(m_window);
}

QWindowsTabletSupport *QWindowsTabletSupport::create()
{
    const HWND window = createDummyWindow(QStringLiteral("TabletDummyWindow"),
                                          L"TabletDummyWindow",
                                          qWindowsTabletSupportWndProc);

    LOGCONTEXT lcMine;
    // build our context from the default context
    QWindowsTabletSupport::m_winTab32DLL.wTInfo(WTI_DEFSYSCTX, 0, &lcMine);
    // Go for the raw coordinates, the tablet event will return good stuff. The
    // defaults for lcOut rectangle are the desktop dimensions in pixels, which
    // means Wintab will do lossy rounding. Instead we specify this trivial
    // scaling for the output context, then do the scaling ourselves later to
    // obtain higher resolution coordinates.
    lcMine.lcOptions |= CXO_MESSAGES | CXO_CSRMESSAGES;
    lcMine.lcPktData = lcMine.lcMoveMask = PACKETDATA;
    lcMine.lcPktMode = PacketMode;
    lcMine.lcOutOrgX = 0;
    lcMine.lcOutExtX = lcMine.lcInExtX;
    lcMine.lcOutOrgY = 0;
    lcMine.lcOutExtY = -lcMine.lcInExtY;
    const HCTX context = QWindowsTabletSupport::m_winTab32DLL.wTOpen(window, &lcMine, true);
    if (!context) {
        dbgTablet << __FUNCTION__ << "Unable to open tablet.";
        DestroyWindow(window);
        return 0;
    }
    // Set the size of the Packet Queue to the correct size
    const int currentQueueSize = QWindowsTabletSupport::m_winTab32DLL.wTQueueSizeGet(context);
    if (currentQueueSize != TabletPacketQSize) {
        if (!QWindowsTabletSupport::m_winTab32DLL.wTQueueSizeSet(context, TabletPacketQSize)) {
            if (!QWindowsTabletSupport::m_winTab32DLL.wTQueueSizeSet(context, currentQueueSize))  {
                qWarning() << "Unable to set queue size on tablet. The tablet will not work.";
                QWindowsTabletSupport::m_winTab32DLL.wTClose(context);
                DestroyWindow(window);
                return 0;
            } // cannot restore old size
        } // cannot set
    } // mismatch
    dbgTablet << "Opened tablet context " << context << " on window "
             <<  window << "changed packet queue size " << currentQueueSize
             << "->" <<  TabletPacketQSize;
    return new QWindowsTabletSupport(window, context);
}


unsigned QWindowsTabletSupport::options() const
{
    UINT result = 0;
    m_winTab32DLL.wTInfo(WTI_INTERFACE, IFC_CTXOPTIONS, &result);
    return result;
}


QString QWindowsTabletSupport::description() const
{
    const unsigned size = m_winTab32DLL.wTInfo(WTI_INTERFACE, IFC_WINTABID, 0);
    if (!size)
        return QString();
    QVarLengthArray<TCHAR> winTabId(size + 1);
    m_winTab32DLL.wTInfo(WTI_INTERFACE, IFC_WINTABID, winTabId.data());
    WORD implementationVersion = 0;
    m_winTab32DLL.wTInfo(WTI_INTERFACE, IFC_IMPLVERSION, &implementationVersion);
    WORD specificationVersion = 0;
    m_winTab32DLL.wTInfo(WTI_INTERFACE, IFC_SPECVERSION, &specificationVersion);
    const unsigned opts = options();
    QString result = QString::fromLatin1("%1 specification: v%2.%3 implementation: v%4.%5 options: 0x%6")
        .arg(QString::fromWCharArray(winTabId.data()))
        .arg(specificationVersion >> 8).arg(specificationVersion & 0xFF)
        .arg(implementationVersion >> 8).arg(implementationVersion & 0xFF)
        .arg(opts, 0, 16);
    if (opts & CXO_MESSAGES)
        result += QStringLiteral(" CXO_MESSAGES");
    if (opts & CXO_CSRMESSAGES)
        result += QStringLiteral(" CXO_CSRMESSAGES");
    if (m_tiltSupport)
        result += QStringLiteral(" tilt");
    return result;
}

void QWindowsTabletSupport::notifyActivate()
{
    // Cooperate with other tablet applications, but when we get focus, I want to use the tablet.
    const bool result = QWindowsTabletSupport::m_winTab32DLL.wTEnable(m_context, true)
                        && QWindowsTabletSupport::m_winTab32DLL.wTOverlap(m_context, true);
    dbgTablet << __FUNCTION__ << result;
}

static inline int indexOfDevice(const QVector<QWindowsTabletDeviceData> &devices, qint64 uniqueId)
{
    for (int i = 0; i < devices.size(); ++i)
        if (devices.at(i).uniqueId == uniqueId)
            return i;
    return -1;
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

QDebug operator<<(QDebug d, const QWindowsTabletDeviceData &t)
{
    d << "TabletDevice id:" << t.uniqueId << " pressure: " << t.minPressure
      << ".." << t.maxPressure << " tan pressure: " << t.minTanPressure << ".."
      << t.maxTanPressure << " area:" << t.minX << t.minY << t.minZ
      << ".." << t.maxX << t.maxY << t.maxZ << " device " << t.currentDevice
      << " pointer " << t.currentPointerType;
    return d;
}

QWindowsTabletDeviceData QWindowsTabletSupport::tabletInit(const quint64 uniqueId, const UINT cursorType) const
{

    QWindowsTabletDeviceData result;
    result.uniqueId = uniqueId;
    /* browse WinTab's many info items to discover pressure handling. */
    AXIS axis;
    LOGCONTEXT lc;
    /* get the current context for its device variable. */
    QWindowsTabletSupport::m_winTab32DLL.wTGet(m_context, &lc);

    if (KisTabletDebugger::instance()->initializationDebugEnabled()) {
        printContext(lc);
    }

    /* get the size of the pressure axis. */
    QWindowsTabletSupport::m_winTab32DLL.wTInfo(WTI_DEVICES + lc.lcDevice, DVC_NPRESSURE, &axis);
    result.minPressure = int(axis.axMin);
    result.maxPressure = int(axis.axMax);

    QWindowsTabletSupport::m_winTab32DLL.wTInfo(WTI_DEVICES + lc.lcDevice, DVC_TPRESSURE, &axis);
    result.minTanPressure = int(axis.axMin);
    result.maxTanPressure = int(axis.axMax);

    result.minX = int(lc.lcOutOrgX);
    result.maxX = int(lc.lcOutExtX) + int(lc.lcOutOrgX);
    result.minY = int(lc.lcOutOrgY);
    result.maxY = -int(lc.lcOutExtY) + int(lc.lcOutOrgY);
    // These are set to 0 when we opened the tablet context in QWindowsTabletSupport::create
    KIS_SAFE_ASSERT_RECOVER_NOOP(lc.lcOutOrgX == 0);
    KIS_SAFE_ASSERT_RECOVER_NOOP(lc.lcOutOrgY == 0);

    result.maxZ = int(lc.lcOutExtZ) - int(lc.lcOutOrgZ);
    result.currentDevice = deviceType(cursorType);

    // Define a rectangle representing the whole screen as seen by Wintab.
    QRect qtDesktopRect = QApplication::desktop()->geometry();
    QRect wintabDesktopRect(lc.lcSysOrgX, lc.lcSysOrgY,
                            lc.lcSysExtX, lc.lcSysExtY);
    qDebug() << ppVar(qtDesktopRect);
    qDebug() << ppVar(wintabDesktopRect);

    // Show screen choice dialog
    if (!dialogOpen) {
        KisScreenSizeChoiceDialog dlg(0,
                                      wintabDesktopRect,
                                      qtDesktopRect);

        KisExtendedModifiersMapper mapper;
        KisExtendedModifiersMapper::ExtendedModifiers modifiers =
            mapper.queryExtendedModifiers();

        if (modifiers.contains(Qt::Key_Shift) ||
            (!dlg.canUseDefaultSettings() &&
             qtDesktopRect != wintabDesktopRect)) {

            dialogOpen = true;
            dlg.exec();
        }

        result.virtualDesktopArea = dlg.screenRect();
        dialogOpen = false;
    } else {
        // This branch should've been explicitly prevented.
        KIS_SAFE_ASSERT_RECOVER_NOOP(!dialogOpen);
        warnTablet << "Trying to init a WinTab device while screen resolution dialog is active, this should not happen!";
        warnTablet << "Tablet coordinates could be wrong as a result.";
        result.virtualDesktopArea = qtDesktopRect;
    }

    return result;
};




bool QWindowsTabletSupport::translateTabletProximityEvent(WPARAM /* wParam */, LPARAM lParam)
{
    if (dialogOpen) {
        // tabletInit(...) may show the screen resolution dialog and is blocking.
        // During this period, don't process any tablet events at all.
        dbgTablet << "WinTab screen resolution dialog is active, ignoring WinTab proximity event";
        return false;
    }
    auto sendProximityEvent = [&](QEvent::Type type) {
        QPointF emptyPos;
        qreal zero = 0.0;
        QTabletEvent e(type, emptyPos, emptyPos, 0, m_devices.at(m_currentDevice).currentPointerType,
                       zero, 0, 0, zero, zero, 0, Qt::NoModifier,
                       m_devices.at(m_currentDevice).uniqueId, Qt::NoButton, (Qt::MouseButtons)0);
        qApp->sendEvent(qApp, &e);
    };

    if (!LOWORD(lParam)) {
        // dbgTablet << "leave proximity for device #" << m_currentDevice;
        sendProximityEvent(QEvent::TabletLeaveProximity);
        return true;
    }
    PACKET proximityBuffer[1]; // we are only interested in the first packet in this case
    const int totalPacks = QWindowsTabletSupport::m_winTab32DLL.wTPacketsGet(m_context, 1, proximityBuffer);
    if (!totalPacks)
        return false;
    UINT pkCursor = proximityBuffer[0].pkCursor;
    // initializing and updating the cursor should be done in response to
    // WT_CSRCHANGE. We do it in WT_PROXIMITY because some wintab never send
    // the event WT_CSRCHANGE even if asked with CXO_CSRMESSAGES
    tabletUpdateCursor(pkCursor);
    // dbgTablet << "enter proximity for device #" << m_currentDevice << m_devices.at(m_currentDevice);

    sendProximityEvent(QEvent::TabletEnterProximity);
    return true;
}



bool QWindowsTabletSupport::translateTabletPacketEvent()
{
    static PACKET localPacketBuf[TabletPacketQSize];  // our own tablet packet queue.
    const int packetCount = QWindowsTabletSupport::m_winTab32DLL.wTPacketsGet(m_context, TabletPacketQSize, &localPacketBuf);
    if (!packetCount || m_currentDevice < 0 || dialogOpen)
        return false;

    // In contrast to Qt, these will not be "const" during our loop.
    // This is because the Surface Pro 3 may cause us to switch devices.
    QWindowsTabletDeviceData tabletData = m_devices.at(m_currentDevice);
    int currentDevice  = tabletData.currentDevice;
    int currentPointerType = tabletData.currentPointerType;

    // static Qt::MouseButtons buttons = Qt::NoButton, btnOld, btnChange;

    static DWORD btnNew, btnOld, btnChange;

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
    static qreal dpr = 1.0;
    auto activeWindow = qApp->activeWindow();
    if (activeWindow) {
        dpr = activeWindow->devicePixelRatio();
    }



    const Qt::KeyboardModifiers keyboardModifiers = QApplication::queryKeyboardModifiers();

    for (int i = 0; i < packetCount; ++i) {
        const PACKET &packet = localPacketBuf[i];

        btnOld = btnNew;
        btnNew = localPacketBuf[i].pkButtons;
        btnChange = btnOld ^ btnNew;

        bool buttonPressed = btnChange && btnNew > btnOld;
        bool buttonReleased = btnChange && btnNew < btnOld;
        bool anyButtonsStillPressed = btnNew;
        Qt::MouseButton button = Qt::NoButton;
        Qt::MouseButtons buttons;

        globalButtonsConverter->convert(btnOld, btnNew, &button, &buttons, tabletData);

        QEvent::Type type = QEvent::TabletMove;
        if (buttonPressed && button != Qt::NoButton) {
            type = QEvent::TabletPress;
        } else if (buttonReleased && button != Qt::NoButton) {
            type = QEvent::TabletRelease;
        }

        const int z = currentDevice == QTabletEvent::FourDMouse ? int(packet.pkZ) : 0;

        // NOTE: we shouldn't postpone the tablet events like Qt does, because we
        //       don't support mouse mode (which was the reason for introducing this
        //       postponing). See bug 363284.
        QPointF globalPosF =
            tabletData.scaleCoordinates(packet.pkX, packet.pkY,
                                        tabletData.virtualDesktopArea);
        globalPosF /= dpr; // Convert from "native" to "device independent pixels."

        QPoint globalPos = globalPosF.toPoint();

        // Find top-level window
        QWidget *w = targetWindow; // If we had a target already, use it.
        if (!w) {
            w = qApp->activePopupWidget();
            if (!w) w = qApp->activeModalWidget();
            if (!w) w = qApp->topLevelAt(globalPos);
            if (!w) continue;
            w = w->window();
        }


        const QPoint localPos = w->mapFromGlobal(globalPos);
        const QPointF delta = globalPosF - globalPos;
        const QPointF localPosF = globalPosF + QPointF(localPos - globalPos) + delta;

        const qreal pressureNew = packet.pkButtons && (currentPointerType == QTabletEvent::Pen || currentPointerType == QTabletEvent::Eraser) ?
                                  m_devices.at(m_currentDevice).scalePressure(packet.pkNormalPressure) :
                                  qreal(0);
        const qreal tangentialPressure = currentDevice == QTabletEvent::Airbrush ?
                                         m_devices.at(m_currentDevice).scaleTangentialPressure(packet.pkTangentPressure) :
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
            dbgTablet
                << "Packet #" << (i+1) << '/' << packetCount << "button:" << packet.pkButtons
                << globalPosF << z << "to:" << w << localPos << "(packet" << packet.pkX
                << packet.pkY << ") dev:" << currentDevice << "pointer:"
                << currentPointerType << "P:" << pressureNew << "tilt:" << tiltX << ','
                << tiltY << "tanP:" << tangentialPressure << "rotation:" << rotation;
        }

        // Reusable helper function. Better than compiler macros!
        auto sendTabletEvent = [&](QTabletEvent::Type t){
            handleTabletEvent(w, localPosF, globalPosF, currentDevice, currentPointerType,
                              button, buttons, pressureNew, tiltX, tiltY, tangentialPressure, rotation, z,
                              m_devices.at(m_currentDevice).uniqueId, keyboardModifiers, t, packet.pkTime);
        };

        /**
         * Workaround to deal with "inline" tool switches.
         * These are caused by the eraser trigger button on the Surface Pro 3.
         * We shoot out a tabletUpdateCursor request and a switchInputDevice request.
         */
        if (isSurfacePro3 && (packet.pkCursor != currentPkCursor)) {

            dbgTablet << "Got an inline tool switch.";
            // Send tablet release event.
            sendTabletEvent(QTabletEvent::TabletRelease);

            // Read the new cursor info.
            UINT pkCursor = packet.pkCursor;
            tabletUpdateCursor(pkCursor);

            // Update the local loop variables.
            tabletData = m_devices.at(m_currentDevice);
            currentDevice  = deviceType(tabletData.currentDevice);
            currentPointerType = pointerType(pkCursor);

            sendTabletEvent(QTabletEvent::TabletPress);
        }

        sendTabletEvent(type);

    } // Loop over packets
    return true;
}





void QWindowsTabletSupport::tabletUpdateCursor(const int pkCursor)
{
    UINT physicalCursorId;
    QWindowsTabletSupport::m_winTab32DLL.wTInfo(WTI_CURSORS + pkCursor, CSR_PHYSID, &physicalCursorId);
    UINT cursorType;
    QWindowsTabletSupport::m_winTab32DLL.wTInfo(WTI_CURSORS + pkCursor, CSR_TYPE, &cursorType);
    const qint64 uniqueId = (qint64(cursorType & DeviceIdMask) << 32L) | qint64(physicalCursorId);

    m_currentDevice = indexOfDevice(m_devices, uniqueId);
    if (m_currentDevice < 0) {
        m_currentDevice = m_devices.size();
        m_devices.push_back(tabletInit(uniqueId, cursorType));

        // Note: ideally we might check this button map for changes every
        // update. However there seems to be an issue with Wintab altering the
        // button map when the user right-clicks in Krita while another
        // application has focus. This forces Krita to load button settings only
        // once, when the tablet is first detected.
        //
        // See https://bugs.kde.org/show_bug.cgi?id=359561
        BYTE logicalButtons[32];
        memset(logicalButtons, 0, 32);
        m_winTab32DLL.wTInfo(WTI_CURSORS + pkCursor, CSR_SYSBTNMAP, &logicalButtons);
        m_devices[m_currentDevice].buttonsMap[0x1] = logicalButtons[0];
        m_devices[m_currentDevice].buttonsMap[0x2] = logicalButtons[1];
        m_devices[m_currentDevice].buttonsMap[0x4] = logicalButtons[2];
    }
    m_devices[m_currentDevice].currentPointerType = pointerType(pkCursor);
    currentPkCursor = pkCursor;

    // Check tablet name to enable Surface Pro 3 workaround.
#ifdef UNICODE
    if (!isSurfacePro3) {
        /**
         * Some really "nice" tablet drivers don't know that they are
         * supposed to return their name length when the buffer is
         * null and they try to write into it effectively causing a
         * suicide. So we cannot rely on it :(
         *
         * We workaround it by just allocating a big array and hoping
         * for the best.
         *
         * Failing tablets:
         *   - Adesso Cybertablet M14
         *   - Peritab-302
         *   - Trust Tablet TB7300
         *   - VisTablet Realm Pro
         *   - Aiptek 14000u (latest driver: v5.03, 2013-10-21)
         *   - Genius G-Pen F350
         *   - Genius G-Pen 560 (supported on win7 only)
         */

        // we cannot use the correct api :(
        // UINT nameLength = QWindowsTabletSupport::m_winTab32DLL.wTInfo(WTI_DEVICES, DVC_NAME, 0);

        // 1024 chars should be enough for everyone! (c)
        UINT nameLength = 1024;

        TCHAR* dvcName = new TCHAR[nameLength + 1];
        memset(dvcName, 0, sizeof(TCHAR) * nameLength);
        UINT writtenBytes = QWindowsTabletSupport::m_winTab32DLL.wTInfo(WTI_DEVICES, DVC_NAME, dvcName);

        if (writtenBytes > sizeof(TCHAR) * nameLength) {
            qWarning() << "WINTAB WARNING: tablet name is too long!" << writtenBytes;

            // avoid crash when trying to read it
            dvcName[nameLength - 1] = (TCHAR)0;
        }

        QString qDvcName = QString::fromWCharArray((const wchar_t*)dvcName);
        dbgInput << "DVC_NAME =" << qDvcName;
        // Name changed between older and newer Surface Pro 3 drivers
        if (qDvcName == QString::fromLatin1("N-trig DuoSense device") ||
            qDvcName == QString::fromLatin1("Microsoft device")) {
            dbgInput << "This looks like a Surface Pro 3. Enabling eraser workaround.";
            isSurfacePro3 = true;
        }
        delete[] dvcName;
    }
#endif
}


QT_END_NAMESPACE
