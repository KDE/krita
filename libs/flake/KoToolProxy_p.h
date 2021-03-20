/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006-2010 Thomas Zander <zander@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KOTOOLPROXYPRIVATE_P
#define KOTOOLPROXYPRIVATE_P

#include <QTimer>
#include <QElapsedTimer>
#include <QPointF>

class KoPointerEvent;
class KoToolBase;
class KoCanvasController;
class KoToolProxy;

class KoToolProxyPrivate
{
public:
    explicit KoToolProxyPrivate(KoToolProxy *p);

    void timeout(); // Auto scroll the canvas

    void checkAutoScroll(const KoPointerEvent &event);

    void selectionChanged(bool newSelection);

    bool isActiveLayerEditable();

    /// the toolManager tells us which KoCanvasController this toolProxy is working for.
    void setCanvasController(KoCanvasController *controller);

    KoToolBase *activeTool {0};
    bool tabletPressed {false};
    bool hasSelection {false};
    QTimer scrollTimer;
    QPoint widgetScrollPoint;
    KoCanvasController *controller {0};
    KoToolProxy *parent {0};

    // used to determine if the mouse-release is after a drag or a simple click
    QPoint mouseDownPoint;

    // up until at least 4.3.0 we get a mouse move event when the tablet leaves the canvas.
    bool mouseLeaveWorkaround {false};

    bool isToolPressed {false};

    // for multi clicking (double click or triple click) we need the following
    int multiClickCount {0};
    QPointF multiClickGlobalPoint;
    QElapsedTimer multiClickTimeStamp;
};

#endif
