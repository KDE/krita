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

#include "kis_config.h"
#include "kis_abstract_input_action.h"
#include "kis_stroke_shortcut.h"
#include "kis_touch_shortcut.h"
#include "kis_input_profile_manager.h"
#include "input/kis_tablet_debugger.h"



// Note: this class is intended to model all event masking logic.
class EventEater : public QObject
{
public:
    EventEater() : QObject(qApp), hungry(false) {}

    bool eventFilter(QObject* object, QEvent* event )
    {
        if (hungry && (event->type() == QEvent::MouseMove ||
                       event->type() == QEvent::MouseButtonPress ||
                       event->type() == QEvent::MouseButtonRelease)) {
            if (KisTabletDebugger::instance()->debugEnabled()) {
                QString pre = QString("[FILTERED]");
                QMouseEvent *ev = static_cast<QMouseEvent*>(event);
                dbgTablet << KisTabletDebugger::instance()->eventToString(*ev,pre);
            }
            return true;
        }
        return false;
    }

    void activate()
    {
        if (!hungry && (KisTabletDebugger::instance()->debugEnabled()))
            dbgTablet << "Ignoring mouse events.";
        hungry = true;
    }

    void deactivate()
    {
        if (!hungry && (KisTabletDebugger::instance()->debugEnabled()))
            dbgTablet << "Accepting mouse events.";
        hungry = false;
    }

    bool isActive()
    {
        return hungry;
    }

private:
    bool hungry;
};

// Should be Q_GLOBAL_STATIC?
EventEater *globalEventEater;

bool KisInputManager::Private::ignoreQtCursorEvents()
{
    return globalEventEater->isActive();
}

KisInputManager::Private::Private(KisInputManager *qq)
    : q(qq)
    , canvas(0)
    , toolProxy(0)
    , forwardAllEventsToTool(false)
    , disableTouchOnCanvas(false)
    , touchHasBlockedPressEvents(false)
    , lastTouchEvent(0)
    , defaultInputAction(0)
    , eventsReceiver(0)
    , moveEventCompressor(10 /* ms */, KisSignalCompressor::FIRST_ACTIVE)
    , testingAcceptCompressedTabletEvents(false)
    , testingCompressBrushEvents(false)
    , focusOnEnter(true)
    , containsPointer(true)
{
    KisConfig cfg;
    disableTouchOnCanvas = cfg.disableTouchOnCanvas();

    moveEventCompressor.setDelay(cfg.tabletEventsDelay());
    testingAcceptCompressedTabletEvents = cfg.testingAcceptCompressedTabletEvents();
    testingCompressBrushEvents = cfg.testingCompressBrushEvents();
    setupActions();
    canvasSwitcher = new CanvasSwitcher(this, q);
    globalEventEater = new EventEater();
    qApp->installEventFilter(globalEventEater);
}



KisInputManager::Private::~Private()
{
    delete canvasSwitcher;
    delete globalEventEater;
}

KisInputManager::Private::CanvasSwitcher::CanvasSwitcher(Private *_d, QObject *p)
    : QObject(p),
      d(_d),
      eatOneMouseStroke(false)
{
}

void KisInputManager::Private::CanvasSwitcher::addCanvas(KisCanvas2 *canvas)
{
    QObject *widget = canvas->canvasWidget();

    if (!canvasResolver.contains(widget)) {
        canvasResolver.insert(widget, canvas);
        d->q->setupAsEventFilter(widget);
        widget->installEventFilter(this);

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
        // d->blockMouseEvents();  Qt sends fake mouse events instead of hover events, so disable this for now.
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
        strokeShortcut->setButtons(modifiers, buttonList);
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
    QList<Qt::Key> modifiers = keys;
    Qt::Key key = modifiers.takeLast();
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

    keyShortcut->setWheel(modifiers, a);
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

Qt::Key KisInputManager::Private::workaroundShiftAltMetaHell(const QKeyEvent *keyEvent)
{
    Qt::Key key = (Qt::Key)keyEvent->key();

    if (keyEvent->key() == Qt::Key_Meta &&
        keyEvent->modifiers().testFlag(Qt::ShiftModifier)) {

        key = Qt::Key_Alt;
    }

    return key;
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
    globalEventEater->activate();
}

void KisInputManager::Private::allowMouseEvents()
{
    globalEventEater->deactivate();
}

bool KisInputManager::Private::handleCompressedTabletEvent(QObject *object, QTabletEvent *tevent)
{
    if(object == 0) return false;

    bool retval = false;

    retval = q->eventFilter(object, tevent);

    if (!retval && !tevent->isAccepted()) {
        dbgTablet << "Rejected a compressed tablet event.";
    }

    return retval;
}
