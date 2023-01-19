/*
 * SPDX-FileCopyrightText: 2017 Alvin Wong <alvinhochun@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

// Get Windows 8 API prototypes and types
#ifdef WINVER
#  undef WINVER
#endif
#ifdef _WIN32_WINNT
#  undef _WIN32_WINNT
#endif
#define WINVER 0x0602
#define _WIN32_WINNT 0x0602

#include "kis_tablet_support_win8.h"

#include <QApplication>
#include <QDebug>
#include <QHash>
#include <QLibrary>
#include <QPointer>
#include <QTabletEvent>
#include <QVector>
#include <QWidget>
#include <QWindow>

#include <utility>

#include <kis_debug.h>

#include <windows.h>
#include <tpcshrd.h>

#ifndef Q_OS_WIN
#  error This file must not be compiled for non-Windows systems
#endif

namespace
{

class Win8PointerInputApi
{
#define WIN8_POINTER_INPUT_API_LIST(FUNC) \
    /* Pointer Input Functions */ \
    FUNC(GetPointerPenInfo) \
    FUNC(GetPointerPenInfoHistory) \
    FUNC(GetPointerType) \
    /* Pointer Device Functions */ \
    FUNC(GetPointerDevices) \
    /*FUNC(GetPointerDeviceProperties)*/ \
    FUNC(GetPointerDevice) \
    FUNC(GetPointerDeviceRects) \
    /*FUNC(RegisterPointerDeviceNotifications)*/ \
    /* end */

    bool m_loaded;

public:

#define DEFINE_FP_FROM_WINAPI(func) \
    public: using p ## func ## _t = std::add_pointer<decltype(func)>::type; \
    private: p ## func ## _t m_p ## func = nullptr; \
    public: const p ## func ## _t &func = m_p ## func; // const fp ref to member

    WIN8_POINTER_INPUT_API_LIST(DEFINE_FP_FROM_WINAPI)

#undef DEFINE_FP_FROM_WINAPI

public:
    Win8PointerInputApi()
        : m_loaded(false)
    {
    }

    bool init() {
        if (m_loaded) {
            return true;
        }

        QLibrary user32Lib("user32");
        if (!user32Lib.load()) {
            qWarning() << "Failed to load user32.dll! This really should not happen.";
            return false;
        }

#define LOAD_AND_CHECK_FP_FROM_WINAPI(func) \
    m_p ## func = reinterpret_cast<p ## func ## _t>(user32Lib.resolve(#func)); \
    if (!m_p ## func) { \
        dbgTablet << "Failed to load function " #func " from user32.dll"; \
        return false; \
    }

        WIN8_POINTER_INPUT_API_LIST(LOAD_AND_CHECK_FP_FROM_WINAPI)

#undef LOAD_AND_CHECK_FP_FROM_WINAPI

        dbgTablet << "Loaded Windows 8 Pointer Input API functions";
        m_loaded = true;
        return true;
    }

    bool isLoaded() {
        return m_loaded;
    }

#undef WIN8_POINTER_INPUT_API_LIST
}; // class Win8PointerInputApi

Win8PointerInputApi api;

class PointerFlagsWrapper
{
    const POINTER_FLAGS f;

public:
    PointerFlagsWrapper(POINTER_FLAGS flags)
        : f(flags)
    {}

    static PointerFlagsWrapper fromPointerInfo(const POINTER_INFO &pointerInfo) {
        return PointerFlagsWrapper(pointerInfo.pointerFlags);
    }

    static PointerFlagsWrapper fromPenInfo(const POINTER_PEN_INFO &penInfo) {
        return fromPointerInfo(penInfo.pointerInfo);
    }

    bool isNew() const {
        return f & POINTER_FLAG_NEW;
    }

    bool isInRange() const {
        return f & POINTER_FLAG_INRANGE;
    }

    bool isInContact() const {
        return f & POINTER_FLAG_INCONTACT;
    }

    bool isFirstButtonDown() const {
        return f & POINTER_FLAG_FIRSTBUTTON;
    }

    bool isSecondButtonDown() const {
        return f & POINTER_FLAG_SECONDBUTTON;
    }

    bool isThirdButtonDown() const {
        return f & POINTER_FLAG_THIRDBUTTON;
    }

    bool isForthButtonDown() const {
        return f & POINTER_FLAG_FOURTHBUTTON;
    }

    bool isFifthButtonDown() const {
        return f & POINTER_FLAG_FIFTHBUTTON;
    }

    bool isPrimary() const {
        return f & POINTER_FLAG_PRIMARY;
    }

    bool isConfidence() const {
        return f & POINTER_FLAG_CONFIDENCE;
    }

    bool isCancelled() const {
        return f & POINTER_FLAG_CANCELED;
    }

    bool isDown() const {
        return f & POINTER_FLAG_DOWN;
    }

    bool isUpdate() const {
        return f & POINTER_FLAG_UPDATE;
    }

    bool isUp() const {
        return f & POINTER_FLAG_UP;
    }

    bool isWheel() const {
        return f & POINTER_FLAG_WHEEL;
    }

    bool isHWheel() const {
        return f & POINTER_FLAG_HWHEEL;
    }

    bool isCaptureChanged() const {
        return f & POINTER_FLAG_CAPTURECHANGED;
    }

    bool hasTransform() const {
        // mingw-w64 headers is missing this flag
        // return f & POINTER_FLAG_HASTRANSFORM;
        return f & 0x00400000;
    }
}; // class PointerFlagsWrapper

