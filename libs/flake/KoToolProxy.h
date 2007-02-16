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
    /**
     * Constructor
     * @param canvas Each canvas has 1 toolProxy. Pass the parent here.
     */
    explicit KoToolProxy(KoCanvasBase *canvas);
    ~KoToolProxy();

    /// Forwarded to the current KoTool
    void paint( QPainter &painter, KoViewConverter &converter );
    /// Forwarded to the current KoTool
    void repaintDecorations();
    /// Forwarded to the current KoTool
    void tabletEvent( QTabletEvent *event, const QPointF &point );
    /// Forwarded to the current KoTool
    void mousePressEvent( QMouseEvent *event, const QPointF &point  );
    /// Forwarded to the current KoTool
    void mouseDoubleClickEvent( QMouseEvent *event, const QPointF &point  );
    /// Forwarded to the current KoTool
    void mouseMoveEvent( QMouseEvent *event, const QPointF &point  );
    /// Forwarded to the current KoTool
    void mouseReleaseEvent( QMouseEvent *event, const QPointF &point  );
    /// Forwarded to the current KoTool
    void keyPressEvent(QKeyEvent *event);
    /// Forwarded to the current KoTool
    void keyReleaseEvent(QKeyEvent *event);
    /// Forwarded to the current KoTool
    void wheelEvent ( QWheelEvent * event, const QPointF &point  );
    /// Set the new active tool.
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
