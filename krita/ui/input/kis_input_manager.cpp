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

#define start_ignore_cursor_events() d->ignoreQtCursorEvents = true
#define stop_ignore_cursor_events() d->ignoreQtCursorEvents = false
#define break_if_should_ignore_cursor_events() if (d->ignoreQtCursorEvents) break;

#define push_and_stop_ignore_cursor_events() bool __saved_ignore_events = d->ignoreQtCursorEvents; d->ignoreQtCursorEvents = false
#define pop_ignore_cursor_events() d->ignoreQtCursorEvents = __saved_ignore_events

// Note: this is placeholder logic!
#define touch_stop_block_press_events() d->blockMouseEvents()
#define touch_start_block_press_events() d->blockMouseEvents()
#define break_if_touch_blocked_press_events() if (d->touchHasBlockedPressEvents) break;

KisInputManager::KisInputManager(QObject *parent)
    : QObject(parent), d(new Private(this))
{
    connect(KoToolManager::instance(), SIGNAL(changedTool(KoCanvasController*,int)),
            SLOT(slotToolChanged()));
    connect(&d->moveEventCompressor, SIGNAL(timeout()), SLOT(slotCompressedMoveEvent()));


#ifndef Q_OS_MAC
    QApplication::instance()->
        installEventFilter(new Private::ProximityNotifier(d, this));
#endif
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

void KisInputManager::slotFocusOnEnter(bool value)
{
    if (d->focusOnEnter == value) {
        return;
    }

    d->focusOnEnter = value;

    if (d->focusOnEnter && d->containsPointer) {
        if (d->canvas) {
            d->canvas->canvasWidget()->setFocus();
        }
    }
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

    // TODO: Handle touch events correctly.
    switch (event->type()) {
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonDblClick: {
        d->debugEvent<QMouseEvent, true>(event);
        break_if_should_ignore_cursor_events();
        // break_if_touch_blocked_press_events();

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
        // break_if_touch_blocked_press_events();

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

        if (!d->matcher.pointerMoved(event)) {
            //Update the current tool so things like the brush outline gets updated.
            d->toolProxy->forwardMouseHoverEvent(static_cast<QMouseEvent*>(event), lastTabletEvent());
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
        d->containsPointer = true;
        //Make sure the input actions know we are active.
        KisAbstractInputAction::setInputManager(this);
        //Ensure we have focus so we get key events.
        if (d->focusOnEnter) {
            d->canvas->canvasWidget()->setFocus();
        }
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
        // break_if_touch_blocked_press_events();


        /**
         * The flow of tablet events means the tablet is in the
         * proximity area, so activate it even when the
         * TabletEnterProximity event was missed (may happen when
         * changing focus of the window with tablet in the proximity
         * area)
         */
        QTabletEvent* tabletEvent = static_cast<QTabletEvent*>(event);
        d->saveTabletEvent(tabletEvent);
        event->ignore();
        start_ignore_cursor_events();
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


void KisInputManager::slotCompressedMoveEvent()
{
    if (d->compressedMoveEvent) {

        push_and_stop_ignore_cursor_events();
        // touch_stop_block_press_events();

        (void) d->handleCompressedTabletEvent(d->eventsReceiver, d->compressedMoveEvent.data());

        pop_ignore_cursor_events();

        d->compressedMoveEvent.reset();
        qDebug() << "Compressed move event received.";
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
        dbgKrita << "No Input Profile Found: canvas interaction will be impossible";
    }
}
