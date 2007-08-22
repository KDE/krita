/*
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2006
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef KIS_PRESCALED_PROJECTION_H
#define KIS_PRESCALED_PROJECTION_H

#include <QObject>

#include <krita_export.h>

class QPixmap;
class QImage;
class QPoint;
class QRect;
class QSize;
class QPainter;

class KoViewConverter;
class KoColorProfile;

#include <kis_types.h>

/**
 * KisPrescaledProjection is responsible for keeping around a
 * prescaled QImage representation that is always suitable for
 * painting onto the canvas.
 *
 * Optionally, the KisPrescaledProjection can also provide a QPixmap
 * with the checkered background blended in.
 *
 * Optionally, the projection can also draw the mask and selection
 * masks and the selection outline.
 *
 * The following scaling methods are supported:
 *
 * <ul>
 *   <li>Qt's smooth scaling
 *   <li>Our own smooth scaling (similar to Blitz, port to using Blitz)
 *   <li>Our own sampling (similar to Blitz, port to using Blitz)
 *   <li>nearest-neighbour sampling on KisImage directly (doesn't need
 *       a QImage of the visible area)
 * </ul>
 *
 * Note: the export macro is only for the unittest.
 *
 * Note: with any method except for nearest-neighbour sampling Krita
 * keeps a QImage the size of the unscaled image in memory. This
 * should become either a QImage the size of the nearest pyramid level
 * or a tiled QImage representation like the OpenGL image textures.
 */
class KRITAUI_EXPORT KisPrescaledProjection : QObject
{
    Q_OBJECT

public:

    KisPrescaledProjection();
    virtual ~KisPrescaledProjection();

    void setImage( KisImageSP image );

    /**
     * @return true if the prescaled projection is set to draw the
     * checkers, too. In that case, prescaledPixmap returns a complete
     * pixmap (which doesn't have transparency) and prescaledQImage
     * returns an empty QImage. This setting is <i>false</i>
     * initially.
     */
    bool drawCheckers() const;

    /**
     * Set the drawCheckers variable to @param drawCheckers. @see
     * drawCheckers.
     */
    void setDrawCheckers( bool drawCheckers );

    /**
     * The pre-scaled pixmap includes the underlying checker
     * represenation. It is only generated when the drawCheckers() is
     * true, otherwise it is empty.
     */
    QPixmap prescaledPixmap() const;

    /**
     * Return the prescaled QImage. This image has a transparency
     * channel and is therefore suitable for generated a prescaled
     * representation of an image for the KritaShape.
     */
    QImage prescaledQImage() const;

    /**
     * Set the view converter, the object that is responsible for
     * translating between image pixels, document points and view
     * pixels, keeping track of zoom levels.
     */
    void setViewConverter( KoViewConverter * viewConverter );

public slots:

    /**
     * Called whenever the configuration settings change.
     */
    void updateSettings();

    /**
     * Called whenever the view widget needs to show a different part of
     * the document
     *
     * @param documentOffset the offset in widget pixels
     */
    void documentOffsetMoved( const QPoint &documentOffset );

    /**
     * The image projection has changed, now update the canvas
     * representation of it.
     */
    void updateCanvasProjection( const QRect & rc );

    /**
     * Called whenever the size of the KisImage changes
     */
    void setImageSize(qint32 w, qint32 h);

    /**
     * Called whenever the zoom level changes or another chunk of the
     * image becomes visible. The currently visible area of the image
     * is complete scaled again.
     */
    void preScale();

    /**
     * preScale and draw onto the scaled projection the specfied rect,
     * in KisImage pixels.
     */
    void preScale( const QRect & rc );

    /**
     * Resize the prescaled image.
     */
    void resizePrescaledImage( QSize newSize, QSize oldSize );

    /**
     * Set the current monitor profile
     */
    void setMonitorProfile( KoColorProfile * profile );

    /**
     * Set the current HDR exposure
     */
    void setHDRExposure( float exposure );

signals:

    /**
     * emitted whenever the prescaled image is ready for painting.
     * This can happen in two stages: a coarse first stage and a
     * smooth second stage.
     */
    void sigPrescaledProjectionUpdated( const QRect & rc );

private:

    KisPrescaledProjection( const KisPrescaledProjection & );
    KisPrescaledProjection operator=( const KisPrescaledProjection & );

    /**
     * Draw the prescaled image onto the painter.
     *
     * @param rc The desired rect in KisImage pixels
     * @param gc The painter we draw on
     */
    void drawScaledImage( const QRect & rc,  QPainter & gc );

    struct Private;
    Private * const m_d;

};

#endif
