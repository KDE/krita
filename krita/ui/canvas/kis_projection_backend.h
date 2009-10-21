/*
 *  Copyright (c) 2009 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef KIS_PROJECTION_BACKEND
#define KIS_PROJECTION_BACKEND

#include <QPainter>
#include <QImage>
#include <QSize>
#include <QRect>
#include <kis_types.h>

class KoColorProfile;
struct KisImagePatch;

/**
 * KisProjectionBackend ia an abstract class represinting
 * an object that can store a cache of KisImage projection.
 * More than that this object can perform some scaling operations
 * that are based on "patches" paradigm
 */
class KisProjectionBackend
{
public:
    virtual ~KisProjectionBackend();

    /**
     * Those methods are related to KisPrescaledProjection's
     * equivalents
     */
    virtual void setImage(KisImageWSP image) = 0;
    virtual void setImageSize(qint32 w, qint32 h) = 0;
    virtual void setMonitorProfile(const KoColorProfile* monitorProfile) = 0;

    /**
     * Updates @rc (in KisImage pixels) from the base image
     */
    virtual void setDirty(const QRect& rc) = 0;

    /**
     * Some backends cannot work with arbitrary areas due to
     * scaling stuff. That's why KisPrescaledProjection asks
     * a backend to align an image rect before any operations.
     */
    virtual void alignSourceRect(QRect& rect, qreal scale);

    /**
     * Get a patch from a backend that can draw a @requestedRect on some
     * QPainter in future. @scaleX and @scaleY are the scales of planned
     * drawing, btw, it doesn't mean that an QImage inside the patch will
     * have these scales - it'll have the nearest suitable scale or even
     * original scale (e.g. KisProjectionCache)
     *
     * If @borderWidth is non-zero, @requestedRect will be axpended by
     * @borderWidth pixels to all directions and image of this rect will
     * actually be written to the patch's QImage. That is done to eliminate
     * border effects in smooth scaling.
     */
    virtual KisImagePatch getNearestPatch(qreal scaleX, qreal scaleY,
                                          const QRect& requestedRect,
                                          qint32 borderWidth) = 0;

    /**
     * Draws a piece of original image onto @gc's canvas
     * @imageRect - area in KisImage pixels where to read from
     * @viewportRect - area in canvas pixels where to write to
     * If @imageRect and @viewportRect don't agree, the image
     * will be scaled
     * @borderWidth has the same meaning as in getNearestPatch
     * @renderHints - hints, transmitted to QPainter during darwing
     */
    virtual void drawFromOriginalImage(QPainter& gc,
                                       const QRect& imageRect,
                                       const QRectF& viewportRect,
                                       qint32 borderWidth,
                                       QPainter::RenderHints renderHints) = 0;
};


struct KisImagePatch {
    /**
     * The scale of the image stored in the patch
     */
    qreal m_scaleX;
    qreal m_scaleY;

    /**
     * The rect of KisImage covered by the image
     * of the patch (in KisImage pixels)
     */
    QRect m_patchRect;

    /**
     * The rect that was requested during creation
     * of the patch. It equals to patchRect withount
     * borders
     * These borders are introdused for more accurate
     * smooth scaling to reduce border effects
     * (IN m_image PIXELS, relative to m_image's topLeft);

     */
    QRectF m_interestRect;

    QImage m_image;

    /**
     * Darws an m_interestRect of the patch onto @gc
     * By the way it fits this rect into @dstRect
     * @renderHints are directly tranmitted to QPainter
     */
    void drawMe(QPainter &gc,
                const QRectF &dstRect,
                QPainter::RenderHints renderHints);

    /**
     * Prescales an interestRect a bit with Blitz
     * It's usefulness is dispulable and should be
     * tested, so - FIXME
     */
    void prescaleWithBlitz(QRectF dstRect);
};

inline void scaleRect(QRectF &rc, qreal scaleX, qreal scaleY)
{
    qreal x, y, w, h;
    rc.getRect(&x, &y, &w, &h);

    x *= scaleX;
    y *= scaleY;
    w *= scaleX;
    h *= scaleY;

    rc.setRect(x, y, w, h);
}

inline void scaleRect(QRect &rc, qreal scaleX, qreal scaleY)
{
    qint32 x, y, w, h;
    rc.getRect(&x, &y, &w, &h);

    x *= scaleX;
    y *= scaleY;
    w *= scaleX;
    h *= scaleY;

    rc.setRect(x, y, w, h);
}

#endif /* KIS_PROJECTION_BACKEND */
