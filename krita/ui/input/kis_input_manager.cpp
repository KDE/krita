/* This file is part of the KDE project * Copyright (C) 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
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

#include "kis_input_manager.h"

#include <QDebug>
#include <QQueue>
#include <QMessageBox>

#include <klocalizedstring.h>
#include <QApplication>

#include <KoToolManager.h>

#include "kis_tool_proxy.h"

#include <kis_config.h>
#include <kis_canvas2.h>
#include <KisViewManager.h>
#include <kis_image.h>
#include <kis_canvas_resource_provider.h>
#include <kis_favorite_resource_manager.h>

#include "kis_abstract_input_action.h"
#include "kis_tool_invocation_action.h"
#include "kis_pan_action.h"
#include "kis_alternate_invocation_action.h"
#include "kis_rotate_canvas_action.h"
#include "kis_zoom_action.h"
#include "kis_show_palette_action.h"
#include "kis_change_primary_setting_action.h"

#include "kis_shortcut_matcher.h"
#include "kis_stroke_shortcut.h"
#include "kis_single_action_shortcut.h"
#include "kis_touch_shortcut.h"

#include "kis_input_profile.h"
#include "kis_input_profile_manager.h"
#include "kis_shortcut_configuration.h"

#include <input/kis_tablet_debugger.h>
#include <input/kis_tablet_event.h>
#include <kis_signal_compressor.h>

#include "kis_extended_modifiers_mapper.h"

template <typename T>
uint qHash(QPointer<T> value) {
    return reinterpret_cast<quintptr>(value.data());
}

class KisInputManager::Private
{
public:
    Private(KisInputManager *qq)
        : q(qq)
        , toolProxy(0)
        , forwardAllEventsToTool(false)
        , ignoreQtCursorEvents(false)
        , disableTouchOnCanvas(false)
        , touchHasBlockedPressEvents(false)
    #ifdef Q_WS_X11
        , hiResEventsWorkaroundCoeff(1.0, 1.0)
    #endif
        , lastTabletEvent(0)
        , lastTouchEvent(0)
        , defaultInputAction(0)
        , eventsReceiver(0)
        , moveEventCompressor(10 /* ms */, KisSignalCompressor::FIRST_ACTIVE)
    {
        KisConfig cfg;
        disableTouchOnCanvas = cfg.disableTouchOnCanvas();
    }

    bool tryHidePopupPalette();
    void saveTabletEvent(const QTabletEvent *event);
    void resetSavedTabletEvent(QEvent::Type type);
    void addStrokeShortcut(KisAbstractInputAction* action, int index, const QList< Qt::Key >& modifiers, Qt::MouseButtons buttons);
    void addKeyShortcut(KisAbstractInputAction* action, int index,const QList<Qt::Key> &keys);
    void addTouchShortcut( KisAbstractInputAction* action, int index, KisShortcutConfiguration::GestureAction gesture );
    void addWheelShortcut(KisAbstractInputAction* action, int index, const QList< Qt::Key >& modifiers, KisShortcutConfiguration::MouseWheelMovement wheelAction);
    bool processUnhandledEvent(QEvent *event);
    Qt::Key workaroundShiftAltMetaHell(const QKeyEvent *keyEvent);
    void setupActions();
    void saveTouchEvent( QTouchEvent* event );
    bool handleKisTabletEvent(QObject *object, KisTabletEvent *tevent);

    KisInputManager *q;

    KisCanvas2 *canvas;
    KisToolProxy *toolProxy;

    bool forwardAllEventsToTool;
    bool ignoreQtCursorEvents;

    bool disableTouchOnCanvas;
    bool touchHasBlockedPressEvents;

    KisShortcutMatcher matcher;
#ifdef Q_WS_X11
    QPointF hiResEventsWorkaroundCoeff;
