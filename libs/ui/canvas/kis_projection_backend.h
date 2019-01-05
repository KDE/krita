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

#include "kis_update_info.h"

class KoColorProfile;
class KisImagePatch;
class KisDisplayFilter;

#include <KoColorConversionTransformation.h>

/**
 * KisProjectionBackend is an abstract class representing
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
    virtual void setMonitorProfile(const KoColorProfile* monitorProfile, KoColorConversionTransformation::Intent renderingIntent, KoColorConversionTransformation::ConversionFlags conversionFlags) = 0;
    virtual void setChannelFlags(const QBitArray &channelFlags) = 0;
    virtual void setDisplayFilter(QSharedPointer<KisDisplayFilter> displayFilter) = 0;

    /**
     * Updates the cache of the backend by reading from
     * an associated image. All data transfers with
     * KisImage should happen here
     */
    virtual void updateCache(const QRect &dirtyImageRect) = 0;

    /**
     * Prescales the cache of the backend. It is intended to be
     * called from a separate thread where you can easily
     * do the calculations. No data transfers with KisImage
     * should happen during this phase
     */
    virtual void recalculateCache(KisPPUpdateInfoSP info) = 0;

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
     * be expended by info.borderWidth pixels to all directions and
     * image of this rect will actually be written to the patch's QImage.
     * That is done to eliminate border effects in smooth scaling.
     */
    virtual KisImagePatch getNearestPatch(KisPPUpdateInfoSP info) = 0;

    /**
     * Draws a piece of original image onto @p gc 's canvas
     * @param info.imageRect - area in KisImage pixels where to read from
     * @param info.viewportRect - area in canvas pixels where to write to
     * If info.imageRect and info.viewportRect don't agree, the image
     * will be scaled
     * @param info.borderWidth has the same meaning as in getNearestPatch
     * @param info.renderHints - hints, transmitted to QPainter during darwing
     */
    virtual void drawFromOriginalImage(QPainter& gc,
                                       KisPPUpdateInfoSP info) = 0;
};

#endif /* KIS_PROJECTION_BACKEND */
