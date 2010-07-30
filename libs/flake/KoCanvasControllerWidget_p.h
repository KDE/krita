/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef KOCANVASCONTROLLERWIDGET_P_H
#define KOCANVASCONTROLLERWIDGET_P_H

#include "KoCanvasControllerWidget.h"

#include <QWidget>

class Viewport;

class KoCanvasControllerWidget::Private
{
public:
    Private(KoCanvasControllerWidget *qq);

    /**
     * Gets called by the tool manager if this canvas controller is the current active canvas controller.
     */
    void setDocumentOffset();

    void resetScrollBars();
    void emitPointerPositionChangedSignals(QEvent *event);

    void activate();

    KoCanvasControllerWidget *q;
    KoCanvasBase * canvas;
    CanvasMode canvasMode;
    int margin; // The viewport margin around the document // TODO can we remove this one? The viewport has a copy...
    QSize documentSize;
    QPoint documentOffset;
    Viewport * viewportWidget;
    qreal preferredCenterFractionX;
    qreal preferredCenterFractionY;
    bool ignoreScrollSignals;
};

class Viewport : public QWidget
{
    Q_OBJECT

public:

    Viewport(KoCanvasControllerWidget *parent);
    ~Viewport() {}

    void setCanvas(QWidget *canvas);
    QWidget *canvas() {
        return m_canvas;
    }
    void setDocumentSize(const QSize &size);

    /**
     * When true, a shadow is drawn around the canvas widet.
     */
    void setDrawShadow(bool drawShadow);

public slots:
    void documentOffsetMoved(const QPoint &);

public:

    void handleDragEnterEvent(QDragEnterEvent *event);
    void handleDropEvent(QDropEvent *event);
    void handleDragMoveEvent(QDragMoveEvent *event);
    void handleDragLeaveEvent(QDragLeaveEvent *event);
    void handlePaintEvent(QPainter &gc, QPaintEvent *event);
    void setMargin(int margin) { m_margin = margin; resetLayout(); }

private:

    QPointF correctPosition(const QPoint &point) const;
    void repaint(KoShape *shape);

    /**
       Decides whether the containing canvas widget should be as
       big as the viewport (i.e., no margins are visible) or whether
       there are margins to be left blank, and then places the canvas
       widget accordingly.
    */
    void resetLayout();

private:

    KoCanvasControllerWidget *m_parent;
    KoShape *m_draggedShape;

    bool m_drawShadow;
    QWidget *m_canvas;
    QSize m_documentSize; // Size in pixels of the document
    QPoint m_documentOffset; // Place where the canvas widget should
    int m_margin; // The viewport margin around the document
};

#endif
