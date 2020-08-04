/*
 *  Copyright (c) 2016 Eugene Ingerman geneing at gmail dot com
 *  Copyright (c) 2020 Dmitry Kazakov <dimula73@gmail.com>
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
    OverviewThumbnailStrokeStrategy(KisPaintDeviceSP device, const QRect& rect, const QSize& thumbnailSize);
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
    KisPaintDeviceSP m_thumbnailDevice;
};

#endif // OVERVIEWTHUMBNAILSTROKESTRATEGY_H
