/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006-2007 Thomas Zander <zander@kde.org>
 * SPDX-FileCopyrightText: 2007-2010 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KOCANVASCONTROLLERWIDGETVIEWPORT_P_H
#define KOCANVASCONTROLLERWIDGETVIEWPORT_P_H

#include <QWidget>
#include <QSize>
#include <QPoint>

class KoCanvasControllerWidget;
class KoShape;

class Viewport : public QWidget
{
    Q_OBJECT

public:

    explicit Viewport(KoCanvasControllerWidget *parent);
    ~Viewport() override {}

    void setCanvas(QWidget *canvas);
    QWidget *canvas() const {
        return m_canvas;
    }
    void setDocumentSize(const QSizeF &size);

public Q_SLOTS:
    void documentOffsetMoved(const QPoint &);

Q_SIGNALS:
    void sizeChanged();

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

    QWidget *m_canvas;
    QSizeF m_documentSize; // Size in pixels of the document
    QPoint m_documentOffset; // Place where the canvas widget should
    int m_margin; // The viewport margin around the document
};

#endif
