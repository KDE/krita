/*
 *  SPDX-FileCopyrightText: 2012 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_shortcut_matcher.h"

#include <QEvent>
#include <QMouseEvent>
#include <QTabletEvent>

#include "kis_assert.h"
#include "kis_abstract_input_action.h"
#include "kis_stroke_shortcut.h"
#include "kis_touch_shortcut.h"
#include "kis_native_gesture_shortcut.h"
#include "kis_config.h"
#include "kis_extended_modifiers_mapper.h"
#include "KisTouchHoldEventsPostponer.h"
#include <KoPointerEvent.h>

#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
// for QMutableEventPoint
#include <QWindow>
#include <QtGui/private/qeventpoint_p.h>
#endif

inline QString debugShortcutName(const KisAbstractShortcut *shortcut)
{
    auto action = shortcut->action();
    return QString("%1/%2").arg(action->name()).arg(action->shortcutIndexes().key(shortcut->shortcutIndex(), "<unknown>"));
}

#define DEBUG_ACTION(text) dbgInputMatcher << __FUNCTION__ << "-" << text;
#define DEBUG_SHORTCUT(text, shortcut) dbgInputMatcher << __FUNCTION__ << "-" << text << "act:" << debugShortcutName(shortcut);
#define DEBUG_KEY(text) dbgInputMatcher << __FUNCTION__ << "-" << text << "keys:" << m_d->keys;
#define DEBUG_BUTTON_ACTION(text, button) dbgInputMatcher << __FUNCTION__ << "-" << text << "button:" << button << "btns:" << m_d->buttons << "keys:" << m_d->keys;
#define DEBUG_EVENT_ACTION(text, event) if (event) {dbgInputMatcher << __FUNCTION__ << "-" << text << "type:" << event->type();}
#define DEBUG_TOUCH_ACTION(text, event)                                                                                \
    if (event) {                                                                                                       \
        dbgInputMatcher << __FUNCTION__ << "-" << text << "type:" << event->type()                                    \
                        << "tps:" << KisTouchShortcut::countTouchPoints(event, KisTouchShortcut::allTouchStates())    \
                        << "pressedTps:" << KisTouchShortcut::countTouchPoints(event, KisTouchShortcut::pressedOnlyTouchStates())    \
                        << "maxTps:" << m_d->maxTouchPoints << "drag:" << m_d->isTouchDragDetected;                   \
    }


namespace
{
QTouchEvent generateFakeTouchEndEvent(const QTouchEvent *event)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    auto points = event->touchPoints();
#else
    auto points = event->points();
#endif

    for (auto it = points.begin(); it != points.end();) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        if (it->state() == Qt::TouchPointReleased) {
            it = points.erase(it);
        } else {
            it->setState(Qt::TouchPointReleased);
            ++it;
        }
#else
        if (it->state() == QEventPoint::Released) {
            it = points.erase(it);
        } else {
           QMutableEventPoint::setState(*it, QEventPoint::Released);
           ++it;
        }
#endif
    }

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    return QTouchEvent(QEvent::TouchEnd, event->device(), event->modifiers(), Qt::TouchPointReleased, points);
#else
    return QTouchEvent(QEvent::TouchEnd, event->pointingDevice(), event->modifiers(), points);
#endif

}

} // namespace

class Q_DECL_HIDDEN KisShortcutMatcher::Private
{
public:
    Private()
        : runningShortcut(0)
        , readyShortcut(0)
        , touchShortcut(0)
        , nativeGestureShortcut(0)
        , actionGroupMask([] () { return AllActionGroup; })
        , suppressAllActions(false)
        , suppressAllKeyboardActions(false)
        , cursorEntered(false)
    {}

    ~Private()
    {
        qDeleteAll(singleActionShortcuts);
        qDeleteAll(strokeShortcuts);
        qDeleteAll(touchShortcuts);
    }

    QList<KisSingleActionShortcut*> singleActionShortcuts;
    QSet<KisSingleActionShortcut*> suppressedSingleActionShortcuts;
    QList<KisStrokeShortcut*> strokeShortcuts;
    QList<KisTouchShortcut*> touchShortcuts;
    QList<KisNativeGestureShortcut*> nativeGestureShortcuts;

    QSet<Qt::Key> keys; // Model of currently pressed keys
    QSet<Qt::MouseButton> buttons; // Model of currently pressed buttons

    QSet<Qt::Key> polledKeys; // Keys that were polled using native platform APIs and thus need to be treated carefully, as they may not generate QT key events.

    KisStrokeShortcut *runningShortcut;
    KisStrokeShortcut *readyShortcut;
    QList<KisStrokeShortcut*> candidateShortcuts;

    KisTouchShortcut *touchShortcut;
    KisNativeGestureShortcut *nativeGestureShortcut;
    std::optional<KisTouchHoldEventsPostponer> touchHoldEventPostponer;

    int maxTouchPoints{0};
    int matchingIteration{0};
    bool isTouchDragDetected {false};
    bool isTouchHeld {false};
    QScopedPointer<QEvent> bestCandidateForTapTouchEvent;
    QScopedPointer<QTouchEvent> lastProcessedTouchEvent;
    bool touchActionTracked {false};
    int touchHoldDelayMs {400};


    // A workaround for some Android systems, which force-cancel all the
    // actions with 3+ fingers. This workaround just converts the "cancel"
    // into "end" event, letting Krita handle these events as "tap" events.
    struct IgnoreMultiFingerCancelWorkaround {
        // NOTE: we cannot reuse maxTouchPoints for this workaround, because
        // maxTouchPoints is used at the lower stages of the touch processing
        // pipeline, **after** the touch events left the hold-postponer. And
        // we need this workaround to cancel the postponer itself.
        int lastRawTouchPointsCount = 0;
    };
    std::optional<IgnoreMultiFingerCancelWorkaround> ignoreMultiFingerCancelWorkaround;

    std::function<KisInputActionGroupsMask()> actionGroupMask;
    bool suppressAllActions;
    bool suppressAllKeyboardActions;
    bool cursorEntered;

    int recursiveCounter = 0;
    int brokenByRecursion = 0;


    struct RecursionNotifier {
        RecursionNotifier(KisShortcutMatcher *_q)
            : q(_q)
        {
            q->m_d->recursiveCounter++;
            q->m_d->brokenByRecursion++;
        }

        ~RecursionNotifier() {
            q->m_d->recursiveCounter--;
        }

        bool isInRecursion() const {
            return q->m_d->recursiveCounter > 1;
        }

        KisShortcutMatcher *q;
    };

    struct RecursionGuard {
        RecursionGuard(KisShortcutMatcher *_q)
            : q(_q)
        {
            q->m_d->brokenByRecursion = 0;
        }

        ~RecursionGuard() {
        }

        bool brokenByRecursion() const {
            return q->m_d->brokenByRecursion > 0;
        }

        KisShortcutMatcher *q;
    };

    inline bool actionsSuppressed() const {
#ifndef Q_OS_ANDROID
        return suppressAllActions || !cursorEntered;
#else
        // when S-pen is not pointing the canvas, actions on canvas are disabled, till it points back to canvas.
        return suppressAllActions;
#endif
    }

    inline bool actionsSuppressedIgnoreFocus() const {
        return suppressAllActions;
    }

    inline bool KeyboardActionsSuppressed() const {
        return suppressAllKeyboardActions;
    }
};

KisShortcutMatcher::KisShortcutMatcher()
    : m_d(new Private)
{}

KisShortcutMatcher::~KisShortcutMatcher()
{
    delete m_d;
}

bool KisShortcutMatcher::hasRunningShortcut() const
{
    return m_d->runningShortcut || m_d->touchShortcut || m_d->nativeGestureShortcut;
}

bool KisShortcutMatcher::hasTouchHoldShortcut() const
{
    for (const KisTouchShortcut *shortcut : m_d->touchShortcuts) {
        if (shortcut->isHoldType() && shortcut->isAvailable(m_d->actionGroupMask())) {
            return true;
        }
    }
    return false;
}

void KisShortcutMatcher::addShortcut(KisSingleActionShortcut *shortcut)
{
    m_d->singleActionShortcuts.append(shortcut);
}

void KisShortcutMatcher::addShortcut(KisStrokeShortcut *shortcut)
{
    m_d->strokeShortcuts.append(shortcut);
}

void KisShortcutMatcher::addShortcut( KisTouchShortcut* shortcut )
{
    m_d->touchShortcuts.append(shortcut);
}

void KisShortcutMatcher::addShortcut(KisNativeGestureShortcut *shortcut) {
    m_d->nativeGestureShortcuts.append(shortcut);
}

bool KisShortcutMatcher::supportsHiResInputEvents()
{
    return (m_d->runningShortcut && m_d->runningShortcut->action()
            && m_d->runningShortcut->action()->supportsHiResInputEvents(m_d->runningShortcut->shortcutIndex()))
        || (m_d->touchShortcut && m_d->touchShortcut->action()
            && m_d->touchShortcut->action()->supportsHiResInputEvents(m_d->touchShortcut->shortcutIndex()))
        || (m_d->nativeGestureShortcut && m_d->nativeGestureShortcut->action()
            && m_d->nativeGestureShortcut->action()->supportsHiResInputEvents(m_d->nativeGestureShortcut->shortcutIndex()));
}

bool KisShortcutMatcher::keyPressed(Qt::Key key)
{
    Private::RecursionNotifier notifier(this);

    bool retval = false;

    if (m_d->keys.contains(key)) { DEBUG_ACTION("Peculiar, records show key was already pressed"); }

    if (!hasRunningShortcut() && !notifier.isInRecursion()) {
        retval =  tryRunSingleActionShortcutImpl(key, (QEvent*)0, m_d->keys);
    }

    m_d->keys.insert(key);
    DEBUG_KEY("Pressed");

    if (notifier.isInRecursion()) {
        forceDeactivateAllActions();
    } else if (!hasRunningShortcut()) {
        prepareReadyShortcuts();
        tryActivateReadyShortcut();
    }

    return retval;
}

bool KisShortcutMatcher::autoRepeatedKeyPressed(Qt::Key key)
{
    Private::RecursionNotifier notifier(this);


    bool retval = false;

    if (!m_d->keys.contains(key)) { DEBUG_ACTION("Peculiar, autorepeated key but can't remember it was pressed"); }

    if (m_d->polledKeys.contains(key)) {
        m_d->polledKeys.remove(key);
    }

    if (notifier.isInRecursion()) {
        forceDeactivateAllActions();
    } else if (!hasRunningShortcut()) {
        // Autorepeated key should not be included in the shortcut
        QSet<Qt::Key> filteredKeys = m_d->keys;
        filteredKeys.remove(key);
        retval = tryRunSingleActionShortcutImpl(key, (QEvent*)0, filteredKeys);
    }

    return retval;
}

bool KisShortcutMatcher::keyReleased(Qt::Key key)
{
    Private::RecursionNotifier notifier(this);

    if (!m_d->keys.contains(key)) { DEBUG_ACTION("Peculiar, key released but can't remember it was pressed"); }
    else m_d->keys.remove(key);

    m_d->polledKeys.remove(key);

    DEBUG_KEY("Released");

    if (notifier.isInRecursion()) {
        forceDeactivateAllActions();
    } else if (!hasRunningShortcut()) {
        prepareReadyShortcuts();
        tryActivateReadyShortcut();
    }

    return false;
}

bool KisShortcutMatcher::buttonPressed(Qt::MouseButton button, QEvent *event)
{
    Private::RecursionNotifier notifier(this);
    DEBUG_BUTTON_ACTION("entered", button);

    // the tablet actions have the priority over any existing
    // touch action, so we should cancel them first
    tryCancelAllCurrentTouchActionsImpl();

    bool retval = false;

    if (m_d->buttons.contains(button)) { DEBUG_ACTION("Peculiar, button was already pressed."); }

    if (!hasRunningShortcut() && !notifier.isInRecursion()) {
        prepareReadyShortcuts();
        retval = tryRunReadyShortcut(button, event);
    }

    m_d->buttons.insert(button);

    if (notifier.isInRecursion()) {
        forceDeactivateAllActions();
    } else if (!hasRunningShortcut()) {
        prepareReadyShortcuts();
        tryActivateReadyShortcut();
    }

    return retval;
}

bool KisShortcutMatcher::buttonReleased(Qt::MouseButton button, QEvent *event)
{
    Private::RecursionNotifier notifier(this);
    DEBUG_BUTTON_ACTION("entered", button);

    bool retval = false;

    // here we check for the presence of the **stroke** shortcut only
    if (m_d->runningShortcut) {
        KIS_SAFE_ASSERT_RECOVER_NOOP(!notifier.isInRecursion());

        retval = tryEndRunningShortcut(button, event);
        DEBUG_BUTTON_ACTION("ended", button);
    }

    if (!m_d->buttons.contains(button)) reset("Peculiar, button released but we can't remember it was pressed");
    else m_d->buttons.remove(button);

    if (notifier.isInRecursion()) {
        forceDeactivateAllActions();
    } else if (!hasRunningShortcut()) {
        prepareReadyShortcuts();
        tryActivateReadyShortcut();
    }

    return retval;
}

bool KisShortcutMatcher::wheelEvent(KisSingleActionShortcut::WheelAction wheelAction, QWheelEvent *event)
{
    Private::RecursionNotifier notifier(this);


    if (hasRunningShortcut() || notifier.isInRecursion()) {
        DEBUG_ACTION("Wheel event canceled.");
        return false;
    }

    return tryRunWheelShortcut(wheelAction, event);
}

bool KisShortcutMatcher::pointerMoved(QEvent *event)
{
    Private::RecursionNotifier notifier(this);

    if (notifier.isInRecursion()) {
        return false;
    }

    bool retval = false;

    if (m_d->runningShortcut) {
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(!m_d->touchShortcut && !m_d->nativeGestureShortcut, false);
        m_d->runningShortcut->action()->inputEvent(event);
        retval = true;
    }

    return retval;
}

void KisShortcutMatcher::enterEvent()
{
    Private::RecursionNotifier notifier(this);

    m_d->cursorEntered = true;

    if (notifier.isInRecursion()) {
        forceDeactivateAllActions();
    } else if (!hasRunningShortcut()) {
        prepareReadyShortcuts();
        tryActivateReadyShortcut();
    }
}

void KisShortcutMatcher::leaveEvent()
{
    Private::RecursionNotifier notifier(this);

    m_d->cursorEntered = false;

    if (notifier.isInRecursion()) {
        forceDeactivateAllActions();
    } else if (!hasRunningShortcut()) {
        prepareReadyShortcuts();
        tryActivateReadyShortcut();
    }
}

bool KisShortcutMatcher::touchBeginEvent( QTouchEvent* event )
{
    DEBUG_TOUCH_ACTION("entered", event)

    Private::RecursionNotifier notifier(this);

    if (hasRunningShortcut()) {
        DEBUG_ACTION("touch action rejected by a running shortcut");

        // touch has been triggered while some tablet action is in progress,
        // just consume and ignore it
        return !notifier.isInRecursion();
    }

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QScopedPointer<QEvent> tmp;
    KoPointerEvent::copyQtPointerEvent(event, tmp);
    m_d->lastProcessedTouchEvent.reset(static_cast<QTouchEvent*>(tmp.take()));
#else
    m_d->lastProcessedTouchEvent.reset(event->clone());
#endif

    // reset state
    m_d->maxTouchPoints = KisTouchShortcut::countTouchPoints(event, KisTouchShortcut::pressedOnlyTouchStates());
    if (m_d->ignoreMultiFingerCancelWorkaround) {
        m_d->ignoreMultiFingerCancelWorkaround->lastRawTouchPointsCount = m_d->maxTouchPoints;
    }
    m_d->matchingIteration = 1;
    m_d->isTouchDragDetected = false;
    m_d->isTouchHeld = false;
    m_d->touchActionTracked = true;
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    KoPointerEvent::copyQtPointerEvent(event, m_d->bestCandidateForTapTouchEvent);
#else
    m_d->bestCandidateForTapTouchEvent.reset(event->clone());
#endif

    // TODO: also check if the first point actually qualifies
    if (hasTouchHoldShortcut()) {
        m_d->touchHoldEventPostponer.emplace(TOUCH_SLOP, m_d->touchHoldDelayMs);
        QObject::connect(&m_d->touchHoldEventPostponer.value(),
                         &KisTouchHoldEventsPostponer::sigHoldCompleted,
                         [this]() {
                             slotTouchHoldCompleted();
                         });
        m_d->touchHoldEventPostponer->pushThrough(event);
        if (m_d->touchHoldEventPostponer->state() == KisTouchHoldEventsPostponer::HoldCancelled) {
            m_d->touchHoldEventPostponer.reset();
        } else {
            DEBUG_ACTION("thouch-hold timer is set up");
        }
    }

    return !notifier.isInRecursion();
}

bool KisShortcutMatcher::touchUpdateEvent(QTouchEvent *event)
{
    // the touch action has been overridden by some tablet action,
    // consume and ignore it.
    if (!m_d->touchActionTracked) return false;

    if (m_d->ignoreMultiFingerCancelWorkaround) {
        m_d->ignoreMultiFingerCancelWorkaround->lastRawTouchPointsCount =
            KisTouchShortcut::countTouchPoints(event, KisTouchShortcut::pressedOnlyTouchStates());
    }

    if (m_d->touchHoldEventPostponer) {
        m_d->touchHoldEventPostponer->pushThrough(event);
        if (m_d->touchHoldEventPostponer->state() == KisTouchHoldEventsPostponer::HoldCancelled) {
            // process all the postponed **update** events,
            // the very first begin event is only used in
            // the hold-completion operation
            Q_FOREACH (QTouchEvent *event, m_d->touchHoldEventPostponer->postponedEvents()) {
                if (event->type() == QEvent::TouchUpdate) {
                    (void) touchUpdateEventImpl(event);
                }
            }
            m_d->touchHoldEventPostponer.reset();
        } else {
            // noop, we are waiting for hold
        }
    } else {
        // TODO: don't enter for hold-drag events
        return touchUpdateEventImpl(event);
    }

    return true;
}

void KisShortcutMatcher::slotTouchHoldCompleted()
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_d->touchHoldEventPostponer);
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_d->touchHoldEventPostponer->state() == KisTouchHoldEventsPostponer::HoldCompleted);

    // Can happen when multiple shortcut sources interact
    KIS_SAFE_ASSERT_RECOVER_RETURN(!m_d->runningShortcut);

    // take the very first begin event to start the action
    QTouchEvent *event = m_d->touchHoldEventPostponer->postponedEvents().front();
    KIS_SAFE_ASSERT_RECOVER_RETURN(event->type() == QEvent::TouchBegin);

    m_d->isTouchHeld = true; // Must be set first, used in tryRunTouchShortcut.

    DEBUG_TOUCH_ACTION("entered", event);

    if (tryRunTouchShortcut(event, KisTouchShortcut::pressedOnlyTouchStates(), TouchShortcutMode::Hold)) {
        // noop, all is fine!
    } else {
        KIS_SAFE_ASSERT_RECOVER(0 && "should not happen")
        {
            // Shouldn't really happen, since KisInputManager checks whether a touch
            // hold shortcut exists beforehand. We'll just handle this though.
            m_d->isTouchHeld = false;
        }
    }

    // remove the postponer to continue processing events in-place
    m_d->touchHoldEventPostponer.reset();
}

bool KisShortcutMatcher::touchUpdateEventImpl(QTouchEvent *event)
{
    if (_41020().isDebugEnabled()) {
        const int oldNumPressedPoints = KisTouchShortcut::countTouchPoints(m_d->lastProcessedTouchEvent.data(), KisTouchShortcut::pressedOnlyTouchStates());
        const int newNumPressedPoints = KisTouchShortcut::countTouchPoints(event, KisTouchShortcut::pressedOnlyTouchStates());
        if (oldNumPressedPoints != newNumPressedPoints) {
            DEBUG_TOUCH_ACTION("touch point state changed", event)
        }
    }

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QScopedPointer<QEvent> tmp;
    KoPointerEvent::copyQtPointerEvent(event, tmp);
    m_d->lastProcessedTouchEvent.reset(static_cast<QTouchEvent*>(tmp.take()));
#else
    m_d->lastProcessedTouchEvent.reset(event->clone());
#endif

    if (m_d->isTouchHeld) {
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(m_d->touchShortcut, false);
        m_d->touchShortcut->action()->inputEvent(event);
        return true;
    }

    bool retval = false;

    // Check whether the touchpoints are relatively stationary or have
    // been moved for dragging. If the drag is detected, until the next
    // TouchBegin event, we'll be assuming the gesture to be of
    // dragging type.
    if (!m_d->isTouchDragDetected) {
        if (KisTouchShortcut::touchDragDistance(event, KisTouchShortcut::allTouchStates()) > TOUCH_SLOP) {
            m_d->isTouchDragDetected = true;
        }
    }

    // Different drag shortcuts can have different drag thresholds. E.g.
    // touch-painting shortcut will have much lower threshold value. The
    // shortcuts will calculate the distance themselves in
    // KisTouchShortcut::matchDragType, so we should just ask them to do that.
    const int numStillDraggedPoints =
        KisTouchShortcut::countTouchPoints(event, KisTouchShortcut::pressedOnlyTouchStates());
    if (!hasRunningShortcut() && numStillDraggedPoints >= m_d->maxTouchPoints) {
        m_d->maxTouchPoints = numStillDraggedPoints;
        if (tryRunTouchShortcut(event, KisTouchShortcut::pressedOnlyTouchStates(), TouchShortcutMode::Drag)) {
            m_d->isTouchDragDetected = true;
            return true;
        }
    }

    // for a first few events we don't process the events right away. But analyze and keep track of the event with most
    // touchpoints. This is done to prevent conditions where in three-finger-tap, two-finger-tap be preceded due to
    // latency
    const int numIterations = 10;
    if (m_d->matchingIteration <= numIterations && !m_d->isTouchDragDetected) {
        m_d->matchingIteration++;
        setMaxTouchPointEvent(event);
        return matchTouchShortcut((QTouchEvent *)m_d->bestCandidateForTapTouchEvent.data(),
                                  KisTouchShortcut::allTouchStates(),
                                  TouchShortcutMode::Tap);
    }

    if (m_d->isTouchDragDetected) {
        if (m_d->touchShortcut
            // TODO: use a different check without threshold! split into match() and matchBegin()
            && !m_d->touchShortcut->matchDragType(event, KisTouchShortcut::pressedOnlyTouchStates())) {
            // we should end the event as an event with more touchpoints was received
            retval = tryEndTouchShortcut(event);
        } else if (m_d->touchShortcut) {
            m_d->touchShortcut->action()->inputEvent(event);
            retval = true;
        }
    } else { // !m_d->isTouchDragDetected
        // triggered if a new finger was added, which might result in shortcut not matching the action
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        const auto FlagReleased = Qt::TouchPointReleased;
#else
        const auto FlagReleased = QEventPoint::Released;
#endif
        if (event->touchPointStates().testFlag(FlagReleased) && !hasRunningShortcut()) {
            const int previousNumPoints = KisTouchShortcut::countTouchPoints(event, KisTouchShortcut::allTouchStates());

            // we should end the event as an event with more touchpoints was received
            if (previousNumPoints >= m_d->maxTouchPoints) {
                m_d->maxTouchPoints = previousNumPoints;
                if (tryFireTapTouchShortcut(event, KisTouchShortcut::allTouchStates())) {
                    m_d->bestCandidateForTapTouchEvent.reset();
                    retval = true;
                }
            }
        }
    }
    // TODO: update maxTouchPoints

    return retval;
}

bool KisShortcutMatcher::touchEndEvent(QTouchEvent *event)
{
    Private::RecursionNotifier notifier(this);

    DEBUG_TOUCH_ACTION("enter", event);

    // the touch action has been overridden by some tablet action,
    // consume and ignore it.
    if (!m_d->touchActionTracked) {
        DEBUG_ACTION("touch was not tracked");
        KIS_SAFE_ASSERT_RECOVER(!m_d->touchHoldEventPostponer.has_value()) {
            m_d->touchHoldEventPostponer->cancelHoldWait();
            m_d->touchHoldEventPostponer.reset();
        }
        return true;
    }

    // flush all the touch-hold postiponed events if they were present
    if (m_d->touchHoldEventPostponer) {
        DEBUG_ACTION("flushing all postponed hold events");

        KIS_SAFE_ASSERT_RECOVER_NOOP(m_d->touchHoldEventPostponer->state() == KisTouchHoldEventsPostponer::WaitingForHold);
        m_d->touchHoldEventPostponer->cancelHoldWait();
        // process all the postponed **update** events,
        // the very first begin event is only used in
        // the hold-completion operation
        Q_FOREACH (QTouchEvent *event, m_d->touchHoldEventPostponer->postponedEvents()) {
            if (event->type() == QEvent::TouchUpdate) {
                (void) touchUpdateEventImpl(event);
            }
        }
        m_d->touchHoldEventPostponer.reset();
    }

    m_d->touchActionTracked = false;

    bool retval = false;

    if (!m_d->isTouchDragDetected && m_d->bestCandidateForTapTouchEvent && !hasRunningShortcut()) {
        retval = tryFireTapTouchShortcut(static_cast<QTouchEvent *>(m_d->bestCandidateForTapTouchEvent.data()),
                                         KisTouchShortcut::allTouchStates());
    }

    // we should try and end the shortcut too (it might be that there is none? (sketch))
    retval |= tryEndTouchShortcut(event);

    if (notifier.isInRecursion()) {
        forceDeactivateAllActions();
    } else if (!hasRunningShortcut()) {
        prepareReadyShortcuts();
        tryActivateReadyShortcut();
    }

    return retval;
}

void KisShortcutMatcher::tryCancelAllCurrentTouchActionsImpl()
{
    // discard all the postponed touch-hold events
    if (m_d->touchHoldEventPostponer) {
        KIS_SAFE_ASSERT_RECOVER_NOOP(m_d->touchHoldEventPostponer->state() == KisTouchHoldEventsPostponer::WaitingForHold);
        m_d->touchHoldEventPostponer->cancelHoldWait();
        m_d->touchHoldEventPostponer.reset();
    }

    // end the touch action if present
    if (m_d->touchShortcut) {
        DEBUG_ACTION("force-cancel touch action")

        KisTouchShortcut *touchShortcut = m_d->touchShortcut;
        m_d->touchShortcut = 0;

        KIS_SAFE_ASSERT_RECOVER_RETURN(m_d->lastProcessedTouchEvent);

        // on some platforms touch-cancel event may have **no**
        // touch points, which would greatly confuse our KoPointerEvent
        // class, so we should just generate a normal touch-end from the
        // last known event
        QTouchEvent touchEvent = generateFakeTouchEndEvent(m_d->lastProcessedTouchEvent.data());
        touchShortcut->action()->end(&touchEvent);
        touchShortcut->action()->deactivate(touchShortcut->shortcutIndex());
    }

    m_d->touchActionTracked = false;
}

void KisShortcutMatcher::touchCancelEvent(QTouchEvent *event)
{
    Q_UNUSED(event)

    // The design requirement of touch-cancel handling is the following:
    //
    // 1) If drag-action has already been started, it is finished normally
    // 2) If tap-action is pending, it is cancelled
    // 3) If hold-action is pending, it is cancelled
    // 4) If hold-action has already been started, it is ended normally

    // On some Android devices, such as Xiaomi Pads, the system always eats
    // multitouch inputs with more than two fingers, even if the user
    // disables all gestures related to them in their system settings or
    // uses the game boost mode that is supposed to disable gestures. So we
    // handle those inputs even when they are cancelled, if the user wants
    // to use it for a system gesture, they can disable the Krita shortcut.
    if (m_d->ignoreMultiFingerCancelWorkaround) {
        // NOTE: we cannot use the number of touch points from the actual
        // cancel event, since Qt sets touch points as empty for the cancel
        // event
        if (m_d->ignoreMultiFingerCancelWorkaround->lastRawTouchPointsCount >= 3) {
            DEBUG_ACTION("IgnoreMultiFingerCancelWorkaround: converting a cancelled action into a normally finished action")
            touchEndEvent(event);
            return;
        }
    }

    Private::RecursionNotifier notifier(this);

    DEBUG_TOUCH_ACTION("enter", event);

    // the touch action has been overridden by some tablet action,
    // consume and ignore it.
    if (!m_d->touchActionTracked) {
        KIS_SAFE_ASSERT_RECOVER(!m_d->touchHoldEventPostponer.has_value()) {
            m_d->touchHoldEventPostponer->cancelHoldWait();
            m_d->touchHoldEventPostponer.reset();
        }
        return;
    }

    KIS_SAFE_ASSERT_RECOVER_NOOP(!m_d->runningShortcut || !m_d->touchShortcut);

    tryCancelAllCurrentTouchActionsImpl();

    if (notifier.isInRecursion()) {
        forceDeactivateAllActions();
    } else if (!hasRunningShortcut()) {
        prepareReadyShortcuts();
        tryActivateReadyShortcut();
    }
}

bool KisShortcutMatcher::nativeGestureBeginEvent(QNativeGestureEvent *event)
{
    Q_UNUSED(event);

    Private::RecursionNotifier notifier(this);

    DEBUG_EVENT_ACTION("entered", event);

    return !notifier.isInRecursion();
}

bool KisShortcutMatcher::nativeGestureEvent(QNativeGestureEvent *event)
{
    bool retval = false;

    DEBUG_EVENT_ACTION("entered", event);

    if (!hasRunningShortcut()) {
        retval = tryRunNativeGestureShortcut( event );
    }
    else if (m_d->nativeGestureShortcut) {
        m_d->nativeGestureShortcut->action()->inputEvent( event );
        retval = true;
    }

    return retval;
}

bool KisShortcutMatcher::nativeGestureEndEvent(QNativeGestureEvent *event)
{
    Private::RecursionNotifier notifier(this);

    DEBUG_EVENT_ACTION("entered", event);

    // TODO: why &&?
    if ( m_d->nativeGestureShortcut && !m_d->nativeGestureShortcut->match( event ) ) {
        tryEndNativeGestureShortcut( event );
    }

    if (notifier.isInRecursion()) {
        forceDeactivateAllActions();
    } else if (!hasRunningShortcut()) {
        prepareReadyShortcuts();
        tryActivateReadyShortcut();
    }

    return true;
}

Qt::MouseButtons listToFlags(const QList<Qt::MouseButton> &list) {
    Qt::MouseButtons flags;
    Q_FOREACH (Qt::MouseButton b, list) {
        flags |= b;
    }
    return flags;
}

void KisShortcutMatcher::reinitialize()
{
    Private::RecursionNotifier notifier(this);


    reset("reinitialize");

    if (notifier.isInRecursion()) {
        forceDeactivateAllActions();
    } else if (!hasRunningShortcut()) {
        prepareReadyShortcuts();
        tryActivateReadyShortcut();
    }
}

void KisShortcutMatcher::reinitializeButtons()
{
    Private::RecursionNotifier notifier(this);

    m_d->buttons.clear();
    DEBUG_ACTION("reinitializing buttons");

    if (notifier.isInRecursion()) {
        forceDeactivateAllActions();
    } else if (!hasRunningShortcut()) {
        prepareReadyShortcuts();
        tryActivateReadyShortcut();
    }
}

void KisShortcutMatcher::handlePolledKeys(const QVector<Qt::Key> &keys)
{
    Q_FOREACH (Qt::Key key, m_d->keys) {
        if (!keys.contains(key)) {
            keyReleased(key);
        }
    }

    Q_FOREACH (Qt::Key key, keys) {
        if (!m_d->keys.contains(key)) {
            keyPressed(key);
            m_d->polledKeys << key;
        }
    }

    Private::RecursionNotifier notifier(this);

    if (notifier.isInRecursion()) {
        forceDeactivateAllActions();
    } else if (!hasRunningShortcut()) {
        prepareReadyShortcuts();
        tryActivateReadyShortcut();
    }

    DEBUG_ACTION("recoverySyncModifiers");
}

bool KisShortcutMatcher::sanityCheckModifiersCorrectness(Qt::KeyboardModifiers modifiers) const
{
    auto checkKey = [this, modifiers] (Qt::Key key, Qt::KeyboardModifier modifier) {
        return m_d->keys.contains(key) == bool(modifiers & modifier);
    };

    return checkKey(Qt::Key_Shift, Qt::ShiftModifier) &&
        checkKey(Qt::Key_Control, Qt::ControlModifier) &&
        checkKey(Qt::Key_Alt, Qt::AltModifier) &&
        checkKey(Qt::Key_Meta, Qt::MetaModifier);

}

QVector<Qt::Key> KisShortcutMatcher::debugPressedKeys() const
{
    QVector<Qt::Key> keys;
    std::copy(m_d->keys.begin(), m_d->keys.end(), std::back_inserter(keys));
    return keys;
}

bool KisShortcutMatcher::hasPolledKeys()
{
    return !m_d->polledKeys.empty();
}

void KisShortcutMatcher::lostFocusEvent(const QPointF &localPos)
{
    Private::RecursionNotifier notifier(this);

    DEBUG_ACTION("lostFocusEvent");

    if (m_d->runningShortcut) {
        forceEndRunningShortcut(localPos);
    }

    forceDeactivateAllActions();

    /// TODO: it might be that we should also deactivate
    /// touch and native gestures on focus-out events.
    /// After testing on Windows it seems like it works
    /// fine without any explicit stopping the touch
    /// strokes. They just continue in the unfocused
    /// application (given that Krita does not get
    /// overlapped by another window)
}

void KisShortcutMatcher::toolHasBeenActivated()
{
    Private::RecursionNotifier notifier(this);

    DEBUG_ACTION("toolHasBeenActivated");

    if (notifier.isInRecursion()) {
        forceDeactivateAllActions();
    } else if (!hasRunningShortcut()) {
        prepareReadyShortcuts();
        tryActivateReadyShortcut();
    }
}

void KisShortcutMatcher::reset()
{
    m_d->keys.clear();
    m_d->buttons.clear();
    DEBUG_ACTION("reset!");
}


void KisShortcutMatcher::reset(QString msg)
{
    m_d->keys.clear();
    m_d->buttons.clear();
    Q_UNUSED(msg);
    DEBUG_ACTION(msg);
}

void KisShortcutMatcher::suppressAllActions(bool value)
{
    m_d->suppressAllActions = value;
}

void KisShortcutMatcher::suppressConflictingKeyActions(const QVector<QKeySequence> &shortcuts)
{
    m_d->suppressedSingleActionShortcuts.clear();

    Q_FOREACH (KisSingleActionShortcut *s, m_d->singleActionShortcuts) {
        Q_FOREACH (const QKeySequence &seq, shortcuts) {
            if (s->conflictsWith(seq)) {
                m_d->suppressedSingleActionShortcuts.insert(s);
            }
        }
    }
}

void KisShortcutMatcher::suppressAllKeyboardActions(bool value)
{
    m_d->suppressAllKeyboardActions = value;
}

void KisShortcutMatcher::clearShortcuts()
{
    reset("Clearing shortcuts");

    qDeleteAll(m_d->singleActionShortcuts);
    m_d->singleActionShortcuts.clear();
    m_d->suppressedSingleActionShortcuts.clear();

    qDeleteAll(m_d->strokeShortcuts);
    m_d->strokeShortcuts.clear();

    qDeleteAll(m_d->touchShortcuts);
    m_d->touchShortcuts.clear();

    qDeleteAll(m_d->nativeGestureShortcuts);
    m_d->nativeGestureShortcuts.clear();

    m_d->candidateShortcuts.clear();
    m_d->runningShortcut = 0;
    m_d->readyShortcut = 0;
    m_d->touchShortcut = 0;
    m_d->nativeGestureShortcut = 0;
}

void KisShortcutMatcher::setInputActionGroupsMaskCallback(std::function<KisInputActionGroupsMask ()> func)
{
    m_d->actionGroupMask = func;
}

bool KisShortcutMatcher::tryRunWheelShortcut(KisSingleActionShortcut::WheelAction wheelAction, QWheelEvent *event)
{
    return tryRunSingleActionShortcutImpl(wheelAction, event, m_d->keys, false);
}

// Note: sometimes event can be zero!!
template<typename T, typename U>
bool KisShortcutMatcher::tryRunSingleActionShortcutImpl(T param, U *event, const QSet<Qt::Key> &keysState, bool keyboard)
{
    if (m_d->actionsSuppressedIgnoreFocus() || (keyboard && m_d->KeyboardActionsSuppressed())) {
        DEBUG_EVENT_ACTION("Event suppressed", event)
        return false;
    }

    KisSingleActionShortcut *goodCandidate = 0;

    Q_FOREACH (KisSingleActionShortcut *s, m_d->singleActionShortcuts) {
        if (!m_d->suppressedSingleActionShortcuts.contains(s) &&
           s->isAvailable(m_d->actionGroupMask()) &&
           s->match(keysState, param) &&
           (!goodCandidate || s->priority() > goodCandidate->priority())) {

            goodCandidate = s;
        }
    }

    if (goodCandidate) {
        DEBUG_EVENT_ACTION("Beginning action for event", event);
        goodCandidate->action()->begin(goodCandidate->shortcutIndex(), event);
        goodCandidate->action()->end(0);
    } else {
        DEBUG_EVENT_ACTION("Could not match a candidate for event", event)
    }

    return goodCandidate;
}

void KisShortcutMatcher::prepareReadyShortcuts()
{
    m_d->candidateShortcuts.clear();
    if (m_d->actionsSuppressed()) return;

    // Allow letting the modifiers to be matched so key_shift + middle mouse move can be matched, but key_v + mouse drag can not.
    bool containsOnlyModifiers = !m_d->keys.isEmpty();
    Q_FOREACH(const Qt::Key k, m_d->keys) {
        if (k != Qt::Key_Shift && k != Qt::Key_Control && k != Qt::Key_Alt && k != Qt::Key_Meta) {
            containsOnlyModifiers = false;
            break;
        }
    }
    if (m_d->KeyboardActionsSuppressed()
            && !containsOnlyModifiers && !m_d->keys.isEmpty()
            && m_d->buttons.isEmpty()) {
        return;
    }

    Q_FOREACH (KisStrokeShortcut *s, m_d->strokeShortcuts) {
        if (s->matchReady(m_d->keys, m_d->buttons)) {
            m_d->candidateShortcuts.append(s);
        }
    }
}

bool KisShortcutMatcher::tryRunReadyShortcut( Qt::MouseButton button, QEvent* event )
{
    KisStrokeShortcut *goodCandidate = 0;

    Q_FOREACH (KisStrokeShortcut *s, m_d->candidateShortcuts) {
        if (s->isAvailable(m_d->actionGroupMask()) &&
            s->matchBegin(button) &&
            (!goodCandidate || s->priority() > goodCandidate->priority())) {

            goodCandidate = s;
        }
    }

    if (goodCandidate) {
        if (m_d->readyShortcut) {
            if (m_d->readyShortcut != goodCandidate) {
                m_d->readyShortcut->action()->deactivate(m_d->readyShortcut->shortcutIndex());
                goodCandidate->action()->activate(goodCandidate->shortcutIndex());
            }
            m_d->readyShortcut = 0;
        } else {
            DEBUG_EVENT_ACTION("Matched *new* shortcut for event", event);
            goodCandidate->action()->activate(goodCandidate->shortcutIndex());
        }

        DEBUG_SHORTCUT("Starting new action", goodCandidate);

        {
            m_d->runningShortcut = goodCandidate;
            Private::RecursionGuard guard(this);
            goodCandidate->action()->begin(goodCandidate->shortcutIndex(), event);

            // the tool might have opened some dialog, which could break our event loop
            if (guard.brokenByRecursion()) {
                goodCandidate->action()->end(event);
                m_d->runningShortcut = 0;

                forceDeactivateAllActions();
            }
        }
    }

    return m_d->runningShortcut;
}

void KisShortcutMatcher::tryActivateReadyShortcut()
{
    KisStrokeShortcut *goodCandidate = 0;

    Q_FOREACH (KisStrokeShortcut *s, m_d->candidateShortcuts) {
        if (!goodCandidate || s->priority() > goodCandidate->priority()) {
            goodCandidate = s;
        }
    }

    if (goodCandidate) {
        if (m_d->readyShortcut && m_d->readyShortcut != goodCandidate) {
            DEBUG_SHORTCUT("Deactivated previous shortcut action", m_d->readyShortcut);
            m_d->readyShortcut->action()->deactivate(m_d->readyShortcut->shortcutIndex());
            m_d->readyShortcut = 0;
        }

        if (!m_d->readyShortcut) {
            DEBUG_SHORTCUT("Preparing new ready action", goodCandidate);

            /**
             * It is important that we first activate the action, and only after
             * that assign it to m_d->readyShortcut. It makes is possible to activate
             * another tool in KisToolInvocationAction and survive the call to
             * forceDeactivateAllActions() from lostFocusEvent(), which would
             * enter infinite loop otherwise.
             */
            goodCandidate->action()->activate(goodCandidate->shortcutIndex());
            m_d->readyShortcut = goodCandidate;
        }
    } else if (m_d->readyShortcut) {
        DEBUG_SHORTCUT("Deactivating action", m_d->readyShortcut);
        m_d->readyShortcut->action()->deactivate(m_d->readyShortcut->shortcutIndex());
        m_d->readyShortcut = 0;
    }
}

