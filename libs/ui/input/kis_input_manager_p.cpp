/*
 *  SPDX-FileCopyrightText: 2015 Michael Abrahams <miabraha@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_input_manager_p.h"

#include <QMap>
#include <QApplication>
#include <QScopedPointer>
#include <QTimer>
#include <QtGlobal>

#include <boost/preprocessor/repeat_from_to.hpp>

#include "kis_input_manager.h"
#include "kis_config.h"
#include "kis_abstract_input_action.h"
#include "kis_tool_invocation_action.h"
#include "kis_stroke_shortcut.h"
#include "kis_touch_shortcut.h"
#include "kis_native_gesture_shortcut.h"
#include "kis_input_profile_manager.h"
#include "kis_extended_modifiers_mapper.h"

#include "kis_zoom_and_rotate_action.h"
#include "kis_popup_palette.h"
#include "config-qt-patches-present.h"

#include <memory>

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
// for namespace Qt::StringLiterals
#include <KisPortingUtils.h>
#endif


/**
 * This hungry class EventEater encapsulates event masking logic.
 *
 * Its basic role is to kill synthetic mouseMove events sent by Xorg or Qt after
 * tablet events. Those events are sent in order to allow widgets that haven't
 * implemented tablet specific functionality to seamlessly behave as if one were
 * using a mouse. These synthetic events are *supposed* to be optional, or at
 * least come with a flag saying "This is a fake event!!" but neither of those
 * methods is trustworthy. (This is correct as of Qt 5.4 + Xorg.)
 *
 * Qt 5.4 provides no reliable way to see if a user's tablet is being hovered
 * over the pad, since it converts all tablethover events into mousemove, with
 * no option to turn this off. Moreover, sometimes the MouseButtonPress event
 * from the tapping their tablet happens BEFORE the TabletPress event. This
 * means we have to resort to a somewhat complicated logic. What makes this
 * truly a joke is that we are not guaranteed to observe TabletProximityEnter
 * events when we're using a tablet, either, you may only see an Enter event.
 *
 * Once we see tablet events heading our way, we can say pretty confidently that
 * every mouse event is fake. There are two painful cases to consider - a
 * mousePress event could arrive before the tabletPress event, or it could
 * arrive much later, e.g. after tabletRelease. The first was only seen on Linux
 * with Qt's XInput2 code, the solution was to hold onto mousePress events
 * temporarily and wait for tabletPress later, this is contained in git history
 * but is now removed. The second case is currently handled by the
 * eatOneMousePress function, which waits as long as necessary to detect and
 * block a single mouse press event.
 */

static bool isMouseEventType(QEvent::Type t)
{
    return (t == QEvent::MouseMove ||
            t == QEvent::MouseButtonPress ||
            t == QEvent::MouseButtonRelease ||
            t == QEvent::MouseButtonDblClick);
}

static bool isTabletEventType(QEvent::Type t)
{
    return (t == QEvent::TabletMove ||
            t == QEvent::TabletPress ||
            t == QEvent::TabletRelease);
}

static bool isTouchEventType(QEvent::Type t)
{
    return (t == QEvent::TouchBegin ||
            t == QEvent::TouchEnd ||
            t == QEvent::TouchUpdate ||
            t == QEvent::TouchCancel);
}

static bool isNativeGestureEventType(QEvent::Type t)
{
    return t == QEvent::NativeGesture;
}

KisInputManager::Private::EventEater::EventEater()
{
    KisConfig cfg(true);
    activateSecondaryButtonsWorkaround = cfg.useRightMiddleTabletButtonWorkaround();
}

