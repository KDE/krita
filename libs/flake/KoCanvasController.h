/* This file is part of the KDE project
 * Copyright (C) 2006, 2008 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007-2010 Boudewijn Rempt <boud@valdyas.org>
 * Copyright (C) 2007-2008 C. Boemann <cbo@boemann.dk>
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

#ifndef KOCANVASCONTROLLER_H
#define KOCANVASCONTROLLER_H

#include "kritaflake_export.h"
#include <QObject>

#include <QSize>
#include <QPoint>
#include <QPointF>
#include <QPointer>

class KActionCollection;
class QRect;
class QRectF;


class KoShape;
class KoCanvasBase;
class KoCanvasControllerProxyObject;

/**
 * KoCanvasController is the base class for wrappers around your canvas
 * that provides scrolling and zooming for your canvas.
 *
 * Flake does not provide a canvas, the application will have to
 * implement a canvas themselves. You canvas can be QWidget-based
 * or something we haven't invented yet -- as long the class that holds the canvas
 * imlements KoCanvasController, tools, scrolling and zooming will work.
 *
 * A KoCanvasController implementation acts as a decorator around the canvas widget
 * and provides a way to scroll the canvas, allows the canvas to be centered
 * in the viewArea and manages tool activation.
 *
 * <p>The using application can instantiate this class and add its
 * canvas using the setCanvas() call. Which is designed so it can be
 * called multiple times if you need to exchange one canvas
 * widget for another, for instance, switching between a plain QWidget or a QOpenGLWidget.
 *
 * <p>There is _one_ KoCanvasController per canvas in your
 * application.
 *
 * <p>The canvas widget is at most as big as the viewport of the scroll
 * area, and when the view on the document is near its edges, smaller.
 * In your canvas widget code, you can find the right place in your
 * document in view coordinates (pixels) by adding the documentOffset
 */
class KRITAFLAKE_EXPORT KoCanvasController
{
public:

    // proxy QObject: use this to connect to slots and signals.
    QPointer<KoCanvasControllerProxyObject> proxyObject;

    /**
     * Constructor.
     * @param actionCollection the action collection for this canvas
     */
    explicit KoCanvasController(KActionCollection* actionCollection);
    virtual ~KoCanvasController();

public:
    /**
     * Returns the current margin that is used to pad the canvas with.
     * This value is read from the KConfig property "canvasmargin"
     */
    virtual int margin() const;

    /**
     * Set the new margin to pad the canvas with.
     */
    virtual void setMargin(int margin);

    /**
     * compatibility with QAbstractScrollArea
     */
    virtual void scrollContentsBy(int dx, int dy) = 0;

    /**
     * @return the size of the viewport
     */
    virtual QSize viewportSize() const = 0;

    /**
     * Set the new canvas to be shown as a child
     * Calling this will emit canvasRemoved() if there was a canvas before, and will emit
     * canvasSet() with the new canvas.
     * @param canvas the new canvas. The KoCanvasBase::canvas() will be called to retrieve the
     *        actual widget which will then be added as child of this one.
     */
    virtual void setCanvas(KoCanvasBase *canvas) = 0;

    /**
     * Return the currently set canvas. The default implementation will return Null
     * @return the currently set canvas
     */
    virtual KoCanvasBase *canvas() const;

    /**
     * return the amount of pixels vertically visible of the child canvas.
     * @return the amount of pixels vertically visible of the child canvas.
     */
    virtual int visibleHeight() const = 0;

    /**
     * return the amount of pixels horizontally visible of the child canvas.
     * @return the amount of pixels horizontally visible of the child canvas.
     */
    virtual int visibleWidth() const = 0;

    /**
     * return the amount of pixels that are not visible on the left side of the canvas.
     * The leftmost pixel that is shown is returned.
     */
    virtual int canvasOffsetX() const = 0;

