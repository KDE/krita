/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_color_sampler_stroke_strategy.h"

#include "kis_tool_utils.h"
#include "kis_paint_device.h"

struct KisColorSamplerStrokeStrategy::Private
{
    Private() : shouldSkipWork(false) {}

    bool shouldSkipWork;
    int radius = 1;
    int blend = 100;

    boost::optional<KoColor> lastSelectedColor;
};

KisColorSamplerStrokeStrategy::KisColorSamplerStrokeStrategy(int radius, int blend, int lod)
    : KisSimpleStrokeStrategy(QLatin1String("KisColorSamplerStrokeStrategy")),
      m_d(new Private)
{
    setSupportsWrapAroundMode(true);
    setClearsRedoOnStart(false);
    enableJob(KisSimpleStrokeStrategy::JOB_DOSTROKE);

    m_d->radius = qMax(1, qRound(radius * KisLodTransform::lodToScale(lod)));
    m_d->blend = blend;
}

KisColorSamplerStrokeStrategy::~KisColorSamplerStrokeStrategy()
{
}

void KisColorSamplerStrokeStrategy::doStrokeCallback(KisStrokeJobData *data)
{
    if (m_d->shouldSkipWork) return;

    Data *d = dynamic_cast<Data*>(data);
    FinalizeData *finalize = dynamic_cast<FinalizeData*>(data);

    if (d) {
        KoColor color;
        KoColor previous = d->currentColor;
        if (KisToolUtils::sampleColor(color, d->dev, d->pt, &previous, m_d->radius, m_d->blend)) {
            m_d->lastSelectedColor = color;
            emit sigColorUpdated(color);
        }
    } else if (finalize) {
        if (m_d->lastSelectedColor) {
            emit sigFinalColorSelected(*m_d->lastSelectedColor);
        }
    }
}

KisStrokeStrategy* KisColorSamplerStrokeStrategy::createLodClone(int levelOfDetail)
{
    m_d->shouldSkipWork = true;

    KisColorSamplerStrokeStrategy *lodStrategy = new KisColorSamplerStrokeStrategy(m_d->radius, m_d->blend, levelOfDetail);
    connect(lodStrategy, &KisColorSamplerStrokeStrategy::sigColorUpdated,
            this, &KisColorSamplerStrokeStrategy::sigColorUpdated,
            Qt::DirectConnection);
    return lodStrategy;
}