bool KisShortcutMatcher::tryEndRunningShortcut( Qt::MouseButton button, QEvent* event )
{
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(m_d->runningShortcut, true);
    KIS_SAFE_ASSERT_RECOVER(!m_d->readyShortcut) {
        // it shouldn't have happened, running and ready shortcuts
        // at the same time should not be possible
        forceDeactivateAllActions();
    }

    if (m_d->runningShortcut && m_d->runningShortcut->matchBegin(button)) {

        // first reset running shortcut to avoid infinite recursion via end()
        KisStrokeShortcut *runningShortcut = m_d->runningShortcut;
        m_d->runningShortcut = 0;

        if (runningShortcut->action()) {
            DEBUG_EVENT_ACTION("Ending running shortcut at event", event);
            KisAbstractInputAction* action = runningShortcut->action();
            int shortcutIndex = runningShortcut->shortcutIndex();
            action->end(event);
            action->deactivate(shortcutIndex);
        }
    }

    return !m_d->runningShortcut;
}

void KisShortcutMatcher::forceEndRunningShortcut(const QPointF &localPos)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_d->runningShortcut);
    KIS_SAFE_ASSERT_RECOVER(!m_d->readyShortcut) {
        // it shouldn't have happened, running and ready shortcuts
        // at the same time should not be possible
        forceDeactivateAllActions();
    }

    // first reset running shortcut to avoid infinite recursion via end()
    KisStrokeShortcut *runningShortcut = m_d->runningShortcut;
    m_d->runningShortcut = 0;

    if (runningShortcut->action()) {
        DEBUG_ACTION("Forced ending running shortcut at event");
        KisAbstractInputAction* action = runningShortcut->action();
        int shortcutIndex = runningShortcut->shortcutIndex();

        QMouseEvent event = runningShortcut->fakeEndEvent(localPos);

        action->end(&event);
        action->deactivate(shortcutIndex);
    }
}

