/*
 *  SPDX-FileCopyrightText: 2015 Michael Abrahams <miabraha@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */


#include <QList>
#include <QPointer>
#include <QEvent>
#include <QScopedPointer>

#include "KisInputEventsEater.h"
#include "kis_input_manager.h"
#include "kis_shortcut_matcher.h"
#include "kis_shortcut_configuration.h"
#include "kis_canvas2.h"
#include "kis_tool_proxy.h"
#include "kis_signal_compressor.h"
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

    KisInputEventsEater eventEater;

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
