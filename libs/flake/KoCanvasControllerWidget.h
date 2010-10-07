/* This file is part of the KDE project
 * Copyright (C) 2006, 2008 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007-2010 Boudewijn Rempt <boud@valdyas.org>
 * Copyright (C) 2007-2008 Casper Boemann <cbr@boemann.dk>
 * Copyright (C) 2006-2007 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2009 Thorsten Zachmann <zachmann@kde.org>
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

#ifndef KOCANVASCONTROLLERWIDGET_H
#define KOCANVASCONTROLLERWIDGET_H

#include "flake_export.h"

#include <QAbstractScrollArea>
#include <QMap>
#include <QString>

#include "KoCanvasController.h"

class KoShape;
class KoCanvasBase;
class KoView;

/**
 * KoCanvasController implementation for QWidget based canvases
 */
class FLAKE_EXPORT KoCanvasControllerWidget : public QAbstractScrollArea, public KoCanvasController
{
    Q_OBJECT
public:

    /**
     * Constructor.
     * @param parent the parent this widget will belong to
     */
    explicit KoCanvasControllerWidget(QWidget *parent = 0);
    virtual ~KoCanvasControllerWidget();

    /// Reimplemented from QObject
    virtual bool eventFilter(QObject *watched, QEvent *event);

    /**
     * Reimplemented from QAbstractScrollArea.
     */
    void scrollContentsBy(int dx, int dy);

    virtual QSize viewportSize() const;

    /// Reimplemented from KoCanvasController

    /**
     * Activate this canvascontroller
     */
    void activate();


    virtual void setDrawShadow(bool drawShadow);

    virtual void setCanvas(KoCanvasBase *canvas);

    virtual KoCanvasBase *canvas() const;

    /**
     * Change the actual canvas widget used by the current canvas. This allows the canvas widget
     * to be changed while keeping the current KoCanvasBase canvas and its associated resources as
     * they are. This might be used, for example, to switch from a QWidget to a QGLWidget canvas.
     * @param widget the new canvas widget.
     */
    void changeCanvasWidget(QWidget *widget);

    virtual int visibleHeight() const;
    virtual int visibleWidth() const;
    virtual int canvasOffsetX() const;
    virtual int canvasOffsetY() const;

    virtual void ensureVisible(const QRectF &rect, bool smooth = false);

    virtual void ensureVisible(KoShape *shape);

    /**
     * will cause the toolOptionWidgetsChanged to be emitted and all
     * listeners to be updated to the new widget.
     *
     * FIXME: This doesn't belong her and it does an
     * inherits("KoView") so it too much tied to komain
     *
     * @param widgets the map of widgets
     */
    void setToolOptionWidgets(const QMap<QString, QWidget *> &widgets);

    virtual void zoomIn(const QPoint &center);

    virtual void zoomOut(const QPoint &center);

    virtual void zoomBy(const QPoint &center, qreal zoom);

    virtual void zoomTo(const QRect &rect);

    virtual void recenterPreferred();

    virtual void setPreferredCenter(const QPoint &viewPoint);

    /// Returns the currently set preferred center point in view coordinates (pixels)
    virtual QPoint preferredCenter() const;

    virtual void pan(const QPoint &distance);

    virtual void setMargin(int margin);

    virtual QPoint scrollBarValue() const;

    virtual void setScrollBarValue(const QPoint &value);

    virtual void updateDocumentSize(const QSize &sz, bool recalculateCenter = true);

    /**
     * Set mouse wheel to zoom behaviour
     * @param zoom if true wheel will zoom instead of scroll, control modifier will scroll
     */
    void setZoomWithWheel(bool zoom);

    virtual void setVastScrolling(qreal factor);

    /**
     * \internal
     */
    class Private;
    KoCanvasControllerWidget::Private *priv();

signals:

    /**
     * Emit the new tool option widgets to be used with this canvas.
     */
    void toolOptionWidgetsChanged(const QMap<QString, QWidget *> &map, QWidget *widgets);

    /**
     * Emit the new tool option widgets to be used with this canvas.
     */
    void toolOptionWidgetsChanged(const QMap<QString, QWidget *> &widgets);

private slots:

    /// Called by the horizontal scrollbar when its value changes
    void updateCanvasOffsetX();

    /// Called by the vertical scrollbar when its value changes
    void updateCanvasOffsetY();

protected:

    /**
     * Moves scroll bars to ensure \p center is in the center
     * of the viewport
     */
    virtual void scrollToCenterPoint(const QPoint &center);

    /**
     * Zoom document keeping point \p widgetPoint unchanged
     * \param widgetPoint sticky point in widget pixels
     */
    virtual void zoomRelativeToPoint(const QPoint &widgetPoint, qreal zoomLevel);

    /// reimplemented from QWidget
    virtual void paintEvent(QPaintEvent *event);
    /// reimplemented from QWidget
    virtual void resizeEvent(QResizeEvent *resizeEvent);
    /// reimplemented from QWidget
    virtual void dragEnterEvent(QDragEnterEvent *event);
    /// reimplemented from QWidget
    virtual void dropEvent(QDropEvent *event);
    /// reimplemented from QWidget
    virtual void dragMoveEvent(QDragMoveEvent *event);
    /// reimplemented from QWidget
    virtual void dragLeaveEvent(QDragLeaveEvent *event);
    /// reimplemented from QWidget
    virtual void wheelEvent(QWheelEvent *event);
    /// reimplemented from QWidget
    virtual void keyPressEvent(QKeyEvent *event);
    /// reimplemented from QWidget
    virtual bool focusNextPrevChild(bool next);

private:
    Q_PRIVATE_SLOT(d, void activate())

    Private * const d;
};

#endif