void KisInputManager::Private::EventEater::debugEaterStateTransition(const QLatin1String &stateName, bool newValue, QEvent::Type eventType, const QLatin1String &comment)
{
    if (comment.isEmpty()) {
        dbgInputEater.noquote().nospace() << stateName << " <- " << newValue << " by " << eventType;
    } else {
        dbgInputEater.noquote().nospace() << stateName << " <- " << newValue << " by " << eventType << " (" << comment << ")";
    }
}
bool KisInputManager::Private::EventEater::eventFilter(QObject* target, QEvent* event )
{
    using namespace Qt::StringLiterals;

    Q_UNUSED(target);

    {
        // handle `newTabletIsHovering` and `tabletIsPressed` state transitions

        if (isTabletEventType(event->type())) {

            QTabletEvent *ev = static_cast<QTabletEvent*>(event);
            const bool newTabletIsHovering = ev->buttons() == Qt::NoButton;
            const bool newTabletIsPressed = !newTabletIsHovering;

            if (newTabletIsHovering != tabletIsHovering) {
                tabletIsHovering = newTabletIsHovering;
                debugEaterStateTransition("tabletIsHovering"_L1, newTabletIsHovering, event->type());
            }

            if (newTabletIsPressed != tabletIsPressed) {
                tabletIsPressed = newTabletIsPressed;
                debugEaterStateTransition("tabletIsPressed"_L1, newTabletIsPressed, event->type());
            }
        }

        if (event->type() == QEvent::FocusOut ||
            event->type() == QEvent::Leave ||
            /**
             * Tablet events have higher priority over the touch events, so, if a touch has been
             * started **before** the tablet stroke, then TouchUpdate events will no cause
             * the EventEater to perform state transitions to the tablet state (on Windows, the
             * streams of touch and tablet event come in parallel independently).
             *
             * Please note that the actual cancellation of the already started touch
             * action is performed inside KisShortcutMatcher, not in the event eater.
             * Here we just allow all the TouchUpdate events pass firther and block synthesized
             * mouse events only.
             */
            (event->type() == QEvent::TouchBegin && !tabletIsInProximity)) {

            if (tabletIsHovering) {
                tabletIsHovering = false;
                debugEaterStateTransition("tabletIsHovering"_L1, false, event->type());
            }
            if (tabletIsPressed) {
                tabletIsPressed = false;
                debugEaterStateTransition("tabletIsPressed"_L1, false, event->type());
            }
        }

        // also proximity-leave in a corresponding function...
    }

    {
        // handle `touchIsActive` state transitions

        if (event->type() == QEvent::TouchBegin ||
            event->type() == QEvent::TouchUpdate) {

            if (!touchIsActive) {
                touchIsActive = true;
                debugEaterStateTransition("touchIsActive"_L1, true, event->type());
            }
        }

        if (event->type() == QEvent::TouchEnd ||
            event->type() == QEvent::TouchCancel) {
            if (touchIsActive) {
                touchIsActive = false;
                debugEaterStateTransition("touchIsActive"_L1, false, event->type());
            }
        }
    }

    if (eatOneMousePressEvent && event->type() == QEvent::MouseButtonPress
        // Drop one mouse press following tabletPress or touchBegin
        && (static_cast<QMouseEvent*>(event)->button() == Qt::LeftButton)) {
        eatOneMousePressEvent = false;
        KisInputManager::Private::debugEvent<QMouseEvent>(event, BlockedByNextPressSuppression);
        return true;
    }

    if (activateSecondaryButtonsWorkaround) {
        if (event->type() == QEvent::TabletPress ||
                event->type() == QEvent::TabletRelease) {

            QTabletEvent *te = static_cast<QTabletEvent*>(event);
            if (te->button() != Qt::LeftButton) {
                KisInputManager::Private::debugEvent<QTabletEvent>(te, BlockedByButtonsWorkaround);
                return true;
            }
        } else if (event->type() == QEvent::MouseButtonPress ||
                   event->type() == QEvent::MouseButtonRelease ||
                   event->type() == QEvent::MouseButtonDblClick) {

            QMouseEvent *me = static_cast<QMouseEvent*>(event);
            if (me->button() != Qt::LeftButton) {
                return false;
            }
        }
    }

    const bool shouldEatMouseEvents =
        // On MacOS Qt sends fake mouse events instead of hover events,
        // so we should not block mouse events when the tablet just hovers
        // the canvas. The real TabletMove events will arrive only when
        // the user touches the surface of the tablet, i.e. after a TabletPress.
#ifndef Q_OS_MACOS
        tabletIsInProximity ||
        tabletIsHovering ||
#endif
        tabletIsPressed ||
        touchIsActive;

    if (isMouseEventType(event->type()) &&
               (shouldEatMouseEvents
            // On Mac, we need mouse events when the tablet is in proximity, but not pressed down
            // since tablet move events are not generated until after tablet press.
            #ifndef Q_OS_MAC
                || (static_cast<QMouseEvent*>(event)->source() != Qt::MouseEventNotSynthesized)
            #endif
                )) {

        EventBlockingReasons reasons = NotBlocked;
#ifndef Q_OS_MACOS
        reasons.setFlag(BlockedByTabletProximity, tabletIsInProximity);
        reasons.setFlag(BlockedByTabletHover, tabletIsHovering);
        reasons.setFlag(BlockedBySynthetic, static_cast<QMouseEvent*>(event)->source() != Qt::MouseEventNotSynthesized);
#endif
        reasons.setFlag(BlockedByTabletPress, tabletIsPressed);
        reasons.setFlag(BlockedByTouchPress, touchIsActive);

        KisInputManager::Private::debugEvent<QMouseEvent>(event, reasons);

        // Drop mouse events if enabled or event was synthetic & synthetic events are disabled
        return true;
    }

    if (event->type() == QEvent::TouchBegin &&
        (tabletIsInProximity || tabletIsHovering || tabletIsPressed)) {

        EventBlockingReasons reasons = NotBlocked;
        reasons.setFlag(BlockedByTabletProximity, tabletIsInProximity);
        reasons.setFlag(BlockedByTabletHover, tabletIsHovering);
        reasons.setFlag(BlockedByTabletPress, tabletIsPressed);

        KisInputManager::Private::debugEvent<QTouchEvent>(event, reasons);

        if (touchIsActive) {
            // don't forget to reset the touch active state, since we are
            // not going to receive the res of the events
            touchIsActive = false;
            debugEaterStateTransition("touchIsActive"_L1, false, event->type(), "cancelled by tablet"_L1);
        }

        // Drop touch-begin event. It will prevent KisShortcutMatcher to
        // start any touch action, while letting it complete any existing
        // actions. We explicitly accept the event to make sure OS will
        // not trigger any global gestures (works on WinInk only).
        event->accept();
        return true;
    }

    if (isMouseEventType(event->type())) {
        KisInputManager::Private::debugEvent<QMouseEvent>(event, NotBlocked);
    } else if (isTabletEventType(event->type())) {
        KisInputManager::Private::debugEvent<QTabletEvent>(event, NotBlocked);
    } else if (isTouchEventType(event->type())) {
        KisInputManager::Private::debugEvent<QTouchEvent>(event, NotBlocked);
    } else if (isNativeGestureEventType(event->type())) {
        KisInputManager::Private::debugEvent<QNativeGestureEvent>(event, NotBlocked);
    }

    return false; // All clear - let this one through!
}

