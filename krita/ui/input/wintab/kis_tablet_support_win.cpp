/*
 *  Copyright (c) 2013 Digia Plc and/or its subsidiary(-ies).
 *  Copyright (c) 2013 Boudewijn Rempt <boud@valdyas.org>
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

#include <input/kis_tablet_event.h>
#include "kis_tablet_support_win.h"
#include "kis_tablet_support.h"

#include <kis_debug.h>
#include <QApplication>
#include <QDesktopWidget>

#include <math.h>
#define Q_PI M_PI

#include <input/kis_extended_modifiers_mapper.h>
#include <input/kis_tablet_debugger.h>

#include "kis_screen_size_choice_dialog.h"

/**
 * A set of definitions to form a structure of a WinTab packet.  This
 * structure must be 100% the same as the structure of the packet
 * defined by Qt internally
 */

#define PACKETDATA  (PK_X | PK_Y | PK_BUTTONS | PK_NORMAL_PRESSURE | PK_TANGENT_PRESSURE \
                     | PK_ORIENTATION | PK_CURSOR | PK_Z)
#define PACKETMODE  0

#include "wintab.h"
#ifndef CSR_TYPE
#define CSR_TYPE 20 // Some old Wacom wintab.h may not provide this constant.
#endif
#include "pktdef.h"


/**
 * Pointers to the API functions resolved manually
 */
typedef UINT (API *PtrWTInfo)(UINT, UINT, LPVOID);
typedef int  (API *PtrWTPacketsGet)(HCTX, int, LPVOID);
typedef int  (API *PtrWTPacketsPeek)(HCTX, int, LPVOID);
typedef BOOL (API *PtrWTGet)(HCTX, LPLOGCONTEXT);
typedef BOOL (API *PtrWTOverlap)(HCTX, BOOL);

static PtrWTInfo ptrWTInfo = 0;
static PtrWTPacketsGet ptrWTPacketsGet = 0;
static PtrWTPacketsPeek ptrWTPacketsPeek = 0;
static PtrWTGet ptrWTGet = 0;
static PtrWTOverlap ptrWTOverlap = 0;

/**
 * A cached array for fetching packets from the WinTab queue
 */
enum { QT_TABLET_NPACKETQSIZE = 128 };
static PACKET globalPacketBuf[QT_TABLET_NPACKETQSIZE];  // our own tablet packet queue.

/**
 * Global variables to handle the Tablet Context
 */
HCTX qt_tablet_context = 0;
bool qt_tablet_tilt_support;

/**
 * The widget that got the latest tablet press.
 *
 * It should be impossible that one widget would get tablet press and
 * the other tablet release. Keeping track of which widget got the
 * press event allows to ensure that exactly the same widget would get
 * the release.
 */
QPointer<QWidget> kis_tablet_pressed = 0;

/**
 * The hash taple of available cursor, containing information about
 * each curror, its resolution and capabilities
 */
typedef QHash<quint64, QTabletDeviceData> QTabletCursorInfo;
Q_GLOBAL_STATIC(QTabletCursorInfo, tCursorInfo)
QTabletDeviceData currentTabletPointer;

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

        button = currentTabletPointer.buttonsMap.value(button);

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
 * Resolves the WINTAB api functions
 */
static void initWinTabFunctions()
{
    QLibrary library(QLatin1String("wintab32"));
    Q_ASSERT(library.isLoaded());
    ptrWTInfo = (PtrWTInfo)library.resolve("WTInfoW");
    ptrWTGet = (PtrWTGet)library.resolve("WTGetW");
    ptrWTPacketsGet = (PtrWTPacketsGet)library.resolve("WTPacketsGet");
    ptrWTPacketsPeek = (PtrWTPacketsGet)library.resolve("WTPacketsPeek");
    ptrWTOverlap = (PtrWTOverlap)library.resolve("WTOverlap");
}