class PenFlagsWrapper
{
    const PEN_FLAGS f;

public:
    PenFlagsWrapper(PEN_FLAGS flags)
        : f(flags)
    {}

    static PenFlagsWrapper fromPenInfo(const POINTER_PEN_INFO &penInfo) {
        return PenFlagsWrapper(penInfo.penFlags);
    }

    bool isBarrelPressed() const {
        return f & PEN_FLAG_BARREL;
    }

    bool isInverted() const {
        return f & PEN_FLAG_INVERTED;
    }

    bool isEraserPressed() const {
        return f & PEN_FLAG_ERASER;
    }
}; // class PenFlagsWrapper

class PenMaskWrapper
{
    const PEN_MASK f;

public:
    PenMaskWrapper(PEN_MASK mask)
        :f(mask)
    {}

    static PenMaskWrapper fromPenInfo(const POINTER_PEN_INFO &penInfo) {
        return PenMaskWrapper(penInfo.penMask);
    }

    bool pressureValid() const {
        return f & PEN_MASK_PRESSURE;
    }

    bool rotationValid() const {
        return f & PEN_MASK_ROTATION;
    }

    bool tiltXValid() const {
        return f & PEN_MASK_TILT_X;
    }

    bool tiltYValid() const {
        return f & PEN_MASK_TILT_Y;
    }
}; // class PenMaskWrapper

struct PointerDeviceItem
{
    // HANDLE handle;
    // RECT pointerDeviceRect;
    // RECT displayRect;
    qreal himetricToPixelX;
    qreal himetricToPixelY;
    qreal pixelOffsetX;
    qreal pixelOffsetY;
    DISPLAYCONFIG_ROTATION deviceOrientation; // This is needed to fix tilt
};

QHash<HANDLE, PointerDeviceItem> penDevices;

struct PenPointerItem
{
    // int pointerId;
    // POINTER_PEN_INFO penInfo;
    HWND hwnd;
    HANDLE deviceHandle;
    QPointer<QWidget> activeWidget; // Current widget receiving events
    qreal oneOverDpr; // 1 / devicePixelRatio of activeWidget
    bool widgetIsCaptured; // Current widget is capturing a pen cown event
    bool widgetIsIgnored; // Pen events should be ignored until pen up
    bool widgetAcceptsPenEvent; // Whether the widget accepts pen events

    bool isCaptured() const {
        return widgetIsCaptured;
    }
};

QHash<int, PenPointerItem> penPointers;
// int primaryPenPointerId;

bool handlePointerMsg(const MSG &msg);

// extern "C" {
//
// LRESULT CALLBACK pointerDeviceNotificationsWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
// {
//     switch (uMsg) {
//     case WM_POINTERDEVICECHANGE:
//         dbgTablet << "I would want to handle this WM_POINTERDEVICECHANGE event, but ms just doesn't want me to use it";
//         dbgTablet << "  wParam:" << wParam;
//         dbgTablet << "  lParam:" << lParam;
//         return 0;
//     case WM_POINTERDEVICEINRANGE:
//         dbgTablet << "I would want to handle this WM_POINTERDEVICEINRANGE event, but ms just doesn't want me to use it";
//         dbgTablet << "  wParam:" << wParam;
//         dbgTablet << "  lParam:" << lParam;
//         return 0;
//     case WM_POINTERDEVICEOUTOFRANGE:
//         dbgTablet << "I would want to handle this WM_POINTERDEVICEOUTOFRANGE event, but ms just doesn't want me to use it";
//         dbgTablet << "  wParam:" << wParam;
//         dbgTablet << "  lParam:" << lParam;
//         return 0;
//     }
//     return DefWindowProcW(hwnd, uMsg, wParam, lParam);
// }
//
// } // extern "C"

} // namespace

bool KisTabletSupportWin8::isAvailable()
{
    // Just try loading the APIs
    return api.init();
}

bool KisTabletSupportWin8::isPenDeviceAvailable()
{
    if (!api.init()) {
        return false;
    }
    UINT32 deviceCount = 0;
    if (!api.GetPointerDevices(&deviceCount, nullptr)) {
        dbgTablet << "GetPointerDevices failed";
        return false;
    }
    if (deviceCount == 0) {
        dbgTablet << "No pointer devices";
        return false;
    }
    QVector<POINTER_DEVICE_INFO> devices(deviceCount);
    if (!api.GetPointerDevices(&deviceCount, devices.data())) {
        dbgTablet << "GetPointerDevices failed";
        return false;
    }
    bool hasPenDevice = false;
    Q_FOREACH (const POINTER_DEVICE_INFO &device, devices) {
        dbgTablet << "Found pointer device" << static_cast<void *>(device.device)
                  << QString::fromWCharArray(device.productString)
                  << "type:" << device.pointerDeviceType;
        if (device.pointerDeviceType == POINTER_DEVICE_TYPE_INTEGRATED_PEN ||
            device.pointerDeviceType == POINTER_DEVICE_TYPE_EXTERNAL_PEN) {
            hasPenDevice = true;
        }
    }
    dbgTablet << "hasPenDevice:" << hasPenDevice;
    return hasPenDevice;
}

bool KisTabletSupportWin8::init()
{
    return api.init();
}

