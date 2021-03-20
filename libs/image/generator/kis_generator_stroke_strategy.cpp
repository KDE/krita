/*
 * This file is part of Krita
 *
 * SPDX-FileCopyrightText: 2020 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
    : KisRunnableBasedStrokeStrategy(QLatin1String("KisGenerator"), kundo2_i18n("Fill Layer Render"))
{
    enableJob(KisSimpleStrokeStrategy::JOB_INIT, true, KisStrokeJobData::BARRIER, KisStrokeJobData::EXCLUSIVE);
    enableJob(KisSimpleStrokeStrategy::JOB_DOSTROKE);

    setRequestsOtherStrokesToEnd(false);
    setClearsRedoOnStart(false);
    setCanForgetAboutMe(false);
}

QVector<KisStrokeJobData *>KisGeneratorStrokeStrategy::createJobsData(const KisGeneratorLayerSP layer, QSharedPointer<bool> cookie, const KisGeneratorSP f, const KisPaintDeviceSP dev, const QRegion &region, const KisFilterConfigurationSP filterConfig)
{
    using namespace KritaUtils;

    QVector<KisStrokeJobData *> jobsData;

    QSharedPointer<KisProcessingVisitor::ProgressHelper> helper(new KisProcessingVisitor::ProgressHelper(layer));

    addJobBarrier(jobsData, nullptr);

    for (const auto& rc: region) {
        if (f->allowsSplittingIntoPatches()) {
            QVector<QRect> tiles = splitRectIntoPatches(rc, optimalPatchSize());

            for(const auto& tile: tiles) {
                KisProcessingInformation dstCfg(dev, tile.topLeft(), KisSelectionSP());
                addJobConcurrent(jobsData, [=]() {
                    const_cast<QSharedPointer<bool> &>(cookie).clear();

                    f->generate(dstCfg, tile.size(), filterConfig, helper->updater());

                    // HACK ALERT!!!
                    // this avoids cyclic loop with KisRecalculateGeneratorLayerJob::run()
                    const_cast<KisGeneratorLayerSP &>(layer)->setDirtyWithoutUpdate({tile});
                });
            }
        } else {
            KisProcessingInformation dstCfg(dev, rc.topLeft(), KisSelectionSP());

            addJobSequential(jobsData, [=]() {
                const_cast<QSharedPointer<bool>&>(cookie).clear();

                f->generate(dstCfg, rc.size(), filterConfig, helper->updater());

                // HACK ALERT!!!
                // this avoids cyclic loop with KisRecalculateGeneratorLayerJob::run()
                const_cast<KisGeneratorLayerSP &>(layer)->setDirtyWithoutUpdate({rc});
            });
        }
    }

    return jobsData;
}
KisGeneratorStrokeStrategy::~KisGeneratorStrokeStrategy()
{
}