#endif
    QTabletEvent *lastTabletEvent;
    QTouchEvent *lastTouchEvent;

    KisToolInvocationAction *defaultInputAction;

    QObject *eventsReceiver;
    KisSignalCompressor moveEventCompressor;
    QScopedPointer<KisTabletEvent> compressedMoveEvent;

    QSet<QPointer<QObject> > priorityEventFilter;

    template <class Event, bool useBlocking>
    void debugEvent(QEvent *event);

    class ProximityNotifier;

    class CanvasSwitcher;
    QPointer<CanvasSwitcher> canvasSwitcher;
};

template <class Event, bool useBlocking>
void KisInputManager::Private::debugEvent(QEvent *event)
{
    if (!KisTabletDebugger::instance()->debugEnabled()) return;
    QString msg1 = useBlocking && ignoreQtCursorEvents ? "[BLOCKED] " : "[       ]";
    Event *specificEvent = static_cast<Event*>(event);
    qDebug() << KisTabletDebugger::instance()->eventToString(*specificEvent, msg1);
}

#define start_ignore_cursor_events() d->ignoreQtCursorEvents = true
#define stop_ignore_cursor_events() d->ignoreQtCursorEvents = false
#define break_if_should_ignore_cursor_events() if (d->ignoreQtCursorEvents) break;

#define push_and_stop_ignore_cursor_events() bool __saved_ignore_events = d->ignoreQtCursorEvents; d->ignoreQtCursorEvents = false
#define pop_ignore_cursor_events() d->ignoreQtCursorEvents = __saved_ignore_events

#define touch_start_block_press_events() d->touchHasBlockedPressEvents = d->disableTouchOnCanvas
#define touch_stop_block_press_events() d->touchHasBlockedPressEvents = false
#define break_if_touch_blocked_press_events() if (d->touchHasBlockedPressEvents) break;

class KisInputManager::Private::CanvasSwitcher : public QObject {
public:
    CanvasSwitcher(Private *_d, QObject *p)
        : QObject(p),
          d(_d),
          eatOneMouseStroke(false)
    {
    }