// void KisTabletSupportWin8::registerPointerDeviceNotifications()
// {
//     const wchar_t *className = L"w8PointerMsgWindow";
//     HINSTANCE hInst = static_cast<HINSTANCE>(GetModuleHandleW(nullptr));
//     WNDCLASSEXW wc;
//     wc.cbSize = sizeof(WNDCLASSEXW);
//     wc.style = 0;
//     wc.lpfnWndProc = pointerDeviceNotificationsWndProc;
//     wc.cbClsExtra = 0;
//     wc.cbWndExtra = 0;
//     wc.hInstance = hInst;
//     wc.hCursor = 0;
//     wc.hbrBackground = GetSysColorBrush(COLOR_WINDOW);
//     wc.hIcon = 0;
//     wc.hIconSm = 0;
//     wc.lpszMenuName = 0;
//     wc.lpszClassName = className;
//
//     if (RegisterClassEx(&wc)) {
//         HWND hwnd = CreateWindowEx(0, className, nullptr, 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, hInst, nullptr);
//         api.RegisterPointerDeviceNotifications(hwnd, TRUE);
//     } else {
//         dbgTablet << "Cannot register dummy window";
//     }
// }

bool KisTabletSupportWin8::nativeEventFilter(const QByteArray &eventType, void *message, long *result)
{
    if (!result) {
        // I don't know why this even happens, but it actually does
        // And the same event is sent in again with result != nullptr
        return false;
    }

    // This is only installed on Windows so there is no reason to check eventType
    MSG &msg = *static_cast<MSG *>(message);

    switch (msg.message) {
    case WM_POINTERDOWN:
    case WM_POINTERUP:
    case WM_POINTERENTER:
    case WM_POINTERLEAVE:
    case WM_POINTERUPDATE:
    case WM_POINTERCAPTURECHANGED:
        {
            bool handled = handlePointerMsg(msg);
            if (handled) {
                *result = 0;
                return true;
            }
            break;
        }
    case WM_TABLET_QUERYSYSTEMGESTURESTATUS:
        *result = 0;
        return true;
    }

    Q_UNUSED(eventType);
    return false;
}

namespace {

QDebug operator<<(QDebug debug, const POINT &pt)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << '(' << pt.x << ", " << pt.y << ')';

    return debug;
}

QDebug operator<<(QDebug debug, const RECT &rect)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << '(' << rect.left << ", " << rect.top << ", " << rect.right << ", " << rect.bottom << ')';

    return debug;
}

bool registerOrUpdateDevice(HANDLE deviceHandle, const RECT &pointerDeviceRect, const RECT &displayRect, const DISPLAYCONFIG_ROTATION deviceOrientation)
{
    bool isPreviouslyRegistered = penDevices.contains(deviceHandle);
    PointerDeviceItem &deviceItem = penDevices[deviceHandle];
    PointerDeviceItem oldDeviceItem = deviceItem;
    // deviceItem.handle = deviceHandle;
    deviceItem.himetricToPixelX =
            static_cast<qreal>(displayRect.right - displayRect.left)
            / (pointerDeviceRect.right - pointerDeviceRect.left);
    deviceItem.himetricToPixelY =
            static_cast<qreal>(displayRect.bottom - displayRect.top)
            / (pointerDeviceRect.bottom - pointerDeviceRect.top);
    deviceItem.pixelOffsetX = static_cast<qreal>(displayRect.left)
            - deviceItem.himetricToPixelX * pointerDeviceRect.left;
    deviceItem.pixelOffsetY = static_cast<qreal>(displayRect.top)
            - deviceItem.himetricToPixelY * pointerDeviceRect.top;
    deviceItem.deviceOrientation = deviceOrientation;
    if (!isPreviouslyRegistered) {
        dbgTablet << "Registered pen device" << deviceHandle
                  << "with displayRect" << displayRect
                  << "and deviceRect" << pointerDeviceRect
                  << "scale" << deviceItem.himetricToPixelX << deviceItem.himetricToPixelY
                  << "offset" << deviceItem.pixelOffsetX << deviceItem.pixelOffsetY
                  << "orientation" << deviceItem.deviceOrientation;
    } else if (deviceItem.himetricToPixelX != oldDeviceItem.himetricToPixelX
            || deviceItem.himetricToPixelY != oldDeviceItem.himetricToPixelY
            || deviceItem.pixelOffsetX != oldDeviceItem.pixelOffsetX
            || deviceItem.pixelOffsetY != oldDeviceItem.pixelOffsetY
            || deviceItem.deviceOrientation != oldDeviceItem.deviceOrientation) {
        dbgTablet << "Updated pen device" << deviceHandle
                  << "with displayRect" << displayRect
                  << "and deviceRect" << pointerDeviceRect
                  << "scale" << deviceItem.himetricToPixelX << deviceItem.himetricToPixelY
                  << "offset" << deviceItem.pixelOffsetX << deviceItem.pixelOffsetY
                  << "orientation" << deviceItem.deviceOrientation;
    }
    return true;
}

bool registerOrUpdateDevice(HANDLE deviceHandle)
{
    RECT pointerDeviceRect, displayRect;
    if (!api.GetPointerDeviceRects(deviceHandle, &pointerDeviceRect, &displayRect)) {
        dbgTablet << "GetPointerDeviceRects failed";
        return false;
    }
    POINTER_DEVICE_INFO pointerDeviceInfo;
    if (!api.GetPointerDevice(deviceHandle, &pointerDeviceInfo)) {
        dbgTablet << "GetPointerDevice failed";
        return false;
    }
    return registerOrUpdateDevice(deviceHandle, pointerDeviceRect, displayRect,
            static_cast<DISPLAYCONFIG_ROTATION>(pointerDeviceInfo.displayOrientation));
}