void printContext(const LOGCONTEXT &lc)
{
    qDebug() << "Context data:";
    qDebug() << ppVar(lc.lcName);
    qDebug() << ppVar(lc.lcDevice);
    qDebug() << ppVar(lc.lcInOrgX);
    qDebug() << ppVar(lc.lcInOrgY);
    qDebug() << ppVar(lc.lcInExtX);
    qDebug() << ppVar(lc.lcInExtY);
    qDebug() << ppVar(lc.lcOutOrgX);
    qDebug() << ppVar(lc.lcOutOrgY);
    qDebug() << ppVar(lc.lcOutExtX);
    qDebug() << ppVar(lc.lcOutExtY);
    qDebug() << ppVar(lc.lcSysOrgX);
    qDebug() << ppVar(lc.lcSysOrgY);
    qDebug() << ppVar(lc.lcSysExtX);
    qDebug() << ppVar(lc.lcSysExtY);

    qDebug() << "Qt Desktop Geometry" << QApplication::desktop()->geometry();
}

/**
 * Initializes the QTabletDeviceData structure for \p uniqueId cursor
 * and stores it in the global cache
 */
static void tabletInit(const quint64 uniqueId, const UINT csr_type, HCTX hTab)
{
    Q_ASSERT(ptrWTInfo);
    Q_ASSERT(ptrWTGet);

    Q_ASSERT(!tCursorInfo()->contains(uniqueId));

    /* browse WinTab's many info items to discover pressure handling. */
    AXIS np;
    LOGCONTEXT lc;

    /* get the current context for its device variable. */
    ptrWTGet(hTab, &lc);

    if (KisTabletDebugger::instance()->initializationDebugEnabled()) {
        qDebug() << "# Getting current context:";
        printContext(lc);
    }

    /* get the size of the pressure axis. */
    QTabletDeviceData tdd;
    tdd.llId = uniqueId;

    ptrWTInfo(WTI_DEVICES + lc.lcDevice, DVC_NPRESSURE, &np);
    tdd.minPressure = int(np.axMin);
    tdd.maxPressure = int(np.axMax);

    ptrWTInfo(WTI_DEVICES + lc.lcDevice, DVC_TPRESSURE, &np);
    tdd.minTanPressure = int(np.axMin);
    tdd.maxTanPressure = int(np.axMax);

    // some tablets don't support tilt, check if it is possible,
    struct tagAXIS tpOri[3];
    qt_tablet_tilt_support = ptrWTInfo(WTI_DEVICES, DVC_ORIENTATION, &tpOri);
    if (qt_tablet_tilt_support) {
        // check for azimuth and altitude
        qt_tablet_tilt_support = tpOri[0].axResolution && tpOri[1].axResolution;
    }

    tdd.minX = int(lc.lcOutOrgX);
    tdd.maxX = int(qAbs(lc.lcOutExtX)) + int(lc.lcOutOrgX);

    tdd.minY = int(lc.lcOutOrgY);
    tdd.maxY = int(qAbs(lc.lcOutExtY)) + int(lc.lcOutOrgY);

    tdd.minZ = int(lc.lcOutOrgZ);
    tdd.maxZ = int(qAbs(lc.lcOutExtZ)) + int(lc.lcOutOrgZ);


    QRect qtDesktopRect = QApplication::desktop()->geometry();
    QRect wintabDesktopRect(lc.lcSysOrgX, lc.lcSysOrgY,
                            lc.lcSysExtX, lc.lcSysExtY);

    qDebug() << ppVar(qtDesktopRect);
    qDebug() << ppVar(wintabDesktopRect);

    QRect desktopRect = wintabDesktopRect;

    {
        KisScreenSizeChoiceDialog dlg(0,
                                      qtDesktopRect,
                                      wintabDesktopRect);

        KisExtendedModifiersMapper mapper;
        KisExtendedModifiersMapper::ExtendedModifiers modifiers =
            mapper.queryExtendedModifiers();

        if (modifiers.contains(Qt::Key_Shift) ||
            (!dlg.canUseDefaultSettings() &&
             qtDesktopRect != wintabDesktopRect)) {

            dlg.exec();
        }

        desktopRect = dlg.screenRect();
    }

    tdd.sysOrgX = desktopRect.x();
    tdd.sysOrgY = desktopRect.y();
    tdd.sysExtX = desktopRect.width();
    tdd.sysExtY = desktopRect.height();

    if (KisTabletDebugger::instance()->initializationDebugEnabled()) {
        qDebug() << "# Axes configuration";
        qDebug() << ppVar(tdd.minPressure) << ppVar(tdd.maxPressure);
        qDebug() << ppVar(tdd.minTanPressure) << ppVar(tdd.maxTanPressure);
        qDebug() << ppVar(qt_tablet_tilt_support);
        qDebug() << ppVar(tdd.minX) << ppVar(tdd.maxX);
        qDebug() << ppVar(tdd.minY) << ppVar(tdd.maxY);
        qDebug() << ppVar(tdd.minZ) << ppVar(tdd.maxZ);
        qDebug().nospace() << "# csr type: 0x" << hex << csr_type;
    }

    if (KisTabletDebugger::instance()->initializationDebugEnabled()) {
        LOGCONTEXT lcMine;

        /* get default region */
        ptrWTInfo(WTI_DEFCONTEXT, 0, &lcMine);

        qDebug() << "# Getting default context:";
        printContext(lcMine);
    }

    const uint cursorTypeBitMask = 0x0F06; // bitmask to find the specific cursor type (see Wacom FAQ)
    if (((csr_type & 0x0006) == 0x0002) && ((csr_type & cursorTypeBitMask) != 0x0902)) {
        tdd.currentDevice = QTabletEvent::Stylus;
    } else if (csr_type == 0x4020) { // Surface Pro 2 tablet device
        tdd.currentDevice = QTabletEvent::Stylus;
    } else {
        switch (csr_type & cursorTypeBitMask) {
            case 0x0802:
                tdd.currentDevice = QTabletEvent::Stylus;
                break;
            case 0x0902:
                tdd.currentDevice = QTabletEvent::Airbrush;
                break;
            case 0x0004:
                tdd.currentDevice = QTabletEvent::FourDMouse;
                break;
            case 0x0006:
                tdd.currentDevice = QTabletEvent::Puck;
                break;
            case 0x0804:
                tdd.currentDevice = QTabletEvent::RotationStylus;
                break;
            default:
                tdd.currentDevice = QTabletEvent::NoDevice;
        }
    }
    tCursorInfo()->insert(uniqueId, tdd);
}

