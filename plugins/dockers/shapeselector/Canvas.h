/*
 * Copyright (C) 2006-2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
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

#ifndef CANVAS_H
#define CANVAS_H

#include "ZoomHandler.h"
#include "ShapeSelector.h"

#include <KoCanvasBase.h>
#include <KoShapeControllerBase.h>

#include <QList>

class ShapeSelector;
class ItemStore;
class InteractionStrategy;
class KoShapeManager;
class QUndoCommand;
class QPointF;
class QAction;
class QMenu;

/**
 * The shape selector docker essentially embeds a flake canvas, this is the canvas.
 * The shape selector is meant to show any flake shape and what better way to do that
 * then to implement a flake canvas. Much like any KOffice application does.
 * This class is the widget that is the main canvas as shown by the docker.
 */
class Canvas : public QWidget, public KoCanvasBase
{
    Q_OBJECT
public:
    /// constructor
    explicit Canvas(ShapeSelector *parent, ItemStore *itemStore);
    /// implementing KoCanvasBase
    virtual void gridSize(qreal *horizontal, qreal *vertical) const;
    /// implementing KoCanvasBase
    virtual bool snapToGrid() const { return false; }
    /// implementing KoCanvasBase
    virtual void addCommand(QUndoCommand *command);
    /// implementing KoCanvasBase
    virtual KoShapeManager * shapeManager() const;
    /// implementing KoCanvasBase
    virtual void updateCanvas(const QRectF &rc);
    /// implementing KoCanvasBase
    virtual KoToolProxy *toolProxy() const { return 0; }
    /// implementing KoCanvasBase
    virtual const KoViewConverter * viewConverter() const { return &m_converter; }
    /// implementing KoCanvasBase
    virtual QWidget *canvasWidget();
    /// implementing KoCanvasBase
    virtual KoUnit unit() const { return KoUnit(KoUnit::Millimeter); }
    /// implementing KoCanvasBase
    virtual void updateInputMethodInfo() {}
    /// implementing KoCanvasBase
    virtual const QWidget* canvasWidget() const { return 0; }
    void setCursor(const QCursor &cursor) {
        QWidget::setCursor(cursor);
    }
    ItemStore *itemStore() const { return m_parent->itemStore(); }

    /**
     * zooms in to a higher magnification level
     */
    void zoomIn(const QPointF &center);
    /// zooms out to a lower magnification level
    void zoomOut(const QPointF &center);
    /**
     * returns the current zoom level index.
     * The index is a simple way to map zoom levels to certain steps of zooming.
     * Zoom index 1 equals 100% zoom, 2 is 200% and 3 is 400%, while index of 0 equals 50%, -1: 25% etc.
     */
    int zoomIndex();

    QAction *popup(QMenu *menu, const QPointF &docCoordinate);

    /// used to pan around the canvas
    void moveDocumentOffset(const QPointF &offset);
    /// reset to show the original position of the shapeSelector document
    void resetDocumentOffset();

signals:
    void resized(const QSize &newSize);

private slots:
    void loadShapeTypes();
    void focusChanged(QWidget *old, QWidget *now);

protected: // event handlers
    virtual void mouseMoveEvent(QMouseEvent *e);
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void mouseReleaseEvent(QMouseEvent *e);
    virtual void tabletEvent(QTabletEvent *e);
    virtual void paintEvent(QPaintEvent * e);
    virtual void dragEnterEvent(QDragEnterEvent *e);
    virtual void dropEvent(QDropEvent *e);
    virtual bool event(QEvent *e);
    virtual void resizeEvent(QResizeEvent *e);
    virtual void keyPressEvent( QKeyEvent *e );
    virtual void keyReleaseEvent(QKeyEvent *e);

private:
    ZoomHandler m_converter;
    ShapeSelector *m_parent;
    InteractionStrategy *m_currentStrategy;
    QPointF m_lastPoint, m_displayOffset;
    int m_zoomIndex;
    QWidget *m_previousFocusOwner;
};

#endif