QTabletEvent makeProximityTabletEvent(const QEvent::Type eventType, const POINTER_PEN_INFO &penInfo)
{
    PenFlagsWrapper penFlags = PenFlagsWrapper::fromPenInfo(penInfo);
    QTabletEvent::PointerType pointerType = penFlags.isInverted() ? QTabletEvent::Eraser : QTabletEvent::Pen;
    const QPointF emptyPoint;
    return QTabletEvent(
        eventType, // type
        emptyPoint, // pos
        emptyPoint, // globalPos
        QTabletEvent::Stylus, // device
        pointerType, // pointerType
        0, // pressure
        0, // xTilt
        0, // yTilt
        0, // tangentialPressure
        0, // rotation
        0, // z
        Qt::NoModifier, // keyState
        reinterpret_cast<qint64>(penInfo.pointerInfo.sourceDevice), // uniqueID
        Qt::NoButton, // button
        (Qt::MouseButtons)0 // buttons
    );
}

// void rotateTiltAngles(int &tiltX, int &tiltY, const DISPLAYCONFIG_ROTATION orientation) {
//     int newTiltX, newTiltY;
//     switch (orientation) {
//     case DISPLAYCONFIG_ROTATION_ROTATE90:
//         newTiltX = -tiltY;
//         newTiltY = tiltX;
//         break;
//     case DISPLAYCONFIG_ROTATION_ROTATE180:
//         newTiltX = -tiltX;
//         newTiltY = -tiltY;
//         break;
//     case DISPLAYCONFIG_ROTATION_ROTATE270:
//         newTiltX = tiltY;
//         newTiltY = -tiltX;
//         break;
//     case DISPLAYCONFIG_ROTATION_IDENTITY:
//     default:
//         newTiltX = tiltX;
//         newTiltY = tiltY;
//         break;
//     }
//     tiltX = newTiltX;
//     tiltY = newTiltY;
// }

QTabletEvent makePositionalTabletEvent(const QWidget *targetWidget, const QEvent::Type eventType, const POINTER_PEN_INFO &penInfo, const PointerDeviceItem &deviceItem, const PenPointerItem &penPointerItem)
{
    PenFlagsWrapper penFlags = PenFlagsWrapper::fromPenInfo(penInfo);
    PointerFlagsWrapper pointerFlags = PointerFlagsWrapper::fromPenInfo(penInfo);
    PenMaskWrapper penMask = PenMaskWrapper::fromPenInfo(penInfo);

    const QPointF globalPosF(
        (deviceItem.himetricToPixelX * penInfo.pointerInfo.ptHimetricLocationRaw.x + deviceItem.pixelOffsetX) * penPointerItem.oneOverDpr,
        (deviceItem.himetricToPixelY * penInfo.pointerInfo.ptHimetricLocationRaw.y + deviceItem.pixelOffsetY) * penPointerItem.oneOverDpr
    );
    const QPoint globalPos = globalPosF.toPoint();
    const QPoint localPos = targetWidget->mapFromGlobal(globalPos);
    const QPointF delta = globalPosF - globalPos;
    const QPointF localPosF = localPos + delta;

    const QTabletEvent::PointerType pointerType = penFlags.isInverted() ? QTabletEvent::Eraser : QTabletEvent::Pen;

    Qt::MouseButton mouseButton;
    if (eventType == QEvent::TabletPress) {
        if (penInfo.pointerInfo.ButtonChangeType == POINTER_CHANGE_SECONDBUTTON_DOWN) {
            mouseButton = Qt::RightButton;
        } else {
            KIS_SAFE_ASSERT_RECOVER(penInfo.pointerInfo.ButtonChangeType == POINTER_CHANGE_FIRSTBUTTON_DOWN) {
                qWarning() << "WM_POINTER* sent unknown ButtonChangeType" << penInfo.pointerInfo.ButtonChangeType;
            }
            mouseButton = Qt::LeftButton;
        }
    } else if (eventType == QEvent::TabletRelease) {
        if (penInfo.pointerInfo.ButtonChangeType == POINTER_CHANGE_SECONDBUTTON_UP) {
            mouseButton = Qt::RightButton;
        } else {
            KIS_SAFE_ASSERT_RECOVER(penInfo.pointerInfo.ButtonChangeType == POINTER_CHANGE_FIRSTBUTTON_UP) {
                qWarning() << "WM_POINTER* sent unknown ButtonChangeType" << penInfo.pointerInfo.ButtonChangeType;
            }
            mouseButton = Qt::LeftButton;
        }
    } else {
        mouseButton = Qt::NoButton;
    }

    Qt::MouseButtons mouseButtons;
    if (pointerFlags.isFirstButtonDown()) {
        mouseButtons |= Qt::LeftButton;
    }
    if (pointerFlags.isSecondButtonDown()) {
        mouseButtons |= Qt::RightButton;
    }

    int tiltX = 0, tiltY = 0;
    if (penMask.tiltXValid()) {
        tiltX = qBound(-60, penInfo.tiltX, 60);
    }
    if (penMask.tiltYValid()) {
        tiltY = qBound(-60, penInfo.tiltY, 60);
    }
    // rotateTiltAngles(tiltX, tiltY, deviceItem.deviceOrientation);

    int rotation = 0;
    if (penMask.rotationValid()) {
        rotation = 360 - penInfo.rotation; // Flip direction and convert to signed int
        if (rotation > 180) {
            rotation -= 360;
        }
    }

    return QTabletEvent(
        eventType, // type
        localPosF, // pos
        globalPosF, // globalPos
        QTabletEvent::Stylus, // device
        pointerType, // pointerType
        penMask.pressureValid() ? static_cast<qreal>(penInfo.pressure) / 1024 : 0, // pressure
        tiltX, // xTilt
        tiltY, // yTilt
        0, // tangentialPressure
        rotation, // rotation
        0, // z
        QApplication::queryKeyboardModifiers(), // keyState
        reinterpret_cast<qint64>(penInfo.pointerInfo.sourceDevice), // uniqueID
        mouseButton, // button
        mouseButtons // buttons
    );
}