    /**
     * return the amount of pixels that are not visible on the top side of the canvas.
     * The topmost pixel that is shown is returned.
     */
    virtual int canvasOffsetY() const = 0;

    /**
     * @brief Scrolls the content of the canvas so that the given rect is visible.
     *
     * The rect is to be specified in view coordinates (pixels). The scrollbar positions
     * are changed so that the centerpoint of the rectangle is centered if possible.
     *
     * @param rect the rectangle to make visible
     * @param smooth if true the viewport translation will make be just enough to ensure visibility, no more.
     * @see KoViewConverter::documentToView()
     */
    virtual void ensureVisible(const QRectF &rect, bool smooth = false) = 0;

    /**
     * @brief Scrolls the content of the canvas so that the given shape is visible.
     *
     * This is just a wrapper function of the above function.
     *
     * @param shape the shape to make visible
     */
    virtual void ensureVisible(KoShape *shape) = 0;

    /**
     * @brief zooms in around the center.
     *
     * The center must be specified in view coordinates (pixels). The scrollbar positions
     * are changed so that the center becomes center if possible.
     *
     * @param center the position to zoom in on
     */
    virtual void zoomIn(const QPoint &center) = 0;

    /**
     * @brief zooms out around the center.
     *
     * The center must be specified in view coordinates (pixels). The scrollbar positions
     * are changed so that the center becomes center if possible.
     *
     * @param center the position to zoom out around
     */
    virtual void zoomOut(const QPoint &center) = 0;

    /**
     * @brief zooms around the center.
     *
     * The center must be specified in view coordinates (pixels). The scrollbar positions
     * are changed so that the center becomes center if possible.
     *
     * @param center the position to zoom around
     * @param zoom the zoom to apply
     */
    virtual void zoomBy(const QPoint &center, qreal zoom) = 0;

    /**
     * @brief zoom so that rect is exactly visible (as close as possible)
     *
     * The rect must be specified in view coordinates (pixels). The scrollbar positions
     * are changed so that the center of the rect becomes center if possible.
     *
     * @param rect the rect in view coordinates (pixels) that should fit the view afterwards
     */
    virtual void zoomTo(const QRect &rect) = 0;

    /**
     * @brief repositions the scrollbars so previous center is once again center
     *
     * The previous center is cached from when the user uses the scrollbars or zoomTo
     * are called. zoomTo is mostly used when a zoom tool of sorts have marked an area
     * to zoom in on
     *
     * The success of this method is limited by the size of thing. But we try our best.
     */
    virtual void recenterPreferred() = 0;

    /**
     * Sets the preferred center point in view coordinates (pixels).
     * @param viewPoint the new preferred center
     */
    virtual void setPreferredCenter(const QPointF &viewPoint) = 0;

    /// Returns the currently set preferred center point in view coordinates (pixels)
    virtual QPointF preferredCenter() const = 0;

    /**
     * Move the canvas over the x and y distance of the parameter distance
     * @param distance the distance in view coordinates (pixels).  A positive distance means moving the canvas up/left.
     */
    virtual void pan(const QPoint &distance) = 0;

    /**
     * Move the canvas up. This behaves the same as \sa pan() with a positive y coordinate.
     */
    virtual void panUp() = 0;

    /**
     * Move the canvas down. This behaves the same as \sa pan() with a negative y coordinate.
     */
    virtual void panDown() = 0;

    /**
     * Move the canvas to the left. This behaves the same as \sa pan() with a positive x coordinate.
     */
    virtual void panLeft() = 0;

    /**
     * Move the canvas to the right. This behaves the same as \sa pan() with a negative x coordinate.
     */
    virtual void panRight() = 0;

    /**
     * Get the position of the scrollbar
     */
    virtual QPoint scrollBarValue() const = 0;

    /**
     * Set the position of the scrollbar
     * @param value the new values of the scroll bars
     */
    virtual void setScrollBarValue(const QPoint &value) = 0;

