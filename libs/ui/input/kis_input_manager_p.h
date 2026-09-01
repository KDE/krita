/*
 *  SPDX-FileCopyrightText: 2015 Michael Abrahams <miabraha@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */


#include <QList>
#include <QPointer>
#include <QEvent>
#include <QTouchEvent>
#include <QScopedPointer>
#include <QQueue>

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

enum EventBlockingReason {
    NotBlocked = 0x0,
    BlockedByTabletProximity = 0x01,
    BlockedByTabletHover = 0x02,
    BlockedByTabletPress = 0x04,
    BlockedByTouchPress = 0x08,
    BlockedByNextPressSuppression = 0x10,
    BlockedByButtonsWorkaround = 0x20,
    BlockedBySynthetic = 0x40
};
Q_DECLARE_FLAGS(EventBlockingReasons, EventBlockingReason)
Q_DECLARE_OPERATORS_FOR_FLAGS(EventBlockingReasons)

class KisInputManager::Private
{
public:
    Private(KisInputManager *qq);
    void addStrokeShortcut(KisAbstractInputAction* action, int index, const QList< Qt::Key >& modifiers, Qt::MouseButtons buttons);
    void addKeyShortcut(KisAbstractInputAction* action, int index,const QList<Qt::Key> &keys);
    void addTouchShortcut( KisAbstractInputAction* action, int index, KisShortcutConfiguration::TouchGestureAction gesture, bool isTouchPainting = false);
    bool addNativeGestureShortcut( KisAbstractInputAction* action, int index, KisShortcutConfiguration::NativeGestureAction gesture );
    void addWheelShortcut(KisAbstractInputAction* action, int index, const QList< Qt::Key >& modifiers, KisShortcutConfiguration::MouseWheelMovement wheelAction);
    bool processUnhandledEvent(QEvent *event);
    void setupActions();
    bool handleCompressedTabletEvent(QEvent *event);
    void fixShortcutMatcherModifiersState();
    void fixShortcutMatcherModifiersState(QVector<Qt::Key> newKeys, Qt::KeyboardModifiers modifiers);

    KisInputManager *q;

    QPointer<KisCanvas2> canvas;
    QPointer<KisToolProxy> toolProxy;

    bool forwardAllEventsToTool = false;

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

    bool popupWasActive = false;

    bool useUnbalancedKeyPressEventWorkaround = false;
    bool shouldSynchronizeOnNextKeyPress = false;

    KisPopupWidgetInterface *popupWidget;

    void setMaskSyntheticEvents(bool value);
    void resetCompressor();

    template <class Event>
    static void debugEvent(QEvent *event, EventBlockingReasons reasons = NotBlocked)
    {
        if (!KisTabletDebugger::instance()->debugEnabled()) return;

        QString reasonsString;

        if (reasons.testFlag(BlockedByTabletProximity)) {
            reasonsString += "Prx";
        }

        if (reasons.testFlag(BlockedByTabletHover)) {
            reasonsString += "Hov";
        }

        if (reasons.testFlag(BlockedByTabletPress)) {
            reasonsString += "Prs";
        }

        if (reasons.testFlag(BlockedByNextPressSuppression)) {
            reasonsString += "Nxt";
        }

        if (reasons.testFlag(BlockedByTouchPress)) {
            reasonsString += "Tch";
        }

        if (reasons.testFlag(BlockedBySynthetic)) {
            reasonsString += "Syn";
        }

        QString msg1 = QString("[%1] ").arg(reasonsString, 15);
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

        void notifyTabletEnterProximity();
        void notifyTabletLeaveProximity();

        // On Windows, we sometimes receive mouse events very late, so watch & wait.
        void eatOneMousePress();

        bool eatOneMousePressEvent{false};  // Eat a single mouse press event
        bool activateSecondaryButtonsWorkaround{false}; // Use mouse events for right- and middle-clicks

        bool tabletIsInProximity {false};
        bool tabletIsHovering {false};
        bool tabletIsPressed {false};
        bool touchIsActive {false};

        void debugEaterStateTransition(const QLatin1String &stateName, bool newValue, QEvent::Type eventType, const QLatin1String &comment = QLatin1String());
    };
    EventEater eventEater;

    int accumulatedScrollDelta = 0;

    class TabletLatencyTracker : public KisLatencyTracker {
    protected:
        virtual qint64 currentTimestamp() const override;
        virtual void print(const QString &message) override;
    };

    KisSharedPtr<TabletLatencyTracker> tabletLatencyTracker;
#ifdef Q_OS_WIN
    bool ignoreHighFunctionKeys = false;
#endif
};
