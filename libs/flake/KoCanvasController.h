/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006, 2008 Thomas Zander <zander@kde.org>
 * SPDX-FileCopyrightText: 2007-2010 Boudewijn Rempt <boud@valdyas.org>
 * SPDX-FileCopyrightText: 2007-2008 C. Boemann <cbo@boemann.dk>
 * SPDX-FileCopyrightText: 2006-2007 Jan Hambrecht <jaham@gmx.net>
 * SPDX-FileCopyrightText: 2009 Thorsten Zachmann <zachmann@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KOCANVASCONTROLLER_H
#define KOCANVASCONTROLLER_H

#include "kritaflake_export.h"
#include <QObject>

#include <QSize>
#include <QPoint>
#include <QPointF>
#include <QPointer>

#include <KoZoomState.h>

class KisKActionCollection;
class QRect;
class QRectF;


class KoShape;
class KoCanvasBase;
class KoCanvasControllerProxyObject;
class KoViewTransformStillPoint;

/**
 * KoCanvasController is the base class for wrappers around your canvas
 * that provides scrolling and zooming for your canvas.
 *
 * Flake does not provide a canvas, the application will have to
 * implement a canvas themselves. You canvas can be QWidget-based
 * or something we haven't invented yet -- as long the class that holds the canvas
 * implements KoCanvasController, tools, scrolling and zooming will work.
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
    explicit KoCanvasController(KisKActionCollection* actionCollection);
    virtual ~KoCanvasController();

public:

    /**
     * Set the new canvas to be shown as a child
     * Calling this will Q_EMIT canvasRemoved() if there was a canvas before, and will emit
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
     * @brief Scrolls the content of the canvas so that the given rect is visible.
     *
     * The rect is to be specified in document coordinates (points). The scrollbar positions
     * are changed so that the centerpoint of the rectangle is centered if possible.
     *
     * @param rect the rectangle to make visible
     * @param smooth if true the viewport translation will make be just enough to ensure visibility, no more.
     */
    virtual void ensureVisibleDoc(const QRectF &docRect, bool smooth) = 0;

    /**
     * @brief zooms in keeping @p stillPoint not moved.
     */
    virtual void zoomIn(const KoViewTransformStillPoint &stillPoint) = 0;
    virtual void zoomIn() = 0;

    /**
     * @brief zooms out keeping @p stillPoint not moved.
     */
    virtual void zoomOut(const KoViewTransformStillPoint &stillPoint) = 0;
    virtual void zoomOut() = 0;

    /**
     * @brief zoom so that rect is exactly visible (as close as possible)
     *
     * The rect must be specified in **widget** coordinates. The scrollbar positions
     * are changed so that the center of the rect becomes center if possible.
     *
     * @param rect the rect in **widget** coordinates that should fit the view afterwards
     */
    virtual void zoomTo(const QRect &rect) = 0;

    virtual void setZoom(KoZoomMode::Mode mode, qreal zoom) = 0;

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
     * Returns the action collection for the window
     * @returns action collection for this window, can be 0
     */
    KisKActionCollection* actionCollection() const;

    /**
     * @return the current position of the cursor fetched from QCursor::pos() and
     *         converted into document coordinates
     */
    virtual QPointF currentCursorPosition() const = 0;

    virtual KoZoomState zoomState() const = 0;

protected:
    void setDocumentOffset(const QPoint &offset);


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

    void emitCanvasRemoved(KoCanvasController *canvasController) { Q_EMIT canvasRemoved(canvasController); }
    void emitCanvasSet(KoCanvasController *canvasController) { Q_EMIT canvasSet(canvasController); }
    void emitCanvasOffsetChanged() { Q_EMIT canvasOffsetChanged(); }
    void emitCanvasMousePositionChanged(const QPoint &position) { Q_EMIT canvasMousePositionChanged(position); }
    void emitDocumentMousePositionChanged(const QPointF &position) { Q_EMIT documentMousePositionChanged(position); }
    void emitSizeChanged(const QSize &size) { Q_EMIT sizeChanged(size); }
    void emitMoveDocumentOffset(const QPointF &oldOffset, const QPointF &newOffset) { Q_EMIT moveDocumentOffset(oldOffset, newOffset); }
    void emitMoveViewportOffset(const QPointF &oldOffset, const QPointF &newOffset) { Q_EMIT moveViewportOffset(oldOffset, newOffset); }
    void emitEffectiveZoomChanged(qreal zoom) { Q_EMIT effectiveZoomChanged(zoom); }
    void emitZoomStateChanged(const KoZoomState &zoomState) { Q_EMIT zoomStateChanged(zoomState); }
    void emitDocumentRectInWidgetPixelsChanged(const QRectF &documentRectInWidgetPixels) { Q_EMIT documentRectInWidgetPixelsChanged(documentRectInWidgetPixels); }
    void emitDocumentRotationChanged(qreal angle) { Q_EMIT documentRotationChanged(angle); }
    void emitDocumentMirrorStatusChanged(bool mirrorX, bool mirrorY) { Q_EMIT documentMirrorStatusChanged(mirrorX, mirrorY); }
    void emitCanvasStateChanged() { Q_EMIT canvasStateChanged(); }

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
     * Emitted when canvasOffset() changes
     */
    void canvasOffsetChanged();

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
    void moveDocumentOffset(const QPointF &oldOffset, const QPointF &newOffset);

    /**
     * Emitted whenever the document is scrolled and the viewport offset is changed.
     *
     * @param point the new top-left point from which the document should
     * be drawn.
     */
     void moveViewportOffset(const QPointF &oldOffset, const QPointF &newOffset);

    void effectiveZoomChanged(qreal zoom);

    void zoomStateChanged(const KoZoomState &zoomState);

    void documentRectInWidgetPixelsChanged(const QRectF &documentRectInWidgetPixels);

    void documentRotationChanged(qreal angle);
    void documentMirrorStatusChanged(bool mirrorX, bool mirrorY);

    void canvasStateChanged();

private:
    KoCanvasController *m_canvasController;
};

#endif