bool sendProximityTabletEvent(const QEvent::Type eventType, const POINTER_PEN_INFO &penInfo)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(
        eventType == QEvent::TabletEnterProximity || eventType == QEvent::TabletLeaveProximity,
        false
    );
    QTabletEvent ev = makeProximityTabletEvent(eventType, penInfo);
    ev.setAccepted(false);
    ev.setTimestamp(penInfo.pointerInfo.dwTime);
    QCoreApplication::sendEvent(qApp, &ev);
    return ev.isAccepted();
}

void synthesizeMouseEvent(const QTabletEvent &ev, const POINTER_PEN_INFO &penInfo)
{
    // Update the cursor position
    BOOL result = SetCursorPos(penInfo.pointerInfo.ptPixelLocationRaw.x, penInfo.pointerInfo.ptPixelLocationRaw.y);
    if (!result) {
        dbgInput << "SetCursorPos failed, err" << GetLastError();
        return;
    }
    // Send mousebutton down/up events. Windows stores the button state.
    DWORD inputDataFlags = 0;
    switch (ev.type()) {
    case QEvent::TabletPress:
        switch (ev.button()) {
        case Qt::LeftButton:
            inputDataFlags = MOUSEEVENTF_LEFTDOWN;
            break;
        case Qt::RightButton:
            inputDataFlags = MOUSEEVENTF_RIGHTDOWN;
            break;
        default:
            return;
        }
        break;
    case QEvent::TabletRelease:
        switch (ev.button()) {
        case Qt::LeftButton:
            inputDataFlags = MOUSEEVENTF_LEFTUP;
            break;
        case Qt::RightButton:
            inputDataFlags = MOUSEEVENTF_RIGHTUP;
            break;
        default:
            return;
        }
        break;
    case QEvent::TabletMove:
    default:
        return;
    }
    INPUT inputData = {};
    inputData.type = INPUT_MOUSE;
    inputData.mi.dwFlags = inputDataFlags;
    inputData.mi.dwExtraInfo = 0xFF515700 | 0x01; // https://msdn.microsoft.com/en-us/library/windows/desktop/ms703320%28v=vs.85%29.aspx
    UINT result2 = SendInput(1, &inputData, sizeof(inputData));
    if (result2 != 1) {
        dbgInput << "SendInput failed, err" << GetLastError();
        return;
    }
}

bool sendPositionalTabletEvent(QWidget *target, const QEvent::Type eventType, const POINTER_PEN_INFO &penInfo, const PointerDeviceItem &device, const PenPointerItem &penPointerItem, const bool shouldSynthesizeMouseEvent)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(
        eventType == QEvent::TabletMove || eventType == QEvent::TabletPress || eventType == QEvent::TabletRelease,
        false
    );
    QTabletEvent ev = makePositionalTabletEvent(target, eventType, penInfo, device, penPointerItem);
    ev.setAccepted(false);
    ev.setTimestamp(penInfo.pointerInfo.dwTime);
    QCoreApplication::sendEvent(target, &ev);
    if (!shouldSynthesizeMouseEvent) {
        // For pen update with multiple updates, only the last update should
        // be used to synthesize a mouse event.
        return false;
    }
    // This is some specialized code to handle synthesizing of mouse events from
    // the pen events. Issues being:
    // 1. We must accept the pointer down/up and the intermediate update events
    //    to indicate that we want all the pen pointer events for painting,
    //    otherwise Windows may do weird stuff and skip passing pointer events.
    // 2. Some Qt and Krita code uses QCursor::pos() which calls GetCursorPos to
    //    get the cursor position. This doesn't work nicely before ver 1709 and
    //    doesn't work at all on ver 1709 if the pen events are handled, so we
    //    need some way to nudge the cursor on the OS level.
    // It appears that using the same way (as in synthesizeMouseEvent) to nudge
    // the cursor does work fine for when painting on canvas (which handles
    // the QTabletEvent), but not for other widgets because it introduces a lag
    // with mouse move events on move start and immediately after mouse down.
    // The resolution is to simulate mouse movement with our own code only for
    // handled pen events, which is what the following code does.
    if (ev.type() == QEvent::TabletMove && ev.buttons() == Qt::NoButton) {
        // Let Windows synthesize mouse hover events
        return false;
    }
    if (ev.type() == QEvent::TabletPress && !ev.isAccepted()) {
        // On pen down event, if the widget doesn't handle the event, let
        // Windows translate the event to touch, mouse or whatever
        return false;
    }
    if (ev.type() != QEvent::TabletPress && !penPointerItem.widgetAcceptsPenEvent) {
        // For other events, if the previous pen down event wasn't handled by
        // the widget, continue to let Windows translate the event
        return false;
    }
    // Otherwise, we synthesize our mouse events
    synthesizeMouseEvent(ev, penInfo);
    return true; // and tell Windows that we do want the pen events.
}