void KisInputManager::Private::EventEater::notifyTabletEnterProximity()
{
    using namespace Qt::StringLiterals;

    tabletIsInProximity = true;
    debugEaterStateTransition("tabletIsInProximity"_L1, true, QEvent::TabletEnterProximity);
}

void KisInputManager::Private::EventEater::notifyTabletLeaveProximity()
{
    using namespace Qt::StringLiterals;

    tabletIsInProximity = false;
    debugEaterStateTransition("tabletIsInProximity"_L1, false, QEvent::TabletLeaveProximity);

    if (tabletIsHovering) {
        tabletIsHovering = false;
        debugEaterStateTransition("tabletIsHovering"_L1, false, QEvent::TabletLeaveProximity);
    }
}

void KisInputManager::Private::EventEater::eatOneMousePress()
{
    // Enable on other platforms if getting full-pressure splotches
    eatOneMousePressEvent = true;
}

void KisInputManager::Private::setMaskSyntheticEvents(bool value)
{
    Q_UNUSED(value)
    // TODO: remove
}

KisInputManager::Private::Private(KisInputManager *qq)
    : q(qq)
    , moveEventCompressor(10 /* ms */,
                          KisSignalCompressor::FIRST_ACTIVE,
                          KisSignalCompressor::ADDITIVE_INTERVAL)
    , priorityEventFilterSeqNo(0)
    , popupWidget(nullptr)
    , canvasSwitcher(this, qq)
{
    KisConfig cfg(true);

    moveEventCompressor.setDelay(cfg.tabletEventsDelay());
    testingAcceptCompressedTabletEvents = cfg.testingAcceptCompressedTabletEvents();
    testingCompressBrushEvents = cfg.testingCompressBrushEvents();

    if (cfg.trackTabletEventLatency()) {
        tabletLatencyTracker = new TabletLatencyTracker();
    }

    matcher.setInputActionGroupsMaskCallback(
        [this] () {
            return this->canvas ? this->canvas->inputActionGroupsMaskInterface()->inputActionGroupsMask() : AllActionGroup;
        });

    /**
     * On Windows and Linux we have a proper fix for this bug
     * patched into our local version of Qt. We don't have a fix
     * for macOS
     */
#ifdef Q_OS_MACOS
    useUnbalancedKeyPressEventWorkaround = true;
#endif

    /**
     * In Linux distributions Qt is not patched, so we should
     * use workaround for them
     */
#if defined Q_OS_LINUX &&  !KRITA_QT_HAS_UNBALANCED_KEY_PRESS_RELEASE_PATCH
    useUnbalancedKeyPressEventWorkaround = true;
#endif

    if (qEnvironmentVariableIsSet("KRITA_FIX_UNBALANCED_KEY_EVENTS")) {
        useUnbalancedKeyPressEventWorkaround = qEnvironmentVariableIntValue("KRITA_FIX_UNBALANCED_KEY_EVENTS");
    }
}

