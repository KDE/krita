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
#include <KisRunnableStrokeJobUtils.h>
#include <filter/kis_filter_configuration.h>
#include <kis_generator_layer.h>
#include <kis_processing_information.h>
#include <kis_processing_visitor.h>
#include <kis_selection.h>
#include <krita_utils.h>

#include "kis_generator_stroke_strategy.h"

KisGeneratorStrokeStrategy::KisGeneratorStrokeStrategy()
    : KisRunnableBasedStrokeStrategy(QLatin1String("KisGeneratorLayer"), kundo2_noi18n("KisGeneratorLayer"))
{
    enableJob(KisSimpleStrokeStrategy::JOB_INIT, true, KisStrokeJobData::BARRIER, KisStrokeJobData::EXCLUSIVE);
    enableJob(KisSimpleStrokeStrategy::JOB_DOSTROKE);

    setRequestsOtherStrokesToEnd(false);
    setClearsRedoOnStart(false);
    setCanForgetAboutMe(false);
}

QVector<KisStrokeJobData *> KisGeneratorStrokeStrategy::createJobsData(KisGeneratorLayerSP layer, QSharedPointer<bool> cookie, KisGeneratorSP f, KisPaintDeviceSP dev, const QRegion &region, const KisFilterConfigurationSP filterConfig)
{
    QVector<KisStrokeJobData *> jobsData;

    QSharedPointer<KisProcessingVisitor::ProgressHelper> helper(new KisProcessingVisitor::ProgressHelper(layer));

    auto process = [layer, helper, cookie, f, filterConfig](KisProcessingInformation dstCfg, QRect tile) {
        const_cast<QSharedPointer<bool> &>(cookie).clear();
        f->generate(dstCfg, tile.size(), filterConfig, helper->updater());

        // HACK ALERT!!!
        // this avoids cyclic loop with KisRecalculateGeneratorLayerJob::run()
        const_cast<KisGeneratorLayerSP&>(layer)->setDirty(QVector<QRect>({tile}));
    };

    for (auto rc {region.begin()}; rc != region.end(); rc++) {
        using KritaUtils::addJobConcurrent;

        if (f->allowsSplittingIntoPatches()) {
            using KritaUtils::optimalPatchSize;
            using KritaUtils::splitRectIntoPatches;

            QVector<QRect> tiles = splitRectIntoPatches(*rc, optimalPatchSize());

            Q_FOREACH (const QRect &tile, tiles) {
                KisProcessingInformation dstCfg(dev, tile.topLeft(), KisSelectionSP());
                addJobConcurrent(jobsData, [process, dstCfg, tile] {
                    process(dstCfg, tile);
                });
            }
        } else {
            KisProcessingInformation dstCfg(dev, (*rc).topLeft(), KisSelectionSP());
            addJobConcurrent(jobsData, [process, dstCfg, rc] {
                process(dstCfg, *rc);
            });
        }
    }

    return jobsData;
}
KisGeneratorStrokeStrategy::~KisGeneratorStrokeStrategy()
{
}
