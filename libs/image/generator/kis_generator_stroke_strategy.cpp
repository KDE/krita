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
#include <filter/kis_filter_configuration.h>
#include <kis_generator_layer.h>
#include <kis_processing_information.h>
#include <kis_selection.h>

#include "kis_generator_stroke_strategy.h"

struct KisGeneratorStrokeStrategy::Private {
    class ProcessData : public KisStrokeJobData
    {
    public:
        ProcessData(KisGeneratorLayer* _layer, KisGeneratorSP _f, KisProcessingInformation &_dstCfg, QRect _rect, KisFilterConfigurationSP _filterConfig, KoUpdater *_updater)
            : KisStrokeJobData(CONCURRENT)
            ,layer(_layer)
            ,f(_f)
            ,dstCfg(_dstCfg)
            ,tile(_rect)
            ,filterConfig(_filterConfig)
            ,updater(_updater)
        {
        }

        KisGeneratorLayer* layer;
        KisGeneratorSP f;
        KisProcessingInformation dstCfg;
        QRect tile;
        KisFilterConfigurationSP filterConfig;
        KoUpdater *updater;
    };
};

KisGeneratorStrokeStrategy::KisGeneratorStrokeStrategy(KisImageWSP image)
    : KisSimpleStrokeStrategy(QLatin1String("KisGenerator"))
    , m_image(image)
{
    enableJob(KisSimpleStrokeStrategy::JOB_INIT, true, KisStrokeJobData::BARRIER, KisStrokeJobData::EXCLUSIVE);
    enableJob(KisSimpleStrokeStrategy::JOB_DOSTROKE);
    enableJob(KisSimpleStrokeStrategy::JOB_CANCEL, true, KisStrokeJobData::SEQUENTIAL, KisStrokeJobData::EXCLUSIVE);

    setRequestsOtherStrokesToEnd(false);
    setClearsRedoOnStart(false);
    setCanForgetAboutMe(true);
}

KisStrokeJobData* KisGeneratorStrokeStrategy::createJobData(KisGeneratorLayer* layer, KisGeneratorSP f, KisPaintDeviceSP dev, const QRect &rc, const KisFilterConfigurationSP filterConfig, KisProcessingVisitor::ProgressHelper &helper)
{
    KisProcessingInformation dstCfg(dev, rc.topLeft(), KisSelectionSP());

    return new Private::ProcessData(layer, f, dstCfg, rc, filterConfig, helper.updater());
}

KisGeneratorStrokeStrategy::~KisGeneratorStrokeStrategy()
{
}

void KisGeneratorStrokeStrategy::initStrokeCallback()
{
}

void KisGeneratorStrokeStrategy::doStrokeCallback(KisStrokeJobData *data)
{
    Private::ProcessData *d_pd = dynamic_cast<Private::ProcessData *>(data);
    if (d_pd) {
        d_pd->f->generate(d_pd->dstCfg, d_pd->tile.size(), d_pd->filterConfig, d_pd->updater);

        // HACK ALERT!!!
        // this avoids cyclic loop with KisRecalculateGeneratorLayerJob::run()
        d_pd->layer->setDirty(d_pd->tile);
        return;
    }
}

void KisGeneratorStrokeStrategy::finishStrokeCallback()
{
}

void KisGeneratorStrokeStrategy::cancelStrokeCallback()
{
}
