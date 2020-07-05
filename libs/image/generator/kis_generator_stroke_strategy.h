/*
 * This file is part of Krita
 *
 * Copyright (c) 2020 L. E. Segovia <amy@amyspark.me>
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

#include <QObject>
#include <kis_generator.h>
#include <kis_generator_layer.h>
#include "kis_simple_stroke_strategy.h"

class KisGeneratorStrokeStrategy: public QObject, public KisSimpleStrokeStrategy
{
    Q_OBJECT
public:
    KisGeneratorStrokeStrategy(KisImageWSP image);
    ~KisGeneratorStrokeStrategy() override;

    static QList<KisStrokeJobData *> createJobsData(KisGeneratorLayer *layer, KisGeneratorSP f, KisPaintDeviceSP dev, const QRect &rc, const KisFilterConfigurationSP filterConfig, QSharedPointer<KisProcessingVisitor::ProgressHelper> helper);

private:
    void initStrokeCallback() override;
    void doStrokeCallback(KisStrokeJobData *data) override;
    void finishStrokeCallback() override;
    void cancelStrokeCallback() override;

private:
    struct Private;
    KisImageSP m_image;
};
