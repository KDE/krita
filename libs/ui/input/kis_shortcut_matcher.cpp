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
#include <KoPointerEvent.h>

//#define DEBUG_MATCHER

#ifdef DEBUG_MATCHER
#include <kis_debug.h>
#define DEBUG_ACTION(text) qDebug() << __FUNCTION__ << "-" << text;
#define DEBUG_SHORTCUT(text, shortcut) qDebug() << __FUNCTION__ << "-" << text << "act:" << shortcut->action()->name();
#define DEBUG_KEY(text) qDebug() << __FUNCTION__ << "-" << text << "keys:" << m_d->keys;
#define DEBUG_BUTTON_ACTION(text, button) qDebug() << __FUNCTION__ << "-" << text << "button:" << button << "btns:" << m_d->buttons << "keys:" << m_d->keys;
#define DEBUG_EVENT_ACTION(text, event) if (event) {qDebug() << __FUNCTION__ << "-" << text << "type:" << event->type();}
#define DEBUG_TOUCH_ACTION(text, event)                                                                                \
    if (event) {                                                                                                       \
        qDebug() << __FUNCTION__ << "-" << text << "type:" << event->type() << "tps:" << event->touchPoints().size()   \
                 << "maxTps:" << m_d->maxTouchPoints << "drag:" << m_d->isTouchDragDetected;                              \
    }
#else
#define DEBUG_ACTION(text)
#define DEBUG_KEY(text)
#define DEBUG_SHORTCUT(text, shortcut)
#define DEBUG_BUTTON_ACTION(text, button)
#define DEBUG_EVENT_ACTION(text, event)
#define DEBUG_TOUCH_ACTION(text, event)
#endif


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
    QList<QTouchEvent::TouchPoint> lastTouchPoints;

    int maxTouchPoints{0};
    int matchingIteration{0};
    bool isTouchDragDetected {false};
    bool isTouchHeld {false};
    QScopedPointer<QEvent> bestCandidateTouchEvent;

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
        return false;
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

    m_d->lastTouchPoints = event->touchPoints();

    // reset state
    m_d->maxTouchPoints = event->touchPoints().size();
    m_d->matchingIteration = 1;
    m_d->isTouchDragDetected = false;
    m_d->isTouchHeld = false;
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    KoPointerEvent::copyQtPointerEvent(event, m_d->bestCandidateTouchEvent);
#else
    m_d->bestCandidateTouchEvent.reset(event->clone());
#endif

    return !notifier.isInRecursion();
}

