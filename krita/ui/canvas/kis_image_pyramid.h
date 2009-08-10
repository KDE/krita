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

#ifndef __KIS_IMAGE_PYRAMID
#define __KIS_IMAGE_PYRAMID

#include <QImage>
#include <QVector>
#include <QThreadPool>

#include <KoColorSpace.h>
#include <kis_image.h>
#include <kis_paint_device.h>
#include "kis_projection_backend.h"


class KisImagePyramid : QObject, public KisProjectionBackend
{
    Q_OBJECT

public:
    KisImagePyramid(qint32 pyramidHeight);
    virtual ~KisImagePyramid();

    void setImage(KisImageSP newImage);
    void setImageSize(qint32 w, qint32 h);
    void setMonitorProfile(const KoColorProfile* monitorProfile);
    void setDirty(const QRect& rc);

    KisImagePatch getNearestPatch(qreal scaleX, qreal scaleY,
                                  const QRect& requestedRect,
                                  qint32 borderWidth);

    void drawFromOriginalImage(QPainter& gc,
                               const QRect& imageRect,
                               const QRectF& viewportRect,
                               qint32 borderWidth,
                               QPainter::RenderHints renderHints);

    /**
     * Render the projection onto a QImage.
     * Color profiling accurs here
     */
    QImage convertToQImage(qreal scale,
                           const QRect& unscaledRect,
                           enum Qt::TransformationMode transformMode);

    QImage convertToQImage(qreal scale,
                           qint32 unscaledX,
                           qint32 unscaledY,
                           qint32 unscaledWidth,
                           qint32 unscaledHeight);

    /**
     * Draw the projection onto a QPainter.
     * Color profiling accurs here
     */
    void drawImage(qreal scale,
                   QPainter& gc,
                   const QPoint& topLeftScaled,
                   const QRect& unscaledSourceRect);

    void alignSourceRect(QRect& rect, qreal scale);

private:
    QVector<KisPaintDeviceSP> m_pyramid;
    //QThreadPool m_pyramidUpdater;
    KisImageSP  m_originalImage;
    const KoColorProfile* m_monitorProfile;
    const KoColorSpace* m_monitorColorSpace;
    KoColorConversionTransformation::Intent m_renderingIntent;

    /**
     * Number of planes inside pyramid
     */
    qint32 m_pyramidHeight;

private:
    void rebuildPyramid();
    void clearPyramid();

    /**
     * Downsamples @srcRect from @src paint device and writes
     * result into proper place of @dst paint device
     * Returns modified rect of @dst paintDevice
     */
    QRect downsampleByFactor2(const QRect& srcRect,
                              KisPaintDevice* src, KisPaintDevice* dst);

    /**
     * Auxiliary function. Downsamples two lines in @srcRow0
     * and @srcRow1 into one line @dstRow
     * Note: @numSrcPixels must be EVEN
     */
    void downsamplePixels(const quint8 *srcRow0, const quint8 *srcRow1,
                          quint8 *dstRow, qint32 numSrcPixels);

    /**
     * Searches for the last pyramid plane that can cover
     * canvans on current zoom level
     */

    int findFirstGoodPlaneIndex(qreal scale, QSize originalSize);


    /**
     * Fast workaround for converting paintDevices
     */
    QImage convertToQImageFast(KisPaintDeviceSP paintDevice,
                               const QRect& unscaledRect);
};

#endif /* __KIS_IMAGE_PYRAMID */