void KisShortcutMatcher::forceDeactivateAllActions()
{
    if (m_d->readyShortcut) {
        DEBUG_SHORTCUT("Forcefully deactivating action", m_d->readyShortcut);
        m_d->readyShortcut->action()->deactivate(m_d->readyShortcut->shortcutIndex());
        m_d->readyShortcut = 0;
    }
}

void KisShortcutMatcher::setMaxTouchPointEvent(QTouchEvent *event)
{
    int previousNumTouchPoints = -1;

    if (m_d->bestCandidateForTapTouchEvent) {
        QTouchEvent *bestTouchEvent = static_cast<QTouchEvent*>(m_d->bestCandidateForTapTouchEvent.data());
        previousNumTouchPoints = KisTouchShortcut::countTouchPoints(bestTouchEvent, KisTouchShortcut::pressedOnlyTouchStates());
    }

    const int newNumTouchPoints = KisTouchShortcut::countTouchPoints(event, KisTouchShortcut::pressedOnlyTouchStates());;

    if (newNumTouchPoints >= previousNumTouchPoints) {
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        KoPointerEvent::copyQtPointerEvent(event, m_d->bestCandidateForTapTouchEvent);
#else
        m_d->bestCandidateForTapTouchEvent.reset(event->clone());
#endif
    }
}