bool KisShortcutMatcher::touchUpdateEvent(QTouchEvent *event)
{
    DEBUG_TOUCH_ACTION("entered", event)

    if (m_d->isTouchHeld) {
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(m_d->touchShortcut, false);
        m_d->touchShortcut->action()->inputEvent(event);
        return true;
    }

    bool retval = false;

    const int touchPointCount = event->touchPoints().size();
    // check whether the touchpoints are relatively stationary or have been moved for dragging.
    for (int i = 0; i < event->touchPoints().size() && !m_d->isTouchDragDetected; ++i) {
        const QTouchEvent::TouchPoint &touchPoint = event->touchPoints().at(i);
        const QPointF delta = touchPoint.pos() - touchPoint.startPos();
        const qreal deltaSquared = delta.x() * delta.x() + delta.y() * delta.y();
        // if the drag is detected, until the next TouchBegin even, we'll be assuming the gesture to be of dragging
        // type.
        m_d->isTouchDragDetected = deltaSquared > TOUCH_SLOP_SQUARED;
    }

    // for a first few events we don't process the events right away. But analyze and keep track of the event with most
    // touchpoints. This is done to prevent conditions where in three-finger-tap, two-finger-tap be preceded due to
    // latency
    const int numIterations = 10;
    if (m_d->matchingIteration <= numIterations && !m_d->isTouchDragDetected) {
        m_d->matchingIteration++;
        setMaxTouchPointEvent(event);
        DEBUG_TOUCH_ACTION("return best", event)
        return matchTouchShortcut((QTouchEvent *)m_d->bestCandidateTouchEvent.data());
    }

    if (m_d->isTouchDragDetected) {
        if (m_d->touchShortcut && !m_d->touchShortcut->matchDragType(event)) {
            DEBUG_TOUCH_ACTION("ending", event)
            // we should end the event as an event with more touchpoints was received
            retval = tryEndTouchShortcut(event);
        }
        if (!hasRunningShortcut() && touchPointCount >= m_d->maxTouchPoints) {
            m_d->maxTouchPoints = touchPointCount;
            DEBUG_TOUCH_ACTION("starting", event);
            retval = tryRunTouchShortcut(event);
        } else if (m_d->touchShortcut) {
            // The typical assumption when we get here is that the shortcut has been matched, for which we use
            // the events with TouchPointPressed state. But there may be instances where shortcut is never
            // un-matched (meaning: never being tryEndTouchShortcut called on it) even when the finger is
            // released, and when the next contact is made, the shortcut proceeds assuming continuity -- which
            // is a false assumption.
            // So, if we see a TouchPointPressed, we should know that somewhere previously finger was lifted
            // and we should let the action know this.
            if (event->touchPointStates() & Qt::TouchPointPressed) {
                m_d->touchShortcut->action()->begin(m_d->touchShortcut->shortcutIndex(), event);
            } else if (event->touchPointStates() & Qt::TouchPointReleased) {
                m_d->touchShortcut->action()->end(event);
            } else {
                m_d->touchShortcut->action()->inputEvent(event);
            }
            retval = true;
        }
    } else {
        // triggered if a new finger was added, which might result in shortcut not matching the action
        if ((event->touchPointStates() & Qt::TouchPointReleased) == Qt::TouchPointReleased && !hasRunningShortcut()) {
            // we should end the event as an event with more touchpoints was received
            if (m_d->maxTouchPoints <= touchPointCount) {
                m_d->maxTouchPoints = touchPointCount;
                DEBUG_TOUCH_ACTION("firing", event);
                fireReadyTouchShortcut(event);
                m_d->bestCandidateTouchEvent.reset();
            }
        }
    }

    return retval;
}

bool KisShortcutMatcher::touchEndEvent(QTouchEvent *event)
{
    Private::RecursionNotifier notifier(this);

    m_d->maxTouchPoints = 0;
    m_d->isTouchHeld = false;

    if (!m_d->isTouchDragDetected && m_d->bestCandidateTouchEvent && !hasRunningShortcut()) {
        fireReadyTouchShortcut(static_cast<QTouchEvent *>(m_d->bestCandidateTouchEvent.data()));
    }

    DEBUG_TOUCH_ACTION("ending", event)
    // we should try and end the shortcut too (it might be that there is none? (sketch))
    const bool retval = tryEndTouchShortcut(event);

    if (notifier.isInRecursion()) {
        forceDeactivateAllActions();
    } else if (!hasRunningShortcut()) {
        prepareReadyShortcuts();
        tryActivateReadyShortcut();
    }

    return retval;
}

void KisShortcutMatcher::touchCancelEvent(QTouchEvent *event, const QPointF &localPos)
{
    Private::RecursionNotifier notifier(this);

    m_d->maxTouchPoints = 0;
    m_d->isTouchHeld = false;

    KIS_SAFE_ASSERT_RECOVER_NOOP(!m_d->runningShortcut || !m_d->touchShortcut);

    // end the stroke types
    if (m_d->touchShortcut) {
        KisTouchShortcut *touchShortcut = m_d->touchShortcut;
        m_d->touchShortcut = 0;
        QScopedPointer<QEvent> dstEvent;
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        KoPointerEvent::copyQtPointerEvent(event, dstEvent);
#else
        dstEvent.reset(event->clone());
#endif

        // HACK: Because TouchEvents in KoPointerEvent need to contain at least one touchpoint
        QTouchEvent* touchEvent = dynamic_cast<QTouchEvent *>(dstEvent.data());
        KIS_ASSERT(touchEvent);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        touchEvent->setTouchPoints(m_d->lastTouchPoints);
#else
        dstEvent.reset(new QTouchEvent(event->type(),
                                       event->pointingDevice(),
                                       event->modifiers(),
                                       m_d->lastTouchPoints));
#endif
        touchShortcut->action()->end(dstEvent.data());
        touchShortcut->action()->deactivate(touchShortcut->shortcutIndex());
    }

    if (notifier.isInRecursion()) {
        forceDeactivateAllActions();
    } else if (!hasRunningShortcut()) {
        prepareReadyShortcuts();
        tryActivateReadyShortcut();
    }
}

