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
#include "kis_simple_stroke_strategy.h"


class OverviewThumbnailStrokeStrategy : public QObject, public KisSimpleStrokeStrategy
{
    Q_OBJECT
public:
    OverviewThumbnailStrokeStrategy(KisPaintDeviceSP device, const QRect& rect, const QSize& thumbnailSize, bool isPixelArt = false);
    ~OverviewThumbnailStrokeStrategy() override;

    KisStrokeStrategy* createLodClone(int levelOfDetail) override;

private:
    void initStrokeCallback() override;
    void doStrokeCallback(KisStrokeJobData *data) override;
    void finishStrokeCallback() override;
    void cancelStrokeCallback() override;

Q_SIGNALS:
    //Emitted when thumbnail is updated and overviewImage is fully generated.
    void thumbnailUpdated(QImage pixmap);


private:
    class ProcessData;

    KisPaintDeviceSP m_device;
    QRect m_rect;
    QSize m_thumbnailSize;
    QSize m_thumbnailOversampledSize;
    bool m_isPixelArt {false};
    KisPaintDeviceSP m_thumbnailDevice;
};

#endif // OVERVIEWTHUMBNAILSTROKESTRATEGY_H