bool KisShortcutMatcher::tryFireTapTouchShortcut(QTouchEvent *event, Qt::TouchPointStates allowedStates)
{
    /**
     * Touch actions don't usually send focus events, since they have no
     * hover functionality, so they should be triggered unrelated to the
     * current focus state.
     */
    if (m_d->actionsSuppressedIgnoreFocus())
        return false;

    KisTouchShortcut *goodCandidate = matchTouchShortcut(event, allowedStates, TouchShortcutMode::Tap);
    if (goodCandidate) {
        DEBUG_SHORTCUT("Starting new touch-tap action", goodCandidate);
        goodCandidate->action()->activate(goodCandidate->shortcutIndex());
        goodCandidate->action()->begin(goodCandidate->shortcutIndex(), event);

        goodCandidate->action()->end(event);
        goodCandidate->action()->deactivate(goodCandidate->shortcutIndex());
    }

    return goodCandidate;
}

KisTouchShortcut *KisShortcutMatcher::matchTouchShortcut(QTouchEvent *event, Qt::TouchPointStates allowedStates, TouchShortcutMode mode)
{
    KisTouchShortcut *goodCandidate = nullptr;
    Q_FOREACH (KisTouchShortcut *shortcut, m_d->touchShortcuts) {
        if (shortcut->isAvailable(m_d->actionGroupMask())
            && matchTouchShortcutBasedOnState(event, shortcut, allowedStates, mode)
            && (!goodCandidate || shortcut->priority() > goodCandidate->priority())) {

            goodCandidate = shortcut;
        }
    }
    return goodCandidate;
}

