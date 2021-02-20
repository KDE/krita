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
};

KisColorSamplerStrokeStrategy::KisColorSamplerStrokeStrategy(int lod)
    : KisSimpleStrokeStrategy(QLatin1String("KisColorSamplerStrokeStrategy")),
      m_d(new Private)
{
    setSupportsWrapAroundMode(true);
    enableJob(KisSimpleStrokeStrategy::JOB_DOSTROKE);

    KisToolUtils::ColorSamplerConfig config;
    config.load();

    m_d->radius = qMax(1, qRound(config.radius * KisLodTransform::lodToScale(lod)));
    m_d->blend = config.blend;
}

KisColorSamplerStrokeStrategy::~KisColorSamplerStrokeStrategy()
{
}

void KisColorSamplerStrokeStrategy::doStrokeCallback(KisStrokeJobData *data)
{
    if (m_d->shouldSkipWork) return;

    Data *d = dynamic_cast<Data*>(data);
    KIS_ASSERT_RECOVER_RETURN(d);

    KoColor color;
    KoColor previous = d->currentColor;
    if (KisToolUtils::sampleColor(color, d->dev, d->pt, &previous, m_d->radius, m_d->blend) == true) {
        emit sigColorUpdated(color);
    }
}

KisStrokeStrategy* KisColorSamplerStrokeStrategy::createLodClone(int levelOfDetail)
{
    m_d->shouldSkipWork = true;

    KisColorSamplerStrokeStrategy *lodStrategy = new KisColorSamplerStrokeStrategy(levelOfDetail);
    connect(lodStrategy, &KisColorSamplerStrokeStrategy::sigColorUpdated,
            this, &KisColorSamplerStrokeStrategy::sigColorUpdated,
            Qt::DirectConnection);
    return lodStrategy;
}
