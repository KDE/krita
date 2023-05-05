/*
 *  SPDX-FileCopyrightText: 2016 Eugene Ingerman geneing at gmail dot com
 *  SPDX-FileCopyrightText: 2020 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef OVERVIEWTHUMBNAILSTROKESTRATEGY_H
#define OVERVIEWTHUMBNAILSTROKESTRATEGY_H

#include <QObject>
#include <QRect>
#include <QSize>
#include <QImage>

#include "kis_types.h"
#include <KoColorConversionTransformation.h>
#include "KisIdleTaskStrokeStrategy.h"

class KoColorProfile;


class OverviewThumbnailStrokeStrategy : public KisIdleTaskStrokeStrategy
{
    Q_OBJECT
public:
    OverviewThumbnailStrokeStrategy(KisPaintDeviceSP device,
                                    const QRect& rect,
                                    const QSize& thumbnailSize,
                                    bool isPixelArt,
                                    const KoColorProfile *profile,
                                    KoColorConversionTransformation::Intent renderingIntent,
                                    KoColorConversionTransformation::ConversionFlags conversionFlags);
    ~OverviewThumbnailStrokeStrategy() override;

private:
    void initStrokeCallback() override;
    void finishStrokeCallback() override;

Q_SIGNALS:
    //Emitted when thumbnail is updated and overviewImage is fully generated.
    void thumbnailUpdated(QImage pixmap);


private:
    KisPaintDeviceSP m_device;
    QRect m_rect;
    QSize m_thumbnailSize;
    QSize m_thumbnailOversampledSize;
    bool m_isPixelArt {false};
    KisPaintDeviceSP m_thumbnailDevice;

    const KoColorProfile *m_profile;
    KoColorConversionTransformation::Intent m_renderingIntent;
    KoColorConversionTransformation::ConversionFlags m_conversionFlags;
};

#endif // OVERVIEWTHUMBNAILSTROKESTRATEGY_H
