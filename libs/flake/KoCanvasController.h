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

#ifndef KOCANVASCONTROLLER_H
#define KOCANVASCONTROLLER_H

#include "flake_export.h"
#include <QObject>

class QRect;
class QRectF;
class QPoint;
class QPointF;
class QSize;

class KoShape;
class KoCanvasBase;
class KoView;
class KoCanvasControllerProxyObject;

/**
 * This widget is a wrapper around your canvas providing scrollbars.
 * Flake does not provide a canvas, the application will have to
 * extend a QWidget and implement that themselves; but Flake does make
 * it a lot easier to do so. One of those things is this widget that
 * acts as a decorator around the canvas widget and provides
 * scrollbars and allows the canvas to be centered in the viewArea
 * <p>The using application can intantiate this class and add its
 * canvas using the setCanvas() call. Which is designed so it can be
 * called multiple times for those that wish to exchange one canvas
 * widget for another.
 *
 * Effectively, there is _one_ KoCanvasController per KoView in your
 * application.
 *
 * The canvas widget is at most as big as the viewport of the scroll
 * area, and when the view on the document is near its edges, smaller.
 * In your canvas widget code, you can find the right place in your
 * document in view coordinates (pixels) by adding the documentOffset
 */
class FLAKE_EXPORT KoCanvasController
{

public:

    /// An enum to alter the positioning and size of the canvas inside the canvas controller
    enum CanvasMode {
        AlignTop,     ///< canvas is top aligned if smaller than the viewport
        Centered,     ///< canvas is centered if smaller than the viewport
        Infinite,     ///< canvas is never smaller than the viewport
        Presentation, ///< canvas is not handled by KoCanvasController, canvas is full screen
        Spreadsheet   ///< same as Infinite, but supports right-to-left layouts
    };


    // proxy QObject
    KoCanvasControllerProxyObject *proxyObject;

    /**
     * Constructor.
     */
    explicit KoCanvasController();
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
     * Sets the how the canvas behaves if the zoomed document becomes smaller than the viewport.
     * @param mode the new canvas mode, CanvasMode::Centered is the default value
     */
    virtual void setCanvasMode(KoCanvasController::CanvasMode mode);

    /// Returns the current canvas mode
    virtual KoCanvasController::CanvasMode canvasMode() const;


    /**
     * compatibility with QAbstractScrollArea
     */
    virtual void scrollContentsBy(int dx, int dy) = 0;

    /**
     * @return the size of the viewport
     */
    virtual QSize viewportSize() const = 0;

    /**
     * Set the shadow option -- by default the canvas controller draws
     * a black shadow around the canvas widget, which you may or may
     * not want.
     *
     * @param drawShadow if true, the shadow is drawn, if false, not
     */
    virtual void setDrawShadow(bool drawShadow) = 0;

    /**
     * Set the new canvas to be shown as a child
     * Calling this will emit canvasRemoved() if there was a canvas before, and will emit
     * canvasSet() with the new canvas.
     * @param canvas the new canvas. The KoCanvasBase::canvas() will be called to retrieve the
     *        actual widget which will then be added as child of this one.
     */
    virtual void setCanvas(KoCanvasBase *canvas) = 0;

    /**
     * Return the currently set canvas
     * @return the currently set canvas
     */
    virtual KoCanvasBase *canvas() const = 0;

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
    virtual void setPreferredCenter(const QPoint &viewPoint) = 0;

    /// Returns the currently set preferred center point in view coordinates (pixels)
    virtual QPoint preferredCenter() const = 0;

    /**
     * Move the canvas over the x and y distance of the parameter distance
     * @param distance the distance in view coordinates (pixels).  A positive distance means moving the canvas up/left.
     */
    virtual void pan(const QPoint &distance) = 0;

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
     * Called when the size of your document in view coordinates (pixels) changes, for instance when zooming.
     *
     * @param newSize the new size, in view coordinates (pixels), of the document.
     * @param recalculateCenter if true the offset in the document we center on after calling
     *      recenterPreferred() will be recalculated for the new document size so the visual offset stays the same.
     */
    virtual void updateDocumentSize(const QSize &sz, bool recalculateCenter) = 0;

protected:

    void setDocumentSize(const QSize &sz);
    QSize documentSize() const;

    void setPreferredCenterFractionX(qreal);
    qreal preferredCenterFractionX() const;

    void setPreferredCenterFractionY(qreal);
    qreal preferredCenterFractionY() const;

    void setDocumentOffset( QPoint &offset);
    QPoint documentOffset() const;

private:

    class Private;
    Private * const d;
};


class FLAKE_EXPORT KoCanvasControllerProxyObject : public QObject {

    Q_OBJECT
    Q_DISABLE_COPY(KoCanvasControllerProxyObject);

public:

    KoCanvasControllerProxyObject(KoCanvasController *canvasController, QObject *parent = 0);

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
    void emitZoomBy(const qreal factor) { emit zoomBy(factor); }

    // Convenience method to retrieve the canvas controller for who needs to use QPointer
    KoCanvasController *canvasController() const { return m_canvasController; }

signals:

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
     * Emitted when zoomTo have calculated a factor by which the zoom should change,
     * or if someone calls requestZoomBy
     * Someone needs to connect to this and take action
     *
     * @param factor by how much the zoom needs to change.
     */
    void zoomBy(const qreal factor);

public slots:

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

#endif