    /**
     * Update the range of scroll bars
     */
    virtual void resetScrollBars() = 0;

    /**
     * Called when the size of your document in view coordinates (pixels) changes, for instance when zooming.
     *
     * @param newSize the new size, in view coordinates (pixels), of the document.
     * @param recalculateCenter if true the offset in the document we center on after calling
     *      recenterPreferred() will be recalculated for the new document size so the visual offset stays the same.
     */
    virtual void updateDocumentSize(const QSize &sz, bool recalculateCenter) = 0;

    /**
     * Set mouse wheel to zoom behaviour
     * @param zoom if true wheel will zoom instead of scroll, control modifier will scroll
     */
    virtual void setZoomWithWheel(bool zoom) = 0;

    /**
     * Set scroll area to be bigger than actual document.
     * It allows the user to move the corner of the document
     * to e.g. the center of the screen
     *
     * @param factor the coefficient, defining how much we can scroll out,
     *     measured in parts of the widget size. Null value means vast
     *     scrolling is disabled.
     */
    virtual void setVastScrolling(qreal factor) = 0;

   /**
     * Returns the action collection for the canvas
     * @returns action collection for this canvas, can be 0
     */
    virtual KActionCollection* actionCollection() const;

    QPoint documentOffset() const;

    /**
     * @return the current position of the cursor fetched from QCursor::pos() and
     *         converted into document coordinates
     */
    virtual QPointF currentCursorPosition() const = 0;

protected:
    void setDocumentSize(const QSize &sz);
    QSize documentSize() const;

    void setPreferredCenterFractionX(qreal);
    qreal preferredCenterFractionX() const;

    void setPreferredCenterFractionY(qreal);
    qreal preferredCenterFractionY() const;

    void setDocumentOffset( QPoint &offset);


private:
    class Private;
    Private * const d;
};


/**
 * Workaround class for the problem that Qt does not allow two QObject base classes.
 * KoCanvasController can be implemented by for instance QWidgets, so it cannot be
 * a QObject directly. The interface of this class should be considered public interface
 * for KoCanvasController.
 */
class KRITAFLAKE_EXPORT KoCanvasControllerProxyObject : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(KoCanvasControllerProxyObject)
public:
    explicit KoCanvasControllerProxyObject(KoCanvasController *canvasController, QObject *parent = 0);

public:

    // Convenience methods to invoke the signals from subclasses

    void emitCanvasRemoved(KoCanvasController *canvasController) { emit canvasRemoved(canvasController); }
    void emitCanvasSet(KoCanvasController *canvasController) { emit canvasSet(canvasController); }
    void emitCanvasOffsetXChanged(int offset) { emit canvasOffsetXChanged(offset); }
    void emitCanvasOffsetYChanged(int offset) { emit canvasOffsetYChanged(offset); }
    void emitCanvasMousePositionChanged(const QPoint &position) { emit canvasMousePositionChanged(position); }
    void emitDocumentMousePositionChanged(const QPointF &position) { emit documentMousePositionChanged(position); }
    void emitSizeChanged(const QSize &size) { emit sizeChanged(size); }
    void emitMoveDocumentOffset(const QPoint &point) { emit moveDocumentOffset(point); }
    void emitZoomRelative(const qreal factor, const QPointF &stillPoint) { emit zoomRelative(factor, stillPoint); }

    // Convenience method to retrieve the canvas controller for who needs to use QPointer
    KoCanvasController *canvasController() const { return m_canvasController; }