static const int InputWidgetsThreshold = 2000;
static const int OtherWidgetsThreshold = 400;

KisInputManager::Private::CanvasSwitcher::CanvasSwitcher(Private *_d, QObject *p)
    : QObject(p),
      d(_d),
      eatOneMouseStroke(false),
      focusSwitchThreshold(InputWidgetsThreshold)
{
}

void KisInputManager::Private::CanvasSwitcher::setupFocusThreshold(QObject* object)
{
    QWidget *widget = qobject_cast<QWidget*>(object);
    KIS_SAFE_ASSERT_RECOVER_RETURN(widget);

    thresholdConnections.clear();
    thresholdConnections.addConnection(&focusSwitchThreshold, SIGNAL(timeout()), widget, SLOT(setFocus()));
}

void KisInputManager::Private::CanvasSwitcher::addCanvas(KisCanvas2 *canvas)
{
    if (!canvas) return;

    QObject *canvasWidget = canvas->canvasWidget();

    if (!canvasResolver.contains(canvasWidget)) {
        canvasResolver.insert(canvasWidget, canvas);
    } else {
        // just a sanity cheek to find out if we are
        // trying to add two canvases concurrently.
        KIS_SAFE_ASSERT_RECOVER_NOOP(d->canvas == canvas);
    }

    if (canvas != d->canvas) {
        d->q->setupAsEventFilter(canvasWidget);
        canvasWidget->installEventFilter(this);

        setupFocusThreshold(canvasWidget);
        focusSwitchThreshold.setEnabled(false);

        d->canvas = canvas;
        d->toolProxy = qobject_cast<KisToolProxy*>(canvas->toolProxy());
    }
}

void KisInputManager::Private::CanvasSwitcher::removeCanvas(KisCanvas2 *canvas)
{
    QObject *widget = canvas->canvasWidget();

    canvasResolver.remove(widget);

    if (d->eventsReceiver == widget) {
        d->q->setupAsEventFilter(0);
    }

    widget->removeEventFilter(this);

    if (d->canvas == canvas) {
        d->canvas = 0;
        d->toolProxy = 0;
    }
}

bool isInputWidget(QWidget *w)
{
    if (!w) return false;


    QList<QLatin1String> types;
    types << QLatin1String("QAbstractSlider");
    types << QLatin1String("QAbstractSpinBox");
    types << QLatin1String("QLineEdit");
    types << QLatin1String("QTextEdit");
    types << QLatin1String("QPlainTextEdit");
    types << QLatin1String("QComboBox");
    types << QLatin1String("QKeySequenceEdit");

    Q_FOREACH (const QLatin1String &type, types) {
        if (w->inherits(type.data())) {
            return true;
        }
    }

    return false;
}