    void addCanvas(KisCanvas2 *canvas) {
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

    void removeCanvas(KisCanvas2 *canvas) {
        QObject *widget = canvas->canvasWidget();

        canvasResolver.remove(widget);

        if (d->eventsReceiver == widget) {
            d->q->setupAsEventFilter(0);
        }

        widget->removeEventFilter(this);
    }

    bool eventFilter(QObject* object, QEvent* event ) {
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

private:
    KisInputManager::Private *d;
    QMap<QObject*, KisCanvas2*> canvasResolver;
    int eatOneMouseStroke;
};

class KisInputManager::Private::ProximityNotifier : public QObject {
public:
    ProximityNotifier(Private *_d, QObject *p) : QObject(p), d(_d) {}

    bool eventFilter(QObject* object, QEvent* event ) {
        switch (event->type()) {
        case QEvent::TabletEnterProximity:
            d->debugEvent<QEvent, false>(event);
            start_ignore_cursor_events();
            break;
        case QEvent::TabletLeaveProximity:
            d->debugEvent<QEvent, false>(event);
            stop_ignore_cursor_events();
            break;
        default:
            break;
        }
        return QObject::eventFilter(object, event);
    }

private:
    KisInputManager::Private *d;
};

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

#ifdef Q_WS_X11
inline QPointF dividePoints(const QPointF &pt1, const QPointF &pt2) {
    return QPointF(pt1.x() / pt2.x(), pt1.y() / pt2.y());
}

inline QPointF multiplyPoints(const QPointF &pt1, const QPointF &pt2) {
    return QPointF(pt1.x() * pt2.x(), pt1.y() * pt2.y());
}
#endif

void KisInputManager::Private::saveTabletEvent(const QTabletEvent *event)
{
    delete lastTabletEvent;

#ifdef Q_WS_X11
    /**
     * There is a bug in Qt-x11 when working in 2 tablets + 2 monitors
     * setup. The hiResGlobalPos() value gets scaled wrongly somehow.
     * Happily, the error is linear (without the offset) so we can simply
     * scale it a bit.
     */
    if (event->type() == QEvent::TabletPress) {
        if ((event->globalPos() - event->hiResGlobalPos()).manhattanLength() > 4) {
            hiResEventsWorkaroundCoeff = dividePoints(event->globalPos(), event->hiResGlobalPos());
        } else {
            hiResEventsWorkaroundCoeff = QPointF(1.0, 1.0);
        }
    }
#endif

    lastTabletEvent =
            new QTabletEvent(event->type(),
                             event->pos(),
                             event->globalPos(),
                         #ifdef Q_WS_X11
                             multiplyPoints(event->hiResGlobalPos(), hiResEventsWorkaroundCoeff),
                         #else
                             event->hiResGlobalPos(),
                         #endif
                             event->device(),
                             event->pointerType(),
                             event->pressure(),
                             event->xTilt(),
                             event->yTilt(),
                             event->tangentialPressure(),
                             event->rotation(),
                             event->z(),
                             event->modifiers(),
                             event->uniqueId());
}

void KisInputManager::Private::saveTouchEvent( QTouchEvent* event )
{
    delete lastTouchEvent;
    lastTouchEvent = new QTouchEvent(event->type(), event->deviceType(), event->modifiers(), event->touchPointStates(), event->touchPoints());
}

void KisInputManager::Private::resetSavedTabletEvent(QEvent::Type /*type*/)
{
    /**
     * On both Windows and Linux each mouse event corresponds to a
     * single tablet event, so the saved event must be reset after
     * every mouse-related event
     */

    delete lastTabletEvent;
    lastTabletEvent = 0;
}

KisInputManager::KisInputManager(QObject *parent)
    : QObject(parent), d(new Private(this))
{
    d->canvas = 0;
    d->toolProxy = 0;

    d->setupActions();

    connect(KoToolManager::instance(), SIGNAL(changedTool(KoCanvasController*,int)),
            SLOT(slotToolChanged()));
    connect(&d->moveEventCompressor, SIGNAL(timeout()), SLOT(slotCompressedMoveEvent()));


#ifndef Q_OS_MAC
    QApplication::instance()->
        installEventFilter(new Private::ProximityNotifier(d, this));
#endif

    d->canvasSwitcher = new Private::CanvasSwitcher(d, this);
}

KisInputManager::~KisInputManager()
{
    delete d;
}

void KisInputManager::addTrackedCanvas(KisCanvas2 *canvas)
{
    d->canvasSwitcher->addCanvas(canvas);
}

void KisInputManager::removeTrackedCanvas(KisCanvas2 *canvas)
{
    d->canvasSwitcher->removeCanvas(canvas);
}

void KisInputManager::toggleTabletLogger()
{
    KisTabletDebugger::instance()->toggleDebugging();

    bool enabled = KisTabletDebugger::instance()->debugEnabled();
    QMessageBox::information(0, "Krita", enabled ? "Tablet Event Logging Enabled" :
                             "Tablet Event Logging Disabled");
    if (enabled) {
        qDebug() << "vvvvvvvvvvvvvvvvvvvvvvv START TABLET EVENT LOG vvvvvvvvvvvvvvvvvvvvvvv";
    }
    else {
        qDebug() << "^^^^^^^^^^^^^^^^^^^^^^^ START TABLET EVENT LOG ^^^^^^^^^^^^^^^^^^^^^^^";
    }
}

void KisInputManager::attachPriorityEventFilter(QObject *filter)
{
    d->priorityEventFilter.insert(QPointer<QObject>(filter));
}

void KisInputManager::detachPriorityEventFilter(QObject *filter)
{
    d->priorityEventFilter.remove(QPointer<QObject>(filter));
}

void KisInputManager::setupAsEventFilter(QObject *receiver)
{
    if (d->eventsReceiver) {
        d->eventsReceiver->removeEventFilter(this);
    }

    d->eventsReceiver = receiver;

    if (d->eventsReceiver) {
        d->eventsReceiver->installEventFilter(this);
    }
}

void KisInputManager::stopIgnoringEvents()
{
    stop_ignore_cursor_events();
}

#if defined (__clang__)
#pragma GCC diagnostic ignored "-Wswitch"
#endif

bool KisInputManager::eventFilter(QObject* object, QEvent* event)
{
    bool retval = false;
    if (object != d->eventsReceiver) return retval;

    if (!d->ignoreQtCursorEvents ||
        (event->type() != QEvent::MouseButtonPress &&
         event->type() != QEvent::MouseButtonDblClick &&
         event->type() != QEvent::MouseButtonRelease &&
         event->type() != QEvent::MouseMove &&
         event->type() != QEvent::TabletPress &&
         event->type() != QEvent::TabletMove &&
         event->type() != QEvent::TabletRelease)) {

        foreach (QPointer<QObject> filter, d->priorityEventFilter) {
            if (filter.isNull()) {
                d->priorityEventFilter.remove(filter);
                continue;
            }

            if (filter->eventFilter(object, event)) return true;
        }
    }

    // KoToolProxy needs to pre-process some events to ensure the
    // global shortcuts (not the input manager's ones) are not
    // executed, in particular, this line will accept events when the
    // tool is in text editing, preventing shortcut triggering
    d->toolProxy->processEvent(event);

    // because we have fake enums in here...
    switch (event->type()) {
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonDblClick: {
        d->debugEvent<QMouseEvent, true>(event);
        break_if_should_ignore_cursor_events();
        break_if_touch_blocked_press_events();

        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);

        if (d->tryHidePopupPalette()) {
            retval = true;
        } else {
            //Make sure the input actions know we are active.
            KisAbstractInputAction::setInputManager(this);
            retval = d->matcher.buttonPressed(mouseEvent->button(), mouseEvent);
        }
        d->resetSavedTabletEvent(event->type());
        event->setAccepted(retval);
        break;
    }
    case QEvent::MouseButtonRelease: {
        d->debugEvent<QMouseEvent, true>(event);
        break_if_should_ignore_cursor_events();
        break_if_touch_blocked_press_events();

        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        retval = d->matcher.buttonReleased(mouseEvent->button(), mouseEvent);
        d->resetSavedTabletEvent(event->type());
        event->setAccepted(retval);
        break;
    }
    case QEvent::ShortcutOverride: {
        d->debugEvent<QKeyEvent, false>(event);
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);

        Qt::Key key = d->workaroundShiftAltMetaHell(keyEvent);

        if (!keyEvent->isAutoRepeat()) {
            retval = d->matcher.keyPressed(key);
        } else {
            retval = d->matcher.autoRepeatedKeyPressed(key);
        }

        /**
         * Workaround for temporary switching of tools by
         * KoCanvasControllerWidget. We don't need this switch because
         * we handle it ourselves.
         */
        retval |= !d->forwardAllEventsToTool &&
                (keyEvent->key() == Qt::Key_Space ||
                 keyEvent->key() == Qt::Key_Escape);

        break;
    }
    case QEvent::KeyRelease: {
        d->debugEvent<QKeyEvent, false>(event);
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);

        if (!keyEvent->isAutoRepeat()) {
            Qt::Key key = d->workaroundShiftAltMetaHell(keyEvent);
            retval = d->matcher.keyReleased(key);
        }
        break;
    }
    case QEvent::MouseMove: {
        d->debugEvent<QMouseEvent, true>(event);
        break_if_should_ignore_cursor_events();

        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (!d->matcher.mouseMoved(mouseEvent)) {
            //Update the current tool so things like the brush outline gets updated.
            d->toolProxy->forwardMouseHoverEvent(mouseEvent, lastTabletEvent());
        }
        retval = true;
        event->setAccepted(retval);
        d->resetSavedTabletEvent(event->type());
        break;
    }
    case QEvent::Wheel: {
        d->debugEvent<QWheelEvent, false>(event);
        QWheelEvent *wheelEvent = static_cast<QWheelEvent*>(event);
        KisSingleActionShortcut::WheelAction action;

        if(wheelEvent->orientation() == Qt::Horizontal) {
            if(wheelEvent->delta() < 0) {
                action = KisSingleActionShortcut::WheelRight;
            }
            else {
                action = KisSingleActionShortcut::WheelLeft;
            }
        }
        else {
            if(wheelEvent->delta() > 0) {
                action = KisSingleActionShortcut::WheelUp;
            }
            else {
                action = KisSingleActionShortcut::WheelDown;
            }
        }

        //Make sure the input actions know we are active.
        KisAbstractInputAction::setInputManager(this);
        retval = d->matcher.wheelEvent(action, wheelEvent);
        break;
    }
    case QEvent::Enter:
        d->debugEvent<QEvent, false>(event);
        //Make sure the input actions know we are active.
        KisAbstractInputAction::setInputManager(this);
        //Ensure we have focus so we get key events.
        d->canvas->canvasWidget()->setFocus();
        stop_ignore_cursor_events();
        touch_stop_block_press_events();

