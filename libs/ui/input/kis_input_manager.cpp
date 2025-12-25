/* This file is part of the KDE project
 *
 *  SPDX-FileCopyrightText: 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 *  SPDX-FileCopyrightText: 2015 Michael Abrahams <miabraha@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_input_manager.h"

#include <kis_debug.h>
#include <QQueue>
#include <klocalizedstring.h>
#include <QApplication>
#include <QTouchEvent>
#include <QElapsedTimer>
#include <QWidget>

#include <KoToolManager.h>
#include <KoPointerEvent.h>

#include "kis_tool_proxy.h"

#include <kis_config.h>
#include <kis_config_notifier.h>
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
#include "KisPopupWidgetAction.h"
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
#include "kis_algebra_2d.h"

template <typename T>
uint qHash(QPointer<T> value) {
    return reinterpret_cast<quintptr>(value.data());
}

KisInputManager::KisInputManager(QObject *parent)
    : QObject(parent), d(new Private(this))
{
    d->setupActions();

    connect(KisConfigNotifier::instance(), SIGNAL(configChanged()), SLOT(slotConfigChanged()));
    slotConfigChanged();

    connect(KoToolManager::instance(), SIGNAL(aboutToChangeTool(KoCanvasController*)), SLOT(slotAboutToChangeTool()));
    connect(KoToolManager::instance(), SIGNAL(changedTool(KoCanvasController*)), SLOT(slotToolChanged()));
    connect(&d->moveEventCompressor, SIGNAL(timeout()), SLOT(slotCompressedMoveEvent()));


    QApplication::instance()->
            installEventFilter(new Private::ProximityNotifier(d, this));

    // on macos global Monitor listen to keypresses when krita is not in focus
    // and local monitor listen presses when krita is in focus.
#ifdef Q_OS_MACOS
    KisExtendedModifiersMapper::setLocalMonitor(true, &d->matcher);
#endif
}

KisInputManager::~KisInputManager()
{
#ifdef Q_OS_MACOS
    KisExtendedModifiersMapper::setLocalMonitor(false);
#endif
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

void KisInputManager::registerPopupWidget(KisPopupWidgetInterface *popupWidget)
{
    d->popupWidget = popupWidget;

    // FUNKY!
    auto popupObject = dynamic_cast<QObject*>(d->popupWidget);
    KIS_ASSERT(popupObject);
    connect(popupObject, SIGNAL(finished()), this, SLOT(deregisterPopupWidget()));
}

void KisInputManager::deregisterPopupWidget()
{
    if (d->popupWidget->onScreen()) {
        d->popupWidget->dismiss();
    }

    // FUNKY!
    auto popupObject = dynamic_cast<QObject*>(d->popupWidget);
    KIS_ASSERT(popupObject);
    disconnect(popupObject, nullptr, this, nullptr); // Disconnect all.

    d->popupWidget = nullptr;
}

void KisInputManager::slotConfigChanged()
{
#ifdef Q_OS_WIN
    d->ignoreHighFunctionKeys = KisConfig(true).ignoreHighFunctionKeys();
    d->fixShortcutMatcherModifiersState();
#endif
}

void KisInputManager::slotTouchHoldTriggered()
{
    d->cancelTouchHoldTimer();
    d->clearBufferedTouchEvents();
    KIS_SAFE_ASSERT_RECOVER_RETURN(d->originatingTouchBeginEvent);
    if (d->matcher.touchHoldBeginEvent(static_cast<QTouchEvent *>(d->originatingTouchBeginEvent.data()))) {
        d->touchHasBlockedPressEvents = true;
    }
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
                      kismpl::mem_equal_to(&Private::PriorityPair::second, filter));

    if (it != end) return;

    it = std::find_if(begin, end,
                      kismpl::mem_greater(&Private::PriorityPair::first, priority));

    d->priorityEventFilter.insert(it, qMakePair(priority, filter));
    d->priorityEventFilterSeqNo++;
}

void KisInputManager::detachPriorityEventFilter(QObject *filter)
{
    Private::PriorityList::iterator it = d->priorityEventFilter.begin();
    Private::PriorityList::iterator end = d->priorityEventFilter.end();

    it = std::find_if(it, end,
                      kismpl::mem_equal_to(&Private::PriorityPair::second, filter));

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
        if (d->toolProxy) {
            d->toolProxy->processEvent(event);
        }
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
                  std::is_same<Event, QTabletEvent>::value ||
                  std::is_same<Event, QTouchEvent>::value,
                  "event should be a mouse or a tablet event");


    bool retval = false;

    /**
     * Compress the events if the tool doesn't need high resolution input
     */
    if ((event->type() == QEvent::MouseMove ||
         event->type() == QEvent::TabletMove ||
         event->type() == QEvent::TouchUpdate) &&
            (!d->matcher.supportsHiResInputEvents() ||
             d->testingCompressBrushEvents)) {

        KoPointerEvent::copyQtPointerEvent(event, d->compressedMoveEvent);
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

bool shouldResetWheelDelta(QEvent * event)
{
    return
        event->type() == QEvent::FocusIn ||
        event->type() == QEvent::FocusOut ||
        event->type() == QEvent::MouseButtonPress ||
        event->type() == QEvent::MouseButtonRelease ||
        event->type() == QEvent::MouseButtonDblClick ||
        event->type() == QEvent::TabletPress ||
        event->type() == QEvent::TabletRelease ||
        event->type() == QEvent::Enter ||
        event->type() == QEvent::Leave ||
        event->type() == QEvent::TouchBegin ||
        event->type() == QEvent::TouchEnd ||
        event->type() == QEvent::TouchCancel ||
        event->type() == QEvent::NativeGesture;

}

bool KisInputManager::eventFilterImpl(QEvent * event)
{
    bool retval = false;

    // Try closing any open popup widget and
    // consume input if possible.
    if (d->popupWidget) {
        QEvent::Type type = event->type();

        if (type == QEvent::MouseButtonPress
         || type == QEvent::MouseButtonDblClick
         || type == QEvent::TabletPress
         || type == QEvent::TouchBegin
         || type == QEvent::NativeGesture) {
            bool wasVisible = d->popupWidget->onScreen();
            deregisterPopupWidget();

            if (wasVisible) {
                d->popupWasActive = true;
                event->setAccepted(true);
                return true; // Event consumed.
            }
        }
    }

    if (shouldResetWheelDelta(event)) {
        d->accumulatedScrollDelta = 0;
    }

    if (event->type() == QEvent::MouseMove ||
        event->type() == QEvent::MouseButtonPress ||
        event->type() == QEvent::MouseButtonRelease ||
        event->type() == QEvent::TabletMove ||
        event->type() == QEvent::TabletPress ||
        event->type() == QEvent::TabletRelease ||
        event->type() == QEvent::Wheel) {

        /**
         * When Krita (as an application) has no input focus, we cannot
         * handle key events. But at the same time, when the user hovers
         * Krita canvas, we should still show him the correct cursor.
         *
         * So here we just add a simple workaround to resync shortcut
         * matcher's state at least against the basic modifiers, like
         * Shift, Control and Alt.
         */
        QWidget *receivingWidget = dynamic_cast<QWidget*>(d->eventsReceiver);
        if (receivingWidget && !receivingWidget->hasFocus()) {
            d->fixShortcutMatcherModifiersState();
        } else {
            /**
             * On Windows, when the user presses some global window manager shortcuts,
             * e.g. Alt+Space (to show window title menu), events for these key presses
             * and releases are not delivered (see bug 424319). This code is a workaround
             * for this problem. It checks consistency of standard modifiers and resets
             * shortcut's matcher state in case of a trouble.
             */
            QInputEvent *inputEvent = static_cast<QInputEvent*>(event);
            if (event->type() != QEvent::ShortcutOverride &&
                !d->matcher.sanityCheckModifiersCorrectness(inputEvent->modifiers())) {

                d->fixShortcutMatcherModifiersState();
            } else if (d->matcher.hasPolledKeys()) {
                /**
                 * Re-check the native platform key API against keys we are unsure about,
                 * and fix them in case they now show as released.
                 *
                 * The other part of the fix is placed in the handler of ShortcutOverride,
                 * because it needs a custom set of the presset keys.
                 */
                d->fixShortcutMatcherModifiersState();
            }
        }
    }

    switch (event->type()) {
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonDblClick: {
        d->debugEvent<QMouseEvent, true>(event);
        if (d->touchHasBlockedPressEvents) break;

        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);

        if (d->popupWidget) {
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
        if (d->touchHasBlockedPressEvents) break;

        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        retval = d->matcher.buttonReleased(mouseEvent->button(), mouseEvent);
        event->setAccepted(retval);
        break;
    }
    case QEvent::ShortcutOverride: {
        d->debugEvent<QKeyEvent, false>(event);
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);

        Qt::Key key = KisExtendedModifiersMapper::workaroundShiftAltMetaHell(keyEvent);

        /** See a comment in the handler of KeyRelease event for
         * shouldSynchronizeOnNextKeyPress explanation
         *
         * There is also a case when Krita gets focus via Win+1 key, then
         * the polled key '1' gets into the matcher, but OS does not deliver any
         * signals for it (see bug 451424)
         */
        if (d->shouldSynchronizeOnNextKeyPress || d->matcher.hasPolledKeys()) {
            QVector<Qt::Key> guessedKeys;
            KisExtendedModifiersMapper mapper;
            Qt::KeyboardModifiers modifiers = mapper.queryStandardModifiers();
            Q_FOREACH (Qt::Key key, mapper.queryExtendedModifiers()) {
                QKeyEvent kevent(QEvent::ShortcutOverride, key, modifiers);
                guessedKeys << KisExtendedModifiersMapper::workaroundShiftAltMetaHell(&kevent);
            }

            if (!d->matcher.debugPressedKeys().contains(key)) {
                guessedKeys.removeOne(key);
            }

            d->fixShortcutMatcherModifiersState(guessedKeys, modifiers);
            d->shouldSynchronizeOnNextKeyPress = false;
        }

        if (!keyEvent->isAutoRepeat()) {
            retval = d->matcher.keyPressed(key);
        } else {
            retval = d->matcher.autoRepeatedKeyPressed(key);
        }

        // In case we matched a shortcut we should accept the event to
        // notify Qt that it shouldn't try to trigger its partially matched
        // shortcuts.
        if (retval) {
            keyEvent->setAccepted(true);
        }

        break;
    }
    case QEvent::KeyRelease: {
        d->debugEvent<QKeyEvent, false>(event);
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);

        if (!keyEvent->isAutoRepeat()) {
            Qt::Key key = KisExtendedModifiersMapper::workaroundShiftAltMetaHell(keyEvent);
            retval = d->matcher.keyReleased(key);

            /**
             * On some systems Qt fails to generate a correct
             * sequence of events when the user releases a
             * modifier key while some other key is pressed.
             *
             * In such cases Qt doesn't understand that the key
             * has changed its name in the meantime and sends
             * incorrect key-release event for it (or an auto-
             * repeated key-release/key-press pair).
             *
             * Example (on en-US keyboard):
             *
             * 1) Press Shift (Key_Shift-press is delivered)
             * 2) Press '2' (Key_At-press is delivered)
             * 3) Release Shift (Key_Shift-release is delivered)
             * 4) Release '2' (Key_2-release is delivered,
             *                 which is unbalanced)
             *
             * The same issue happens with non-latin keyboards,
             * where Qt does auto-key-replace routines when
             * Control modifier is pressed.
             *
             * https://bugs.kde.org/show_bug.cgi?id=454256
             * https://bugreports.qt.io/browse/QTBUG-103868
             */

            if (d->useUnbalancedKeyPressEventWorkaround &&
                !d->matcher.debugPressedKeys().isEmpty() &&
                (key == Qt::Key_Shift || key == Qt::Key_Alt ||
                 key == Qt::Key_Control || key == Qt::Key_Meta)) {

                d->shouldSynchronizeOnNextKeyPress = true;
            }
        }
        break;
    }
    case QEvent::MouseMove: {
        d->debugEvent<QMouseEvent, true>(event);

        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        retval = compressMoveEventCommon(mouseEvent);

        break;
    }
    case QEvent::Wheel: {
        d->debugEvent<QWheelEvent, false>(event);
        QWheelEvent *wheelEvent = static_cast<QWheelEvent*>(event);

#ifdef Q_OS_MACOS
        // Some QT wheel events are actually touch pad pan events. From the QT docs:
        // "Wheel events are generated for both mouse wheels and trackpad scroll gestures."

        // We differentiate between touchpad events and real mouse wheels by inspecting the
        // event source.

        if (wheelEvent->source() == Qt::MouseEventSource::MouseEventSynthesizedBySystem) {
            KisAbstractInputAction::setInputManager(this);
            retval = d->matcher.wheelEvent(KisSingleActionShortcut::WheelTrackpad, wheelEvent);
            break;
        }
#endif

        d->accumulatedScrollDelta += wheelEvent->delta();
        KisSingleActionShortcut::WheelAction action;

        /**
         * Ignore delta 0 events on OSX, since they are triggered by tablet
         * proximity when using Wacom devices.
         */
#ifdef Q_OS_MACOS
        if(wheelEvent->delta() == 0) {
            retval = true;
            break;
        }
#endif

        if (wheelEvent->orientation() == Qt::Horizontal) {
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

        bool wasScrolled = false;

        while (qAbs(d->accumulatedScrollDelta) >= QWheelEvent::DefaultDeltasPerStep) {
            //Make sure the input actions know we are active.
            KisAbstractInputAction::setInputManager(this);
            retval = d->matcher.wheelEvent(action, wheelEvent);
            d->accumulatedScrollDelta -=
                KisAlgebra2D::signPZ(d->accumulatedScrollDelta) *
                QWheelEvent::DefaultDeltasPerStep;
            wasScrolled = true;
        }

        if (wasScrolled) {
            d->accumulatedScrollDelta = 0;
        }

        retval = !wasScrolled;
        break;
    }
#ifndef Q_OS_ANDROID
    case QEvent::Enter:
        d->debugEvent<QEvent, false>(event);
        //Make sure the input actions know we are active.
        KisAbstractInputAction::setInputManager(this);
        if (!d->containsPointer) {
            d->containsPointer = true;

            d->allowMouseEvents();
            d->touchHasBlockedPressEvents = false;
        }
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
        d->allowMouseEvents();
        d->touchHasBlockedPressEvents = false;

        d->matcher.leaveEvent();
        break;
#endif
    case QEvent::FocusIn:
        d->debugEvent<QEvent, false>(event);
        KisAbstractInputAction::setInputManager(this);

        d->fixShortcutMatcherModifiersState();
        d->matcher.reinitializeButtons();

        d->allowMouseEvents();
        break;

    case QEvent::FocusOut: {
        d->debugEvent<QEvent, false>(event);
        KisAbstractInputAction::setInputManager(this);

        QPointF currentLocalPos =
                canvas()->canvasWidget()->mapFromGlobal(QCursor::pos());

        d->matcher.lostFocusEvent(currentLocalPos);

        break;
    }
    case QEvent::TabletPress: {
        d->debugEvent<QTabletEvent, false>(event);
        QTabletEvent *tabletEvent = static_cast<QTabletEvent*>(event);

        {
            //Make sure the input actions know we are active.
            KisAbstractInputAction::setInputManager(this);
            retval = d->matcher.buttonPressed(tabletEvent->button(), tabletEvent);
            if (!d->containsPointer) {
                d->containsPointer = true;
                d->touchHasBlockedPressEvents = false;
            }
        }

        event->setAccepted(true);
        retval = true;
        d->blockMouseEvents();
        d->startBlockingTouch();
        //Reset signal compressor to prevent processing events before press late
        d->resetCompressor();


#if defined Q_OS_LINUX && !defined QT_HAS_ENTER_LEAVE_PATCH
        // remove this hack when this patch is integrated:
        // https://codereview.qt-project.org/#/c/255384/
        event->setAccepted(false);
        d->eatOneMousePress();
#elif defined Q_OS_WIN32
        /**
         * Windows is the only platform that synthesizes mouse events for
         * the tablet on OS-level, that is, even when we accept the event
         */
        d->eatOneMousePress();
#endif

        break;
    }
    case QEvent::TabletMove: {
        d->debugEvent<QTabletEvent, false>(event);

        QTabletEvent *tabletEvent = static_cast<QTabletEvent*>(event);
        retval = compressMoveEventCommon(tabletEvent);

        if (d->tabletLatencyTracker) {
            d->tabletLatencyTracker->push(tabletEvent->timestamp());
        }

        /**
         * The flow of tablet events means the tablet is in the
         * proximity area, so activate it even when the
         * TabletEnterProximity event was missed (may happen when
         * changing focus of the window with tablet in the proximity
         * area)
         */
        d->blockMouseEvents();

#if defined Q_OS_LINUX && !defined QT_HAS_ENTER_LEAVE_PATCH
        // remove this hack when this patch is integrated:
        // https://codereview.qt-project.org/#/c/255384/
        event->setAccepted(false);
#endif

        break;
    }
    case QEvent::TabletRelease: {
#if defined(Q_OS_MAC) || defined(Q_OS_ANDROID)
        d->allowMouseEvents();
#endif
        d->stopBlockingTouch();
        d->debugEvent<QTabletEvent, false>(event);

        QTabletEvent *tabletEvent = static_cast<QTabletEvent*>(event);
        retval = d->matcher.buttonReleased(tabletEvent->button(), tabletEvent);
        retval = true;
        event->setAccepted(true);

#if defined Q_OS_LINUX && !defined QT_HAS_ENTER_LEAVE_PATCH
        // remove this hack when this patch is integrated:
        // https://codereview.qt-project.org/#/c/255384/
        event->setAccepted(false);
#endif

        break;
    }

    case QEvent::TouchBegin:
    {
        d->debugEvent<QTouchEvent, false>(event);
        // The popup was dismissed in previous TouchBegin->TouchEnd sequence. We now have a new TouchBegin.
        d->popupWasActive = false;
        if (startTouch(retval)) {
            QTouchEvent *touchEvent = static_cast<QTouchEvent *>(event);
            KisAbstractInputAction::setInputManager(this);
            d->startingPos = touchEvent->touchPoints().at(0).pos();
            d->previousPos = d->startingPos;
            // we don't want to lose this event
            KoPointerEvent::copyQtPointerEvent(touchEvent, d->originatingTouchBeginEvent);

            // In the face of a touch and hold shortcut being present, we need
            // to disambiguate whether this is a touch being held or the user
            // is doing something else. For that purpose, we start buffering
            // received touch events until we can actually make a decision.
            d->clearBufferedTouchEvents();
            if (d->matcher.hasTouchHoldShortcut() && touchEvent->touchPoints().length() == 1) {
                d->bufferTouchEvent(touchEvent);
                d->restartTouchHoldTimer();
                retval = true;
            } else {
                d->cancelTouchHoldTimer();
                retval = handleTouchBegin(touchEvent);
            }

            KIS_SAFE_ASSERT_RECOVER(!d->touchStrokeStarted) {
                d->touchStrokeStarted = false;
            }
            d->resetCompressor();
            event->accept();
        }
        break;
    }

    case QEvent::TouchUpdate:
    {
        if (d->popupWasActive) {
            event->setAccepted(true);
            return true;
        }
        QTouchEvent *touchEvent = static_cast<QTouchEvent*>(event);
        d->debugEvent<QTouchEvent, false>(event);

#ifdef Q_OS_MAC
        int count = 0;
        Q_FOREACH (const QTouchEvent::TouchPoint &point, touchEvent->touchPoints()) {
            if (point.state() != Qt::TouchPointReleased) {
                count++;
            }
        }

        if (count < 2 && touchEvent->touchPoints().length() > count) {
            d->touchHasBlockedPressEvents = false;
            d->cancelTouchHoldTimer();
            retval = d->matcher.touchEndEvent(touchEvent);
        } else {
#endif
            // Touch hold shortcuts need to buffer events, see TouchBegin.
            if (touchHoldBufferUpdate(touchEvent)) {
                retval = true; // Event was buffered.
            } else {
                retval = handleTouchUpdate(touchEvent);
            }
#ifdef Q_OS_MACOS
        }
#endif
        // if the event isn't handled, Qt starts to send MouseEvents
        if (!KisConfig(true).disableTouchOnCanvas())
            retval = true;

        event->accept();
        break;
    }

    case QEvent::TouchEnd:
    {
        d->cancelTouchHoldTimer();
        d->flushBufferedTouchEvents();

        if (d->popupWasActive) {
            event->setAccepted(true);
            return true;
        }
        d->debugEvent<QTouchEvent, false>(event);
        QTouchEvent *touchEvent = static_cast<QTouchEvent*>(event);

        retval = d->matcher.touchEndEvent(touchEvent);
        if (d->touchStrokeStarted) {
            retval = d->matcher.buttonReleased(Qt::LeftButton, touchEvent);
            d->startingPos = {0, 0};
            d->previousPos = {0, 0};
            d->touchStrokeStarted = false; // stroke ended
        } else if (!KisConfig(true).disableTouchOnCanvas() && !d->touchHasBlockedPressEvents
                   && touchEvent->touchPoints().count() == 1) {
            // If no stroke has been started while touch painting is enabled,
            // the user tapped with one finger, but didn't make any motion that
            // caused us to start a stroke. We produce a press and release in
            // response so that the tool responds to their input.
            d->matcher.buttonPressed(Qt::LeftButton, d->originatingTouchBeginEvent.data());
            d->matcher.buttonReleased(Qt::LeftButton, touchEvent);
        }

        endTouch();
        d->allowMouseEvents();

        // if the event isn't handled, Qt starts to send MouseEvents
        if (!KisConfig(true).disableTouchOnCanvas())
            retval = true;

        event->accept();
        break;
    }
    case QEvent::TouchCancel:
    {
        d->cancelTouchHoldTimer();
        d->clearBufferedTouchEvents();

        if (d->popupWasActive) {
            event->setAccepted(true);
            return true;
        }
        d->debugEvent<QTouchEvent, false>(event);
        endTouch();
        d->allowMouseEvents();
        QTouchEvent *touchEvent = static_cast<QTouchEvent*>(event);
        d->matcher.touchCancelEvent(touchEvent, d->previousPos);
        // reset state
        d->startingPos = {0, 0};
        d->previousPos = {0, 0};
        d->touchStrokeStarted = false;
        retval = true;
        event->accept();
        break;
    }

    case QEvent::NativeGesture:
    {
        QNativeGestureEvent *gevent = static_cast<QNativeGestureEvent*>(event);
        switch (gevent->gestureType()) {
            case Qt::BeginNativeGesture:
            {
                if (startTouch(retval)) {
                    KisAbstractInputAction::setInputManager(this);
                    retval = d->matcher.nativeGestureBeginEvent(gevent);
                    event->accept();
                }
                break;
            }
            case Qt::EndNativeGesture:
            {
                endTouch();
                retval = d->matcher.nativeGestureEndEvent(gevent);
                event->accept();
                break;
            }
            default:
            {
                KisAbstractInputAction::setInputManager(this);
                retval = d->matcher.nativeGestureEvent(gevent);
                event->accept();
                break;
            }
        }
        break;
    }

    default:
        break;
    }

    return !retval ? d->processUnhandledEvent(event) : true;
}