Q_SIGNALS:
    /**
     * Emitted when a previously added canvas is about to be removed.
     * @param canvasController this object
     */
    void canvasRemoved(KoCanvasController *canvasController);

    /**
     * Emitted when a canvas is set on this widget
     * @param canvasController this object
     */
    void canvasSet(KoCanvasController *canvasController);

    /**
     * Emitted when canvasOffsetX() changes
     * @param offset the new canvas offset
     */
    void canvasOffsetXChanged(int offset);

    /**
     * Emitted when canvasOffsetY() changes
     * @param offset the new canvas offset
     */
    void canvasOffsetYChanged(int offset);

    /**
     * Emitted when the cursor is moved over the canvas widget.
     * @param position the position in view coordinates (pixels).
     */
    void canvasMousePositionChanged(const QPoint &position);

    /**
     * Emitted when the cursor is moved over the canvas widget.
     * @param position the position in document coordinates.
     *
     * Use \ref canvasMousePositionChanged to get the position
     * in view coordinates.
     */
    void documentMousePositionChanged(const QPointF &position);

    /**
     * Emitted when the entire controller size changes
     * @param size the size in widget pixels.
     */
    void sizeChanged(const QSize &size);

    /**
     * Emitted whenever the document is scrolled.
     *
     * @param point the new top-left point from which the document should
     * be drawn.
     */
    void moveDocumentOffset(const QPoint &point);

    /**
     * Emitted when zoomRelativeToPoint have calculated a factor by which
     * the zoom should change and the point which should stand still
     * on screen.
     * Someone needs to connect to this and take action
     *
     * @param factor by how much the zoom needs to change.
     * @param stillPoint the point which will not change its position
     *                   in widget during the zooming. It is measured in
     *                   view coordinate system *before* zoom.
     */
    void zoomRelative(const qreal factor, const QPointF &stillPoint);

public Q_SLOTS:
    /**
     * Call this slot whenever the size of your document in view coordinates (pixels)
     * changes, for instance when zooming.
     * @param newSize the new size, in view coordinates (pixels), of the document.
     * @param recalculateCenter if true the offset in the document we center on after calling
     *      recenterPreferred() will be recalculated for the new document size so the visual offset stays the same.
     */
    void updateDocumentSize(const QSize &newSize, bool recalculateCenter = true);

private:
    KoCanvasController *m_canvasController;
};

class KRITAFLAKE_EXPORT  KoDummyCanvasController : public KoCanvasController {

public:

    explicit KoDummyCanvasController(KActionCollection* actionCollection)
        : KoCanvasController(actionCollection)
    {}

    ~KoDummyCanvasController() override
    {}


    void scrollContentsBy(int /*dx*/, int /*dy*/) override {}
    QSize viewportSize() const override { return QSize(); }
    void setCanvas(KoCanvasBase *canvas) override {Q_UNUSED(canvas)}
    KoCanvasBase *canvas() const override {return 0;}
    int visibleHeight() const override {return 0;}
    int visibleWidth() const override {return 0;}
    int canvasOffsetX() const override {return 0;}
    int canvasOffsetY() const override {return 0;}
    void ensureVisible(const QRectF &/*rect*/, bool /*smooth */ = false) override {}
    void ensureVisible(KoShape *shape) override {Q_UNUSED(shape)}
    void zoomIn(const QPoint &/*center*/) override {}
    void zoomOut(const QPoint &/*center*/) override {}
    void zoomBy(const QPoint &/*center*/, qreal /*zoom*/) override {}
    void zoomTo(const QRect &/*rect*/) override {}
    void recenterPreferred() override {}
    void setPreferredCenter(const QPointF &/*viewPoint*/) override {}
    QPointF preferredCenter() const override {return QPointF();}
    void pan(const QPoint &/*distance*/) override {}
    void panUp() override {}
    void panDown() override {}
    void panLeft() override {}
    void panRight() override {}
    QPoint scrollBarValue() const override {return QPoint();}
    void setScrollBarValue(const QPoint &/*value*/) override {}
    void resetScrollBars() override {}
    void updateDocumentSize(const QSize &/*sz*/, bool /*recalculateCenter*/) override {}
    void setZoomWithWheel(bool /*zoom*/) override {}
    void setVastScrolling(qreal /*factor*/) override {}
    QPointF currentCursorPosition() const override { return QPointF(); }
};

#endif
