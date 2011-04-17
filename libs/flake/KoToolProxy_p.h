/* This file is part of the KDE project
 * Copyright (C) 2006-2010 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KOTOOLPROXYPRIVATE_P
#define KOTOOLPROXYPRIVATE_P

#include <QTimer>
#include <QPointF>

class KoPointerEvent;
class KoToolBase;
class KoCanvasController;
class KoToolProxy;

class KoToolProxyPrivate
{
public:
    KoToolProxyPrivate(KoToolProxy *p);

    void timeout(); // Auto scroll the canvas

    void checkAutoScroll(const KoPointerEvent &event);

    void selectionChanged(bool newSelection);

    bool isActiveLayerEditable();

    /// the toolManager tells us which KoCanvasController this toolProxy is working for.
    void setCanvasController(KoCanvasController *controller);

    KoToolBase *activeTool;
    bool tabletPressed;
    bool hasSelection;
    QTimer scrollTimer;
    QPoint scrollEdgePoint;
    KoCanvasController *controller;
    KoToolProxy *parent;

    // used to determine if the mouse-release is after a drag or a simple click
    QPoint mouseDownPoint;

    // up until at least 4.3.0 we get a mouse move event when the tablet leaves the canvas.
    bool mouseLeaveWorkaround;
};

#endif
