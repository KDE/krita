/* This file is part of the KDE project
 *
 *  Copyright (C) 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
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

#include "kis_input_manager.h"

#include <kis_debug.h>
#include <QQueue>
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
#include <kis_signal_compressor.h>

#include "kis_extended_modifiers_mapper.h"
#include "kis_input_manager_p.h"

template <typename T>
uint qHash(QPointer<T> value) {
    return reinterpret_cast<quintptr>(value.data());
}


#define start_ignore_cursor_events() d->blockMouseEvents()
#define stop_ignore_cursor_events() d->allowMouseEvents()
#define break_if_should_ignore_cursor_events() if (d->ignoringQtCursorEvents()) break;
#define break_if_tablet_active() if (d->tabletActive) break;

// Touch rejection: if touch is disabled on canvas, no need to block mouse press events
#define touch_start_block_press_events()  d->touchHasBlockedPressEvents = d->disableTouchOnCanvas;
#define touch_stop_block_press_events()  d->touchHasBlockedPressEvents = false;
#define break_if_touch_blocked_press_events() if (d->touchHasBlockedPressEvents) break;
#define touch_eat_one_mouse_press() if (d->disableTouchOnCanvas) d->eatOneMousePress();

KisInputManager::KisInputManager(QObject *parent)
    : QObject(parent), d(new Private(this))
{
    d->setupActions();

    connect(KoToolManager::instance(), SIGNAL(changedTool(KoCanvasController*,int)),
            SLOT(slotToolChanged()));
    connect(&d->moveEventCompressor, SIGNAL(timeout()), SLOT(slotCompressedMoveEvent()));


    QApplication::instance()->
        installEventFilter(new Private::ProximityNotifier(d, this));
}

KisInputManager::~KisInputManager()
{
    delete d;
}

void KisInputManager::addTrackedCanvas(KisCanvas2 *canvas)
{
    d->canvasSwitcher.addCanvas(canvas);
}

void KisInputManager::removeTrackedCanvas(KisCanvas2 *canvas)
{
    d->canvasSwitcher.removeCanvas(canvas);
}

void KisInputManager::toggleTabletLogger()
{
    KisTabletDebugger::instance()->toggleDebugging();
}

void KisInputManager::attachPriorityEventFilter(QObject *filter, int priority)
{
    Private::PriorityList::iterator begin = d->priorityEventFilter.begin();
    Private::PriorityList::iterator it = begin;
    Private::PriorityList::iterator end = d->priorityEventFilter.end();

    it = std::find_if(begin, end,
                      [filter] (const Private::PriorityPair &a) { return a.second == filter; });

    if (it != end) return;

    it = std::find_if(begin, end,
                      [priority] (const Private::PriorityPair &a) { return a.first > priority; });

    d->priorityEventFilter.insert(it, qMakePair(priority, filter));
    d->priorityEventFilterSeqNo++;
}

void KisInputManager::detachPriorityEventFilter(QObject *filter)
{
    Private::PriorityList::iterator it = d->priorityEventFilter.begin();
    Private::PriorityList::iterator end = d->priorityEventFilter.end();

    it = std::find_if(it, end,
                      [filter] (const Private::PriorityPair &a) { return a.second == filter; });

    if (it != end) {
        d->priorityEventFilter.erase(it);
    }
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

void KisInputManager::slotFocusOnEnter(bool value)
{
    Q_UNUSED(value);
    // not used anymore
}

#if defined (__clang__)
#pragma GCC diagnostic ignored "-Wswitch"
#endif

bool KisInputManager::eventFilter(QObject* object, QEvent* event)
{
    if (object != d->eventsReceiver) return false;

    if (d->eventEater.eventFilter(object, event)) return false;

    if (!d->matcher.hasRunningShortcut()) {

        int savedPriorityEventFilterSeqNo = d->priorityEventFilterSeqNo;

        for (auto it = d->priorityEventFilter.begin(); it != d->priorityEventFilter.end(); /*noop*/) {
            const QPointer<QObject> &filter = it->second;

            if (filter.isNull()) {
                it = d->priorityEventFilter.erase(it);

                d->priorityEventFilterSeqNo++;
                savedPriorityEventFilterSeqNo++;
                continue;
            }

            if (filter->eventFilter(object, event)) return true;

            /**
             * If the filter removed itself from the filters list or
             * added something there, just exit the loop
             */
            if (d->priorityEventFilterSeqNo != savedPriorityEventFilterSeqNo) {
                return true;
            }

            ++it;
        }

        // KoToolProxy needs to pre-process some events to ensure the
        // global shortcuts (not the input manager's ones) are not
        // executed, in particular, this line will accept events when the
        // tool is in text editing, preventing shortcut triggering
        d->toolProxy->processEvent(event);
    }

    // Continue with the actual switch statement...
    return eventFilterImpl(event);
}