bool handlePenEnterMsg(const POINTER_PEN_INFO &penInfo)
{
    PointerFlagsWrapper pointerFlags = PointerFlagsWrapper::fromPenInfo(penInfo);
    if (!pointerFlags.isPrimary()) {
        // Don't handle non-primary pointer messages for now
        dbgTablet << "Pointer" << penInfo.pointerInfo.pointerId
                  << "of device" << penInfo.pointerInfo.sourceDevice
                  << "is not flagged PRIMARY";
        return false;
    }

    // Update the device scaling factors here
    // It doesn't cost much to recalculate anyway
    // This ensures that the screen resolution changes are reflected
    // WM_POINTERDEVICECHANGE might be useful for this, but its docs are too unclear to use
    registerOrUpdateDevice(penInfo.pointerInfo.sourceDevice);
    // TODO: Need a way to remove from device registration when devices are changed

    // We now only handle one pointer at a time, so just clear the pointer registration
    penPointers.clear();

    int pointerId = penInfo.pointerInfo.pointerId;
    PenPointerItem penPointerItem;
    penPointerItem.hwnd = penInfo.pointerInfo.hwndTarget;
    penPointerItem.deviceHandle = penInfo.pointerInfo.sourceDevice;
    penPointerItem.activeWidget = nullptr;
    penPointerItem.oneOverDpr = 1.0;
    penPointerItem.widgetIsCaptured = false;
    penPointerItem.widgetIsIgnored = false;
    penPointerItem.widgetAcceptsPenEvent = false;
    // penPointerItem.pointerId = pointerId;

    penPointers.insert(pointerId, penPointerItem);
    // primaryPenPointerId = pointerId;

    // penEnter
    sendProximityTabletEvent(QEvent::TabletEnterProximity, penInfo);

    return false;
}

bool handlePenLeaveMsg(const POINTER_PEN_INFO &penInfo)
{
    if (!penPointers.contains(penInfo.pointerInfo.pointerId)) {
        dbgTablet << "Pointer" << penInfo.pointerInfo.pointerId << "wasn't being handled";
        return false;
    }
    if (!penDevices.contains(penInfo.pointerInfo.sourceDevice)) {
        dbgTablet << "Device is gone from the registration???";
        // TODO: re-register device?
        penPointers.remove(penInfo.pointerInfo.pointerId);
        return false;
    }

    // penLeave
    sendProximityTabletEvent(QEvent::TabletLeaveProximity, penInfo);

    penPointers.remove(penInfo.pointerInfo.pointerId);

    return false;
}

bool handleSinglePenUpdate(PenPointerItem &penPointerItem, const POINTER_PEN_INFO &penInfo, const PointerDeviceItem &device, const bool shouldSynthesizeMouseEvent)
{
    QWidget *targetWidget;
    if (penPointerItem.isCaptured()) {
        if (penPointerItem.widgetIsIgnored) {
            return false;
        }
        targetWidget = penPointerItem.activeWidget;
        if (!targetWidget) {
            return false;
        }
    } else {
        QWidget *hwndWidget = QWidget::find(reinterpret_cast<WId>(penInfo.pointerInfo.hwndTarget));
        if (!hwndWidget) {
            dbgTablet << "HWND cannot be mapped to QWidget (what?)";
            return false;
        }
        {
            // Check popup / modal widget
            QWidget *modalWidget = QApplication::activePopupWidget();
            if (!modalWidget) {
                modalWidget = QApplication::activeModalWidget();
            }
            if (modalWidget && modalWidget != hwndWidget && !modalWidget->isAncestorOf(hwndWidget)) {
                return false;
            }
        }
        {
            QWindow *topLevelWindow = hwndWidget->windowHandle();
            if (topLevelWindow) {
                penPointerItem.oneOverDpr = 1.0 / topLevelWindow->devicePixelRatio();
            } else {
                penPointerItem.oneOverDpr = 1.0 / qApp->devicePixelRatio();
            }
        }
        QPoint posInHwndWidget = hwndWidget->mapFromGlobal(QPoint(
            static_cast<int>(penInfo.pointerInfo.ptPixelLocationRaw.x * penPointerItem.oneOverDpr),
            static_cast<int>(penInfo.pointerInfo.ptPixelLocationRaw.y * penPointerItem.oneOverDpr)
        ));
        targetWidget = hwndWidget->childAt(posInHwndWidget);
        if (!targetWidget) {
            // dbgTablet << "No childQWidget at cursor position";
            targetWidget = hwndWidget;
        }
        // penPointerItem.activeWidget = targetWidget;
    }

    bool handled = sendPositionalTabletEvent(targetWidget, QEvent::TabletMove, penInfo, device, penPointerItem, shouldSynthesizeMouseEvent);
    return handled;
}

