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


#include <QList>
#include <QPointer>
#include <QSet>
#include <QEvent>
#include <QTouchEvent>
#include <QScopedPointer>

#include "kis_input_manager.h"
#include "kis_shortcut_matcher.h"
#include "kis_tool_invocation_action.h"
#include "kis_alternate_invocation_action.h"
#include "kis_shortcut_configuration.h"
#include "kis_canvas2.h"
#include "kis_tool_proxy.h"
#include "kis_signal_compressor.h"
#include "input/kis_tablet_debugger.h"

#include "kis_abstract_input_action.h"


class KisInputManager::Private
{
public:
    Private(KisInputManager *qq);
    ~Private();
    bool tryHidePopupPalette();
    void addStrokeShortcut(KisAbstractInputAction* action, int index, const QList< Qt::Key >& modifiers, Qt::MouseButtons buttons);
    void addKeyShortcut(KisAbstractInputAction* action, int index,const QList<Qt::Key> &keys);
    void addTouchShortcut( KisAbstractInputAction* action, int index, KisShortcutConfiguration::GestureAction gesture );
    void addWheelShortcut(KisAbstractInputAction* action, int index, const QList< Qt::Key >& modifiers, KisShortcutConfiguration::MouseWheelMovement wheelAction);
    bool processUnhandledEvent(QEvent *event);
    Qt::Key workaroundShiftAltMetaHell(const QKeyEvent *keyEvent);
    void setupActions();
    void saveTouchEvent( QTouchEvent* event );
    bool handleCompressedTabletEvent(QObject *object, QTabletEvent *tevent);

    KisInputManager *q;

    KisCanvas2 *canvas;
    KisToolProxy *toolProxy;

    bool forwardAllEventsToTool;
    bool ignoreQtCursorEvents();

    bool disableTouchOnCanvas;
    bool touchHasBlockedPressEvents;

    KisShortcutMatcher matcher;
    QTouchEvent *lastTouchEvent;

    KisToolInvocationAction *defaultInputAction;

    QObject *eventsReceiver;
    KisSignalCompressor moveEventCompressor;
    QScopedPointer<QTabletEvent> compressedMoveEvent;
    bool testingAcceptCompressedTabletEvents;
    bool testingCompressBrushEvents;


    QSet<QPointer<QObject> > priorityEventFilter;

    void blockMouseEvents();
    void allowMouseEvents();

    template <class Event, bool useBlocking>
    void debugEvent(QEvent *event)
    {
      if (!KisTabletDebugger::instance()->debugEnabled()) return;
      QString msg1 = useBlocking && ignoreQtCursorEvents() ? "[BLOCKED] " : "[       ]";
      Event *specificEvent = static_cast<Event*>(event);
      dbgInput << KisTabletDebugger::instance()->eventToString(*specificEvent, msg1);
    }

    class ProximityNotifier : public QObject
    {
    public:
        ProximityNotifier(Private *_d, QObject *p);
        bool eventFilter(QObject* object, QEvent* event );
    private:
        KisInputManager::Private *d;
    };

    class CanvasSwitcher : public QObject
    {
    public:
        CanvasSwitcher(Private *_d, QObject *p);
        void addCanvas(KisCanvas2 *canvas);
        void removeCanvas(KisCanvas2 *canvas);
        bool eventFilter(QObject* object, QEvent* event );

    private:
        KisInputManager::Private *d;
        QMap<QObject*, KisCanvas2*> canvasResolver;
        int eatOneMouseStroke;
    };
    QPointer<CanvasSwitcher> canvasSwitcher;

    bool focusOnEnter;
    bool containsPointer;
};