template <class Event>
bool KisInputManager::compressMoveEventCommon(Event *event)
{
    /**
     * We construct a copy of this event object, so we must ensure it
     * has a correct type.
     */
    static_assert(std::is_same<Event, QMouseEvent>::value ||
                  std::is_same<Event, QTabletEvent>::value,
                  "event should be a mouse or a tablet event");

    bool retval = false;

    /**
     * Compress the events if the tool doesn't need high resolution input
     */
    if ((event->type() == QEvent::MouseMove ||
         event->type() == QEvent::TabletMove) &&
        (!d->matcher.supportsHiResInputEvents() ||
         d->testingCompressBrushEvents)) {

        d->compressedMoveEvent.reset(new Event(*event));
        d->moveEventCompressor.start();

        /**
         * On Linux Qt eats the rest of unneeded events if we
         * ignore the first of the chunk of tablet events. So
         * generally we should never activate this feature. Only
         * for testing purposes!
         */
        if (d->testingAcceptCompressedTabletEvents) {
            event->setAccepted(true);
        }

        retval = true;
    } else {
        slotCompressedMoveEvent();
        retval = d->handleCompressedTabletEvent(event);
    }

    return retval;
}

bool KisInputManager::eventFilterImpl(QEvent * event)
{
    // TODO: Handle touch events correctly.
    bool retval = false;

    switch (event->type()) {
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonDblClick: {
        d->debugEvent<QMouseEvent, true>(event);
        //Block mouse press events on Genius tablets
        break_if_tablet_active();
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
        //Reset signal compressor to prevent processing events before press late
        d->resetCompressor();
        event->setAccepted(retval);
        break;
    }
    case QEvent::MouseButtonRelease: {
        d->debugEvent<QMouseEvent, true>(event);
        break_if_should_ignore_cursor_events();
        break_if_touch_blocked_press_events();

        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        retval = d->matcher.buttonReleased(mouseEvent->button(), mouseEvent);
        event->setAccepted(retval);
        break;
    }
    case QEvent::ShortcutOverride: {
        d->debugEvent<QKeyEvent, false>(event);
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);

        Qt::Key key = KisExtendedModifiersMapper::workaroundShiftAltMetaHell(keyEvent);

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
            Qt::Key key = KisExtendedModifiersMapper::workaroundShiftAltMetaHell(keyEvent);
            retval = d->matcher.keyReleased(key);
        }
        break;
    }
    case QEvent::MouseMove: {
        d->debugEvent<QMouseEvent, true>(event);
        break_if_should_ignore_cursor_events();

        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        retval = compressMoveEventCommon(mouseEvent);

        break;
    }
    case QEvent::Wheel: {
        d->debugEvent<QWheelEvent, false>(event);
        QWheelEvent *wheelEvent = static_cast<QWheelEvent*>(event);
        KisSingleActionShortcut::WheelAction action;

        /**
         * Ignore delta 0 events on OSX, since they are triggered by tablet
         * proximity when using Wacom devices.
         */
#ifdef Q_OS_OSX
        if(wheelEvent->delta() == 0) {
            retval = true;
            break;
        }
#endif

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
        d->containsPointer = true;
        //Make sure the input actions know we are active.
        KisAbstractInputAction::setInputManager(this);

        stop_ignore_cursor_events();
        touch_stop_block_press_events();

        d->matcher.enterEvent();
        break;
    case QEvent::Leave:
        d->debugEvent<QEvent, false>(event);
        d->containsPointer = false;
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
            Q_FOREACH (Qt::Key key, mapper.queryExtendedModifiers()) {
                QKeyEvent kevent(QEvent::ShortcutOverride, key, modifiers);
                eventFilterImpl(&kevent);
            }
        }

        stop_ignore_cursor_events();
        break;

    case QEvent::FocusOut: {
        d->debugEvent<QEvent, false>(event);
        KisAbstractInputAction::setInputManager(this);

        QPointF currentLocalPos =
            canvas()->canvasWidget()->mapFromGlobal(QCursor::pos());

        d->matcher.lostFocusEvent(currentLocalPos);

        break;
    }
    case QEvent::TabletRelease: {
#ifdef Q_OS_OSX
        stop_ignore_cursor_events();
#endif
        // break_if_touch_blocked_press_events();
        d->debugEvent<QTabletEvent, false>(event);

        QTabletEvent *tabletEvent = static_cast<QTabletEvent*>(event);
        retval = d->matcher.buttonReleased(tabletEvent->button(), tabletEvent);
        retval = true;
        event->setAccepted(true);
        break;
    }


    case QEvent::TabletMove: {
        d->debugEvent<QTabletEvent, false>(event);

        QTabletEvent *tabletEvent = static_cast<QTabletEvent*>(event);
        retval = compressMoveEventCommon(tabletEvent);

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

    case QEvent::TabletPress: {
        d->debugEvent<QTabletEvent, false>(event);
        QTabletEvent *tabletEvent = static_cast<QTabletEvent*>(event);
        if (d->tryHidePopupPalette()) {
            retval = true;
        } else {
            //Make sure the input actions know we are active.
            KisAbstractInputAction::setInputManager(this);
            retval = d->matcher.buttonPressed(tabletEvent->button(), tabletEvent);
        }
        event->setAccepted(true);
        retval = true;
        start_ignore_cursor_events();
        //Reset signal compressor to prevent processing events before press late
        d->resetCompressor();
        d->eatOneMousePress();
        break;
    }

    case QEvent::TouchBegin:
        touch_start_block_press_events();
        touch_eat_one_mouse_press();
        if (d->tryHidePopupPalette()) {
            retval = true;
        } else {
            KisAbstractInputAction::setInputManager(this);
            retval = d->matcher.touchBeginEvent(static_cast<QTouchEvent*>(event));
            event->accept();
        }
        // d->resetSavedTabletEvent(event->type());
        break;
    case QEvent::TouchUpdate: {
        QTouchEvent *tevent = static_cast<QTouchEvent*>(event);
#ifdef Q_OS_OSX
        int count = 0;
        Q_FOREACH (const QTouchEvent::TouchPoint &point, tevent->touchPoints()) {
            if (point.state() != Qt::TouchPointReleased) {
                count++;
            }
        }

        if (count < 2 && tevent->touchPoints().length() > count) {
            touch_stop_block_press_events();
            d->saveTouchEvent(tevent);
            retval = d->matcher.touchEndEvent(tevent);
            delete d->lastTouchEvent;
            d->lastTouchEvent = 0;
        } else {
#endif
            touch_start_block_press_events();
            KisAbstractInputAction::setInputManager(this);
            retval = d->matcher.touchUpdateEvent(tevent);
#ifdef Q_OS_OSX
        }
#endif
        event->accept();

        // d->resetSavedTabletEvent(event->type());
        break;
    }
    case QEvent::TouchEnd:
        touch_stop_block_press_events();
        d->saveTouchEvent(static_cast<QTouchEvent*>(event));
        retval = d->matcher.touchEndEvent(static_cast<QTouchEvent*>(event));
        event->accept();
        // d->resetSavedTabletEvent(event->type());
        delete d->lastTouchEvent;
        d->lastTouchEvent = 0;
        break;
    default:
        break;
    }

    return !retval ? d->processUnhandledEvent(event) : true;
}


