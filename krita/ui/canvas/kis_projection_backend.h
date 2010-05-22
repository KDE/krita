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
class UpdateInformation;
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
     * Gets a patch from a backend that can draw a info.imageRect on some
     * QPainter in future. info.scaleX and info.scaleY are the scales
     * of planned drawing, btw, it doesn't mean that an QImage inside
     * the patch will have these scales - it'll have the nearest suitable
     * scale or even original scale (e.g. KisProjectionCache)
     *
     * If info.borderWidth is non-zero, info.requestedRect will
     * be axpended by info.borderWidth pixels to all directions and
     * image of this rect will actually be written to the patch's QImage.
     * That is done to eliminate border effects in smooth scaling.
     */
    virtual KisImagePatch getNearestPatch(UpdateInformation &info) = 0;

    /**
     * Draws a piece of original image onto @gc's canvas
     * @param info.imageRect - area in KisImage pixels where to read from
     * @param info.viewportRect - area in canvas pixels where to write to
     * If info.imageRect and info.viewportRect don't agree, the image
     * will be scaled
     * @param info.borderWidth has the same meaning as in getNearestPatch
     * @param info.renderHints - hints, transmitted to QPainter during darwing
     */
    virtual void drawFromOriginalImage(QPainter& gc,
                                       UpdateInformation &info) = 0;
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

class UpdateInformation
{
public:
    enum TransferType {
        DIRECT,
        PATCH
    };

    /**
     * Rect of KisImage corresponding to @viewportRect
     */
    QRect imageRect;

    /**
     * Rect of canvas widget corresponding to @imageRect
     */
    QRectF viewportRect;

    qreal scaleX;
    qreal scaleY;

    /**
     * Defines the way the source image is painted onto
     * prescaled QImage
     */
    TransferType transfer;

    /**
     * Render hints for painting the direct painting/patch painting
     */
    QPainter::RenderHints renderHints;

    /**
     * The number of additional pixels those should be added
     * to the patch
     */
    qint32 borderWidth;

    /**
     * Used for temporary sorage of KisImage's data
     * by KisPrescaledCache
     */
    KisImagePatch patch;
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
