/* This file is part of the KDE project
 * Copyright (C) 2010 KO GmbH <boud@kogmbh.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (  at your option ) any later version.
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
#ifndef KOCANVASCONTROLLERGRAPHICSWIDGET_H
#define KOCANVASCONTROLLERGRAPHICSWIDGET_H

#include <QGraphicsWidget>

#include <KoCanvasController.h>

#include "flake_export.h"

class QCursor;
class QRect;
class QPoint;
class QSize;

class KoCanvasBase;

/**
 * A QGraphicsWidget that can embed a KOffice canvas, handling scrolling and zooming. You can
 * then add the QGraphicsWidget to an QGraphicsScene and show it in a QGraphicsView.
 */
class FLAKE_EXPORT KoCanvasControllerGraphicsWidget : public QGraphicsWidget, public KoCanvasController
{

    Q_OBJECT

public:

    KoCanvasControllerGraphicsWidget(QGraphicsItem * parent = 0, Qt::WindowFlags wFlags = 0);
    virtual ~KoCanvasControllerGraphicsWidget();

    // KoCanvasController implementation
    void scrollContentsBy(int dx, int dy);
    QSize viewportSize() const;
    void setDrawShadow(bool drawShadow);
    void setCanvas(KoCanvasBase *canvas);
    KoCanvasBase *canvas() const;
    int visibleHeight() const;
    int visibleWidth() const;
    int canvasOffsetX() const;
    int canvasOffsetY() const;
    void ensureVisible(const QRectF &rect, bool smooth = false);
    void ensureVisible(KoShape *shape);
    void zoomIn(const QPoint &center);
    void zoomOut(const QPoint &center);
    void zoomBy(const QPoint &center, qreal zoom);
    void zoomTo(const QRect &rect);
    void recenterPreferred();
    void setPreferredCenter(const QPoint &viewPoint);
    QPoint preferredCenter() const;
    void pan(const QPoint &distance);
    void setMargin(int margin);
    QPoint scrollBarValue() const;
    void setScrollBarValue(const QPoint &value);
    void updateDocumentSize(const QSize &sz, bool recalculateCenter = true);
    QCursor setCursor(const QCursor &cursor);
    void setZoomWithWheel(bool);
    void setVastScrolling(qreal);

private:
    class Private;
    Private * const d;

};

#endif // KOCANVASCONTROLLERGRAPHICSWIDGET_H