        d->matcher.enterEvent();
        break;
    case QEvent::Leave:
        d->debugEvent<QEvent, false>(event);
        /**
         * We won't get a TabletProximityLeave event when the tablet
         * is hovering above some other widget, so restore cursor
         * events processing right now.
         */
        stop_ignore_cursor_events();
        touch_stop_block_press_events();

        d->matcher.leaveEvent();
        break;
    case QEvent::FocusIn:
        d->debugEvent<QEvent, false>(event);
        KisAbstractInputAction::setInputManager(this);

        //Clear all state so we don't have half-matched shortcuts dangling around.
        d->matcher.reinitialize();

        { // Emulate pressing of the key that are already pressed
            KisExtendedModifiersMapper mapper;

            Qt::KeyboardModifiers modifiers = mapper.queryStandardModifiers();
            foreach (Qt::Key key, mapper.queryExtendedModifiers()) {
                QKeyEvent kevent(QEvent::KeyPress, key, modifiers);
                eventFilter(object, &kevent);
            }
        }

        stop_ignore_cursor_events();
        break;
    case QEvent::TabletPress:
    case QEvent::TabletMove:
    case QEvent::TabletRelease: {
        d->debugEvent<QTabletEvent, true>(event);
        break_if_should_ignore_cursor_events();
        break_if_touch_blocked_press_events();

        //We want both the tablet information and the mouse button state.
        //Since QTabletEvent only provides the tablet information, we
        //save that and then ignore the event so it will generate a mouse
        //event.
        QTabletEvent* tabletEvent = static_cast<QTabletEvent*>(event);
        d->saveTabletEvent(tabletEvent);
        event->ignore();

        break;
    }
    case KisTabletEvent::TabletPressEx:
    case KisTabletEvent::TabletMoveEx:
    case KisTabletEvent::TabletReleaseEx: {
        d->debugEvent<KisTabletEvent, false>(event);
        stop_ignore_cursor_events();
        touch_stop_block_press_events();

        KisTabletEvent *tevent = static_cast<KisTabletEvent*>(event);

        if (tevent->type() == (QEvent::Type)KisTabletEvent::TabletMoveEx &&
            !d->matcher.supportsHiResInputEvents()) {

            d->compressedMoveEvent.reset(new KisTabletEvent(*tevent));
            d->moveEventCompressor.start();
            tevent->setAccepted(true);

            retval = true;
        } else {
            slotCompressedMoveEvent();
            retval = d->handleKisTabletEvent(object, tevent);
        }

        /**
         * The flow of tablet events means the tablet is in the
         * proximity area, so activate it even when the
         * TabletEnterProximity event was missed (may happen when
         * changing focus of the window with tablet in the proximity
         * area)
         */
        start_ignore_cursor_events();

        break;
    }
    case KisTabletEvent::TouchProximityInEx: {
        touch_start_block_press_events();
        break;
    }
    case KisTabletEvent::TouchProximityOutEx: {
        touch_stop_block_press_events();
        break;
    }

