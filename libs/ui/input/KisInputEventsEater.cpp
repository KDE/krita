/*
 *  SPDX-FileCopyrightText: 2015 Michael Abrahams <miabraha@gmail.com>
 *  SPDX-FileCopyrightText: 2026 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisInputEventsEater.h"

#include <kis_config.h>

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

KisInputEventsEater::KisInputEventsEater()
{
    KisConfig cfg(true);
    activateSecondaryButtonsWorkaround = cfg.useRightMiddleTabletButtonWorkaround();
}

void KisInputEventsEater::debugEaterStateTransition(const QLatin1String &stateName, bool newValue, QEvent::Type eventType, const QLatin1String &comment)
{
    if (comment.isEmpty()) {
        dbgInputEater.noquote().nospace() << stateName << " <- " << newValue << " by " << eventType;
    } else {
        dbgInputEater.noquote().nospace() << stateName << " <- " << newValue << " by " << eventType << " (" << comment << ")";
    }
}
bool KisInputEventsEater::eventFilter(QObject* target, QEvent* event )
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

#ifdef KRITA_ENABLE_MISSING_PROXIMITY_ENTER_WORKAROUND
            if (!tabletIsInProximity && !missingTabletProximityEventsWorkaround) {
                warnInputEater << "WARNING: received a tablet event while the tablet is not in proximity! Activating a workaround...";
                missingTabletProximityEventsWorkaround = MissingTabletProximityEnterWorkaround();
            } else if (tabletIsInProximity && missingTabletProximityEventsWorkaround) {
                warnInputEater << "WARNING: received a delayed tablet proximity event, disabling the workaround...";
                missingTabletProximityEventsWorkaround = std::nullopt;
            } else if (missingTabletProximityEventsWorkaround) {
                missingTabletProximityEventsWorkaround->notifyTabletEventArrived();
            }
#endif /* KRITA_ENABLE_MISSING_PROXIMITY_ENTER_WORKAROUND */
        }

#ifdef KRITA_ENABLE_MISSING_PROXIMITY_ENTER_WORKAROUND
        if (missingTabletProximityEventsWorkaround && !tabletIsPressed && isMouseEventType(event->type())) {
            missingTabletProximityEventsWorkaround->notifyMouseEventArrived();
            if (missingTabletProximityEventsWorkaround->shouldForgetAboutTabletInProximity()) {
                if (tabletIsHovering) {
                    tabletIsHovering = false;
                    debugEaterStateTransition("tabletIsHovering"_L1, false, event->type(), "missing proximity workaround"_L1);
                }
                missingTabletProximityEventsWorkaround = std::nullopt;
            }
        }
#endif /* KRITA_ENABLE_MISSING_PROXIMITY_ENTER_WORKAROUND */

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

#ifdef KRITA_ENABLE_MISSING_PROXIMITY_ENTER_WORKAROUND
            missingTabletProximityEventsWorkaround = std::nullopt;
#endif /* KRITA_ENABLE_MISSING_PROXIMITY_ENTER_WORKAROUND */
        }

        // also proximity-leave in a corresponding function...
    }

    {
        // handle `touchIsActive` state transitions

        bool assumeTouchStarted = event->type() == QEvent::TouchBegin;

        if (!assumeTouchStarted && event->type() == QEvent::TouchUpdate) {
            QTouchEvent *touchEvent = static_cast<QTouchEvent*>(event);

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
            using DeviceType = QInputDevice::DeviceType;
#else
            using DeviceType = QTouchDevice::DeviceType;
#endif

            if (touchEvent->device()->type() == DeviceType::TouchPad) {
#ifdef Q_OS_MACOS
                /**
                 * On MacOS TouchUpdate events arrive for normal touchpad
                 * movements. These TouchUpdate events have lower resolution
                 * and they are not wrapped into TouchBegin/TouchEnd pair.
                 * So we just allow synthesized events for them.
                 *
                 * For all other systems TouchUpdate events arrive for
                 * touch screens only, and only inside a valid gesture, so
                 * we should block mouse events for them.
                 *
                 * Also see a comment about hover events in the handler
                 * of QEvent::TouchUpdate in KisInputManager::eventFilter.
                 */

                 // noop, keep assumeTouchStarted false
#else
                dbgInputEater << "Received a TouchUpdate for a touchpad device on a non-MacOS system!";
#endif
            } else {
                assumeTouchStarted = true;
            }
        }

        if (assumeTouchStarted) {
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
        debugEvent<QMouseEvent>(event, BlockedByNextPressSuppression);
        return true;
    }

    if (activateSecondaryButtonsWorkaround) {
        if (event->type() == QEvent::TabletPress ||
                event->type() == QEvent::TabletRelease) {

            QTabletEvent *te = static_cast<QTabletEvent*>(event);
            if (te->button() != Qt::LeftButton) {
                debugEvent<QTabletEvent>(te, BlockedByButtonsWorkaround);
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

#ifdef Q_OS_MACOS
    /**
     * MacOS is the only platform where we can differentiate between
     * wheel events generated by a mouse and by a touchpad. More than
     * that, on MacOS touchpad generates NativeGesture events, hence
     * we should just blocked all the synthetic wheel events.
     */

    if (event->type() == QEvent::Wheel) {
        QWheelEvent *we = static_cast<QWheelEvent *>(event);
        if (we->source() == Qt::MouseEventSource::MouseEventSynthesizedBySystem) {
            debugEvent<QWheelEvent>(event, BlockedBySynthetic);
            return true;
        }
    }
#endif

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

        debugEvent<QMouseEvent>(event, reasons);

        // Drop mouse events if enabled or event was synthetic & synthetic events are disabled
        return true;
    }

    if (event->type() == QEvent::TouchBegin &&
        (tabletIsInProximity || tabletIsHovering || tabletIsPressed)) {

        EventBlockingReasons reasons = NotBlocked;
        reasons.setFlag(BlockedByTabletProximity, tabletIsInProximity);
        reasons.setFlag(BlockedByTabletHover, tabletIsHovering);
        reasons.setFlag(BlockedByTabletPress, tabletIsPressed);

        debugEvent<QTouchEvent>(event, reasons);

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
        debugEvent<QMouseEvent>(event, NotBlocked);
    } else if (isTabletEventType(event->type())) {
        debugEvent<QTabletEvent>(event, NotBlocked);
    } else if (isTouchEventType(event->type())) {
        debugEvent<QTouchEvent>(event, NotBlocked);
    } else if (isNativeGestureEventType(event->type())) {
        debugEvent<QNativeGestureEvent>(event, NotBlocked);
    }

    return false; // All clear - let this one through!
}

void KisInputEventsEater::notifyTabletEnterProximity()
{
    using namespace Qt::StringLiterals;

    tabletIsInProximity = true;
    debugEaterStateTransition("tabletIsInProximity"_L1, true, QEvent::TabletEnterProximity);
}

void KisInputEventsEater::notifyTabletLeaveProximity()
{
    using namespace Qt::StringLiterals;

    tabletIsInProximity = false;
    debugEaterStateTransition("tabletIsInProximity"_L1, false, QEvent::TabletLeaveProximity);

    if (tabletIsHovering) {
        tabletIsHovering = false;
        debugEaterStateTransition("tabletIsHovering"_L1, false, QEvent::TabletLeaveProximity);
    }
}

void KisInputEventsEater::eatOneMousePress()
{
    // Enable on other platforms if getting full-pressure splotches
    eatOneMousePressEvent = true;
}
