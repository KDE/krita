/* This file is part of the KDE project
 * Copyright (C) 2006, 2008 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Boudewijn Rempt <boud@valdyas.org>
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

#include <QAbstractScrollArea>

class KoShape;
class KoCanvasBase;
class KoView;

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
class FLAKE_EXPORT KoCanvasController : public QAbstractScrollArea
{
    Q_OBJECT
public:
    /// An enum to alter the positioning and size of the canvas inside the canvas controller
    enum CanvasMode {
        AlignTop,     ///< canvas is top aligned if smaller than the viewport
        Centered,     ///< canvas is centered if smaller than the viewport
        Infinite,     ///< canvas is never smaller than the viewport
        Presentation, ///< canvas is not handled by KoCanvasController, canvas is full screen
        Spreadsheet   ///< same as Infinite, but supports right-to-left layouts
    };

    /**
     * Constructor.
     * @param parent the parent this widget will belong to
     */
    explicit KoCanvasController(QWidget *parent = 0);
    virtual ~KoCanvasController();
    /**
     * Reimplemented from QAbstractScrollArea.
     */
    void scrollContentsBy(int dx, int dy);

    /**
     * Set the shadow option -- by default the canvas controller draws
     * a black shadow around the canvas widget, which you may or may
     * not want.
     *
     * @param drawShadow if true, the shadow is drawn, if false, not
     */
    void setDrawShadow(bool drawShadow);

    /**
     * Set the new canvas to be shown as a child
     * Calling this will emit canvasRemoved() if there was a canvas before, and will emit
     * canvasSet() with the new canvas.
     * @param canvas the new canvas. The KoCanvasBase::canvas() will be called to retrieve the
     *        actual widget which will then be added as child of this one.
     */
    void setCanvas(KoCanvasBase *canvas);
    /**
     * Return the currently set canvas
     * @return the currently set canvas
     */
    KoCanvasBase *canvas() const;

    /**
     * Change the actual canvas widget used by the current canvas. This allows the canvas widget
     * to be changed while keeping the current KoCanvasBase canvas and its associated resources as
     * they are. This might be used, for example, to switch from a QWidget to a QGLWidget canvas.
     * @param widget the new canvas widget.
     */
    void changeCanvasWidget(QWidget *widget);

    /**
     * return the amount of pixels vertically visible of the child canvas.
     * @return the amount of pixels vertically visible of the child canvas.
     */
    int visibleHeight() const;
    /**
     * return the amount of pixels horizontally visible of the child canvas.
     * @return the amount of pixels horizontally visible of the child canvas.
     */
    int visibleWidth() const;
    /**
     * return the amount of pixels that are not visible on the left side of the canvas.
     * The leftmost pixel that is shown is returned.
     */
    int canvasOffsetX() const;
    /**
     * return the amount of pixels that are not visible on the top side of the canvas.
     * The topmost pixel that is shown is returned.
     */
    int canvasOffsetY() const;

    /**
     * Sets the how the canvas behaves if the zoomed document becomes smaller than the viewport.
     * @param mode the new canvas mode, CanvasMode::Centered is the default value
     */
    void setCanvasMode(CanvasMode mode);

    /// Returns the current canvas mode
    CanvasMode canvasMode() const;

    /// Returns true if canvas uses QGLWidget (OpenGL can be used for painting on canvas)
    bool isCanvasOpenGL() const;

    /// Reimplemented from QObject
    virtual bool eventFilter(QObject *watched, QEvent *event);

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
    void ensureVisible(const QRectF &rect, bool smooth = false);

    /**
     * @brief Scrolls the content of the canvas so that the given shape is visible.
     *
     * This is just a wrapper function of the above function.
     *
     * @param shape the shape to make visible
     */
    void ensureVisible(KoShape *shape);

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

    /**
     * @brief zooms in around the center.
     *
     * The center must be specified in view coordinates (pixels). The scrollbar positions
     * are changed so that the center becomes center if possible.
     *
     * @param center the position to zoom in on
     */
    void zoomIn(const QPoint &center);

    /**
     * @brief zooms out around the center.
     *
     * The center must be specified in view coordinates (pixels). The scrollbar positions
     * are changed so that the center becomes center if possible.
     *
     * @param center the position to zoom out around
     */
    void zoomOut(const QPoint &center);

    /**
     * @brief zooms around the center.
     *
     * The center must be specified in view coordinates (pixels). The scrollbar positions
     * are changed so that the center becomes center if possible.
     *
     * @param center the position to zoom around
     * @param zoom the zoom to apply
     */
    void zoomBy(const QPoint &center, qreal zoom);

    /**
     * @brief zoom so that rect is exactly visible (as close as possible)
     *
     * The rect must be specified in view coordinates (pixels). The scrollbar positions
     * are changed so that the center of the rect becomes center if possible.
     *
     * @param rect the rect in view coordinates (pixels) that should fit the view afterwards
     */
    void zoomTo(const QRect &rect);

    /**
     * @brief repositions the scrollbars so previous center is once again center
     *
     * The previous center is cached from when the user uses the scrollbars or zoomTo
     * are called. zoomTo is mostly used when a zoom tool of sorts have marked an area
     * to zoom in on
     *
     * The success of this method is limited by the size of thing. But we try our best.
     */
    void recenterPreferred();

    /**
     * Sets the preferred center point in view coordinates (pixels).
     * @param viewPoint the new preferred center
     */
    void setPreferredCenter(const QPoint &viewPoint);

    /// Returns the currently set preferred center point in view coordinates (pixels)
    QPoint preferredCenter() const;

    /**
     * Move the canvas over the x and y distance of the parameter distance
     * @param distance the distance in view coordinates (pixels).  A positive distance means moving the canvas up/left.
     */
    void pan(const QPoint &distance);

    /**
     * Returns the current margin that is used to pad the canvas with.
     * This value is read from the KConfig property "canvasmargin"
     */
    int margin() const;

    /**
     * Set the new margin to pad the canvas with.
     */
    void setMargin(int margin);

    /**
     * Get the position of the scrollbar
     */
    QPoint scrollBarValue() const;

    /**
     * Set the position of the scrollbar
     * @param value the new values of the scroll bars
     */
    void setScrollBarValue(const QPoint &value);

    /**
     * \internal
     */
    class Private;
    KoCanvasController::Private *priv();

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
     * Emit the new tool option widgets to be used with this canvas.
     */
    void toolOptionWidgetsChanged(const QMap<QString, QWidget *> &map, QWidget *widgets);

    /**
     * Emit the new tool option widgets to be used with this canvas.
     */
    void toolOptionWidgetsChanged(const QMap<QString, QWidget *> &widgets);

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
    void setDocumentSize(const QSize &newSize, bool recalculateCenter = true);

protected slots:

    /// Called by the horizontal scrollbar when its value changes
    void updateCanvasOffsetX();

    /// Called by the vertical scrollbar when its value changes
    void updateCanvasOffsetY();

protected:
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