/**
 * Updates the current cursor of the stylus
 */
static void tabletUpdateCursor(QTabletDeviceData &tdd, const UINT currentCursor)
{
    switch (currentCursor % 3) { // %3 for dual track
    case 0:
        tdd.currentPointerType = QTabletEvent::Cursor;
        break;
    case 1:
        tdd.currentPointerType = QTabletEvent::Pen;
        break;
    case 2:
        tdd.currentPointerType = QTabletEvent::Eraser;
        break;
    default:
        tdd.currentPointerType = QTabletEvent::UnknownPointer;
    }
}

class EventEater : public QObject {
public:
    EventEater(QObject *p) : QObject(p), m_eventType(QEvent::None) {}

    bool eventFilter(QObject* object, QEvent* event ) {
        if (event->type() == m_eventType) {
            m_eventType = QEvent::None;
            return true;
        }

        return QObject::eventFilter(object, event);
    }

    void pleaseEatNextEvent(QEvent::Type eventType) {
        m_eventType = eventType;
    }

private:
    QEvent::Type m_eventType;
};

static EventEater *globalEventEater = 0;

bool translateTabletEvent(const MSG &msg, PACKET *localPacketBuf,
                                      int numPackets)
{
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

    /**
     * There is a bug in Qt (tested on 4.8.5) and if you press
     * Win+UpKey hotkey right after the start of the application, the
     * next call to QApplication::keyboardModifiers() will return Alt
     * key pressed, although it isn't.  That is why we do not rely on
     * keyboardModifiers(), but just query the keys state directly.
     */
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

        QPointF hiResGlobal = currentTabletPointer.scaleCoord(ptNew.x, ptNew.y,
                                                              currentTabletPointer.sysOrgX, currentTabletPointer.sysExtX,
                                                              currentTabletPointer.sysOrgY, currentTabletPointer.sysExtY);


        if (KisTabletDebugger::instance()->debugRawTabletValues()) {
            qDebug() << "WinTab (RC):"
                     << "Dsk:"  << QRect(currentTabletPointer.sysOrgX, currentTabletPointer.sysOrgY, currentTabletPointer.sysExtX,  currentTabletPointer.sysExtY)
                     << "Raw:" << ptNew.x << ptNew.y
                     << "Scaled:" << hiResGlobal;

            qDebug() << "WinTab (BN):"
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
            if (currentTabletPointer.currentPointerType == QTabletEvent::Pen || currentTabletPointer.currentPointerType == QTabletEvent::Eraser)
                prsNew = localPacketBuf[i].pkNormalPressure
                            / qreal(currentTabletPointer.maxPressure
                                    - currentTabletPointer.minPressure);
            else
                prsNew = 0;
        }

        QPoint globalPos(qRound(hiResGlobal.x()), qRound(hiResGlobal.y()));

        // make sure the tablet event get's sent to the proper widget...
        QWidget *w = 0;

        /**
         * Find the appropriate window in an order of preference
         */

        if (!w) w = qApp->widgetAt(globalPos);
        if (!w) w = QWidget::find(msg.hwnd);

        QWidget *parentOverride = 0;

        if (!parentOverride) parentOverride = qApp->activePopupWidget();
        if (!parentOverride) parentOverride = qApp->activeModalWidget();

        if (!w || (parentOverride && !parentOverride->isAncestorOf(w))) {
            w = parentOverride;
        }

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
        if (currentTabletPointer.currentDevice == QTabletEvent::Airbrush) {
            tangentialPressure = localPacketBuf[i].pkTangentPressure
                                / qreal(currentTabletPointer.maxTanPressure
                                        - currentTabletPointer.minTanPressure);
        } else {
            tangentialPressure = 0.0;
        }

        if (!qt_tablet_tilt_support) {
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

            qDebug() << "WinTab (RS):"
                     << "NP:" << localPacketBuf[i].pkNormalPressure
                     << "TP:" << localPacketBuf[i].pkTangentPressure
                     << "Az:" << ort.orAzimuth
                     << "Alt:" << ort.orAltitude
                     << "Twist:" << ort.orTwist;
        }

        KisTabletEvent e(t, localPos, globalPos, hiResGlobal, currentTabletPointer.currentDevice,
                         currentTabletPointer.currentPointerType, prsNew, tiltX, tiltY,
                         tangentialPressure, rotation, z, modifiers, currentTabletPointer.llId,
                         button, buttons);

        if (button == Qt::NoButton &&
            (t == KisTabletEvent::TabletPressEx ||
             t == KisTabletEvent::TabletReleaseEx)) {

            /**
             * Eat events which do not correcpond to any mouse
             * button. This can happen when the user assinged a stylus
             * key to e.g. some keyboard key
             */
            e.accept();
        } else {
            e.ignore();
            sendEvent = qApp->sendEvent(w, &e);
        }

        if (e.isAccepted()) {
            globalEventEater->pleaseEatNextEvent(e.getMouseEventType());
        } else {
            QTabletEvent t = e.toQTabletEvent();
            qApp->sendEvent(w,  &t);
        }
    }
    return sendEvent;
}

