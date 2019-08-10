/*
 *  Copyright (c) 2012 Dmitry Kazakov <dimula73@gmail.com>
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

//#define DEBUG_MATCHER

#ifdef DEBUG_MATCHER
#include <kis_debug.h>
#define DEBUG_ACTION(text) qDebug() << __FUNCTION__ << "-" << text;
#define DEBUG_SHORTCUT(text, shortcut) qDebug() << __FUNCTION__ << "-" << text << "act:" << shortcut->action()->name();
#define DEBUG_KEY(text) qDebug() << __FUNCTION__ << "-" << text << "keys:" << m_d->keys;
#define DEBUG_BUTTON_ACTION(text, button) qDebug() << __FUNCTION__ << "-" << text << "button:" << button << "btns:" << m_d->buttons << "keys:" << m_d->keys;
#define DEBUG_EVENT_ACTION(text, event) if (event) {qDebug() << __FUNCTION__ << "-" << text << "type:" << event->type();}
#else
#define DEBUG_ACTION(text)
#define DEBUG_KEY(text)
#define DEBUG_SHORTCUT(text, shortcut)
#define DEBUG_BUTTON_ACTION(text, button)
#define DEBUG_EVENT_ACTION(text, event)
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
        , cursorEntered(false)
        , usingTouch(false)
        , usingNativeGesture(false)
    {}

    ~Private()
    {
        qDeleteAll(singleActionShortcuts);
        qDeleteAll(strokeShortcuts);
        qDeleteAll(touchShortcuts);
    }

    QList<KisSingleActionShortcut*> singleActionShortcuts;
    QList<KisStrokeShortcut*> strokeShortcuts;
    QList<KisTouchShortcut*> touchShortcuts;
    QList<KisNativeGestureShortcut*> nativeGestureShortcuts;

    QSet<Qt::Key> keys; // Model of currently pressed keys
    QSet<Qt::MouseButton> buttons; // Model of currently pressed buttons

    KisStrokeShortcut *runningShortcut;
    KisStrokeShortcut *readyShortcut;
    QList<KisStrokeShortcut*> candidateShortcuts;

    KisTouchShortcut *touchShortcut;
    KisNativeGestureShortcut *nativeGestureShortcut;

    std::function<KisInputActionGroupsMask()> actionGroupMask;
    bool suppressAllActions;
    bool cursorEntered;
    bool usingTouch;
    bool usingNativeGesture;

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
        return (suppressAllActions || !cursorEntered)
                && KisConfig(true).disableTouchOnCanvas();
    }

    inline bool actionsSuppressedIgnoreFocus() const {
        return suppressAllActions;
    }

    // only for touch events with touchPoints count >= 2
    inline bool isUsingTouch() const {
        return usingTouch || usingNativeGesture;
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
    return m_d->runningShortcut;
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
    return
        m_d->runningShortcut &&
        m_d->runningShortcut->action() &&
        m_d->runningShortcut->action()->supportsHiResInputEvents();
}

bool KisShortcutMatcher::keyPressed(Qt::Key key)
{
    Private::RecursionNotifier notifier(this);

    bool retval = false;

    if (m_d->keys.contains(key)) { DEBUG_ACTION("Peculiar, records show key was already pressed"); }

    if (!m_d->runningShortcut && !notifier.isInRecursion()) {
        retval =  tryRunSingleActionShortcutImpl(key, (QEvent*)0, m_d->keys);
    }

    m_d->keys.insert(key);
    DEBUG_KEY("Pressed");

    if (notifier.isInRecursion()) {
        forceDeactivateAllActions();
    } else if (!m_d->runningShortcut) {
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

    if (notifier.isInRecursion()) {
        forceDeactivateAllActions();
    } else if (!m_d->runningShortcut) {
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

    if (!m_d->keys.contains(key)) reset("Peculiar, key released but can't remember it was pressed");
    else m_d->keys.remove(key);

    if (notifier.isInRecursion()) {
        forceDeactivateAllActions();
    } else if (!m_d->runningShortcut) {
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

    if (m_d->isUsingTouch()) {
        return retval;
    }

    if (m_d->buttons.contains(button)) { DEBUG_ACTION("Peculiar, button was already pressed."); }

    if (!m_d->runningShortcut && !notifier.isInRecursion()) {
        prepareReadyShortcuts();
        retval = tryRunReadyShortcut(button, event);
    }

    m_d->buttons.insert(button);

    if (notifier.isInRecursion()) {
        forceDeactivateAllActions();
    } else if (!m_d->runningShortcut) {
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

    if (m_d->isUsingTouch()) {
        return retval;
    }

    if (m_d->runningShortcut) {
        KIS_SAFE_ASSERT_RECOVER_NOOP(!notifier.isInRecursion());

        retval = tryEndRunningShortcut(button, event);
        DEBUG_BUTTON_ACTION("ended", button);
    }

    if (!m_d->buttons.contains(button)) reset("Peculiar, button released but we can't remember it was pressed");
    else m_d->buttons.remove(button);

    if (notifier.isInRecursion()) {
        forceDeactivateAllActions();
    } else if (!m_d->runningShortcut) {
        prepareReadyShortcuts();
        tryActivateReadyShortcut();
    }

    return retval;
}

bool KisShortcutMatcher::wheelEvent(KisSingleActionShortcut::WheelAction wheelAction, QWheelEvent *event)
{
    Private::RecursionNotifier notifier(this);


    if (m_d->runningShortcut || m_d->isUsingTouch() || notifier.isInRecursion()) {
        DEBUG_ACTION("Wheel event canceled.");
        return false;
    }

    return tryRunWheelShortcut(wheelAction, event);
}

bool KisShortcutMatcher::pointerMoved(QEvent *event)
{
    Private::RecursionNotifier notifier(this);


    if (m_d->isUsingTouch() || !m_d->runningShortcut || notifier.isInRecursion()) {
        return false;
    }

    m_d->runningShortcut->action()->inputEvent(event);
    return true;
}

void KisShortcutMatcher::enterEvent()
{
    Private::RecursionNotifier notifier(this);

    m_d->cursorEntered = true;

    if (notifier.isInRecursion()) {
        forceDeactivateAllActions();
    } else if (!m_d->runningShortcut) {
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
    } else if (!m_d->runningShortcut) {
        prepareReadyShortcuts();
        tryActivateReadyShortcut();
    }
}

bool KisShortcutMatcher::touchBeginEvent( QTouchEvent* event )
{
    Q_UNUSED(event)

    Private::RecursionNotifier notifier(this);

    return !notifier.isInRecursion();
}

bool KisShortcutMatcher::touchUpdateEvent( QTouchEvent* event )
{
    bool retval = false;

    if (m_d->touchShortcut && !m_d->touchShortcut->match( event ) ) {
        retval = tryEndTouchShortcut( event );
    }

    if (!m_d->touchShortcut ) {
        retval = tryRunTouchShortcut( event );
    }
    else {
        m_d->touchShortcut->action()->inputEvent( event );
        retval = true;
    }

    return retval;
}

bool KisShortcutMatcher::touchEndEvent( QTouchEvent* event )
{
    m_d->usingTouch = false; // we need to say we are done because qt will not send further event

    // we should try and end the shortcut too (it might be that there is none? (sketch))
    if (tryEndTouchShortcut(event)) {
        return true;
    }

    return false;
}

bool KisShortcutMatcher::nativeGestureBeginEvent(QNativeGestureEvent *event)
{
    Q_UNUSED(event)

    Private::RecursionNotifier notifier(this);

    return !notifier.isInRecursion();
}

bool KisShortcutMatcher::nativeGestureEvent(QNativeGestureEvent *event)
{
    bool retval = false;
    if ( !m_d->nativeGestureShortcut ) {
        retval = tryRunNativeGestureShortcut( event );
    }
    else {
        m_d->nativeGestureShortcut->action()->inputEvent( event );
        retval = true;
    }

    return retval;
}

bool KisShortcutMatcher::nativeGestureEndEvent(QNativeGestureEvent *event)
{
    if ( m_d->nativeGestureShortcut && !m_d->nativeGestureShortcut->match( event ) ) {
        tryEndNativeGestureShortcut( event );
    }
    m_d->usingNativeGesture = false;
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
    } else if (!m_d->runningShortcut) {
        prepareReadyShortcuts();
        tryActivateReadyShortcut();
    }
}

void KisShortcutMatcher::recoveryModifiersWithoutFocus(const QVector<Qt::Key> &keys)
{
    Private::RecursionNotifier notifier(this);


    Q_FOREACH (Qt::Key key, m_d->keys) {
        if (!keys.contains(key)) {
            keyReleased(key);
        }
    }

    Q_FOREACH (Qt::Key key, keys) {
        if (!m_d->keys.contains(key)) {
            keyPressed(key);
        }
    }

    if (notifier.isInRecursion()) {
        forceDeactivateAllActions();
    } else if (!m_d->runningShortcut) {
        prepareReadyShortcuts();
        tryActivateReadyShortcut();
    }

    DEBUG_ACTION("recoverySyncModifiers");
}

void KisShortcutMatcher::lostFocusEvent(const QPointF &localPos)
{
    if (m_d->runningShortcut) {
        forceEndRunningShortcut(localPos);
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
    return tryRunSingleActionShortcutImpl(wheelAction, event, m_d->keys);
}

// Note: sometimes event can be zero!!
template<typename T, typename U>
bool KisShortcutMatcher::tryRunSingleActionShortcutImpl(T param, U *event, const QSet<Qt::Key> &keysState)
{
    if (m_d->actionsSuppressedIgnoreFocus()) {
        DEBUG_EVENT_ACTION("Event suppressed", event)
        return false;
    }

    KisSingleActionShortcut *goodCandidate = 0;

    Q_FOREACH (KisSingleActionShortcut *s, m_d->singleActionShortcuts) {
        if(s->isAvailable(m_d->actionGroupMask()) &&
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

            // the tool migh have opened some dialog, which could break our event loop
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

    if (m_d->runningShortcut->matchBegin(button)) {

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

bool KisShortcutMatcher::tryRunTouchShortcut( QTouchEvent* event )
{
    KisTouchShortcut *goodCandidate = 0;

    if (m_d->actionsSuppressed())
        return false;

    Q_FOREACH (KisTouchShortcut* shortcut, m_d->touchShortcuts) {
        if (shortcut->isAvailable(m_d->actionGroupMask()) &&
            shortcut->match( event ) &&
            (!goodCandidate || shortcut->priority() > goodCandidate->priority()) ) {

            goodCandidate = shortcut;
        }
    }

    if( goodCandidate ) {
        if( m_d->runningShortcut ) {
            QTouchEvent touchEvent(QEvent::TouchEnd,
                                   event->device(),
                                   event->modifiers(),
                                   Qt::TouchPointReleased,
                                   event->touchPoints());
            tryEndRunningShortcut(Qt::LeftButton, &touchEvent);
        }

        m_d->touchShortcut = goodCandidate;
        m_d->usingTouch = true;

        Private::RecursionGuard guard(this);
        goodCandidate->action()->activate(goodCandidate->shortcutIndex());
        goodCandidate->action()->begin(goodCandidate->shortcutIndex(), event);

        // the tool migh have opened some dialog, which could break our event loop
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
    if(m_d->touchShortcut) {
        // first reset running shortcut to avoid infinite recursion via end()
        KisTouchShortcut *touchShortcut = m_d->touchShortcut;

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
        m_d->nativeGestureShortcut = goodCandidate;
        m_d->usingNativeGesture = true;

        Private::RecursionGuard guard(this);
        goodCandidate->action()->activate(goodCandidate->shortcutIndex());
        goodCandidate->action()->begin(goodCandidate->shortcutIndex(), event);

        // the tool migh have opened some dialog, which could break our event loop
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
    if (m_d->nativeGestureShortcut) {
        // first reset running shortcut to avoid infinite recursion via end()
        KisNativeGestureShortcut *nativeGestureShortcut = m_d->nativeGestureShortcut;

        nativeGestureShortcut->action()->end(event);
        nativeGestureShortcut->action()->deactivate(m_d->nativeGestureShortcut->shortcutIndex());

        m_d->nativeGestureShortcut = 0; // empty it out now that we are done with it

        return true;
    }

    return false;
}