bool KisInputManager::Private::CanvasSwitcher::eventFilter(QObject* object, QEvent* event )
{
    if (canvasResolver.contains(object)) {
        switch (event->type()) {
        case QEvent::FocusIn: {
            QFocusEvent *fevent = static_cast<QFocusEvent*>(event);
            KisCanvas2 *canvas = canvasResolver.value(object);

            // only relevant canvases from the same main window should be
            // registered in the switcher
            KIS_SAFE_ASSERT_RECOVER_BREAK(canvas);

            if (canvas != d->canvas) {
                eatOneMouseStroke = 2 * (fevent->reason() == Qt::MouseFocusReason);
            }

            d->canvas = canvas;
            d->toolProxy = qobject_cast<KisToolProxy*>(canvas->toolProxy());

            d->q->setupAsEventFilter(object);

            object->removeEventFilter(this);
            object->installEventFilter(this);

            setupFocusThreshold(object);
            focusSwitchThreshold.setEnabled(false);

            const QPoint globalPos = QCursor::pos();
            const QPoint localPos = d->canvas->canvasWidget()->mapFromGlobal(globalPos);
            QWidget *canvasWindow = d->canvas->canvasWidget()->window();
            const QPoint windowsPos = canvasWindow ? canvasWindow->mapFromGlobal(globalPos) : localPos;

            QEnterEvent event(localPos, windowsPos, globalPos);
            d->q->eventFilter(object, &event);
            break;
        }
        case QEvent::FocusOut: {
            focusSwitchThreshold.setEnabled(true);
            break;
        }
        case QEvent::Enter: {
            break;
        }
        case QEvent::Leave: {
            focusSwitchThreshold.stop();
            break;
        }
        case QEvent::Wheel: {
            QWidget *widget = static_cast<QWidget*>(object);
            widget->setFocus();
            break;
        }
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::TabletPress:
        case QEvent::TabletRelease:
            focusSwitchThreshold.forceDone();

            if (eatOneMouseStroke) {
                eatOneMouseStroke--;
                return true;
            }
            break;
        case QEvent::MouseButtonDblClick:
            focusSwitchThreshold.forceDone();
            if (eatOneMouseStroke) {
                return true;
            }
            break;
        case QEvent::MouseMove:
        case QEvent::TabletMove: {
            QWidget *widget = static_cast<QWidget*>(object);

            if (!widget->hasFocus()) {
                const int delay =
                    isInputWidget(QApplication::focusWidget()) ?
                    InputWidgetsThreshold : OtherWidgetsThreshold;

                focusSwitchThreshold.setDelayThreshold(delay);
                focusSwitchThreshold.start();
            }
        }
            break;
        default:
            break;
        }
    }
    return QObject::eventFilter(object, event);
}

KisInputManager::Private::ProximityNotifier::ProximityNotifier(KisInputManager::Private *_d, QObject *p)
    : QObject(p), d(_d)
{}

bool KisInputManager::Private::ProximityNotifier::eventFilter(QObject* object, QEvent* event )
{
    switch (event->type()) {
    case QEvent::TabletEnterProximity:
        d->debugEvent<QEvent>(event);
        d->eventEater.notifyTabletEnterProximity();
        break;
    case QEvent::TabletLeaveProximity:
        d->debugEvent<QEvent>(event);
        d->eventEater.notifyTabletLeaveProximity();
        break;
#ifdef Q_OS_WIN
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
    case QEvent::ShortcutOverride:
        if (d->ignoreHighFunctionKeys) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            int key = keyEvent->key();

            if (key >= Qt::Key_F13 && key <= Qt::Key_F35) {
                if (KisTabletDebugger::instance()->debugEnabled()) {
                    const QString pre = "[BLOCKED HIGH F-KEY]";
                    dbgTablet << KisTabletDebugger::instance()->eventToString(*keyEvent, pre);
                }
                return true;
            }
            break;
        }
#endif /* Q_OS_WIN */
    default:
        break;
    }
    return QObject::eventFilter(object, event);
}

#define EXTRA_BUTTON(z, n, _) \
    if(buttons & Qt::ExtraButton##n) { \
        buttonSet << Qt::ExtraButton##n; \
    }

void KisInputManager::Private::addStrokeShortcut(KisAbstractInputAction* action, int index,
                                                 const QList<Qt::Key> &modifiers,
                                                 Qt::MouseButtons buttons)
{
    KisStrokeShortcut *strokeShortcut =
        new KisStrokeShortcut(action, index);

    QSet<Qt::MouseButton> buttonSet;
    if(buttons & Qt::LeftButton) {
        buttonSet << Qt::LeftButton;
    }
    if(buttons & Qt::RightButton) {
        buttonSet << Qt::RightButton;
    }
    if(buttons & Qt::MiddleButton) {
        buttonSet << Qt::MiddleButton;
    }

BOOST_PP_REPEAT_FROM_TO(1, 25, EXTRA_BUTTON, _)

    if (!buttonSet.empty()) {
        strokeShortcut->setButtons(QSet<Qt::Key>(modifiers.cbegin(), modifiers.cend()), buttonSet);
        matcher.addShortcut(strokeShortcut);
    }
    else {
        delete strokeShortcut;
    }
}

