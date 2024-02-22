/*
 *  SPDX-FileCopyrightText: 2023 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISCHANNELSTHUMBNAILSSTROKESTRATEGY_H
#define KISCHANNELSTHUMBNAILSSTROKESTRATEGY_H

#include <KisImageThumbnailStrokeStrategy.h>


class KisChannelsThumbnailsStrokeStrategy : public KisImageThumbnailStrokeStrategyBase
{
    Q_OBJECT
public:
    using KisImageThumbnailStrokeStrategyBase::KisImageThumbnailStrokeStrategyBase;

protected:
    void reportThumbnailGenerationCompleted(KisPaintDeviceSP device, const QRect &rect) override;

Q_SIGNALS:
    //Emitted when thumbnail is updated and overviewImage is fully generated.
    void thumbnailsUpdated(const QVector<QImage> &channels, const KoColorSpace *cs);

};

Q_DECLARE_METATYPE(QVector<QImage>)

#endif // KISCHANNELSTHUMBNAILSSTROKESTRATEGY_H