bool KisInputManager::startTouch(bool &retval)
{
    Q_UNUSED(retval);

    // Touch rejection: if touch is disabled on canvas, no need to block mouse press events
    if (KisConfig(true).disableTouchOnCanvas()) {
        d->eatOneMousePress();
    }

    return true;
}

void KisInputManager::endTouch()
{
    d->touchHasBlockedPressEvents = false;
}

bool KisInputManager::touchHoldBufferUpdate(QTouchEvent *touchEvent)
{
    if (d->isPendingTouchHold()) {
        if (touchEvent->touchPoints().length() == 1 && d->isWithinTouchHoldSlopRange(touchEvent->touchPoints().at(0).pos())) {
            d->bufferTouchEvent(touchEvent);
            return true;
        } else {
            d->cancelTouchHoldTimer();
            d->flushBufferedTouchEvents();
        }
    }
    return false;
}

bool KisInputManager::handleTouchBegin(QTouchEvent *touchEvent)
{
    return d->matcher.touchBeginEvent(touchEvent);
}

bool KisInputManager::handleTouchUpdate(QTouchEvent *touchEvent)
{
    QPointF currentPos = touchEvent->touchPoints().at(0).pos();
    if (d->touchStrokeStarted
        || (!KisConfig(true).disableTouchOnCanvas() && !d->touchHasBlockedPressEvents
            && touchEvent->touchPoints().count() == 1 && touchEvent->touchPointStates() != Qt::TouchPointStationary
            && (qAbs(currentPos.x() - d->previousPos.x()) > 1 // stop wobbliness which Qt sends us
                || qAbs(currentPos.y() - d->previousPos.y()) > 1))) {
        d->previousPos = currentPos;
        if (!d->touchStrokeStarted) {
            // we start it here not in TouchBegin, because Qt::TouchPointStationary doesn't work with hpdi devices.
            bool retval = d->matcher.buttonPressed(Qt::LeftButton, d->originatingTouchBeginEvent.data());
            d->touchStrokeStarted = retval;
            return retval;
        } else {
            // if it is a full-fledged stroke, then ignore (currentPos.x - previousPos.x)
            bool retval = compressMoveEventCommon(touchEvent);
            d->blockMouseEvents();
            return retval;
        }
    } else {
        KisAbstractInputAction::setInputManager(this);
        bool retval = d->matcher.touchUpdateEvent(touchEvent);
        d->touchHasBlockedPressEvents = retval;
        return retval;
    }
}