void KisInputManager::Private::addKeyShortcut(KisAbstractInputAction* action, int index,
                                              const QList<Qt::Key> &keys)
{
    if (keys.size() == 0) return;

    KisSingleActionShortcut *keyShortcut =
        new KisSingleActionShortcut(action, index);

    //Note: Ordering is important here, Shift + V is different from V + Shift,
    //which is the reason we use the last key here since most users will enter
    //shortcuts as "Shift + V". Ideally this should not happen, but this is
    //the way the shortcut matcher is currently implemented.
    QList<Qt::Key> allKeys = keys;
    Qt::Key key = allKeys.takeLast();
    QSet<Qt::Key> modifiers = QSet<Qt::Key>(allKeys.begin(), allKeys.end());
    keyShortcut->setKey(modifiers, key);
    matcher.addShortcut(keyShortcut);
}

void KisInputManager::Private::addWheelShortcut(KisAbstractInputAction* action, int index,
                                                const QList<Qt::Key> &modifiers,
                                                KisShortcutConfiguration::MouseWheelMovement wheelAction)
{
    std::unique_ptr<KisSingleActionShortcut> keyShortcut(
        new KisSingleActionShortcut(action, index));

    KisSingleActionShortcut::WheelAction a;
    switch(wheelAction) {
    case KisShortcutConfiguration::WheelUp:
        a = KisSingleActionShortcut::WheelUp;
        break;
    case KisShortcutConfiguration::WheelDown:
        a = KisSingleActionShortcut::WheelDown;
        break;
    case KisShortcutConfiguration::WheelLeft:
        a = KisSingleActionShortcut::WheelLeft;
        break;
    case KisShortcutConfiguration::WheelRight:
        a = KisSingleActionShortcut::WheelRight;
        break;
    case KisShortcutConfiguration::WheelTrackpad:
        a = KisSingleActionShortcut::WheelTrackpad;
        break;
    default:
        return;
    }
    keyShortcut->setWheel(QSet<Qt::Key>(modifiers.begin(), modifiers.end()), a);
    matcher.addShortcut(keyShortcut.release());
}

void KisInputManager::Private::addTouchShortcut(KisAbstractInputAction* action, int index, KisShortcutConfiguration::TouchGestureAction gesture, bool isTouchPainting)
{
    KisTouchShortcut *shortcut = new KisTouchShortcut(action, index, gesture);
    if (isTouchPainting) {
        shortcut->setIsTouchPainting(true);
        shortcut->setMinDragThreshold(1.5); // min threshold for touch painting is 1.5 px
    }
    dbgKrita << "TouchAction:" << action->name() << (isTouchPainting ? "touch-painting" : "");
    switch(gesture) {
    case KisShortcutConfiguration::OneFingerTap:
    case KisShortcutConfiguration::OneFingerDrag:
    case KisShortcutConfiguration::OneFingerHold:
        shortcut->setMinimumTouchPoints(1);
        shortcut->setMaximumTouchPoints(1);
        break;
    case KisShortcutConfiguration::TwoFingerTap:
    case KisShortcutConfiguration::TwoFingerDrag:
        shortcut->setMinimumTouchPoints(2);
        shortcut->setMaximumTouchPoints(2);
        break;
    case KisShortcutConfiguration::ThreeFingerTap:
    case KisShortcutConfiguration::ThreeFingerDrag:
        shortcut->setMinimumTouchPoints(3);
        shortcut->setMaximumTouchPoints(3);
        break;
    case KisShortcutConfiguration::FourFingerTap:
    case KisShortcutConfiguration::FourFingerDrag:
        shortcut->setMinimumTouchPoints(4);
        shortcut->setMaximumTouchPoints(4);
        break;
    case KisShortcutConfiguration::FiveFingerTap:
    case KisShortcutConfiguration::FiveFingerDrag:
        shortcut->setMinimumTouchPoints(5);
        shortcut->setMaximumTouchPoints(5);
    default:
        break;
    }
    matcher.addShortcut(shortcut);
}

bool KisInputManager::Private::addNativeGestureShortcut(KisAbstractInputAction* action, int index, KisShortcutConfiguration::NativeGestureAction gesture)
{
    // Qt5 only implements QNativeGestureEvent for macOS
    // Qt6 implements QNativeGestureEvent for macOS and Wayland
    KisNativeGestureShortcut::Type type = KisNativeGestureShortcut::PinchNavigation;
    switch (gesture) {
        case KisShortcutConfiguration::PinchGesture:
            type = KisNativeGestureShortcut::PinchNavigation;
            break;
        case KisShortcutConfiguration::SmartZoomGesture:
            type = KisNativeGestureShortcut::SmartZoomNativeGesture;
            break;
        default:
            return false;
    }

    KisNativeGestureShortcut *shortcut = new KisNativeGestureShortcut(action, index, type);
    matcher.addShortcut(shortcut);
    return true;
}

