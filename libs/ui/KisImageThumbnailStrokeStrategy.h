/*
 *  SPDX-FileCopyrightText: 2016 Eugene Ingerman geneing at gmail dot com
 *  SPDX-FileCopyrightText: 2020 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISIMAGETHUMBNAILSTROKESTRATEGY_H
#define KISIMAGETHUMBNAILSTROKESTRATEGY_H

#include <QObject>
#include <QRect>
#include <QSize>
#include <QImage>

#include "kritaui_export.h"
#include "kis_types.h"
#include <KoColorConversionTransformation.h>
#include "KisIdleTaskStrokeStrategy.h"

class KoColorProfile;


class KRITAUI_EXPORT KisImageThumbnailStrokeStrategyBase : public KisIdleTaskStrokeStrategy
{
    Q_OBJECT
public:
    KisImageThumbnailStrokeStrategyBase(KisPaintDeviceSP device,
                                        const QRect& rect,
                                        const QSize& thumbnailSize,
                                        bool isPixelArt,
                                        const KoColorProfile *profile,
                                        KoColorConversionTransformation::Intent renderingIntent,
                                        KoColorConversionTransformation::ConversionFlags conversionFlags);
    ~KisImageThumbnailStrokeStrategyBase() override;

private:
    void initStrokeCallback() override;

protected:
    virtual void reportThumbnailGenerationCompleted(KisPaintDeviceSP device, const QRect &rect) = 0;

private:
    KisPaintDeviceSP m_device;
    QRect m_rect;
    QSize m_thumbnailSize;
    QSize m_thumbnailOversampledSize;
    bool m_isPixelArt {false};
    KisPaintDeviceSP m_thumbnailDevice;

protected:
    const KoColorProfile *m_profile;
    KoColorConversionTransformation::Intent m_renderingIntent;
    KoColorConversionTransformation::ConversionFlags m_conversionFlags;
};

class KRITAUI_EXPORT KisImageThumbnailStrokeStrategy : public KisImageThumbnailStrokeStrategyBase
{
    Q_OBJECT
public:
    using KisImageThumbnailStrokeStrategyBase::KisImageThumbnailStrokeStrategyBase;


protected:
    virtual void reportThumbnailGenerationCompleted(KisPaintDeviceSP device, const QRect &rect);

Q_SIGNALS:
    //Emitted when thumbnail is updated and overviewImage is fully generated.
    void thumbnailUpdated(QImage pixmap);
};

#endif // KISIMAGETHUMBNAILSTROKESTRATEGY_H