void KisInputManager::slotCompressedMoveEvent()
{
    if (d->compressedMoveEvent) {
        // touch_stop_block_press_events();

        (void) d->handleCompressedTabletEvent(d->compressedMoveEvent.data());
        d->compressedMoveEvent.reset();
        dbgKrita << "Compressed move event received.";
    } else {
        dbgKrita << "Unexpected empty move event";
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

QTouchEvent *KisInputManager::lastTouchEvent() const
{
    return d->lastTouchEvent;
}

void KisInputManager::slotToolChanged()
{
    KoToolManager *toolManager = KoToolManager::instance();
    KoToolBase *tool = toolManager->toolById(canvas(), toolManager->activeToolId());
    if (tool && tool->isInTextMode()) {
        d->forwardAllEventsToTool = true;
        d->matcher.suppressAllActions(true);
    } else {
        d->forwardAllEventsToTool = false;
        d->matcher.suppressAllActions(false);
    }

    d->maskSyntheticEvents(tool->maskSyntheticEvents());
}

QPointF KisInputManager::widgetToDocument(const QPointF& position)
{
    const QPointF half = QPointF(.5f, .5f);
    QPointF pixel = position + half;
    return d->canvas->coordinatesConverter()->widgetToDocument(pixel);
}

void KisInputManager::profileChanged()
{
    d->matcher.clearShortcuts();

    KisInputProfile *profile = KisInputProfileManager::instance()->currentProfile();
    if (profile) {
        const QList<KisShortcutConfiguration*> shortcuts = profile->allShortcuts();

        for (KisShortcutConfiguration * const shortcut : shortcuts) {
            dbgUI << "Adding shortcut" << shortcut->keys() << "for action" << shortcut->action()->name();
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
        dbgKrita << "No Input Profile Found: canvas interaction will be impossible";
    }
}