bool KisShortcutMatcher::touchHoldBeginEvent(QTouchEvent *event)
{
    DEBUG_TOUCH_ACTION("hold", event)

    // Can happen when multiple shortcut sources interact.
    if (m_d->runningShortcut) {
        return false;
    }

    m_d->isTouchHeld = true; // Must be set first, used in tryRunTouchShortcut.
    if (tryRunTouchShortcut(event)) {
        DEBUG_ACTION("touch shortcut found");
        return true;
    } else {
        // Shouldn't really happen, since KisInputManager checks whether a touch
        // hold shortcut exists beforehand. We'll just handle this though.
        DEBUG_ACTION("touch shortcut not found");
        m_d->isTouchHeld = false;
        return touchBeginEvent(event);
    }
}

bool KisShortcutMatcher::nativeGestureBeginEvent(QNativeGestureEvent *event)
{
    Q_UNUSED(event);

    Private::RecursionNotifier notifier(this);

    return !notifier.isInRecursion();
}

bool KisShortcutMatcher::nativeGestureEvent(QNativeGestureEvent *event)
{
    bool retval = false;
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
    qDeleteAll(m_d->strokeShortcuts);
    qDeleteAll(m_d->touchShortcuts);
    m_d->strokeShortcuts.clear();
    m_d->candidateShortcuts.clear();
    m_d->touchShortcuts.clear();
    m_d->runningShortcut = 0;
    m_d->readyShortcut = 0;
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
    int touchPointCount = event->touchPoints().size();
    if (touchPointCount > m_d->maxTouchPoints) {
        m_d->maxTouchPoints = touchPointCount;
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        KoPointerEvent::copyQtPointerEvent(event, m_d->bestCandidateTouchEvent);
#else
        m_d->bestCandidateTouchEvent.reset(event->clone());
#endif

    }
}

void KisShortcutMatcher::fireReadyTouchShortcut(QTouchEvent *event)
{
    KisTouchShortcut *goodCandidate = matchTouchShortcut(event);
    if (goodCandidate) {
        DEBUG_TOUCH_ACTION("starting", event)
        goodCandidate->action()->activate(goodCandidate->shortcutIndex());
        goodCandidate->action()->begin(goodCandidate->shortcutIndex(), event);

        goodCandidate->action()->end(event);
        goodCandidate->action()->deactivate(goodCandidate->shortcutIndex());
    }
}

KisTouchShortcut *KisShortcutMatcher::matchTouchShortcut(QTouchEvent *event)
{
    KisTouchShortcut *goodCandidate = nullptr;
    Q_FOREACH (KisTouchShortcut *shortcut, m_d->touchShortcuts) {
        if (shortcut->isAvailable(m_d->actionGroupMask())
            && matchTouchShortcutBasedOnState(event, shortcut)
            && (!goodCandidate || shortcut->priority() > goodCandidate->priority())) {

            goodCandidate = shortcut;
        }
    }
    return goodCandidate;
}

bool KisShortcutMatcher::matchTouchShortcutBasedOnState(QTouchEvent *event, KisTouchShortcut *shortcut)
{
    if (m_d->isTouchHeld) {
        return shortcut->matchHoldType(event);
    } else if (m_d->isTouchDragDetected) {
        return shortcut->matchDragType(event);
    } else {
        return shortcut->matchTapType(event);
    }
}

bool KisShortcutMatcher::tryRunTouchShortcut( QTouchEvent* event )
{
    KisTouchShortcut *goodCandidate = matchTouchShortcut(event);

    if (m_d->actionsSuppressed())
        return false;

    if (goodCandidate) {
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(!m_d->runningShortcut, false);

        // Because we don't match keyboard or button based actions with touch system, we have to ensure that we first
        // deactivate an activated readyShortcut, to not throw other statemachines out of place.
        forceDeactivateAllActions();

        m_d->touchShortcut = goodCandidate;

        Private::RecursionGuard guard(this);
        DEBUG_SHORTCUT("Running a touch shortcut", goodCandidate)

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
