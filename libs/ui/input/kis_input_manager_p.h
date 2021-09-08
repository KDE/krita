/*
 *  SPDX-FileCopyrightText: 2015 Michael Abrahams <miabraha@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */


#include <QList>
#include <QPointer>
#include <QSet>
#include <QEvent>
#include <QTouchEvent>
#include <QScopedPointer>
#include <QQueue>
#include <QElapsedTimer>

#include "kis_input_manager.h"
#include "kis_shortcut_matcher.h"
#include "kis_shortcut_configuration.h"
#include "kis_canvas2.h"
#include "kis_tool_proxy.h"
#include "kis_signal_compressor.h"
#include "input/kis_tablet_debugger.h"
#include "kis_timed_signal_threshold.h"
#include "kis_signal_auto_connection.h"
#include "kis_latency_tracker.h"

class KisToolInvocationAction;


class KisInputManager::Private
{
public:
    Private(KisInputManager *qq);
    void addStrokeShortcut(KisAbstractInputAction* action, int index, const QList< Qt::Key >& modifiers, Qt::MouseButtons buttons);
    void addKeyShortcut(KisAbstractInputAction* action, int index,const QList<Qt::Key> &keys);
    void addTouchShortcut( KisAbstractInputAction* action, int index, KisShortcutConfiguration::GestureAction gesture );
    bool addNativeGestureShortcut( KisAbstractInputAction* action, int index, KisShortcutConfiguration::GestureAction gesture );
    void addWheelShortcut(KisAbstractInputAction* action, int index, const QList< Qt::Key >& modifiers, KisShortcutConfiguration::MouseWheelMovement wheelAction);
    bool processUnhandledEvent(QEvent *event);
    void setupActions();
    bool handleCompressedTabletEvent(QEvent *event);
    void fixShortcutMatcherModifiersState();

    KisInputManager *q;

    QPointer<KisCanvas2> canvas;
    QPointer<KisToolProxy> toolProxy;

    bool forwardAllEventsToTool = false;
    bool ignoringQtCursorEvents();

    bool touchHasBlockedPressEvents = false;

    KisShortcutMatcher matcher;

    KisToolInvocationAction *defaultInputAction = 0;

    QObject *eventsReceiver = 0;
    KisSignalCompressor moveEventCompressor;
    QScopedPointer<QEvent> compressedMoveEvent;
    bool testingAcceptCompressedTabletEvents = false;
    bool testingCompressBrushEvents = false;

    typedef QPair<int, QPointer<QObject> > PriorityPair;
    typedef QList<PriorityPair> PriorityList;
    PriorityList priorityEventFilter;
    int priorityEventFilterSeqNo;

    bool touchStrokeStarted = false;

    QPointF previousPos;
    bool buttonPressed = false;

    KisPopupWidgetInterface* popupWidget;

    void blockMouseEvents();
    void allowMouseEvents();
    void eatOneMousePress();
    void setMaskSyntheticEvents(bool value);
    void resetCompressor();
    void startBlockingTouch();
    void stopBlockingTouch();

    template <class Event, bool useBlocking>
    void debugEvent(QEvent *event)
    {
      if (!KisTabletDebugger::instance()->debugEnabled()) return;

      QString msg1 = useBlocking && ignoringQtCursorEvents() ? "[BLOCKED] " : "[       ]";
      Event *specificEvent = static_cast<Event*>(event);
      dbgTablet << KisTabletDebugger::instance()->eventToString(*specificEvent, msg1);
    }

    class ProximityNotifier : public QObject
    {
    public:
        ProximityNotifier(Private *_d, QObject *p);
        bool eventFilter(QObject* object, QEvent* event ) override;
    private:
        KisInputManager::Private *d;
    };

    class CanvasSwitcher : public QObject
    {
    public:
        CanvasSwitcher(Private *_d, QObject *p);
        void addCanvas(KisCanvas2 *canvas);
        void removeCanvas(KisCanvas2 *canvas);
        bool eventFilter(QObject* object, QEvent* event ) override;

    private:
        void setupFocusThreshold(QObject *object);

    private:
        KisInputManager::Private *d;
        QMap<QObject*, QPointer<KisCanvas2>> canvasResolver;
        int eatOneMouseStroke;
        KisTimedSignalThreshold focusSwitchThreshold;
        KisSignalAutoConnectionsStore thresholdConnections;
    };
    CanvasSwitcher canvasSwitcher;

    struct EventEater
    {
        EventEater();

        bool eventFilter(QObject* target, QEvent* event);

        // This should be called after we're certain a tablet stroke has started.
        void activate();
        // This should be called after a tablet stroke has ended.
        void deactivate();

        // On Windows, we sometimes receive mouse events very late, so watch & wait.
        void eatOneMousePress();

        // This should be called after the tablet is pressed,
        void startBlockingTouch();
        // This should be called after the tablet is released.
        void stopBlockingTouch();

        bool hungry{false};   // Continue eating mouse strokes
        bool peckish{false};  // Eat a single mouse press event
        bool eatSyntheticEvents{false}; // Mask all synthetic events
        bool activateSecondaryButtonsWorkaround{false}; // Use mouse events for right- and middle-clicks
        bool eatTouchEvents{false}; // Eat touch interactions
    };
    EventEater eventEater;

    bool containsPointer = false;

    int accumulatedScrollDelta = 0;

    class TabletLatencyTracker : public KisLatencyTracker {
    protected:
        virtual qint64 currentTimestamp() const override;
        virtual void print(const QString &message) override;
    };

    KisSharedPtr<TabletLatencyTracker> tabletLatencyTracker;
};
