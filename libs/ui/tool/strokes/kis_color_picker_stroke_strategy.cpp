/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_color_picker_stroke_strategy.h"

#include "kis_tool_utils.h"
#include "kis_paint_device.h"

struct KisColorPickerStrokeStrategy::Private
{
    Private() : shouldSkipWork(false) {}

    bool shouldSkipWork;
    int radius = 1;
    int blend = 100;
};

KisColorPickerStrokeStrategy::KisColorPickerStrokeStrategy(int lod)
    : KisSimpleStrokeStrategy(QLatin1String("KisColorPickerStrokeStrategy")),
      m_d(new Private)
{
    setSupportsWrapAroundMode(true);
    enableJob(KisSimpleStrokeStrategy::JOB_DOSTROKE);

    KisToolUtils::ColorPickerConfig config;
    config.load();

    m_d->radius = qMax(1, qRound(config.radius * KisLodTransform::lodToScale(lod)));
    m_d->blend = config.blend;
}

KisColorPickerStrokeStrategy::~KisColorPickerStrokeStrategy()
{
}

void KisColorPickerStrokeStrategy::doStrokeCallback(KisStrokeJobData *data)
{
    if (m_d->shouldSkipWork) return;

    Data *d = dynamic_cast<Data*>(data);
    KIS_ASSERT_RECOVER_RETURN(d);

    KoColor color;
    KoColor previous = d->currentColor;
    if (KisToolUtils::pickColor(color, d->dev, d->pt, &previous, m_d->radius, m_d->blend) == true) {
        emit sigColorUpdated(color);
    }
}

KisStrokeStrategy* KisColorPickerStrokeStrategy::createLodClone(int levelOfDetail)
{
    m_d->shouldSkipWork = true;

    KisColorPickerStrokeStrategy *lodStrategy = new KisColorPickerStrokeStrategy(levelOfDetail);
    connect(lodStrategy, &KisColorPickerStrokeStrategy::sigColorUpdated,
            this, &KisColorPickerStrokeStrategy::sigColorUpdated,
            Qt::DirectConnection);
    return lodStrategy;
}
