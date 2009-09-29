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
#include <kis_shared.h>

class QPixmap;
class QImage;
class QPoint;
class QRect;
class QRectF;
class QSize;
class QPainter;

class KoViewConverter;
class KoColorProfile;

class KisProjectionBackend;

#include <kis_types.h>

class KisPrescaledProjection;
typedef KisSharedPtr<KisPrescaledProjection> KisPrescaledProjectionSP;

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
class KRITAUI_EXPORT KisPrescaledProjection : public QObject, public KisShared
{
    Q_OBJECT

public:

    KisPrescaledProjection();
    virtual ~KisPrescaledProjection();

    void setImage(KisImageWSP image);

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
    void setDrawCheckers(bool drawCheckers);

    /**
     * The pre-scaled pixmap includes the underlying checker
     * represenation. It is only generated when the drawCheckers() is
     * true, otherwise it is empty. The prescaled pixmal is exactly as
     * big as the canvas widget in pixels.
     */
    QPixmap prescaledPixmap() const;

    /**
     * Return the prescaled QImage. This image has a transparency
     * channel and is therefore suitable for generated a prescaled
     * representation of an image for the KritaShape. The prescaled
     * image is exactly as big as the canvas widget in pixels.
     */
    QImage prescaledQImage() const;

    /**
     * Set the view converter, the object that is responsible for
     * translating between image pixels, document points and view
     * pixels, keeping track of zoom levels.
     */
    void setViewConverter(KoViewConverter * viewConverter);

    /**
     * Return the intersection of the widget size and the given rect
     * in image pixels converted to widget pixels.
     */
    void updateDocumentOrigin(const QPoint &documentOrigin);

public slots:

    /**
     * Called whenever the configuration settings change.
     */
    void updateSettings();

    /**
     * Called from updateSettings to set up chosen backend:
     * KisProjectionCache or KisImagePyramidBased
     */
    void initBackend(bool useMipmapping, bool cacheKisImageAsQImage);

    /**
     * Called whenever the view widget needs to show a different part of
     * the document
     *
     * @param documentOffset the offset in widget pixels
     */
    void documentOffsetMoved(const QPoint &documentOffset);

    /**
     * The image projection has changed, now update the canvas
     * representation of it.
     *
     * @param rc the are to be updated in image pixels
     * @return a rect to be updated in widget pixels
     */
    QRect updateCanvasProjection(const QRect & rc);

    /**
     * Called whenever the size of the KisImage changes
     */
    void setImageSize(qint32 w, qint32 h);

    /**
     * Resize the prescaled image. The size is given in canvas
     * widget pixels.
     */
    void resizePrescaledImage(const QSize & newSize);

    /**
     * Set the current monitor profile
     */
    void setMonitorProfile(const KoColorProfile * profile);

    /**
     * Set the current node
     */
    void setCurrentNode(const KisNodeSP node);

    /**
     * Toggle whether the selection should be displayed as a mask.
     * (The display as ants should be a toggle, too, but is done
     * elsewhere.)
     */
    void showCurrentMask(bool showMask);


    /**
     * Called whenever the zoom level changes or another chunk of the
     * image becomes visible. The currently visible area of the image
     * is complete scaled again.
     */
    void preScale();

    /**
     * preScale and draw onto the scaled projection the specified rect,
     * in canvas view pixels.
     */
    QRect preScale(const QRect & rc);


signals:

    /**
     * emitted whenever the prescaled image is ready for painting.
     * This can happen in two stages: a coarse first stage and a
     * smooth second stage.
     *
     * @param rc the updated area in image pixels
     */
    // FIXME: seems to be not used
    //void sigPrescaledProjectionUpdated(const QRect & rc);

private:
    friend class KisPrescaledProjectionTest;

    KisPrescaledProjection(const KisPrescaledProjection &);
    KisPrescaledProjection operator=(const KisPrescaledProjection &);

    /**
     * Draw the prescaled image onto the painter.
     *
     * @param rc The desired rect (NOT in KisImage pixels)
     * It's in viewport pixels
     * @param gc The painter we draw on
     * directly to the blitz code
     * @return a rect actually updated during drawing
     * it can be greater that @rc do to KisImage alignment
     */
    QRect drawScaledImage(const QRect & rc,  QPainter & gc);


    /**
     * Actual drawing is done here
     * @param imageRect - region of the KisImage to read from
     * (in KisImage pixels)
     * @param viewportRect - region of @gc to draw to. Of course
     * it is in @gc's pixels. Actual image will be scaled down to
     * fit viewportRect
     */
    void drawUsingBackend(KisProjectionBackend *backend,
                          QPainter &gc,
                          qreal &scaleX, qreal &scaleY,
                          const QRect   &imageRect,
                          const QRectF  &viewportRect);

    /**
     * Converts image pixels into widget pixels
     * viewRect will always correspond to imageRect (no border
     * adjustment is done)
     * Note: caller should check whether viewRect is inside
     *       canvas area himself
     */
    QRectF viewRectFromImagePixels(const QRect& imageRect);

    /**
     * Converts widget pixels into image pixels
     * Note1: imageRect will always be adjusted to be inside
     *        an image, so it _may not_ correspond viewportRect
     * Note2: caller should check whether viewportRect is inside
     *        canvas area himself before calling.
     */
    QRect imageRectFromViewPortPixels(const QRectF& viewportRect);

    struct Private;
    Private * const m_d;
};

/**
 * A workaroung for Qt's strange behavior of QRect::toAlignedRect()
 */
KRITAUI_EXPORT QRect toAlignedRectWorkaround(const QRectF& rc);

/**
 * Heh.. "Due to hitory reasons" [1] Qt's QRect::right() doesn't return
 * the coordinate of rightmost side of the rect, it returns
 * the position of the last (rightmost) pixel owned by the rect.
 * However QRectF implements the former case.
 * We workaround it here AND help rounding process a bit.
 * Returned QRectF is ~2e-10 pixels smaller, than original @rc.
 * It doesn't play any role in drawing, but it helps to round up
 * back to @rc afterwards.
 *
 * [1] - see Qt documentation for more
 */
KRITAUI_EXPORT QRectF toFloatRectWorkaround(const QRect& rc);


#endif