void KisTabletSupportWin::init()
{
    globalEventEater = new EventEater(qApp);
    qApp->installEventFilter(globalEventEater);

    initWinTabFunctions();
}

void KisTabletSupportWin::setButtonsConverter(ButtonsConverter *buttonsConverter)
{
    globalButtonsConverter = buttonsConverter;
}

bool KisTabletSupportWin::eventFilter(void *message, long *result)
{
    MSG *msg = static_cast<MSG*>(message);
    Q_UNUSED(result);

    static bool mouseEnteredFlag = false;

    switch(msg->message){
    case WT_CTXOPEN:
        qt_tablet_context = reinterpret_cast<HCTX>(msg->wParam);
        break;
    case WT_CTXCLOSE:
        qt_tablet_context = 0;
        break;
    case WM_ACTIVATE: {
        /**
         * Workaround for a focus bug by Qt
         *
         * Looks like modal windows do not grab focus on Windows. The
         * parent widget will still be regarded as a focusWidget()
         * although it gets no events. So notify the pure parent that
         * he is not in focus anymore.
         */
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
        if (qt_tablet_context && !mouseEnteredFlag) {
            ptrWTOverlap(qt_tablet_context, true);
            mouseEnteredFlag = true;
        }
        break;
    case WT_PROXIMITY:
            if (ptrWTPacketsPeek && ptrWTInfo) {
                const bool enteredProximity = LOWORD(msg->lParam) != 0;
                PACKET proximityBuffer[1]; // we are only interested in the first packet in this case
                const int totalPacks = ptrWTPacketsPeek(qt_tablet_context, 1, proximityBuffer);
                if (totalPacks > 0) {
                    const UINT currentCursor = proximityBuffer[0].pkCursor;

                    UINT csr_physid;
                    ptrWTInfo(WTI_CURSORS + currentCursor, CSR_PHYSID, &csr_physid);
                    UINT csr_type;
                    ptrWTInfo(WTI_CURSORS + currentCursor, CSR_TYPE, &csr_type);
                    const UINT deviceIdMask = 0xFF6; // device type mask && device color mask
                    quint64 uniqueId = (csr_type & deviceIdMask);
                    uniqueId = (uniqueId << 32) | csr_physid;

                    // initialising and updating the cursor should be done in response to
                    // WT_CSRCHANGE. We do it in WT_PROXIMITY because some wintab never send
                    // the event WT_CSRCHANGE even if asked with CXO_CSRMESSAGES
                    const QTabletCursorInfo *const globalCursorInfo = tCursorInfo();
                    if (!globalCursorInfo->contains(uniqueId))
                        tabletInit(uniqueId, csr_type, qt_tablet_context);

                    currentTabletPointer = globalCursorInfo->value(uniqueId);
                    tabletUpdateCursor(currentTabletPointer, currentCursor);

                    BYTE logicalButtons[32];
                    memset(logicalButtons, 0, 32);
                    ptrWTInfo(WTI_CURSORS + currentCursor, CSR_SYSBTNMAP, &logicalButtons);

                    currentTabletPointer.buttonsMap[0x1] = logicalButtons[0];
                    currentTabletPointer.buttonsMap[0x2] = logicalButtons[1];
                    currentTabletPointer.buttonsMap[0x4] = logicalButtons[2];

                    if (KisTabletDebugger::instance()->initializationDebugEnabled()) {
                        qDebug() << "--------------------------";
                        qDebug() << "--- Tablet buttons map ---";
                        for (int i = 0; i < 32; i++) {
                            qDebug() << "( 1 <<" << i << ")" << "->" << logicalButtons[i];
                        }
                        qDebug() << "--------------------------";
                    }
                }
            }
        break;
    case WT_PACKET: {
        Q_ASSERT(qt_tablet_context);

        /**
         * We eat WT_PACKET events.. We are bad!
         *
         * But we can do nothing, because the packets are removed from the
         * WinTab queue by WTPacketsGet
         */

        int nPackets;
        if ((nPackets = ptrWTPacketsGet(qt_tablet_context, QT_TABLET_NPACKETQSIZE, &globalPacketBuf))) {
            return translateTabletEvent(*msg, globalPacketBuf, nPackets);
        }

        break;
    }
    }

    return false;
}

