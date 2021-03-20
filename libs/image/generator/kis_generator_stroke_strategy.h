/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2020 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <QObject>
#include <QSharedPointer>
#include <kis_generator.h>
#include <kis_generator_layer.h>
#include <KisRunnableBasedStrokeStrategy.h>

class KisGeneratorStrokeStrategy: public QObject, public KisRunnableBasedStrokeStrategy
{
    Q_OBJECT
public:
    KisGeneratorStrokeStrategy();
    ~KisGeneratorStrokeStrategy() override;

    static QVector<KisStrokeJobData *> createJobsData(const KisGeneratorLayerSP layer, QSharedPointer<bool> cookie, const KisGeneratorSP f, const KisPaintDeviceSP dev, const QRegion &rc, const KisFilterConfigurationSP filterConfig);
};
