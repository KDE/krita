/*
 * SPDX-FileCopyrightText: 2020 Ashwin Dhakaita <ashwingpdhakaita@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "MyPaintPaintOpSettings.h"

#include <cmath>
#include <KisOptimizedBrushOutline.h>
#include <MyPaintStandardOptionData.h>

#include <KisValueCache.h>


struct KisMyPaintOpSettings::Private
{
    struct Cache
    {
        qreal paintOpSize{0.0};
        qreal paintOpAngle{0.0};  // the original angle value as seen by KisPaintOpSettings interface
        qreal offsetValue{0.0};
    };

    struct CacheInitializer {
        CacheInitializer(KisMyPaintOpSettings *q) : m_q(q) {}

        Cache initialize() {
            Cache value;

            {
                MyPaintOffsetByRandomData data;
                data.read(m_q);
                value.offsetValue = data.strengthValue;
            }

            {
                MyPaintRadiusLogarithmicData data;
                data.read(m_q);
                value.paintOpSize = 2 * exp(data.strengthValue);
            }

            {
                MyPaintEllipticalDabAngleData data;
                data.read(m_q);
                value.paintOpAngle = 180.0 - data.strengthValue;
            }

            return value;
        }

        KisMyPaintOpSettings *m_q;
    };

    Private(KisMyPaintOpSettings *q)
        : cache(q)
    {
    }

    KisValueCache<CacheInitializer> cache;
};


KisMyPaintOpSettings::KisMyPaintOpSettings(KisResourcesInterfaceSP resourcesInterface)
    : KisOutlineGenerationPolicy<KisPaintOpSettings>(KisCurrentOutlineFetcher::SIZE_OPTION |
                                                     KisCurrentOutlineFetcher::ROTATION_OPTION,
                                                     resourcesInterface),
    m_d(new Private(this))
{
}

KisMyPaintOpSettings::~KisMyPaintOpSettings()
{
}

void KisMyPaintOpSettings::setPaintOpSize(qreal value)
{
    MyPaintRadiusLogarithmicData data;
    data.read(this);
    data.strengthValue = log(0.5 * value);
    data.write(this);
}

qreal KisMyPaintOpSettings::paintOpSize() const
{
    return m_d->cache.value().paintOpSize;
}

void KisMyPaintOpSettings::setPaintOpAngle(qreal value)
{
    MyPaintEllipticalDabAngleData data;
    data.read(this);

    value = normalizeAngleDegrees(value);

    if (value > 180.0) {
        value -= 180.0;
    }

    /**
     * All brushes are rotated in Krita counterclockwise,
     * so we should invert the value for MyPaint
     */
    value = 180.0 - value;

    data.strengthValue = value;
    data.write(this);
}

qreal KisMyPaintOpSettings::paintOpAngle() const
{
    return m_d->cache.value().paintOpAngle;
}

void KisMyPaintOpSettings::setPaintOpOpacity(qreal value)
{
    MyPaintOpacityData data;
    data.read(this);
    data.strengthValue = value;
    data.write(this);
}

qreal KisMyPaintOpSettings::paintOpOpacity()
{
    MyPaintOpacityData data;
    data.read(this);
    return data.strengthValue;
}

bool KisMyPaintOpSettings::paintIncremental()
{
    return true;
}

void KisMyPaintOpSettings::resetSettings(const QStringList &preserveProperties)
{
    QStringList allKeys = preserveProperties;
    allKeys << MYPAINT_JSON;
    KisOutlineGenerationPolicy<KisPaintOpSettings>::resetSettings(allKeys);
}

void KisMyPaintOpSettings::onPropertyChanged()
{
    m_d->cache.clear();
    KisOutlineGenerationPolicy::onPropertyChanged();
}

KisOptimizedBrushOutline KisMyPaintOpSettings::brushOutline(const KisPaintInformation &info, const OutlineMode &mode, qreal alignForZoom)
{
    KisOptimizedBrushOutline path;

    if (mode.isVisible) {
        qreal finalScale = 1.0;

        const qreal offset = m_d->cache.value().offsetValue;

        qreal radius = 0.5 * m_d->cache.value().paintOpSize;
        radius = radius + 2 * radius * offset;
        radius = qBound(3.5, radius, 500.0);

        QPainterPath realOutline;
        realOutline.addEllipse(QPointF(), radius, radius);

        path = outlineFetcher()->fetchOutline(info, this, realOutline, mode, alignForZoom, finalScale);

        if (mode.showTiltDecoration) {
            QPainterPath tiltLine = makeTiltIndicator(info,
                realOutline.boundingRect().center(),
                realOutline.boundingRect().width() * 0.5,
                3.0);
            path.addPath(outlineFetcher()->fetchOutline(info, this, tiltLine, mode, alignForZoom, finalScale, 0.0, true, realOutline.boundingRect().center().x(), realOutline.boundingRect().center().y()));
        }
    }

    return path;
}
