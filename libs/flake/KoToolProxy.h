/* This file is part of the KDE project
 *
 * Copyright (c) 2006 Boudewijn Rempt <boud@valdyas.org>
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
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
#include "flake_export.h"

#include <QPainter>

class KAction;
class QAction;
class QMouseEvent;
class QKeyEvent;
class QWheelEvent;
class QTabletEvent;
class KoToolSelection;
class KoTool;
class KoCanvasBase;
class KoCanvasController;
class QInputMethodEvent;

/**
 * Simple proxy interface that provides a point d'appui for canvas
 * implementations to pass events to the current tool. For the paint
 * event it is possible that there more than one tool is asked to
 * paint their decorations because tools can be stacked.
 *
 * The implementator of KoToolProxy should be solely responsible
 * for knowing which tool is currently in the user's hands.
 */
class FLAKE_EXPORT KoToolProxy : public QObject
{
    Q_OBJECT
public:
    /**
     * Constructor
     * @param canvas Each canvas has 1 toolProxy. Pass the parent here.
     * @param parent a parent QObject for memory management purposes.
     */
    explicit KoToolProxy(KoCanvasBase *canvas, QObject *parent = 0);
    ~KoToolProxy();

    /// Forwarded to the current KoTool
    void paint(QPainter &painter, const KoViewConverter &converter);
    /// Forwarded to the current KoTool
    void repaintDecorations();
    /// Forwarded to the current KoTool
    void tabletEvent(QTabletEvent *event, const QPointF &point);
    /// Forwarded to the current KoTool
    void mousePressEvent(QMouseEvent *event, const QPointF &point);
    /// Forwarded to the current KoTool
    void mouseDoubleClickEvent(QMouseEvent *event, const QPointF &point);
    /// Forwarded to the current KoTool
    void mouseMoveEvent(QMouseEvent *event, const QPointF &point);
    /// Forwarded to the current KoTool
    void mouseReleaseEvent(QMouseEvent *event, const QPointF &point);
    /// Forwarded to the current KoTool
    void keyPressEvent(QKeyEvent *event);
    /// Forwarded to the current KoTool
    void keyReleaseEvent(QKeyEvent *event);
    /// Forwarded to the current KoTool
    void wheelEvent(QWheelEvent * event, const QPointF &point);
    /// Forwarded to the current KoTool
    QVariant inputMethodQuery(Qt::InputMethodQuery query, const KoViewConverter &converter) const;
    /// Forwarded to the current KoTool
    void inputMethodEvent(QInputMethodEvent *event);
    /// Forwarded to the current KoTool
    QList<QAction*> popupActionList() const;
    /// Forwarded to the current KoTool
    void deleteSelection();

    /**
     * Retrieves the entire collection of actions for the active tool
     * or an empty hash if there is no active tool yet.
     */
    QHash<QString, KAction*> actions() const;

    /**
     * Proxies for KoTool::selection()
     */
    KoToolSelection *selection();

    /// Forwarded to the current KoTool
    void cut();
    /// Forwarded to the current KoTool
    void copy() const;
    /// Forwarded to the current KoTool
    bool paste();
    /// Forwarded to the current KoTool
    QStringList supportedPasteMimeTypes() const;
    /// Set the new active tool.
    void setActiveTool(KoTool *tool);

signals:
    /**
     * A tool can have a selection that is copy-able, this signal is emitted when that status changes.
     * @param hasSelection is true when the tool holds selected data.
     */
    void selectionChanged(bool hasSelection);

    /**
     * Emitted every time a tool is changed.
     * @param toolId the id of the tool.
     * @see KoTool::toolId()
     */
    void toolChanged(const QString &toolId);

protected:
    friend class KoToolManager;
    /// the toolManager tells us which KoCanvasController this toolProxy is working for.
    void setCanvasController(KoCanvasController *controller);

private:
    Q_PRIVATE_SLOT(d, void timeout())
    Q_PRIVATE_SLOT(d, void selectionChanged(bool))

    class Private;
    Private * const d;
};


#endif // _KO_TOOL_PROXY_H_
