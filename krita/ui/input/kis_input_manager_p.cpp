/*
 *  Copyright (C) 2015 Michael Abrahams <miabraha@gmail.com>
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

#include "kis_input_manager_p.h"

#include <QMap>
#include <QApplication>
#include <QScopedPointer>
#include <QtGlobal>

#include "kis_input_manager.h"
#include "kis_config.h"
#include "kis_abstract_input_action.h"
#include "kis_tool_invocation_action.h"
#include "kis_stroke_shortcut.h"
#include "kis_touch_shortcut.h"
#include "kis_input_profile_manager.h"



/**
 * This hungry class EventEater encapsulates event masking logic.
 *
 * It should not be necessary if we handle our own events. Currently this is done on X11 only.
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
 * every mouse event is fake. The only problem is the boundary case at the
 * beginning of a stroke: since a newly arriving MousePress event may be a fake
 * synthetic one or an actual press, and the only context that can tell us this
 * is if the event *after* it is a TabletPress. The solution is to store one
 * MousePress event in the EventEater until the next event arrives. Once it sees
 * the event following, it will know whether to forward the MouseMove event or
 * to toss it away. If the event after a MousePress event is a tablet event, the
 * MousePress was synthetic, so toss it. If the event was a MouseMove, it was a
 * real mouse click, so keep it. If it is possible that the arrival of events
 * can be off by more than one, this solution will not work. On the other
 * hand, that would be simply preposterous.
 */

#if !defined(HAVE_X11)
bool KisInputManager::Private::EventEater::eventFilter(QObject* target, QEvent* event )
{
    if ((hungry && (event->type() == QEvent::MouseMove ||
                    event->type() == QEvent::MouseButtonPress ||
                    event->type() == QEvent::MouseButtonRelease))
        //  || (peckish && (event->type() == QEvent::MouseButtonPress))
        )
    {
        // Chow down
        if (KisTabletDebugger::instance()->debugEnabled()) {
            QString pre = QString("[BLOCKED]");
            QMouseEvent *ev = static_cast<QMouseEvent*>(event);
            dbgInput << KisTabletDebugger::instance()->eventToString(*ev,pre);
        }
        peckish = false;
        return true;
    }
    else if ((event->type() == QEvent::MouseButtonPress) /* Need to scrutinize */ &&
             (!savedEvent)) /* Otherwise we enter a loop repeatedly storing the same event */
    {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        // Pocket the event and decide what to do with it later
        // savedEvent = *(static_cast<QMouseEvent*>(event));
        savedEvent = new QMouseEvent(QEvent::MouseButtonPress,
                                     mouseEvent->pos(),
                                     mouseEvent->windowPos(),
                                     mouseEvent->screenPos(),
                                     mouseEvent->button(),
                                     mouseEvent->buttons(),
                                     mouseEvent->modifiers());
        savedTarget = target;
        mouseEvent->accept();
        return true;
    }

    return false; // All clear - let this one through!
}


void KisInputManager::Private::EventEater::activate()
{
    if (!hungry && (KisTabletDebugger::instance()->debugEnabled()))
        dbgInput << "Ignoring mouse events.";
    hungry = true;
}

void KisInputManager::Private::EventEater::deactivate()
{
    if (!hungry && (KisTabletDebugger::instance()->debugEnabled()))
        dbgInput << "Accepting mouse events.";
    hungry = false;
}

// This would be a solution if we had reliable proximity events. SIGH
// void KisInputManager::Private::eatOneMousePress()
// {
//     peckish = true;
// }

bool KisInputManager::Private::EventEater::isActive()
{
    return hungry;
}
#endif

bool KisInputManager::Private::ignoreQtCursorEvents()
{
#if !defined(HAVE_X11)
    return eventEater.isActive();
#else
    return false;
#endif
}

KisInputManager::Private::Private(KisInputManager *qq)
    : q(qq)
    , moveEventCompressor(10 /* ms */, KisSignalCompressor::FIRST_ACTIVE)
    , canvasSwitcher(this, qq)
{
    KisConfig cfg;
    disableTouchOnCanvas = cfg.disableTouchOnCanvas();

    moveEventCompressor.setDelay(cfg.tabletEventsDelay());
    testingAcceptCompressedTabletEvents = cfg.testingAcceptCompressedTabletEvents();
    testingCompressBrushEvents = cfg.testingCompressBrushEvents();
    setupActions();
}


KisInputManager::Private::CanvasSwitcher::CanvasSwitcher(Private *_d, QObject *p)
    : QObject(p),
      d(_d),
      eatOneMouseStroke(false)
{
}