    case QEvent::TouchBegin:
        touch_start_block_press_events();
        KisAbstractInputAction::setInputManager(this);

        retval = d->matcher.touchBeginEvent(static_cast<QTouchEvent*>(event));
        event->accept();
        d->resetSavedTabletEvent(event->type());
        break;
    case QEvent::TouchUpdate:
        touch_start_block_press_events();
        KisAbstractInputAction::setInputManager(this);

        retval = d->matcher.touchUpdateEvent(static_cast<QTouchEvent*>(event));
        event->accept();
        d->resetSavedTabletEvent(event->type());
        break;
    case QEvent::TouchEnd:
        touch_stop_block_press_events();
        d->saveTouchEvent(static_cast<QTouchEvent*>(event));
        retval = d->matcher.touchEndEvent(static_cast<QTouchEvent*>(event));
        event->accept();
        d->resetSavedTabletEvent(event->type());
        delete d->lastTouchEvent;
        d->lastTouchEvent = 0;
        break;
    default:
        break;
    }

    return !retval ? d->processUnhandledEvent(event) : true;
}

bool KisInputManager::Private::handleKisTabletEvent(QObject *object, KisTabletEvent *tevent)
{
    bool retval = false;

    QTabletEvent qte = tevent->toQTabletEvent();
    qte.ignore();
    q->eventFilter(object, &qte);
    tevent->setAccepted(qte.isAccepted());

    if (!retval && !qte.isAccepted()) {
        QMouseEvent qme = tevent->toQMouseEvent();
        qme.ignore();
        q->eventFilter(object, &qme);
        tevent->setAccepted(qme.isAccepted());
    }

    return tevent->isAccepted();
}