bool KisShortcutMatcher::matchTouchShortcutBasedOnState(QTouchEvent *event, KisTouchShortcut *shortcut, Qt::TouchPointStates allowedStates, TouchShortcutMode mode)
{
    switch (mode) {
    case TouchShortcutMode::Hold:
        return shortcut->matchHoldType(event, allowedStates);
        break;
    case TouchShortcutMode::Drag:
        return shortcut->matchDragType(event, allowedStates);
        break;
    case TouchShortcutMode::Tap:
        return shortcut->matchTapType(event, allowedStates);
        break;
    }

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    return false;
#else
    Q_UNREACHABLE_RETURN(false);
#endif
}

bool KisShortcutMatcher::tryRunTouchShortcut(QTouchEvent* event, Qt::TouchPointStates allowedStates, TouchShortcutMode mode)
{
    KisTouchShortcut *goodCandidate = matchTouchShortcut(event, allowedStates, mode);

    /**
     * Touch actions don't usually send focus events, since they have no
     * hover functionality, so they should be triggered unrelated to the
     * current focus state.
     */
    if (m_d->actionsSuppressedIgnoreFocus())
        return false;

    if (goodCandidate) {
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(!m_d->runningShortcut, false);

        // Because we don't match keyboard or button based actions with touch system, we have to ensure that we first
        // deactivate an activated readyShortcut, to not throw other statemachines out of place.
        forceDeactivateAllActions();

        m_d->touchShortcut = goodCandidate;

        Private::RecursionGuard guard(this);
        DEBUG_SHORTCUT("Running a new touch action", goodCandidate)

        goodCandidate->action()->activate(goodCandidate->shortcutIndex());
        goodCandidate->action()->begin(goodCandidate->shortcutIndex(), event);

        // the tool might have opened some dialog, which could break our event loop
        if (guard.brokenByRecursion()) {
            goodCandidate->action()->end(event);
            m_d->touchShortcut = 0;

            forceDeactivateAllActions();
        }
    }

    return m_d->touchShortcut;
}