void KisInputManager::Private::CanvasSwitcher::addCanvas(KisCanvas2 *canvas)
{
    QObject *canvasWidget = canvas->canvasWidget();

    if (!canvasResolver.contains(canvasWidget)) {
        canvasResolver.insert(canvasWidget, canvas);
        d->q->setupAsEventFilter(canvasWidget);
        canvasWidget->installEventFilter(this);

        d->canvas = canvas;
        d->toolProxy = dynamic_cast<KisToolProxy*>(canvas->toolProxy());
    } else {
        KIS_ASSERT_RECOVER_RETURN(d->canvas == canvas);
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
}

bool KisInputManager::Private::CanvasSwitcher::eventFilter(QObject* object, QEvent* event )
{
    if (canvasResolver.contains(object)) {
        switch (event->type()) {
        case QEvent::FocusIn: {
            QFocusEvent *fevent = static_cast<QFocusEvent*>(event);
            eatOneMouseStroke = 2 * (fevent->reason() == Qt::MouseFocusReason);

            KisCanvas2 *canvas = canvasResolver.value(object);
            d->canvas = canvas;
            d->toolProxy = dynamic_cast<KisToolProxy*>(canvas->toolProxy());

            d->q->setupAsEventFilter(object);

            object->removeEventFilter(this);
            object->installEventFilter(this);

            QEvent event(QEvent::Enter);
            d->q->eventFilter(object, &event);
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
            if (eatOneMouseStroke) {
                eatOneMouseStroke--;
                return true;
            }
            break;
        case QEvent::MouseButtonDblClick:
            if (eatOneMouseStroke) {
                return true;
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
        d->debugEvent<QEvent, false>(event);
        // Tablet proximity events are unreliable AND fake mouse events do not
        // necessarily come after tablet events, so this is insufficient.
        // d->eventEater.eatOneMousePress();

        // Qt sends fake mouse events instead of hover events, so not very useful.
        d->blockMouseEvents();
        break;
    case QEvent::TabletLeaveProximity:
        d->debugEvent<QEvent, false>(event);
        d->allowMouseEvents();
        break;
    default:
        break;
    }
    return QObject::eventFilter(object, event);
}

void KisInputManager::Private::addStrokeShortcut(KisAbstractInputAction* action, int index,
                                                 const QList<Qt::Key> &modifiers,
                                                 Qt::MouseButtons buttons)
{
    KisStrokeShortcut *strokeShortcut =
        new KisStrokeShortcut(action, index);

    QList<Qt::MouseButton> buttonList;
    if(buttons & Qt::LeftButton) {
        buttonList << Qt::LeftButton;
    }
    if(buttons & Qt::RightButton) {
        buttonList << Qt::RightButton;
    }
    if(buttons & Qt::MidButton) {
        buttonList << Qt::MidButton;
    }
    if(buttons & Qt::XButton1) {
        buttonList << Qt::XButton1;
    }
    if(buttons & Qt::XButton2) {
        buttonList << Qt::XButton2;
    }

    if (buttonList.size() > 0) {
        strokeShortcut->setButtons(QSet<Qt::Key>::fromList(modifiers), QSet<Qt::MouseButton>::fromList(buttonList));
        matcher.addShortcut(strokeShortcut);
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
    QSet<Qt::Key> modifiers = QSet<Qt::Key>::fromList(allKeys);
    keyShortcut->setKey(modifiers, key);
    matcher.addShortcut(keyShortcut);
}

void KisInputManager::Private::addWheelShortcut(KisAbstractInputAction* action, int index,
                                                const QList<Qt::Key> &modifiers,
                                                KisShortcutConfiguration::MouseWheelMovement wheelAction)
{
    KisSingleActionShortcut *keyShortcut =
        new KisSingleActionShortcut(action, index);

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
    default:
        return;
    }

    keyShortcut->setWheel(QSet<Qt::Key>::fromList(modifiers), a);
    matcher.addShortcut(keyShortcut);
}

void KisInputManager::Private::addTouchShortcut( KisAbstractInputAction* action, int index, KisShortcutConfiguration::GestureAction gesture)
{
    KisTouchShortcut *shortcut = new KisTouchShortcut(action, index);
    switch(gesture) {
    case KisShortcutConfiguration::PinchGesture:
        shortcut->setMinimumTouchPoints(2);
        shortcut->setMaximumTouchPoints(2);
        break;
    case KisShortcutConfiguration::PanGesture:
        shortcut->setMinimumTouchPoints(3);
        shortcut->setMaximumTouchPoints(10);
        break;
    default:
        break;
    }
    matcher.addShortcut(shortcut);
}

void KisInputManager::Private::setupActions()
{
    QList<KisAbstractInputAction*> actions = KisInputProfileManager::instance()->actions();
    foreach(KisAbstractInputAction *action, actions) {
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

bool KisInputManager::Private::tryHidePopupPalette()
{
    if (canvas->isPopupPaletteVisible()) {
        canvas->slotShowPopupPalette();
        return true;
    }
    return false;
}

#ifdef HAVE_X11
inline QPointF dividePoints(const QPointF &pt1, const QPointF &pt2) {
    return QPointF(pt1.x() / pt2.x(), pt1.y() / pt2.y());
}

inline QPointF multiplyPoints(const QPointF &pt1, const QPointF &pt2) {
    return QPointF(pt1.x() * pt2.x(), pt1.y() * pt2.y());
}
#endif

void KisInputManager::Private::saveTouchEvent( QTouchEvent* event )
{
    delete lastTouchEvent;
    lastTouchEvent = new QTouchEvent(event->type(), event->device(), event->modifiers(), event->touchPointStates(), event->touchPoints());
}


void KisInputManager::Private::blockMouseEvents()
{
#if !defined(HAVE_X11)
    eventEater.activate();
#endif
}

void KisInputManager::Private::allowMouseEvents()
{
#if !defined(HAVE_X11)
    eventEater.deactivate();
#endif
}

bool KisInputManager::Private::handleCompressedTabletEvent(QObject *object, QTabletEvent *tevent)
{
    if(object == 0) return false;

    bool retval = false;

    retval = q->eventFilter(object, tevent);

    if (!retval && !tevent->isAccepted()) {
        dbgInput << "Rejected a compressed tablet event.";
    }

    return retval;
}
