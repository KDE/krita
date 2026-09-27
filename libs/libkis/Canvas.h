/*
 *  SPDX-FileCopyrightText: 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef LIBKIS_CANVAS_H
#define LIBKIS_CANVAS_H

#include <QObject>

#include "kritalibkis_export.h"
#include "libkis.h"

class KoCanvasBase;
class KisDisplayColorConverter;
class KoZoomState;

/**
 * Canvas wraps the canvas inside a view on an image/document.
 * It is responsible for the view parameters of the document:
 * zoom, rotation, mirror, wraparound and instant preview.
 */
class KRITALIBKIS_EXPORT Canvas : public QObject
{
    Q_OBJECT

public:
    enum ZoomMode
    {
        ZOOM_CONSTANT = 0,  // zoom x %
        ZOOM_PAGE     = 1,  // zoom to page
        ZOOM_WIDTH    = 2,  // zoom to width
        ZOOM_HEIGHT   = 16, // zoom to height
    };

    explicit Canvas(KoCanvasBase *canvas, QObject *parent = 0);
    ~Canvas() override;

    bool operator==(const Canvas &other) const;
    bool operator!=(const Canvas &other) const;

public Q_SLOTS:

    /**
     * @return the current zoomlevel. 1.0 is 100%.
     */
    qreal zoomLevel() const;

    /**
     * @brief setZoomLevel set the zoomlevel to the given @p value. 1.0 is 100%.
     */
    void setZoomLevel(qreal value);

    /**
     * @brief setPan Centers the image pixel at \p imagePos in the current view
     */
    void setPreferredCenter(const QPointF& imagePos);

    /**
     * @brief \return the position of the image pixel that is placed in the center of the current view
     */
    QPointF preferredCenter() const;

    /**
     * @brief pan the current view in pixels.
     */
    void pan(const QPoint& offset);

    /**
     * @brief resetZoom set the zoomlevel to 100%
     */
    void resetZoom();

    /**
     * @return the rotation of the canvas in degrees.
     */
    qreal rotation() const;

    /**
     * @brief setRotation set the rotation of the canvas to the given  @param angle in degrees.
     */
    void setRotation(qreal angle);

    /**
     * @brief resetRotation reset the canvas rotation.
     */
    void resetRotation();

    /**
     * @return return true if the canvas is mirrored, false otherwise.
     */
    bool mirror() const;

    /**
     * @brief setMirror turn the canvas mirroring on or off depending on @param value
     */
    void setMirror(bool value);

    /**
     * @return true if the canvas is in wraparound mode, false if not. Only when OpenGL is enabled,
     * is wraparound mode available.
     */
    bool wrapAroundMode() const;

    /**
     * @brief setWrapAroundMode set wraparound mode to  @param enable
     */
    void setWrapAroundMode(bool enable);

    /**
     * @return true if the canvas is in Instant Preview mode, false if not. Only when OpenGL is enabled,
     * is Instant Preview mode available.
     */
    bool levelOfDetailMode() const;

    /**
     * @brief setLevelOfDetailMode sets Instant Preview to @param enable
     */
    void setLevelOfDetailMode(bool enable);

    /**
     * @return the view that holds this canvas
     */
    View *view() const;

private Q_SLOTS:
    void emitZoomStateChanged(const KoZoomState &zoomState);

Q_SIGNALS:

/**
 * Emitted when the canvas is about to be removed.
 */
void removed();

/**
 * Emitted when the canvas is panned.
 * See \ref documentOffsetChanged
 */
void offsetChanged();

/**
 * Emitted when the cursor is moved over the canvas widget.
 * @param position the position in view coordinates (pixels).
 */
void cursorPositionChanged(const QPoint &position);

/**
 * Emitted when the cursor is moved over the canvas widget.
 * @param position the position in document coordinates.
 *
 * Use \ref cursorPositionChanged to get the position
 * in view coordinates.
 */
void documentCursorPositionChanged(const QPointF &position);

/**
 * Emitted when the viewport size changes.
 * @param size the size in widget pixels.
 */
void sizeChanged(const QSize &size);

/**
 * Emitted whenever the canvas is panned.
 *
 * @param point the new top-left point from which the document should
 * be drawn.
 */
void documentOffsetChanged(const QPointF &oldOffset, const QPointF &newOffset);

/**
 * Emitted whenever the effective zoom level changes.
 *
 * @param effectiveZoom In pixel size display mode, zoom * 1 / display scaling.
 * In print size display mode, zoom * display DPI / image PPI.
 */
void effectiveZoomChanged(qreal effectiveZoom);

/**
 * Emitted whenever the zoom state changes.
 *
 * @param zoomMode \ref ZoomMode
 * @param zoom zoom level, 1.0 is 100%
 * @param minZoom the minimum allowed zoom level
 * @param maxZoom the maximum allowed zoom level
 */
void zoomStateChanged(const ZoomMode zoomMode, qreal zoom, qreal minZoom, qreal maxZoom);

/**
 * Emitted whenever the document's position or size within the viewport widget changes.
 *
 * @param documentRectInWidgetPixels rect with position of documentOffset and size of document size * effective zoom.
 */
void documentRectInWidgetPixelsChanged(const QRectF &documentRectInWidgetPixels);

/**
 * Emitted when rotation changes.
 * @param angle the angle in degrees
 */
void rotationChanged(qreal angle);

/**
 * Emitted when mirrored status changes.
 * @param mirrorX whether the canvas's view is mirrored horizontally (reversed)
 * @param mirrorY whether the canvas's view is mirrored vertically (inverted)
 */
void mirrorChanged(bool mirrorX, bool mirrorY);

/**
 * Emitted when this canvas's zoom, rotation, mirror, offset, or viewport change.
 */
void stateChanged();

private:

    friend class ManagedColor;

    KisDisplayColorConverter *displayColorConverter() const;

    struct Private;
    Private *const d;

};

#endif // LIBKIS_CANVAS_H