void KisInputManager::Private::setupActions()
{
    QList<KisAbstractInputAction*> actions = KisInputProfileManager::instance()->actions();
    Q_FOREACH (KisAbstractInputAction *action, actions) {
        KisToolInvocationAction *toolAction =
            dynamic_cast<KisToolInvocationAction*>(action);

        if(toolAction) {
            defaultInputAction = toolAction;
        }
    }

    connect(KisInputProfileManager::instance(), SIGNAL(currentProfileChanged()), q, SLOT(profileChanged()));
    if(KisInputProfileManager::instance()->currentProfile()) {
        q->profileChanged();
    }
}

bool KisInputManager::Private::processUnhandledEvent(QEvent *event)
{
    bool retval = false;

    if (forwardAllEventsToTool ||
        event->type() == QEvent::KeyPress ||
        event->type() == QEvent::KeyRelease) {

        defaultInputAction->processUnhandledEvent(event);
        retval = true;
    }

    return retval && !forwardAllEventsToTool;
}

void KisInputManager::Private::resetCompressor() {
    compressedMoveEvent.reset();
    moveEventCompressor.stop();
}

bool KisInputManager::Private::handleCompressedTabletEvent(QEvent *event)
{
    bool retval = false;

    if (!matcher.pointerMoved(event) && toolProxy) {
        toolProxy->forwardHoverEvent(event);
    }
    retval = true;
    event->setAccepted(true);

    return retval;
}

void KisInputManager::Private::fixShortcutMatcherModifiersState()
{
    KisExtendedModifiersMapper mapper;

    QVector<Qt::Key> newKeys;
    Qt::KeyboardModifiers modifiers = mapper.queryStandardModifiers();
    Q_FOREACH (Qt::Key key, mapper.queryExtendedModifiers()) {
        QKeyEvent kevent(QEvent::ShortcutOverride, key, modifiers);
        newKeys << KisExtendedModifiersMapper::workaroundShiftAltMetaHell(&kevent);
    }

    fixShortcutMatcherModifiersState(newKeys, modifiers);
}

void KisInputManager::Private::fixShortcutMatcherModifiersState(QVector<Qt::Key> newKeys, Qt::KeyboardModifiers modifiers)
{
    QVector<Qt::Key> danglingKeys = matcher.debugPressedKeys();

    matcher.handlePolledKeys(newKeys);

    for (auto it = danglingKeys.begin(); it != danglingKeys.end();) {
        if (newKeys.contains(*it)) {
            newKeys.removeOne(*it);
            it = danglingKeys.erase(it);
        } else {
            ++it;
        }
    }

    /**
     * This function may be called from slotConfigChanged() from
     * the constructor of KisInputManager. It may theoretically
     * happen that we haven't recieved any sensible events to
     * set up the link to the input manager.
     */
    KisAbstractInputAction::setInputManager(q);

    Q_FOREACH (Qt::Key key, danglingKeys) {
        QKeyEvent kevent(QEvent::KeyRelease, key, modifiers);
        processUnhandledEvent(&kevent);
    }

    Q_FOREACH (Qt::Key key, newKeys) {
        // just replay the whole sequence
        {
            QKeyEvent kevent(QEvent::ShortcutOverride, key, modifiers);
            processUnhandledEvent(&kevent);
        }
        {
            QKeyEvent kevent(QEvent::KeyPress, key, modifiers);
            processUnhandledEvent(&kevent);
        }
    }
}

qint64 KisInputManager::Private::TabletLatencyTracker::currentTimestamp() const
{
    // on OS X, we need to compute the timestamp that compares correctly against the native event timestamp,
    // which seems to be the msecs since system startup. On Linux with WinTab, we produce the timestamp that
    // we compare against ourselves in QWindowSystemInterface.

    QElapsedTimer elapsed;
    elapsed.start();
    return elapsed.msecsSinceReference();
}

void KisInputManager::Private::TabletLatencyTracker::print(const QString &message)
{
    dbgTablet << qUtf8Printable(message);
}