bool handlePenUpdateMsg(const POINTER_PEN_INFO &penInfo)
{
    auto currentPointerIt = penPointers.find(penInfo.pointerInfo.pointerId);
    if (currentPointerIt == penPointers.end()) {
        // dbgTablet << "Pointer" << penInfo.pointerInfo.pointerId << "wasn't being handled";
        return false;
    }

    const auto devIt = penDevices.find(penInfo.pointerInfo.sourceDevice);
    if (devIt == penDevices.end()) {
        dbgTablet << "Device not registered???";
        return false;
    }

    // UINT32 entriesCount = 0;
    // if (!api.GetPointerPenInfoHistory(penInfo.pointerInfo.pointerId, &entriesCount, nullptr)) {
    //     dbgTablet << "GetPointerPenInfoHistory (getting count) failed";
    //     return false;
    // }
    UINT32 entriesCount = penInfo.pointerInfo.historyCount;
    // dbgTablet << "entriesCount:" << entriesCount;
    bool handled = false;
    if (entriesCount != 1) {
        QVector<POINTER_PEN_INFO> penInfoArray(entriesCount);
        if (!api.GetPointerPenInfoHistory(penInfo.pointerInfo.pointerId, &entriesCount, penInfoArray.data())) {
            dbgTablet << "GetPointerPenInfoHistory failed";
            return false;
        }
        bool handled = false;
        // The returned array is in reverse chronological order
        const auto rbegin = penInfoArray.rbegin();
        const auto rend = penInfoArray.rend();
        const auto rlast = rend - 1; // Only synthesize mouse event for the last one
        for (auto it = rbegin; it != rend; ++it) {
            handled |= handleSinglePenUpdate(*currentPointerIt, *it, *devIt, it == rlast); // Bitwise OR doesn't short circuit
        }
    } else {
        handled = handleSinglePenUpdate(*currentPointerIt, penInfo, *devIt, true);
    }
    return handled;
}

bool handlePenDownMsg(const POINTER_PEN_INFO &penInfo)
{
    // PointerFlagsWrapper pointerFlags = PointerFlagsWrapper::fromPenInfo(penInfo);
    // if (!pointerFlags.isPrimary()) {
    //     // Don't handle non-primary pointer messages for now
    //     return false;
    // }
    auto currentPointerIt = penPointers.find(penInfo.pointerInfo.pointerId);
    if (currentPointerIt == penPointers.end()) {
        dbgTablet << "Pointer" << penInfo.pointerInfo.pointerId << "wasn't being handled";
        return false;
    }
    currentPointerIt->hwnd = penInfo.pointerInfo.hwndTarget; // They *should* be the same, but just in case
    QWidget *hwndWidget = QWidget::find(reinterpret_cast<WId>(penInfo.pointerInfo.hwndTarget));
    if (!hwndWidget) {
        dbgTablet << "HWND cannot be mapped to QWidget (what?)";
        return false;
    }
    {
        QWindow *topLevelWindow = hwndWidget->windowHandle();
        if (topLevelWindow) {
            currentPointerIt->oneOverDpr = 1.0 / topLevelWindow->devicePixelRatio();
        } else {
            currentPointerIt->oneOverDpr = 1.0 / qApp->devicePixelRatio();
        }
    }
    QPoint posInHwndWidget = hwndWidget->mapFromGlobal(QPoint(
        static_cast<int>(penInfo.pointerInfo.ptPixelLocationRaw.x * currentPointerIt->oneOverDpr),
        static_cast<int>(penInfo.pointerInfo.ptPixelLocationRaw.y * currentPointerIt->oneOverDpr)
    ));
    QWidget *targetWidget = hwndWidget->childAt(posInHwndWidget);
    if (!targetWidget) {
        dbgTablet << "No childQWidget at cursor position";
        targetWidget = hwndWidget;
    }

    currentPointerIt->activeWidget = targetWidget;
    currentPointerIt->widgetIsCaptured = true;
    // dbgTablet << "QWidget" << targetWidget->windowTitle() << "is capturing pointer" << penInfo.pointerInfo.pointerId;
    {
        // Check popup / modal widget
        QWidget *modalWidget = QApplication::activePopupWidget();
        if (!modalWidget) {
            modalWidget = QApplication::activeModalWidget();
        }
        if (modalWidget && modalWidget != hwndWidget && !modalWidget->isAncestorOf(hwndWidget)) {
            currentPointerIt->widgetIsIgnored = true;
            dbgTablet << "Pointer" << penInfo.pointerInfo.pointerId << "is being captured but will be ignored";
            return false;
        }
    }

    // penDown
    const auto devIt = penDevices.find(penInfo.pointerInfo.sourceDevice);
    if (devIt == penDevices.end()) {
        dbgTablet << "Device not registered???";
        return false;
    }

    bool handled = sendPositionalTabletEvent(targetWidget, QEvent::TabletPress, penInfo, *devIt, *currentPointerIt, true);
    currentPointerIt->widgetAcceptsPenEvent = handled;
    if (!handled) {
        // dbgTablet << "QWidget did not handle tablet down event";
    }
    return handled;
}