bool KisShortcutMatcher::tryEndTouchShortcut( QTouchEvent* event )
{
    if (m_d->touchShortcut) {
        // first reset running shortcut to avoid infinite recursion via end()
        KisTouchShortcut *touchShortcut = m_d->touchShortcut;

        DEBUG_SHORTCUT("ending", touchShortcut)
        touchShortcut->action()->end(event);
        touchShortcut->action()->deactivate(m_d->touchShortcut->shortcutIndex());

        m_d->touchShortcut = 0; // empty it out now that we are done with it

        return true;
    }

    return false;
}

bool KisShortcutMatcher::tryRunNativeGestureShortcut(QNativeGestureEvent* event)
{
    KisNativeGestureShortcut *goodCandidate = 0;

    if (m_d->actionsSuppressed())
        return false;

    Q_FOREACH (KisNativeGestureShortcut* shortcut, m_d->nativeGestureShortcuts) {
        if (shortcut->match(event) && (!goodCandidate || shortcut->priority() > goodCandidate->priority())) {
            goodCandidate = shortcut;
        }
    }

    if (goodCandidate) {
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(!m_d->runningShortcut, false);

        // Because we don't match keyboard or button based actions with touch system, we have to ensure that we first
        // deactivate an activated readyShortcut, to not throw other statemachines out of place.
        forceDeactivateAllActions();

        m_d->nativeGestureShortcut = goodCandidate;

        Private::RecursionGuard guard(this);

        DEBUG_SHORTCUT("starting", goodCandidate)

        goodCandidate->action()->activate(goodCandidate->shortcutIndex());
        goodCandidate->action()->begin(goodCandidate->shortcutIndex(), event);

        // the tool might have opened some dialog, which could break our event loop
        if (guard.brokenByRecursion()) {
            goodCandidate->action()->end(event);
            m_d->nativeGestureShortcut = 0;

            forceDeactivateAllActions();
        }
    }

    return m_d->nativeGestureShortcut;
}