void KisInputManager::slotCompressedMoveEvent()
{
    if (d->compressedMoveEvent) {

        push_and_stop_ignore_cursor_events();
        touch_stop_block_press_events();

        (void) d->handleKisTabletEvent(d->eventsReceiver, d->compressedMoveEvent.data());

        pop_ignore_cursor_events();

        d->compressedMoveEvent.reset();
    }
}

KisCanvas2* KisInputManager::canvas() const
{
    return d->canvas;
}

KisToolProxy* KisInputManager::toolProxy() const
{
    return d->toolProxy;
}

QTabletEvent* KisInputManager::lastTabletEvent() const
{
    return d->lastTabletEvent;
}

QTouchEvent *KisInputManager::lastTouchEvent() const
{
    return d->lastTouchEvent;
}

void KisInputManager::slotToolChanged()
{
    QString toolId = KoToolManager::instance()->activeToolId();
    if (toolId == "ArtisticTextToolFactoryID" || toolId == "TextToolFactory_ID") {
        d->forwardAllEventsToTool = true;
        d->matcher.suppressAllActions(true);
    } else {
        d->forwardAllEventsToTool = false;
        d->matcher.suppressAllActions(false);
    }
}

QPointF KisInputManager::widgetToDocument(const QPointF& position)
{
    QPointF pixel = QPointF(position.x() + 0.5f, position.y() + 0.5f);
    return d->canvas->coordinatesConverter()->widgetToDocument(pixel);
}

void KisInputManager::profileChanged()
{
    d->matcher.clearShortcuts();

    KisInputProfile *profile = KisInputProfileManager::instance()->currentProfile();
    if (profile) {
        QList<KisShortcutConfiguration*> shortcuts = profile->allShortcuts();
        foreach(KisShortcutConfiguration *shortcut, shortcuts) {
            switch(shortcut->type()) {
            case KisShortcutConfiguration::KeyCombinationType:
                d->addKeyShortcut(shortcut->action(), shortcut->mode(), shortcut->keys());
                break;
            case KisShortcutConfiguration::MouseButtonType:
                d->addStrokeShortcut(shortcut->action(), shortcut->mode(), shortcut->keys(), shortcut->buttons());
                break;
            case KisShortcutConfiguration::MouseWheelType:
                d->addWheelShortcut(shortcut->action(), shortcut->mode(), shortcut->keys(), shortcut->wheel());
                break;
            case KisShortcutConfiguration::GestureType:
                d->addTouchShortcut(shortcut->action(), shortcut->mode(), shortcut->gesture());
                break;
            default:
                break;
            }
        }
    }
    else {
        kWarning() << "No Input Profile Found: canvas interaction will be impossible";
    }
}

