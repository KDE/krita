/* This file is part of the KDE project
 *
 * Copyright (c) 2006 Boudewijn Rempt <boud@valdyas.org>
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
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
#ifndef _KO_TOOL_PROXY_H_
#define _KO_TOOL_PROXY_H_

#include <KoViewConverter.h>
#include <flake_export.h>

#include <QPainter>

class QPaintEvent;
class QMouseEvent;
class QKeyEvent;
class QWheelEvent;
class QTabletEvent;
class KoToolSelection;
class KoTool;
class KoCanvasBase;

/**
 * Simple proxy interface that provides a point d'appui for canvas
 * implementations to pass events to the current tool. For the paint
 * event it is possible that there more than one tool is asked to
 * paint their decorations because tools can be stacked.
 *
 * The implementator of KoToolProxy should be solely responsible
 * for knowing which tool is currently in the user's hands.
 */
class FLAKE_EXPORT KoToolProxy {

public:
    explicit KoToolProxy(KoCanvasBase *canvas);
    ~KoToolProxy();

    /**
     * Call this when the tools in the current tool stack need to
     * repaint their decorations
     */
    void paint( QPainter &painter, KoViewConverter &converter );
    void repaintDecorations();
    void tabletEvent( QTabletEvent *event, const QPointF &point );
    void mousePressEvent( QMouseEvent *event, const QPointF &point  );
    void mouseDoubleClickEvent( QMouseEvent *event, const QPointF &point  );
    void mouseMoveEvent( QMouseEvent *event, const QPointF &point  );
    void mouseReleaseEvent( QMouseEvent *event, const QPointF &point  );
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);
    void wheelEvent ( QWheelEvent * event, const QPointF &point  );
    void setActiveTool(KoTool *tool);

    /**
     * Call this to get data _from_ the tool, instead of events _to_ the tool,
     * namely what the tool thinks is the current selection on the data contained
     * inside the tool.
     */
    KoToolSelection* selection();

private:
    class Private;
    Private * const d;
};


#endif // _KO_TOOL_PROXY_H_