bool KisShortcutMatcher::tryEndNativeGestureShortcut(QNativeGestureEvent* event)
{
    Private::RecursionNotifier notifier(this);

    if (m_d->nativeGestureShortcut) {
        // first reset running shortcut to avoid infinite recursion via end()
        KisNativeGestureShortcut *nativeGestureShortcut = m_d->nativeGestureShortcut;

        DEBUG_SHORTCUT("ending", nativeGestureShortcut)

        nativeGestureShortcut->action()->end(event);
        nativeGestureShortcut->action()->deactivate(m_d->nativeGestureShortcut->shortcutIndex());

        m_d->nativeGestureShortcut = 0; // empty it out now that we are done with it

        return true;
    }

    if (notifier.isInRecursion()) {
        forceDeactivateAllActions();
    } else if (!hasRunningShortcut()) {
        prepareReadyShortcuts();
        tryActivateReadyShortcut();
    }

    return false;
}

int KisShortcutMatcher::touchHoldDelay() const
{
    return m_d->touchHoldDelayMs;
}

void KisShortcutMatcher::setTouchHoldDelay(int value)
{
    m_d->touchHoldDelayMs = value;
}

void KisShortcutMatcher::setIgnoreMultiFingerCancelWorkaroundEnalbed(bool value)
{
    if (value) {
        m_d->ignoreMultiFingerCancelWorkaround = Private::IgnoreMultiFingerCancelWorkaround();
    } else {
        m_d->ignoreMultiFingerCancelWorkaround = std::nullopt;
    }
}

bool KisShortcutMatcher::ignoreMultiFingerCancelWorkaroundEnalbed() const
{
    return m_d->ignoreMultiFingerCancelWorkaround.has_value();
}