bool handlePenUpMsg(const POINTER_PEN_INFO &penInfo)
{
    auto currentPointerIt = penPointers.find(penInfo.pointerInfo.pointerId);
    if (currentPointerIt == penPointers.end()) {
        dbgTablet << "Pointer" << penInfo.pointerInfo.pointerId << "wasn't being handled";
        return false;
    }
    PenPointerItem &penPointerItem = *currentPointerIt;

    if (!penPointerItem.isCaptured()) {
        dbgTablet << "Pointer wasn't captured";
        return false;
    }
    if (penPointerItem.widgetIsIgnored) {
        penPointerItem.widgetIsCaptured = false;
        penPointerItem.widgetIsIgnored = false;
        return false;
    }

    // penUp
    QWidget *targetWidget = penPointerItem.activeWidget;
    if (!targetWidget) {
        dbgTablet << "Previously captured target has been deleted";
        penPointerItem.widgetIsCaptured = false;
        return false;
    }

    const auto devIt = penDevices.find(penInfo.pointerInfo.sourceDevice);
    if (devIt == penDevices.end()) {
        dbgTablet << "Device not registered???";
        return false;
    }

    bool handled = sendPositionalTabletEvent(targetWidget, QEvent::TabletRelease, penInfo, *devIt, penPointerItem, true);

    // dbgTablet << "QWidget" << currentPointerIt->activeWidget->windowTitle() << "is releasing capture to pointer" << penInfo.pointerInfo.pointerId;
    penPointerItem.widgetIsCaptured = false;
    penPointerItem.widgetAcceptsPenEvent = false;
    return handled;
}

bool handlePointerMsg(const MSG &msg)
{
    if (!api.isLoaded()) {
        qWarning() << "Windows 8 Pointer Input API functions not loaded";
        return false;
    }

    int pointerId = GET_POINTERID_WPARAM(msg.wParam);
    POINTER_INPUT_TYPE pointerType;
    if (!api.GetPointerType(pointerId, &pointerType)) {
        dbgTablet << "GetPointerType failed";
        return false;
    }
    if (pointerType != PT_PEN) {
        // dbgTablet << "pointerType" << pointerType << "is not PT_PEN";
        return false;
    }

    POINTER_PEN_INFO penInfo;
    if (!api.GetPointerPenInfo(pointerId, &penInfo)) {
        dbgTablet << "GetPointerPenInfo failed";
        return false;
    }

    switch (msg.message) {
    case WM_POINTERDOWN:
        // dbgTablet << "WM_POINTERDOWN";
        break;
    case WM_POINTERUP:
        // dbgTablet << "WM_POINTERUP";
        break;
    case WM_POINTERENTER:
        // dbgTablet << "WM_POINTERENTER";
        break;
    case WM_POINTERLEAVE:
        // dbgTablet << "WM_POINTERLEAVE";
        break;
    case WM_POINTERUPDATE:
        // dbgTablet << "WM_POINTERUPDATE";
        break;
    case WM_POINTERCAPTURECHANGED:
        // dbgTablet << "WM_POINTERCAPTURECHANGED";
        break;
    default:
        dbgTablet << "I missed this message: " << msg.message;
        break;
    }
    // dbgTablet << "  hwnd:        " << penInfo.pointerInfo.hwndTarget;
    // dbgTablet << "  msg hwnd:    " << msg.hwnd;
    // dbgTablet << "  pointerId:   " << pointerId;
    // dbgTablet << "  sourceDevice:" << penInfo.pointerInfo.sourceDevice;
    // dbgTablet << "  pointerFlags:" << penInfo.pointerInfo.pointerFlags;
    // dbgTablet << "  btnChgType:  " << penInfo.pointerInfo.ButtonChangeType;
    // dbgTablet << "  penFlags:    " << penInfo.penFlags;
    // dbgTablet << "  penMask:     " << penInfo.penMask;
    // dbgTablet << "  pressure:    " << penInfo.pressure;
    // dbgTablet << "  rotation:    " << penInfo.rotation;
    // dbgTablet << "  tiltX:       " << penInfo.tiltX;
    // dbgTablet << "  tiltY:       " << penInfo.tiltY;
    // dbgTablet << "  ptPixelLocationRaw:   " << penInfo.pointerInfo.ptPixelLocationRaw;
    // dbgTablet << "  ptHimetricLocationRaw:" << penInfo.pointerInfo.ptHimetricLocationRaw;
    // RECT pointerDeviceRect, displayRect;
    // if (!api.GetPointerDeviceRects(penInfo.pointerInfo.sourceDevice, &pointerDeviceRect, &displayRect)) {
    //     dbgTablet << "GetPointerDeviceRects failed";
    //     return false;
    // }
    // dbgTablet << "  pointerDeviceRect:" << pointerDeviceRect;
    // dbgTablet << "  displayRect:" << displayRect;
    // dbgTablet << "  scaled X:" << static_cast<qreal>(penInfo.pointerInfo.ptHimetricLocationRaw.x) / (pointerDeviceRect.right - pointerDeviceRect.left) * (displayRect.right - displayRect.left);
    // dbgTablet << "  scaled Y:" << static_cast<qreal>(penInfo.pointerInfo.ptHimetricLocationRaw.y) / (pointerDeviceRect.bottom - pointerDeviceRect.top) * (displayRect.bottom - displayRect.top);

    switch (msg.message) {
    case WM_POINTERDOWN:
        return handlePenDownMsg(penInfo);
    case WM_POINTERUP:
        return handlePenUpMsg(penInfo);
    case WM_POINTERENTER:
        return handlePenEnterMsg(penInfo);
    case WM_POINTERLEAVE:
        return handlePenLeaveMsg(penInfo);
    case WM_POINTERUPDATE:
        return handlePenUpdateMsg(penInfo);
    case WM_POINTERCAPTURECHANGED:
        // TODO: Should this event be handled?
        dbgTablet << "FIXME: WM_POINTERCAPTURECHANGED isn't handled";
        break;
    }

    return false;
}

} // namespace
