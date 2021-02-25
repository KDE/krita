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
#include <QThreadStorage>

#include <KoColorSpace.h>
#include <kis_image.h>
#include <kis_paint_device.h>
#include "kis_projection_backend.h"


class KisImagePyramid : QObject, public KisProjectionBackend
{
    Q_OBJECT

public:
    KisImagePyramid(qint32 pyramidHeight);
    ~KisImagePyramid() override;

    void setImage(KisImageWSP newImage) override;
    void setImageSize(qint32 w, qint32 h) override;
    void setMonitorProfile(const KoColorProfile* monitorProfile, KoColorConversionTransformation::Intent renderingIntent, KoColorConversionTransformation::ConversionFlags conversionFlags) override;
    void setChannelFlags(const QBitArray &channelFlags) override;
    void setDisplayFilter(QSharedPointer<KisDisplayFilter> displayFilter) override;
    void updateCache(const QRect &dirtyImageRect) override;
    void recalculateCache(KisPPUpdateInfoSP info) override;

    KisImagePatch getNearestPatch(KisPPUpdateInfoSP info) override;
    void drawFromOriginalImage(QPainter& gc, KisPPUpdateInfoSP info) override;

    /**
     * Render the projection onto a QImage.
     * Color profiling occurs here
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
     * Color profiling occurs here
     */
    void drawImage(qreal scale,
                   QPainter& gc,
                   const QPoint& topLeftScaled,
                   const QRect& unscaledSourceRect);

    void alignSourceRect(QRect& rect, qreal scale) override;

private:

    void retrieveImageData(const QRect &rect);
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

private Q_SLOTS:

    void configChanged();

private:

    QVector<KisPaintDeviceSP> m_pyramid;
    KisImageWSP  m_originalImage;

    const KoColorProfile* m_monitorProfile {0};
    const KoColorSpace* m_monitorColorSpace {0};

    QSharedPointer<KisDisplayFilter> m_displayFilter;

    KoColorConversionTransformation::Intent m_renderingIntent { KoColorConversionTransformation::IntentPerceptual };
    KoColorConversionTransformation::ConversionFlags m_conversionFlags { KoColorConversionTransformation::Empty };


    /**
     * Number of planes inside pyramid
     */
    qint32 m_pyramidHeight {0};

    bool m_useOcio {false};

    QBitArray m_channelFlags;
    bool m_allChannelsSelected {false};
    bool m_onlyOneChannelSelected {false};
    int m_selectedChannelIndex {-1};

};

#endif /* __KIS_IMAGE_PYRAMID */