void KisInputManager::slotCompressedMoveEvent()
{
    if (d->compressedMoveEvent) {
        // d->touchHasBlockedPressEvents = false;

        (void) d->handleCompressedTabletEvent(d->compressedMoveEvent.data());
        d->compressedMoveEvent.reset();
        //dbgInput << "Compressed move event received.";
    } else {
        //dbgInput << "Unexpected empty move event";
    }
}

KisCanvas2* KisInputManager::canvas() const
{
    return d->canvas;
}

QPointer<KisToolProxy> KisInputManager::toolProxy() const
{
    return d->toolProxy;
}

void KisInputManager::slotAboutToChangeTool()
{
    QPointF currentLocalPos;
    if (canvas() && canvas()->canvasWidget()) {
        currentLocalPos = canvas()->canvasWidget()->mapFromGlobal(QCursor::pos());
    }
    // once more, don't forget the global state whenever matcher may trigger a KisAbstractInputAction
    KisAbstractInputAction::setInputManager(this);

    d->matcher.lostFocusEvent(currentLocalPos);
}

void KisInputManager::slotToolChanged()
{
    if (!d->canvas) return;
    KoToolManager *toolManager = KoToolManager::instance();
    KoToolBase *tool = toolManager->toolById(canvas(), toolManager->activeToolId());
    if (tool) {
        // once more, don't forget the global state whenever matcher may trigger a KisAbstractInputAction
        KisAbstractInputAction::setInputManager(this);

        d->setMaskSyntheticEvents(tool->maskSyntheticEvents());
        if (tool->isInTextMode()) {
            d->forwardAllEventsToTool = true;
            d->matcher.suppressAllActions(true);
        } else {
            d->forwardAllEventsToTool = false;
            d->matcher.suppressAllActions(false);
        }

        d->matcher.suppressConflictingKeyActions(toolProxy()->toolPriorityShortcuts());
        d->matcher.toolHasBeenActivated();
    }
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
                if (!d->addNativeGestureShortcut(shortcut->action(), shortcut->mode(), shortcut->gesture())) {
                    d->addTouchShortcut(shortcut->action(), shortcut->mode(), shortcut->gesture());
                }
                break;
            default:
                break;
            }
        }
    }
    else {
        dbgInput << "No Input Profile Found: canvas interaction will be impossible";
    }
}
