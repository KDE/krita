/* This file is part of the KDE project
   Copyright (C) 2006-2007 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2010 Boudewijn Rempt <boud@valdyas.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KOPACANVASITEM_H
#define KOPACANVASITEM_H

#include <QGraphicsWidget>
#include <QList>
#include <KoPACanvasBase.h>

#include "kopageapp_export.h"

/// GraphicsWidget that shows a KoPAPage
class KOPAGEAPP_EXPORT KoPACanvasItem : public QGraphicsWidget, public KoPACanvasBase
{
    Q_OBJECT
public:
    explicit KoPACanvasItem( KoPADocument * doc );

    void repaint();

    QCursor setCursor(const QCursor &cursor);

    QWidget* canvasWidget() { return 0; }
    const QWidget* canvasWidget() const { return 0; }

    QGraphicsWidget *canvasItem() { return this; }
    const QGraphicsWidget *canvasItem() const{ return this; }

    /// reimplemented method
    virtual void updateCanvas( const QRectF& rc );

    /// reimplemented method
    virtual void updateInputMethodInfo();

    /// Recalculates the size of the canvas (needed when zooming or changing pagelayout)
    void updateSize();

public slots:

    void slotSetDocumentOffset(const QPoint &offset) { setDocumentOffset(offset); }

signals:

    void documentSize(const QSize &size);

    /**
     * Emitted when the entire controller size changes
     * @param size the size in widget pixels.
     */
    void sizeChanged( const QSize & size );

    /// Emitted when updateCanvas has been called.
    void canvasUpdated();

protected:
    /// reimplemented method from superclass
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    /// reimplemented method from superclass
    void mousePressEvent( QGraphicsSceneMouseEvent *event );
    /// reimplemented method from superclass
    void mouseDoubleClickEvent( QGraphicsSceneMouseEvent *event );
    /// reimplemented method from superclass
    void mouseMoveEvent( QGraphicsSceneMouseEvent *event );
    /// reimplemented method from superclass
    void mouseReleaseEvent( QGraphicsSceneMouseEvent *event );
    /// reimplemented method from superclass
    void keyPressEvent( QKeyEvent *event );
    /// reimplemented method from superclass
    void keyReleaseEvent( QKeyEvent *event );
    /// reimplemented method from superclass
    void wheelEvent ( QGraphicsSceneWheelEvent * event );
    /// reimplemented method from superclass
    void closeEvent( QCloseEvent * event );
    /// reimplemented method from superclass
    virtual QVariant inputMethodQuery(Qt::InputMethodQuery query) const;
    /// reimplemented method from superclass
    virtual void inputMethodEvent(QInputMethodEvent *event);

    /// reimplemented method from superclass
    virtual void resizeEvent( QGraphicsSceneResizeEvent * event );

    /**
     * Shows the default context menu
     * @param globalPos global position to show the menu at.
     * @param actionList action list to be inserted into the menu
     */
    void showContextMenu( const QPoint& globalPos, const QList<QAction*>& actionList );
};

#endif /* KOPACANVAS_H */
